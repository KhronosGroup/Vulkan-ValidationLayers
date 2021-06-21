/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#include "image_state.h"
#include "pipeline_state.h"
#include "descriptor_sets.h"

static VkImageSubresourceRange MakeImageFullRange(const VkImageCreateInfo &create_info) {
    const auto format = create_info.format;
    VkImageSubresourceRange init_range{0, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    const VkExternalFormatANDROID *external_format_android = LvlFindInChain<VkExternalFormatANDROID>(&create_info);
    bool is_external_format_conversion = (external_format_android != nullptr && external_format_android->externalFormat != 0);
#else
    bool is_external_format_conversion = false;
#endif

    if (FormatIsColor(format) || FormatIsMultiplane(format) || is_external_format_conversion) {
        init_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // Normalization will expand this for multiplane
    } else {
        init_range.aspectMask =
            (FormatHasDepth(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) | (FormatHasStencil(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    }
    return NormalizeSubresourceRange(create_info, init_range);
}

uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels) {
    // Return correct number of mip levels taking into account VK_REMAINING_MIP_LEVELS
    uint32_t mip_level_count = range->levelCount;
    if (range->levelCount == VK_REMAINING_MIP_LEVELS) {
        mip_level_count = mip_levels - range->baseMipLevel;
    }
    return mip_level_count;
}

uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers) {
    // Return correct number of layers taking into account VK_REMAINING_ARRAY_LAYERS
    uint32_t array_layer_count = range->layerCount;
    if (range->layerCount == VK_REMAINING_ARRAY_LAYERS) {
        array_layer_count = layers - range->baseArrayLayer;
    }
    return array_layer_count;
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info,
                                                  const VkImageSubresourceRange &range) {
    VkImageSubresourceRange norm = range;
    norm.levelCount = ResolveRemainingLevels(&range, image_create_info.mipLevels);

    // Special case for 3D images with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT flag bit, where <extent.depth> and
    // <arrayLayers> can potentially alias.
    uint32_t layer_limit = (0 != (image_create_info.flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT))
                               ? image_create_info.extent.depth
                               : image_create_info.arrayLayers;
    norm.layerCount = ResolveRemainingLayers(&range, layer_limit);

    // For multiplanar formats, IMAGE_ASPECT_COLOR is equivalent to adding the aspect of the individual planes
    VkImageAspectFlags &aspect_mask = norm.aspectMask;
    if (FormatIsMultiplane(image_create_info.format)) {
        if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
            aspect_mask &= ~VK_IMAGE_ASPECT_COLOR_BIT;
            aspect_mask |= (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
            if (FormatPlaneCount(image_create_info.format) > 2) {
                aspect_mask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
            }
        }
    }
    return norm;
}

static VkExternalMemoryHandleTypeFlags GetExternalHandleType(const VkImageCreateInfo *pCreateInfo) {
    auto *external_memory_info = LvlFindInChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

IMAGE_STATE::IMAGE_STATE(VkDevice dev, VkImage img, const VkImageCreateInfo *pCreateInfo)
    : BINDABLE(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleType(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      valid(false),
      acquired(false),
      shared_presentable(false),
      layout_locked(false),
      get_sparse_reqs_called(false),
      sparse_metadata_required(false),
      sparse_metadata_bound(false),
      has_ahb_format(false),
      is_swapchain_image(false),
      ahb_format(0),
      full_range{MakeImageFullRange(createInfo)},
      create_from_swapchain(VK_NULL_HANDLE),
      bind_swapchain(VK_NULL_HANDLE),
      bind_swapchain_imageIndex(0),
      range_encoder(full_range),
      disjoint(false),
      requirements{},
      memory_requirements_checked{{false, false, false}},
      subresource_encoder(full_range),
      fragment_encoder(nullptr),
      store_device_as_workaround(dev),  // TODO REMOVE WHEN encoder can be const
      swapchain_fake_address(0U),
      sparse_requirements{} {
    if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
        uint32_t *queue_family_indices = new uint32_t[createInfo.queueFamilyIndexCount];
        for (uint32_t i = 0; i < createInfo.queueFamilyIndexCount; i++) {
            queue_family_indices[i] = pCreateInfo->pQueueFamilyIndices[i];
        }
        createInfo.pQueueFamilyIndices = queue_family_indices;
    }
}

void IMAGE_STATE::Destroy() {
    for (auto *alias_state : aliasing_images) {
        assert(alias_state);
        alias_state->aliasing_images.erase(this);
    }
    aliasing_images.clear();
    BINDABLE::Destroy();
}

void IMAGE_STATE::NotifyInvalidate(const LogObjectList &invalid_handles, bool unlink) {
    BINDABLE::NotifyInvalidate(invalid_handles, unlink);
    if (unlink) {
        aliasing_images.clear();
    }
}

bool IMAGE_STATE::IsCreateInfoEqual(const VkImageCreateInfo &other_createInfo) const {
    bool is_equal = (createInfo.sType == other_createInfo.sType) && (createInfo.flags == other_createInfo.flags);
    is_equal = is_equal && IsImageTypeEqual(other_createInfo) && IsFormatEqual(other_createInfo);
    is_equal = is_equal && IsMipLevelsEqual(other_createInfo) && IsArrayLayersEqual(other_createInfo);
    is_equal = is_equal && IsUsageEqual(other_createInfo) && IsInitialLayoutEqual(other_createInfo);
    is_equal = is_equal && IsExtentEqual(other_createInfo) && IsTilingEqual(other_createInfo);
    is_equal = is_equal && IsSamplesEqual(other_createInfo) && IsSharingModeEqual(other_createInfo);
    return is_equal &&
           ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) ? IsQueueFamilyIndicesEqual(other_createInfo) : true);
}

// Check image compatibility rules for VK_NV_dedicated_allocation_image_aliasing
bool IMAGE_STATE::IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_createInfo) const {
    bool is_compatible = (createInfo.sType == other_createInfo.sType) && (createInfo.flags == other_createInfo.flags);
    is_compatible = is_compatible && IsImageTypeEqual(other_createInfo) && IsFormatEqual(other_createInfo);
    is_compatible = is_compatible && IsMipLevelsEqual(other_createInfo);
    is_compatible = is_compatible && IsUsageEqual(other_createInfo) && IsInitialLayoutEqual(other_createInfo);
    is_compatible = is_compatible && IsSamplesEqual(other_createInfo) && IsSharingModeEqual(other_createInfo);
    is_compatible = is_compatible &&
                    ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) ? IsQueueFamilyIndicesEqual(other_createInfo) : true);
    is_compatible = is_compatible && IsTilingEqual(other_createInfo);

    is_compatible = is_compatible && createInfo.extent.width <= other_createInfo.extent.width &&
                    createInfo.extent.height <= other_createInfo.extent.height &&
                    createInfo.extent.depth <= other_createInfo.extent.depth &&
                    createInfo.arrayLayers <= other_createInfo.arrayLayers;
    return is_compatible;
}

bool IMAGE_STATE::IsCompatibleAliasing(IMAGE_STATE *other_image_state) const {
    if (!is_swapchain_image && !other_image_state->is_swapchain_image &&
        !(createInfo.flags & other_image_state->createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT)) {
        return false;
    }
    const auto binding = Binding();
    const auto other_binding = other_image_state->Binding();
    if ((create_from_swapchain == VK_NULL_HANDLE) && binding && other_binding && (binding->mem_state == other_binding->mem_state) &&
        (binding->offset == other_binding->offset) && IsCreateInfoEqual(other_image_state->createInfo)) {
        return true;
    }
    if ((bind_swapchain == other_image_state->bind_swapchain) && (bind_swapchain != VK_NULL_HANDLE)) {
        return true;
    }
    return false;
}

void IMAGE_STATE::AddAliasingImage(IMAGE_STATE *bound_image) {
    assert(bound_image);
    if (bound_image != this && bound_image->IsCompatibleAliasing(this)) {
        auto inserted = bound_image->aliasing_images.emplace(this);
        if (inserted.second) {
            aliasing_images.emplace(bound_image);
        }
    }
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
VkExtent3D IMAGE_STATE::GetSubresourceExtent(const VkImageSubresourceLayers &subresource) const {
    const uint32_t mip = subresource.mipLevel;

    // Return zero extent if mip level doesn't exist
    if (mip >= createInfo.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    // Don't allow mip adjustment to create 0 dim, but pass along a 0 if that's what subresource specified
    VkExtent3D extent = createInfo.extent;

    // If multi-plane, adjust per-plane extent
    if (FormatIsMultiplane(createInfo.format)) {
        VkExtent2D divisors = FindMultiplaneExtentDivisors(createInfo.format, subresource.aspectMask);
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    if (createInfo.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
        extent.width = (0 == extent.width ? 0 : std::max(2U, 1 + ((extent.width - 1) >> mip)));
        extent.height = (0 == extent.height ? 0 : std::max(2U, 1 + ((extent.height - 1) >> mip)));
        extent.depth = (0 == extent.depth ? 0 : std::max(2U, 1 + ((extent.depth - 1) >> mip)));
    } else {
        extent.width = (0 == extent.width ? 0 : std::max(1U, extent.width >> mip));
        extent.height = (0 == extent.height ? 0 : std::max(1U, extent.height >> mip));
        extent.depth = (0 == extent.depth ? 0 : std::max(1U, extent.depth >> mip));
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != createInfo.imageType) {
        extent.depth = createInfo.arrayLayers;
    }

    return extent;
}

IMAGE_VIEW_STATE::IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &im, VkImageView iv, const VkImageViewCreateInfo *ci)
    : BASE_NODE(iv, kVulkanObjectTypeImageView),
      create_info(*ci),
      normalized_subresource_range(::NormalizeSubresourceRange(im->createInfo, ci->subresourceRange)),
      range_generator(im->subresource_encoder, normalized_subresource_range),
      samplerConversion(VK_NULL_HANDLE),
      image_state(im) {
    auto *conversion_info = LvlFindInChain<VkSamplerYcbcrConversionInfo>(create_info.pNext);
    if (conversion_info) samplerConversion = conversion_info->conversion;
    if (image_state) {
        // A light normalization of the createInfo range
        auto &sub_res_range = create_info.subresourceRange;
        sub_res_range.levelCount = ResolveRemainingLevels(&sub_res_range, image_state->createInfo.mipLevels);
        sub_res_range.layerCount = ResolveRemainingLayers(&sub_res_range, image_state->createInfo.arrayLayers);

        // Cache a full normalization (for "full image/whole image" comparisons)
        // normalized_subresource_range = NormalizeSubresourceRange(*image_state, ci->subresourceRange);
        samples = image_state->createInfo.samples;

        if (image_state->has_ahb_format) {
            // When the image has a external format the views format must be VK_FORMAT_UNDEFINED and it is required to use a sampler
            // Ycbcr conversion. Thus we can't extract any meaningful information from the format parameter. As a Sampler Ycbcr
            // conversion must be used the shader type is always float.
            descriptor_format_bits = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
        } else {
            descriptor_format_bits = DescriptorRequirementsBitsFromFormat(create_info.format);
        }
        image_state->AddParent(this);
    }
}
