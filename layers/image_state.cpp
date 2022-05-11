/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
#include "state_tracker.h"
#include <limits>

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

static uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels) {
    // Return correct number of mip levels taking into account VK_REMAINING_MIP_LEVELS
    uint32_t mip_level_count = range->levelCount;
    if (range->levelCount == VK_REMAINING_MIP_LEVELS) {
        mip_level_count = mip_levels - range->baseMipLevel;
    }
    return mip_level_count;
}

static uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers) {
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
    norm.layerCount = ResolveRemainingLayers(&range, image_create_info.arrayLayers);

    // For multiplanar formats, IMAGE_ASPECT_COLOR is equivalent to adding the aspect of the individual planes
    if (FormatIsMultiplane(image_create_info.format)) {
        if (norm.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
            norm.aspectMask &= ~VK_IMAGE_ASPECT_COLOR_BIT;
            norm.aspectMask |= (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
            if (FormatPlaneCount(image_create_info.format) > 2) {
                norm.aspectMask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
            }
        }
    }
    return norm;
}

static bool IsDepthSliced(const VkImageCreateInfo &image_create_info, const VkImageViewCreateInfo &create_info) {
    return ((image_create_info.flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) != 0) &&
           (create_info.viewType == VK_IMAGE_VIEW_TYPE_2D || create_info.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY);
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info,
                                                  const VkImageViewCreateInfo &create_info) {
    auto subres_range = create_info.subresourceRange;

    // if we're mapping a 3D image to a 2d image view, convert the view's subresource range to be compatible with the
    // image's understanding of the world. From the VkImageSubresourceRange section of the Vulkan spec:
    //
    //     When the VkImageSubresourceRange structure is used to select a subset of the slices of a 3D image’s mip level in
    //     order to create a 2D or 2D array image view of a 3D image created with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,
    //     baseArrayLayer and layerCount specify the first slice index and the number of slices to include in the created
    //     image view. Such an image view can be used as a framebuffer attachment that refers only to the specified range
    //     of slices of the selected mip level. However, any layout transitions performed on such an attachment view during
    //     a render pass instance still apply to the entire subresource referenced which includes all the slices of the
    //     selected mip level.
    //
    if (IsDepthSliced(image_create_info, create_info)) {
        subres_range.baseArrayLayer = 0;
        subres_range.layerCount = 1;
    }
    return NormalizeSubresourceRange(image_create_info, subres_range);
}

static VkExternalMemoryHandleTypeFlags GetExternalHandleType(const VkImageCreateInfo *pCreateInfo) {
    const auto *external_memory_info = LvlFindInChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkSwapchainKHR GetSwapchain(const VkImageCreateInfo *pCreateInfo) {
    const auto *swapchain_info = LvlFindInChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    return swapchain_info ? swapchain_info->swapchain : VK_NULL_HANDLE;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
static uint64_t GetExternalFormat(const VkImageCreateInfo *info) {
    const VkExternalFormatANDROID *ext_format_android = LvlFindInChain<VkExternalFormatANDROID>(info->pNext);
    return ext_format_android ? ext_format_android->externalFormat : 0;
}
#else
static uint64_t GetExternalFormat(const VkImageCreateInfo *info) { return 0; }
#endif  // VK_USE_PLATFORM_ANDROID_KHR

static IMAGE_STATE::MemoryReqs GetMemoryRequirements(const ValidationStateTracker *dev_data, VkImage img,
                                                     const VkImageCreateInfo *create_info, bool disjoint, bool is_external_ahb) {
    IMAGE_STATE::MemoryReqs result{};
    // Record the memory requirements in case they won't be queried
    // External AHB memory can't be queried until after memory is bound
    if (!is_external_ahb) {
        if (disjoint == false) {
            DispatchGetImageMemoryRequirements(dev_data->device, img, &result[0]);
        } else {
            uint32_t plane_count = FormatPlaneCount(create_info->format);
            static const std::array<VkImageAspectFlagBits, 3> aspects{VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT,
                                                                      VK_IMAGE_ASPECT_PLANE_2_BIT};
            assert(plane_count <= aspects.size());
            auto image_plane_req = lvl_init_struct<VkImagePlaneMemoryRequirementsInfo>();
            auto mem_req_info2 = lvl_init_struct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
            mem_req_info2.image = img;

            for (uint32_t i = 0; i < plane_count; i++) {
                auto mem_reqs2 = lvl_init_struct<VkMemoryRequirements2>();

                image_plane_req.planeAspect = aspects[i];
                switch (dev_data->device_extensions.vk_khr_get_memory_requirements2) {
                    case kEnabledByApiLevel:
                        DispatchGetImageMemoryRequirements2(dev_data->device, &mem_req_info2, &mem_reqs2);
                        break;
                    case kEnabledByCreateinfo:
                        DispatchGetImageMemoryRequirements2KHR(dev_data->device, &mem_req_info2, &mem_reqs2);
                        break;
                    default:
                        // The VK_KHR_sampler_ycbcr_conversion extension requires VK_KHR_get_memory_requirements2,
                        // so validation of this vkCreateImage call should have already failed.
                        assert(false);
                }
                result[i] = mem_reqs2.memoryRequirements;
            }
        }
    }
    return result;
}

static IMAGE_STATE::SparseReqs GetSparseRequirements(const ValidationStateTracker *dev_data, VkImage img,
                                                     const VkImageCreateInfo *create_info) {
    IMAGE_STATE::SparseReqs result;
    if (create_info->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
        uint32_t count = 0;
        DispatchGetImageSparseMemoryRequirements(dev_data->device, img, &count, nullptr);
        result.resize(count);
        DispatchGetImageSparseMemoryRequirements(dev_data->device, img, &count, result.data());
    }
    return result;
}

static bool SparseMetaDataRequired(const IMAGE_STATE::SparseReqs &sparse_reqs) {
    bool result = false;
    for (const auto &req : sparse_reqs) {
        if (req.formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
            result = true;
            break;
        }
    }
    return result;
}

IMAGE_STATE::IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
                         VkFormatFeatureFlags2KHR ff)
    : BINDABLE_OLD(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
                   (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0,
                   (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleType(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      shared_presentable(false),
      layout_locked(false),
      ahb_format(GetExternalFormat(pCreateInfo)),
      full_range{MakeImageFullRange(*pCreateInfo)},
      create_from_swapchain(GetSwapchain(pCreateInfo)),
      swapchain_image_index(0),
      format_features(ff),
      disjoint((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0),
      requirements(GetMemoryRequirements(dev_data, img, pCreateInfo, disjoint, IsExternalAHB())),
      memory_requirements_checked{{false, false, false}},
      sparse_requirements(GetSparseRequirements(dev_data, img, pCreateInfo)),
      sparse_metadata_required(SparseMetaDataRequired(sparse_requirements)),
      get_sparse_reqs_called(false),
      sparse_metadata_bound(false),
      subresource_encoder(full_range),
      fragment_encoder(nullptr),
      store_device_as_workaround(dev_data->device) {  // TODO REMOVE WHEN encoder can be const
    // TODO: Correctly load resource size individually
    // Ideally we would have memory tracking as part of the object and not inherited due to its limitations
    resource_size_ = requirements[0].size;
}

IMAGE_STATE::IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
                         VkSwapchainKHR swapchain, uint32_t swapchain_index, VkFormatFeatureFlags2KHR ff)
    : BINDABLE_OLD(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
                   (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0,
                   (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleType(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      shared_presentable(false),
      layout_locked(false),
      ahb_format(GetExternalFormat(pCreateInfo)),
      full_range{MakeImageFullRange(*pCreateInfo)},
      create_from_swapchain(swapchain),
      swapchain_image_index(swapchain_index),
      format_features(ff),
      disjoint((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0),
      requirements{},
      memory_requirements_checked{false, false, false},
      sparse_requirements{},
      sparse_metadata_required(false),
      get_sparse_reqs_called(false),
      sparse_metadata_bound(false),
      subresource_encoder(full_range),
      fragment_encoder(nullptr),
      store_device_as_workaround(dev_data->device) {  // TODO REMOVE WHEN encoder can be const
    fragment_encoder =
        std::unique_ptr<const subresource_adapter::ImageRangeEncoder>(new subresource_adapter::ImageRangeEncoder(*this));
    // TODO: Correctly load resource size individually
    // Ideally we would have memory tracking as part of the object and not inherited due to its limitations
    resource_size_ = requirements[0].size;
}

void IMAGE_STATE::Destroy() {
    // NOTE: due to corner cases in aliased images, the layout_range_map MUST not be cleaned up here.
    // If it is, bad local entries could be created by CMD_BUFFER_STATE::GetImageSubresourceLayoutMap()
    // If an aliasing image was being destroyed (and layout_range_map was reset()), a nullptr keyed
    // entry could get put into CMD_BUFFER_STATE::aliased_image_layout_map.
    if (bind_swapchain) {
        bind_swapchain->RemoveParent(this);
        bind_swapchain = nullptr;
    }
    BINDABLE::Destroy();
}

void IMAGE_STATE::NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) {
    BINDABLE::NotifyInvalidate(invalid_nodes, unlink);
    if (unlink) {
        bind_swapchain = nullptr;
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
    if (!IsSwapchainImage() && !other_image_state->IsSwapchainImage() &&
        !(createInfo.flags & other_image_state->createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT)) {
        return false;
    }
    const auto binding = Binding();
    const auto other_binding = other_image_state->Binding();
    if ((create_from_swapchain == VK_NULL_HANDLE) && binding && other_binding && (binding->mem_state == other_binding->mem_state) &&
        (binding->memory_offset == other_binding->memory_offset) && IsCreateInfoEqual(other_image_state->createInfo)) {
        return true;
    }
    if (bind_swapchain && (bind_swapchain == other_image_state->bind_swapchain) &&
        (swapchain_image_index == other_image_state->swapchain_image_index)) {
        return true;
    }
    return false;
}

void IMAGE_STATE::SetInitialLayoutMap() {
    if (layout_range_map) {
        return;
    }
    if ((createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT) != 0) {
        const auto *binding = Binding();
        assert(binding);
        // Look for another aliasing image and point at its layout state.
        // ObjectBindings() is thread safe since returns by value, and once
        // the weak_ptr is successfully locked, the other image state won't
        // be freed out from under us.
        for (auto &entry : binding->mem_state->ObjectBindings()) {
            if (entry.first.type == kVulkanObjectTypeImage) {
                auto base_node = entry.second.lock();
                if (base_node) {
                    auto other_image = static_cast<IMAGE_STATE *>(base_node.get());
                    if (other_image != this && other_image->IsCompatibleAliasing(this)) {
                        layout_range_map = other_image->layout_range_map;
                        break;
                    }
                }
            }
        }
    } else if (bind_swapchain) {
        // Swapchains can also alias if multiple images are bound (or retrieved
        // with vkGetSwapchainImages()) for a (single swapchain, index) pair.
        // ObjectBindings() is thread safe since returns by value, and once
        // the weak_ptr is successfully locked, the other image state won't
        // be freed out from under us.
        for (auto &entry : bind_swapchain->ObjectBindings()) {
            if (entry.first.type == kVulkanObjectTypeImage) {
                auto base_node = entry.second.lock();
                if (base_node) {
                    auto other_image = static_cast<IMAGE_STATE *>(base_node.get());
                    if (other_image != this && other_image->IsCompatibleAliasing(this)) {
                        layout_range_map = other_image->layout_range_map;
                        break;
                    }
                }
            }
        }
    }
    // ... otherwise set up the new map.
    if (!layout_range_map) {
        // set up the new map completely before making it available
        auto new_map = std::make_shared<GlobalImageLayoutRangeMap>(subresource_encoder.SubresourceCount());
        auto range_gen = subresource_adapter::RangeGenerator(subresource_encoder);
        for (; range_gen->non_empty(); ++range_gen) {
            new_map->insert(new_map->end(), std::make_pair(*range_gen, createInfo.initialLayout));
        }
        layout_range_map = std::move(new_map);
    }
}

void IMAGE_STATE::SetSwapchain(std::shared_ptr<SWAPCHAIN_NODE> &swapchain, uint32_t swapchain_index) {
    assert(IsSwapchainImage());
    bind_swapchain = swapchain;
    swapchain_image_index = swapchain_index;
    bind_swapchain->AddParent(this);
}

VkDeviceSize IMAGE_STATE::GetFakeBaseAddress() const {
    if (!IsSwapchainImage()) {
        return BINDABLE::GetFakeBaseAddress();
    }
    if (!bind_swapchain) {
        return 0;
    }
    return bind_swapchain->images[swapchain_image_index].fake_base_address;
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

static VkSamplerYcbcrConversion GetSamplerConversion(const VkImageViewCreateInfo *ci) {
    auto *conversion_info = LvlFindInChain<VkSamplerYcbcrConversionInfo>(ci->pNext);
    return conversion_info ? conversion_info->conversion : VK_NULL_HANDLE;
}

static VkImageUsageFlags GetInheritedUsage(const VkImageViewCreateInfo *ci, const IMAGE_STATE &image_state) {
    auto usage_create_info = LvlFindInChain<VkImageViewUsageCreateInfo>(ci->pNext);
    return (usage_create_info) ? usage_create_info->usage : image_state.createInfo.usage;
}

static float GetImageViewMinLod(const VkImageViewCreateInfo* ci) {
    auto image_view_min_lod = LvlFindInChain<VkImageViewMinLodCreateInfoEXT>(ci->pNext);
    return (image_view_min_lod) ? image_view_min_lod->minLod : 0.0f;
}

IMAGE_VIEW_STATE::IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &im, VkImageView iv, const VkImageViewCreateInfo *ci,
                                   VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props)
    : BASE_NODE(iv, kVulkanObjectTypeImageView),
      safe_create_info(ci),
      create_info(*safe_create_info.ptr()),
      normalized_subresource_range(::NormalizeSubresourceRange(im->createInfo, *ci)),
      range_generator(im->subresource_encoder, normalized_subresource_range),
      samples(im->createInfo.samples),
      // When the image has a external format the views format must be VK_FORMAT_UNDEFINED and it is required to use a sampler
      // Ycbcr conversion. Thus we can't extract any meaningful information from the format parameter. As a Sampler Ycbcr
      // conversion must be used the shader type is always float.
      descriptor_format_bits(im->HasAHBFormat() ? static_cast<unsigned>(DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT)
                                                : DescriptorRequirementsBitsFromFormat(ci->format)),
      samplerConversion(GetSamplerConversion(ci)),
      filter_cubic_props(cubic_props),
      min_lod(GetImageViewMinLod(ci)),
      format_features(ff),
      inherited_usage(GetInheritedUsage(ci, *im)),
      image_state(im) {}

void IMAGE_VIEW_STATE::Destroy() {
    if (image_state) {
        image_state->RemoveParent(this);
        image_state = nullptr;
    }
    BASE_NODE::Destroy();
}

bool IMAGE_VIEW_STATE::IsDepthSliced() const { return ::IsDepthSliced(image_state->createInfo, create_info); }

VkOffset3D IMAGE_VIEW_STATE::GetOffset() const {
    VkOffset3D result = {0, 0, 0};
    if (IsDepthSliced()) {
        result.z = create_info.subresourceRange.baseArrayLayer;
    }
    return result;
}

VkExtent3D IMAGE_VIEW_STATE::GetExtent() const {
    VkExtent3D result = image_state->createInfo.extent;
    if (IsDepthSliced()) {
        result.depth = create_info.subresourceRange.layerCount;
    }
    return result;
}

uint32_t IMAGE_VIEW_STATE::GetAttachmentLayerCount() const {
    if (create_info.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS && !IsDepthSliced()) {
        return image_state->createInfo.arrayLayers;
    }
    return create_info.subresourceRange.layerCount;
}

static safe_VkImageCreateInfo GetImageCreateInfo(const VkSwapchainCreateInfoKHR *pCreateInfo) {
    auto image_ci = LvlInitStruct<VkImageCreateInfo>();
    // Pull out the format list only. This stack variable will get copied onto the heap
    // by the 'safe' constructor used to build the return value below.
    VkImageFormatListCreateInfo fmt_info;
    auto chain_fmt_info = LvlFindInChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
    if (chain_fmt_info) {
        fmt_info = *chain_fmt_info;
        fmt_info.pNext = nullptr;
        image_ci.pNext = &fmt_info;
    } else {
        image_ci.pNext = nullptr;
    }
    image_ci.flags = 0;  // to be updated below
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = pCreateInfo->imageFormat;
    image_ci.extent.width = pCreateInfo->imageExtent.width;
    image_ci.extent.height = pCreateInfo->imageExtent.height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = pCreateInfo->imageArrayLayers;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = pCreateInfo->imageUsage;
    image_ci.sharingMode = pCreateInfo->imageSharingMode;
    image_ci.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
    image_ci.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) {
        image_ci.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    }
    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) {
        image_ci.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }
    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) {
        image_ci.flags |= (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
    }
    return safe_VkImageCreateInfo(&image_ci);
}

SWAPCHAIN_NODE::SWAPCHAIN_NODE(ValidationStateTracker *dev_data_, const VkSwapchainCreateInfoKHR *pCreateInfo,
                               VkSwapchainKHR swapchain)
    : BASE_NODE(swapchain, kVulkanObjectTypeSwapchainKHR),
      createInfo(pCreateInfo),
      shared_presentable(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR == pCreateInfo->presentMode ||
                         VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR == pCreateInfo->presentMode),
      image_create_info(GetImageCreateInfo(pCreateInfo)),
      dev_data(dev_data_) {}

void SWAPCHAIN_NODE::PresentImage(uint32_t image_index) {
    if (image_index >= images.size()) return;
    assert(acquired_images > 0);
    acquired_images--;
    images[image_index].acquired = false;
    if (shared_presentable) {
        IMAGE_STATE *image_state = images[image_index].image_state;
        if (image_state) {
            image_state->layout_locked = true;
        }
    }
}

void SWAPCHAIN_NODE::AcquireImage(uint32_t image_index) {
    if (image_index >= images.size()) return;

    assert(acquired_images < std::numeric_limits<uint32_t>::max());
    acquired_images++;
    images[image_index].acquired = true;
    if (shared_presentable) {
        IMAGE_STATE *image_state = images[image_index].image_state;
        if (image_state) {
            image_state->shared_presentable = shared_presentable;
        }
    }
}

void SWAPCHAIN_NODE::Destroy() {
    for (auto &swapchain_image : images) {
        if (swapchain_image.image_state) {
            RemoveParent(swapchain_image.image_state);
            dev_data->Destroy<IMAGE_STATE>(swapchain_image.image_state->image());
        }
        // NOTE: We don't have access to dev_data->fake_memory.Free() here, but it is currently a no-op
    }
    images.clear();
    if (surface) {
        surface->RemoveParent(this);
        surface = nullptr;
    }
    BASE_NODE::Destroy();
}

void SWAPCHAIN_NODE::NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) {
    BASE_NODE::NotifyInvalidate(invalid_nodes, unlink);
    if (unlink) {
        surface = nullptr;
    }
}

void SURFACE_STATE::Destroy() {
    if (swapchain) {
        swapchain = nullptr;
    }
    BASE_NODE::Destroy();
}

void SURFACE_STATE::RemoveParent(BASE_NODE *parent_node) {
    if (swapchain == parent_node) {
        swapchain = nullptr;
    }
    BASE_NODE::RemoveParent(parent_node);
}

void SURFACE_STATE::SetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi, bool supported) {
    auto guard = Lock();
    assert(phys_dev);
    GpuQueue key{phys_dev, qfi};
    gpu_queue_support_[key] = supported;
}

bool SURFACE_STATE::GetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi) const {
    auto guard = Lock();
    assert(phys_dev);
    GpuQueue key{phys_dev, qfi};
    auto iter = gpu_queue_support_.find(key);
    if (iter != gpu_queue_support_.end()) {
        return iter->second;
    }
    VkBool32 supported = VK_FALSE;
    DispatchGetPhysicalDeviceSurfaceSupportKHR(phys_dev, qfi, surface(), &supported);
    gpu_queue_support_[key] = (supported == VK_TRUE);
    return supported == VK_TRUE;
}

void SURFACE_STATE::SetPresentModes(VkPhysicalDevice phys_dev, std::vector<VkPresentModeKHR> &&modes) {
    auto guard = Lock();
    assert(phys_dev);
    present_modes_[phys_dev] = std::move(modes);
}

std::vector<VkPresentModeKHR> SURFACE_STATE::GetPresentModes(VkPhysicalDevice phys_dev) const {
    auto guard = Lock();
    assert(phys_dev);
    auto iter = present_modes_.find(phys_dev);
    if (iter != present_modes_.end()) {
        return iter->second;
    }
    std::vector<VkPresentModeKHR> result;
    uint32_t count = 0;
    DispatchGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface(), &count, nullptr);
    result.resize(count);
    DispatchGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface(), &count, result.data());
    return result;
}

void SURFACE_STATE::SetFormats(VkPhysicalDevice phys_dev, std::vector<VkSurfaceFormatKHR> &&fmts) {
    auto guard = Lock();
    assert(phys_dev);
    formats_[phys_dev] = std::move(fmts);
}

std::vector<VkSurfaceFormatKHR> SURFACE_STATE::GetFormats(VkPhysicalDevice phys_dev) const {
    auto guard = Lock();
    assert(phys_dev);
    auto iter = formats_.find(phys_dev);
    if (iter != formats_.end()) {
        return iter->second;
    }
    std::vector<VkSurfaceFormatKHR> result;
    uint32_t count = 0;
    DispatchGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface(), &count, nullptr);
    result.resize(count);
    DispatchGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface(), &count, result.data());
    formats_[phys_dev] = result;
    return result;
}

void SURFACE_STATE::SetCapabilities(VkPhysicalDevice phys_dev, const VkSurfaceCapabilitiesKHR &caps) {
    auto guard = Lock();
    assert(phys_dev);
    capabilities_[phys_dev] = caps;
}

VkSurfaceCapabilitiesKHR SURFACE_STATE::GetCapabilities(VkPhysicalDevice phys_dev) const {
    auto guard = Lock();
    assert(phys_dev);
    auto iter = capabilities_.find(phys_dev);
    if (iter != capabilities_.end()) {
        return iter->second;
    }
    VkSurfaceCapabilitiesKHR result{};
    DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface(), &result);
    capabilities_[phys_dev] = result;
    return result;
}
