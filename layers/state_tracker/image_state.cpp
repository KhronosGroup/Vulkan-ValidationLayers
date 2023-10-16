/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
 */
#include "state_tracker/image_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/descriptor_sets.h"
#include <limits>
#include <string_view>

static VkImageSubresourceRange MakeImageFullRange(const VkImageCreateInfo &create_info) {
    const auto format = create_info.format;
    VkImageSubresourceRange init_range{0, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

    if (vkuFormatIsColor(format) || vkuFormatIsMultiplane(format) || GetExternalFormat(create_info.pNext) != 0) {
        init_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // Normalization will expand this for multiplane
    } else {
        init_range.aspectMask =
            (vkuFormatHasDepth(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) | (vkuFormatHasStencil(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    }
    return NormalizeSubresourceRange(create_info, init_range);
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info,
                                                  const VkImageSubresourceRange &range) {
    VkImageSubresourceRange norm = range;
    norm.levelCount = ResolveRemainingLevels(image_create_info, range);
    norm.layerCount = ResolveRemainingLayers(image_create_info, range);

    // For multiplanar formats, IMAGE_ASPECT_COLOR is equivalent to adding the aspect of the individual planes
    if (vkuFormatIsMultiplane(image_create_info.format)) {
        if (norm.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
            norm.aspectMask &= ~VK_IMAGE_ASPECT_COLOR_BIT;
            norm.aspectMask |= (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
            if (vkuFormatPlaneCount(image_create_info.format) > 2) {
                norm.aspectMask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
            }
        }
    }
    return norm;
}

static bool IsDepthSliced(const VkImageCreateInfo &image_create_info, const VkImageViewCreateInfo &create_info) {
    auto kDepthSlicedFlags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;
    return ((image_create_info.flags & kDepthSlicedFlags) != 0) &&
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

static VkExternalMemoryHandleTypeFlags GetExternalHandleTypes(const VkImageCreateInfo *pCreateInfo) {
    const auto *external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkSwapchainKHR GetSwapchain(const VkImageCreateInfo *pCreateInfo) {
    const auto *swapchain_info = vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    return swapchain_info ? swapchain_info->swapchain : VK_NULL_HANDLE;
}

static IMAGE_STATE::MemoryReqs GetMemoryRequirements(const ValidationStateTracker *dev_data, VkImage img,
                                                     const VkImageCreateInfo *create_info, bool disjoint, bool is_external_ahb) {
    IMAGE_STATE::MemoryReqs result{};
    // Record the memory requirements in case they won't be queried
    // External AHB memory can't be queried until after memory is bound
    if (!is_external_ahb) {
        if (disjoint == false) {
            DispatchGetImageMemoryRequirements(dev_data->device, img, &result[0]);
        } else {
            uint32_t plane_count = vkuFormatPlaneCount(create_info->format);
            static const std::array<VkImageAspectFlagBits, 3> aspects{VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT,
                                                                      VK_IMAGE_ASPECT_PLANE_2_BIT};
            assert(plane_count <= aspects.size());
            VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
            VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
            mem_req_info2.image = img;

            for (uint32_t i = 0; i < plane_count; i++) {
                VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();

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

static IMAGE_STATE::SparseReqs GetSparseRequirements(const ValidationStateTracker *dev_data, VkImage img, bool sparse_residency) {
    IMAGE_STATE::SparseReqs result;
    if (sparse_residency) {
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
#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkImageCreateInfo *info, VkExportMetalObjectTypeFlagBitsEXT object_type_required) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == object_type_required) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

IMAGE_STATE::IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
                         VkFormatFeatureFlags2KHR ff)
    : BINDABLE(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      shared_presentable(false),
      layout_locked(false),
      ahb_format(GetExternalFormat(pCreateInfo->pNext)),
      full_range{MakeImageFullRange(*pCreateInfo)},
      create_from_swapchain(GetSwapchain(pCreateInfo)),
      owned_by_swapchain(false),
      swapchain_image_index(0),
      format_features(ff),
      disjoint((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0),
      requirements(GetMemoryRequirements(dev_data, img, pCreateInfo, disjoint, IsExternalBuffer())),
      sparse_residency((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0),
      sparse_requirements(GetSparseRequirements(dev_data, img, sparse_residency)),
      sparse_metadata_required(SparseMetaDataRequired(sparse_requirements)),
      get_sparse_reqs_called(false),
      sparse_metadata_bound(false),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_image_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT)),
      metal_io_surface_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      subresource_encoder(full_range),
      fragment_encoder(nullptr),
      store_device_as_workaround(dev_data->device),  // TODO REMOVE WHEN encoder can be const
      supported_video_profiles(dev_data->video_profile_cache_.Get(
          dev_data, vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext))) {
    if (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) {
        bool is_resident = (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0;
        tracker_.emplace<BindableSparseMemoryTracker>(requirements.data(), is_resident);
        SetMemoryTracker(&std::get<BindableSparseMemoryTracker>(tracker_));
    } else if (pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) {
        tracker_.emplace<BindableMultiplanarMemoryTracker>(requirements.data(), vkuFormatPlaneCount(pCreateInfo->format));
        SetMemoryTracker(&std::get<BindableMultiplanarMemoryTracker>(tracker_));
    } else {
        tracker_.emplace<BindableLinearMemoryTracker>(requirements.data());
        SetMemoryTracker(&std::get<BindableLinearMemoryTracker>(tracker_));
    }
}

IMAGE_STATE::IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
                         VkSwapchainKHR swapchain, uint32_t swapchain_index, VkFormatFeatureFlags2KHR ff)
    : BINDABLE(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      shared_presentable(false),
      layout_locked(false),
      ahb_format(GetExternalFormat(pCreateInfo->pNext)),
      full_range{MakeImageFullRange(*pCreateInfo)},
      create_from_swapchain(swapchain),
      owned_by_swapchain(true),
      swapchain_image_index(swapchain_index),
      format_features(ff),
      disjoint((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0),
      requirements{},
      sparse_residency(false),
      sparse_requirements{},
      sparse_metadata_required(false),
      get_sparse_reqs_called(false),
      sparse_metadata_bound(false),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_image_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT)),
      metal_io_surface_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      subresource_encoder(full_range),
      fragment_encoder(nullptr),
      store_device_as_workaround(dev_data->device),  // TODO REMOVE WHEN encoder can be const
      supported_video_profiles(dev_data->video_profile_cache_.Get(
          dev_data, vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext))) {
    fragment_encoder =
        std::unique_ptr<const subresource_adapter::ImageRangeEncoder>(new subresource_adapter::ImageRangeEncoder(*this));

    tracker_.emplace<BindableNoMemoryTracker>(requirements.data());
    SetMemoryTracker(&std::get<BindableNoMemoryTracker>(tracker_));
}

void IMAGE_STATE::Destroy() {
    // NOTE: due to corner cases in aliased images, the layout_range_map MUST not be cleaned up here.
    // If it is, bad local entries could be created by CMD_BUFFER_STATE::GetImageSubresourceLayoutMap()
    // If an aliasing image was being destroyed (and layout_range_map was reset()), a nullptr keyed
    // entry could get put into CMD_BUFFER_STATE::aliased_image_layout_map.
    //
    // NOTE: the fragment_encoder should not be cleaned-up in case a semaphore to an acquired image is being processed
    //       after the swapchain is waited, and the range generation needs an intact encoder.
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

bool IMAGE_STATE::IsCompatibleAliasing(const IMAGE_STATE *other_image_state) const {
    if (!IsSwapchainImage() && !other_image_state->IsSwapchainImage() &&
        !(createInfo.flags & other_image_state->createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT)) {
        return false;
    }
    const auto binding = Binding();
    const auto other_binding = other_image_state->Binding();
    if ((create_from_swapchain == VK_NULL_HANDLE) && binding && other_binding &&
        (binding->memory_state == other_binding->memory_state) && (binding->memory_offset == other_binding->memory_offset) &&
        IsCreateInfoEqual(other_image_state->createInfo)) {
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

    std::shared_ptr<GlobalImageLayoutRangeMap> layout_map;
    auto get_layout_map = [&layout_map](const IMAGE_STATE &other_image) {
        layout_map = other_image.layout_range_map;
        return true;
    };

    // See if an alias already has a layout map
    if (HasAliasFlag()) {
        AnyImageAliasOf(get_layout_map);
    } else if (bind_swapchain) {
        // Swapchains can also alias if multiple images are bound (or retrieved
        // with vkGetSwapchainImages()) for a (single swapchain, index) pair.
        AnyAliasBindingOf(bind_swapchain->ObjectBindings(), get_layout_map);
    }

    if (!layout_map) {
        // otherwise set up a new map.
        // set up the new map completely before making it available
        layout_map = std::make_shared<GlobalImageLayoutRangeMap>(subresource_encoder.SubresourceCount());
        auto range_gen = subresource_adapter::RangeGenerator(subresource_encoder);
        for (; range_gen->non_empty(); ++range_gen) {
            layout_map->insert(layout_map->end(), std::make_pair(*range_gen, createInfo.initialLayout));
        }
    }
    // And store in the object
    layout_range_map = std::move(layout_map);
}

void IMAGE_STATE::SetImageLayout(const VkImageSubresourceRange &range, VkImageLayout layout) {
    using sparse_container::update_range_value;
    using sparse_container::value_precedence;
    GlobalImageLayoutRangeMap::RangeGenerator range_gen(subresource_encoder, NormalizeSubresourceRange(range));
    auto guard = layout_range_map->WriteLock();
    for (; range_gen->non_empty(); ++range_gen) {
        update_range_value(*layout_range_map, *range_gen, layout, value_precedence::prefer_source);
    }
}

void IMAGE_STATE::SetSwapchain(std::shared_ptr<SWAPCHAIN_NODE> &swapchain, uint32_t swapchain_index) {
    assert(IsSwapchainImage());
    bind_swapchain = swapchain;
    swapchain_image_index = swapchain_index;
    bind_swapchain->AddParent(this);
}

static VkSamplerYcbcrConversion GetSamplerConversion(const VkImageViewCreateInfo *ci) {
    auto *conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(ci->pNext);
    return conversion_info ? conversion_info->conversion : VK_NULL_HANDLE;
}

static VkImageUsageFlags GetInheritedUsage(const VkImageViewCreateInfo *ci, const IMAGE_STATE &image_state) {
    auto usage_create_info = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(ci->pNext);
    return (usage_create_info) ? usage_create_info->usage : image_state.createInfo.usage;
}

static float GetImageViewMinLod(const VkImageViewCreateInfo *ci) {
    auto image_view_min_lod = vku::FindStructInPNextChain<VkImageViewMinLodCreateInfoEXT>(ci->pNext);
    return (image_view_min_lod) ? image_view_min_lod->minLod : 0.0f;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkImageViewCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

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
      descriptor_format_bits(im->HasAHBFormat() ? static_cast<unsigned>(NumericTypeFloat) : GetFormatType(ci->format)),
      samplerConversion(GetSamplerConversion(ci)),
      filter_cubic_props(cubic_props),
      min_lod(GetImageViewMinLod(ci)),
      format_features(ff),
      inherited_usage(GetInheritedUsage(ci, *im)),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_imageview_export(GetMetalExport(ci)),
#endif
      image_state(im),
      is_depth_sliced(::IsDepthSliced(im->createInfo, *ci)) {
}

void IMAGE_VIEW_STATE::Destroy() {
    if (image_state) {
        image_state->RemoveParent(this);
        image_state = nullptr;
    }
    BASE_NODE::Destroy();
}

uint32_t IMAGE_VIEW_STATE::GetAttachmentLayerCount() const {
    if (create_info.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS && !IsDepthSliced()) {
        return image_state->createInfo.arrayLayers;
    }
    return create_info.subresourceRange.layerCount;
}

bool IMAGE_VIEW_STATE::OverlapSubresource(const IMAGE_VIEW_STATE &compare_view) const {
    if (image_view() == compare_view.image_view()) {
        return true;
    }
    if (image_state->image() != compare_view.image_state->image()) {
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

static safe_VkImageCreateInfo GetImageCreateInfo(const VkSwapchainCreateInfoKHR *pCreateInfo) {
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    // Pull out the format list only. This stack variable will get copied onto the heap
    // by the 'safe' constructor used to build the return value below.
    VkImageFormatListCreateInfo fmt_info;
    auto chain_fmt_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(pCreateInfo->pNext);
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
      images(),
      exclusive_full_screen_access(false),
      shared_presentable(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR == pCreateInfo->presentMode ||
                         VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR == pCreateInfo->presentMode),
      image_create_info(GetImageCreateInfo(pCreateInfo)),
      dev_data(dev_data_) {}

void SWAPCHAIN_NODE::PresentImage(uint32_t image_index, uint64_t present_id) {
    if (image_index >= images.size()) return;
    assert(acquired_images > 0);
    if (!shared_presentable) {
        acquired_images--;
        images[image_index].acquired = false;
    } else {
        IMAGE_STATE *image_state = images[image_index].image_state;
        if (image_state) {
            image_state->layout_locked = true;
        }
    }
    if (present_id > max_present_id) {
        max_present_id = present_id;
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

SWAPCHAIN_IMAGE SWAPCHAIN_NODE::GetSwapChainImage(uint32_t index) const {
    if (index < images.size()) {
        return images[index];
    }
    return SWAPCHAIN_IMAGE();
}

std::shared_ptr<const IMAGE_STATE> SWAPCHAIN_NODE::GetSwapChainImageShared(uint32_t index) const {
    const SWAPCHAIN_IMAGE swapchain_image(GetSwapChainImage(index));
    if (swapchain_image.image_state) {
        return swapchain_image.image_state->shared_from_this();
    }
    return std::shared_ptr<const IMAGE_STATE>();
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

// Save data from vkGetPhysicalDeviceSurfacePresentModes
void SURFACE_STATE::SetPresentModes(VkPhysicalDevice phys_dev, vvl::span<const VkPresentModeKHR> modes) {
    auto guard = Lock();
    assert(phys_dev);
    for (auto new_present_mode : modes) {
        if ((present_modes_data_.find(phys_dev) == present_modes_data_.end()) ||
            (present_modes_data_[phys_dev].find(new_present_mode) == present_modes_data_[phys_dev].end())) {
            present_modes_data_[phys_dev][new_present_mode] = std::nullopt;
        }
    }
}

// Helper for data obtained from vkGetPhysicalDeviceSurfacePresentModesKHR
std::vector<VkPresentModeKHR> SURFACE_STATE::GetPresentModes(VkPhysicalDevice phys_dev,
                                                             const ValidationObject *validation_obj) const {
    auto guard = Lock();
    assert(phys_dev);
    std::vector<VkPresentModeKHR> result;
    if (auto search = present_modes_data_.find(phys_dev); search != present_modes_data_.end()) {
        for (auto mode = search->second.begin(); mode != search->second.end(); mode++) {
            result.push_back(mode->first);
        }
        return result;
    }

    const auto log_internal_error = [validation_obj](VkResult err, auto &&...objects) {
        if (validation_obj) {
            LogObjectList obj_list(std::forward<decltype(objects)>(objects)...);
            validation_obj->LogInternalError(VVL_PRETTY_FUNCTION, obj_list, "vkGetPhysicalDeviceSurfacePresentModesKHR", err);
        }
    };

    uint32_t count = 0;
    if (const VkResult err = DispatchGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface(), &count, nullptr);
        !IsValueIn(err, {VK_SUCCESS, VK_INCOMPLETE})) {
        log_internal_error(err, phys_dev, surface());
        return result;
    }
    result.resize(count);
    if (const VkResult err = DispatchGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface(), &count, result.data());
        err != VK_SUCCESS) {
        log_internal_error(err, phys_dev, surface());
        return result;
    }
    return result;
}

void SURFACE_STATE::SetFormats(VkPhysicalDevice phys_dev, std::vector<safe_VkSurfaceFormat2KHR> &&fmts) {
    auto guard = Lock();
    assert(phys_dev);
    formats_[phys_dev] = std::move(fmts);
}

vvl::span<const safe_VkSurfaceFormat2KHR> SURFACE_STATE::GetFormats(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                                    const void *surface_info2_pnext,
                                                                    const ValidationObject *validation_obj) const {
    auto guard = Lock();
    assert(phys_dev);

    if (const auto search = formats_.find(phys_dev); search != formats_.end()) {
        vvl::span<const safe_VkSurfaceFormat2KHR>(search->second);
    }

    std::vector<safe_VkSurfaceFormat2KHR> result;
    if (get_surface_capabilities2) {
        const auto log_internal_error = [validation_obj](VkResult err, auto &&...objects) {
            if (validation_obj) {
                LogObjectList obj_list(std::forward<decltype(objects)>(objects)...);
                validation_obj->LogInternalError(VVL_PRETTY_FUNCTION, obj_list, "vkGetPhysicalDeviceSurfaceFormats2KHR", err);
            }
        };

        uint32_t count = 0;
        const auto surface_info2 = GetSurfaceInfo2(surface_info2_pnext);
        if (const VkResult err = DispatchGetPhysicalDeviceSurfaceFormats2KHR(phys_dev, &surface_info2, &count, nullptr);
            !IsValueIn(err, {VK_SUCCESS, VK_INCOMPLETE})) {
            log_internal_error(err, phys_dev, surface_info2.surface);
            return result;
        }
        std::vector<VkSurfaceFormat2KHR> formats2(count, vku::InitStruct<VkSurfaceFormat2KHR>());

        if (const VkResult err = DispatchGetPhysicalDeviceSurfaceFormats2KHR(phys_dev, &surface_info2, &count, formats2.data());
            err != VK_SUCCESS) {
            log_internal_error(err, phys_dev, surface_info2.surface);
            result.clear();
        } else {
            result.resize(count);
            for (uint32_t surface_format_index = 0; surface_format_index < count; ++surface_format_index) {
                result.emplace_back(safe_VkSurfaceFormat2KHR(&formats2[surface_format_index]));
            }
        }

    } else {
        const auto log_internal_error = [validation_obj](VkResult err, auto &&...objects) {
            if (validation_obj) {
                LogObjectList obj_list(std::forward<decltype(objects)>(objects)...);
                validation_obj->LogInternalError(VVL_PRETTY_FUNCTION, obj_list, "vkGetPhysicalDeviceSurfaceFormatsKHR", err);
            }
        };

        std::vector<VkSurfaceFormatKHR> formats;
        uint32_t count = 0;
        if (const VkResult err = DispatchGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface(), &count, nullptr);
            !IsValueIn(err, {VK_SUCCESS, VK_INCOMPLETE})) {
            log_internal_error(err, phys_dev, surface());
            return result;
        }
        formats.resize(count);

        if (const VkResult err = DispatchGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface(), &count, formats.data());
            err != VK_SUCCESS) {
            log_internal_error(err, phys_dev, surface());
            result.clear();
        } else {
            result.reserve(count);
            VkSurfaceFormat2KHR format2 = vku::InitStructHelper();
            for (const auto &format : formats) {
                format2.surfaceFormat = format;
                result.emplace_back(safe_VkSurfaceFormat2KHR(&format2));
            }
        }
    }
    formats_[phys_dev] = std::move(result);
    return vvl::span<const safe_VkSurfaceFormat2KHR>(formats_[phys_dev]);
}

void SURFACE_STATE::SetCapabilities(VkPhysicalDevice phys_dev, const safe_VkSurfaceCapabilities2KHR &caps) {
    auto guard = Lock();
    assert(phys_dev);
    capabilities_[phys_dev] = caps;
}

safe_VkSurfaceCapabilities2KHR SURFACE_STATE::GetCapabilities(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                              const void *surface_info2_pnext,
                                                              const ValidationObject *validation_obj) const {
    auto guard = Lock();
    assert(phys_dev);

    if (auto search = capabilities_.find(phys_dev); search != capabilities_.end()) {
        return search->second;
    }

    const auto log_internal_error = [validation_obj](VkResult err, auto &&...objects) {
        if (validation_obj) {
            LogObjectList obj_list(std::forward<decltype(objects)>(objects)...);
            validation_obj->LogInternalError(VVL_PRETTY_FUNCTION, obj_list, "vkGetPhysicalDeviceSurfaceCapabilities2KHR", err);
        }
    };

    VkSurfaceCapabilities2KHR surface_caps2 = vku::InitStructHelper();
    if (get_surface_capabilities2) {
        const auto surface_info2 = GetSurfaceInfo2(surface_info2_pnext);
        if (const VkResult err = DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info2, &surface_caps2);
            err != VK_SUCCESS) {
            log_internal_error(err, phys_dev, surface_info2.surface);
        }
    } else {
        VkSurfaceCapabilitiesKHR caps{};
        if (const VkResult err = DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface(), &caps); err != VK_SUCCESS) {
            log_internal_error(err, phys_dev, surface());
        }
        surface_caps2.surfaceCapabilities = caps;
    }
    safe_VkSurfaceCapabilities2KHR safe_surface_caps2(&surface_caps2);
    capabilities_[phys_dev] = safe_surface_caps2;
    return safe_surface_caps2;
}

void SURFACE_STATE::SetCompatibleModes(VkPhysicalDevice phys_dev, const VkPresentModeKHR present_mode,
                                       vvl::span<const VkPresentModeKHR> compatible_modes) {
    auto guard = Lock();
    assert(phys_dev);

    // If this surface or the present_mode is not in the map, or if the state structure has no value,
    // create and add the new present_mode state structure for each of the compatible modes
    auto surface_map = present_modes_data_.find(phys_dev);
    if ((surface_map == present_modes_data_.end()) || (surface_map->second.find(present_mode) == surface_map->second.end()) ||
        (surface_map->second.find(present_mode)->second.has_value() == false)) {
        auto present_mode_state = std::make_shared<PresentModeState>();
        present_mode_state->compatible_present_modes_.assign(compatible_modes.begin(), compatible_modes.end());

        // For every present mode in compatible modes, add present_mode_state for it in present_modes_data_
        for (auto mode : compatible_modes) {
            present_modes_data_[phys_dev][mode] = present_mode_state;
        }
    }
}

std::vector<VkPresentModeKHR> SURFACE_STATE::GetCompatibleModes(VkPhysicalDevice phys_dev,
                                                                const VkPresentModeKHR present_mode) const {
    auto guard = Lock();
    assert(phys_dev);
    auto iter = present_modes_data_.find(phys_dev);
    if ((iter != present_modes_data_.end()) && (iter->second.find(present_mode) != iter->second.end())) {
        if (((iter->second)[present_mode]).has_value()) {
            auto &compatible_modes = *(iter->second)[present_mode];
            if (compatible_modes->compatible_present_modes_.empty()) {
                return compatible_modes->compatible_present_modes_;
            }
        }
    }

    // Compatible modes not in state tracker, call to get compatible modes
    std::vector<VkPresentModeKHR> result;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = surface();
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    surface_info.pNext = &surface_present_mode;
    VkSurfacePresentModeCompatibilityEXT present_mode_compatibility = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_capabilities = vku::InitStructHelper();
    surface_capabilities.pNext = &present_mode_compatibility;
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_capabilities);
    result.resize(present_mode_compatibility.presentModeCount);
    present_mode_compatibility.pPresentModes = result.data();
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_capabilities);
    return result;
}

// Set the surface and scaling caps for this present mode
void SURFACE_STATE::SetPresentModeCapabilities(VkPhysicalDevice phys_dev, const VkPresentModeKHR present_mode,
                                               const VkSurfaceCapabilitiesKHR &caps,
                                               const VkSurfacePresentScalingCapabilitiesEXT &scaling_caps) {
    auto guard = Lock();
    assert(phys_dev);
    if (!present_modes_data_[phys_dev][present_mode].has_value()) {
        present_modes_data_[phys_dev][present_mode] = std::make_shared<PresentModeState>();
    }
    auto &present_mode_state = present_modes_data_[phys_dev][present_mode].value();
    present_mode_state->scaling_capabilities_ = scaling_caps;
    present_mode_state->surface_capabilities_ = caps;
}

// Get the surface caps this particular present mode
VkSurfaceCapabilitiesKHR SURFACE_STATE::GetPresentModeSurfaceCapabilities(VkPhysicalDevice phys_dev,
                                                                          const VkPresentModeKHR present_mode) const {
    auto iter = present_modes_data_.find(phys_dev);
    if ((iter != present_modes_data_.end()) && (iter->second.find(present_mode) != iter->second.end())) {
        auto const caps = (iter->second)[present_mode];
        if (caps.has_value()) {
            auto &surface_caps = *caps;
            return surface_caps->surface_capabilities_;
        }
    }

    // Present mode surface capabilties not in state tracker, call to get surface capabilities
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = surface();
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    surface_info.pNext = &surface_present_mode;
    VkSurfaceCapabilities2KHR surface_capabilities = vku::InitStructHelper();
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_capabilities);
    return surface_capabilities.surfaceCapabilities;
}

// Get the scaling capabilities for this particular present mode
VkSurfacePresentScalingCapabilitiesEXT SURFACE_STATE::GetPresentModeScalingCapabilities(VkPhysicalDevice phys_dev,
                                                                                        const VkPresentModeKHR present_mode) const {
    auto iter = present_modes_data_.find(phys_dev);
    if ((iter != present_modes_data_.end()) && (iter->second.find(present_mode) != iter->second.end())) {
        auto const &caps = (iter->second)[present_mode];
        if (caps.has_value()) {
            auto &scaling_caps = *caps;
            return scaling_caps->scaling_capabilities_;
        }
    }

    // Present mode scaling capabilties not in state tracker, call to get scaling capabilities
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = surface();
    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = present_mode;
    surface_info.pNext = &surface_present_mode;
    VkSurfacePresentScalingCapabilitiesEXT scaling_caps = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_capabilities = vku::InitStructHelper();
    surface_capabilities.pNext = &scaling_caps;
    DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info, &surface_capabilities);
    return scaling_caps;
}

bool GlobalImageLayoutRangeMap::AnyInRange(RangeGenerator &gen,
                                           std::function<bool(const key_type &range, const mapped_type &state)> &&func) const {
    for (; gen->non_empty(); ++gen) {
        for (auto pos = lower_bound(*gen); (pos != end()) && (gen->intersects(pos->first)); ++pos) {
            if (func(pos->first, pos->second)) {
                return true;
            }
        }
    }
    return false;
}
