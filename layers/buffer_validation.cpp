/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Shannon McPherson <shannon@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include <cmath>
#include <set>
#include <sstream>
#include <string>
#include <iostream>

#include "vk_enum_string_helper.h"
#include "vk_format_utils.h"
#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "vk_layer_logging.h"
#include "vk_typemap_helper.h"

#include "chassis.h"
#include "core_validation.h"
#include "core_error_location.h"
#include "shader_validation.h"
#include "descriptor_sets.h"
#include "buffer_validation.h"
#include "sync_vuid_maps.h"

// All VUID from copy_bufferimage_to_imagebuffer_common.txt
static const char *GetBufferImageCopyCommandVUID(std::string id, bool image_to_buffer, bool copy2) {
    // clang-format off
    static const std::map<std::string, std::array<const char *, 4>> copy_imagebuffer_vuid = {
        {"00193", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-00193",      // !copy2 & !image_to_buffer
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-00193",      // !copy2 &  image_to_buffer
            "VUID-VkCopyBufferToImageInfo2KHR-bufferOffset-00193", //  copy2 & !image_to_buffer
            "VUID-VkCopyImageToBufferInfo2KHR-bufferOffset-00193", //  copy2 &  image_to_buffer
        }},
        {"01558", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-01558",
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-01558",
            "VUID-VkCopyBufferToImageInfo2KHR-bufferOffset-01558",
            "VUID-VkCopyImageToBufferInfo2KHR-bufferOffset-01558",
        }},
        {"01559", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-01559",
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-01559",
            "VUID-VkCopyBufferToImageInfo2KHR-bufferOffset-01559",
            "VUID-VkCopyImageToBufferInfo2KHR-bufferOffset-01559",
        }},
        {"00197", {
            "VUID-vkCmdCopyBufferToImage-imageOffset-00197",
            "VUID-vkCmdCopyImageToBuffer-imageOffset-00197",
            "VUID-VkCopyBufferToImageInfo2KHR-imageOffset-00197",
            "VUID-VkCopyImageToBufferInfo2KHR-imageOffset-00197",
        }},
        {"00198", {
            "VUID-vkCmdCopyBufferToImage-imageOffset-00198",
            "VUID-vkCmdCopyImageToBuffer-imageOffset-00198",
            "VUID-VkCopyBufferToImageInfo2KHR-imageOffset-00198",
            "VUID-VkCopyImageToBufferInfo2KHR-imageOffset-00198",
        }},
        {"00199", {
            "VUID-vkCmdCopyBufferToImage-srcImage-00199",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00199",
            "VUID-VkCopyBufferToImageInfo2KHR-srcImage-00199",
            "VUID-VkCopyImageToBufferInfo2KHR-srcImage-00199",
        }},
        {"00200", {
            "VUID-vkCmdCopyBufferToImage-imageOffset-00200",
            "VUID-vkCmdCopyImageToBuffer-imageOffset-00200",
            "VUID-VkCopyBufferToImageInfo2KHR-imageOffset-00200",
            "VUID-VkCopyImageToBufferInfo2KHR-imageOffset-00200",
        }},
        {"00201", {
            "VUID-vkCmdCopyBufferToImage-srcImage-00201",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00201",
            "VUID-VkCopyBufferToImageInfo2KHR-srcImage-00201",
            "VUID-VkCopyImageToBufferInfo2KHR-srcImage-00201",
        }},
        {"00203", {
            "VUID-vkCmdCopyBufferToImage-bufferRowLength-00203",
            "VUID-vkCmdCopyImageToBuffer-bufferRowLength-00203",
            "VUID-VkCopyBufferToImageInfo2KHR-bufferRowLength-00203",
            "VUID-VkCopyImageToBufferInfo2KHR-bufferRowLength-00203",
        }},
        {"00204", {
            "VUID-vkCmdCopyBufferToImage-bufferImageHeight-00204",
            "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-00204",
            "VUID-VkCopyBufferToImageInfo2KHR-bufferImageHeight-00204",
            "VUID-VkCopyImageToBufferInfo2KHR-bufferImageHeight-00204",
        }},
        {"00205", {
            "VUID-vkCmdCopyBufferToImage-imageOffset-00205",
            "VUID-vkCmdCopyImageToBuffer-imageOffset-00205",
            "VUID-VkCopyBufferToImageInfo2KHR-imageOffset-00205",
            "VUID-VkCopyImageToBufferInfo2KHR-imageOffset-00205",
        }},
        {"00206", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-00206",
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-00206",
            "VUID-VkCopyBufferToImageInfo2KHR-bufferOffset-00206",
            "VUID-VkCopyImageToBufferInfo2KHR-bufferOffset-00206",
        }},
        {"00207", {
            "VUID-vkCmdCopyBufferToImage-imageExtent-00207",
            "VUID-vkCmdCopyImageToBuffer-imageExtent-00207",
            "VUID-VkCopyBufferToImageInfo2KHR-imageExtent-00207",
            "VUID-VkCopyImageToBufferInfo2KHR-imageExtent-00207",
        }},
        {"00208", {
            "VUID-vkCmdCopyBufferToImage-imageExtent-00208",
            "VUID-vkCmdCopyImageToBuffer-imageExtent-00208",
            "VUID-VkCopyBufferToImageInfo2KHR-imageExtent-00208",
            "VUID-VkCopyImageToBufferInfo2KHR-imageExtent-00208",
        }},
        {"00209", {
            "VUID-vkCmdCopyBufferToImage-imageExtent-00209",
            "VUID-vkCmdCopyImageToBuffer-imageExtent-00209",
            "VUID-VkCopyBufferToImageInfo2KHR-imageExtent-00209",
            "VUID-VkCopyImageToBufferInfo2KHR-imageExtent-00209",
        }},
        {"00211", {
            "VUID-vkCmdCopyBufferToImage-aspectMask-00211",
            "VUID-vkCmdCopyImageToBuffer-aspectMask-00211",
            "VUID-VkCopyBufferToImageInfo2KHR-aspectMask-00211",
            "VUID-VkCopyImageToBufferInfo2KHR-aspectMask-00211",
        }},
        {"01560", {
            "VUID-vkCmdCopyBufferToImage-aspectMask-01560",
            "VUID-vkCmdCopyImageToBuffer-aspectMask-01560",
            "VUID-VkCopyBufferToImageInfo2KHR-aspectMask-01560",
            "VUID-VkCopyImageToBufferInfo2KHR-aspectMask-01560",
        }},
        {"00213", {
            "VUID-vkCmdCopyBufferToImage-baseArrayLayer-00213",
            "VUID-vkCmdCopyImageToBuffer-baseArrayLayer-00213",
            "VUID-VkCopyBufferToImageInfo2KHR-baseArrayLayer-00213",
            "VUID-VkCopyImageToBufferInfo2KHR-baseArrayLayer-00213",
        }},
        {"04052", {
            "VUID-vkCmdCopyBufferToImage-commandBuffer-04052",
            "VUID-vkCmdCopyImageToBuffer-commandBuffer-04052",
            "VUID-VkCopyBufferToImageInfo2KHR-commandBuffer-04052",
            "VUID-VkCopyImageToBufferInfo2KHR-commandBuffer-04052",
        }},
        {"04053", {
            "VUID-vkCmdCopyBufferToImage-srcImage-04053",
            "VUID-vkCmdCopyImageToBuffer-srcImage-04053",
            "VUID-VkCopyBufferToImageInfo2KHR-srcImage-04053",
            "VUID-VkCopyImageToBufferInfo2KHR-srcImage-04053",
        }}
    };
    // clang-format on

    uint8_t index = 0;
    index |= (image_to_buffer) ? 0x1 : 0;
    index |= (copy2) ? 0x2 : 0;
    return copy_imagebuffer_vuid.at(id).at(index);
}

// Transfer VkImageSubresourceRange into VkImageSubresourceLayers struct
static VkImageSubresourceLayers LayersFromRange(const VkImageSubresourceRange &subresource_range) {
    VkImageSubresourceLayers subresource_layers;
    subresource_layers.aspectMask = subresource_range.aspectMask;
    subresource_layers.baseArrayLayer = subresource_range.baseArrayLayer;
    subresource_layers.layerCount = subresource_range.layerCount;
    subresource_layers.mipLevel = subresource_range.baseMipLevel;
    return subresource_layers;
}

// Transfer VkImageSubresourceLayers into VkImageSubresourceRange struct
static VkImageSubresourceRange RangeFromLayers(const VkImageSubresourceLayers &subresource_layers) {
    VkImageSubresourceRange subresource_range;
    subresource_range.aspectMask = subresource_layers.aspectMask;
    subresource_range.baseArrayLayer = subresource_layers.baseArrayLayer;
    subresource_range.layerCount = subresource_layers.layerCount;
    subresource_range.baseMipLevel = subresource_layers.mipLevel;
    subresource_range.levelCount = 1;
    return subresource_range;
}

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

IMAGE_STATE::IMAGE_STATE(VkDevice dev, VkImage img, const VkImageCreateInfo *pCreateInfo)
    : image(img),
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
      plane0_memory_requirements_checked(false),
      plane1_memory_requirements_checked(false),
      plane2_memory_requirements_checked(false),
      subresource_encoder(full_range),
      fragment_encoder(nullptr),
      store_device_as_workaround(dev),  // TODO REMOVE WHEN encoder can be const
      sparse_requirements{} {
    if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
        uint32_t *queue_family_indices = new uint32_t[createInfo.queueFamilyIndexCount];
        for (uint32_t i = 0; i < createInfo.queueFamilyIndexCount; i++) {
            queue_family_indices[i] = pCreateInfo->pQueueFamilyIndices[i];
        }
        createInfo.pQueueFamilyIndices = queue_family_indices;
    }

    if (createInfo.flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) {
        sparse = true;
    }

    auto *external_memory_info = LvlFindInChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    if (external_memory_info) {
        external_memory_handle = external_memory_info->handleTypes;
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

bool IMAGE_STATE::IsCompatibleAliasing(IMAGE_STATE *other_image_state) {
    if (!is_swapchain_image && !other_image_state->is_swapchain_image &&
        !(createInfo.flags & other_image_state->createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT)) {
        return false;
    }
    if ((create_from_swapchain == VK_NULL_HANDLE) && binding.mem_state &&
        (binding.mem_state == other_image_state->binding.mem_state) && (binding.offset == other_image_state->binding.offset) &&
        IsCreateInfoEqual(other_image_state->createInfo)) {
        return true;
    }
    if ((bind_swapchain == other_image_state->bind_swapchain) && (bind_swapchain != VK_NULL_HANDLE)) {
        return true;
    }
    return false;
}

IMAGE_VIEW_STATE::IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &im, VkImageView iv, const VkImageViewCreateInfo *ci)
    : image_view(iv),
      create_info(*ci),
      normalized_subresource_range(NormalizeSubresourceRange(*im, ci->subresourceRange)),
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
    }
}

static VkImageLayout NormalizeImageLayout(VkImageLayout layout, VkImageLayout non_normal, VkImageLayout normal) {
    return (layout == non_normal) ? normal : layout;
}

static VkImageLayout NormalizeDepthImageLayout(VkImageLayout layout) {
    return NormalizeImageLayout(layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
}

static VkImageLayout NormalizeStencilImageLayout(VkImageLayout layout) {
    return NormalizeImageLayout(layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
}

static VkImageLayout NormalizeSynchronization2Layout(const VkImageAspectFlags aspect_mask, VkImageLayout layout) {
    if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
        if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        }
    } else if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
        if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
            layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        }
    }
    return layout;
}

static bool ImageLayoutMatches(const VkImageAspectFlags aspect_mask, VkImageLayout a, VkImageLayout b) {
    bool matches = (a == b);
    if (!matches) {
        a = NormalizeSynchronization2Layout(aspect_mask, a);
        b = NormalizeSynchronization2Layout(aspect_mask, b);
        matches = (a == b);
        if (!matches) {
            // Relaxed rules when referencing *only* the depth or stencil aspects
            if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
                matches = NormalizeDepthImageLayout(a) == NormalizeDepthImageLayout(b);
            } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
                matches = NormalizeStencilImageLayout(a) == NormalizeStencilImageLayout(b);
            }
        }
    }
    return matches;
}

// Utility type for ForRange callbacks
struct LayoutUseCheckAndMessage {
    const static VkImageAspectFlags kDepthOrStencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    const ImageSubresourceLayoutMap *layout_map;
    const VkImageAspectFlags aspect_mask;
    const char *message;
    VkImageLayout layout;

    LayoutUseCheckAndMessage() = delete;
    LayoutUseCheckAndMessage(const ImageSubresourceLayoutMap *layout_map_, const VkImageAspectFlags aspect_mask_ = 0)
        : layout_map(layout_map_), aspect_mask{aspect_mask_}, message(nullptr), layout(kInvalidLayout) {}
    bool Check(const VkImageSubresource &subres, VkImageLayout check, VkImageLayout current_layout, VkImageLayout initial_layout) {
        message = nullptr;
        layout = kInvalidLayout;  // Success status
        if (current_layout != kInvalidLayout && !ImageLayoutMatches(aspect_mask, check, current_layout)) {
            message = "previous known";
            layout = current_layout;
        } else if ((initial_layout != kInvalidLayout) && !ImageLayoutMatches(aspect_mask, check, initial_layout)) {
            // To check the relaxed rule matching we need to see how the initial use was used
            const auto initial_layout_state = layout_map->GetSubresourceInitialLayoutState(subres);
            assert(initial_layout_state);  // If we have an initial layout, we better have a state for it
            if (!((initial_layout_state->aspect_mask & kDepthOrStencil) &&
                  ImageLayoutMatches(initial_layout_state->aspect_mask, check, initial_layout))) {
                message = "previously used";
                layout = initial_layout;
            }
        }
        return layout == kInvalidLayout;
    }
};

bool IMAGE_VIEW_STATE::OverlapSubresource(const IMAGE_VIEW_STATE &compare_view) const {
    if (image_view == compare_view.image_view) {
        return true;
    }
    if (image_state->image != compare_view.image_state->image) {
        return false;
    }
    if (normalized_subresource_range.aspectMask != compare_view.normalized_subresource_range.aspectMask) {
        return false;
    }

    // compare if overlap mip level
    if ((normalized_subresource_range.baseMipLevel < compare_view.normalized_subresource_range.baseMipLevel) &&
        ((normalized_subresource_range.baseMipLevel + normalized_subresource_range.levelCount) <=
         compare_view.normalized_subresource_range.baseMipLevel)) {
        return false;
    }

    if ((normalized_subresource_range.baseMipLevel > compare_view.normalized_subresource_range.baseMipLevel) &&
        (normalized_subresource_range.baseMipLevel >=
         (compare_view.normalized_subresource_range.baseMipLevel + compare_view.normalized_subresource_range.levelCount))) {
        return false;
    }

    // compare if overlap array layer
    if ((normalized_subresource_range.baseArrayLayer < compare_view.normalized_subresource_range.baseArrayLayer) &&
        ((normalized_subresource_range.baseArrayLayer + normalized_subresource_range.layerCount) <=
         compare_view.normalized_subresource_range.baseArrayLayer)) {
        return false;
    }

    if ((normalized_subresource_range.baseArrayLayer > compare_view.normalized_subresource_range.baseArrayLayer) &&
        (normalized_subresource_range.baseArrayLayer >=
         (compare_view.normalized_subresource_range.baseArrayLayer + compare_view.normalized_subresource_range.layerCount))) {
        return false;
    }
    return true;
}

uint32_t FullMipChainLevels(uint32_t height, uint32_t width, uint32_t depth) {
    // uint cast applies floor()
    return 1u + static_cast<uint32_t>(log2(std::max({height, width, depth})));
}

uint32_t FullMipChainLevels(VkExtent3D extent) { return FullMipChainLevels(extent.height, extent.width, extent.depth); }

uint32_t FullMipChainLevels(VkExtent2D extent) { return FullMipChainLevels(extent.height, extent.width); }

bool CoreChecks::FindLayouts(VkImage image, std::vector<VkImageLayout> &layouts) const {
    auto image_state = GetImageState(image);
    if (!image_state) return false;

    const auto *layout_range_map = GetLayoutRangeMap(imageLayoutMap, image);
    if (!layout_range_map) return false;
    // TODO: FindLayouts function should mutate into a ValidatePresentableLayout with the loop wrapping the LogError
    //       from the caller. You can then use decode to add the subresource of the range::begin to the error message.

    // TODO: what is this test and what is it supposed to do?! -- the logic doesn't match the comment below?!

    // TODO: Make this robust for >1 aspect mask. Now it will just say ignore potential errors in this case.
    if (layout_range_map->size() >= (image_state->createInfo.arrayLayers * image_state->createInfo.mipLevels + 1)) {
        return false;
    }

    for (auto entry : *layout_range_map) {
        layouts.push_back(entry.second);
    }
    return true;
}

// Set image layout for given VkImageSubresourceRange struct
void CoreChecks::SetImageLayout(CMD_BUFFER_STATE *cb_node, const IMAGE_STATE &image_state,
                                const VkImageSubresourceRange &image_subresource_range, VkImageLayout layout,
                                VkImageLayout expected_layout) {
    auto *subresource_map = GetImageSubresourceLayoutMap(cb_node, image_state);
    assert(subresource_map);  // the non-const getter must return a valid pointer
    if (subresource_map->SetSubresourceRangeLayout(*cb_node, image_subresource_range, layout, expected_layout)) {
        cb_node->image_layout_change_count++;  // Change the version of this data to force revalidation
    }
    for (const auto &image : image_state.aliasing_images) {
        auto alias_state = GetImageState(image);
        // The map state of the aliases should all be in sync, so no need to check the return value
        subresource_map = GetImageSubresourceLayoutMap(cb_node, *alias_state);
        assert(subresource_map);
        subresource_map->SetSubresourceRangeLayout(*cb_node, image_subresource_range, layout, expected_layout);
    }
}

// Set the initial image layout for all slices of an image view
void CoreChecks::SetImageViewInitialLayout(CMD_BUFFER_STATE *cb_node, const IMAGE_VIEW_STATE &view_state, VkImageLayout layout) {
    if (disabled[image_layout_validation]) {
        return;
    }
    IMAGE_STATE *image_state = view_state.image_state.get();
    auto *subresource_map = GetImageSubresourceLayoutMap(cb_node, *image_state);
    subresource_map->SetSubresourceRangeInitialLayout(*cb_node, layout, view_state);
    for (const auto &image : image_state->aliasing_images) {
        image_state = GetImageState(image);
        subresource_map = GetImageSubresourceLayoutMap(cb_node, *image_state);
        subresource_map->SetSubresourceRangeInitialLayout(*cb_node, layout, view_state);
    }
}

// Set the initial image layout for a passed non-normalized subresource range
void CoreChecks::SetImageInitialLayout(CMD_BUFFER_STATE *cb_node, const IMAGE_STATE &image_state,
                                       const VkImageSubresourceRange &range, VkImageLayout layout) {
    auto *subresource_map = GetImageSubresourceLayoutMap(cb_node, image_state);
    assert(subresource_map);
    subresource_map->SetSubresourceRangeInitialLayout(*cb_node, NormalizeSubresourceRange(image_state, range), layout);
    for (const auto &image : image_state.aliasing_images) {
        auto alias_state = GetImageState(image);
        subresource_map = GetImageSubresourceLayoutMap(cb_node, *alias_state);
        assert(subresource_map);
        subresource_map->SetSubresourceRangeInitialLayout(*cb_node, NormalizeSubresourceRange(*alias_state, range), layout);
    }
}

void CoreChecks::SetImageInitialLayout(CMD_BUFFER_STATE *cb_node, VkImage image, const VkImageSubresourceRange &range,
                                       VkImageLayout layout) {
    const IMAGE_STATE *image_state = GetImageState(image);
    if (!image_state) return;
    SetImageInitialLayout(cb_node, *image_state, range, layout);
};

void CoreChecks::SetImageInitialLayout(CMD_BUFFER_STATE *cb_node, const IMAGE_STATE &image_state,
                                       const VkImageSubresourceLayers &layers, VkImageLayout layout) {
    SetImageInitialLayout(cb_node, image_state, RangeFromLayers(layers), layout);
}

// Set image layout for all slices of an image view
void CoreChecks::SetImageViewLayout(CMD_BUFFER_STATE *cb_node, const IMAGE_VIEW_STATE &view_state, VkImageLayout layout,
                                    VkImageLayout layoutStencil) {
    IMAGE_STATE *image_state = view_state.image_state.get();

    VkImageSubresourceRange sub_range = view_state.normalized_subresource_range;
    // When changing the layout of a 3D image subresource via a 2D or 2D_ARRRAY image view, all depth slices of
    // the subresource mip level(s) are transitioned, ignoring any layers restriction in the subresource info.
    if ((image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) && (view_state.create_info.viewType != VK_IMAGE_VIEW_TYPE_3D)) {
        sub_range.baseArrayLayer = 0;
        sub_range.layerCount = image_state->createInfo.extent.depth;
    }

    if (sub_range.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) && layoutStencil != kInvalidLayout) {
        sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        SetImageLayout(cb_node, *image_state, sub_range, layout);
        sub_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        SetImageLayout(cb_node, *image_state, sub_range, layoutStencil);
    } else {
        SetImageLayout(cb_node, *image_state, sub_range, layout);
    }
}

bool CoreChecks::ValidateRenderPassLayoutAgainstFramebufferImageUsage(RenderPassCreateVersion rp_version, VkImageLayout layout,
                                                                      VkImage image, VkImageView image_view,
                                                                      VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                                      uint32_t attachment_index, const char *variable_name) const {
    bool skip = false;
    auto image_state = GetImageState(image);
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *function_name = use_rp2 ? "vkCmdBeginRenderPass2()" : "vkCmdBeginRenderPass()";

    if (!image_state) {
        LogObjectList objlist(image);
        objlist.add(renderpass);
        objlist.add(framebuffer);
        objlist.add(image_view);
        skip |=
            LogError(image, "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                     "%s: RenderPass %s uses %s where pAttachments[%" PRIu32 "] = %s, which refers to an invalid image",
                     function_name, report_data->FormatHandle(renderpass).c_str(), report_data->FormatHandle(framebuffer).c_str(),
                     attachment_index, report_data->FormatHandle(image_view).c_str());
        return skip;
    }

    auto image_usage = image_state->createInfo.usage;
    const auto stencil_usage_info = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_state->createInfo.pNext);
    if (stencil_usage_info) {
        image_usage |= stencil_usage_info->stencilUsage;
    }

    // Check for layouts that mismatch image usages in the framebuffer
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03094" : "VUID-vkCmdBeginRenderPass-initialLayout-00895";
        LogObjectList objlist(image);
        objlist.add(renderpass);
        objlist.add(framebuffer);
        objlist.add(image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %u in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        !(image_usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT))) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03097" : "VUID-vkCmdBeginRenderPass-initialLayout-00897";
        LogObjectList objlist(image);
        objlist.add(renderpass);
        objlist.add(framebuffer);
        objlist.add(image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %u in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT or VK_IMAGE_USAGE_SAMPLED_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03098" : "VUID-vkCmdBeginRenderPass-initialLayout-00898";
        LogObjectList objlist(image);
        objlist.add(renderpass);
        objlist.add(framebuffer);
        objlist.add(image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %u in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03099" : "VUID-vkCmdBeginRenderPass-initialLayout-00899";
        LogObjectList objlist(image);
        objlist.add(renderpass);
        objlist.add(framebuffer);
        objlist.add(image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %u in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_TRANSFER_DST_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (device_extensions.vk_khr_maintenance2) {
        if ((layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
            !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03096" : "VUID-vkCmdBeginRenderPass-initialLayout-01758";
            LogObjectList objlist(image);
            objlist.add(renderpass);
            objlist.add(framebuffer);
            objlist.add(image_view);
            skip |= LogError(objlist, vuid,
                             "%s: Layout/usage mismatch for attachment %u in %s"
                             " - the %s is %s but the image attached to %s via %s"
                             " was not created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",
                             function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                             string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                             report_data->FormatHandle(image_view).c_str());
        }
    } else {
        // The create render pass 2 extension requires maintenance 2 (the previous branch), so no vuid switch needed here.
        if ((layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
            !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            LogObjectList objlist(image);
            objlist.add(renderpass);
            objlist.add(framebuffer);
            objlist.add(image_view);
            skip |= LogError(objlist, "VUID-vkCmdBeginRenderPass-initialLayout-00896",
                             "%s: Layout/usage mismatch for attachment %u in %s"
                             " - the %s is %s but the image attached to %s via %s"
                             " was not created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",
                             function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                             string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                             report_data->FormatHandle(image_view).c_str());
        }
    }
    return skip;
}

bool CoreChecks::VerifyFramebufferAndRenderPassLayouts(RenderPassCreateVersion rp_version, const CMD_BUFFER_STATE *pCB,
                                                       const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const FRAMEBUFFER_STATE *framebuffer_state) const {
    bool skip = false;
    auto const render_pass_info = GetRenderPassState(pRenderPassBegin->renderPass)->createInfo.ptr();
    auto const &framebuffer_info = framebuffer_state->createInfo;
    const VkImageView *attachments = framebuffer_info.pAttachments;

    auto render_pass = GetRenderPassState(pRenderPassBegin->renderPass)->renderPass;
    auto framebuffer = framebuffer_state->framebuffer;

    if (render_pass_info->attachmentCount != framebuffer_info.attachmentCount) {
        skip |= LogError(pCB->commandBuffer, kVUID_Core_DrawState_InvalidRenderpass,
                         "You cannot start a render pass using a framebuffer with a different number of attachments.");
    }

    const auto *attachment_info = LvlFindInChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
    if (((framebuffer_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0) && attachment_info != nullptr) {
        attachments = attachment_info->pAttachments;
    }

    if (attachments != nullptr) {
        const auto *const_p_cb = static_cast<const CMD_BUFFER_STATE *>(pCB);
        for (uint32_t i = 0; i < render_pass_info->attachmentCount; ++i) {
            auto image_view = attachments[i];
            auto view_state = GetImageViewState(image_view);

            if (!view_state) {
                LogObjectList objlist(pRenderPassBegin->renderPass);
                objlist.add(framebuffer_state->framebuffer);
                objlist.add(image_view);
                skip |= LogError(objlist, "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                                 "vkCmdBeginRenderPass(): %s pAttachments[%" PRIu32 "] = %s is not a valid VkImageView handle",
                                 report_data->FormatHandle(framebuffer_state->framebuffer).c_str(), i,
                                 report_data->FormatHandle(image_view).c_str());
                continue;
            }

            const VkImage image = view_state->create_info.image;
            const IMAGE_STATE *image_state = GetImageState(image);

            if (!image_state) {
                LogObjectList objlist(pRenderPassBegin->renderPass);
                objlist.add(framebuffer_state->framebuffer);
                objlist.add(image_view);
                objlist.add(image);
                skip |= LogError(objlist, "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                                 "vkCmdBeginRenderPass(): %s pAttachments[%" PRIu32 "] =  %s references non-extant %s.",
                                 report_data->FormatHandle(framebuffer_state->framebuffer).c_str(), i,
                                 report_data->FormatHandle(image_view).c_str(), report_data->FormatHandle(image).c_str());
                continue;
            }
            auto attachment_initial_layout = render_pass_info->pAttachments[i].initialLayout;
            auto final_layout = render_pass_info->pAttachments[i].finalLayout;

            // Default to expecting stencil in the same layout.
            auto attachment_stencil_initial_layout = attachment_initial_layout;

            // If a separate layout is specified, look for that.
            const auto *attachment_description_stencil_layout =
                LvlFindInChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
            if (attachment_description_stencil_layout) {
                attachment_stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
            }

            // Cast pCB to const because we don't want to create entries that don't exist here (in case the key changes to something
            // in common with the non-const version.)
            const ImageSubresourceLayoutMap *subresource_map = (attachment_initial_layout != VK_IMAGE_LAYOUT_UNDEFINED)
                                                                   ? GetImageSubresourceLayoutMap(const_p_cb, image)
                                                                   : nullptr;

            if (subresource_map) {  // If no layout information for image yet, will be checked at QueueSubmit time
                LayoutUseCheckAndMessage layout_check(subresource_map);
                bool subres_skip = false;
                auto pos = subresource_map->Find(view_state->normalized_subresource_range);
                // IncrementInterval skips over all the subresources that have the same state as we just checked, incrementing to
                // the next "constant value" range
                for (; !(pos.AtEnd()) && !subres_skip; pos.IncrementInterval()) {
                    const VkImageSubresource &subres = pos->subresource;

                    // Allow for differing depth and stencil layouts
                    VkImageLayout check_layout = attachment_initial_layout;
                    if (subres.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) check_layout = attachment_stencil_initial_layout;

                    if (!layout_check.Check(subres, check_layout, pos->current_layout, pos->initial_layout)) {
                        subres_skip |= LogError(
                            device, kVUID_Core_DrawState_InvalidRenderpass,
                            "You cannot start a render pass using attachment %u where the render pass initial layout is %s "
                            "and the %s layout of the attachment is %s. The layouts must match, or the render "
                            "pass initial layout for the attachment must be VK_IMAGE_LAYOUT_UNDEFINED",
                            i, string_VkImageLayout(check_layout), layout_check.message, string_VkImageLayout(layout_check.layout));
                    }
                }

                skip |= subres_skip;
            }

            ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, attachment_initial_layout, image, image_view,
                                                                 framebuffer, render_pass, i, "initial layout");

            ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, final_layout, image, image_view, framebuffer,
                                                                 render_pass, i, "final layout");
        }

        for (uint32_t j = 0; j < render_pass_info->subpassCount; ++j) {
            auto &subpass = render_pass_info->pSubpasses[j];
            for (uint32_t k = 0; k < render_pass_info->pSubpasses[j].inputAttachmentCount; ++k) {
                auto &attachment_ref = subpass.pInputAttachments[k];
                if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                    auto image_view = attachments[attachment_ref.attachment];
                    auto view_state = GetImageViewState(image_view);

                    if (view_state) {
                        auto image = view_state->create_info.image;
                        ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, attachment_ref.layout, image, image_view,
                                                                             framebuffer, render_pass, attachment_ref.attachment,
                                                                             "input attachment layout");
                    }
                }
            }

            for (uint32_t k = 0; k < render_pass_info->pSubpasses[j].colorAttachmentCount; ++k) {
                auto &attachment_ref = subpass.pColorAttachments[k];
                if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                    auto image_view = attachments[attachment_ref.attachment];
                    auto view_state = GetImageViewState(image_view);

                    if (view_state) {
                        auto image = view_state->create_info.image;
                        ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, attachment_ref.layout, image, image_view,
                                                                             framebuffer, render_pass, attachment_ref.attachment,
                                                                             "color attachment layout");
                        if (subpass.pResolveAttachments) {
                            ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                                rp_version, attachment_ref.layout, image, image_view, framebuffer, render_pass,
                                attachment_ref.attachment, "resolve attachment layout");
                        }
                    }
                }
            }

            if (render_pass_info->pSubpasses[j].pDepthStencilAttachment) {
                auto &attachment_ref = *subpass.pDepthStencilAttachment;
                if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                    auto image_view = attachments[attachment_ref.attachment];
                    auto view_state = GetImageViewState(image_view);

                    if (view_state) {
                        auto image = view_state->create_info.image;
                        ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, attachment_ref.layout, image, image_view,
                                                                             framebuffer, render_pass, attachment_ref.attachment,
                                                                             "input attachment layout");
                    }
                }
            }
        }
    }
    return skip;
}

void CoreChecks::TransitionAttachmentRefLayout(CMD_BUFFER_STATE *pCB, FRAMEBUFFER_STATE *pFramebuffer,
                                               const safe_VkAttachmentReference2 &ref) {
    if (ref.attachment != VK_ATTACHMENT_UNUSED) {
        IMAGE_VIEW_STATE *image_view = GetActiveAttachmentImageViewState(pCB, ref.attachment);
        if (image_view) {
            VkImageLayout stencil_layout = kInvalidLayout;
            const auto *attachment_reference_stencil_layout = LvlFindInChain<VkAttachmentReferenceStencilLayout>(ref.pNext);
            if (attachment_reference_stencil_layout) {
                stencil_layout = attachment_reference_stencil_layout->stencilLayout;
            }

            SetImageViewLayout(pCB, *image_view, ref.layout, stencil_layout);
        }
    }
}

void CoreChecks::TransitionSubpassLayouts(CMD_BUFFER_STATE *pCB, const RENDER_PASS_STATE *render_pass_state,
                                          const int subpass_index, FRAMEBUFFER_STATE *framebuffer_state) {
    assert(render_pass_state);

    if (framebuffer_state) {
        auto const &subpass = render_pass_state->createInfo.pSubpasses[subpass_index];
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            TransitionAttachmentRefLayout(pCB, framebuffer_state, subpass.pInputAttachments[j]);
        }
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            TransitionAttachmentRefLayout(pCB, framebuffer_state, subpass.pColorAttachments[j]);
        }
        if (subpass.pDepthStencilAttachment) {
            TransitionAttachmentRefLayout(pCB, framebuffer_state, *subpass.pDepthStencilAttachment);
        }
    }
}

// Transition the layout state for renderpass attachments based on the BeginRenderPass() call. This includes:
// 1. Transition into initialLayout state
// 2. Transition from initialLayout to layout used in subpass 0
void CoreChecks::TransitionBeginRenderPassLayouts(CMD_BUFFER_STATE *cb_state, const RENDER_PASS_STATE *render_pass_state,
                                                  FRAMEBUFFER_STATE *framebuffer_state) {
    // First record expected initialLayout as a potential initial layout usage.
    auto const rpci = render_pass_state->createInfo.ptr();
    for (uint32_t i = 0; i < rpci->attachmentCount; ++i) {
        auto *view_state = GetActiveAttachmentImageViewState(cb_state, i);
        if (view_state) {
            IMAGE_STATE *image_state = view_state->image_state.get();
            const auto initial_layout = rpci->pAttachments[i].initialLayout;
            const auto *attachment_description_stencil_layout =
                LvlFindInChain<VkAttachmentDescriptionStencilLayout>(rpci->pAttachments[i].pNext);
            if (attachment_description_stencil_layout) {
                const auto stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
                VkImageSubresourceRange sub_range = view_state->normalized_subresource_range;
                sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                SetImageInitialLayout(cb_state, *image_state, sub_range, initial_layout);
                sub_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                SetImageInitialLayout(cb_state, *image_state, sub_range, stencil_initial_layout);
            } else {
                SetImageInitialLayout(cb_state, *image_state, view_state->normalized_subresource_range, initial_layout);
            }
        }
    }
    // Now transition for first subpass (index 0)
    TransitionSubpassLayouts(cb_state, render_pass_state, 0, framebuffer_state);
}

bool VerifyAspectsPresent(VkImageAspectFlags aspect_mask, VkFormat format) {
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        if (!(FormatIsColor(format) || FormatIsMultiplane(format))) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
        if (!FormatHasDepth(format)) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
        if (!FormatHasStencil(format)) return false;
    }
    if (0 != (aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
        if (FormatPlaneCount(format) == 1) return false;
    }
    return true;
}

// Verify an ImageMemoryBarrier's old/new ImageLayouts are compatible with the Image's ImageUsageFlags.
bool CoreChecks::ValidateBarrierLayoutToImageUsage(const CoreErrorLocation &loc, VkImage image, VkImageLayout layout,
                                                   VkImageUsageFlags usage_flags) const {
    bool skip = false;
    bool is_error = false;
    switch (layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0);
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
            is_error = ((usage_flags & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        default:
            // Other VkImageLayout values do not have VUs defined in this context.
            break;
    }

    if (is_error) {
        const auto &vuid = sync_vuid_maps::GetBadImageLayoutVUID(loc, layout);

        skip |=
            LogError(image, vuid, "%s Image barrier Layout=%s is not compatible with %s usage flags 0x%" PRIx32 ".",
                     loc.Message().c_str(), string_VkImageLayout(layout), report_data->FormatHandle(image).c_str(), usage_flags);
    }
    return skip;
}

// Verify image barriers are compatible with the images they reference.
template <typename ImageBarrier>
bool CoreChecks::ValidateBarriersToImages(const CoreErrorLocation &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                          uint32_t imageMemoryBarrierCount, const ImageBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;
    // Scoreboard for checking for duplicate and inconsistent barriers to images
    struct ImageBarrierScoreboardEntry {
        uint32_t index;
        // This is designed for temporary storage within the scope of the API call.  If retained storage of the barriers is
        // required, copies should be made and smart or unique pointers used in some other stucture (or this one refactored)
        const ImageBarrier *barrier;
    };
    // Necessary to resolve warning C4503 when building with Visual Studio 2015.
    // Adding a struct wrapper is their recommend solution for the expanded type name growing too long
    // when creating maps full of maps.
    struct ImageBarrierScoreboardSubresMap {
        std::unordered_map<VkImageSubresourceRange, ImageBarrierScoreboardEntry> map;
    };
    using ImageBarrierScoreboardImageMap = std::unordered_map<VkImage, ImageBarrierScoreboardSubresMap>;

    // Scoreboard for duplicate layout transition barriers within the list
    // Pointers retained in the scoreboard only have the lifetime of *this* call (i.e. within the scope of the API call)
    ImageBarrierScoreboardImageMap layout_transitions;

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        auto loc = outer_loc.dot(Field::pImageMemoryBarriers, i);
        const auto &img_barrier = pImageMemoryBarriers[i];

        // Update the scoreboard of layout transitions and check for barriers affecting the same image and subresource
        // TODO: a higher precision could be gained by adapting the command_buffer image_layout_map logic looking for conflicts
        // at a per sub-resource level
        if (img_barrier.oldLayout != img_barrier.newLayout) {
            const ImageBarrierScoreboardEntry new_entry{i, &img_barrier};
            const auto image_it = layout_transitions.find(img_barrier.image);
            if (image_it != layout_transitions.end()) {
                auto &subres_map = image_it->second.map;
                auto subres_it = subres_map.find(img_barrier.subresourceRange);
                if (subres_it != subres_map.end()) {
                    auto &entry = subres_it->second;
                    auto entry_layout =
                        NormalizeSynchronization2Layout(entry.barrier->subresourceRange.aspectMask, entry.barrier->newLayout);
                    auto old_layout =
                        NormalizeSynchronization2Layout(img_barrier.subresourceRange.aspectMask, img_barrier.oldLayout);
                    if ((entry_layout != old_layout) && (old_layout != VK_IMAGE_LAYOUT_UNDEFINED)) {
                        const VkImageSubresourceRange &range = img_barrier.subresourceRange;
                        const auto &vuid = GetImageBarrierVUID(loc, ImageError::kConflictingLayout);
                        skip = LogError(
                            cb_state->commandBuffer, vuid,
                            "%s conflicts with earlier entry pImageMemoryBarrier[%u]. %s"
                            " subresourceRange: aspectMask=%u baseMipLevel=%u levelCount=%u, baseArrayLayer=%u, layerCount=%u; "
                            "conflicting barrier transitions image layout from %s when earlier barrier transitioned to layout %s.",
                            loc.Message().c_str(), entry.index, report_data->FormatHandle(img_barrier.image).c_str(),
                            range.aspectMask, range.baseMipLevel, range.levelCount, range.baseArrayLayer, range.layerCount,
                            string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(entry.barrier->newLayout));
                    }
                    entry = new_entry;
                } else {
                    subres_map[img_barrier.subresourceRange] = new_entry;
                }
            } else {
                layout_transitions[img_barrier.image].map[img_barrier.subresourceRange] = new_entry;
            }
        }

        auto image_state = GetImageState(img_barrier.image);
        if (image_state) {
            VkImageUsageFlags usage_flags = image_state->createInfo.usage;
            skip |=
                ValidateBarrierLayoutToImageUsage(loc.dot(Field::oldLayout), img_barrier.image, img_barrier.oldLayout, usage_flags);
            skip |=
                ValidateBarrierLayoutToImageUsage(loc.dot(Field::newLayout), img_barrier.image, img_barrier.newLayout, usage_flags);

            // Make sure layout is able to be transitioned, currently only presented shared presentable images are locked
            if (image_state->layout_locked) {
                // TODO: Add unique id for error when available
                skip |= LogError(
                    img_barrier.image, 0,
                    "%s Attempting to transition shared presentable %s"
                    " from layout %s to layout %s, but image has already been presented and cannot have its layout transitioned.",
                    loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                    string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
            }

            const VkImageCreateInfo &image_create_info = image_state->createInfo;
            const VkFormat image_format = image_create_info.format;
            const VkImageAspectFlags aspect_mask = img_barrier.subresourceRange.aspectMask;
            // For a Depth/Stencil image both aspects MUST be set
            auto image_loc = loc.dot(Field::image);
            if (FormatIsDepthAndStencil(image_format)) {
                if (enabled_features.core12.separateDepthStencilLayouts) {
                    if (!(aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
                        auto vuid = GetImageBarrierVUID(loc, ImageError::kNotDepthOrStencilAspect);
                        skip |= LogError(img_barrier.image, vuid,
                                         "%s references %s of format %s that must have either the depth or stencil "
                                         "aspects set, but its aspectMask is 0x%" PRIx32 ".",
                                         image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                         string_VkFormat(image_format), aspect_mask);
                    }
                } else {
                    auto const ds_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                    if ((aspect_mask & ds_mask) != (ds_mask)) {
                        auto error = device_extensions.vk_khr_separate_depth_stencil_layouts
                                         ? ImageError::kNotSeparateDepthAndStencilAspect
                                         : ImageError::kNotDepthAndStencilAspect;
                        auto vuid = GetImageBarrierVUID(image_loc, error);
                        skip |= LogError(img_barrier.image, vuid,
                                         "%s references %s of format %s that must have the depth and stencil "
                                         "aspects set, but its aspectMask is 0x%" PRIx32 ".",
                                         image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                         string_VkFormat(image_format), aspect_mask);
                    }
                }
            }

            const auto *subresource_map = GetImageSubresourceLayoutMap(cb_state, img_barrier.image);
            if (img_barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                // TODO: Set memory invalid which is in mem_tracker currently
                // Not sure if this needs to be in the ForRange traversal, pulling it out as it is currently invariant with
                // subresource.
            } else if (subresource_map && !QueueFamilyIsExternal(img_barrier.srcQueueFamilyIndex)) {
                bool subres_skip = false;
                LayoutUseCheckAndMessage layout_check(subresource_map);
                VkImageSubresourceRange normalized_isr = NormalizeSubresourceRange(*image_state, img_barrier.subresourceRange);
                // IncrementInterval skips over all the subresources that have the same state as we just checked, incrementing to
                // the next "constant value" range
                for (auto pos = subresource_map->Find(normalized_isr); !(pos.AtEnd()) && !subres_skip; pos.IncrementInterval()) {
                    const auto &value = *pos;
                    auto old_layout =
                        NormalizeSynchronization2Layout(img_barrier.subresourceRange.aspectMask, img_barrier.oldLayout);
                    if (!layout_check.Check(value.subresource, old_layout, value.current_layout, value.initial_layout)) {
                        const auto &vuid = GetImageBarrierVUID(loc, ImageError::kConflictingLayout);
                        subres_skip = LogError(cb_state->commandBuffer, vuid,
                                               "%s %s cannot transition the layout of aspect=%d level=%d layer=%d from %s when the "
                                               "%s layout is %s.",
                                               loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                               value.subresource.aspectMask, value.subresource.mipLevel,
                                               value.subresource.arrayLayer, string_VkImageLayout(img_barrier.oldLayout),
                                               layout_check.message, string_VkImageLayout(layout_check.layout));
                    }
                }
                skip |= subres_skip;
            }

            // checks color format and (single-plane or non-disjoint)
            // if ycbcr extension is not supported then single-plane and non-disjoint are always both true
            if ((FormatIsColor(image_format) == true) &&
                ((FormatIsMultiplane(image_format) == false) || (image_state->disjoint == false))) {
                if (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT) {
                    auto error = device_extensions.vk_khr_sampler_ycbcr_conversion ? ImageError::kNotColorAspect
                                                                                   : ImageError::kNotColorAspectYcbcr;
                    const auto &vuid = GetImageBarrierVUID(loc, error);
                    skip |= LogError(img_barrier.image, vuid,
                                     "%s references %s of format %s that must be only VK_IMAGE_ASPECT_COLOR_BIT, "
                                     "but its aspectMask is 0x%" PRIx32 ".",
                                     image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                     string_VkFormat(image_format), aspect_mask);
                }
            }

            VkImageAspectFlags valid_disjoint_mask =
                VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
            if ((FormatIsMultiplane(image_format) == true) && (image_state->disjoint == true) &&
                ((aspect_mask & valid_disjoint_mask) == 0)) {
                const auto &vuid = GetImageBarrierVUID(image_loc, ImageError::kBadMultiplanarAspect);
                skip |= LogError(img_barrier.image, vuid,
                                 "%s references %s of format %s has aspectMask (0x%" PRIx32
                                 ") but needs to include either an VK_IMAGE_ASPECT_PLANE_*_BIT or VK_IMAGE_ASPECT_COLOR_BIT.",
                                 image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                 string_VkFormat(image_format), aspect_mask);
            }

            if ((FormatPlaneCount(image_format) == 2) && ((aspect_mask & VK_IMAGE_ASPECT_PLANE_2_BIT) != 0)) {
                const auto &vuid = GetImageBarrierVUID(image_loc, ImageError::kBadPlaneCount);
                skip |= LogError(img_barrier.image, vuid,
                                 "%s references %s of format %s has only two planes but included "
                                 "VK_IMAGE_ASPECT_PLANE_2_BIT in its aspectMask (0x%" PRIx32 ").",
                                 image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                 string_VkFormat(image_format), aspect_mask);
            }
        }
    }
    return skip;
}

template <typename Barrier, typename TransferBarrier>
bool CoreChecks::ValidateQFOTransferBarrierUniqueness(const CoreErrorLocation &loc, const CMD_BUFFER_STATE *cb_state,
                                                      const Barrier &barrier,
                                                      const QFOTransferBarrierSets<TransferBarrier> &barrier_sets) const {
    bool skip = false;
    auto pool = cb_state->command_pool.get();
    const char *handle_name = TransferBarrier::HandleName();
    const char *transfer_type = nullptr;
    if (!IsTransferOp(barrier)) {
        return skip;
    }
    const TransferBarrier *barrier_record = nullptr;
    if (TempIsReleaseOp<Barrier, true /* Assume IsTransfer */>(pool, barrier) &&
        !QueueFamilyIsExternal(barrier.dstQueueFamilyIndex)) {
        const auto found = barrier_sets.release.find(barrier);
        if (found != barrier_sets.release.cend()) {
            barrier_record = &(*found);
            transfer_type = "releasing";
        }
    } else if (IsAcquireOp<Barrier, true /*Assume IsTransfer */>(pool, barrier) &&
               !QueueFamilyIsExternal(barrier.srcQueueFamilyIndex)) {
        const auto found = barrier_sets.acquire.find(barrier);
        if (found != barrier_sets.acquire.cend()) {
            barrier_record = &(*found);
            transfer_type = "acquiring";
        }
    }
    if (barrier_record != nullptr) {
        skip |=
            LogWarning(cb_state->commandBuffer, TransferBarrier::ErrMsgDuplicateQFOInCB(),
                       "%s %s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                       " duplicates existing barrier recorded in this command buffer.",
                       loc.Message().c_str(), transfer_type, handle_name, report_data->FormatHandle(barrier_record->handle).c_str(),
                       barrier_record->srcQueueFamilyIndex, barrier_record->dstQueueFamilyIndex);
    }
    return skip;
}

VulkanTypedHandle BarrierTypedHandle(const VkImageMemoryBarrier &barrier) {
    return VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage);
}

VulkanTypedHandle BarrierTypedHandle(const VkImageMemoryBarrier2KHR &barrier) {
    return VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage);
}

const IMAGE_STATE *BarrierHandleState(const ValidationStateTracker &device_state, const VkImageMemoryBarrier &barrier) {
    return device_state.GetImageState(barrier.image);
}

const IMAGE_STATE *BarrierHandleState(const ValidationStateTracker &device_state, const VkImageMemoryBarrier2KHR &barrier) {
    return device_state.GetImageState(barrier.image);
}

VulkanTypedHandle BarrierTypedHandle(const VkBufferMemoryBarrier &barrier) {
    return VulkanTypedHandle(barrier.buffer, kVulkanObjectTypeBuffer);
}

VulkanTypedHandle BarrierTypedHandle(const VkBufferMemoryBarrier2KHR &barrier) {
    return VulkanTypedHandle(barrier.buffer, kVulkanObjectTypeBuffer);
}

const BUFFER_STATE *BarrierHandleState(const ValidationStateTracker &device_state, const VkBufferMemoryBarrier &barrier) {
    return device_state.GetBufferState(barrier.buffer);
}

const BUFFER_STATE *BarrierHandleState(const ValidationStateTracker &device_state, const VkBufferMemoryBarrier2KHR &barrier) {
    return device_state.GetBufferState(barrier.buffer);
}

template <typename Barrier, typename TransferBarrier>
void CoreChecks::RecordBarrierValidationInfo(const CoreErrorLocation &loc, CMD_BUFFER_STATE *cb_state, const Barrier &barrier,
                                             QFOTransferBarrierSets<TransferBarrier> &barrier_sets) {
    auto pool = cb_state->command_pool.get();
    if (IsTransferOp(barrier)) {
        if (TempIsReleaseOp<Barrier, true /* Assume IsTransfer*/>(pool, barrier) &&
            !QueueFamilyIsExternal(barrier.dstQueueFamilyIndex)) {
            barrier_sets.release.emplace(barrier);
        } else if (IsAcquireOp<Barrier, true /*Assume IsTransfer */>(pool, barrier) &&
                   !QueueFamilyIsExternal(barrier.srcQueueFamilyIndex)) {
            barrier_sets.acquire.emplace(barrier);
        }
    }

    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    if (!QueueFamilyIsIgnored(src_queue_family) && !QueueFamilyIsIgnored(dst_queue_family)) {
        // Only enqueue submit time check if it is needed. If more submit time checks are added, change the criteria
        // TODO create a better named list, or rename the submit time lists to something that matches the broader usage...
        auto handle_state = BarrierHandleState(*this, barrier);
        bool mode_concurrent = handle_state ? handle_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT : false;
        if (!mode_concurrent) {
            const auto typed_handle = BarrierTypedHandle(barrier);
            CoreErrorLocationCapture loc_capture(loc);
            cb_state->queue_submit_functions.emplace_back(
                [loc_capture, cb_state, typed_handle, src_queue_family, dst_queue_family](const ValidationStateTracker *device_data,
                                                                                          const QUEUE_STATE *queue_state) {
                    return ValidateConcurrentBarrierAtSubmit(loc_capture.Get(), device_data, queue_state, cb_state, typed_handle,
                                                             src_queue_family, dst_queue_family);
                });
        }
    }
}

// Verify image barrier image state and that the image is consistent with FB image
template <typename ImgBarrier>
bool CoreChecks::ValidateImageBarrierAttachment(const CoreErrorLocation &loc, CMD_BUFFER_STATE const *cb_state,
                                                const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                const safe_VkSubpassDescription2 &sub_desc, const VkRenderPass rp_handle,
                                                const ImgBarrier &img_barrier, const CMD_BUFFER_STATE *primary_cb_state) const {
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;

    bool skip = false;
    const auto *fb_state = framebuffer;
    assert(fb_state);
    const auto img_bar_image = img_barrier.image;
    bool image_match = false;
    bool sub_image_found = false;  // Do we find a corresponding subpass description
    VkImageLayout sub_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t attach_index = 0;
    // Verify that a framebuffer image matches barrier image
    const auto attachment_count = fb_state->createInfo.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
        auto view_state = GetActiveAttachmentImageViewState(cb_state, attachment, primary_cb_state);
        if (view_state && (img_bar_image == view_state->create_info.image)) {
            image_match = true;
            attach_index = attachment;
            break;
        }
    }
    if (image_match) {  // Make sure subpass is referring to matching attachment
        if (sub_desc.pDepthStencilAttachment && sub_desc.pDepthStencilAttachment->attachment == attach_index) {
            sub_image_layout = sub_desc.pDepthStencilAttachment->layout;
            sub_image_found = true;
        }
        if (!sub_image_found && device_extensions.vk_khr_depth_stencil_resolve) {
            const auto *resolve = LvlFindInChain<VkSubpassDescriptionDepthStencilResolve>(sub_desc.pNext);
            if (resolve && resolve->pDepthStencilResolveAttachment &&
                resolve->pDepthStencilResolveAttachment->attachment == attach_index) {
                sub_image_layout = resolve->pDepthStencilResolveAttachment->layout;
                sub_image_found = true;
            }
        }
        if (!sub_image_found) {
            for (uint32_t j = 0; j < sub_desc.colorAttachmentCount; ++j) {
                if (sub_desc.pColorAttachments && sub_desc.pColorAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pColorAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
                if (!sub_image_found && sub_desc.pResolveAttachments &&
                    sub_desc.pResolveAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pResolveAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
            }
        }
        if (!sub_image_found) {
            auto img_loc = loc.dot(Field::image);
            const auto &vuid = GetImageBarrierVUID(img_loc, ImageError::kRenderPassMismatch);
            skip |=
                LogError(rp_handle, vuid,
                         "%s Barrier for %s is not referenced by the VkSubpassDescription for active subpass (%d) of current %s.",
                         img_loc.Message().c_str(), report_data->FormatHandle(img_bar_image).c_str(), active_subpass,
                         report_data->FormatHandle(rp_handle).c_str());
        }
    } else {  // !image_match
        auto img_loc = loc.dot(Field::image);
        const auto &vuid = GetImageBarrierVUID(img_loc, ImageError::kRenderPassMismatch);
        skip |= LogError(fb_state->framebuffer, vuid, "%s Barrier for %s does not match an image from the current %s.",
                         img_loc.Message().c_str(), report_data->FormatHandle(img_bar_image).c_str(),
                         report_data->FormatHandle(fb_state->framebuffer).c_str());
    }
    if (img_barrier.oldLayout != img_barrier.newLayout) {
        auto layout_loc = loc.dot(Field::oldLayout);
        const auto &vuid = GetImageBarrierVUID(layout_loc, ImageError::kRenderPassLayoutChange);
        skip |= LogError(cb_state->commandBuffer, vuid,
                         "%s As the Image Barrier for %s is being executed within a render pass instance, oldLayout must "
                         "equal newLayout yet they are %s and %s.",
                         layout_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                         string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
    } else {
        if (sub_image_found && sub_image_layout != img_barrier.oldLayout) {
            LogObjectList objlist(rp_handle);
            objlist.add(img_bar_image);
            auto layout_loc = loc.dot(Field::oldLayout);
            const auto &vuid = GetImageBarrierVUID(layout_loc, ImageError::kRenderPassLayoutChange);
            skip |= LogError(objlist, vuid,
                             "%s Barrier for %s is referenced by the VkSubpassDescription for active "
                             "subpass (%d) of current %s as having layout %s, but image barrier has layout %s.",
                             layout_loc.Message().c_str(), report_data->FormatHandle(img_bar_image).c_str(), active_subpass,
                             report_data->FormatHandle(rp_handle).c_str(), string_VkImageLayout(sub_image_layout),
                             string_VkImageLayout(img_barrier.oldLayout));
        }
    }
    return skip;
}
// explictly instantiate so these can be used in core_validation.cpp
template bool CoreChecks::ValidateImageBarrierAttachment(const CoreErrorLocation &loc, CMD_BUFFER_STATE const *cb_state,
                                                         const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                         const safe_VkSubpassDescription2 &sub_desc, const VkRenderPass rp_handle,
                                                         const VkImageMemoryBarrier &img_barrier,
                                                         const CMD_BUFFER_STATE *primary_cb_state) const;
template bool CoreChecks::ValidateImageBarrierAttachment(const CoreErrorLocation &loc, CMD_BUFFER_STATE const *cb_state,
                                                         const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                         const safe_VkSubpassDescription2 &sub_desc, const VkRenderPass rp_handle,
                                                         const VkImageMemoryBarrier2KHR &img_barrier,
                                                         const CMD_BUFFER_STATE *primary_cb_state) const;

template <typename ImgBarrier>
void CoreChecks::EnqueueSubmitTimeValidateImageBarrierAttachment(const CoreErrorLocation &loc, CMD_BUFFER_STATE *cb_state,
                                                                 const ImgBarrier &barrier) {
    // Secondary CBs can have null framebuffer so queue up validation in that case 'til FB is known
    if ((cb_state->activeRenderPass) && (VK_NULL_HANDLE == cb_state->activeFramebuffer) &&
        (VK_COMMAND_BUFFER_LEVEL_SECONDARY == cb_state->createInfo.level)) {
        const auto active_subpass = cb_state->activeSubpass;
        const auto rp_state = cb_state->activeRenderPass;
        const auto &sub_desc = rp_state->createInfo.pSubpasses[active_subpass];
        // Secondary CB case w/o FB specified delay validation
        auto *this_ptr = this;  // Required for older compilers with c++20 compatibility
        CoreErrorLocationCapture loc_capture(loc);
        const auto render_pass = rp_state->renderPass;
        cb_state->cmd_execute_commands_functions.emplace_back(
            [this_ptr, loc_capture, cb_state, active_subpass, sub_desc, render_pass, barrier](const CMD_BUFFER_STATE *primary_cb,
                                                                                              const FRAMEBUFFER_STATE *fb) {
                return this_ptr->ValidateImageBarrierAttachment(loc_capture.Get(), cb_state, fb, active_subpass, sub_desc,
                                                                render_pass, barrier, primary_cb);
            });
    }
}

void CoreChecks::RecordBarriers(ErrFunc func_name, CMD_BUFFER_STATE *cb_state, uint32_t bufferBarrierCount,
                                const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                const VkImageMemoryBarrier *pImageMemBarriers) {
    for (uint32_t i = 0; i < bufferBarrierCount; i++) {
        CoreErrorLocation loc(func_name, RefPage::VkBufferMemoryBarrier, Field::pBufferMemoryBarriers, i);
        RecordBarrierValidationInfo(loc, cb_state, pBufferMemBarriers[i], cb_state->qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; i++) {
        CoreErrorLocation loc(func_name, RefPage::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        const auto &img_barrier = pImageMemBarriers[i];
        RecordBarrierValidationInfo(loc, cb_state, img_barrier, cb_state->qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(loc, cb_state, img_barrier);
    }
}

void CoreChecks::RecordBarriers(ErrFunc func_name, CMD_BUFFER_STATE *cb_state, const VkDependencyInfoKHR &dep_info) {
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        CoreErrorLocation loc(func_name, RefPage::VkBufferMemoryBarrier2KHR, Field::pBufferMemoryBarriers, i);
        RecordBarrierValidationInfo(loc, cb_state, dep_info.pBufferMemoryBarriers[i], cb_state->qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        CoreErrorLocation loc(func_name, RefPage::VkImageMemoryBarrier2KHR, Field::pImageMemoryBarriers, i);
        const auto &img_barrier = dep_info.pImageMemoryBarriers[i];
        RecordBarrierValidationInfo(loc, cb_state, img_barrier, cb_state->qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(loc, cb_state, img_barrier);
    }
}

template <typename TransferBarrier, typename Scoreboard>
bool CoreChecks::ValidateAndUpdateQFOScoreboard(const debug_report_data *report_data, const CMD_BUFFER_STATE *cb_state,
                                                const char *operation, const TransferBarrier &barrier,
                                                Scoreboard *scoreboard) const {
    // Record to the scoreboard or report that we have a duplication
    bool skip = false;
    auto inserted = scoreboard->insert(std::make_pair(barrier, cb_state));
    if (!inserted.second && inserted.first->second != cb_state) {
        // This is a duplication (but don't report duplicates from the same CB, as we do that at record time
        LogObjectList objlist(cb_state->commandBuffer);
        objlist.add(barrier.handle);
        objlist.add(inserted.first->second->commandBuffer);
        skip = LogWarning(objlist, TransferBarrier::ErrMsgDuplicateQFOInSubmit(),
                          "%s: %s %s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                          " to dstQueueFamilyIndex %" PRIu32 " duplicates existing barrier submitted in this batch from %s.",
                          "vkQueueSubmit()", TransferBarrier::BarrierName(), operation, TransferBarrier::HandleName(),
                          report_data->FormatHandle(barrier.handle).c_str(), barrier.srcQueueFamilyIndex,
                          barrier.dstQueueFamilyIndex, report_data->FormatHandle(inserted.first->second->commandBuffer).c_str());
    }
    return skip;
}

template <typename TransferBarrier>
bool CoreChecks::ValidateQueuedQFOTransferBarriers(
    const CMD_BUFFER_STATE *cb_state, QFOTransferCBScoreboards<TransferBarrier> *scoreboards,
    const GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers) const {
    bool skip = false;
    const auto &cb_barriers = GetQFOBarrierSets(cb_state, TransferBarrier());
    const char *barrier_name = TransferBarrier::BarrierName();
    const char *handle_name = TransferBarrier::HandleName();
    // No release should have an extant duplicate (WARNING)
    for (const auto &release : cb_barriers.release) {
        // Check the global pending release barriers
        const auto set_it = global_release_barriers.find(release.handle);
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            const auto found = set_for_handle.find(release);
            if (found != set_for_handle.cend()) {
                skip |= LogWarning(cb_state->commandBuffer, TransferBarrier::ErrMsgDuplicateQFOSubmitted(),
                                   "%s: %s releasing queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                                   " to dstQueueFamilyIndex %" PRIu32
                                   " duplicates existing barrier queued for execution, without intervening acquire operation.",
                                   "vkQueueSubmit()", barrier_name, handle_name, report_data->FormatHandle(found->handle).c_str(),
                                   found->srcQueueFamilyIndex, found->dstQueueFamilyIndex);
            }
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "releasing", release, &scoreboards->release);
    }
    // Each acquire must have a matching release (ERROR)
    for (const auto &acquire : cb_barriers.acquire) {
        const auto set_it = global_release_barriers.find(acquire.handle);
        bool matching_release_found = false;
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            matching_release_found = set_for_handle.find(acquire) != set_for_handle.cend();
        }
        if (!matching_release_found) {
            skip |= LogError(cb_state->commandBuffer, TransferBarrier::ErrMsgMissingQFOReleaseInSubmit(),
                             "%s: in submitted command buffer %s acquiring ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                             " to dstQueueFamilyIndex %" PRIu32 " has no matching release barrier queued for execution.",
                             "vkQueueSubmit()", barrier_name, handle_name, report_data->FormatHandle(acquire.handle).c_str(),
                             acquire.srcQueueFamilyIndex, acquire.dstQueueFamilyIndex);
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "acquiring", acquire, &scoreboards->acquire);
    }
    return skip;
}

bool CoreChecks::ValidateQueuedQFOTransfers(const CMD_BUFFER_STATE *cb_state,
                                            QFOTransferCBScoreboards<QFOImageTransferBarrier> *qfo_image_scoreboards,
                                            QFOTransferCBScoreboards<QFOBufferTransferBarrier> *qfo_buffer_scoreboards) const {
    bool skip = false;
    skip |=
        ValidateQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state, qfo_image_scoreboards, qfo_release_image_barrier_map);
    skip |= ValidateQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state, qfo_buffer_scoreboards,
                                                                        qfo_release_buffer_barrier_map);
    return skip;
}

template <typename TransferBarrier>
void RecordQueuedQFOTransferBarriers(QFOTransferBarrierSets<TransferBarrier> &cb_barriers,
                                     GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers) {
    // Add release barriers from this submit to the global map
    for (const auto &release : cb_barriers.release) {
        // the global barrier list is mapped by resource handle to allow cleanup on resource destruction
        // NOTE: We're using [] because creation of a Set is a needed side effect for new handles
        global_release_barriers[release.handle].insert(release);
    }

    // Erase acquired barriers from this submit from the global map -- essentially marking releases as consumed
    for (const auto &acquire : cb_barriers.acquire) {
        // NOTE: We're not using [] because we don't want to create entries for missing releases
        auto set_it = global_release_barriers.find(acquire.handle);
        if (set_it != global_release_barriers.end()) {
            QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            set_for_handle.erase(acquire);
            if (set_for_handle.size() == 0) {  // Clean up empty sets
                global_release_barriers.erase(set_it);
            }
        }
    }
}

void CoreChecks::RecordQueuedQFOTransfers(CMD_BUFFER_STATE *cb_state) {
    RecordQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state->qfo_transfer_image_barriers, qfo_release_image_barrier_map);
    RecordQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state->qfo_transfer_buffer_barriers,
                                                              qfo_release_buffer_barrier_map);
}

template <typename ImgBarrier>
void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count, const ImgBarrier *barriers) {
    // For ownership transfers, the barrier is specified twice; as a release
    // operation on the yielding queue family, and as an acquire operation
    // on the acquiring queue family. This barrier may also include a layout
    // transition, which occurs 'between' the two operations. For validation
    // purposes it doesn't seem important which side performs the layout
    // transition, but it must not be performed twice. We'll arbitrarily
    // choose to perform it as part of the acquire operation.
    //
    // However, we still need to record initial layout for the "initial layout" validation
    for (uint32_t i = 0; i < barrier_count; i++) {
        const auto &mem_barrier = barriers[i];
        const bool is_release_op = IsReleaseOp(cb_state, mem_barrier);
        auto *image_state = GetImageState(mem_barrier.image);
        if (image_state) {
            RecordTransitionImageLayout(cb_state, image_state, mem_barrier, is_release_op);
        }
    }
}
// explictly instantiate this template so it can be used in core_validation.cpp
template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                 const VkImageMemoryBarrier *barrier);
template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                 const VkImageMemoryBarrier2KHR *barrier);

VkImageLayout NormalizeSynchronization2Layout(const VkImageAspectFlags aspect_mask, VkImageLayout layout);

template <typename ImgBarrier>
void CoreChecks::RecordTransitionImageLayout(CMD_BUFFER_STATE *cb_state, const IMAGE_STATE *image_state,
                                             const ImgBarrier &mem_barrier, bool is_release_op) {
    if (enabled_features.synchronization2_features.synchronization2) {
        if (mem_barrier.oldLayout == mem_barrier.newLayout) {
            return;
        }
    }
    VkImageSubresourceRange normalized_isr = NormalizeSubresourceRange(*image_state, mem_barrier.subresourceRange);
    const auto &image_create_info = image_state->createInfo;

    // Special case for 3D images with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT flag bit, where <extent.depth> and
    // <arrayLayers> can potentially alias.  When recording layout for the entire image, pre-emptively record layouts
    // for all (potential) layer sub_resources.
    if (0 != (image_create_info.flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT)) {
        normalized_isr.baseArrayLayer = 0;
        normalized_isr.layerCount = image_create_info.extent.depth;  // Treat each depth slice as a layer subresource
    }

    VkImageLayout initial_layout = NormalizeSynchronization2Layout(mem_barrier.subresourceRange.aspectMask, mem_barrier.oldLayout);
    VkImageLayout new_layout = NormalizeSynchronization2Layout(mem_barrier.subresourceRange.aspectMask, mem_barrier.newLayout);

    // Layout transitions in external instance are not tracked, so don't validate initial layout.
    if (QueueFamilyIsExternal(mem_barrier.srcQueueFamilyIndex)) {
        initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (is_release_op) {
        SetImageInitialLayout(cb_state, *image_state, normalized_isr, initial_layout);
    } else {
        SetImageLayout(cb_state, *image_state, normalized_isr, new_layout, initial_layout);
    }
}

bool CoreChecks::VerifyImageLayout(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *image_state,
                                   const VkImageSubresourceRange &range, VkImageAspectFlags aspect_mask,
                                   VkImageLayout explicit_layout, VkImageLayout optimal_layout, const char *caller,
                                   const char *layout_invalid_msg_code, const char *layout_mismatch_msg_code, bool *error) const {
    if (disabled[image_layout_validation]) return false;
    assert(cb_node);
    assert(image_state);
    const auto image = image_state->image;
    bool skip = false;

    const auto *subresource_map = GetImageSubresourceLayoutMap(cb_node, image);
    if (subresource_map) {
        bool subres_skip = false;
        LayoutUseCheckAndMessage layout_check(subresource_map, aspect_mask);
        // IncrementInterval skips over all the subresources that have the same state as we just checked, incrementing to
        // the next "constant value" range
        for (auto pos = subresource_map->Find(range); !(pos.AtEnd()) && !subres_skip; pos.IncrementInterval()) {
            if (!layout_check.Check(pos->subresource, explicit_layout, pos->current_layout, pos->initial_layout)) {
                *error = true;
                subres_skip |= LogError(cb_node->commandBuffer, layout_mismatch_msg_code,
                                        "%s: Cannot use %s (layer=%u mip=%u) with specific layout %s that doesn't match the "
                                        "%s layout %s.",
                                        caller, report_data->FormatHandle(image).c_str(), pos->subresource.arrayLayer,
                                        pos->subresource.mipLevel, string_VkImageLayout(explicit_layout), layout_check.message,
                                        string_VkImageLayout(layout_check.layout));
            }
        }
        skip |= subres_skip;
    }

    // If optimal_layout is not UNDEFINED, check that layout matches optimal for this case
    if ((VK_IMAGE_LAYOUT_UNDEFINED != optimal_layout) && (explicit_layout != optimal_layout)) {
        if (VK_IMAGE_LAYOUT_GENERAL == explicit_layout) {
            if (image_state->createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
                // LAYOUT_GENERAL is allowed, but may not be performance optimal, flag as perf warning.
                skip |= LogPerformanceWarning(cb_node->commandBuffer, kVUID_Core_DrawState_InvalidImageLayout,
                                              "%s: For optimal performance %s layout should be %s instead of GENERAL.", caller,
                                              report_data->FormatHandle(image).c_str(), string_VkImageLayout(optimal_layout));
            }
        } else if (device_extensions.vk_khr_shared_presentable_image) {
            if (image_state->shared_presentable) {
                if (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != explicit_layout) {
                    skip |=
                        LogError(device, layout_invalid_msg_code,
                                 "%s: Layout for shared presentable image is %s but must be VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR.",
                                 caller, string_VkImageLayout(optimal_layout));
                }
            }
        } else {
            *error = true;
            skip |= LogError(cb_node->commandBuffer, layout_invalid_msg_code,
                             "%s: Layout for %s is %s but can only be %s or VK_IMAGE_LAYOUT_GENERAL.", caller,
                             report_data->FormatHandle(image).c_str(), string_VkImageLayout(explicit_layout),
                             string_VkImageLayout(optimal_layout));
        }
    }
    return skip;
}
bool CoreChecks::VerifyImageLayout(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *image_state,
                                   const VkImageSubresourceLayers &subLayers, VkImageLayout explicit_layout,
                                   VkImageLayout optimal_layout, const char *caller, const char *layout_invalid_msg_code,
                                   const char *layout_mismatch_msg_code, bool *error) const {
    return VerifyImageLayout(cb_node, image_state, RangeFromLayers(subLayers), explicit_layout, optimal_layout, caller,
                             layout_invalid_msg_code, layout_mismatch_msg_code, error);
}

void CoreChecks::TransitionFinalSubpassLayouts(CMD_BUFFER_STATE *pCB, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               FRAMEBUFFER_STATE *framebuffer_state) {
    auto render_pass = GetRenderPassState(pRenderPassBegin->renderPass);
    if (!render_pass) return;

    const VkRenderPassCreateInfo2 *render_pass_info = render_pass->createInfo.ptr();
    if (framebuffer_state) {
        for (uint32_t i = 0; i < render_pass_info->attachmentCount; ++i) {
            auto *view_state = GetActiveAttachmentImageViewState(pCB, i);
            if (view_state) {
                VkImageLayout stencil_layout = kInvalidLayout;
                const auto *attachment_description_stencil_layout =
                    LvlFindInChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
                if (attachment_description_stencil_layout) {
                    stencil_layout = attachment_description_stencil_layout->stencilFinalLayout;
                }
                SetImageViewLayout(pCB, *view_state, render_pass_info->pAttachments[i].finalLayout, stencil_layout);
            }
        }
    }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only with VK_USE_PLATFORM_ANDROID_KHR
// This could also move into a seperate core_validation_android.cpp file... ?

//
// AHB-specific validation within non-AHB APIs
//
bool CoreChecks::ValidateCreateImageANDROID(const debug_report_data *report_data, const VkImageCreateInfo *create_info) const {
    bool skip = false;

    const VkExternalFormatANDROID *ext_fmt_android = LvlFindInChain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android) {
        if (0 != ext_fmt_android->externalFormat) {
            if (VK_FORMAT_UNDEFINED != create_info->format) {
                skip |=
                    LogError(device, "VUID-VkImageCreateInfo-pNext-01974",
                             "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with non-zero "
                             "externalFormat, but the VkImageCreateInfo's format is not VK_FORMAT_UNDEFINED.");
            }

            if (0 != (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT & create_info->flags)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02396",
                                 "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                 "non-zero externalFormat, but flags include VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }

            if (0 != (~VK_IMAGE_USAGE_SAMPLED_BIT & create_info->usage)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02397",
                                 "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                 "non-zero externalFormat, but usage includes bits (0x%" PRIx64
                                 ") other than VK_IMAGE_USAGE_SAMPLED_BIT.",
                                 create_info->usage);
            }

            if (VK_IMAGE_TILING_OPTIMAL != create_info->tiling) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02398",
                                 "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                 "non-zero externalFormat, but layout is not VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        if ((0 != ext_fmt_android->externalFormat) &&
            (ahb_ext_formats_map.find(ext_fmt_android->externalFormat) == ahb_ext_formats_map.end())) {
            skip |= LogError(device, "VUID-VkExternalFormatANDROID-externalFormat-01894",
                             "vkCreateImage(): Chained VkExternalFormatANDROID struct contains a non-zero externalFormat (%" PRIu64
                             ") which has "
                             "not been previously retrieved by vkGetAndroidHardwareBufferPropertiesANDROID().",
                             ext_fmt_android->externalFormat);
        }
    }

    if ((nullptr == ext_fmt_android) || (0 == ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED == create_info->format) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-pNext-01975",
                         "vkCreateImage(): VkImageCreateInfo struct's format is VK_FORMAT_UNDEFINED, but either does not have a "
                         "chained VkExternalFormatANDROID struct or the struct exists but has an externalFormat of 0.");
        }
    }

    const VkExternalMemoryImageCreateInfo *emici = LvlFindInChain<VkExternalMemoryImageCreateInfo>(create_info->pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        if (create_info->imageType != VK_IMAGE_TYPE_2D) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-pNext-02393",
                         "vkCreateImage(): VkImageCreateInfo struct with imageType %s has chained VkExternalMemoryImageCreateInfo "
                         "struct with handleType VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                         string_VkImageType(create_info->imageType));
        }

        if ((create_info->mipLevels != 1) && (create_info->mipLevels != FullMipChainLevels(create_info->extent))) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-pNext-02394",
                             "vkCreateImage(): VkImageCreateInfo struct with chained VkExternalMemoryImageCreateInfo struct of "
                             "handleType VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID "
                             "specifies mipLevels = %" PRId32 " (full chain mipLevels are %" PRId32 ").",
                             create_info->mipLevels, FullMipChainLevels(create_info->extent));
        }
    }

    return skip;
}

bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo *create_info) const {
    bool skip = false;
    const IMAGE_STATE *image_state = GetImageState(create_info->image);

    if (image_state->has_ahb_format) {
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            skip |= LogError(create_info->image, "VUID-VkImageViewCreateInfo-image-02399",
                             "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                             "format member is %s and must be VK_FORMAT_UNDEFINED.",
                             string_VkFormat(create_info->format));
        }

        // Chain must include a compatible ycbcr conversion
        bool conv_found = false;
        uint64_t external_format = 0;
        const VkSamplerYcbcrConversionInfo *ycbcr_conv_info = LvlFindInChain<VkSamplerYcbcrConversionInfo>(create_info->pNext);
        if (ycbcr_conv_info != nullptr) {
            VkSamplerYcbcrConversion conv_handle = ycbcr_conv_info->conversion;
            if (ycbcr_conversion_ahb_fmt_map.find(conv_handle) != ycbcr_conversion_ahb_fmt_map.end()) {
                conv_found = true;
                external_format = ycbcr_conversion_ahb_fmt_map.at(conv_handle);
            }
        }
        if ((!conv_found) || (external_format != image_state->ahb_format)) {
            skip |= LogError(create_info->image, "VUID-VkImageViewCreateInfo-image-02400",
                             "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                             "an externalFormat (%" PRIu64
                             ") but needs a chained VkSamplerYcbcrConversionInfo struct with a VkSamplerYcbcrConversion created "
                             "with the same external format.",
                             image_state->ahb_format);
        }

        // Errors in create_info swizzles
        if (IsIdentitySwizzle(create_info->components) == false) {
            skip |= LogError(
                create_info->image, "VUID-VkImageViewCreateInfo-image-02401",
                "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                "includes one or more non-identity component swizzles, r swizzle = %s, g swizzle = %s, b swizzle = %s, a swizzle "
                "= %s.",
                string_VkComponentSwizzle(create_info->components.r), string_VkComponentSwizzle(create_info->components.g),
                string_VkComponentSwizzle(create_info->components.b), string_VkComponentSwizzle(create_info->components.a));
        }
    }

    return skip;
}

bool CoreChecks::ValidateGetImageSubresourceLayoutANDROID(const VkImage image) const {
    bool skip = false;

    const IMAGE_STATE *image_state = GetImageState(image);
    if (image_state != nullptr) {
        if (image_state->external_ahb && (0 == image_state->GetBoundMemory().size())) {
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-image-01895",
                             "vkGetImageSubresourceLayout(): Attempt to query layout from an image created with "
                             "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType which has not yet been "
                             "bound to memory.");
        }
    }
    return skip;
}

#else

bool CoreChecks::ValidateCreateImageANDROID(const debug_report_data *report_data, const VkImageCreateInfo *create_info) const {
    return false;
}

bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo *create_info) const { return false; }

bool CoreChecks::ValidateGetImageSubresourceLayoutANDROID(const VkImage image) const { return false; }

#endif  // VK_USE_PLATFORM_ANDROID_KHR

bool CoreChecks::ValidateImageFormatFeatures(const VkImageCreateInfo *pCreateInfo) const {
    bool skip = false;

    // validates based on imageCreateFormatFeatures from vkspec.html#resources-image-creation-limits
    VkFormatFeatureFlags tiling_features = VK_FORMAT_FEATURE_FLAG_BITS_MAX_ENUM;
    const VkImageTiling image_tiling = pCreateInfo->tiling;
    const VkFormat image_format = pCreateInfo->format;

    if (image_format == VK_FORMAT_UNDEFINED) {
        // VU 01975 states format can't be undefined unless an android externalFormat
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        const VkExternalFormatANDROID *ext_fmt_android = LvlFindInChain<VkExternalFormatANDROID>(pCreateInfo->pNext);
        if ((image_tiling == VK_IMAGE_TILING_OPTIMAL) && (ext_fmt_android != nullptr) && (0 != ext_fmt_android->externalFormat)) {
            auto it = ahb_ext_formats_map.find(ext_fmt_android->externalFormat);
            if (it != ahb_ext_formats_map.end()) {
                tiling_features = it->second;
            }
        }
#endif
    } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        uint64_t drm_format_modifier = 0;
        const VkImageDrmFormatModifierExplicitCreateInfoEXT *drm_explicit =
            LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        const VkImageDrmFormatModifierListCreateInfoEXT *drm_implicit =
            LvlFindInChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);

        if (drm_explicit != nullptr) {
            drm_format_modifier = drm_explicit->drmFormatModifier;
        } else {
            // VUID 02261 makes sure its only explict or implict in parameter checking
            assert(drm_implicit != nullptr);
            for (uint32_t i = 0; i < drm_implicit->drmFormatModifierCount; i++) {
                drm_format_modifier |= drm_implicit->pDrmFormatModifiers[i];
            }
        }

        VkFormatProperties2 format_properties_2 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, nullptr};
        VkDrmFormatModifierPropertiesListEXT drm_properties_list = {VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT,
                                                                    nullptr};
        format_properties_2.pNext = (void *)&drm_properties_list;
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, image_format, &format_properties_2);
        std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
        drm_properties.resize(drm_properties_list.drmFormatModifierCount);
        drm_properties_list.pDrmFormatModifierProperties = &drm_properties[0];
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, image_format, &format_properties_2);

        for (uint32_t i = 0; i < drm_properties_list.drmFormatModifierCount; i++) {
            if ((drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifier & drm_format_modifier) != 0) {
                tiling_features |= drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
            }
        }
    } else {
        VkFormatProperties format_properties = GetPDFormatProperties(image_format);
        tiling_features = (image_tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures
                                                                   : format_properties.optimalTilingFeatures;
    }

    // Lack of disjoint format feature support while using the flag
    if (FormatIsMultiplane(image_format) && ((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) &&
        ((tiling_features & VK_FORMAT_FEATURE_DISJOINT_BIT) == 0)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-imageCreateFormatFeatures-02260",
                         "vkCreateImage(): can't use VK_IMAGE_CREATE_DISJOINT_BIT because %s doesn't support "
                         "VK_FORMAT_FEATURE_DISJOINT_BIT based on imageCreateFormatFeatures.",
                         string_VkFormat(pCreateInfo->format));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkImage *pImage) const {
    bool skip = false;

    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateCreateImageANDROID(report_data, pCreateInfo);
    } else {  // These checks are omitted or replaced when Android HW Buffer extension is active
        if (pCreateInfo->format == VK_FORMAT_UNDEFINED) {
            return LogError(device, "VUID-VkImageCreateInfo-format-00943",
                            "vkCreateImage(): VkFormat for image must not be VK_FORMAT_UNDEFINED.");
        }
    }

    if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
        if (VK_IMAGE_TYPE_2D != pCreateInfo->imageType) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-00949",
                             "vkCreateImage(): Image type must be VK_IMAGE_TYPE_2D when VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT "
                             "flag bit is set");
        }

        if ((pCreateInfo->extent.width != pCreateInfo->extent.height) || (pCreateInfo->arrayLayers < 6)) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageType-00954",
                             "vkCreateImage(): If VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT flag bit is set, width (%d) must equal "
                             "height (%d) and arrayLayers (%d) must be >= 6.",
                             pCreateInfo->extent.width, pCreateInfo->extent.height, pCreateInfo->arrayLayers);
        }
    }

    const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
    VkImageUsageFlags attach_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    if ((pCreateInfo->usage & attach_flags) && (pCreateInfo->extent.width > device_limits->maxFramebufferWidth)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00964",
                         "vkCreateImage(): Image usage flags include a frame buffer attachment bit and image width (%u) exceeds "
                         "device maxFramebufferWidth (%u).",
                         pCreateInfo->extent.width, device_limits->maxFramebufferWidth);
    }

    if ((pCreateInfo->usage & attach_flags) && (pCreateInfo->extent.height > device_limits->maxFramebufferHeight)) {
        skip |= LogError(device, "VUID-VkImageCreateInfo-usage-00965",
                         "vkCreateImage(): Image usage flags include a frame buffer attachment bit and image height (%u) exceeds "
                         "device maxFramebufferHeight (%u).",
                         pCreateInfo->extent.height, device_limits->maxFramebufferHeight);
    }

    if (device_extensions.vk_ext_fragment_density_map || device_extensions.vk_ext_fragment_density_map_2) {
        uint32_t ceiling_width = static_cast<uint32_t>(ceil(
            static_cast<float>(device_limits->maxFramebufferWidth) /
            std::max(static_cast<float>(phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.width), 1.0f)));
        if ((pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) && (pCreateInfo->extent.width > ceiling_width)) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-usage-02559",
                         "vkCreateImage(): Image usage flags include a fragment density map bit and image width (%u) exceeds the "
                         "ceiling of device "
                         "maxFramebufferWidth (%u) / minFragmentDensityTexelSize.width (%u). The ceiling value: %u",
                         pCreateInfo->extent.width, device_limits->maxFramebufferWidth,
                         phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.width, ceiling_width);
        }

        uint32_t ceiling_height = static_cast<uint32_t>(ceil(
            static_cast<float>(device_limits->maxFramebufferHeight) /
            std::max(static_cast<float>(phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.height), 1.0f)));
        if ((pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) && (pCreateInfo->extent.height > ceiling_height)) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-usage-02560",
                         "vkCreateImage(): Image usage flags include a fragment density map bit and image height (%u) exceeds the "
                         "ceiling of device "
                         "maxFramebufferHeight (%u) / minFragmentDensityTexelSize.height (%u). The ceiling value: %u",
                         pCreateInfo->extent.height, device_limits->maxFramebufferHeight,
                         phys_dev_ext_props.fragment_density_map_props.minFragmentDensityTexelSize.height, ceiling_height);
        }
    }

    VkImageFormatProperties format_limits = {};
    VkResult result = VK_SUCCESS;
    if (pCreateInfo->tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        result = DispatchGetPhysicalDeviceImageFormatProperties(physical_device, pCreateInfo->format, pCreateInfo->imageType,
                                                                pCreateInfo->tiling, pCreateInfo->usage, pCreateInfo->flags,
                                                                &format_limits);
    } else {
        auto modifier_list = LvlFindInChain<VkImageDrmFormatModifierListCreateInfoEXT>(pCreateInfo->pNext);
        auto explicit_modifier = LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(pCreateInfo->pNext);
        if (modifier_list) {
            for (uint32_t i = 0; i < modifier_list->drmFormatModifierCount; i++) {
                auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
                drm_format_modifier.drmFormatModifier = modifier_list->pDrmFormatModifiers[i];
                auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
                image_format_info.type = pCreateInfo->imageType;
                image_format_info.format = pCreateInfo->format;
                image_format_info.tiling = pCreateInfo->tiling;
                image_format_info.usage = pCreateInfo->usage;
                image_format_info.flags = pCreateInfo->flags;
                auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>();

                result =
                    DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
                format_limits = image_format_properties.imageFormatProperties;

                /* The application gives a list of modifier and the driver
                 * selects one. If one is wrong, stop there.
                 */
                if (result != VK_SUCCESS) break;
            }
        } else if (explicit_modifier) {
            auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
            drm_format_modifier.drmFormatModifier = explicit_modifier->drmFormatModifier;
            auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
            image_format_info.type = pCreateInfo->imageType;
            image_format_info.format = pCreateInfo->format;
            image_format_info.tiling = pCreateInfo->tiling;
            image_format_info.usage = pCreateInfo->usage;
            image_format_info.flags = pCreateInfo->flags;
            auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>();

            result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
            format_limits = image_format_properties.imageFormatProperties;
        }
    }

    // 1. vkGetPhysicalDeviceImageFormatProperties[2] only success code is VK_SUCCESS
    // 2. If call returns an error, then "imageCreateImageFormatPropertiesList" is defined to be the empty list
    // 3. All values in 02251 are undefined if "imageCreateImageFormatPropertiesList" is empty.
    if (result != VK_SUCCESS) {
        // External memory will always have a "imageCreateImageFormatPropertiesList" so skip
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        if (!LvlFindInChain<VkExternalFormatANDROID>(pCreateInfo->pNext)) {
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251",
                             "vkCreateImage(): Format %s is not supported for this combination of parameters and "
                             "VkGetPhysicalDeviceImageFormatProperties returned back %s.",
                             string_VkFormat(pCreateInfo->format), string_VkResult(result));
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    } else {
        if (pCreateInfo->mipLevels > format_limits.maxMipLevels) {
            const char *format_string = string_VkFormat(pCreateInfo->format);
            skip |= LogError(device, "VUID-VkImageCreateInfo-mipLevels-02255",
                             "vkCreateImage(): Image mip levels=%d exceed image format maxMipLevels=%d for format %s.",
                             pCreateInfo->mipLevels, format_limits.maxMipLevels, format_string);
        }

        uint64_t texel_count = static_cast<uint64_t>(pCreateInfo->extent.width) *
                               static_cast<uint64_t>(pCreateInfo->extent.height) *
                               static_cast<uint64_t>(pCreateInfo->extent.depth) * static_cast<uint64_t>(pCreateInfo->arrayLayers) *
                               static_cast<uint64_t>(pCreateInfo->samples);
        uint64_t total_size =
            static_cast<uint64_t>(std::ceil(FormatTexelSize(pCreateInfo->format) * static_cast<double>(texel_count)));

        // Round up to imageGranularity boundary
        VkDeviceSize image_granularity = phys_dev_props.limits.bufferImageGranularity;
        uint64_t ig_mask = image_granularity - 1;
        total_size = (total_size + ig_mask) & ~ig_mask;

        if (total_size > format_limits.maxResourceSize) {
            skip |= LogWarning(device, kVUID_Core_Image_InvalidFormatLimitsViolation,
                               "vkCreateImage(): resource size exceeds allowable maximum Image resource size = 0x%" PRIxLEAST64
                               ", maximum resource size = 0x%" PRIxLEAST64 " ",
                               total_size, format_limits.maxResourceSize);
        }

        if (pCreateInfo->arrayLayers > format_limits.maxArrayLayers) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-arrayLayers-02256",
                             "vkCreateImage(): arrayLayers=%d exceeds allowable maximum supported by format of %d.",
                             pCreateInfo->arrayLayers, format_limits.maxArrayLayers);
        }

        if ((pCreateInfo->samples & format_limits.sampleCounts) == 0) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-samples-02258",
                             "vkCreateImage(): samples %s is not supported by format 0x%.8X.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples), format_limits.sampleCounts);
        }

        if (pCreateInfo->extent.width > format_limits.maxExtent.width) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-extent-02252",
                             "vkCreateImage(): extent.width %u exceeds allowable maximum image extent width %u.",
                             pCreateInfo->extent.width, format_limits.maxExtent.width);
        }

        if (pCreateInfo->extent.height > format_limits.maxExtent.height) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-extent-02253",
                             "vkCreateImage(): extent.height %u exceeds allowable maximum image extent height %u.",
                             pCreateInfo->extent.height, format_limits.maxExtent.height);
        }

        if (pCreateInfo->extent.depth > format_limits.maxExtent.depth) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-extent-02254",
                             "vkCreateImage(): extent.depth %u exceeds allowable maximum image extent depth %u.",
                             pCreateInfo->extent.depth, format_limits.maxExtent.depth);
        }
    }

    // Tests for "Formats requiring sampler YCBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views"
    if (FormatRequiresYcbcrConversion(pCreateInfo->format)) {
        if (!enabled_features.ycbcr_image_array_features.ycbcrImageArrays && pCreateInfo->arrayLayers != 1) {
            const char *error_vuid = (device_extensions.vk_ext_ycbcr_image_arrays) ? "VUID-VkImageCreateInfo-format-02653"
                                                                                   : "VUID-VkImageCreateInfo-format-02564";
            skip |= LogError(device, error_vuid,
                             "vkCreateImage(): arrayLayers = %d, but when the ycbcrImagesArrays feature is not enabled and using a "
                             "YCbCr Conversion format, arrayLayers must be 1",
                             pCreateInfo->arrayLayers);
        }

        if (pCreateInfo->mipLevels != 1) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-format-02561",
                             "vkCreateImage(): mipLevels = %d, but when using a YCbCr Conversion format, mipLevels must be 1",
                             pCreateInfo->arrayLayers);
        }

        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-format-02562",
                "vkCreateImage(): samples = %s, but when using a YCbCr Conversion format, samples must be VK_SAMPLE_COUNT_1_BIT",
                string_VkSampleCountFlagBits(pCreateInfo->samples));
        }

        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError(
                device, "VUID-VkImageCreateInfo-format-02563",
                "vkCreateImage(): imageType = %s, but when using a YCbCr Conversion format, imageType must be VK_IMAGE_TYPE_2D ",
                string_VkImageType(pCreateInfo->imageType));
        }
    }

    if (device_extensions.vk_khr_maintenance2) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
            if (!(FormatIsCompressed_BC(pCreateInfo->format) || FormatIsCompressed_ASTC(pCreateInfo->format) ||
                  FormatIsCompressed_ETC2_EAC(pCreateInfo->format))) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01572",
                                 "vkCreateImage(): If pCreateInfo->flags contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "
                                 "format must be block, ETC or ASTC compressed, but is %s",
                                 string_VkFormat(pCreateInfo->format));
            }
            if (!(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01573",
                                 "vkCreateImage(): If pCreateInfo->flags contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "
                                 "flags must also contain VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }
        }
    }

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    "vkCreateImage", "pCreateInfo->pQueueFamilyIndices",
                                                    "VUID-VkImageCreateInfo-sharingMode-01420");
    }

    if (!FormatIsMultiplane(pCreateInfo->format) && !(pCreateInfo->flags & VK_IMAGE_CREATE_ALIAS_BIT) &&
        (pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT)) {
        skip |=
            LogError(device, "VUID-VkImageCreateInfo-format-01577",
                     "vkCreateImage(): format is %s and flags are %s. The flags should not include VK_IMAGE_CREATE_DISJOINT_BIT.",
                     string_VkFormat(pCreateInfo->format), string_VkImageCreateFlags(pCreateInfo->flags).c_str());
    }

    const auto swapchain_create_info = LvlFindInChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    if (swapchain_create_info != nullptr) {
        if (swapchain_create_info->swapchain != VK_NULL_HANDLE) {
            const SWAPCHAIN_NODE *swapchain_state = GetSwapchainState(swapchain_create_info->swapchain);
            const VkSwapchainCreateFlagsKHR swapchain_flags = swapchain_state->createInfo.flags;

            // Validate rest of Swapchain Image create check that require swapchain state
            const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) == 0)) {
                skip |= LogError(
                    device, vuid,
                    "vkCreateImage(): Swapchain was created with VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR flag so "
                    "all swapchain images must have the VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT flag set.");
            }
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0)) {
                skip |= LogError(device, vuid,
                                 "vkCreateImage(): Swapchain was created with VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR flag so all "
                                 "swapchain images must have the VK_IMAGE_CREATE_PROTECTED_BIT flag set.");
            }
            const VkImageCreateFlags mutable_flags = (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
            if (((swapchain_flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) &&
                ((pCreateInfo->flags & mutable_flags) != mutable_flags)) {
                skip |= LogError(device, vuid,
                                 "vkCreateImage(): Swapchain was created with VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR flag so "
                                 "all swapchain images must have the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT and "
                                 "VK_IMAGE_CREATE_EXTENDED_USAGE_BIT flags both set.");
            }
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.core11.protectedMemory == VK_FALSE) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-flags-01890",
                             "vkCreateImage(): the protectedMemory device feature is disabled: Images cannot be created with the "
                             "VK_IMAGE_CREATE_PROTECTED_BIT set.");
        }
        const VkImageCreateFlags invalid_flags =
            VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
        if ((pCreateInfo->flags & invalid_flags) != 0) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-None-01891",
                             "vkCreateImage(): VK_IMAGE_CREATE_PROTECTED_BIT is set so no sparse create flags can be used at same "
                             "time (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | "
                             "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT).");
        }
    }

    skip |= ValidateImageFormatFeatures(pCreateInfo);

    // Check compatibility with VK_KHR_portability_subset
    if (ExtEnabled::kNotEnabled != device_extensions.vk_khr_portability_subset) {
        if (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT & pCreateInfo->flags &&
            VK_FALSE == enabled_features.portability_subset_features.imageView2DOn3DImage) {
            skip |= LogError(device, "VUID-VkImageCreateInfo-imageView2DOn3DImage-04459",
                             "vkCreateImage (portability error): VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT is not supported.");
        }
        if ((VK_SAMPLE_COUNT_1_BIT != pCreateInfo->samples) && (1 != pCreateInfo->arrayLayers) &&
            (VK_FALSE == enabled_features.portability_subset_features.multisampleArrayImage)) {
            skip |=
                LogError(device, "VUID-VkImageCreateInfo-multisampleArrayImage-04460",
                         "vkCreateImage (portability error): Cannot create an image with samples/texel > 1 && arrayLayers != 1");
        }
    }

    return skip;
}

void CoreChecks::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage, VkResult result) {
    if (VK_SUCCESS != result) return;

    StateTracker::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    auto image_state = Get<IMAGE_STATE>(*pImage);
    AddInitialLayoutintoImageLayoutMap(*image_state, imageLayoutMap);
}

bool CoreChecks::PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) const {
    const IMAGE_STATE *image_state = GetImageState(image);
    const VulkanTypedHandle obj_struct(image, kVulkanObjectTypeImage);
    bool skip = false;
    if (image_state) {
        skip |= ValidateObjectNotInUse(image_state, obj_struct, "vkDestroyImage", "VUID-vkDestroyImage-image-01000");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    // Clean up validation specific data
    qfo_release_image_barrier_map.erase(image);

    imageLayoutMap.erase(image);

    // Clean up generic image state
    StateTracker::PreCallRecordDestroyImage(device, image, pAllocator);
}

bool CoreChecks::ValidateImageAttributes(const IMAGE_STATE *image_state, const VkImageSubresourceRange &range,
                                         const char *param_name) const {
    bool skip = false;
    const VkImage image = image_state->image;
    const VkFormat format = image_state->createInfo.format;

    if (range.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) {
        skip |= LogError(image, "VUID-vkCmdClearColorImage-aspectMask-02498",
                         "vkCmdClearColorImage(): %s.aspectMasks must only be set to VK_IMAGE_ASPECT_COLOR_BIT.", param_name);
    }

    if (FormatIsDepthOrStencil(format)) {
        skip |= LogError(image, "VUID-vkCmdClearColorImage-image-00007",
                         "vkCmdClearColorImage(): %s called with image %s which has a depth/stencil format (%s).", param_name,
                         report_data->FormatHandle(image).c_str(), string_VkFormat(format));
    } else if (FormatIsCompressed(format)) {
        skip |= LogError(image, "VUID-vkCmdClearColorImage-image-00007",
                         "vkCmdClearColorImage(): %s called with image %s which has a compressed format (%s).", param_name,
                         report_data->FormatHandle(image).c_str(), string_VkFormat(format));
    }

    if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        skip |=
            LogError(image, "VUID-vkCmdClearColorImage-image-00002",
                     "vkCmdClearColorImage() %s called with image %s which was created without VK_IMAGE_USAGE_TRANSFER_DST_BIT.",
                     param_name, report_data->FormatHandle(image).c_str());
    }
    return skip;
}

bool CoreChecks::VerifyClearImageLayout(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *image_state,
                                        const VkImageSubresourceRange &range, VkImageLayout dest_image_layout,
                                        const char *func_name) const {
    bool skip = false;
    if (strcmp(func_name, "vkCmdClearDepthStencilImage()") == 0) {
        if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL)) {
            skip |= LogError(image_state->image, "VUID-vkCmdClearDepthStencilImage-imageLayout-00012",
                             "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL or GENERAL.", func_name,
                             string_VkImageLayout(dest_image_layout));
        }

    } else {
        assert(strcmp(func_name, "vkCmdClearColorImage()") == 0);
        if (!device_extensions.vk_khr_shared_presentable_image) {
            if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL)) {
                skip |= LogError(image_state->image, "VUID-vkCmdClearColorImage-imageLayout-00005",
                                 "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL or GENERAL.", func_name,
                                 string_VkImageLayout(dest_image_layout));
            }
        } else {
            if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL) &&
                (dest_image_layout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)) {
                skip |= LogError(
                    image_state->image, "VUID-vkCmdClearColorImage-imageLayout-01394",
                    "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL, SHARED_PRESENT_KHR, or GENERAL.",
                    func_name, string_VkImageLayout(dest_image_layout));
            }
        }
    }

    // Cast to const to prevent creation at validate time.
    const auto *subresource_map = GetImageSubresourceLayoutMap(cb_node, image_state->image);
    if (subresource_map) {
        bool subres_skip = false;
        LayoutUseCheckAndMessage layout_check(subresource_map);
        VkImageSubresourceRange normalized_isr = NormalizeSubresourceRange(*image_state, range);
        // IncrementInterval skips over all the subresources that have the same state as we just checked, incrementing to
        // the next "constant value" range
        for (auto pos = subresource_map->Find(normalized_isr); !(pos.AtEnd()) && !subres_skip; pos.IncrementInterval()) {
            if (!layout_check.Check(pos->subresource, dest_image_layout, pos->current_layout, pos->initial_layout)) {
                const char *error_code = "VUID-vkCmdClearColorImage-imageLayout-00004";
                if (strcmp(func_name, "vkCmdClearDepthStencilImage()") == 0) {
                    error_code = "VUID-vkCmdClearDepthStencilImage-imageLayout-00011";
                } else {
                    assert(strcmp(func_name, "vkCmdClearColorImage()") == 0);
                }
                subres_skip |= LogError(cb_node->commandBuffer, error_code,
                                        "%s: Cannot clear an image whose layout is %s and doesn't match the %s layout %s.",
                                        func_name, string_VkImageLayout(dest_image_layout), layout_check.message,
                                        string_VkImageLayout(layout_check.layout));
            }
        }
        skip |= subres_skip;
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                   const VkClearColorValue *pColor, uint32_t rangeCount,
                                                   const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    const auto *cb_node = GetCBState(commandBuffer);
    const auto *image_state = GetImageState(image);
    if (cb_node && image_state) {
        skip |= ValidateMemoryIsBoundToImage(image_state, "vkCmdClearColorImage()", "VUID-vkCmdClearColorImage-image-00003");
        skip |= ValidateCmdQueueFlags(cb_node, "vkCmdClearColorImage()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdClearColorImage-commandBuffer-cmdpool");
        skip |= ValidateCmd(cb_node, CMD_CLEARCOLORIMAGE, "vkCmdClearColorImage()");
        if (device_extensions.vk_khr_maintenance1) {
            skip |= ValidateImageFormatFeatureFlags(image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, "vkCmdClearColorImage",
                                                    "VUID-vkCmdClearColorImage-image-01993");
        }
        skip |= InsideRenderPass(cb_node, "vkCmdClearColorImage()", "VUID-vkCmdClearColorImage-renderpass");
        skip |=
            ValidateProtectedImage(cb_node, image_state, "vkCmdClearColorImage()", "VUID-vkCmdClearColorImage-commandBuffer-01805");
        skip |= ValidateUnprotectedImage(cb_node, image_state, "vkCmdClearColorImage()",
                                         "VUID-vkCmdClearColorImage-commandBuffer-01806");
        for (uint32_t i = 0; i < rangeCount; ++i) {
            std::string param_name = "pRanges[" + std::to_string(i) + "]";
            skip |= ValidateCmdClearColorSubresourceRange(image_state, pRanges[i], param_name.c_str());
            skip |= ValidateImageAttributes(image_state, pRanges[i], param_name.c_str());
            skip |= VerifyClearImageLayout(cb_node, image_state, pRanges[i], imageLayout, "vkCmdClearColorImage()");
        }
        // Tests for "Formats requiring sampler Y’CBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views"
        if (FormatRequiresYcbcrConversion(image_state->createInfo.format)) {
            skip |= LogError(device, "VUID-vkCmdClearColorImage-image-01545",
                             "vkCmdClearColorImage(): format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             string_VkFormat(image_state->createInfo.format));
        }
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                 const VkClearColorValue *pColor, uint32_t rangeCount,
                                                 const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);

    auto cb_node = GetCBState(commandBuffer);
    auto image_state = GetImageState(image);
    if (cb_node && image_state) {
        for (uint32_t i = 0; i < rangeCount; ++i) {
            SetImageInitialLayout(cb_node, image, pRanges[i], imageLayout);
        }
    }
}

bool CoreChecks::ValidateClearDepthStencilValue(VkCommandBuffer commandBuffer, VkClearDepthStencilValue clearValue,
                                                const char *apiName) const {
    bool skip = false;

    // The extension was not created with a feature bit whichs prevents displaying the 2 variations of the VUIDs
    if (!device_extensions.vk_ext_depth_range_unrestricted) {
        if (!(clearValue.depth >= 0.0) || !(clearValue.depth <= 1.0)) {
            // Also VUID-VkClearDepthStencilValue-depth-00022
            skip |= LogError(commandBuffer, "VUID-VkClearDepthStencilValue-depth-02506",
                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and VkClearDepthStencilValue::depth "
                             "(=%f) is not within the [0.0, 1.0] range.",
                             apiName, clearValue.depth);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                          const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                          const VkImageSubresourceRange *pRanges) const {
    bool skip = false;

    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    const auto *cb_node = GetCBState(commandBuffer);
    const auto *image_state = GetImageState(image);
    if (cb_node && image_state) {
        const VkFormat image_format = image_state->createInfo.format;
        skip |= ValidateMemoryIsBoundToImage(image_state, "vkCmdClearDepthStencilImage()",
                                             "VUID-vkCmdClearDepthStencilImage-image-00010");
        skip |= ValidateCmdQueueFlags(cb_node, "vkCmdClearDepthStencilImage()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdClearDepthStencilImage-commandBuffer-cmdpool");
        skip |= ValidateCmd(cb_node, CMD_CLEARDEPTHSTENCILIMAGE, "vkCmdClearDepthStencilImage()");
        if (device_extensions.vk_khr_maintenance1) {
            skip |= ValidateImageFormatFeatureFlags(image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, "vkCmdClearDepthStencilImage",
                                                    "VUID-vkCmdClearDepthStencilImage-image-01994");
        }
        skip |= ValidateClearDepthStencilValue(commandBuffer, *pDepthStencil, "vkCmdClearDepthStencilImage()");
        skip |= InsideRenderPass(cb_node, "vkCmdClearDepthStencilImage()", "VUID-vkCmdClearDepthStencilImage-renderpass");
        skip |= ValidateProtectedImage(cb_node, image_state, "vkCmdClearDepthStencilImage()",
                                       "VUID-vkCmdClearDepthStencilImage-commandBuffer-01807");
        skip |= ValidateUnprotectedImage(cb_node, image_state, "vkCmdClearDepthStencilImage()",
                                         "VUID-vkCmdClearDepthStencilImage-commandBuffer-01808");

        bool any_include_aspect_depth_bit = false;
        bool any_include_aspect_stencil_bit = false;

        for (uint32_t i = 0; i < rangeCount; ++i) {
            std::string param_name = "pRanges[" + std::to_string(i) + "]";
            skip |= ValidateCmdClearDepthSubresourceRange(image_state, pRanges[i], param_name.c_str());
            skip |= VerifyClearImageLayout(cb_node, image_state, pRanges[i], imageLayout, "vkCmdClearDepthStencilImage()");
            // Image aspect must be depth or stencil or both
            VkImageAspectFlags valid_aspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if (((pRanges[i].aspectMask & valid_aspects) == 0) || ((pRanges[i].aspectMask & ~valid_aspects) != 0)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdClearDepthStencilImage-aspectMask-02824",
                                 "vkCmdClearDepthStencilImage(): pRanges[%u].aspectMask can only be VK_IMAGE_ASPECT_DEPTH_BIT "
                                 "and/or VK_IMAGE_ASPECT_STENCIL_BIT.",
                                 i);
            }
            if ((pRanges[i].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
                any_include_aspect_depth_bit = true;
                if (FormatHasDepth(image_format) == false) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdClearDepthStencilImage-image-02826",
                                     "vkCmdClearDepthStencilImage(): pRanges[%u].aspectMask has a VK_IMAGE_ASPECT_DEPTH_BIT but %s "
                                     "doesn't have a depth component.",
                                     i, string_VkFormat(image_format));
                }
            }
            if ((pRanges[i].aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
                any_include_aspect_stencil_bit = true;
                if (FormatHasStencil(image_format) == false) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdClearDepthStencilImage-image-02825",
                                     "vkCmdClearDepthStencilImage(): pRanges[%u].aspectMask has a VK_IMAGE_ASPECT_STENCIL_BIT but "
                                     "%s doesn't have a stencil component.",
                                     i, string_VkFormat(image_format));
                }
            }
        }
        if (any_include_aspect_stencil_bit) {
            const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_state->createInfo.pNext);
            if (image_stencil_struct != nullptr) {
                if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                    skip |=
                        LogError(device, "VUID-vkCmdClearDepthStencilImage-pRanges-02658",
                                 "vkCmdClearDepthStencilImage(): an element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT "
                                 "and image was created with separate stencil usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT must be "
                                 "included in VkImageStencilUsageCreateInfo::stencilUsage used to create image");
                }
            } else {
                if ((image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                    skip |= LogError(
                        device, "VUID-vkCmdClearDepthStencilImage-pRanges-02659",
                        "vkCmdClearDepthStencilImage(): an element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT and "
                        "image was not created with separate stencil usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT must be included "
                        "in VkImageCreateInfo::usage used to create image");
                }
            }
        }
        if (any_include_aspect_depth_bit && (image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
            skip |= LogError(device, "VUID-vkCmdClearDepthStencilImage-pRanges-02660",
                             "vkCmdClearDepthStencilImage(): an element of pRanges.aspect includes VK_IMAGE_ASPECT_DEPTH_BIT, "
                             "VK_IMAGE_USAGE_TRANSFER_DST_BIT must be included in VkImageCreateInfo::usage used to create image");
        }
        if (image_state && !FormatIsDepthOrStencil(image_format)) {
            skip |= LogError(image, "VUID-vkCmdClearDepthStencilImage-image-00014",
                             "vkCmdClearDepthStencilImage(): called with image %s which doesn't have a depth/stencil format (%s).",
                             report_data->FormatHandle(image).c_str(), string_VkFormat(image_format));
        }
        if (VK_IMAGE_USAGE_TRANSFER_DST_BIT != (VK_IMAGE_USAGE_TRANSFER_DST_BIT & image_state->createInfo.usage)) {
            skip |= LogError(image, "VUID-vkCmdClearDepthStencilImage-image-00009",
                             "vkCmdClearDepthStencilImage(): called with image %s which was not created with the "
                             "VK_IMAGE_USAGE_TRANSFER_DST_BIT set.",
                             report_data->FormatHandle(image).c_str());
        }
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                        const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                        const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    auto cb_node = GetCBState(commandBuffer);
    auto image_state = GetImageState(image);
    if (cb_node && image_state) {
        for (uint32_t i = 0; i < rangeCount; ++i) {
            SetImageInitialLayout(cb_node, image, pRanges[i], imageLayout);
        }
    }
}

// Returns true if [x, xoffset] and [y, yoffset] overlap
static bool RangesIntersect(int32_t start, uint32_t start_offset, int32_t end, uint32_t end_offset) {
    bool result = false;
    uint32_t intersection_min = std::max(static_cast<uint32_t>(start), static_cast<uint32_t>(end));
    uint32_t intersection_max = std::min(static_cast<uint32_t>(start) + start_offset, static_cast<uint32_t>(end) + end_offset);

    if (intersection_max > intersection_min) {
        result = true;
    }
    return result;
}

// Returns true if source area of first vkImageCopy/vkImageCopy2KHR region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
template <typename RegionType>
static bool RegionIntersects(const RegionType *rgn0, const RegionType *rgn1, VkImageType type, bool is_multiplane) {
    bool result = false;

    // Separate planes within a multiplane image cannot intersect
    if (is_multiplane && (rgn0->srcSubresource.aspectMask != rgn1->dstSubresource.aspectMask)) {
        return result;
    }

    if ((rgn0->srcSubresource.mipLevel == rgn1->dstSubresource.mipLevel) &&
        (RangesIntersect(rgn0->srcSubresource.baseArrayLayer, rgn0->srcSubresource.layerCount, rgn1->dstSubresource.baseArrayLayer,
                         rgn1->dstSubresource.layerCount))) {
        result = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                result &= RangesIntersect(rgn0->srcOffset.z, rgn0->extent.depth, rgn1->dstOffset.z, rgn1->extent.depth);
                // fall through
            case VK_IMAGE_TYPE_2D:
                result &= RangesIntersect(rgn0->srcOffset.y, rgn0->extent.height, rgn1->dstOffset.y, rgn1->extent.height);
                // fall through
            case VK_IMAGE_TYPE_1D:
                result &= RangesIntersect(rgn0->srcOffset.x, rgn0->extent.width, rgn1->dstOffset.x, rgn1->extent.width);
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
}

// Returns non-zero if offset and extent exceed image extents
static const uint32_t kXBit = 1;
static const uint32_t kYBit = 2;
static const uint32_t kZBit = 4;
static uint32_t ExceedsBounds(const VkOffset3D *offset, const VkExtent3D *extent, const VkExtent3D *image_extent) {
    uint32_t result = 0;
    // Extents/depths cannot be negative but checks left in for clarity
    if ((offset->z + extent->depth > image_extent->depth) || (offset->z < 0) ||
        ((offset->z + static_cast<int32_t>(extent->depth)) < 0)) {
        result |= kZBit;
    }
    if ((offset->y + extent->height > image_extent->height) || (offset->y < 0) ||
        ((offset->y + static_cast<int32_t>(extent->height)) < 0)) {
        result |= kYBit;
    }
    if ((offset->x + extent->width > image_extent->width) || (offset->x < 0) ||
        ((offset->x + static_cast<int32_t>(extent->width)) < 0)) {
        result |= kXBit;
    }
    return result;
}

// Test if two VkExtent3D structs are equivalent
static inline bool IsExtentEqual(const VkExtent3D *extent, const VkExtent3D *other_extent) {
    bool result = true;
    if ((extent->width != other_extent->width) || (extent->height != other_extent->height) ||
        (extent->depth != other_extent->depth)) {
        result = false;
    }
    return result;
}

// Test if the extent argument has all dimensions set to 0.
static inline bool IsExtentAllZeroes(const VkExtent3D *extent) {
    return ((extent->width == 0) && (extent->height == 0) && (extent->depth == 0));
}

// Returns the image transfer granularity for a specific image scaled by compressed block size if necessary.
VkExtent3D CoreChecks::GetScaledItg(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *img) const {
    // Default to (0, 0, 0) granularity in case we can't find the real granularity for the physical device.
    VkExtent3D granularity = {0, 0, 0};
    auto pool = cb_node->command_pool.get();
    if (pool) {
        granularity = GetPhysicalDeviceState()->queue_family_properties[pool->queueFamilyIndex].minImageTransferGranularity;
        if (FormatIsCompressed(img->createInfo.format) || FormatIsSinglePlane_422(img->createInfo.format)) {
            auto block_size = FormatTexelBlockExtent(img->createInfo.format);
            granularity.width *= block_size.width;
            granularity.height *= block_size.height;
        }
    }
    return granularity;
}

// Test elements of a VkExtent3D structure against alignment constraints contained in another VkExtent3D structure
static inline bool IsExtentAligned(const VkExtent3D *extent, const VkExtent3D *granularity) {
    bool valid = true;
    if ((SafeModulo(extent->depth, granularity->depth) != 0) || (SafeModulo(extent->width, granularity->width) != 0) ||
        (SafeModulo(extent->height, granularity->height) != 0)) {
        valid = false;
    }
    return valid;
}

// Check elements of a VkOffset3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::CheckItgOffset(const CMD_BUFFER_STATE *cb_node, const VkOffset3D *offset, const VkExtent3D *granularity,
                                const uint32_t i, const char *function, const char *member, const char *vuid) const {
    bool skip = false;
    VkExtent3D offset_extent = {};
    offset_extent.width = static_cast<uint32_t>(abs(offset->x));
    offset_extent.height = static_cast<uint32_t>(abs(offset->y));
    offset_extent.depth = static_cast<uint32_t>(abs(offset->z));
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the offset must always be (0, 0, 0)
        if (IsExtentAllZeroes(&offset_extent) == false) {
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: pRegion[%d].%s (x=%d, y=%d, z=%d) must be (x=0, y=0, z=0) when the command buffer's queue family "
                             "image transfer granularity is (w=0, h=0, d=0).",
                             function, i, member, offset->x, offset->y, offset->z);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the offset dimensions must always be even
        // integer multiples of the image transfer granularity.
        if (IsExtentAligned(&offset_extent, granularity) == false) {
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: pRegion[%d].%s (x=%d, y=%d, z=%d) dimensions must be even integer multiples of this command "
                             "buffer's queue family image transfer granularity (w=%d, h=%d, d=%d).",
                             function, i, member, offset->x, offset->y, offset->z, granularity->width, granularity->height,
                             granularity->depth);
        }
    }
    return skip;
}

// Check elements of a VkExtent3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::CheckItgExtent(const CMD_BUFFER_STATE *cb_node, const VkExtent3D *extent, const VkOffset3D *offset,
                                const VkExtent3D *granularity, const VkExtent3D *subresource_extent, const VkImageType image_type,
                                const uint32_t i, const char *function, const char *member, const char *vuid) const {
    bool skip = false;
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the extent must always match the image
        // subresource extent.
        if (IsExtentEqual(extent, subresource_extent) == false) {
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: pRegion[%d].%s (w=%d, h=%d, d=%d) must match the image subresource extents (w=%d, h=%d, d=%d) "
                             "when the command buffer's queue family image transfer granularity is (w=0, h=0, d=0).",
                             function, i, member, extent->width, extent->height, extent->depth, subresource_extent->width,
                             subresource_extent->height, subresource_extent->depth);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the extent dimensions must always be even
        // integer multiples of the image transfer granularity or the offset + extent dimensions must always match the image
        // subresource extent dimensions.
        VkExtent3D offset_extent_sum = {};
        offset_extent_sum.width = static_cast<uint32_t>(abs(offset->x)) + extent->width;
        offset_extent_sum.height = static_cast<uint32_t>(abs(offset->y)) + extent->height;
        offset_extent_sum.depth = static_cast<uint32_t>(abs(offset->z)) + extent->depth;
        bool x_ok = true;
        bool y_ok = true;
        bool z_ok = true;
        switch (image_type) {
            case VK_IMAGE_TYPE_3D:
                z_ok = ((0 == SafeModulo(extent->depth, granularity->depth)) ||
                        (subresource_extent->depth == offset_extent_sum.depth));
                // fall through
            case VK_IMAGE_TYPE_2D:
                y_ok = ((0 == SafeModulo(extent->height, granularity->height)) ||
                        (subresource_extent->height == offset_extent_sum.height));
                // fall through
            case VK_IMAGE_TYPE_1D:
                x_ok = ((0 == SafeModulo(extent->width, granularity->width)) ||
                        (subresource_extent->width == offset_extent_sum.width));
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
        if (!(x_ok && y_ok && z_ok)) {
            skip |=
                LogError(cb_node->commandBuffer, vuid,
                         "%s: pRegion[%d].%s (w=%d, h=%d, d=%d) dimensions must be even integer multiples of this command "
                         "buffer's queue family image transfer granularity (w=%d, h=%d, d=%d) or offset (x=%d, y=%d, z=%d) + "
                         "extent (w=%d, h=%d, d=%d) must match the image subresource extents (w=%d, h=%d, d=%d).",
                         function, i, member, extent->width, extent->height, extent->depth, granularity->width, granularity->height,
                         granularity->depth, offset->x, offset->y, offset->z, extent->width, extent->height, extent->depth,
                         subresource_extent->width, subresource_extent->height, subresource_extent->depth);
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageMipLevel(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *img, uint32_t mip_level,
                                       const uint32_t i, const char *function, const char *member, const char *vuid) const {
    bool skip = false;
    if (mip_level >= img->createInfo.mipLevels) {
        skip |= LogError(cb_node->commandBuffer, vuid, "In %s, pRegions[%u].%s.mipLevel is %u, but provided %s has %u mip levels.",
                         function, i, member, mip_level, report_data->FormatHandle(img->image).c_str(), img->createInfo.mipLevels);
    }
    return skip;
}

bool CoreChecks::ValidateImageArrayLayerRange(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *img, const uint32_t base_layer,
                                              const uint32_t layer_count, const uint32_t i, const char *function,
                                              const char *member, const char *vuid) const {
    bool skip = false;
    if (base_layer >= img->createInfo.arrayLayers || layer_count > img->createInfo.arrayLayers ||
        (base_layer + layer_count) > img->createInfo.arrayLayers) {
        skip |= LogError(cb_node->commandBuffer, vuid,
                         "In %s, pRegions[%u].%s.baseArrayLayer is %u and .layerCount is "
                         "%u, but provided %s has %u array layers.",
                         function, i, member, base_layer, layer_count, report_data->FormatHandle(img->image).c_str(),
                         img->createInfo.arrayLayers);
    }
    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkBufferImageCopy/VkBufferImageCopy2KHR structure
template <typename BufferImageCopyRegionType>
bool CoreChecks::ValidateCopyBufferImageTransferGranularityRequirements(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *img,
                                                                        const BufferImageCopyRegionType *region, const uint32_t i,
                                                                        const char *function, const char *vuid) const {
    bool skip = false;
    VkExtent3D granularity = GetScaledItg(cb_node, img);
    skip |= CheckItgOffset(cb_node, &region->imageOffset, &granularity, i, function, "imageOffset", vuid);
    VkExtent3D subresource_extent = GetImageSubresourceExtent(img, &region->imageSubresource);
    skip |= CheckItgExtent(cb_node, &region->imageExtent, &region->imageOffset, &granularity, &subresource_extent,
                           img->createInfo.imageType, i, function, "imageExtent", vuid);
    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkImageCopy/VkImageCopy2KHR structure
template <typename RegionType>
bool CoreChecks::ValidateCopyImageTransferGranularityRequirements(const CMD_BUFFER_STATE *cb_node, const IMAGE_STATE *src_img,
                                                                  const IMAGE_STATE *dst_img, const RegionType *region,
                                                                  const uint32_t i, const char *function,
                                                                  CopyCommandVersion version) const {
    bool skip = false;
    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *vuid;

    // Source image checks
    VkExtent3D granularity = GetScaledItg(cb_node, src_img);
    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
    skip |= CheckItgOffset(cb_node, &region->srcOffset, &granularity, i, function, "srcOffset", vuid);
    VkExtent3D subresource_extent = GetImageSubresourceExtent(src_img, &region->srcSubresource);
    const VkExtent3D extent = region->extent;
    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
    skip |= CheckItgExtent(cb_node, &extent, &region->srcOffset, &granularity, &subresource_extent, src_img->createInfo.imageType,
                           i, function, "extent", vuid);

    // Destination image checks
    granularity = GetScaledItg(cb_node, dst_img);
    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
    skip |= CheckItgOffset(cb_node, &region->dstOffset, &granularity, i, function, "dstOffset", vuid);
    // Adjust dest extent, if necessary
    const VkExtent3D dest_effective_extent =
        GetAdjustedDestImageExtent(src_img->createInfo.format, dst_img->createInfo.format, extent);
    subresource_extent = GetImageSubresourceExtent(dst_img, &region->dstSubresource);
    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
    skip |= CheckItgExtent(cb_node, &dest_effective_extent, &region->dstOffset, &granularity, &subresource_extent,
                           dst_img->createInfo.imageType, i, function, "extent", vuid);
    return skip;
}

// Validate contents of a VkImageCopy or VkImageCopy2KHR struct
template <typename ImageCopyRegionType>
bool CoreChecks::ValidateImageCopyData(const uint32_t regionCount, const ImageCopyRegionType *ic_regions,
                                       const IMAGE_STATE *src_state, const IMAGE_STATE *dst_state,
                                       CopyCommandVersion version) const {
    bool skip = false;
    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdCopyImage2KHR()" : "vkCmdCopyImage()";
    const char *vuid;

    for (uint32_t i = 0; i < regionCount; i++) {
        const ImageCopyRegionType region = ic_regions[i];

        // For comp<->uncomp copies, the copy extent for the dest image must be adjusted
        const VkExtent3D src_copy_extent = region.extent;
        const VkExtent3D dst_copy_extent =
            GetAdjustedDestImageExtent(src_state->createInfo.format, dst_state->createInfo.format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if ((VK_IMAGE_TYPE_3D == src_state->createInfo.imageType) && (VK_IMAGE_TYPE_3D != dst_state->createInfo.imageType)) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if ((VK_IMAGE_TYPE_3D == dst_state->createInfo.imageType) && (VK_IMAGE_TYPE_3D != src_state->createInfo.imageType)) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        // Do all checks on source image
        if (src_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.srcOffset.y) || (1 != src_copy_extent.height)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00146" : "VUID-vkCmdCopyImage-srcImage-00146";
                skip |= LogError(src_state->image, vuid,
                                 "%s: pRegion[%d] srcOffset.y is %d and extent.height is %d. For 1D images these must "
                                 "be 0 and 1, respectively.",
                                 func_name, i, region.srcOffset.y, src_copy_extent.height);
            }
        }

        if ((src_state->createInfo.imageType == VK_IMAGE_TYPE_1D) && ((0 != region.srcOffset.z) || (1 != src_copy_extent.depth))) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01785" : "VUID-vkCmdCopyImage-srcImage-01785";
            skip |= LogError(src_state->image, vuid,
                             "%s: pRegion[%d] srcOffset.z is %d and extent.depth is %d. For 1D images "
                             "these must be 0 and 1, respectively.",
                             func_name, i, region.srcOffset.z, src_copy_extent.depth);
        }

        if ((src_state->createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.srcOffset.z)) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01787" : "VUID-vkCmdCopyImage-srcImage-01787";
            skip |= LogError(src_state->image, vuid, "%s: pRegion[%d] srcOffset.z is %d. For 2D images the z-offset must be 0.",
                             func_name, i, region.srcOffset.z);
        }

        // Source checks that apply only to compressed images (or to _422 images if ycbcr enabled)
        bool ext_ycbcr = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion);
        if (FormatIsCompressed(src_state->createInfo.format) ||
            (ext_ycbcr && FormatIsSinglePlane_422(src_state->createInfo.format))) {
            const VkExtent3D block_size = FormatTexelBlockExtent(src_state->createInfo.format);
            //  image offsets must be multiples of block dimensions
            if ((SafeModulo(region.srcOffset.x, block_size.width) != 0) ||
                (SafeModulo(region.srcOffset.y, block_size.height) != 0) ||
                (SafeModulo(region.srcOffset.z, block_size.depth) != 0)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01727" : "VUID-vkCmdCopyImage-srcImage-01727";
                skip |= LogError(src_state->image, vuid,
                                 "%s: pRegion[%d] srcOffset (%d, %d) must be multiples of the compressed image's "
                                 "texel width & height (%d, %d).",
                                 func_name, i, region.srcOffset.x, region.srcOffset.y, block_size.width, block_size.height);
            }

            const VkExtent3D mip_extent = GetImageSubresourceExtent(src_state, &(region.srcSubresource));
            if ((SafeModulo(src_copy_extent.width, block_size.width) != 0) &&
                (src_copy_extent.width + region.srcOffset.x != mip_extent.width)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01728" : "VUID-vkCmdCopyImage-srcImage-01728";
                skip |= LogError(src_state->image, vuid,
                                 "%s: pRegion[%d] extent width (%d) must be a multiple of the compressed texture block "
                                 "width (%d), or when added to srcOffset.x (%d) must equal the image subresource width (%d).",
                                 func_name, i, src_copy_extent.width, block_size.width, region.srcOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or extent+offset height must equal subresource height
            if ((SafeModulo(src_copy_extent.height, block_size.height) != 0) &&
                (src_copy_extent.height + region.srcOffset.y != mip_extent.height)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01729" : "VUID-vkCmdCopyImage-srcImage-01729";
                skip |= LogError(src_state->image, vuid,
                                 "%s: pRegion[%d] extent height (%d) must be a multiple of the compressed texture block "
                                 "height (%d), or when added to srcOffset.y (%d) must equal the image subresource height (%d).",
                                 func_name, i, src_copy_extent.height, block_size.height, region.srcOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : src_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.srcOffset.z != mip_extent.depth)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01730" : "VUID-vkCmdCopyImage-srcImage-01730";
                skip |= LogError(src_state->image, vuid,
                                 "%s: pRegion[%d] extent width (%d) must be a multiple of the compressed texture block "
                                 "depth (%d), or when added to srcOffset.z (%d) must equal the image subresource depth (%d).",
                                 func_name, i, src_copy_extent.depth, block_size.depth, region.srcOffset.z, mip_extent.depth);
            }
        }  // Compressed

        // Do all checks on dest image
        if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.dstOffset.y) || (1 != dst_copy_extent.height)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-00152" : "VUID-vkCmdCopyImage-dstImage-00152";
                skip |= LogError(dst_state->image, vuid,
                                 "%s: pRegion[%d] dstOffset.y is %d and dst_copy_extent.height is %d. For 1D images "
                                 "these must be 0 and 1, respectively.",
                                 func_name, i, region.dstOffset.y, dst_copy_extent.height);
            }
        }

        if ((dst_state->createInfo.imageType == VK_IMAGE_TYPE_1D) && ((0 != region.dstOffset.z) || (1 != dst_copy_extent.depth))) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01786" : "VUID-vkCmdCopyImage-dstImage-01786";
            skip |= LogError(dst_state->image, vuid,
                             "%s: pRegion[%d] dstOffset.z is %d and extent.depth is %d. For 1D images these must be 0 "
                             "and 1, respectively.",
                             func_name, i, region.dstOffset.z, dst_copy_extent.depth);
        }

        if ((dst_state->createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.dstOffset.z)) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01788" : "VUID-vkCmdCopyImage-dstImage-01788";
            skip |= LogError(dst_state->image, vuid, "%s: pRegion[%d] dstOffset.z is %d. For 2D images the z-offset must be 0.",
                             func_name, i, region.dstOffset.z);
        }

        // Handle difference between Maintenance 1
        if (device_extensions.vk_khr_maintenance1) {
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-04443" : "VUID-vkCmdCopyImage-srcImage-04443";
                    skip |= LogError(src_state->image, vuid,
                                     "%s: pRegion[%d] srcSubresource.baseArrayLayer is %d and srcSubresource.layerCount "
                                     "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                     func_name, i, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
            }
            if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-04444" : "VUID-vkCmdCopyImage-dstImage-04444";
                    skip |= LogError(dst_state->image, vuid,
                                     "%s: pRegion[%d] dstSubresource.baseArrayLayer is %d and dstSubresource.layerCount "
                                     "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                     func_name, i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        } else {  // Pre maint 1
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D || dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00139" : "VUID-vkCmdCopyImage-srcImage-00139";
                    skip |= LogError(src_state->image, vuid,
                                     "%s: pRegion[%d] srcSubresource.baseArrayLayer is %d and "
                                     "srcSubresource.layerCount is %d. For copies with either source or dest of type "
                                     "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                     func_name, i, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00139" : "VUID-vkCmdCopyImage-srcImage-00139";
                    skip |= LogError(dst_state->image, vuid,
                                     "%s: pRegion[%d] dstSubresource.baseArrayLayer is %d and "
                                     "dstSubresource.layerCount is %d. For copies with either source or dest of type "
                                     "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                     func_name, i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        }

        // Dest checks that apply only to compressed images (or to _422 images if ycbcr enabled)
        if (FormatIsCompressed(dst_state->createInfo.format) ||
            (ext_ycbcr && FormatIsSinglePlane_422(dst_state->createInfo.format))) {
            const VkExtent3D block_size = FormatTexelBlockExtent(dst_state->createInfo.format);

            //  image offsets must be multiples of block dimensions
            if ((SafeModulo(region.dstOffset.x, block_size.width) != 0) ||
                (SafeModulo(region.dstOffset.y, block_size.height) != 0) ||
                (SafeModulo(region.dstOffset.z, block_size.depth) != 0)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01731" : "VUID-vkCmdCopyImage-dstImage-01731";
                skip |= LogError(dst_state->image, vuid,
                                 "%s: pRegion[%d] dstOffset (%d, %d) must be multiples of the compressed image's "
                                 "texel width & height (%d, %d).",
                                 func_name, i, region.dstOffset.x, region.dstOffset.y, block_size.width, block_size.height);
            }

            const VkExtent3D mip_extent = GetImageSubresourceExtent(dst_state, &(region.dstSubresource));
            if ((SafeModulo(dst_copy_extent.width, block_size.width) != 0) &&
                (dst_copy_extent.width + region.dstOffset.x != mip_extent.width)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01732" : "VUID-vkCmdCopyImage-dstImage-01732";
                skip |= LogError(dst_state->image, vuid,
                                 "%s: pRegion[%d] dst_copy_extent width (%d) must be a multiple of the compressed texture "
                                 "block width (%d), or when added to dstOffset.x (%d) must equal the image subresource width (%d).",
                                 func_name, i, dst_copy_extent.width, block_size.width, region.dstOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or dst_copy_extent+offset height must equal subresource height
            if ((SafeModulo(dst_copy_extent.height, block_size.height) != 0) &&
                (dst_copy_extent.height + region.dstOffset.y != mip_extent.height)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
                skip |= LogError(dst_state->image, vuid,
                                 "%s: pRegion[%d] dst_copy_extent height (%d) must be a multiple of the compressed "
                                 "texture block height (%d), or when added to dstOffset.y (%d) must equal the image subresource "
                                 "height (%d).",
                                 func_name, i, dst_copy_extent.height, block_size.height, region.dstOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or dst_copy_extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : dst_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.dstOffset.z != mip_extent.depth)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01734" : "VUID-vkCmdCopyImage-dstImage-01734";
                skip |= LogError(dst_state->image, vuid,
                                 "%s: pRegion[%d] dst_copy_extent width (%d) must be a multiple of the compressed texture "
                                 "block depth (%d), or when added to dstOffset.z (%d) must equal the image subresource depth (%d).",
                                 func_name, i, dst_copy_extent.depth, block_size.depth, region.dstOffset.z, mip_extent.depth);
            }
        }  // Compressed
    }
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, CopyCommandVersion version) const {
    const auto *cb_node = GetCBState(commandBuffer);
    const auto *src_image_state = GetImageState(srcImage);
    const auto *dst_image_state = GetImageState(dstImage);
    const VkFormat src_format = src_image_state->createInfo.format;
    const VkFormat dst_format = dst_image_state->createInfo.format;
    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    bool skip = false;

    const char *func_name = is_2khr ? "vkCmdCopyImage2KHR()" : "vkCmdCopyImage()";
    const CMD_TYPE cmd_type = is_2khr ? CMD_COPYIMAGE2KHR : CMD_COPYIMAGE;
    const char *vuid;

    skip = ValidateImageCopyData(regionCount, pRegions, src_image_state, dst_image_state, version);

    VkCommandBuffer command_buffer = cb_node->commandBuffer;

    for (uint32_t i = 0; i < regionCount; i++) {
        const RegionType region = pRegions[i];

        // For comp/uncomp copies, the copy extent for the dest image must be adjusted
        VkExtent3D src_copy_extent = region.extent;
        VkExtent3D dst_copy_extent = GetAdjustedDestImageExtent(src_format, dst_format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if ((VK_IMAGE_TYPE_3D == src_image_state->createInfo.imageType) &&
            (VK_IMAGE_TYPE_3D != dst_image_state->createInfo.imageType)) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if ((VK_IMAGE_TYPE_3D == dst_image_state->createInfo.imageType) &&
                   (VK_IMAGE_TYPE_3D != src_image_state->createInfo.imageType)) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        skip |= ValidateImageSubresourceLayers(cb_node, &region.srcSubresource, func_name, "srcSubresource", i);
        skip |= ValidateImageSubresourceLayers(cb_node, &region.dstSubresource, func_name, "dstSubresource", i);
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcSubresource-01696" : "VUID-vkCmdCopyImage-srcSubresource-01696";
        skip |=
            ValidateImageMipLevel(cb_node, src_image_state, region.srcSubresource.mipLevel, i, func_name, "srcSubresource", vuid);
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstSubresource-01697" : "VUID-vkCmdCopyImage-dstSubresource-01697";
        skip |=
            ValidateImageMipLevel(cb_node, dst_image_state, region.dstSubresource.mipLevel, i, func_name, "dstSubresource", vuid);
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcSubresource-01698" : "VUID-vkCmdCopyImage-srcSubresource-01698";
        skip |= ValidateImageArrayLayerRange(cb_node, src_image_state, region.srcSubresource.baseArrayLayer,
                                             region.srcSubresource.layerCount, i, func_name, "srcSubresource", vuid);
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstSubresource-01699" : "VUID-vkCmdCopyImage-dstSubresource-01699";
        skip |= ValidateImageArrayLayerRange(cb_node, dst_image_state, region.dstSubresource.baseArrayLayer,
                                             region.dstSubresource.layerCount, i, func_name, "dstSubresource", vuid);

        if (device_extensions.vk_khr_maintenance1) {
            // No chance of mismatch if we're overriding depth slice count
            if (!slice_override) {
                // The number of depth slices in srcSubresource and dstSubresource must match
                // Depth comes from layerCount for 1D,2D resources, from extent.depth for 3D
                uint32_t src_slices =
                    (VK_IMAGE_TYPE_3D == src_image_state->createInfo.imageType ? src_copy_extent.depth
                                                                               : region.srcSubresource.layerCount);
                uint32_t dst_slices =
                    (VK_IMAGE_TYPE_3D == dst_image_state->createInfo.imageType ? dst_copy_extent.depth
                                                                               : region.dstSubresource.layerCount);
                if (src_slices != dst_slices) {
                    vuid = is_2khr ? "VUID-VkImageCopy2KHR-extent-00140" : "VUID-VkImageCopy-extent-00140";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: number of depth slices in source (%u) and destination (%u) subresources for pRegions[%u] "
                                     "do not match.",
                                     func_name, src_slices, dst_slices, i);
                }
            }
        } else {
            // For each region the layerCount member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.layerCount != region.dstSubresource.layerCount) {
                vuid = is_2khr ? "VUID-VkImageCopy2KHR-layerCount-00138" : "VUID-VkImageCopy-layerCount-00138";
                skip |=
                    LogError(command_buffer, vuid,
                             "%s: number of layers in source (%u) and destination (%u) subresources for pRegions[%u] do not match",
                             func_name, region.srcSubresource.layerCount, region.dstSubresource.layerCount, i);
            }
        }

        // Do multiplane-specific checks, if extension enabled
        if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
            if ((!FormatIsMultiplane(src_format)) && (!FormatIsMultiplane(dst_format))) {
                // If neither image is multi-plane the aspectMask member of src and dst must match
                if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01551" : "VUID-vkCmdCopyImage-srcImage-01551";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: Copy between non-multiplane images with differing aspectMasks in pRegions[%u] with "
                                     "source (0x%x) destination (0x%x).",
                                     func_name, i, region.srcSubresource.aspectMask, region.dstSubresource.aspectMask);
                }
            } else {
                // Source image multiplane checks
                uint32_t planes = FormatPlaneCount(src_format);
                VkImageAspectFlags aspect = region.srcSubresource.aspectMask;
                if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01552" : "VUID-vkCmdCopyImage-srcImage-01552";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%u].srcSubresource.aspectMask (0x%x) is invalid for 2-plane format.", func_name,
                                     i, aspect);
                }
                if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                    (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01553" : "VUID-vkCmdCopyImage-srcImage-01553";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%u].srcSubresource.aspectMask (0x%x) is invalid for 3-plane format.", func_name,
                                     i, aspect);
                }
                // Single-plane to multi-plane
                if ((!FormatIsMultiplane(src_format)) && (FormatIsMultiplane(dst_format)) &&
                    (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01557" : "VUID-vkCmdCopyImage-dstImage-01557";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%u].srcSubresource.aspectMask (0x%x) is not VK_IMAGE_ASPECT_COLOR_BIT.",
                                     func_name, i, aspect);
                }

                // Dest image multiplane checks
                planes = FormatPlaneCount(dst_format);
                aspect = region.dstSubresource.aspectMask;
                if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01554" : "VUID-vkCmdCopyImage-dstImage-01554";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%u].dstSubresource.aspectMask (0x%x) is invalid for 2-plane format.", func_name,
                                     i, aspect);
                }
                if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                    (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01555" : "VUID-vkCmdCopyImage-dstImage-01555";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%u].dstSubresource.aspectMask (0x%x) is invalid for 3-plane format.", func_name,
                                     i, aspect);
                }
                // Multi-plane to single-plane
                if ((FormatIsMultiplane(src_format)) && (!FormatIsMultiplane(dst_format)) &&
                    (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01556" : "VUID-vkCmdCopyImage-srcImage-01556";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%u].dstSubresource.aspectMask (0x%x) is not VK_IMAGE_ASPECT_COLOR_BIT.",
                                     func_name, i, aspect);
                }
            }
        } else {
            // !vk_khr_sampler_ycbcr_conversion
            // not multi-plane, the aspectMask member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                vuid = is_2khr ? "VUID-VkImageCopy2KHR-aspectMask-00137" : "VUID-VkImageCopy-aspectMask-00137";
                skip |= LogError(
                    command_buffer, vuid,
                    "%s: Copy between images with differing aspectMasks in pRegions[%u] with source (0x%x) destination (0x%x).",
                    func_name, i, region.srcSubresource.aspectMask, region.dstSubresource.aspectMask);
            }
        }

        // For each region, the aspectMask member of srcSubresource must be present in the source image
        if (!VerifyAspectsPresent(region.srcSubresource.aspectMask, src_format)) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-aspectMask-00142" : "VUID-vkCmdCopyImage-aspectMask-00142";
            skip |=
                LogError(command_buffer, vuid,
                         "%s: pRegions[%u].srcSubresource.aspectMask (0x%x) cannot specify aspects not present in source image.",
                         func_name, i, region.srcSubresource.aspectMask);
        }

        // For each region, the aspectMask member of dstSubresource must be present in the destination image
        if (!VerifyAspectsPresent(region.dstSubresource.aspectMask, dst_format)) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-aspectMask-00143" : "VUID-vkCmdCopyImage-aspectMask-00143";
            skip |= LogError(
                command_buffer, vuid,
                "%s: pRegions[%u].dstSubresource.aspectMask (0x%x) cannot specify aspects not present in destination image.",
                func_name, i, region.dstSubresource.aspectMask);
        }

        // Each dimension offset + extent limits must fall with image subresource extent
        VkExtent3D subresource_extent = GetImageSubresourceExtent(src_image_state, &(region.srcSubresource));
        if (slice_override) src_copy_extent.depth = depth_slices;
        uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &src_copy_extent, &subresource_extent);
        if (extent_check & kXBit) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcOffset-00144" : "VUID-vkCmdCopyImage-srcOffset-00144";
            skip |= LogError(command_buffer, vuid,
                             "%s: Source image pRegion[%u] x-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "width [%1d].",
                             func_name, i, region.srcOffset.x, src_copy_extent.width, subresource_extent.width);
        }

        if (extent_check & kYBit) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcOffset-00145" : "VUID-vkCmdCopyImage-srcOffset-00145";
            skip |= LogError(command_buffer, vuid,
                             "%s: Source image pRegion[%u] y-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "height [%1d].",
                             func_name, i, region.srcOffset.y, src_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & kZBit) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcOffset-00147" : "VUID-vkCmdCopyImage-srcOffset-00147";
            skip |= LogError(command_buffer, vuid,
                             "%s: Source image pRegion[%u] z-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "depth [%1d].",
                             func_name, i, region.srcOffset.z, src_copy_extent.depth, subresource_extent.depth);
        }

        // Adjust dest extent if necessary
        subresource_extent = GetImageSubresourceExtent(dst_image_state, &(region.dstSubresource));
        if (slice_override) dst_copy_extent.depth = depth_slices;

        extent_check = ExceedsBounds(&(region.dstOffset), &dst_copy_extent, &subresource_extent);
        if (extent_check & kXBit) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstOffset-00150" : "VUID-vkCmdCopyImage-dstOffset-00150";
            skip |= LogError(command_buffer, vuid,
                             "%s: Dest image pRegion[%u] x-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "width [%1d].",
                             func_name, i, region.dstOffset.x, dst_copy_extent.width, subresource_extent.width);
        }
        if (extent_check & kYBit) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstOffset-00151" : "VUID-vkCmdCopyImage-dstOffset-00151";
            skip |= LogError(command_buffer, vuid,
                             "%s): Dest image pRegion[%u] y-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "height [%1d].",
                             func_name, i, region.dstOffset.y, dst_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & kZBit) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstOffset-00153" : "VUID-vkCmdCopyImage-dstOffset-00153";
            skip |= LogError(command_buffer, vuid,
                             "%s: Dest image pRegion[%u] z-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "depth [%1d].",
                             func_name, i, region.dstOffset.z, dst_copy_extent.depth, subresource_extent.depth);
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        if (src_image_state->image == dst_image_state->image) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (RegionIntersects(&region, &pRegions[j], src_image_state->createInfo.imageType,
                                     FormatIsMultiplane(src_format))) {
                    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-pRegions-00124" : "VUID-vkCmdCopyImage-pRegions-00124";
                    skip |= LogError(command_buffer, vuid, "%s: pRegion[%u] src overlaps with pRegions[%u].", func_name, i, j);
                }
            }
        }

        // Check depth for 2D as post Maintaince 1 requires both while prior only required one to be 2D
        if (device_extensions.vk_khr_maintenance1) {
            if (((VK_IMAGE_TYPE_2D == src_image_state->createInfo.imageType) &&
                 (VK_IMAGE_TYPE_2D == dst_image_state->createInfo.imageType)) &&
                (src_copy_extent.depth != 1)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01790" : "VUID-vkCmdCopyImage-srcImage-01790";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%u] both srcImage and dstImage are 2D and extent.depth is %u and has to be 1",
                                 func_name, i, src_copy_extent.depth);
            }
        } else {
            if (((VK_IMAGE_TYPE_2D == src_image_state->createInfo.imageType) ||
                 (VK_IMAGE_TYPE_2D == dst_image_state->createInfo.imageType)) &&
                (src_copy_extent.depth != 1)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01789" : "VUID-vkCmdCopyImage-srcImage-01789";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%u] either srcImage or dstImage is 2D and extent.depth is %u and has to be 1",
                                 func_name, i, src_copy_extent.depth);
            }
        }

        // Check if 2D with 3D and depth not equal to 2D layerCount
        if ((VK_IMAGE_TYPE_2D == src_image_state->createInfo.imageType) &&
            (VK_IMAGE_TYPE_3D == dst_image_state->createInfo.imageType) &&
            (src_copy_extent.depth != region.srcSubresource.layerCount)) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01791" : "VUID-vkCmdCopyImage-srcImage-01791";
            skip |= LogError(command_buffer, vuid,
                             "%s: pRegion[%u] srcImage is 2D, dstImage is 3D and extent.depth is %u and has to be "
                             "srcSubresource.layerCount (%u)",
                             func_name, i, src_copy_extent.depth, region.srcSubresource.layerCount);
        } else if ((VK_IMAGE_TYPE_3D == src_image_state->createInfo.imageType) &&
                   (VK_IMAGE_TYPE_2D == dst_image_state->createInfo.imageType) &&
                   (src_copy_extent.depth != region.dstSubresource.layerCount)) {
            vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01792" : "VUID-vkCmdCopyImage-dstImage-01792";
            skip |= LogError(command_buffer, vuid,
                             "%s: pRegion[%u] srcImage is 3D, dstImage is 2D and extent.depth is %u and has to be "
                             "dstSubresource.layerCount (%u)",
                             func_name, i, src_copy_extent.depth, region.dstSubresource.layerCount);
        }

        // Check for multi-plane format compatiblity
        if (FormatIsMultiplane(src_format) || FormatIsMultiplane(dst_format)) {
            size_t src_format_size = 0;
            size_t dst_format_size = 0;
            if (FormatIsMultiplane(src_format)) {
                const VkFormat plane_format = FindMultiplaneCompatibleFormat(src_format, region.srcSubresource.aspectMask);
                src_format_size = FormatElementSize(plane_format);
            } else {
                src_format_size = FormatElementSize(src_format);
            }
            if (FormatIsMultiplane(dst_format)) {
                const VkFormat plane_format = FindMultiplaneCompatibleFormat(dst_format, region.dstSubresource.aspectMask);
                dst_format_size = FormatElementSize(plane_format);
            } else {
                dst_format_size = FormatElementSize(dst_format);
            }
            // If size is still zero, then format is invalid and will be caught in another VU
            if ((src_format_size != dst_format_size) && (src_format_size != 0) && (dst_format_size != 0)) {
                vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-None-01549" : "VUID-vkCmdCopyImage-None-01549";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegions[%u] called with non-compatible image formats. "
                                 "The src format %s with aspectMask %s is not compatible with dst format %s aspectMask %s.",
                                 func_name, i, string_VkFormat(src_format),
                                 string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(), string_VkFormat(dst_format),
                                 string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str());
            }
        }
    }

    // The formats of non-multiplane src_image and dst_image must be compatible. Formats are considered compatible if their texel
    // size in bytes is the same between both formats. For example, VK_FORMAT_R8G8B8A8_UNORM is compatible with VK_FORMAT_R32_UINT
    // because because both texels are 4 bytes in size.
    if (!FormatIsMultiplane(src_format) && !FormatIsMultiplane(dst_format)) {
        const char *compatible_vuid =
            (device_extensions.vk_khr_sampler_ycbcr_conversion)
                ? (is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01548" : "VUID-vkCmdCopyImage-srcImage-01548")
                : (is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00135" : "VUID-vkCmdCopyImage-srcImage-00135");
        // Depth/stencil formats must match exactly.
        if (FormatIsDepthOrStencil(src_format) || FormatIsDepthOrStencil(dst_format)) {
            if (src_format != dst_format) {
                skip |= LogError(command_buffer, compatible_vuid,
                                 "%s: Depth/stencil formats must match exactly for src (%s) and dst (%s).", func_name,
                                 string_VkFormat(src_format), string_VkFormat(dst_format));
            }
        } else {
            if (FormatElementSize(src_format) != FormatElementSize(dst_format)) {
                skip |= LogError(command_buffer, compatible_vuid,
                                 "%s: Unmatched image format sizes. "
                                 "The src format %s has size of %zu and dst format %s has size of %zu.",
                                 func_name, string_VkFormat(src_format), FormatElementSize(src_format), string_VkFormat(dst_format),
                                 FormatElementSize(dst_format));
            }
        }
    }

    // Source and dest image sample counts must match
    if (src_image_state->createInfo.samples != dst_image_state->createInfo.samples) {
        std::stringstream ss;
        ss << func_name << " called on image pair with non-identical sample counts.";
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00136" : "VUID-vkCmdCopyImage-srcImage-00136";
        skip |=
            LogError(command_buffer, vuid, "%s: The src image sample count (%s) dose not match the dst image sample count (%s).",
                     func_name, string_VkSampleCountFlagBits(src_image_state->createInfo.samples),
                     string_VkSampleCountFlagBits(dst_image_state->createInfo.samples));
    }

    vuid = (device_extensions.vk_khr_sampler_ycbcr_conversion)
               ? (is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01546" : "VUID-vkCmdCopyImage-srcImage-01546")
               : (is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00127" : "VUID-vkCmdCopyImage-srcImage-00127");
    skip |= ValidateMemoryIsBoundToImage(src_image_state, func_name, vuid);
    vuid = (device_extensions.vk_khr_sampler_ycbcr_conversion)
               ? (is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01547" : "VUID-vkCmdCopyImage-dstImage-01547")
               : (is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-00132" : "VUID-vkCmdCopyImage-dstImage-00132");
    skip |= ValidateMemoryIsBoundToImage(dst_image_state, func_name, vuid);
    // Validate that SRC & DST images have correct usage flags set
    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-00126" : "VUID-vkCmdCopyImage-srcImage-00126";
    skip |= ValidateImageUsageFlags(src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name,
                                    "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
    vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-00131" : "VUID-vkCmdCopyImage-dstImage-00131";
    skip |= ValidateImageUsageFlags(dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, func_name,
                                    "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
    vuid = is_2khr ? "VUID-vkCmdCopyImage2KHR-commandBuffer-01825" : "VUID-vkCmdCopyImage-commandBuffer-01825";
    skip |= ValidateProtectedImage(cb_node, src_image_state, func_name, vuid);
    vuid = is_2khr ? "VUID-vkCmdCopyImage2KHR-commandBuffer-01826" : "VUID-vkCmdCopyImage-commandBuffer-01826";
    skip |= ValidateProtectedImage(cb_node, dst_image_state, func_name, vuid);
    vuid = is_2khr ? "VUID-vkCmdCopyImage2KHR-commandBuffer-01827" : "VUID-vkCmdCopyImage-commandBuffer-01827";
    skip |= ValidateUnprotectedImage(cb_node, dst_image_state, func_name, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-02542" : "VUID-vkCmdCopyImage-dstImage-02542";
        skip |=
            LogError(command_buffer, vuid,
                     "%s: srcImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT", func_name);
    }
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-02542" : "VUID-vkCmdCopyImage-dstImage-02542";
        skip |=
            LogError(command_buffer, vuid,
                     "%s: dstImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT", func_name);
    }

    if (device_extensions.vk_khr_maintenance1) {
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImage-01995" : "VUID-vkCmdCopyImage-srcImage-01995";
        skip |= ValidateImageFormatFeatureFlags(src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, func_name, vuid);
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImage-01996" : "VUID-vkCmdCopyImage-dstImage-01996";
        skip |= ValidateImageFormatFeatureFlags(dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, func_name, vuid);
    }
    vuid = is_2khr ? "VUID-vkCmdCopyImage2KHR-commandBuffer-cmdpool" : "VUID-vkCmdCopyImage-commandBuffer-cmdpool";
    skip |= ValidateCmdQueueFlags(cb_node, func_name, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, vuid);
    skip |= ValidateCmd(cb_node, cmd_type, func_name);
    vuid = is_2khr ? "VUID-vkCmdCopyImage2KHR-renderpass" : "VUID-vkCmdCopyImage-renderpass";
    skip |= InsideRenderPass(cb_node, func_name, vuid);
    bool hit_error = false;

    const char *invalid_src_layout_vuid =
        (src_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
            ? (is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImageLayout-01917" : "VUID-vkCmdCopyImage-srcImageLayout-01917")
            : (is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImageLayout-00129" : "VUID-vkCmdCopyImage-srcImageLayout-00129");
    const char *invalid_dst_layout_vuid =
        (dst_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
            ? (is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImageLayout-01395" : "VUID-vkCmdCopyImage-dstImageLayout-01395")
            : (is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImageLayout-00134" : "VUID-vkCmdCopyImage-dstImageLayout-00134");

    for (uint32_t i = 0; i < regionCount; ++i) {
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-srcImageLayout-00128" : "VUID-vkCmdCopyImage-srcImageLayout-00128";
        skip |= VerifyImageLayout(cb_node, src_image_state, pRegions[i].srcSubresource, srcImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, func_name, invalid_src_layout_vuid, vuid, &hit_error);
        vuid = is_2khr ? "VUID-VkCopyImageInfo2KHR-dstImageLayout-00133" : "VUID-vkCmdCopyImage-dstImageLayout-00133";
        skip |= VerifyImageLayout(cb_node, dst_image_state, pRegions[i].dstSubresource, dstImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, func_name, invalid_dst_layout_vuid, vuid, &hit_error);
        skip |= ValidateCopyImageTransferGranularityRequirements(cb_node, src_image_state, dst_image_state, &pRegions[i], i,
                                                                 func_name, version);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageCopy *pRegions) const {
    return ValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                COPY_COMMAND_VERSION_1);
}

bool CoreChecks::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) const {
    return ValidateCmdCopyImage(commandBuffer, pCopyImageInfo->srcImage, pCopyImageInfo->srcImageLayout, pCopyImageInfo->dstImage,
                                pCopyImageInfo->dstImageLayout, pCopyImageInfo->regionCount, pCopyImageInfo->pRegions,
                                COPY_COMMAND_VERSION_2);
}

void CoreChecks::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions);
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    auto dst_image_state = GetImageState(dstImage);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageInitialLayout(cb_node, *src_image_state, pRegions[i].srcSubresource, srcImageLayout);
        SetImageInitialLayout(cb_node, *dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
    }
}

void CoreChecks::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) {
    StateTracker::PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(pCopyImageInfo->srcImage);
    auto dst_image_state = GetImageState(pCopyImageInfo->dstImage);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < pCopyImageInfo->regionCount; ++i) {
        SetImageInitialLayout(cb_node, *src_image_state, pCopyImageInfo->pRegions[i].srcSubresource,
                              pCopyImageInfo->srcImageLayout);
        SetImageInitialLayout(cb_node, *dst_image_state, pCopyImageInfo->pRegions[i].dstSubresource,
                              pCopyImageInfo->dstImageLayout);
    }
}

// Returns true if sub_rect is entirely contained within rect
static inline bool ContainsRect(VkRect2D rect, VkRect2D sub_rect) {
    if ((sub_rect.offset.x < rect.offset.x) || (sub_rect.offset.x + sub_rect.extent.width > rect.offset.x + rect.extent.width) ||
        (sub_rect.offset.y < rect.offset.y) || (sub_rect.offset.y + sub_rect.extent.height > rect.offset.y + rect.extent.height)) {
        return false;
    }
    return true;
}

bool CoreChecks::ValidateClearAttachmentExtent(VkCommandBuffer command_buffer, uint32_t attachment_index,
                                               const FRAMEBUFFER_STATE *framebuffer, uint32_t fb_attachment,
                                               const VkRect2D &render_area, uint32_t rect_count, const VkClearRect *clear_rects,
                                               const CMD_BUFFER_STATE *primary_cb_state) const {
    bool skip = false;
    const IMAGE_VIEW_STATE *image_view_state = nullptr;
    if (framebuffer && (fb_attachment != VK_ATTACHMENT_UNUSED) && (fb_attachment < framebuffer->createInfo.attachmentCount)) {
        image_view_state = GetActiveAttachmentImageViewState(GetCBState(command_buffer), fb_attachment, primary_cb_state);
    }

    for (uint32_t j = 0; j < rect_count; j++) {
        if (!ContainsRect(render_area, clear_rects[j].rect)) {
            skip |= LogError(command_buffer, "VUID-vkCmdClearAttachments-pRects-00016",
                             "vkCmdClearAttachments(): The area defined by pRects[%d] is not contained in the area of "
                             "the current render pass instance.",
                             j);
        }

        if (image_view_state) {
            // The layers specified by a given element of pRects must be contained within every attachment that
            // pAttachments refers to
            const auto attachment_layer_count = image_view_state->create_info.subresourceRange.layerCount;
            if ((clear_rects[j].baseArrayLayer >= attachment_layer_count) ||
                (clear_rects[j].baseArrayLayer + clear_rects[j].layerCount > attachment_layer_count)) {
                skip |= LogError(command_buffer, "VUID-vkCmdClearAttachments-pRects-00017",
                                 "vkCmdClearAttachments(): The layers defined in pRects[%d] are not contained in the layers "
                                 "of pAttachment[%d].",
                                 j, attachment_index);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                    const VkClearRect *pRects) const {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_node = GetCBState(commandBuffer);  // TODO: Should be const, and never modified during validation
    if (!cb_node) return skip;

    skip |= ValidateCmdQueueFlags(cb_node, "vkCmdClearAttachments()", VK_QUEUE_GRAPHICS_BIT,
                                  "VUID-vkCmdClearAttachments-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_node, CMD_CLEARATTACHMENTS, "vkCmdClearAttachments()");
    skip |= OutsideRenderPass(cb_node, "vkCmdClearAttachments()", "VUID-vkCmdClearAttachments-renderpass");

    // Validate that attachment is in reference list of active subpass
    if (cb_node->activeRenderPass) {
        const VkRenderPassCreateInfo2 *renderpass_create_info = cb_node->activeRenderPass->createInfo.ptr();
        const uint32_t renderpass_attachment_count = renderpass_create_info->attachmentCount;
        const VkSubpassDescription2 *subpass_desc = &renderpass_create_info->pSubpasses[cb_node->activeSubpass];
        const auto *framebuffer = cb_node->activeFramebuffer.get();
        const auto &render_area = cb_node->activeRenderPassBeginInfo.renderArea;

        for (uint32_t attachment_index = 0; attachment_index < attachmentCount; attachment_index++) {
            auto clear_desc = &pAttachments[attachment_index];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            const VkImageAspectFlags aspect_mask = clear_desc->aspectMask;

            if (aspect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
                skip |= LogError(commandBuffer, "VUID-VkClearAttachment-aspectMask-00020",
                                 "vkCmdClearAttachments() pAttachments[%u] mask contains VK_IMAGE_ASPECT_METADATA_BIT",
                                 attachment_index);
            } else if (aspect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                                      VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
                skip |=
                    LogError(commandBuffer, "VUID-VkClearAttachment-aspectMask-02246",
                             "vkCmdClearAttachments() pAttachments[%u] mask contains a VK_IMAGE_ASPECT_MEMORY_PLANE_*_BIT_EXT bit",
                             attachment_index);
            } else if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
                uint32_t color_attachment = VK_ATTACHMENT_UNUSED;
                if (clear_desc->colorAttachment < subpass_desc->colorAttachmentCount) {
                    color_attachment = subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment;
                    if ((color_attachment != VK_ATTACHMENT_UNUSED) && (color_attachment >= renderpass_attachment_count)) {
                        skip |= LogError(
                            commandBuffer, "VUID-vkCmdClearAttachments-aspectMask-02501",
                            "vkCmdClearAttachments() pAttachments[%u].colorAttachment=%u is not VK_ATTACHMENT_UNUSED "
                            "and not a valid attachment for %s attachmentCount=%u. Subpass %u pColorAttachment[%u]=%u.",
                            attachment_index, clear_desc->colorAttachment,
                            report_data->FormatHandle(cb_node->activeRenderPass->renderPass).c_str(), cb_node->activeSubpass,
                            clear_desc->colorAttachment, color_attachment, renderpass_attachment_count);

                        color_attachment = VK_ATTACHMENT_UNUSED;  // Defensive, prevent lookup past end of renderpass attachment
                    }
                } else {
                    skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-aspectMask-02501",
                                     "vkCmdClearAttachments() pAttachments[%u].colorAttachment=%u out of range for %s"
                                     " subpass %u. colorAttachmentCount=%u",
                                     attachment_index, clear_desc->colorAttachment,
                                     report_data->FormatHandle(cb_node->activeRenderPass->renderPass).c_str(),
                                     cb_node->activeSubpass, subpass_desc->colorAttachmentCount);
                }
                fb_attachment = color_attachment;

                if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
                    (clear_desc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    skip |= LogError(commandBuffer, "VUID-VkClearAttachment-aspectMask-00019",
                                     "vkCmdClearAttachments() pAttachments[%u] aspectMask must set only VK_IMAGE_ASPECT_COLOR_BIT "
                                     "of a color attachment.",
                                     attachment_index);
                }
            } else {  // Must be depth and/or stencil
                bool subpass_depth = false;
                bool subpass_stencil = false;
                if (subpass_desc->pDepthStencilAttachment &&
                    (subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
                    auto index = subpass_desc->pDepthStencilAttachment->attachment;
                    subpass_depth = FormatHasDepth(renderpass_create_info->pAttachments[index].format);
                    subpass_stencil = FormatHasStencil(renderpass_create_info->pAttachments[index].format);
                }
                if (!subpass_desc->pDepthStencilAttachment ||
                    (subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
                    if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) && !subpass_depth) {
                        skip |= LogError(
                            commandBuffer, "VUID-vkCmdClearAttachments-aspectMask-02502",
                            "vkCmdClearAttachments() pAttachments[%u] aspectMask has VK_IMAGE_ASPECT_DEPTH_BIT but there is no "
                            "depth attachment in subpass",
                            attachment_index);
                    }
                    if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) && !subpass_stencil) {
                        skip |= LogError(
                            commandBuffer, "VUID-vkCmdClearAttachments-aspectMask-02503",
                            "vkCmdClearAttachments() pAttachments[%u] aspectMask has VK_IMAGE_ASPECT_STENCIL_BIT but there is no "
                            "stencil attachment in subpass",
                            attachment_index);
                    }
                } else {
                    fb_attachment = subpass_desc->pDepthStencilAttachment->attachment;
                }
                if (subpass_depth) {
                    skip |= ValidateClearDepthStencilValue(commandBuffer, clear_desc->clearValue.depthStencil,
                                                           "vkCmdClearAttachments()");
                }
            }
            if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
                skip |= ValidateClearAttachmentExtent(commandBuffer, attachment_index, framebuffer, fb_attachment, render_area,
                                                      rectCount, pRects);
            }

            // Once the framebuffer attachment is found, can get the image view state
            if (framebuffer && (fb_attachment != VK_ATTACHMENT_UNUSED) &&
                (fb_attachment < framebuffer->createInfo.attachmentCount)) {
                const IMAGE_VIEW_STATE *image_view_state =
                    GetActiveAttachmentImageViewState(GetCBState(commandBuffer), fb_attachment);
                if (image_view_state != nullptr) {
                    skip |= ValidateProtectedImage(cb_node, image_view_state->image_state.get(), "vkCmdClearAttachments()",
                                                   "VUID-vkCmdClearAttachments-commandBuffer-02504");
                    skip |= ValidateUnprotectedImage(cb_node, image_view_state->image_state.get(), "vkCmdClearAttachments()",
                                                     "VUID-vkCmdClearAttachments-commandBuffer-02505");
                }
            }
        }
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                  const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                  const VkClearRect *pRects) {
    auto *cb_node = GetCBState(commandBuffer);
    if (cb_node->activeRenderPass && (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)) {
        const VkRenderPassCreateInfo2 *renderpass_create_info = cb_node->activeRenderPass->createInfo.ptr();
        const VkSubpassDescription2 *subpass_desc = &renderpass_create_info->pSubpasses[cb_node->activeSubpass];
        std::shared_ptr<std::vector<VkClearRect>> clear_rect_copy;
        for (uint32_t attachment_index = 0; attachment_index < attachmentCount; attachment_index++) {
            const auto clear_desc = &pAttachments[attachment_index];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
                (clear_desc->colorAttachment < subpass_desc->colorAttachmentCount)) {
                fb_attachment = subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment;
            } else if ((clear_desc->aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) &&
                       subpass_desc->pDepthStencilAttachment) {
                fb_attachment = subpass_desc->pDepthStencilAttachment->attachment;
            }
            if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                if (!clear_rect_copy) {
                    // We need a copy of the clear rectangles that will persist until the last lambda executes
                    // but we want to create it as lazily as possible
                    clear_rect_copy.reset(new std::vector<VkClearRect>(pRects, pRects + rectCount));
                }
                // if a secondary level command buffer inherits the framebuffer from the primary command buffer
                // (see VkCommandBufferInheritanceInfo), this validation must be deferred until queue submit time
                auto val_fn = [this, commandBuffer, attachment_index, fb_attachment, rectCount, clear_rect_copy](
                                  const CMD_BUFFER_STATE *prim_cb, const FRAMEBUFFER_STATE *fb) {
                    assert(rectCount == clear_rect_copy->size());
                    const auto &render_area = prim_cb->activeRenderPassBeginInfo.renderArea;
                    bool skip = false;
                    skip = ValidateClearAttachmentExtent(commandBuffer, attachment_index, fb, fb_attachment, render_area, rectCount,
                                                         clear_rect_copy->data(), prim_cb);
                    return skip;
                };
                cb_node->cmd_execute_commands_functions.emplace_back(val_fn);
            }
        }
    }
}

template <typename RegionType>
bool CoreChecks::ValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, CopyCommandVersion version) const {
    const auto *cb_node = GetCBState(commandBuffer);
    const auto *src_image_state = GetImageState(srcImage);
    const auto *dst_image_state = GetImageState(dstImage);
    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdResolveImage2KHR()" : "vkCmdResolveImage()";
    const CMD_TYPE cmd_type = is_2khr ? CMD_RESOLVEIMAGE : CMD_RESOLVEIMAGE2KHR;
    const char *vuid;

    bool skip = false;
    if (cb_node && src_image_state && dst_image_state) {
        vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-00256" : "VUID-vkCmdResolveImage-srcImage-00256";
        skip |= ValidateMemoryIsBoundToImage(src_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-00258" : "VUID-vkCmdResolveImage-dstImage-00258";
        skip |= ValidateMemoryIsBoundToImage(dst_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdResolveImage2KHR-commandBuffer-cmdpool" : "VUID-vkCmdResolveImage-commandBuffer-cmdpool";
        skip |= ValidateCmdQueueFlags(cb_node, func_name, VK_QUEUE_GRAPHICS_BIT, vuid);
        skip |= ValidateCmd(cb_node, cmd_type, func_name);
        vuid = is_2khr ? "VUID-vkCmdResolveImage2KHR-renderpass" : "VUID-vkCmdResolveImage-renderpass";
        skip |= InsideRenderPass(cb_node, func_name, vuid);
        vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-02003" : "VUID-vkCmdResolveImage-dstImage-02003";
        skip |= ValidateImageFormatFeatureFlags(dst_image_state, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdResolveImage2KHR-commandBuffer-01837" : "VUID-vkCmdResolveImage-commandBuffer-01837";
        skip |= ValidateProtectedImage(cb_node, src_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdResolveImage2KHR-commandBuffer-01838" : "VUID-vkCmdResolveImage-commandBuffer-01838";
        skip |= ValidateProtectedImage(cb_node, dst_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdResolveImage2KHR-commandBuffer-01839" : "VUID-vkCmdResolveImage-commandBuffer-01839";
        skip |= ValidateUnprotectedImage(cb_node, dst_image_state, func_name, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: srcImage must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }
        if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: dstImage must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }

        bool hit_error = false;
        const char *invalid_src_layout_vuid =
            is_2khr ? ((src_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-VkResolveImageInfo2KHR-srcImageLayout-01400"
                           : "VUID-VkResolveImageInfo2KHR-srcImageLayout-00261")
                    : ((src_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-vkCmdResolveImage-srcImageLayout-01400"
                           : "VUID-vkCmdResolveImage-srcImageLayout-00261");
        const char *invalid_dst_layout_vuid =
            is_2khr ? ((dst_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-VkResolveImageInfo2KHR-dstImageLayout-01401"
                           : "VUID-VkResolveImageInfo2KHR-dstImageLayout-00263")
                    : ((dst_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-vkCmdResolveImage-dstImageLayout-01401"
                           : "VUID-vkCmdResolveImage-dstImageLayout-00263");
        // For each region, the number of layers in the image subresource should not be zero
        // For each region, src and dest image aspect must be color only
        for (uint32_t i = 0; i < regionCount; i++) {
            const RegionType region = pRegions[i];
            const VkImageSubresourceLayers src_subresource = region.srcSubresource;
            const VkImageSubresourceLayers dst_subresource = region.dstSubresource;
            skip |= ValidateImageSubresourceLayers(cb_node, &src_subresource, func_name, "srcSubresource", i);
            skip |= ValidateImageSubresourceLayers(cb_node, &dst_subresource, func_name, "dstSubresource", i);
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImageLayout-00260" : "VUID-vkCmdResolveImage-srcImageLayout-00260";
            skip |= VerifyImageLayout(cb_node, src_image_state, src_subresource, srcImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, func_name, invalid_src_layout_vuid, vuid, &hit_error);
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImageLayout-00262" : "VUID-vkCmdResolveImage-dstImageLayout-00262";
            skip |= VerifyImageLayout(cb_node, dst_image_state, dst_subresource, dstImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, func_name, invalid_dst_layout_vuid, vuid, &hit_error);
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcSubresource-01709" : "VUID-vkCmdResolveImage-srcSubresource-01709";
            skip |= ValidateImageMipLevel(cb_node, src_image_state, src_subresource.mipLevel, i, func_name, "srcSubresource", vuid);
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstSubresource-01710" : "VUID-vkCmdResolveImage-dstSubresource-01710";
            skip |= ValidateImageMipLevel(cb_node, dst_image_state, dst_subresource.mipLevel, i, func_name, "dstSubresource", vuid);
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcSubresource-01711" : "VUID-vkCmdResolveImage-srcSubresource-01711";
            skip |= ValidateImageArrayLayerRange(cb_node, src_image_state, src_subresource.baseArrayLayer,
                                                 src_subresource.layerCount, i, func_name, "srcSubresource", vuid);
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstSubresource-01712" : "VUID-vkCmdResolveImage-dstSubresource-01712";
            skip |= ValidateImageArrayLayerRange(cb_node, dst_image_state, dst_subresource.baseArrayLayer,
                                                 dst_subresource.layerCount, i, func_name, "srcSubresource", vuid);

            // layer counts must match
            if (src_subresource.layerCount != dst_subresource.layerCount) {
                vuid = is_2khr ? "VUID-VkImageResolve2KHR-layerCount-00267" : "VUID-VkImageResolve-layerCount-00267";
                skip |=
                    LogError(cb_node->commandBuffer, vuid,
                             "%s: layerCount in source and destination subresource of pRegions[%u] does not match.", func_name, i);
            }
            // For each region, src and dest image aspect must be color only
            if ((src_subresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) ||
                (dst_subresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT)) {
                vuid = is_2khr ? "VUID-VkImageResolve2KHR-aspectMask-00266" : "VUID-VkImageResolve-aspectMask-00266";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: src and dest aspectMasks for pRegions[%u] must specify only VK_IMAGE_ASPECT_COLOR_BIT.",
                                 func_name, i);
            }

            const VkImageType src_image_type = src_image_state->createInfo.imageType;
            const VkImageType dst_image_type = dst_image_state->createInfo.imageType;

            if ((VK_IMAGE_TYPE_3D == src_image_type) || (VK_IMAGE_TYPE_3D == dst_image_type)) {
                if ((0 != src_subresource.baseArrayLayer) || (1 != src_subresource.layerCount)) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-04446" : "VUID-vkCmdResolveImage-srcImage-04446";
                    skip |= LogError(objlist, vuid,
                                     "%s: pRegions[%u] baseArrayLayer must be 0 and layerCount must be 1 for all "
                                     "subresources if the src or dst image is 3D.",
                                     func_name, i);
                }
                if ((0 != dst_subresource.baseArrayLayer) || (1 != dst_subresource.layerCount)) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-04447" : "VUID-vkCmdResolveImage-srcImage-04447";
                    skip |= LogError(objlist, vuid,
                                     "%s: pRegions[%u] baseArrayLayer must be 0 and layerCount must be 1 for all "
                                     "subresources if the src or dst image is 3D.",
                                     func_name, i);
                }
            }

            if (VK_IMAGE_TYPE_1D == src_image_type) {
                if ((pRegions[i].srcOffset.y != 0) || (pRegions[i].extent.height != 1)) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-00271" : "VUID-vkCmdResolveImage-srcImage-00271";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) is 1D but pRegions[%u] srcOffset.y (%d) is not 0 or "
                                     "extent.height (%u) is not 1.",
                                     func_name, report_data->FormatHandle(src_image_state->image).c_str(), i,
                                     pRegions[i].srcOffset.y, pRegions[i].extent.height);
                }
            }
            if ((VK_IMAGE_TYPE_1D == src_image_type) || (VK_IMAGE_TYPE_2D == src_image_type)) {
                if ((pRegions[i].srcOffset.z != 0) || (pRegions[i].extent.depth != 1)) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-00273" : "VUID-vkCmdResolveImage-srcImage-00273";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) is 2D but pRegions[%u] srcOffset.z (%d) is not 0 or "
                                     "extent.depth (%u) is not 1.",
                                     func_name, report_data->FormatHandle(src_image_state->image).c_str(), i,
                                     pRegions[i].srcOffset.z, pRegions[i].extent.depth);
                }
            }

            if (VK_IMAGE_TYPE_1D == dst_image_type) {
                if ((pRegions[i].dstOffset.y != 0) || (pRegions[i].extent.height != 1)) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-00276" : "VUID-vkCmdResolveImage-dstImage-00276";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) is 1D but pRegions[%u] dstOffset.y (%d) is not 0 or "
                                     "extent.height (%u) is not 1.",
                                     func_name, report_data->FormatHandle(dst_image_state->image).c_str(), i,
                                     pRegions[i].dstOffset.y, pRegions[i].extent.height);
                }
            }
            if ((VK_IMAGE_TYPE_1D == dst_image_type) || (VK_IMAGE_TYPE_2D == dst_image_type)) {
                if ((pRegions[i].dstOffset.z != 0) || (pRegions[i].extent.depth != 1)) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-00278" : "VUID-vkCmdResolveImage-dstImage-00278";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) is 2D but pRegions[%u] dstOffset.z (%d) is not 0 or "
                                     "extent.depth (%u) is not 1.",
                                     func_name, report_data->FormatHandle(dst_image_state->image).c_str(), i,
                                     pRegions[i].dstOffset.z, pRegions[i].extent.depth);
                }
            }

            // Each srcImage dimension offset + extent limits must fall with image subresource extent
            VkExtent3D subresource_extent = GetImageSubresourceExtent(src_image_state, &src_subresource);
            // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
            // developer
            if (src_subresource.mipLevel < src_image_state->createInfo.mipLevels) {
                uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &(region.extent), &subresource_extent);
                if ((extent_check & kXBit) != 0) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcOffset-00269" : "VUID-vkCmdResolveImage-srcOffset-00269";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) pRegions[%u] x-dimension offset [%1d] + extent [%u] "
                                     "exceeds subResource width [%u].",
                                     func_name, report_data->FormatHandle(src_image_state->image).c_str(), i, region.srcOffset.x,
                                     region.extent.width, subresource_extent.width);
                }

                if ((extent_check & kYBit) != 0) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcOffset-00270" : "VUID-vkCmdResolveImage-srcOffset-00270";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) pRegions[%u] y-dimension offset [%1d] + extent [%u] "
                                     "exceeds subResource height [%u].",
                                     func_name, report_data->FormatHandle(src_image_state->image).c_str(), i, region.srcOffset.y,
                                     region.extent.height, subresource_extent.height);
                }

                if ((extent_check & kZBit) != 0) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(src_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcOffset-00272" : "VUID-vkCmdResolveImage-srcOffset-00272";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) pRegions[%u] z-dimension offset [%1d] + extent [%u] "
                                     "exceeds subResource depth [%u].",
                                     func_name, report_data->FormatHandle(src_image_state->image).c_str(), i, region.srcOffset.z,
                                     region.extent.depth, subresource_extent.depth);
                }
            }

            // Each dstImage dimension offset + extent limits must fall with image subresource extent
            subresource_extent = GetImageSubresourceExtent(dst_image_state, &dst_subresource);
            // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
            // developer
            if (dst_subresource.mipLevel < dst_image_state->createInfo.mipLevels) {
                uint32_t extent_check = ExceedsBounds(&(region.dstOffset), &(region.extent), &subresource_extent);
                if ((extent_check & kXBit) != 0) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstOffset-00274" : "VUID-vkCmdResolveImage-dstOffset-00274";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) pRegions[%u] x-dimension offset [%1d] + extent [%u] "
                                     "exceeds subResource width [%u].",
                                     func_name, report_data->FormatHandle(dst_image_state->image).c_str(), i, region.srcOffset.x,
                                     region.extent.width, subresource_extent.width);
                }

                if ((extent_check & kYBit) != 0) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstOffset-00275" : "VUID-vkCmdResolveImage-dstOffset-00275";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) pRegions[%u] y-dimension offset [%1d] + extent [%u] "
                                     "exceeds subResource height [%u].",
                                     func_name, report_data->FormatHandle(dst_image_state->image).c_str(), i, region.srcOffset.y,
                                     region.extent.height, subresource_extent.height);
                }

                if ((extent_check & kZBit) != 0) {
                    LogObjectList objlist(cb_node->commandBuffer);
                    objlist.add(dst_image_state->image);
                    vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstOffset-00277" : "VUID-vkCmdResolveImage-dstOffset-00277";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) pRegions[%u] z-dimension offset [%1d] + extent [%u] "
                                     "exceeds subResource depth [%u].",
                                     func_name, report_data->FormatHandle(dst_image_state->image).c_str(), i, region.srcOffset.z,
                                     region.extent.depth, subresource_extent.depth);
                }
            }
        }

        if (src_image_state->createInfo.format != dst_image_state->createInfo.format) {
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-01386" : "VUID-vkCmdResolveImage-srcImage-01386";
            skip |= LogError(cb_node->commandBuffer, vuid, "%s: srcImage format (%s) and dstImage format (%s) are not the same.",
                             func_name, string_VkFormat(src_image_state->createInfo.format),
                             string_VkFormat(dst_image_state->createInfo.format));
        }
        if (src_image_state->createInfo.imageType != dst_image_state->createInfo.imageType) {
            skip |= LogWarning(cb_node->commandBuffer, kVUID_Core_DrawState_MismatchedImageType,
                               "%s: srcImage type (%s) and dstImage type (%s) are not the same.", func_name,
                               string_VkImageType(src_image_state->createInfo.imageType),
                               string_VkImageType(dst_image_state->createInfo.imageType));
        }
        if (src_image_state->createInfo.samples == VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-srcImage-00257" : "VUID-vkCmdResolveImage-srcImage-00257";
            skip |= LogError(cb_node->commandBuffer, vuid, "%s: srcImage sample count is VK_SAMPLE_COUNT_1_BIT.", func_name);
        }
        if (dst_image_state->createInfo.samples != VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2khr ? "VUID-VkResolveImageInfo2KHR-dstImage-00259" : "VUID-vkCmdResolveImage-dstImage-00259";
            skip |= LogError(cb_node->commandBuffer, vuid, "%s: dstImage sample count (%s) is not VK_SAMPLE_COUNT_1_BIT.",
                             func_name, string_VkSampleCountFlagBits(dst_image_state->createInfo.samples));
        }
    } else {
        assert(0);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageResolve *pRegions) const {
    return ValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                   COPY_COMMAND_VERSION_1);
}

bool CoreChecks::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkResolveImageInfo2KHR *pResolveImageInfo) const {
    return ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->srcImageLayout,
                                   pResolveImageInfo->dstImage, pResolveImageInfo->dstImageLayout, pResolveImageInfo->regionCount,
                                   pResolveImageInfo->pRegions, COPY_COMMAND_VERSION_2);
}

template <typename RegionType>
bool CoreChecks::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, VkFilter filter, CopyCommandVersion version) const {
    const auto *cb_node = GetCBState(commandBuffer);
    const auto *src_image_state = GetImageState(srcImage);
    const auto *dst_image_state = GetImageState(dstImage);

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdBlitImage2KHR()" : "vkCmdBlitImage()";
    const CMD_TYPE cmd_type = is_2khr ? CMD_BLITIMAGE : CMD_BLITIMAGE2KHR;

    bool skip = false;
    if (cb_node) {
        skip |= ValidateCmd(cb_node, cmd_type, func_name);
    }
    if (cb_node && src_image_state && dst_image_state) {
        const char *vuid;
        const char *location;
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00233" : "VUID-vkCmdBlitImage-srcImage-00233";
        location = is_2khr ? "vkCmdBlitImage2KHR(): pBlitImageInfo->srcImage" : "vkCmdBlitImage(): srcImage";
        skip |= ValidateImageSampleCount(src_image_state, VK_SAMPLE_COUNT_1_BIT, location, vuid);
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-00234" : "VUID-vkCmdBlitImage-dstImage-00234";
        location = is_2khr ? "vkCmdBlitImage2KHR(): pBlitImageInfo->dstImage" : "vkCmdBlitImage(): dstImage";
        skip |= ValidateImageSampleCount(dst_image_state, VK_SAMPLE_COUNT_1_BIT, location, vuid);
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00220" : "VUID-vkCmdBlitImage-srcImage-00220";
        skip |= ValidateMemoryIsBoundToImage(src_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-00225" : "VUID-vkCmdBlitImage-dstImage-00225";
        skip |= ValidateMemoryIsBoundToImage(dst_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00219" : "VUID-vkCmdBlitImage-srcImage-00219";
        skip |= ValidateImageUsageFlags(src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name,
                                        "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-00224" : "VUID-vkCmdBlitImage-dstImage-00224";
        skip |= ValidateImageUsageFlags(dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, func_name,
                                        "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
        vuid = is_2khr ? "VUID-vkCmdBlitImage2KHR-commandBuffer-cmdpool" : "VUID-vkCmdBlitImage-commandBuffer-cmdpool";
        skip |= ValidateCmdQueueFlags(cb_node, func_name, VK_QUEUE_GRAPHICS_BIT, vuid);
        skip |= ValidateCmd(cb_node, cmd_type, func_name);
        vuid = is_2khr ? "VUID-vkCmdBlitImage2KHR-renderpass" : "VUID-vkCmdBlitImage-renderpass";
        skip |= InsideRenderPass(cb_node, func_name, vuid);
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-01999" : "VUID-vkCmdBlitImage-srcImage-01999";
        skip |= ValidateImageFormatFeatureFlags(src_image_state, VK_FORMAT_FEATURE_BLIT_SRC_BIT, func_name, vuid);
        vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-02000" : "VUID-vkCmdBlitImage-dstImage-02000";
        skip |= ValidateImageFormatFeatureFlags(dst_image_state, VK_FORMAT_FEATURE_BLIT_DST_BIT, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdBlitImage2KHR-commandBuffer-01834" : "VUID-vkCmdBlitImage-commandBuffer-01834";
        skip |= ValidateProtectedImage(cb_node, src_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdBlitImage2KHR-commandBuffer-01835" : "VUID-vkCmdBlitImage-commandBuffer-01835";
        skip |= ValidateProtectedImage(cb_node, dst_image_state, func_name, vuid);
        vuid = is_2khr ? "VUID-vkCmdBlitImage2KHR-commandBuffer-01836" : "VUID-vkCmdBlitImage-commandBuffer-01836";
        skip |= ValidateUnprotectedImage(cb_node, dst_image_state, func_name, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: srcImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }
        if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: dstImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }

        // TODO: Need to validate image layouts, which will include layout validation for shared presentable images

        VkFormat src_format = src_image_state->createInfo.format;
        VkFormat dst_format = dst_image_state->createInfo.format;
        VkImageType src_type = src_image_state->createInfo.imageType;
        VkImageType dst_type = dst_image_state->createInfo.imageType;

        if (VK_FILTER_LINEAR == filter) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-filter-02001" : "VUID-vkCmdBlitImage-filter-02001";
            skip |= ValidateImageFormatFeatureFlags(src_image_state, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, func_name,
                                                    vuid);
        } else if (VK_FILTER_CUBIC_IMG == filter) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-filter-02002" : "VUID-vkCmdBlitImage-filter-02002";
            skip |= ValidateImageFormatFeatureFlags(src_image_state, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG,
                                                    func_name, vuid);
        }

        if (FormatRequiresYcbcrConversion(src_format)) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-01561" : "VUID-vkCmdBlitImage-srcImage-01561";
            skip |= LogError(device, vuid,
                             "%s: srcImage format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             func_name, string_VkFormat(src_format));
        }

        if (FormatRequiresYcbcrConversion(dst_format)) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-01562" : "VUID-vkCmdBlitImage-dstImage-01562";
            skip |= LogError(device, vuid,
                             "%s: dstImage format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             func_name, string_VkFormat(dst_format));
        }

        if ((VK_FILTER_CUBIC_IMG == filter) && (VK_IMAGE_TYPE_3D != src_type)) {
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-filter-00237" : "VUID-vkCmdBlitImage-filter-00237";
            skip |= LogError(cb_node->commandBuffer, vuid,
                             "%s: source image type must be VK_IMAGE_TYPE_3D when cubic filtering is specified.", func_name);
        }

        // Validate consistency for unsigned formats
        if (FormatIsUInt(src_format) != FormatIsUInt(dst_format)) {
            std::stringstream ss;
            ss << func_name << ": If one of srcImage and dstImage images has unsigned integer format, "
               << "the other one must also have unsigned integer format.  "
               << "Source format is " << string_VkFormat(src_format) << " Destination format is " << string_VkFormat(dst_format);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00230" : "VUID-vkCmdBlitImage-srcImage-00230";
            skip |= LogError(cb_node->commandBuffer, vuid, "%s.", ss.str().c_str());
        }

        // Validate consistency for signed formats
        if (FormatIsSInt(src_format) != FormatIsSInt(dst_format)) {
            std::stringstream ss;
            ss << func_name << ": If one of srcImage and dstImage images has signed integer format, "
               << "the other one must also have signed integer format.  "
               << "Source format is " << string_VkFormat(src_format) << " Destination format is " << string_VkFormat(dst_format);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00229" : "VUID-vkCmdBlitImage-srcImage-00229";
            skip |= LogError(cb_node->commandBuffer, vuid, "%s.", ss.str().c_str());
        }

        // Validate filter for Depth/Stencil formats
        if (FormatIsDepthOrStencil(src_format) && (filter != VK_FILTER_NEAREST)) {
            std::stringstream ss;
            ss << func_name << ": If the format of srcImage is a depth, stencil, or depth stencil "
               << "then filter must be VK_FILTER_NEAREST.";
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00232" : "VUID-vkCmdBlitImage-srcImage-00232";
            skip |= LogError(cb_node->commandBuffer, vuid, "%s.", ss.str().c_str());
        }

        // Validate aspect bits and formats for depth/stencil images
        if (FormatIsDepthOrStencil(src_format) || FormatIsDepthOrStencil(dst_format)) {
            if (src_format != dst_format) {
                std::stringstream ss;
                ss << func_name << ": If one of srcImage and dstImage images has a format of depth, stencil or depth "
                   << "stencil, the other one must have exactly the same format.  "
                   << "Source format is " << string_VkFormat(src_format) << " Destination format is "
                   << string_VkFormat(dst_format);
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00231" : "VUID-vkCmdBlitImage-srcImage-00231";
                skip |= LogError(cb_node->commandBuffer, vuid, "%s.", ss.str().c_str());
            }
        }  // Depth or Stencil

        // Do per-region checks
        const char *invalid_src_layout_vuid =
            is_2khr ? ((src_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-VkBlitImageInfo2KHR-srcImageLayout-01398"
                           : "VUID-VkBlitImageInfo2KHR-srcImageLayout-00222")
                    : ((src_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-vkCmdBlitImage-srcImageLayout-01398"
                           : "VUID-vkCmdBlitImage-srcImageLayout-00222");
        const char *invalid_dst_layout_vuid =
            is_2khr ? ((dst_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-VkBlitImageInfo2KHR-dstImageLayout-01399"
                           : "VUID-VkBlitImageInfo2KHR-dstImageLayout-00227")
                    : ((dst_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                           ? "VUID-vkCmdBlitImage-dstImageLayout-01399"
                           : "VUID-vkCmdBlitImage-dstImageLayout-00227");
        for (uint32_t i = 0; i < regionCount; i++) {
            const RegionType rgn = pRegions[i];
            bool hit_error = false;
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImageLayout-00221" : "VUID-vkCmdBlitImage-srcImageLayout-00221";
            skip |= VerifyImageLayout(cb_node, src_image_state, rgn.srcSubresource, srcImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, func_name, invalid_src_layout_vuid, vuid, &hit_error);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImageLayout-00226" : "VUID-vkCmdBlitImage-dstImageLayout-00226";
            skip |= VerifyImageLayout(cb_node, dst_image_state, rgn.dstSubresource, dstImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, func_name, invalid_dst_layout_vuid, vuid, &hit_error);
            skip |= ValidateImageSubresourceLayers(cb_node, &rgn.srcSubresource, func_name, "srcSubresource", i);
            skip |= ValidateImageSubresourceLayers(cb_node, &rgn.dstSubresource, func_name, "dstSubresource", i);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcSubresource-01705" : "VUID-vkCmdBlitImage-srcSubresource-01705";
            skip |=
                ValidateImageMipLevel(cb_node, src_image_state, rgn.srcSubresource.mipLevel, i, func_name, "srcSubresource", vuid);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstSubresource-01706" : "VUID-vkCmdBlitImage-dstSubresource-01706";
            skip |=
                ValidateImageMipLevel(cb_node, dst_image_state, rgn.dstSubresource.mipLevel, i, func_name, "dstSubresource", vuid);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcSubresource-01707" : "VUID-vkCmdBlitImage-srcSubresource-01707";
            skip |= ValidateImageArrayLayerRange(cb_node, src_image_state, rgn.srcSubresource.baseArrayLayer,
                                                 rgn.srcSubresource.layerCount, i, func_name, "srcSubresource", vuid);
            vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstSubresource-01708" : "VUID-vkCmdBlitImage-dstSubresource-01708";
            skip |= ValidateImageArrayLayerRange(cb_node, dst_image_state, rgn.dstSubresource.baseArrayLayer,
                                                 rgn.dstSubresource.layerCount, i, func_name, "dstSubresource", vuid);
            // Warn for zero-sized regions
            if ((rgn.srcOffsets[0].x == rgn.srcOffsets[1].x) || (rgn.srcOffsets[0].y == rgn.srcOffsets[1].y) ||
                (rgn.srcOffsets[0].z == rgn.srcOffsets[1].z)) {
                std::stringstream ss;
                ss << func_name << ": pRegions[" << i << "].srcOffsets specify a zero-volume area.";
                skip |= LogWarning(cb_node->commandBuffer, kVUID_Core_DrawState_InvalidExtents, "%s", ss.str().c_str());
            }
            if ((rgn.dstOffsets[0].x == rgn.dstOffsets[1].x) || (rgn.dstOffsets[0].y == rgn.dstOffsets[1].y) ||
                (rgn.dstOffsets[0].z == rgn.dstOffsets[1].z)) {
                std::stringstream ss;
                ss << func_name << ": pRegions[" << i << "].dstOffsets specify a zero-volume area.";
                skip |= LogWarning(cb_node->commandBuffer, kVUID_Core_DrawState_InvalidExtents, "%s", ss.str().c_str());
            }

            // Check that src/dst layercounts match
            if (rgn.srcSubresource.layerCount != rgn.dstSubresource.layerCount) {
                vuid = is_2khr ? "VUID-VkImageBlit2KHR-layerCount-00239" : "VUID-VkImageBlit-layerCount-00239";
                skip |=
                    LogError(cb_node->commandBuffer, vuid,
                             "%s: layerCount in source and destination subresource of pRegions[%d] does not match.", func_name, i);
            }

            if (rgn.srcSubresource.aspectMask != rgn.dstSubresource.aspectMask) {
                vuid = is_2khr ? "VUID-VkImageBlit2KHR-aspectMask-00238" : "VUID-VkImageBlit-aspectMask-00238";
                skip |=
                    LogError(cb_node->commandBuffer, vuid, "%s: aspectMask members for pRegion[%d] do not match.", func_name, i);
            }

            if (!VerifyAspectsPresent(rgn.srcSubresource.aspectMask, src_format)) {
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-aspectMask-00241" : "VUID-vkCmdBlitImage-aspectMask-00241";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] source aspectMask (0x%x) specifies aspects not present in source "
                                 "image format %s.",
                                 func_name, i, rgn.srcSubresource.aspectMask, string_VkFormat(src_format));
            }

            if (!VerifyAspectsPresent(rgn.dstSubresource.aspectMask, dst_format)) {
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-aspectMask-00242" : "VUID-vkCmdBlitImage-aspectMask-00242";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] dest aspectMask (0x%x) specifies aspects not present in dest image format %s.",
                                 func_name, i, rgn.dstSubresource.aspectMask, string_VkFormat(dst_format));
            }

            // Validate source image offsets
            VkExtent3D src_extent = GetImageSubresourceExtent(src_image_state, &(rgn.srcSubresource));
            if (VK_IMAGE_TYPE_1D == src_type) {
                if ((0 != rgn.srcOffsets[0].y) || (1 != rgn.srcOffsets[1].y)) {
                    vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00245" : "VUID-vkCmdBlitImage-srcImage-00245";
                    skip |= LogError(cb_node->commandBuffer, vuid,
                                     "%s: region [%d], source image of type VK_IMAGE_TYPE_1D with srcOffset[].y values "
                                     "of (%1d, %1d). These must be (0, 1).",
                                     func_name, i, rgn.srcOffsets[0].y, rgn.srcOffsets[1].y);
                }
            }

            if ((VK_IMAGE_TYPE_1D == src_type) || (VK_IMAGE_TYPE_2D == src_type)) {
                if ((0 != rgn.srcOffsets[0].z) || (1 != rgn.srcOffsets[1].z)) {
                    vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00247" : "VUID-vkCmdBlitImage-srcImage-00247";
                    skip |= LogError(cb_node->commandBuffer, vuid,
                                     "%s: region [%d], source image of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D with "
                                     "srcOffset[].z values of (%1d, %1d). These must be (0, 1).",
                                     func_name, i, rgn.srcOffsets[0].z, rgn.srcOffsets[1].z);
                }
            }

            bool oob = false;
            if ((rgn.srcOffsets[0].x < 0) || (rgn.srcOffsets[0].x > static_cast<int32_t>(src_extent.width)) ||
                (rgn.srcOffsets[1].x < 0) || (rgn.srcOffsets[1].x > static_cast<int32_t>(src_extent.width))) {
                oob = true;
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcOffset-00243" : "VUID-vkCmdBlitImage-srcOffset-00243";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] srcOffset[].x values (%1d, %1d) exceed srcSubresource width extent (%1d).",
                                 func_name, i, rgn.srcOffsets[0].x, rgn.srcOffsets[1].x, src_extent.width);
            }
            if ((rgn.srcOffsets[0].y < 0) || (rgn.srcOffsets[0].y > static_cast<int32_t>(src_extent.height)) ||
                (rgn.srcOffsets[1].y < 0) || (rgn.srcOffsets[1].y > static_cast<int32_t>(src_extent.height))) {
                oob = true;
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcOffset-00244" : "VUID-vkCmdBlitImage-srcOffset-00244";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] srcOffset[].y values (%1d, %1d) exceed srcSubresource height extent (%1d).",
                                 func_name, i, rgn.srcOffsets[0].y, rgn.srcOffsets[1].y, src_extent.height);
            }
            if ((rgn.srcOffsets[0].z < 0) || (rgn.srcOffsets[0].z > static_cast<int32_t>(src_extent.depth)) ||
                (rgn.srcOffsets[1].z < 0) || (rgn.srcOffsets[1].z > static_cast<int32_t>(src_extent.depth))) {
                oob = true;
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcOffset-00246" : "VUID-vkCmdBlitImage-srcOffset-00246";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] srcOffset[].z values (%1d, %1d) exceed srcSubresource depth extent (%1d).",
                                 func_name, i, rgn.srcOffsets[0].z, rgn.srcOffsets[1].z, src_extent.depth);
            }
            if (oob) {
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-pRegions-00215" : "VUID-vkCmdBlitImage-pRegions-00215";
                skip |= LogError(cb_node->commandBuffer, vuid, "%s: region [%d] source image blit region exceeds image dimensions.",
                                 func_name, i);
            }

            // Validate dest image offsets
            VkExtent3D dst_extent = GetImageSubresourceExtent(dst_image_state, &(rgn.dstSubresource));
            if (VK_IMAGE_TYPE_1D == dst_type) {
                if ((0 != rgn.dstOffsets[0].y) || (1 != rgn.dstOffsets[1].y)) {
                    vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-00250" : "VUID-vkCmdBlitImage-dstImage-00250";
                    skip |= LogError(cb_node->commandBuffer, vuid,
                                     "%s: region [%d], dest image of type VK_IMAGE_TYPE_1D with dstOffset[].y values of "
                                     "(%1d, %1d). These must be (0, 1).",
                                     func_name, i, rgn.dstOffsets[0].y, rgn.dstOffsets[1].y);
                }
            }

            if ((VK_IMAGE_TYPE_1D == dst_type) || (VK_IMAGE_TYPE_2D == dst_type)) {
                if ((0 != rgn.dstOffsets[0].z) || (1 != rgn.dstOffsets[1].z)) {
                    vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstImage-00252" : "VUID-vkCmdBlitImage-dstImage-00252";
                    skip |= LogError(cb_node->commandBuffer, vuid,
                                     "%s: region [%d], dest image of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D with "
                                     "dstOffset[].z values of (%1d, %1d). These must be (0, 1).",
                                     func_name, i, rgn.dstOffsets[0].z, rgn.dstOffsets[1].z);
                }
            }

            oob = false;
            if ((rgn.dstOffsets[0].x < 0) || (rgn.dstOffsets[0].x > static_cast<int32_t>(dst_extent.width)) ||
                (rgn.dstOffsets[1].x < 0) || (rgn.dstOffsets[1].x > static_cast<int32_t>(dst_extent.width))) {
                oob = true;
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstOffset-00248" : "VUID-vkCmdBlitImage-dstOffset-00248";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] dstOffset[].x values (%1d, %1d) exceed dstSubresource width extent (%1d).",
                                 func_name, i, rgn.dstOffsets[0].x, rgn.dstOffsets[1].x, dst_extent.width);
            }
            if ((rgn.dstOffsets[0].y < 0) || (rgn.dstOffsets[0].y > static_cast<int32_t>(dst_extent.height)) ||
                (rgn.dstOffsets[1].y < 0) || (rgn.dstOffsets[1].y > static_cast<int32_t>(dst_extent.height))) {
                oob = true;
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstOffset-00249" : "VUID-vkCmdBlitImage-dstOffset-00249";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] dstOffset[].y values (%1d, %1d) exceed dstSubresource height extent (%1d).",
                                 func_name, i, rgn.dstOffsets[0].y, rgn.dstOffsets[1].y, dst_extent.height);
            }
            if ((rgn.dstOffsets[0].z < 0) || (rgn.dstOffsets[0].z > static_cast<int32_t>(dst_extent.depth)) ||
                (rgn.dstOffsets[1].z < 0) || (rgn.dstOffsets[1].z > static_cast<int32_t>(dst_extent.depth))) {
                oob = true;
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-dstOffset-00251" : "VUID-vkCmdBlitImage-dstOffset-00251";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] dstOffset[].z values (%1d, %1d) exceed dstSubresource depth extent (%1d).",
                                 func_name, i, rgn.dstOffsets[0].z, rgn.dstOffsets[1].z, dst_extent.depth);
            }
            if (oob) {
                vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-pRegions-00216" : "VUID-vkCmdBlitImage-pRegions-00216";
                skip |= LogError(cb_node->commandBuffer, vuid,
                                 "%s: region [%d] destination image blit region exceeds image dimensions.", func_name, i);
            }

            if ((VK_IMAGE_TYPE_3D == src_type) || (VK_IMAGE_TYPE_3D == dst_type)) {
                if ((0 != rgn.srcSubresource.baseArrayLayer) || (1 != rgn.srcSubresource.layerCount) ||
                    (0 != rgn.dstSubresource.baseArrayLayer) || (1 != rgn.dstSubresource.layerCount)) {
                    vuid = is_2khr ? "VUID-VkBlitImageInfo2KHR-srcImage-00240" : "VUID-vkCmdBlitImage-srcImage-00240";
                    skip |= LogError(cb_node->commandBuffer, vuid,
                                     "%s: region [%d] blit to/from a 3D image type with a non-zero baseArrayLayer, or a "
                                     "layerCount other than 1.",
                                     func_name, i);
                }
            }
        }  // per-region checks
    } else {
        assert(0);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageBlit *pRegions, VkFilter filter) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                COPY_COMMAND_VERSION_1);
}

bool CoreChecks::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, COPY_COMMAND_VERSION_2);
}

template <typename RegionType>
void CoreChecks::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                    VkFilter filter) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    auto dst_image_state = GetImageState(dstImage);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageInitialLayout(cb_node, *src_image_state, pRegions[i].srcSubresource, srcImageLayout);
        SetImageInitialLayout(cb_node, *dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
    }
}

void CoreChecks::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageBlit *pRegions, VkFilter filter) {
    StateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CoreChecks::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter);
}

GlobalImageLayoutRangeMap *GetLayoutRangeMap(GlobalImageLayoutMap *map, const IMAGE_STATE &image_state) {
    assert(map);
    // This approach allows for a single hash lookup or/create new
    auto &layout_map = (*map)[image_state.image];
    if (!layout_map) {
        layout_map.emplace(image_state.subresource_encoder.SubresourceCount());
    }
    return &layout_map;
}

const GlobalImageLayoutRangeMap *GetLayoutRangeMap(const GlobalImageLayoutMap &map, VkImage image) {
    auto it = map.find(image);
    if (it != map.end()) {
        return &it->second;
    }
    return nullptr;
}

// This validates that the initial layout specified in the command buffer for the IMAGE is the same as the global IMAGE layout
bool CoreChecks::ValidateCmdBufImageLayouts(const CMD_BUFFER_STATE *pCB, const GlobalImageLayoutMap &globalImageLayoutMap,
                                            GlobalImageLayoutMap *overlayLayoutMap_arg) const {
    if (disabled[image_layout_validation]) return false;
    bool skip = false;
    GlobalImageLayoutMap &overlay_layout_map = *overlayLayoutMap_arg;
    // Iterate over the layout maps for each referenced image
    GlobalImageLayoutRangeMap empty_map(1);
    for (const auto &layout_map_entry : pCB->image_layout_map) {
        const auto image = layout_map_entry.first;
        const auto *image_state = GetImageState(image);
        if (!image_state) continue;  // Can't check layouts of a dead image
        const auto &subres_map = layout_map_entry.second;
        const auto &initial_layout_map = subres_map->GetInitialLayoutMap();
        // Validate the initial_uses for each subresource referenced
        if (initial_layout_map.empty()) continue;

        auto *overlay_map = GetLayoutRangeMap(&overlay_layout_map, *image_state);
        const auto *global_map = GetLayoutRangeMap(globalImageLayoutMap, image);
        if (global_map == nullptr) {
            global_map = &empty_map;
        }

        // Note: don't know if it would matter
        // if (global_map->empty() && overlay_map->empty()) // skip this next loop...;

        auto pos = initial_layout_map.begin();
        const auto end = initial_layout_map.end();
        sparse_container::parallel_iterator<const ImageSubresourceLayoutMap::LayoutMap> current_layout(*overlay_map, *global_map,
                                                                                                       pos->first.begin);
        while (pos != end) {
            VkImageLayout initial_layout = pos->second;
            VkImageLayout image_layout = kInvalidLayout;
            if (current_layout->range.empty()) break;  // When we are past the end of data in overlay and global... stop looking
            if (current_layout->pos_A->valid) {        // pos_A denotes the overlay map in the parallel iterator
                image_layout = current_layout->pos_A->lower_bound->second;
            } else if (current_layout->pos_B->valid) {  // pos_B denotes the global map in the parallel iterator
                image_layout = current_layout->pos_B->lower_bound->second;
            }
            const auto intersected_range = pos->first & current_layout->range;
            if (initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                // TODO: Set memory invalid which is in mem_tracker currently
            } else if (image_layout != initial_layout) {
                // Need to look up the inital layout *state* to get a bit more information
                const auto *initial_layout_state = subres_map->GetSubresourceInitialLayoutState(pos->first.begin);
                assert(initial_layout_state);  // There's no way we should have an initial layout without matching state...
                bool matches = ImageLayoutMatches(initial_layout_state->aspect_mask, image_layout, initial_layout);
                if (!matches) {
                    // We can report all the errors for the intersected range directly
                    for (auto index : sparse_container::range_view<decltype(intersected_range)>(intersected_range)) {
                        const auto subresource = image_state->subresource_encoder.Decode(index);
                        skip |= LogError(
                            pCB->commandBuffer, kVUID_Core_DrawState_InvalidImageLayout,
                            "Submitted command buffer expects %s (subresource: aspectMask 0x%X array layer %u, mip level %u) "
                            "to be in layout %s--instead, current layout is %s.",
                            report_data->FormatHandle(image).c_str(), subresource.aspectMask, subresource.arrayLayer,
                            subresource.mipLevel, string_VkImageLayout(initial_layout), string_VkImageLayout(image_layout));
                    }
                }
            }
            if (pos->first.includes(intersected_range.end)) {
                current_layout.seek(intersected_range.end);
            } else {
                ++pos;
                if (pos != end) {
                    current_layout.seek(pos->first.begin);
                }
            }
        }

        // Update all layout set operations (which will be a subset of the initial_layouts)
        sparse_container::splice(overlay_map, subres_map->GetCurrentLayoutMap(), sparse_container::value_precedence::prefer_source);
    }

    return skip;
}

void CoreChecks::UpdateCmdBufImageLayouts(CMD_BUFFER_STATE *pCB) {
    for (const auto &layout_map_entry : pCB->image_layout_map) {
        const auto image = layout_map_entry.first;
        const auto &subres_map = layout_map_entry.second;
        const auto *image_state = GetImageState(image);
        if (!image_state) continue;  // Can't set layouts of a dead image
        auto *global_map = GetLayoutRangeMap(&imageLayoutMap, *image_state);
        sparse_container::splice(global_map, subres_map->GetCurrentLayoutMap(), sparse_container::value_precedence::prefer_source);
    }
}

// ValidateLayoutVsAttachmentDescription is a general function where we can validate various state associated with the
// VkAttachmentDescription structs that are used by the sub-passes of a renderpass. Initial check is to make sure that READ_ONLY
// layout attachments don't have CLEAR as their loadOp.
bool CoreChecks::ValidateLayoutVsAttachmentDescription(const debug_report_data *report_data, RenderPassCreateVersion rp_version,
                                                       const VkImageLayout first_layout, const uint32_t attachment,
                                                       const VkAttachmentDescription2 &attachment_description) const {
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);

    // Verify that initial loadOp on READ_ONLY attachments is not CLEAR
    // for both loadOp and stencilLoaOp rp2 has it in 1 VU while rp1 has it in 2 VU with half behind Maintenance2 extension
    // Each is VUID is below in following order: rp2 -> rp1 with Maintenance2 -> rp1 with no extenstion
    if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo2-pAttachments-02522",
                             "vkCreateRenderPass2(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && (device_extensions.vk_khr_maintenance2) &&
                   (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-01566",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                          (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-00836",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        }
    }

    // Same as above for loadOp, but for stencilLoadOp
    if (attachment_description.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo2-pAttachments-02523",
                             "vkCreateRenderPass2(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && (device_extensions.vk_khr_maintenance2) &&
                   (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-01567",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                          (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-02511",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        }
    }

    return skip;
}

// Helper function to validate correct usage bits set for buffers or images. Verify that (actual & desired) flags != 0 or, if strict
// is true, verify that (actual & desired) flags == desired
template <typename T1>
bool CoreChecks::ValidateUsageFlags(VkFlags actual, VkFlags desired, VkBool32 strict, const T1 object,
                                    const VulkanTypedHandle &typed_handle, const char *msgCode, char const *func_name,
                                    char const *usage_str) const {
    bool correct_usage = false;
    bool skip = false;
    const char *type_str = object_string[typed_handle.type];
    if (strict) {
        correct_usage = ((actual & desired) == desired);
    } else {
        correct_usage = ((actual & desired) != 0);
    }

    if (!correct_usage) {
        // All callers should have a valid VUID
        assert(msgCode != kVUIDUndefined);
        skip =
            LogError(object, msgCode, "Invalid usage flag for %s used by %s. In this case, %s should have %s set during creation.",
                     report_data->FormatHandle(typed_handle).c_str(), func_name, type_str, usage_str);
    }
    return skip;
}

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
bool CoreChecks::ValidateImageUsageFlags(IMAGE_STATE const *image_state, VkFlags desired, bool strict, const char *msgCode,
                                         char const *func_name, char const *usage_string) const {
    return ValidateUsageFlags(image_state->createInfo.usage, desired, strict, image_state->image,
                              VulkanTypedHandle(image_state->image, kVulkanObjectTypeImage), msgCode, func_name, usage_string);
}

bool CoreChecks::ValidateImageFormatFeatureFlags(IMAGE_STATE const *image_state, VkFormatFeatureFlags desired,
                                                 char const *func_name, const char *vuid) const {
    bool skip = false;
    const VkFormatFeatureFlags image_format_features = image_state->format_features;
    if ((image_format_features & desired) != desired) {
        // Same error, but more details if it was an AHB external format
        if (image_state->has_ahb_format == true) {
            skip |= LogError(image_state->image, vuid,
                             "In %s, VkFormatFeatureFlags (0x%08X) does not support required feature %s for the external format "
                             "found in VkAndroidHardwareBufferFormatPropertiesANDROID::formatFeatures used by %s.",
                             func_name, image_format_features, string_VkFormatFeatureFlags(desired).c_str(),
                             report_data->FormatHandle(image_state->image).c_str());
        } else {
            skip |= LogError(image_state->image, vuid,
                             "In %s, VkFormatFeatureFlags (0x%08X) does not support required feature %s for format %u used by %s "
                             "with tiling %s.",
                             func_name, image_format_features, string_VkFormatFeatureFlags(desired).c_str(),
                             image_state->createInfo.format, report_data->FormatHandle(image_state->image).c_str(),
                             string_VkImageTiling(image_state->createInfo.tiling));
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageSubresourceLayers(const CMD_BUFFER_STATE *cb_node, const VkImageSubresourceLayers *subresource_layers,
                                                char const *func_name, char const *member, uint32_t i) const {
    bool skip = false;
    const VkImageAspectFlags apsect_mask = subresource_layers->aspectMask;
    // layerCount must not be zero
    if (subresource_layers->layerCount == 0) {
        skip |= LogError(cb_node->commandBuffer, "VUID-VkImageSubresourceLayers-layerCount-01700",
                         "In %s, pRegions[%u].%s.layerCount must not be zero.", func_name, i, member);
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_METADATA_BIT
    if (apsect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
        skip |= LogError(cb_node->commandBuffer, "VUID-VkImageSubresourceLayers-aspectMask-00168",
                         "In %s, pRegions[%u].%s.aspectMask has VK_IMAGE_ASPECT_METADATA_BIT set.", func_name, i, member);
    }
    // if aspectMask contains COLOR, it must not contain either DEPTH or STENCIL
    if ((apsect_mask & VK_IMAGE_ASPECT_COLOR_BIT) && (apsect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
        skip |= LogError(cb_node->commandBuffer, "VUID-VkImageSubresourceLayers-aspectMask-00167",
                         "In %s, pRegions[%u].%s.aspectMask has VK_IMAGE_ASPECT_COLOR_BIT and either VK_IMAGE_ASPECT_DEPTH_BIT or "
                         "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                         func_name, i, member);
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT
    if (apsect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                       VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
        skip |= LogError(cb_node->commandBuffer, "VUID-VkImageSubresourceLayers-aspectMask-02247",
                         "In %s, pRegions[%u].%s.aspectMask has a VK_IMAGE_ASPECT_MEMORY_PLANE_*_BIT_EXT bit set.", func_name, i,
                         member);
    }
    return skip;
}

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
bool CoreChecks::ValidateBufferUsageFlags(BUFFER_STATE const *buffer_state, VkFlags desired, bool strict, const char *msgCode,
                                          char const *func_name, char const *usage_string) const {
    return ValidateUsageFlags(buffer_state->createInfo.usage, desired, strict, buffer_state->buffer,
                              VulkanTypedHandle(buffer_state->buffer, kVulkanObjectTypeBuffer), msgCode, func_name, usage_string);
}

bool CoreChecks::ValidateBufferViewRange(const BUFFER_STATE *buffer_state, const VkBufferViewCreateInfo *pCreateInfo,
                                         const VkPhysicalDeviceLimits *device_limits) const {
    bool skip = false;

    const VkDeviceSize &range = pCreateInfo->range;
    if (range != VK_WHOLE_SIZE) {
        // Range must be greater than 0
        if (range <= 0) {
            skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-range-00928",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range must be greater than 0.",
                             range);
        }
        // Range must be a multiple of the element size of format
        const uint32_t format_size = FormatElementSize(pCreateInfo->format);
        if (SafeModulo(range, format_size) != 0) {
            skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-range-00929",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range must be a multiple of the element size of the format "
                             "(%" PRIu32 ").",
                             range, format_size);
        }
        // Range divided by the element size of format must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements
        if (SafeDivision(range, format_size) > device_limits->maxTexelBufferElements) {
            skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-range-00930",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range divided by the element size of the format (%" PRIu32
                             ") must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             range, format_size, device_limits->maxTexelBufferElements);
        }
        // The sum of range and offset must be less than or equal to the size of buffer
        if (range + pCreateInfo->offset > buffer_state->createInfo.size) {
            skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-offset-00931",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, the sum of offset (%" PRIuLEAST64
                             ") and range must be less than or equal to the size of the buffer (%" PRIuLEAST64 ").",
                             range, pCreateInfo->offset, buffer_state->createInfo.size);
        }
    } else {
        const uint32_t format_size = FormatElementSize(pCreateInfo->format);

        // Size of buffer - offset, divided by the element size of format must be less than or equal to
        // VkPhysicalDeviceLimits::maxTexelBufferElements
        if (SafeDivision(buffer_state->createInfo.size - pCreateInfo->offset, format_size) >
            device_limits->maxTexelBufferElements) {
            skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-range-04059",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") equals VK_WHOLE_SIZE, the buffer's size (%" PRIuLEAST64 ") minus the offset (%" PRIuLEAST64
                             "), divided by the element size of the format (%" PRIu32
                             ") must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             range, buffer_state->createInfo.size, pCreateInfo->offset, format_size,
                             device_limits->maxTexelBufferElements);
        }
    }
    return skip;
}

bool CoreChecks::ValidateBufferViewBuffer(const BUFFER_STATE *buffer_state, const VkBufferViewCreateInfo *pCreateInfo) const {
    bool skip = false;
    const VkFormatProperties format_properties = GetPDFormatProperties(pCreateInfo->format);
    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-buffer-00933",
                         "vkCreateBufferView(): If buffer was created with `usage` containing "
                         "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, format (%s) must "
                         "be supported for uniform texel buffers",
                         string_VkFormat(pCreateInfo->format));
    }
    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-buffer-00934",
                         "vkCreateBufferView(): If buffer was created with `usage` containing "
                         "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, format (%s) must "
                         "be supported for storage texel buffers",
                         string_VkFormat(pCreateInfo->format));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) const {
    bool skip = false;

    // TODO: Add check for "VUID-vkCreateBuffer-flags-00911"        (sparse address space accounting)

    auto chained_devaddr_struct = LvlFindInChain<VkBufferDeviceAddressCreateInfoEXT>(pCreateInfo->pNext);
    if (chained_devaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_devaddr_struct->deviceAddress != 0) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-deviceAddress-02604",
                             "vkCreateBuffer(): Non-zero VkBufferDeviceAddressCreateInfoEXT::deviceAddress "
                             "requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.");
        }
    }

    auto chained_opaqueaddr_struct = LvlFindInChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
    if (chained_opaqueaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_opaqueaddr_struct->opaqueCaptureAddress != 0) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337",
                             "vkCreateBuffer(): Non-zero VkBufferOpaqueCaptureAddressCreateInfo::opaqueCaptureAddress"
                             "requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.");
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
        !enabled_features.core12.bufferDeviceAddressCaptureReplay &&
        !enabled_features.buffer_device_address_ext.bufferDeviceAddressCaptureReplay) {
        skip |= LogError(
            device, "VUID-VkBufferCreateInfo-flags-03338",
            "vkCreateBuffer(): the bufferDeviceAddressCaptureReplay device feature is disabled: Buffers cannot be created with "
            "the VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set.");
    }

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    "vkCreateBuffer", "pCreateInfo->pQueueFamilyIndices",
                                                    "VUID-VkBufferCreateInfo-sharingMode-01419");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.core11.protectedMemory == VK_FALSE) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-flags-01887",
                             "vkCreateBuffer(): the protectedMemory device feature is disabled: Buffers cannot be created with the "
                             "VK_BUFFER_CREATE_PROTECTED_BIT set.");
        }
        const VkBufferCreateFlags invalid_flags =
            VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
        if ((pCreateInfo->flags & invalid_flags) != 0) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-None-01888",
                             "vkCreateBuffer(): VK_BUFFER_CREATE_PROTECTED_BIT is set so no sparse create flags can be used at "
                             "same time (VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | "
                             "VK_BUFFER_CREATE_SPARSE_ALIASED_BIT).");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView) const {
    bool skip = false;
    const BUFFER_STATE *buffer_state = GetBufferState(pCreateInfo->buffer);
    // If this isn't a sparse buffer, it needs to have memory backing it at CreateBufferView time
    if (buffer_state) {
        skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCreateBufferView()", "VUID-VkBufferViewCreateInfo-buffer-00935");
        // In order to create a valid buffer view, the buffer must have been created with at least one of the following flags:
        // UNIFORM_TEXEL_BUFFER_BIT or STORAGE_TEXEL_BUFFER_BIT
        skip |= ValidateBufferUsageFlags(buffer_state,
                                         VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, false,
                                         "VUID-VkBufferViewCreateInfo-buffer-00932", "vkCreateBufferView()",
                                         "VK_BUFFER_USAGE_[STORAGE|UNIFORM]_TEXEL_BUFFER_BIT");

        // Buffer view offset must be less than the size of buffer
        if (pCreateInfo->offset >= buffer_state->createInfo.size) {
            skip |= LogError(buffer_state->buffer, "VUID-VkBufferViewCreateInfo-offset-00925",
                             "vkCreateBufferView(): VkBufferViewCreateInfo offset (%" PRIuLEAST64
                             ") must be less than the size of the buffer (%" PRIuLEAST64 ").",
                             pCreateInfo->offset, buffer_state->createInfo.size);
        }

        const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
        // Buffer view offset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment
        if ((pCreateInfo->offset % device_limits->minTexelBufferOffsetAlignment) != 0 &&
            !enabled_features.texel_buffer_alignment_features.texelBufferAlignment) {
            const char *vuid = device_extensions.vk_ext_texel_buffer_alignment ? "VUID-VkBufferViewCreateInfo-offset-02749"
                                                                               : "VUID-VkBufferViewCreateInfo-offset-00926";
            skip |= LogError(buffer_state->buffer, vuid,
                             "vkCreateBufferView(): VkBufferViewCreateInfo offset (%" PRIuLEAST64
                             ") must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment (%" PRIuLEAST64 ").",
                             pCreateInfo->offset, device_limits->minTexelBufferOffsetAlignment);
        }

        if (enabled_features.texel_buffer_alignment_features.texelBufferAlignment) {
            VkDeviceSize element_size = FormatElementSize(pCreateInfo->format);
            if ((element_size % 3) == 0) {
                element_size /= 3;
            }
            if (buffer_state->createInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
                VkDeviceSize alignment_requirement =
                    phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetAlignmentBytes;
                if (phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetSingleTexelAlignment) {
                    alignment_requirement = std::min(alignment_requirement, element_size);
                }
                if (SafeModulo(pCreateInfo->offset, alignment_requirement) != 0) {
                    skip |= LogError(
                        buffer_state->buffer, "VUID-VkBufferViewCreateInfo-buffer-02750",
                        "vkCreateBufferView(): If buffer was created with usage containing "
                        "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "
                        "VkBufferViewCreateInfo offset (%" PRIuLEAST64
                        ") must be a multiple of the lesser of "
                        "VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::storageTexelBufferOffsetAlignmentBytes (%" PRIuLEAST64
                        ") or, if VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::storageTexelBufferOffsetSingleTexelAlignment "
                        "(%" PRId32
                        ") is VK_TRUE, the size of a texel of the requested format. "
                        "If the size of a texel is a multiple of three bytes, then the size of a "
                        "single component of format is used instead",
                        pCreateInfo->offset, phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetAlignmentBytes,
                        phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetSingleTexelAlignment);
                }
            }
            if (buffer_state->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
                VkDeviceSize alignment_requirement =
                    phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetAlignmentBytes;
                if (phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetSingleTexelAlignment) {
                    alignment_requirement = std::min(alignment_requirement, element_size);
                }
                if (SafeModulo(pCreateInfo->offset, alignment_requirement) != 0) {
                    skip |= LogError(
                        buffer_state->buffer, "VUID-VkBufferViewCreateInfo-buffer-02751",
                        "vkCreateBufferView(): If buffer was created with usage containing "
                        "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, "
                        "VkBufferViewCreateInfo offset (%" PRIuLEAST64
                        ") must be a multiple of the lesser of "
                        "VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::uniformTexelBufferOffsetAlignmentBytes (%" PRIuLEAST64
                        ") or, if VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::uniformTexelBufferOffsetSingleTexelAlignment "
                        "(%" PRId32
                        ") is VK_TRUE, the size of a texel of the requested format. "
                        "If the size of a texel is a multiple of three bytes, then the size of a "
                        "single component of format is used instead",
                        pCreateInfo->offset, phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetAlignmentBytes,
                        phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetSingleTexelAlignment);
                }
            }
        }

        skip |= ValidateBufferViewRange(buffer_state, pCreateInfo, device_limits);

        skip |= ValidateBufferViewBuffer(buffer_state, pCreateInfo);
    }
    return skip;
}

// For the given format verify that the aspect masks make sense
bool CoreChecks::ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, const char *func_name,
                                         const char *vuid) const {
    bool skip = false;
    const IMAGE_STATE *image_state = GetImageState(image);
    // checks color format and (single-plane or non-disjoint)
    // if ycbcr extension is not supported then single-plane and non-disjoint are always both true
    if ((FormatIsColor(format)) && ((FormatIsMultiplane(format) == false) || (image_state->disjoint == false))) {
        if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= LogError(
                image, vuid,
                "%s: Using format (%s) with aspect flags (%s) but color image formats must have the VK_IMAGE_ASPECT_COLOR_BIT set.",
                func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but color image formats must have ONLY the "
                             "VK_IMAGE_ASPECT_COLOR_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsDepthAndStencil(format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) == 0) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but depth/stencil image formats must have at least one "
                             "of VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but combination depth/stencil image formats can have "
                             "only the VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsDepthOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != VK_IMAGE_ASPECT_DEPTH_BIT) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but depth-only image formats must have the "
                             "VK_IMAGE_ASPECT_DEPTH_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but depth-only image formats can have only the "
                             "VK_IMAGE_ASPECT_DEPTH_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsStencilOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != VK_IMAGE_ASPECT_STENCIL_BIT) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but stencil-only image formats must have the "
                             "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        } else if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but stencil-only image formats can have only the "
                             "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    } else if (FormatIsMultiplane(format)) {
        VkImageAspectFlags valid_flags = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
        if (3 == FormatPlaneCount(format)) {
            valid_flags = valid_flags | VK_IMAGE_ASPECT_PLANE_2_BIT;
        }
        if ((aspect_mask & valid_flags) != aspect_mask) {
            skip |= LogError(image, vuid,
                             "%s: Using format (%s) with aspect flags (%s) but multi-plane image formats may have only "
                             "VK_IMAGE_ASPECT_COLOR_BIT or VK_IMAGE_ASPECT_PLANE_n_BITs set, where n = [0, 1, 2].",
                             func_name, string_VkFormat(format), string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                               const VkImageSubresourceRange &subresourceRange, const char *cmd_name,
                                               const char *param_name, const char *image_layer_count_var_name, const VkImage image,
                                               SubresourceRangeErrorCodes errorCodes) const {
    bool skip = false;

    // Validate mip levels
    if (subresourceRange.baseMipLevel >= image_mip_count) {
        skip |= LogError(image, errorCodes.base_mip_err,
                         "%s: %s.baseMipLevel (= %" PRIu32
                         ") is greater or equal to the mip level count of the image (i.e. greater or equal to %" PRIu32 ").",
                         cmd_name, param_name, subresourceRange.baseMipLevel, image_mip_count);
    }

    if (subresourceRange.levelCount != VK_REMAINING_MIP_LEVELS) {
        if (subresourceRange.levelCount == 0) {
            skip |=
                LogError(image, "VUID-VkImageSubresourceRange-levelCount-01720", "%s: %s.levelCount is 0.", cmd_name, param_name);
        } else {
            const uint64_t necessary_mip_count = uint64_t{subresourceRange.baseMipLevel} + uint64_t{subresourceRange.levelCount};

            if (necessary_mip_count > image_mip_count) {
                skip |= LogError(image, errorCodes.mip_count_err,
                                 "%s: %s.baseMipLevel + .levelCount (= %" PRIu32 " + %" PRIu32 " = %" PRIu64
                                 ") is greater than the mip level count of the image (i.e. greater than %" PRIu32 ").",
                                 cmd_name, param_name, subresourceRange.baseMipLevel, subresourceRange.levelCount,
                                 necessary_mip_count, image_mip_count);
            }
        }
    }

    // Validate array layers
    if (subresourceRange.baseArrayLayer >= image_layer_count) {
        skip |= LogError(image, errorCodes.base_layer_err,
                         "%s: %s.baseArrayLayer (= %" PRIu32
                         ") is greater or equal to the %s of the image when it was created (i.e. greater or equal to %" PRIu32 ").",
                         cmd_name, param_name, subresourceRange.baseArrayLayer, image_layer_count_var_name, image_layer_count);
    }

    if (subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
        if (subresourceRange.layerCount == 0) {
            skip |=
                LogError(image, "VUID-VkImageSubresourceRange-layerCount-01721", "%s: %s.layerCount is 0.", cmd_name, param_name);
        } else {
            const uint64_t necessary_layer_count =
                uint64_t{subresourceRange.baseArrayLayer} + uint64_t{subresourceRange.layerCount};

            if (necessary_layer_count > image_layer_count) {
                skip |= LogError(image, errorCodes.layer_count_err,
                                 "%s: %s.baseArrayLayer + .layerCount (= %" PRIu32 " + %" PRIu32 " = %" PRIu64
                                 ") is greater than the %s of the image when it was created (i.e. greater than %" PRIu32 ").",
                                 cmd_name, param_name, subresourceRange.baseArrayLayer, subresourceRange.layerCount,
                                 necessary_layer_count, image_layer_count_var_name, image_layer_count);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateCreateImageViewSubresourceRange(const IMAGE_STATE *image_state, bool is_imageview_2d_type,
                                                         const VkImageSubresourceRange &subresourceRange) const {
    bool is_khr_maintenance1 = IsExtEnabled(device_extensions.vk_khr_maintenance1);
    bool is_image_slicable = image_state->createInfo.imageType == VK_IMAGE_TYPE_3D &&
                             (image_state->createInfo.flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT);
    bool is_3_d_to_2_d_map = is_khr_maintenance1 && is_image_slicable && is_imageview_2d_type;

    uint32_t image_layer_count;

    if (is_3_d_to_2_d_map) {
        const auto layers = LayersFromRange(subresourceRange);
        const auto extent = GetImageSubresourceExtent(image_state, &layers);
        image_layer_count = extent.depth;
    } else {
        image_layer_count = image_state->createInfo.arrayLayers;
    }

    const auto image_layer_count_var_name = is_3_d_to_2_d_map ? "extent.depth" : "arrayLayers";

    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-VkImageViewCreateInfo-subresourceRange-01478";
    subresource_range_error_codes.mip_count_err = "VUID-VkImageViewCreateInfo-subresourceRange-01718";
    subresource_range_error_codes.base_layer_err =
        is_khr_maintenance1
            ? (is_3_d_to_2_d_map ? "VUID-VkImageViewCreateInfo-image-02724" : "VUID-VkImageViewCreateInfo-image-01482")
            : "VUID-VkImageViewCreateInfo-subresourceRange-01480";
    subresource_range_error_codes.layer_count_err = is_khr_maintenance1
                                                        ? (is_3_d_to_2_d_map ? "VUID-VkImageViewCreateInfo-subresourceRange-02725"
                                                                             : "VUID-VkImageViewCreateInfo-subresourceRange-01483")
                                                        : "VUID-VkImageViewCreateInfo-subresourceRange-01719";

    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_layer_count, subresourceRange,
                                         "vkCreateImageView", "pCreateInfo->subresourceRange", image_layer_count_var_name,
                                         image_state->image, subresource_range_error_codes);
}

bool CoreChecks::ValidateCmdClearColorSubresourceRange(const IMAGE_STATE *image_state,
                                                       const VkImageSubresourceRange &subresourceRange,
                                                       const char *param_name) const {
    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-vkCmdClearColorImage-baseMipLevel-01470";
    subresource_range_error_codes.mip_count_err = "VUID-vkCmdClearColorImage-pRanges-01692";
    subresource_range_error_codes.base_layer_err = "VUID-vkCmdClearColorImage-baseArrayLayer-01472";
    subresource_range_error_codes.layer_count_err = "VUID-vkCmdClearColorImage-pRanges-01693";

    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers, subresourceRange,
                                         "vkCmdClearColorImage", param_name, "arrayLayers", image_state->image,
                                         subresource_range_error_codes);
}

bool CoreChecks::ValidateCmdClearDepthSubresourceRange(const IMAGE_STATE *image_state,
                                                       const VkImageSubresourceRange &subresourceRange,
                                                       const char *param_name) const {
    SubresourceRangeErrorCodes subresource_range_error_codes = {};
    subresource_range_error_codes.base_mip_err = "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474";
    subresource_range_error_codes.mip_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01694";
    subresource_range_error_codes.base_layer_err = "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476";
    subresource_range_error_codes.layer_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01695";

    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers, subresourceRange,
                                         "vkCmdClearDepthStencilImage", param_name, "arrayLayers", image_state->image,
                                         subresource_range_error_codes);
}

bool CoreChecks::ValidateImageBarrierSubresourceRange(const CoreErrorLocation &loc, const IMAGE_STATE *image_state,
                                                      const VkImageSubresourceRange &subresourceRange) const {
    return ValidateImageSubresourceRange(image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers, subresourceRange,
                                         loc.StringFuncName().c_str(), loc.StringField().c_str(), "arrayLayers", image_state->image,
                                         sync_vuid_maps::GetSubResourceVUIDs(loc));
}

namespace barrier_queue_families {
using sync_vuid_maps::GetBarrierQueueVUID;
using sync_vuid_maps::kQueueErrorSummary;
using sync_vuid_maps::QueueError;

class ValidatorState {
  public:
    ValidatorState(const ValidationStateTracker *device_data, LogObjectList &&obj, const CoreErrorLocation &location,
                   const VulkanTypedHandle &barrier_handle, const VkSharingMode sharing_mode)
        : device_data_(device_data),
          objects_(std::move(obj)),
          loc_(location),
          barrier_handle_(barrier_handle),
          sharing_mode_(sharing_mode),
          limit_(static_cast<uint32_t>(device_data->physical_device_state->queue_family_properties.size())),
          mem_ext_(IsExtEnabled(device_data->device_extensions.vk_khr_external_memory)) {}

    // Log the messages using boilerplate from object state, and Vu specific information from the template arg
    // One and two family versions, in the single family version, Vu holds the name of the passed parameter
    bool LogMsg(QueueError vu_index, uint32_t family, const char *param_name) const {
        const std::string val_code = GetBarrierQueueVUID(loc_, vu_index);
        const char *annotation = GetFamilyAnnotation(family);
        return device_data_->LogError(objects_, val_code, "%s Barrier using %s %s created with sharingMode %s, has %s %u%s. %s",
                                      loc_.Message().c_str(), GetTypeString(),
                                      device_data_->report_data->FormatHandle(barrier_handle_).c_str(), GetModeString(), param_name,
                                      family, annotation, kQueueErrorSummary.at(vu_index).c_str());
    }

    bool LogMsg(QueueError vu_index, uint32_t src_family, uint32_t dst_family) const {
        const std::string val_code = GetBarrierQueueVUID(loc_, vu_index);
        const char *src_annotation = GetFamilyAnnotation(src_family);
        const char *dst_annotation = GetFamilyAnnotation(dst_family);
        return device_data_->LogError(
            objects_, val_code,
            "%s Barrier using %s %s created with sharingMode %s, has srcQueueFamilyIndex %u%s and dstQueueFamilyIndex %u%s. %s",
            loc_.Message().c_str(), GetTypeString(), device_data_->report_data->FormatHandle(barrier_handle_).c_str(),
            GetModeString(), src_family, src_annotation, dst_family, dst_annotation, kQueueErrorSummary.at(vu_index).c_str());
    }

    // This abstract Vu can only be tested at submit time, thus we need a callback from the closure containing the needed
    // data. Note that the mem_barrier is copied to the closure as the lambda lifespan exceed the guarantees of validity for
    // application input.
    static bool ValidateAtQueueSubmit(const QUEUE_STATE *queue_state, const ValidationStateTracker *device_data,
                                      uint32_t src_family, uint32_t dst_family, const ValidatorState &val) {
        auto error_code = QueueError::kSubmitQueueMustMatchSrcOrDst;
        uint32_t queue_family = queue_state->queueFamilyIndex;
        if ((src_family != queue_family) && (dst_family != queue_family)) {
            const std::string val_code = GetBarrierQueueVUID(val.loc_, error_code);
            const char *src_annotation = val.GetFamilyAnnotation(src_family);
            const char *dst_annotation = val.GetFamilyAnnotation(dst_family);
            return device_data->LogError(
                queue_state->queue, val_code,
                "%s Barrier submitted to queue with family index %u, using %s %s created with sharingMode %s, has "
                "srcQueueFamilyIndex %u%s and dstQueueFamilyIndex %u%s. %s",
                val.loc_.Message().c_str(), queue_family, val.GetTypeString(),
                device_data->report_data->FormatHandle(val.barrier_handle_).c_str(), val.GetModeString(), src_family,
                src_annotation, dst_family, dst_annotation, kQueueErrorSummary.at(error_code).c_str());
        }
        return false;
    }
    // Logical helpers for semantic clarity
    inline bool KhrExternalMem() const { return mem_ext_; }
    inline bool IsValid(uint32_t queue_family) const { return (queue_family < limit_); }
    inline bool IsValidOrSpecial(uint32_t queue_family) const {
        return IsValid(queue_family) || (mem_ext_ && QueueFamilyIsExternal(queue_family));
    }

    // Helpers for LogMsg
    const char *GetModeString() const { return string_VkSharingMode(sharing_mode_); }

    // Descriptive text for the various types of queue family index
    const char *GetFamilyAnnotation(uint32_t family) const {
        const char *external = " (VK_QUEUE_FAMILY_EXTERNAL)";
        const char *foreign = " (VK_QUEUE_FAMILY_FOREIGN_EXT)";
        const char *ignored = " (VK_QUEUE_FAMILY_IGNORED)";
        const char *valid = " (VALID)";
        const char *invalid = " (INVALID)";
        switch (family) {
            case VK_QUEUE_FAMILY_EXTERNAL:
                return external;
            case VK_QUEUE_FAMILY_FOREIGN_EXT:
                return foreign;
            case VK_QUEUE_FAMILY_IGNORED:
                return ignored;
            default:
                if (IsValid(family)) {
                    return valid;
                }
                return invalid;
        };
    }
    const char *GetTypeString() const { return object_string[barrier_handle_.type]; }
    VkSharingMode GetSharingMode() const { return sharing_mode_; }

  protected:
    const ValidationStateTracker *device_data_;
    const LogObjectList objects_;
    const CoreErrorLocation loc_;
    const VulkanTypedHandle barrier_handle_;
    const VkSharingMode sharing_mode_;
    const uint32_t limit_;
    const bool mem_ext_;
};

bool Validate(const CoreChecks *device_data, const CMD_BUFFER_STATE *cb_state, const ValidatorState &val,
              const uint32_t src_queue_family, const uint32_t dst_queue_family) {
    bool skip = false;

    const bool mode_concurrent = val.GetSharingMode() == VK_SHARING_MODE_CONCURRENT;
    const bool src_ignored = QueueFamilyIsIgnored(src_queue_family);
    const bool dst_ignored = QueueFamilyIsIgnored(dst_queue_family);
    if (val.KhrExternalMem()) {
        if (mode_concurrent) {
            bool sync2 = device_data->enabled_features.synchronization2_features.synchronization2 != 0;
            // this requirement is removed by VK_KHR_synchronization2
            if (!(src_ignored || dst_ignored) && !sync2) {
                skip |= val.LogMsg(QueueError::kSrcOrDstMustBeIgnore, src_queue_family, dst_queue_family);
            }
            if ((src_ignored && !(dst_ignored || QueueFamilyIsExternal(dst_queue_family))) ||
                (dst_ignored && !(src_ignored || QueueFamilyIsExternal(src_queue_family)))) {
                skip |= val.LogMsg(QueueError::kSpecialOrIgnoreOnly, src_queue_family, dst_queue_family);
            }
        } else {
            // VK_SHARING_MODE_EXCLUSIVE
            if (src_queue_family != dst_queue_family) {
                if (!val.IsValidOrSpecial(dst_queue_family)) {
                    skip |= val.LogMsg(QueueError::kSrcAndDstValidOrSpecial, dst_queue_family, "dstQueueFamilyIndex");
                }
                if (!val.IsValidOrSpecial(src_queue_family)) {
                    skip |= val.LogMsg(QueueError::kSrcAndDstValidOrSpecial, src_queue_family, "srcQueueFamilyIndex");
                }
            }
        }
    } else {
        // No memory extension
        if (mode_concurrent) {
            bool sync2 = device_data->enabled_features.synchronization2_features.synchronization2 != 0;
            // this requirement is removed by VK_KHR_synchronization2
            if ((!src_ignored || !dst_ignored) && !sync2) {
                skip |= val.LogMsg(QueueError::kSrcAndDestMustBeIgnore, src_queue_family, dst_queue_family);
            }
        } else {
            // VK_SHARING_MODE_EXCLUSIVE
            if ((src_queue_family != dst_queue_family) && !(val.IsValid(src_queue_family) && val.IsValid(dst_queue_family))) {
                skip |= val.LogMsg(QueueError::kSrcAndDstBothValid, src_queue_family, dst_queue_family);
            }
        }
    }
    return skip;
}
}  // namespace barrier_queue_families

bool CoreChecks::ValidateConcurrentBarrierAtSubmit(const CoreErrorLocation &loc, const ValidationStateTracker *state_data,
                                                   const QUEUE_STATE *queue_state, const CMD_BUFFER_STATE *cb_state,
                                                   const VulkanTypedHandle &typed_handle, uint32_t src_queue_family,
                                                   uint32_t dst_queue_family) {
    using barrier_queue_families::ValidatorState;
    ValidatorState val(state_data, LogObjectList(cb_state->commandBuffer), loc, typed_handle, VK_SHARING_MODE_CONCURRENT);
    return ValidatorState::ValidateAtQueueSubmit(queue_state, state_data, src_queue_family, dst_queue_family, val);
}

// Type specific wrapper for image barriers
template <typename ImgBarrier>
bool CoreChecks::ValidateBarrierQueueFamilies(const CoreErrorLocation &loc, const CMD_BUFFER_STATE *cb_state,
                                              const ImgBarrier &barrier, const IMAGE_STATE *state_data) const {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the image state
    barrier_queue_families::ValidatorState val(this, LogObjectList(cb_state->commandBuffer), loc,
                                               VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage),
                                               state_data->createInfo.sharingMode);
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, cb_state, val, src_queue_family, dst_queue_family);
}

// Type specific wrapper for buffer barriers
template <typename BufBarrier>
bool CoreChecks::ValidateBarrierQueueFamilies(const CoreErrorLocation &loc, const CMD_BUFFER_STATE *cb_state,
                                              const BufBarrier &barrier, const BUFFER_STATE *state_data) const {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the buffer state
    barrier_queue_families::ValidatorState val(this, LogObjectList(cb_state->commandBuffer), loc,
                                               VulkanTypedHandle(barrier.buffer, kVulkanObjectTypeBuffer),
                                               state_data->createInfo.sharingMode);
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, cb_state, val, src_queue_family, dst_queue_family);
}

template <typename Barrier>
bool CoreChecks::ValidateBufferBarrier(const LogObjectList &objects, const CoreErrorLocation &loc, const CMD_BUFFER_STATE *cb_state,
                                       const Barrier &mem_barrier) const {
    using sync_vuid_maps::BufferError;
    using sync_vuid_maps::GetBufferBarrierVUID;

    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(loc, cb_state, mem_barrier, cb_state->qfo_transfer_buffer_barriers);

    // Validate buffer barrier queue family indices
    auto buffer_state = GetBufferState(mem_barrier.buffer);
    if (buffer_state) {
        auto buf_loc = loc.dot(Field::buffer);
        const auto &mem_vuid = GetBufferBarrierVUID(buf_loc, BufferError::kNoMemory);
        skip |= ValidateMemoryIsBoundToBuffer(buffer_state, loc.StringFuncName().c_str(), mem_vuid.c_str());

        skip |= ValidateBarrierQueueFamilies(buf_loc, cb_state, mem_barrier, buffer_state);

        auto buffer_size = buffer_state->createInfo.size;
        if (mem_barrier.offset >= buffer_size) {
            auto offset_loc = loc.dot(Field::offset);
            const auto &vuid = GetBufferBarrierVUID(offset_loc, BufferError::kOffsetTooBig);

            skip |= LogError(objects, vuid, "%s %s has offset 0x%" PRIx64 " which is not less than total size 0x%" PRIx64 ".",
                             offset_loc.Message().c_str(), report_data->FormatHandle(mem_barrier.buffer).c_str(),
                             HandleToUint64(mem_barrier.offset), HandleToUint64(buffer_size));
        } else if (mem_barrier.size != VK_WHOLE_SIZE && (mem_barrier.offset + mem_barrier.size > buffer_size)) {
            auto size_loc = loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeOutOfRange);
            skip |= LogError(objects, vuid,
                             "%s %s has offset 0x%" PRIx64 " and size 0x%" PRIx64 " whose sum is greater than total size 0x%" PRIx64
                             ".",
                             size_loc.Message().c_str(), report_data->FormatHandle(mem_barrier.buffer).c_str(),
                             HandleToUint64(mem_barrier.offset), HandleToUint64(mem_barrier.size), HandleToUint64(buffer_size));
        }
        if (mem_barrier.size == 0) {
            auto size_loc = loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeZero);
            skip |= LogError(objects, vuid, "%s %s has a size of 0.", loc.Message().c_str(),
                             report_data->FormatHandle(mem_barrier.buffer).c_str());
        }
    }
    return skip;
}

template <typename Barrier>
bool CoreChecks::ValidateImageBarrier(const LogObjectList &objects, const CoreErrorLocation &loc, const CMD_BUFFER_STATE *cb_state,
                                      const Barrier &mem_barrier) const {
    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(loc, cb_state, mem_barrier, cb_state->qfo_transfer_image_barriers);

    bool is_ilt = true;
    if (enabled_features.synchronization2_features.synchronization2) {
        is_ilt = mem_barrier.oldLayout != mem_barrier.newLayout;
    }

    if (is_ilt) {
        if (mem_barrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED || mem_barrier.newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            auto layout_loc = loc.dot(Field::newLayout);
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(loc, sync_vuid_maps::ImageError::kBadLayout);
            skip |=
                LogError(cb_state->commandBuffer, vuid, "%s Image Layout cannot be transitioned to UNDEFINED or PREINITIALIZED.",
                         layout_loc.Message().c_str());
        }
    }

    auto image_data = GetImageState(mem_barrier.image);
    if (image_data) {
        auto image_loc = loc.dot(Field::image);

        skip |= ValidateMemoryIsBoundToImage(image_data, loc);

        skip |= ValidateBarrierQueueFamilies(image_loc, cb_state, mem_barrier, image_data);

        skip |= ValidateImageAspectMask(image_data->image, image_data->createInfo.format, mem_barrier.subresourceRange.aspectMask,
                                        loc.StringFuncName().c_str());

        skip |= ValidateImageBarrierSubresourceRange(loc.dot(Field::subresourceRange), image_data, mem_barrier.subresourceRange);
    }
    return skip;
}

bool CoreChecks::ValidateBarriers(const CoreErrorLocation &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                  VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                  uint32_t memBarrierCount, const VkMemoryBarrier *pMemBarriers, uint32_t bufferBarrierCount,
                                  const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                  const VkImageMemoryBarrier *pImageMemBarriers) const {
    bool skip = false;
    auto queue_flags = GetQueueFlags(*cb_state);
    LogObjectList objects(cb_state->commandBuffer);
    auto op_type =
        ComputeBarrierOperationsType(cb_state, bufferBarrierCount, pBufferMemBarriers, imageMemBarrierCount, pImageMemBarriers);

    // NOTE: for vkCmdPipelineBarrier() and vkCmdWaitEvents() the access mask checks use the function name for {refpage}
    for (uint32_t i = 0; i < memBarrierCount; ++i) {
        const auto &mem_barrier = pMemBarriers[i];
        CoreErrorLocation loc(outer_loc.func_name, outer_loc.refpage, Field::pMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, op_type, queue_flags, mem_barrier, src_stage_mask, dst_stage_mask);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; ++i) {
        const auto &mem_barrier = pImageMemBarriers[i];
        CoreErrorLocation loc(outer_loc.func_name, outer_loc.refpage, Field::pImageMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, op_type, queue_flags, mem_barrier, src_stage_mask, dst_stage_mask);

        loc.refpage = RefPage::VkImageMemoryBarrier;
        skip |= ValidateImageBarrier(objects, loc, cb_state, mem_barrier);
    }
    {
        CoreErrorLocation loc(outer_loc.func_name, RefPage::VkImageMemoryBarrier);
        skip |= ValidateBarriersToImages(loc, cb_state, imageMemBarrierCount, pImageMemBarriers);
    }
    for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
        const auto &mem_barrier = pBufferMemBarriers[i];
        CoreErrorLocation loc(outer_loc.func_name, outer_loc.refpage, Field::pBufferMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, op_type, queue_flags, mem_barrier, src_stage_mask, dst_stage_mask);

        loc.refpage = RefPage::VkBufferMemoryBarrier;
        skip |= ValidateBufferBarrier(objects, loc, cb_state, mem_barrier);
    }
    return skip;
}

bool CoreChecks::ValidateDependencyInfo(const LogObjectList &objects, const CoreErrorLocation &outer_loc,
                                        const CMD_BUFFER_STATE *cb_state, BarrierOperationsType op_type,
                                        const VkDependencyInfoKHR *dep_info) const {
    bool skip = false;

    if (cb_state->activeRenderPass) {
        skip |= ValidateRenderPassPipelineBarriers(outer_loc, cb_state, dep_info);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    }
    auto queue_flags = GetQueueFlags(*cb_state);
    for (uint32_t i = 0; i < dep_info->memoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pMemoryBarriers[i];
        CoreErrorLocation loc = outer_loc.dot(Field::pMemoryBarriers, i);
        loc.refpage = RefPage::VkMemoryBarrier2KHR;
        skip |= ValidateMemoryBarrier(objects, loc, op_type, queue_flags, mem_barrier);
    }
    for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pImageMemoryBarriers[i];
        CoreErrorLocation loc = outer_loc.dot(Field::pImageMemoryBarriers, i);
        loc.refpage = RefPage::VkImageMemoryBarrier2KHR;
        skip |= ValidateMemoryBarrier(objects, loc, op_type, queue_flags, mem_barrier);
        skip |= ValidateImageBarrier(objects, loc, cb_state, mem_barrier);
    }
    {
        CoreErrorLocation loc(outer_loc.func_name, RefPage::VkImageMemoryBarrier2KHR);
        skip |= ValidateBarriersToImages(loc, cb_state, dep_info->imageMemoryBarrierCount, dep_info->pImageMemoryBarriers);
    }

    for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pBufferMemoryBarriers[i];
        CoreErrorLocation loc = outer_loc.dot(Field::pBufferMemoryBarriers, i);
        loc.refpage = RefPage::VkBufferMemoryBarrier2KHR;
        skip |= ValidateMemoryBarrier(objects, loc, op_type, queue_flags, mem_barrier);
        skip |= ValidateBufferBarrier(objects, loc, cb_state, mem_barrier);
    }

    return skip;
}

template <typename Barrier>
bool CoreChecks::ValidateMemoryBarrier(const LogObjectList &objects, const CoreErrorLocation &loc, BarrierOperationsType op_type,
                                       VkQueueFlags queue_flags, const Barrier &barrier, VkPipelineStageFlags src_stage_mask,
                                       VkPipelineStageFlags dst_stage_mask) const {
    bool skip = false;
    if (op_type == kAllRelease || op_type == kGeneral) {
        skip |= ValidateAccessMask(objects, loc.dot(Field::srcAccessMask), queue_flags, barrier.srcAccessMask, src_stage_mask);
    }
    if (op_type == kAllAcquire || op_type == kGeneral) {
        skip |= ValidateAccessMask(objects, loc.dot(Field::dstAccessMask), queue_flags, barrier.dstAccessMask, dst_stage_mask);
    }
    return skip;
}

template <typename Barrier>
bool CoreChecks::ValidateMemoryBarrier(const LogObjectList &objects, const CoreErrorLocation &loc, BarrierOperationsType op_type,
                                       VkQueueFlags queue_flags, const Barrier &barrier) const {
    bool skip = false;
    if (op_type == kAllRelease || op_type == kGeneral) {
        skip |= ValidatePipelineStage(objects, loc.dot(Field::srcStageMask), queue_flags, barrier.srcStageMask);
        skip |=
            ValidateAccessMask(objects, loc.dot(Field::srcAccessMask), queue_flags, barrier.srcAccessMask, barrier.srcStageMask);
    }
    if (op_type == kAllAcquire || op_type == kGeneral) {
        skip |= ValidatePipelineStage(objects, loc.dot(Field::dstStageMask), queue_flags, barrier.dstStageMask);
        skip |=
            ValidateAccessMask(objects, loc.dot(Field::dstAccessMask), queue_flags, barrier.dstAccessMask, barrier.dstStageMask);
    }
    return skip;
}

bool CoreChecks::ValidateSubpassBarrier(const LogObjectList &objects, const CoreErrorLocation &loc, VkQueueFlags queue_flags,
                                        const VkSubpassDependency2 &barrier) const {
    bool skip = false;
    const auto *mem_barrier = LvlFindInChain<VkMemoryBarrier2KHR>(barrier.pNext);
    if (mem_barrier) {
        if (enabled_features.synchronization2_features.synchronization2) {
            if (barrier.srcAccessMask != 0) {
                skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcAccessMask",
                                 "%s is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                                 loc.dot(Field::srcAccessMask).Message().c_str());
            }
            if (barrier.dstAccessMask != 0) {
                skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstAccessMask",
                                 "%s dstAccessMask is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                                 loc.dot(Field::dstAccessMask).Message().c_str());
            }
            if (barrier.srcStageMask != 0 || barrier.dstStageMask != 0) {
                skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcStageMask",
                                 "%s srcStageMask is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                                 loc.dot(Field::srcStageMask).Message().c_str());
            }
            if (barrier.dstStageMask != 0) {
                skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstStageMask",
                                 "%s dstStageMask is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                                 loc.dot(Field::dstStageMask).Message().c_str());
            }

            skip |= CoreChecks::ValidateMemoryBarrier(objects, loc.dot(Field::pNext), kGeneral, queue_flags, *mem_barrier);
            return skip;
        } else {
            skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-pNext",
                             "%s a VkMemoryBarrier2KHR is present in pNext but synchronization2 is not enabled.",
                             loc.Message().c_str());
        }
    }
    skip |= CoreChecks::ValidateMemoryBarrier(objects, loc, kGeneral, queue_flags, barrier);
    return skip;
}

bool CoreChecks::ValidateImageViewFormatFeatures(const IMAGE_STATE *image_state, const VkFormat view_format,
                                                 const VkImageUsageFlags image_usage) const {
    // Pass in image_usage here instead of extracting it from image_state in case there's a chained VkImageViewUsageCreateInfo
    bool skip = false;

    VkFormatFeatureFlags tiling_features = VK_FORMAT_FEATURE_FLAG_BITS_MAX_ENUM;
    const VkImageTiling image_tiling = image_state->createInfo.tiling;

    if (image_state->has_ahb_format == true) {
        // AHB image view and image share same feature sets
        tiling_features = image_state->format_features;
    } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        // Parameter validation should catch if this is used without VK_EXT_image_drm_format_modifier
        assert(device_extensions.vk_ext_image_drm_format_modifier);
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = {VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT,
                                                                       nullptr};
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image_state->image, &drm_format_properties);

        VkFormatProperties2 format_properties_2 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, nullptr};
        VkDrmFormatModifierPropertiesListEXT drm_properties_list = {VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT,
                                                                    nullptr};
        format_properties_2.pNext = (void *)&drm_properties_list;
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, view_format, &format_properties_2);

        for (uint32_t i = 0; i < drm_properties_list.drmFormatModifierCount; i++) {
            if ((drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifier & drm_format_properties.drmFormatModifier) !=
                0) {
                tiling_features |= drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
            }
        }
    } else {
        VkFormatProperties format_properties = GetPDFormatProperties(view_format);
        tiling_features = (image_tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures
                                                                   : format_properties.optimalTilingFeatures;
    }

    if (tiling_features == 0) {
        skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-None-02273",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s has no supported format features on this "
                         "physical device.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(tiling_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-usage-02274",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_SAMPLED_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(tiling_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-usage-02275",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_STORAGE_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(tiling_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
        skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-usage-02276",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
               !(tiling_features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-usage-02277",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) &&
               !(tiling_features & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))) {
        skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-usage-02652",
                         "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                         "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT or VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                         string_VkFormat(view_format), string_VkImageTiling(image_tiling));
    } else if ((image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) &&
               !(tiling_features & VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
            skip |= LogError(image_state->image, "VUID-VkImageViewCreateInfo-usage-04550",
                             "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                             "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR.",
                             string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkImageView *pView) const {
    bool skip = false;
    const IMAGE_STATE *image_state = GetImageState(pCreateInfo->image);
    if (image_state) {
        skip |=
            ValidateImageUsageFlags(image_state,
                                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV |
                                        VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
                                    false, "VUID-VkImageViewCreateInfo-image-04441", "vkCreateImageView()",
                                    "VK_IMAGE_USAGE_[SAMPLED|STORAGE|COLOR_ATTACHMENT|DEPTH_STENCIL_ATTACHMENT|INPUT_ATTACHMENT|"
                                    "TRANSIENT_ATTACHMENT|SHADING_RATE_IMAGE|FRAGMENT_DENSITY_MAP]_BIT");
        // If this isn't a sparse image, it needs to have memory backing it at CreateImageView time
        skip |= ValidateMemoryIsBoundToImage(image_state, "vkCreateImageView()", "VUID-VkImageViewCreateInfo-image-01020");
        // Checks imported from image layer
        skip |= ValidateCreateImageViewSubresourceRange(
            image_state, pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D || pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            pCreateInfo->subresourceRange);

        VkImageCreateFlags image_flags = image_state->createInfo.flags;
        VkFormat image_format = image_state->createInfo.format;
        VkImageUsageFlags image_usage = image_state->createInfo.usage;
        VkFormat view_format = pCreateInfo->format;
        VkImageAspectFlags aspect_mask = pCreateInfo->subresourceRange.aspectMask;
        VkImageType image_type = image_state->createInfo.imageType;
        VkImageViewType view_type = pCreateInfo->viewType;

        // If there's a chained VkImageViewUsageCreateInfo struct, modify image_usage to match
        auto chained_ivuci_struct = LvlFindInChain<VkImageViewUsageCreateInfo>(pCreateInfo->pNext);
        if (chained_ivuci_struct) {
            if (device_extensions.vk_khr_maintenance2) {
                if (!device_extensions.vk_ext_separate_stencil_usage) {
                    if ((image_usage | chained_ivuci_struct->usage) != image_usage) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02661",
                                         "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo, usage must not "
                                         "include any bits that were not set in VkImageCreateInfo::usage used to create image");
                    }
                } else {
                    const auto image_stencil_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_state->createInfo.pNext);
                    if (image_stencil_struct == nullptr) {
                        if ((image_usage | chained_ivuci_struct->usage) != image_usage) {
                            skip |= LogError(
                                pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02662",
                                "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo and image was not created "
                                "with a VkImageStencilUsageCreateInfo in pNext of vkImageCreateInfo, usage must not include "
                                "any bits that were not set in VkImageCreateInfo::usage used to create image");
                        }
                    } else {
                        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT &&
                            (image_stencil_struct->stencilUsage | chained_ivuci_struct->usage) !=
                                image_stencil_struct->stencilUsage) {
                            skip |= LogError(
                                pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02663",
                                "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo, image was created with a "
                                "VkImageStencilUsageCreateInfo in pNext of vkImageCreateInfo, and subResourceRange.aspectMask "
                                "includes VK_IMAGE_ASPECT_STENCIL_BIT, VkImageViewUsageCreateInfo::usage must not include any "
                                "bits that were not set in VkImageStencilUsageCreateInfo::stencilUsage used to create image");
                        }
                        if ((aspect_mask & ~VK_IMAGE_ASPECT_STENCIL_BIT) != 0 &&
                            (image_usage | chained_ivuci_struct->usage) != image_usage) {
                            skip |= LogError(
                                pCreateInfo->image, "VUID-VkImageViewCreateInfo-pNext-02664",
                                "vkCreateImageView(): pNext chain includes VkImageViewUsageCreateInfo, image was created with a "
                                "VkImageStencilUsageCreateInfo in pNext of vkImageCreateInfo, and subResourceRange.aspectMask "
                                "includes bits other than VK_IMAGE_ASPECT_STENCIL_BIT, VkImageViewUsageCreateInfo::usage must not "
                                "include any bits that were not set in VkImageCreateInfo::usage used to create image");
                        }
                    }
                }
            }

            image_usage = chained_ivuci_struct->usage;
        }

        // Validate VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT state, if view/image formats differ
        if ((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (image_format != view_format)) {
            if (FormatIsMultiplane(image_format)) {
                VkFormat compat_format = FindMultiplaneCompatibleFormat(image_format, aspect_mask);
                if (view_format != compat_format) {
                    // View format must match the multiplane compatible format
                    std::stringstream ss;
                    ss << "vkCreateImageView(): ImageView format " << string_VkFormat(view_format)
                       << " is not compatible with plane " << GetPlaneIndex(aspect_mask) << " of underlying image format "
                       << string_VkFormat(image_format) << ", must be " << string_VkFormat(compat_format) << ".";
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-01586", "%s", ss.str().c_str());
                }
            } else {
                if (!(image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT)) {
                    // Format MUST be compatible (in the same format compatibility class) as the format the image was created with
                    if (FormatCompatibilityClass(image_format) != FormatCompatibilityClass(view_format)) {
                        const char *error_vuid;
                        if ((!device_extensions.vk_khr_maintenance2) && (!device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                            error_vuid = "VUID-VkImageViewCreateInfo-image-01018";
                        } else if ((device_extensions.vk_khr_maintenance2) &&
                                   (!device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                            error_vuid = "VUID-VkImageViewCreateInfo-image-01759";
                        } else if ((!device_extensions.vk_khr_maintenance2) &&
                                   (device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                            error_vuid = "VUID-VkImageViewCreateInfo-image-01760";
                        } else {
                            // both enabled
                            error_vuid = "VUID-VkImageViewCreateInfo-image-01761";
                        }
                        std::stringstream ss;
                        ss << "vkCreateImageView(): ImageView format " << string_VkFormat(view_format)
                           << " is not in the same format compatibility class as "
                           << report_data->FormatHandle(pCreateInfo->image).c_str() << "  format " << string_VkFormat(image_format)
                           << ".  Images created with the VK_IMAGE_CREATE_MUTABLE_FORMAT BIT "
                           << "can support ImageViews with differing formats but they must be in the same compatibility class.";
                        skip |= LogError(pCreateInfo->image, error_vuid, "%s", ss.str().c_str());
                    }
                }
            }
        } else {
            // Format MUST be IDENTICAL to the format the image was created with
            // Unless it is a multi-planar color bit aspect
            if ((image_format != view_format) &&
                ((FormatIsMultiplane(image_format) == false) || (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT))) {
                const char *vuid = (device_extensions.vk_khr_sampler_ycbcr_conversion) ? "VUID-VkImageViewCreateInfo-image-01762"
                                                                                       : "VUID-VkImageViewCreateInfo-image-01019";
                std::stringstream ss;
                ss << "vkCreateImageView() format " << string_VkFormat(view_format) << " differs from "
                   << report_data->FormatHandle(pCreateInfo->image).c_str() << " format " << string_VkFormat(image_format)
                   << ".  Formats MUST be IDENTICAL unless VK_IMAGE_CREATE_MUTABLE_FORMAT BIT was set on image creation.";
                skip |= LogError(pCreateInfo->image, vuid, "%s", ss.str().c_str());
            }
        }

        // Validate correct image aspect bits for desired formats and format consistency
        skip |= ValidateImageAspectMask(image_state->image, image_format, aspect_mask, "vkCreateImageView()");

        switch (image_type) {
            case VK_IMAGE_TYPE_1D:
                if (view_type != VK_IMAGE_VIEW_TYPE_1D && view_type != VK_IMAGE_VIEW_TYPE_1D_ARRAY) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                     "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                     string_VkImageViewType(view_type), string_VkImageType(image_type));
                }
                break;
            case VK_IMAGE_TYPE_2D:
                if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                    if ((view_type == VK_IMAGE_VIEW_TYPE_CUBE || view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) &&
                        !(image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-01003",
                                         "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    } else if (view_type != VK_IMAGE_VIEW_TYPE_CUBE && view_type != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                         "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    }
                }
                break;
            case VK_IMAGE_TYPE_3D:
                if (device_extensions.vk_khr_maintenance1) {
                    if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                        if ((view_type == VK_IMAGE_VIEW_TYPE_2D || view_type == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
                            if (!(image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT)) {
                                skip |=
                                    LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-01005",
                                             "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                             string_VkImageViewType(view_type), string_VkImageType(image_type));
                            } else if ((image_flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT |
                                                       VK_IMAGE_CREATE_SPARSE_ALIASED_BIT))) {
                                skip |= LogError(
                                    pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                    "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s "
                                    "when the VK_IMAGE_CREATE_SPARSE_BINDING_BIT, VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or "
                                    "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT flags are enabled.",
                                    string_VkImageViewType(view_type), string_VkImageType(image_type));
                            }
                        } else {
                            skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                             "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                             string_VkImageViewType(view_type), string_VkImageType(image_type));
                        }
                    }
                } else {
                    if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                        skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                         "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                         string_VkImageViewType(view_type), string_VkImageType(image_type));
                    }
                }
                break;
            default:
                break;
        }

        // External format checks needed when VK_ANDROID_external_memory_android_hardware_buffer enabled
        if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
            skip |= ValidateCreateImageViewANDROID(pCreateInfo);
        }

        skip |= ValidateImageViewFormatFeatures(image_state, view_format, image_usage);

        if (enabled_features.shading_rate_image.shadingRateImage) {
            if (image_usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
                if (view_format != VK_FORMAT_R8_UINT) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-02087",
                                     "vkCreateImageView() If image was created with usage containing "
                                     "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, format must be VK_FORMAT_R8_UINT.");
                }
            }
        }

        if (enabled_features.shading_rate_image.shadingRateImage ||
            enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
            if (image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
                if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-02086",
                                     "vkCreateImageView() If image was created with usage containing "
                                     "VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, viewType must be "
                                     "VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
                }
            }
        }

        if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate &&
            !phys_dev_ext_props.fragment_shading_rate_props.layeredShadingRateAttachments &&
            image_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR &&
            pCreateInfo->subresourceRange.layerCount != 1) {
            skip |= LogError(device, "VUID-VkImageViewCreateInfo-usage-04551",
                             "vkCreateImageView(): subresourceRange.layerCount is %u for a shading rate attachment image view.",
                             pCreateInfo->subresourceRange.layerCount);
        }

        if (pCreateInfo->subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS) {
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE &&
                image_state->createInfo.arrayLayers - pCreateInfo->subresourceRange.baseArrayLayer != 6) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-viewType-02962",
                                 "vkCreateImageView(): subresourceRange.layerCount VK_REMAINING_ARRAY_LAYERS=(%d) must be 6",
                                 image_state->createInfo.arrayLayers - pCreateInfo->subresourceRange.baseArrayLayer);
            }
            if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY &&
                ((image_state->createInfo.arrayLayers - pCreateInfo->subresourceRange.baseArrayLayer) % 6) != 0) {
                skip |= LogError(
                    device, "VUID-VkImageViewCreateInfo-viewType-02963",
                    "vkCreateImageView(): subresourceRange.layerCount VK_REMAINING_ARRAY_LAYERS=(%d) must be a multiple of 6",
                    image_state->createInfo.arrayLayers - pCreateInfo->subresourceRange.baseArrayLayer);
            }
        }

        if (image_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
            if (pCreateInfo->subresourceRange.levelCount != 1) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-02571",
                                 "vkCreateImageView(): If image was created with usage containing "
                                 "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, subresourceRange.levelCount (%d) must: be 1",
                                 pCreateInfo->subresourceRange.levelCount);
            }
        }
        if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT) {
            if (!enabled_features.fragment_density_map_features.fragmentDensityMapDynamic) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-02572",
                                 "vkCreateImageView(): If the fragmentDensityMapDynamic feature is not enabled, "
                                 "flags must not contain VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT");
            }
        } else {
            if (image_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
                if (image_flags & (VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT |
                                   VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) {
                    skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-04116",
                                     "vkCreateImageView(): If image was created with usage containing "
                                     "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT flags must not contain any of "
                                     "VK_IMAGE_CREATE_PROTECTED_BIT, VK_IMAGE_CREATE_SPARSE_BINDING_BIT, "
                                     "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT");
                }
            }
        }

        if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT) {
            if (!enabled_features.fragment_density_map2_features.fragmentDensityMapDeferred) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-03567",
                                 "vkCreateImageView(): If the fragmentDensityMapDeferred feature is not enabled, "
                                 "flags must not contain VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT");
            }
            if (pCreateInfo->flags & VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT) {
                skip |=
                    LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-flags-03568",
                             "vkCreateImageView(): If flags contains VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT, "
                             "flags must not contain VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT");
            }
        }
        if (device_extensions.vk_ext_fragment_density_map_2) {
            if ((image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) && (image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) &&
                (pCreateInfo->subresourceRange.layerCount >
                 phys_dev_ext_props.fragment_density_map2_props.maxSubsampledArrayLayers)) {
                skip |= LogError(pCreateInfo->image, "VUID-VkImageViewCreateInfo-image-03569",
                                 "vkCreateImageView(): If image was created with flags containing "
                                 "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT and usage containing VK_IMAGE_USAGE_SAMPLED_BIT "
                                 "subresourceRange.layerCount (%d) must: be less than or equal to maxSubsampledArrayLayers (%d)",
                                 pCreateInfo->subresourceRange.layerCount,
                                 phys_dev_ext_props.fragment_density_map2_props.maxSubsampledArrayLayers);
            }
        }

        auto astc_decode_mode = LvlFindInChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
        if ((device_extensions.vk_ext_astc_decode_mode) && (astc_decode_mode != nullptr)) {
            if ((enabled_features.astc_decode_features.decodeModeSharedExponent == VK_FALSE) &&
                (astc_decode_mode->decodeMode == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
                skip |= LogError(device, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02231",
                                 "vkCreateImageView(): decodeModeSharedExponent is not enabled but "
                                 "VkImageViewASTCDecodeModeEXT::decodeMode is VK_FORMAT_E5B9G9R9_UFLOAT_PACK32.");
            }
        }

        if (ExtEnabled::kNotEnabled != device_extensions.vk_khr_portability_subset) {
            // If swizzling is disabled, make sure it isn't used
            if ((VK_FALSE == enabled_features.portability_subset_features.imageViewFormatSwizzle) &&
                (pCreateInfo->components.r != VK_COMPONENT_SWIZZLE_IDENTITY ||
                 pCreateInfo->components.g != VK_COMPONENT_SWIZZLE_IDENTITY ||
                 pCreateInfo->components.b != VK_COMPONENT_SWIZZLE_IDENTITY ||
                 pCreateInfo->components.a != VK_COMPONENT_SWIZZLE_IDENTITY)) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-imageViewFormatSwizzle-04465",
                                 "vkCreateImageView (portability error): swizzle is disabled for this device.");
            }

            // Ensure ImageView's format has the same number of bits and components as Image's format if format reinterpretation is
            // disabled
            // TODO (ncesario): This is not correct for some cases (e.g., VK_FORMAT_B10G11R11_UFLOAT_PACK32 and
            // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32), but requires additional information that should probably be generated from the
            // spec. See Github issue #2361.
            if ((VK_FALSE == enabled_features.portability_subset_features.imageViewFormatReinterpretation) &&
                ((FormatElementSize(pCreateInfo->format, VK_IMAGE_ASPECT_COLOR_BIT) !=
                  FormatElementSize(image_state->createInfo.format, VK_IMAGE_ASPECT_COLOR_BIT)) ||
                 (FormatChannelCount(pCreateInfo->format) != FormatChannelCount(image_state->createInfo.format)))) {
                skip |= LogError(device, "VUID-VkImageViewCreateInfo-imageViewFormatReinterpretation-04466",
                                 "vkCreateImageView (portability error): ImageView format must have"
                                 " the same number of components and bits per component as the Image's format");
            }
        }
    }
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferBounds(const BUFFER_STATE *src_buffer_state, const BUFFER_STATE *dst_buffer_state,
                                             uint32_t regionCount, const RegionType *pRegions, CopyCommandVersion version) const {
    bool skip = false;
    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdCopyBuffer2KHR()" : "vkCmdCopyBuffer()";
    const char *vuid;

    VkDeviceSize src_buffer_size = src_buffer_state->createInfo.size;
    VkDeviceSize dst_buffer_size = dst_buffer_state->createInfo.size;
    VkDeviceSize src_min = UINT64_MAX;
    VkDeviceSize src_max = 0;
    VkDeviceSize dst_min = UINT64_MAX;
    VkDeviceSize dst_max = 0;

    for (uint32_t i = 0; i < regionCount; i++) {
        src_min = std::min(src_min, pRegions[i].srcOffset);
        src_max = std::max(src_max, (pRegions[i].srcOffset + pRegions[i].size));
        dst_min = std::min(dst_min, pRegions[i].dstOffset);
        dst_max = std::max(dst_max, (pRegions[i].dstOffset + pRegions[i].size));

        // The srcOffset member of each element of pRegions must be less than the size of srcBuffer
        if (pRegions[i].srcOffset >= src_buffer_size) {
            vuid = is_2khr ? "VUID-VkCopyBufferInfo2KHR-srcOffset-00113" : "VUID-vkCmdCopyBuffer-srcOffset-00113";
            skip |= LogError(src_buffer_state->buffer, vuid,
                             "%s: pRegions[%d].srcOffset (%" PRIuLEAST64 ") is greater than pRegions[%d].size (%" PRIuLEAST64 ").",
                             func_name, i, pRegions[i].srcOffset, i, pRegions[i].size);
        }

        // The dstOffset member of each element of pRegions must be less than the size of dstBuffer
        if (pRegions[i].dstOffset >= dst_buffer_size) {
            vuid = is_2khr ? "VUID-VkCopyBufferInfo2KHR-dstOffset-00114" : "VUID-vkCmdCopyBuffer-dstOffset-00114";
            skip |= LogError(dst_buffer_state->buffer, vuid,
                             "%s: pRegions[%d].dstOffset (%" PRIuLEAST64 ") is greater than pRegions[%d].size (%" PRIuLEAST64 ").",
                             func_name, i, pRegions[i].dstOffset, i, pRegions[i].size);
        }

        // The size member of each element of pRegions must be less than or equal to the size of srcBuffer minus srcOffset
        if (pRegions[i].size > (src_buffer_size - pRegions[i].srcOffset)) {
            vuid = is_2khr ? "VUID-VkCopyBufferInfo2KHR-size-00115" : "VUID-vkCmdCopyBuffer-size-00115";
            skip |= LogError(src_buffer_state->buffer, vuid,
                             "%s: pRegions[%d].size (%" PRIuLEAST64 ") is greater than the source buffer size (%" PRIuLEAST64
                             ") minus pRegions[%d].srcOffset (%" PRIuLEAST64 ").",
                             func_name, i, pRegions[i].size, src_buffer_size, i, pRegions[i].srcOffset);
        }

        // The size member of each element of pRegions must be less than or equal to the size of dstBuffer minus dstOffset
        if (pRegions[i].size > (dst_buffer_size - pRegions[i].dstOffset)) {
            vuid = is_2khr ? "VUID-VkCopyBufferInfo2KHR-size-00116" : "VUID-vkCmdCopyBuffer-size-00116";
            skip |= LogError(dst_buffer_state->buffer, vuid,
                             "%s: pRegions[%d].size (%" PRIuLEAST64 ") is greater than the destination buffer size (%" PRIuLEAST64
                             ") minus pRegions[%d].dstOffset (%" PRIuLEAST64 ").",
                             func_name, i, pRegions[i].size, dst_buffer_size, i, pRegions[i].dstOffset);
        }
    }

    // The union of the source regions, and the union of the destination regions, must not overlap in memory
    if (src_buffer_state->buffer == dst_buffer_state->buffer) {
        if (((src_min > dst_min) && (src_min < dst_max)) || ((src_max > dst_min) && (src_max < dst_max))) {
            vuid = is_2khr ? "VUID-VkCopyBufferInfo2KHR-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";
            skip |= LogError(src_buffer_state->buffer, vuid, "%s: Detected overlap between source and dest regions in memory.",
                             func_name);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                              uint32_t regionCount, const VkBufferCopy *pRegions) const {
    const auto cb_node = GetCBState(commandBuffer);
    const auto src_buffer_state = GetBufferState(srcBuffer);
    const auto dst_buffer_state = GetBufferState(dstBuffer);

    bool skip = false;
    skip |= ValidateMemoryIsBoundToBuffer(src_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-srcBuffer-00119");
    skip |= ValidateMemoryIsBoundToBuffer(dst_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-dstBuffer-00121");
    // Validate that SRC & DST buffers have correct usage flags set
    skip |=
        ValidateBufferUsageFlags(src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, "VUID-vkCmdCopyBuffer-srcBuffer-00118",
                                 "vkCmdCopyBuffer()", "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    skip |=
        ValidateBufferUsageFlags(dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, "VUID-vkCmdCopyBuffer-dstBuffer-00120",
                                 "vkCmdCopyBuffer()", "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |=
        ValidateCmdQueueFlags(cb_node, "vkCmdCopyBuffer()", VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                              "VUID-vkCmdCopyBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_node, CMD_COPYBUFFER, "vkCmdCopyBuffer()");
    skip |= InsideRenderPass(cb_node, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-renderpass");
    skip |= ValidateCmdCopyBufferBounds(src_buffer_state, dst_buffer_state, regionCount, pRegions, COPY_COMMAND_VERSION_1);
    skip |= ValidateProtectedBuffer(cb_node, src_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-commandBuffer-01822");
    skip |= ValidateProtectedBuffer(cb_node, dst_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-commandBuffer-01823");
    skip |= ValidateUnprotectedBuffer(cb_node, dst_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-commandBuffer-01824");
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                  const VkCopyBufferInfo2KHR *pCopyBufferInfos) const {
    const auto cb_node = GetCBState(commandBuffer);
    const auto src_buffer_state = GetBufferState(pCopyBufferInfos->srcBuffer);
    const auto dst_buffer_state = GetBufferState(pCopyBufferInfos->dstBuffer);

    bool skip = false;
    skip |= ValidateMemoryIsBoundToBuffer(src_buffer_state, "vkCmdCopyBuffer2KHR()", "VUID-VkCopyBufferInfo2KHR-srcBuffer-00119");
    skip |= ValidateMemoryIsBoundToBuffer(dst_buffer_state, "vkCmdCopyBuffer2KHR()", "VUID-VkCopyBufferInfo2KHR-dstBuffer-00121");
    // Validate that SRC & DST buffers have correct usage flags set
    skip |= ValidateBufferUsageFlags(src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true,
                                     "VUID-VkCopyBufferInfo2KHR-srcBuffer-00118", "vkCmdCopyBuffer2KHR()",
                                     "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    skip |= ValidateBufferUsageFlags(dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-VkCopyBufferInfo2KHR-dstBuffer-00120", "vkCmdCopyBuffer2KHR()",
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= ValidateCmdQueueFlags(cb_node, "vkCmdCopyBuffer2KHR()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdCopyBuffer2KHR-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_node, CMD_COPYBUFFER2KHR, "vkCmdCopyBuffer2KHR()");
    skip |= InsideRenderPass(cb_node, "vkCmdCopyBuffer2KHR()", "VUID-vkCmdCopyBuffer2KHR-renderpass");
    skip |= ValidateCmdCopyBufferBounds(src_buffer_state, dst_buffer_state, pCopyBufferInfos->regionCount,
                                        pCopyBufferInfos->pRegions, COPY_COMMAND_VERSION_2);
    skip |=
        ValidateProtectedBuffer(cb_node, src_buffer_state, "vkCmdCopyBuffer2KHR()", "VUID-vkCmdCopyBuffer2KHR-commandBuffer-01822");
    skip |=
        ValidateProtectedBuffer(cb_node, dst_buffer_state, "vkCmdCopyBuffer2KHR()", "VUID-vkCmdCopyBuffer2KHR-commandBuffer-01823");
    skip |= ValidateUnprotectedBuffer(cb_node, dst_buffer_state, "vkCmdCopyBuffer2KHR()",
                                      "VUID-vkCmdCopyBuffer2KHR-commandBuffer-01824");
    return skip;
}

bool CoreChecks::ValidateIdleBuffer(VkBuffer buffer) const {
    bool skip = false;
    auto buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        if (buffer_state->in_use.load()) {
            skip |= LogError(buffer, "VUID-vkDestroyBuffer-buffer-00922", "Cannot free %s that is in use by a command buffer.",
                             report_data->FormatHandle(buffer).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView,
                                                 const VkAllocationCallbacks *pAllocator) const {
    const IMAGE_VIEW_STATE *image_view_state = GetImageViewState(imageView);
    const VulkanTypedHandle obj_struct(imageView, kVulkanObjectTypeImageView);

    bool skip = false;
    if (image_view_state) {
        skip |=
            ValidateObjectNotInUse(image_view_state, obj_struct, "vkDestroyImageView", "VUID-vkDestroyImageView-imageView-01026");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) const {
    auto buffer_state = GetBufferState(buffer);

    bool skip = false;
    if (buffer_state) {
        skip |= ValidateIdleBuffer(buffer);
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                  const VkAllocationCallbacks *pAllocator) const {
    auto buffer_view_state = GetBufferViewState(bufferView);
    const VulkanTypedHandle obj_struct(bufferView, kVulkanObjectTypeBufferView);
    bool skip = false;
    if (buffer_view_state) {
        skip |= ValidateObjectNotInUse(buffer_view_state, obj_struct, "vkDestroyBufferView",
                                       "VUID-vkDestroyBufferView-bufferView-00936");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                              VkDeviceSize size, uint32_t data) const {
    auto cb_node = GetCBState(commandBuffer);
    auto buffer_state = GetBufferState(dstBuffer);
    bool skip = false;
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-dstBuffer-00031");
    skip |=
        ValidateCmdQueueFlags(cb_node, "vkCmdFillBuffer()", VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                              "VUID-vkCmdFillBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_node, CMD_FILLBUFFER, "vkCmdFillBuffer()");
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, "VUID-vkCmdFillBuffer-dstBuffer-00029",
                                     "vkCmdFillBuffer()", "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= InsideRenderPass(cb_node, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-renderpass");

    skip |= ValidateProtectedBuffer(cb_node, buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-commandBuffer-01811");
    skip |= ValidateUnprotectedBuffer(cb_node, buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-commandBuffer-01812");

    if (dstOffset >= buffer_state->createInfo.size) {
        skip |= LogError(dstBuffer, "VUID-vkCmdFillBuffer-dstOffset-00024",
                         "vkCmdFillBuffer(): dstOffset (0x%" PRIxLEAST64
                         ") is not less than destination buffer (%s) size (0x%" PRIxLEAST64 ").",
                         dstOffset, report_data->FormatHandle(dstBuffer).c_str(), buffer_state->createInfo.size);
    }

    if ((size != VK_WHOLE_SIZE) && (size > (buffer_state->createInfo.size - dstOffset))) {
        skip |= LogError(dstBuffer, "VUID-vkCmdFillBuffer-size-00027",
                         "vkCmdFillBuffer(): size (0x%" PRIxLEAST64 ") is greater than dstBuffer (%s) size (0x%" PRIxLEAST64
                         ") minus dstOffset (0x%" PRIxLEAST64 ").",
                         size, report_data->FormatHandle(dstBuffer).c_str(), buffer_state->createInfo.size, dstOffset);
    }

    return skip;
}

template <typename BufferImageCopyRegionType>
bool CoreChecks::ValidateBufferImageCopyData(const CMD_BUFFER_STATE *cb_node, uint32_t regionCount,
                                             const BufferImageCopyRegionType *pRegions, const IMAGE_STATE *image_state,
                                             const char *function, CopyCommandVersion version, bool image_to_buffer) const {
    bool skip = false;
    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *vuid;

    assert(image_state != nullptr);
    const VkFormat image_format = image_state->createInfo.format;

    for (uint32_t i = 0; i < regionCount; i++) {
        const VkImageAspectFlags region_aspect_mask = pRegions[i].imageSubresource.aspectMask;
        if (image_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((pRegions[i].imageOffset.y != 0) || (pRegions[i].imageExtent.height != 1)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00199", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] imageOffset.y is %d and imageExtent.height is %d. For 1D images these must be 0 "
                                 "and 1, respectively.",
                                 function, i, pRegions[i].imageOffset.y, pRegions[i].imageExtent.height);
            }
        }

        if ((image_state->createInfo.imageType == VK_IMAGE_TYPE_1D) || (image_state->createInfo.imageType == VK_IMAGE_TYPE_2D)) {
            if ((pRegions[i].imageOffset.z != 0) || (pRegions[i].imageExtent.depth != 1)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00201", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] imageOffset.z is %d and imageExtent.depth is %d. For 1D and 2D images these "
                                 "must be 0 and 1, respectively.",
                                 function, i, pRegions[i].imageOffset.z, pRegions[i].imageExtent.depth);
            }
        }

        if (image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
            if ((0 != pRegions[i].imageSubresource.baseArrayLayer) || (1 != pRegions[i].imageSubresource.layerCount)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00213", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] imageSubresource.baseArrayLayer is %d and imageSubresource.layerCount is %d. "
                                 "For 3D images these must be 0 and 1, respectively.",
                                 function, i, pRegions[i].imageSubresource.baseArrayLayer, pRegions[i].imageSubresource.layerCount);
            }
        }

        // If the the calling command's VkImage parameter's format is not a depth/stencil format,
        // then bufferOffset must be a multiple of the calling command's VkImage parameter's element size
        const uint32_t element_size = FormatElementSize(image_format, region_aspect_mask);
        const VkDeviceSize bufferOffset = pRegions[i].bufferOffset;

        if (FormatIsDepthOrStencil(image_format)) {
            if (SafeModulo(bufferOffset, 4) != 0) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("04053", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                                 " must be a multiple 4 if using a depth/stencil format (%s).",
                                 function, i, bufferOffset, string_VkFormat(image_format));
            }
        } else {
            // If not depth/stencil and not multi-plane
            if (!FormatIsMultiplane(image_format) && (SafeModulo(bufferOffset, element_size) != 0)) {
                vuid = (device_extensions.vk_khr_sampler_ycbcr_conversion)
                           ? GetBufferImageCopyCommandVUID("01558", image_to_buffer, is_2khr)
                           : GetBufferImageCopyCommandVUID("00193", image_to_buffer, is_2khr);
                skip |= LogError(image_state->image, vuid,
                                 "%s: pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                                 " must be a multiple of this format's texel size (%" PRIu32 ").",
                                 function, i, bufferOffset, element_size);
            }
        }

        //  BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
        if ((pRegions[i].bufferRowLength != 0) && (pRegions[i].bufferRowLength < pRegions[i].imageExtent.width)) {
            vuid = (is_2khr) ? "VUID-VkBufferImageCopy2KHR-bufferRowLength-00195" : "VUID-VkBufferImageCopy-bufferRowLength-00195";
            skip |=
                LogError(image_state->image, vuid,
                         "%s: pRegion[%d] bufferRowLength (%d) must be zero or greater-than-or-equal-to imageExtent.width (%d).",
                         function, i, pRegions[i].bufferRowLength, pRegions[i].imageExtent.width);
        }

        //  BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
        if ((pRegions[i].bufferImageHeight != 0) && (pRegions[i].bufferImageHeight < pRegions[i].imageExtent.height)) {
            vuid =
                (is_2khr) ? "VUID-VkBufferImageCopy2KHR-bufferImageHeight-00196" : "VUID-VkBufferImageCopy-bufferImageHeight-00196";
            skip |=
                LogError(image_state->image, vuid,
                         "%s: pRegion[%d] bufferImageHeight (%d) must be zero or greater-than-or-equal-to imageExtent.height (%d).",
                         function, i, pRegions[i].bufferImageHeight, pRegions[i].imageExtent.height);
        }

        // Calculate adjusted image extent, accounting for multiplane image factors
        VkExtent3D adjusted_image_extent = GetImageSubresourceExtent(image_state, &pRegions[i].imageSubresource);
        // imageOffset.x and (imageExtent.width + imageOffset.x) must both be >= 0 and <= image subresource width
        if ((pRegions[i].imageOffset.x < 0) || (pRegions[i].imageOffset.x > static_cast<int32_t>(adjusted_image_extent.width)) ||
            ((pRegions[i].imageOffset.x + static_cast<int32_t>(pRegions[i].imageExtent.width)) >
             static_cast<int32_t>(adjusted_image_extent.width))) {
            skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00197", image_to_buffer, is_2khr),
                             "%s: Both pRegion[%d] imageoffset.x (%d) and (imageExtent.width + imageOffset.x) (%d) must be >= "
                             "zero or <= image subresource width (%d).",
                             function, i, pRegions[i].imageOffset.x, (pRegions[i].imageOffset.x + pRegions[i].imageExtent.width),
                             adjusted_image_extent.width);
        }

        // imageOffset.y and (imageExtent.height + imageOffset.y) must both be >= 0 and <= image subresource height
        if ((pRegions[i].imageOffset.y < 0) || (pRegions[i].imageOffset.y > static_cast<int32_t>(adjusted_image_extent.height)) ||
            ((pRegions[i].imageOffset.y + static_cast<int32_t>(pRegions[i].imageExtent.height)) >
             static_cast<int32_t>(adjusted_image_extent.height))) {
            skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00198", image_to_buffer, is_2khr),
                             "%s: Both pRegion[%d] imageoffset.y (%d) and (imageExtent.height + imageOffset.y) (%d) must be >= "
                             "zero or <= image subresource height (%d).",
                             function, i, pRegions[i].imageOffset.y, (pRegions[i].imageOffset.y + pRegions[i].imageExtent.height),
                             adjusted_image_extent.height);
        }

        // imageOffset.z and (imageExtent.depth + imageOffset.z) must both be >= 0 and <= image subresource depth
        if ((pRegions[i].imageOffset.z < 0) || (pRegions[i].imageOffset.z > static_cast<int32_t>(adjusted_image_extent.depth)) ||
            ((pRegions[i].imageOffset.z + static_cast<int32_t>(pRegions[i].imageExtent.depth)) >
             static_cast<int32_t>(adjusted_image_extent.depth))) {
            skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00200", image_to_buffer, is_2khr),
                             "%s: Both pRegion[%d] imageoffset.z (%d) and (imageExtent.depth + imageOffset.z) (%d) must be >= "
                             "zero or <= image subresource depth (%d).",
                             function, i, pRegions[i].imageOffset.z, (pRegions[i].imageOffset.z + pRegions[i].imageExtent.depth),
                             adjusted_image_extent.depth);
        }

        // subresource aspectMask must have exactly 1 bit set
        const int num_bits = sizeof(VkFlags) * CHAR_BIT;
        std::bitset<num_bits> aspect_mask_bits(region_aspect_mask);
        if (aspect_mask_bits.count() != 1) {
            vuid = (is_2khr) ? "VUID-VkBufferImageCopy2KHR-aspectMask-00212" : "VUID-VkBufferImageCopy-aspectMask-00212";
            skip |= LogError(image_state->image, vuid,
                             "%s: aspectMasks for imageSubresource in pRegion[%d] must have only a single bit set.", function, i);
        }

        // image subresource aspect bit must match format
        if (!VerifyAspectsPresent(region_aspect_mask, image_format)) {
            skip |=
                LogError(image_state->image, GetBufferImageCopyCommandVUID("00211", image_to_buffer, is_2khr),
                         "%s: pRegion[%d] subresource aspectMask 0x%x specifies aspects that are not present in image format 0x%x.",
                         function, i, region_aspect_mask, image_format);
        }

        // Checks that apply only to compressed images
        if (FormatIsCompressed(image_format) || FormatIsSinglePlane_422(image_format)) {
            auto block_size = FormatTexelBlockExtent(image_format);

            //  BufferRowLength must be a multiple of block width
            if (SafeModulo(pRegions[i].bufferRowLength, block_size.width) != 0) {
                skip |=
                    LogError(image_state->image, GetBufferImageCopyCommandVUID("00203", image_to_buffer, is_2khr),
                             "%s: pRegion[%d] bufferRowLength (%d) must be a multiple of the compressed image's texel width (%d)..",
                             function, i, pRegions[i].bufferRowLength, block_size.width);
            }

            //  BufferRowHeight must be a multiple of block height
            if (SafeModulo(pRegions[i].bufferImageHeight, block_size.height) != 0) {
                skip |= LogError(
                    image_state->image, GetBufferImageCopyCommandVUID("00204", image_to_buffer, is_2khr),
                    "%s: pRegion[%d] bufferImageHeight (%d) must be a multiple of the compressed image's texel height (%d)..",
                    function, i, pRegions[i].bufferImageHeight, block_size.height);
            }

            //  image offsets must be multiples of block dimensions
            if ((SafeModulo(pRegions[i].imageOffset.x, block_size.width) != 0) ||
                (SafeModulo(pRegions[i].imageOffset.y, block_size.height) != 0) ||
                (SafeModulo(pRegions[i].imageOffset.z, block_size.depth) != 0)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00205", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] imageOffset(x,y) (%d, %d) must be multiples of the compressed image's texel "
                                 "width & height (%d, %d)..",
                                 function, i, pRegions[i].imageOffset.x, pRegions[i].imageOffset.y, block_size.width,
                                 block_size.height);
            }

            // bufferOffset must be a multiple of block size (linear bytes)
            uint32_t block_size_in_bytes = FormatElementSize(image_format);
            if (SafeModulo(bufferOffset, block_size_in_bytes) != 0) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00206", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] bufferOffset (0x%" PRIxLEAST64
                                 ") must be a multiple of the compressed image's texel block size (%" PRIu32 ")..",
                                 function, i, bufferOffset, block_size_in_bytes);
            }

            // imageExtent width must be a multiple of block width, or extent+offset width must equal subresource width
            VkExtent3D mip_extent = GetImageSubresourceExtent(image_state, &(pRegions[i].imageSubresource));
            if ((SafeModulo(pRegions[i].imageExtent.width, block_size.width) != 0) &&
                (pRegions[i].imageExtent.width + pRegions[i].imageOffset.x != mip_extent.width)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00207", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] extent width (%d) must be a multiple of the compressed texture block width "
                                 "(%d), or when added to offset.x (%d) must equal the image subresource width (%d)..",
                                 function, i, pRegions[i].imageExtent.width, block_size.width, pRegions[i].imageOffset.x,
                                 mip_extent.width);
            }

            // imageExtent height must be a multiple of block height, or extent+offset height must equal subresource height
            if ((SafeModulo(pRegions[i].imageExtent.height, block_size.height) != 0) &&
                (pRegions[i].imageExtent.height + pRegions[i].imageOffset.y != mip_extent.height)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00208", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] extent height (%d) must be a multiple of the compressed texture block height "
                                 "(%d), or when added to offset.y (%d) must equal the image subresource height (%d)..",
                                 function, i, pRegions[i].imageExtent.height, block_size.height, pRegions[i].imageOffset.y,
                                 mip_extent.height);
            }

            // imageExtent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
            if ((SafeModulo(pRegions[i].imageExtent.depth, block_size.depth) != 0) &&
                (pRegions[i].imageExtent.depth + pRegions[i].imageOffset.z != mip_extent.depth)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("00209", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] extent width (%d) must be a multiple of the compressed texture block depth "
                                 "(%d), or when added to offset.z (%d) must equal the image subresource depth (%d)..",
                                 function, i, pRegions[i].imageExtent.depth, block_size.depth, pRegions[i].imageOffset.z,
                                 mip_extent.depth);
            }
        }

        // Checks that apply only to multi-planar format images
        if (FormatIsMultiplane(image_format)) {
            // VK_IMAGE_ASPECT_PLANE_2_BIT valid only for image formats with three planes
            if ((FormatPlaneCount(image_format) < 3) && (region_aspect_mask == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("01560", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] subresource aspectMask cannot be VK_IMAGE_ASPECT_PLANE_2_BIT unless image "
                                 "format has three planes.",
                                 function, i);
            }

            // image subresource aspectMask must be VK_IMAGE_ASPECT_PLANE_*_BIT
            if (0 ==
                (region_aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
                skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("01560", image_to_buffer, is_2khr),
                                 "%s: pRegion[%d] subresource aspectMask for multi-plane image formats must have a "
                                 "VK_IMAGE_ASPECT_PLANE_*_BIT when copying to or from.",
                                 function, i);
            } else {
                // Know aspect mask is valid
                const VkFormat compatible_format = FindMultiplaneCompatibleFormat(image_format, region_aspect_mask);
                const uint32_t compatible_size = FormatElementSize(compatible_format);
                if (SafeModulo(bufferOffset, compatible_size) != 0) {
                    skip |= LogError(
                        image_state->image, GetBufferImageCopyCommandVUID("01559", image_to_buffer, is_2khr),
                        "%s: pRegion[%d]->bufferOffset is 0x%" PRIxLEAST64
                        " but must be a multiple of the multi-plane compatible format's texel size (%u) for plane %u (%s).",
                        function, i, bufferOffset, element_size, GetPlaneIndex(region_aspect_mask),
                        string_VkFormat(compatible_format));
                }
            }
        }

        // TODO - Don't use ValidateCmdQueueFlags due to currently not having way to add more descriptive message
        const COMMAND_POOL_STATE *command_pool = cb_node->command_pool.get();
        assert(command_pool != nullptr);
        const uint32_t queue_family_index = command_pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = GetPhysicalDeviceState()->queue_family_properties[queue_family_index].queueFlags;
        if (((queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == 0) && (SafeModulo(bufferOffset, 4) != 0)) {
            LogObjectList objlist(cb_node->commandBuffer);
            objlist.add(command_pool->commandPool);
            skip |= LogError(image_state->image, GetBufferImageCopyCommandVUID("04052", image_to_buffer, is_2khr),
                             "%s: pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                             " must be a multiple 4 because the command buffer %s was allocated from the command pool %s "
                             "which was created with queueFamilyIndex %u, which doesn't contain the VK_QUEUE_GRAPHICS_BIT or "
                             "VK_QUEUE_COMPUTE_BIT flag.",
                             function, i, bufferOffset, report_data->FormatHandle(cb_node->commandBuffer).c_str(),
                             report_data->FormatHandle(command_pool->commandPool).c_str(), queue_family_index);
        }
    }

    return skip;
}

template <typename BufferImageCopyRegionType>
bool CoreChecks::ValidateImageBounds(const IMAGE_STATE *image_state, const uint32_t regionCount,
                                     const BufferImageCopyRegionType *pRegions, const char *func_name, const char *msg_code) const {
    bool skip = false;
    const VkImageCreateInfo *image_info = &(image_state->createInfo);

    for (uint32_t i = 0; i < regionCount; i++) {
        VkExtent3D extent = pRegions[i].imageExtent;
        VkOffset3D offset = pRegions[i].imageOffset;

        if (IsExtentSizeZero(&extent))  // Warn on zero area subresource
        {
            skip |= LogWarning(image_state->image, kVUID_Core_Image_ZeroAreaSubregion,
                               "%s: pRegion[%d] imageExtent of {%1d, %1d, %1d} has zero area", func_name, i, extent.width,
                               extent.height, extent.depth);
        }

        VkExtent3D image_extent = GetImageSubresourceExtent(image_state, &(pRegions[i].imageSubresource));

        // If we're using a compressed format, valid extent is rounded up to multiple of block size (per 18.1)
        if (FormatIsCompressed(image_info->format) || FormatIsSinglePlane_422(image_state->createInfo.format)) {
            auto block_extent = FormatTexelBlockExtent(image_info->format);
            if (image_extent.width % block_extent.width) {
                image_extent.width += (block_extent.width - (image_extent.width % block_extent.width));
            }
            if (image_extent.height % block_extent.height) {
                image_extent.height += (block_extent.height - (image_extent.height % block_extent.height));
            }
            if (image_extent.depth % block_extent.depth) {
                image_extent.depth += (block_extent.depth - (image_extent.depth % block_extent.depth));
            }
        }

        if (0 != ExceedsBounds(&offset, &extent, &image_extent)) {
            skip |= LogError(image_state->image, msg_code, "%s: pRegion[%d] exceeds image bounds..", func_name, i);
        }
    }

    return skip;
}

template <typename BufferImageCopyRegionType>
bool CoreChecks::ValidateBufferBounds(const IMAGE_STATE *image_state, const BUFFER_STATE *buff_state, uint32_t regionCount,
                                      const BufferImageCopyRegionType *pRegions, const char *func_name,
                                      const char *msg_code) const {
    bool skip = false;

    VkDeviceSize buffer_size = buff_state->createInfo.size;

    for (uint32_t i = 0; i < regionCount; i++) {
        VkDeviceSize max_buffer_offset =
            GetBufferSizeFromCopyImage(pRegions[i], image_state->createInfo.format) + pRegions[i].bufferOffset;
        if (buffer_size < max_buffer_offset) {
            skip |=
                LogError(device, msg_code, "%s: pRegion[%d] exceeds buffer size of %" PRIu64 " bytes..", func_name, i, buffer_size);
        }
    }

    return skip;
}

template <typename BufferImageCopyRegionType>
bool CoreChecks::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkBuffer dstBuffer, uint32_t regionCount, const BufferImageCopyRegionType *pRegions,
                                              CopyCommandVersion version) const {
    const auto cb_node = GetCBState(commandBuffer);
    const auto src_image_state = GetImageState(srcImage);
    const auto dst_buffer_state = GetBufferState(dstBuffer);

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdCopyImageToBuffer2KHR()" : "vkCmdCopyImageToBuffer()";
    const CMD_TYPE cmd_type = is_2khr ? CMD_COPYIMAGETOBUFFER2KHR : CMD_COPYIMAGETOBUFFER;
    const char *vuid;

    bool skip = ValidateBufferImageCopyData(cb_node, regionCount, pRegions, src_image_state, func_name, version, true);

    // Validate command buffer state
    skip |= ValidateCmd(cb_node, cmd_type, func_name);

    // Command pool must support graphics, compute, or transfer operations
    const auto pool = cb_node->command_pool.get();

    VkQueueFlags queue_flags = GetPhysicalDeviceState()->queue_family_properties[pool->queueFamilyIndex].queueFlags;

    if (0 == (queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))) {
        vuid =
            is_2khr ? "VUID-vkCmdCopyImageToBuffer2KHR-commandBuffer-cmdpool" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-cmdpool";
        skip |= LogError(cb_node->createInfo.commandPool, vuid,
                         "Cannot call %s on a command buffer allocated from a pool without graphics, compute, "
                         "or transfer capabilities.",
                         func_name);
    }
    vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-pRegions-00182" : "VUID-vkCmdCopyImageToBuffer-pRegions-00182";
    skip |= ValidateImageBounds(src_image_state, regionCount, pRegions, func_name, vuid);
    vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-pRegions-00183" : "VUID-vkCmdCopyImageToBuffer-pRegions-00183";
    skip |= ValidateBufferBounds(src_image_state, dst_buffer_state, regionCount, pRegions, func_name, vuid);

    vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImage-00188" : "VUID-vkCmdCopyImageToBuffer-srcImage-00188";
    const char *location = is_2khr ? "vkCmdCopyImageToBuffer2KHR(): srcImage" : "vkCmdCopyImageToBuffer(): srcImage";
    skip |= ValidateImageSampleCount(src_image_state, VK_SAMPLE_COUNT_1_BIT, location, vuid);

    vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImage-00187" : "VUID-vkCmdCopyImageToBuffer-srcImage-00187";
    skip |= ValidateMemoryIsBoundToImage(src_image_state, func_name, vuid);
    vuid = is_2khr ? "vkCmdCopyImageToBuffer-dstBuffer2KHR-00192" : "vkCmdCopyImageToBuffer dstBuffer-00192";
    skip |= ValidateMemoryIsBoundToBuffer(dst_buffer_state, func_name, vuid);

    // Validate that SRC image & DST buffer have correct usage flags set
    vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImage-00186" : "VUID-vkCmdCopyImageToBuffer-srcImage-00186";
    skip |= ValidateImageUsageFlags(src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name,
                                    "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
    vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-dstBuffer-00191" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00191";
    skip |= ValidateBufferUsageFlags(dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, vuid, func_name,
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    vuid = is_2khr ? "VUID-vkCmdCopyImageToBuffer2KHR-commandBuffer-01831" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831";
    skip |= ValidateProtectedImage(cb_node, src_image_state, func_name, vuid);
    vuid = is_2khr ? "VUID-vkCmdCopyImageToBuffer2KHR-commandBuffer-01832" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832";
    skip |= ValidateProtectedBuffer(cb_node, dst_buffer_state, func_name, vuid);
    vuid = is_2khr ? "VUID-vkCmdCopyImageToBuffer2KHR-commandBuffer-01833" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833";
    skip |= ValidateUnprotectedBuffer(cb_node, dst_buffer_state, func_name, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImage-02544" : "VUID-vkCmdCopyImageToBuffer-srcImage-02544";
        skip |= LogError(cb_node->commandBuffer, vuid,
                         "%s: srcImage must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                         func_name);
    }

    if (device_extensions.vk_khr_maintenance1) {
        vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImage-01998" : "VUID-vkCmdCopyImageToBuffer-srcImage-01998";
        skip |= ValidateImageFormatFeatureFlags(src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, func_name, vuid);
    }
    vuid = is_2khr ? "VUID-vkCmdCopyImageToBuffer-renderpass" : "VUID-vkCmdCopyImageToBuffer-renderpass";
    skip |= InsideRenderPass(cb_node, func_name, vuid);
    bool hit_error = false;

    const char *src_invalid_layout_vuid = (src_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                                              ? (vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImageLayout-01397"
                                                                : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397")
                                              : (vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImageLayout-00190"
                                                                : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00190");

    for (uint32_t i = 0; i < regionCount; ++i) {
        skip |= ValidateImageSubresourceLayers(cb_node, &pRegions[i].imageSubresource, func_name, "imageSubresource", i);
        vuid =
            is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-srcImageLayout-00189" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189";
        skip |= VerifyImageLayout(cb_node, src_image_state, pRegions[i].imageSubresource, srcImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, func_name, src_invalid_layout_vuid, vuid, &hit_error);
        vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-imageOffset-01794" : "VUID-vkCmdCopyImageToBuffer-imageOffset-01794";
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_node, src_image_state, &pRegions[i], i, func_name, vuid);
        vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-imageSubresource-01703"
                       : "VUID-vkCmdCopyImageToBuffer-imageSubresource-01703";
        skip |= ValidateImageMipLevel(cb_node, src_image_state, pRegions[i].imageSubresource.mipLevel, i, func_name,
                                      "imageSubresource", vuid);
        vuid = is_2khr ? "VUID-VkCopyImageToBufferInfo2KHR-imageSubresource-01704"
                       : "VUID-vkCmdCopyImageToBuffer-imageSubresource-01704";
        skip |= ValidateImageArrayLayerRange(cb_node, src_image_state, pRegions[i].imageSubresource.baseArrayLayer,
                                             pRegions[i].imageSubresource.layerCount, i, func_name, "imageSubresource", vuid);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkBuffer dstBuffer, uint32_t regionCount,
                                                     const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        COPY_COMMAND_VERSION_1);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, COPY_COMMAND_VERSION_2);
}

void CoreChecks::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);

    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    // Make sure that all image slices record referenced layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageInitialLayout(cb_node, *src_image_state, pRegions[i].imageSubresource, srcImageLayout);
    }
}

void CoreChecks::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);

    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(pCopyImageToBufferInfo->srcImage);
    // Make sure that all image slices record referenced layout
    for (uint32_t i = 0; i < pCopyImageToBufferInfo->regionCount; ++i) {
        SetImageInitialLayout(cb_node, *src_image_state, pCopyImageToBufferInfo->pRegions[i].imageSubresource,
                              pCopyImageToBufferInfo->srcImageLayout);
    }
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                              CopyCommandVersion version) const {
    const auto cb_node = GetCBState(commandBuffer);
    const auto src_buffer_state = GetBufferState(srcBuffer);
    const auto dst_image_state = GetImageState(dstImage);

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdCopyBufferToImage2KHR()" : "vkCmdCopyBufferToImage()";
    const CMD_TYPE cmd_type = is_2khr ? CMD_COPYBUFFERTOIMAGE2KHR : CMD_COPYBUFFERTOIMAGE;
    const char *vuid;

    bool skip = ValidateBufferImageCopyData(cb_node, regionCount, pRegions, dst_image_state, func_name, version, false);

    // Validate command buffer state
    skip |= ValidateCmd(cb_node, cmd_type, func_name);

    vuid = is_2khr ? "VUID-vkCmdCopyBufferToImage2KHR-commandBuffer-cmdpool" : "VUID-vkCmdCopyBufferToImage-commandBuffer-cmdpool";
    skip |= ValidateCmdQueueFlags(cb_node, func_name, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, vuid);

    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-pRegions-00172" : "VUID-vkCmdCopyBufferToImage-pRegions-00172";
    skip |= ValidateImageBounds(dst_image_state, regionCount, pRegions, func_name, vuid);
    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-pRegions-00171" : "VUID-vkCmdCopyBufferToImage-pRegions-00171";
    skip |= ValidateBufferBounds(dst_image_state, src_buffer_state, regionCount, pRegions, func_name, vuid);

    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImage-00179" : "VUID-vkCmdCopyBufferToImage-dstImage-00179";
    const char *location = is_2khr ? "vkCmdCopyBufferToImage2KHR(): dstImage" : "vkCmdCopyBufferToImage(): dstImage";
    skip |= ValidateImageSampleCount(dst_image_state, VK_SAMPLE_COUNT_1_BIT, location, vuid);
    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-srcBuffer-00176" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00176";
    skip |= ValidateMemoryIsBoundToBuffer(src_buffer_state, func_name, vuid);
    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImage-00178" : "VUID-vkCmdCopyBufferToImage-dstImage-00178";
    skip |= ValidateMemoryIsBoundToImage(dst_image_state, func_name, vuid);
    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-srcBuffer-00174" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00174";
    skip |= ValidateBufferUsageFlags(src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name,
                                     "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImage-00177" : "VUID-vkCmdCopyBufferToImage-dstImage-00177";
    skip |= ValidateImageUsageFlags(dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, func_name,
                                    "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
    vuid = is_2khr ? "VUID-vkCmdCopyBufferToImage2KHR-commandBuffer-01828" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01828";
    skip |= ValidateProtectedBuffer(cb_node, src_buffer_state, func_name, vuid);
    vuid = is_2khr ? "VUID-vkCmdCopyBufferToImage2KHR-commandBuffer-01829" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01829";
    skip |= ValidateProtectedImage(cb_node, dst_image_state, func_name, vuid);
    vuid = is_2khr ? "VUID-vkCmdCopyBufferToImage-commandBuffer-01830" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01830";
    skip |= ValidateUnprotectedImage(cb_node, dst_image_state, func_name, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImage-02543" : "VUID-vkCmdCopyBufferToImage-dstImage-02543";
        skip |= LogError(cb_node->commandBuffer, vuid,
                         "%s: dstImage must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                         func_name);
    }

    if (device_extensions.vk_khr_maintenance1) {
        vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImage-01997" : "VUID-vkCmdCopyBufferToImage-dstImage-01997";
        skip |= ValidateImageFormatFeatureFlags(dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, func_name, vuid);
    }
    vuid = is_2khr ? "VUID-vkCmdCopyBufferToImage2KHR-renderpass" : "VUID-vkCmdCopyBufferToImage-renderpass";
    skip |= InsideRenderPass(cb_node, func_name, vuid);
    bool hit_error = false;

    const char *dst_invalid_layout_vuid = (dst_image_state->shared_presentable && device_extensions.vk_khr_shared_presentable_image)
                                              ? (is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImageLayout-01396"
                                                         : "VUID-vkCmdCopyBufferToImage-dstImageLayout-01396")
                                              : (is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImageLayout-00181"
                                                         : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00181");

    for (uint32_t i = 0; i < regionCount; ++i) {
        skip |= ValidateImageSubresourceLayers(cb_node, &pRegions[i].imageSubresource, func_name, "imageSubresource", i);
        vuid =
            is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-dstImageLayout-00180" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180";
        skip |= VerifyImageLayout(cb_node, dst_image_state, pRegions[i].imageSubresource, dstImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, func_name, dst_invalid_layout_vuid, vuid, &hit_error);
        vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-imageOffset-01793" : "VUID-vkCmdCopyBufferToImage-imageOffset-01793";
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_node, dst_image_state, &pRegions[i], i, func_name, vuid);
        vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-imageSubresource-01701"
                       : "VUID-vkCmdCopyBufferToImage-imageSubresource-01701";
        skip |= ValidateImageMipLevel(cb_node, dst_image_state, pRegions[i].imageSubresource.mipLevel, i, func_name,
                                      "imageSubresource", vuid);
        vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-imageSubresource-01702"
                       : "VUID-vkCmdCopyBufferToImage-imageSubresource-01702";
        skip |= ValidateImageArrayLayerRange(cb_node, dst_image_state, pRegions[i].imageSubresource.baseArrayLayer,
                                             pRegions[i].imageSubresource.layerCount, i, func_name, "imageSubresource", vuid);

        // TODO - Don't use ValidateCmdQueueFlags due to currently not having way to add more descriptive message
        const COMMAND_POOL_STATE *command_pool = cb_node->command_pool.get();
        assert(command_pool != nullptr);
        const uint32_t queue_family_index = command_pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = GetPhysicalDeviceState()->queue_family_properties[queue_family_index].queueFlags;
        const VkImageAspectFlags region_aspect_mask = pRegions[i].imageSubresource.aspectMask;
        if (((queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
            ((region_aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)) {
            LogObjectList objlist(cb_node->commandBuffer);
            objlist.add(command_pool->commandPool);
            vuid = is_2khr ? "VUID-VkCopyBufferToImageInfo2KHR-commandBuffer-04477"
                           : "VUID-vkCmdCopyBufferToImage-commandBuffer-04477";
            skip |= LogError(dst_image_state->image, vuid,
                             "%s(): pRegion[%d] subresource aspectMask 0x%x specifies VK_IMAGE_ASPECT_DEPTH_BIT or "
                             "VK_IMAGE_ASPECT_STENCIL_BIT but the command buffer %s was allocated from the command pool %s "
                             "which was created with queueFamilyIndex %u, which doesn't contain the VK_QUEUE_GRAPHICS_BIT flag.",
                             func_name, i, region_aspect_mask, report_data->FormatHandle(cb_node->commandBuffer).c_str(),
                             report_data->FormatHandle(command_pool->commandPool).c_str(), queue_family_index);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        COPY_COMMAND_VERSION_1);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, COPY_COMMAND_VERSION_2);
}

void CoreChecks::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                   VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);

    auto cb_node = GetCBState(commandBuffer);
    auto dst_image_state = GetImageState(dstImage);
    // Make sure that all image slices are record referenced layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageInitialLayout(cb_node, *dst_image_state, pRegions[i].imageSubresource, dstImageLayout);
    }
}

void CoreChecks::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo2KHR) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo2KHR);

    auto cb_node = GetCBState(commandBuffer);
    auto dst_image_state = GetImageState(pCopyBufferToImageInfo2KHR->dstImage);
    // Make sure that all image slices are record referenced layout
    for (uint32_t i = 0; i < pCopyBufferToImageInfo2KHR->regionCount; ++i) {
        SetImageInitialLayout(cb_node, *dst_image_state, pCopyBufferToImageInfo2KHR->pRegions[i].imageSubresource,
                              pCopyBufferToImageInfo2KHR->dstImageLayout);
    }
}
bool CoreChecks::PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
                                                          VkSubresourceLayout *pLayout) const {
    bool skip = false;
    const VkImageAspectFlags sub_aspect = pSubresource->aspectMask;

    // The aspectMask member of pSubresource must only have a single bit set
    const int num_bits = sizeof(sub_aspect) * CHAR_BIT;
    std::bitset<num_bits> aspect_mask_bits(sub_aspect);
    if (aspect_mask_bits.count() != 1) {
        skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-aspectMask-00997",
                         "vkGetImageSubresourceLayout(): VkImageSubresource.aspectMask must have exactly 1 bit set.");
    }

    const IMAGE_STATE *image_entry = GetImageState(image);
    if (!image_entry) {
        return skip;
    }

    // Image must have been created with tiling equal to VK_IMAGE_TILING_LINEAR
    if (device_extensions.vk_ext_image_drm_format_modifier) {
        if ((image_entry->createInfo.tiling != VK_IMAGE_TILING_LINEAR) &&
            (image_entry->createInfo.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)) {
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-image-02270",
                             "vkGetImageSubresourceLayout(): Image must have tiling of VK_IMAGE_TILING_LINEAR or "
                             "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.");
        }
    } else {
        if (image_entry->createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-image-00996",
                             "vkGetImageSubresourceLayout(): Image must have tiling of VK_IMAGE_TILING_LINEAR.");
        }
    }

    // mipLevel must be less than the mipLevels specified in VkImageCreateInfo when the image was created
    if (pSubresource->mipLevel >= image_entry->createInfo.mipLevels) {
        skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-mipLevel-01716",
                         "vkGetImageSubresourceLayout(): pSubresource.mipLevel (%d) must be less than %d.", pSubresource->mipLevel,
                         image_entry->createInfo.mipLevels);
    }

    // arrayLayer must be less than the arrayLayers specified in VkImageCreateInfo when the image was created
    if (pSubresource->arrayLayer >= image_entry->createInfo.arrayLayers) {
        skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-arrayLayer-01717",
                         "vkGetImageSubresourceLayout(): pSubresource.arrayLayer (%d) must be less than %d.",
                         pSubresource->arrayLayer, image_entry->createInfo.arrayLayers);
    }

    // subresource's aspect must be compatible with image's format.
    const VkFormat img_format = image_entry->createInfo.format;
    if (image_entry->createInfo.tiling == VK_IMAGE_TILING_LINEAR) {
        if (FormatIsMultiplane(img_format)) {
            VkImageAspectFlags allowed_flags = (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
            const char *vuid = "VUID-vkGetImageSubresourceLayout-format-01581";  // 2-plane version
            if (FormatPlaneCount(img_format) > 2u) {
                allowed_flags |= VK_IMAGE_ASPECT_PLANE_2_BIT;
                vuid = "VUID-vkGetImageSubresourceLayout-format-01582";  // 3-plane version
            }
            if (sub_aspect != (sub_aspect & allowed_flags)) {
                skip |= LogError(image, vuid,
                                 "vkGetImageSubresourceLayout(): For multi-planar images, VkImageSubresource.aspectMask (0x%" PRIx32
                                 ") must be a single-plane specifier flag.",
                                 sub_aspect);
            }
        } else if (FormatIsColor(img_format)) {
            if (sub_aspect != VK_IMAGE_ASPECT_COLOR_BIT) {
                skip |= LogError(image, kVUID_Core_DrawState_InvalidImageAspect,
                                 "vkGetImageSubresourceLayout(): For color formats, VkImageSubresource.aspectMask must be "
                                 "VK_IMAGE_ASPECT_COLOR.");
            }
        } else if (FormatIsDepthOrStencil(img_format)) {
            if ((sub_aspect != VK_IMAGE_ASPECT_DEPTH_BIT) && (sub_aspect != VK_IMAGE_ASPECT_STENCIL_BIT)) {
            }
        }
    } else if (image_entry->createInfo.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        if ((sub_aspect != VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT) && (sub_aspect != VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT) &&
            (sub_aspect != VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT) && (sub_aspect != VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
            // TODO: This VU also needs to ensure that the DRM index is in range and valid.
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout-tiling-02271",
                             "vkGetImageSubresourceLayout(): VkImageSubresource.aspectMask must be "
                             "VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT.");
        }
    }

    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateGetImageSubresourceLayoutANDROID(image);
    }

    return skip;
}

// Validates the image is allowed to be protected
bool CoreChecks::ValidateProtectedImage(const CMD_BUFFER_STATE *cb_state, const IMAGE_STATE *image_state, const char *cmd_name,
                                        const char *vuid, const char *more_message) const {
    bool skip = false;
    if ((cb_state->unprotected == true) && (image_state->unprotected == false)) {
        LogObjectList objlist(cb_state->commandBuffer);
        objlist.add(image_state->image);
        skip |= LogError(objlist, vuid, "%s: command buffer %s is unprotected while image %s is a protected image.%s", cmd_name,
                         report_data->FormatHandle(cb_state->commandBuffer).c_str(),
                         report_data->FormatHandle(image_state->image).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedImage(const CMD_BUFFER_STATE *cb_state, const IMAGE_STATE *image_state, const char *cmd_name,
                                          const char *vuid, const char *more_message) const {
    bool skip = false;
    if ((cb_state->unprotected == false) && (image_state->unprotected == true)) {
        LogObjectList objlist(cb_state->commandBuffer);
        objlist.add(image_state->image);
        skip |= LogError(objlist, vuid, "%s: command buffer %s is protected while image %s is an unprotected image.%s", cmd_name,
                         report_data->FormatHandle(cb_state->commandBuffer).c_str(),
                         report_data->FormatHandle(image_state->image).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be protected
bool CoreChecks::ValidateProtectedBuffer(const CMD_BUFFER_STATE *cb_state, const BUFFER_STATE *buffer_state, const char *cmd_name,
                                         const char *vuid, const char *more_message) const {
    bool skip = false;
    if ((cb_state->unprotected == true) && (buffer_state->unprotected == false)) {
        LogObjectList objlist(cb_state->commandBuffer);
        objlist.add(buffer_state->buffer);
        skip |= LogError(objlist, vuid, "%s: command buffer %s is unprotected while buffer %s is a protected buffer.%s", cmd_name,
                         report_data->FormatHandle(cb_state->commandBuffer).c_str(),
                         report_data->FormatHandle(buffer_state->buffer).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedBuffer(const CMD_BUFFER_STATE *cb_state, const BUFFER_STATE *buffer_state, const char *cmd_name,
                                           const char *vuid, const char *more_message) const {
    bool skip = false;
    if ((cb_state->unprotected == false) && (buffer_state->unprotected == true)) {
        LogObjectList objlist(cb_state->commandBuffer);
        objlist.add(buffer_state->buffer);
        skip |= LogError(objlist, vuid, "%s: command buffer %s is protected while buffer %s is an unprotected buffer.%s", cmd_name,
                         report_data->FormatHandle(cb_state->commandBuffer).c_str(),
                         report_data->FormatHandle(buffer_state->buffer).c_str(), more_message);
    }
    return skip;
}
