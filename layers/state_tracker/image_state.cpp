/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#include <cmath>
#include <cstdint>
#include <string>
#include "error_message/error_strings.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/semaphore_state.h"
#include "state_tracker/wsi_state.h"
#include "generated/dispatch_functions.h"
#include "utils/math_utils.h"
#include "utils/image_utils.h"

using RangeGenerator = subresource_adapter::RangeGenerator;

static VkExternalMemoryHandleTypeFlags GetExternalHandleTypes(const VkImageCreateInfo *pCreateInfo) {
    const auto *external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkSwapchainKHR GetSwapchain(const VkImageCreateInfo *pCreateInfo) {
    const auto *swapchain_info = vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    return swapchain_info ? swapchain_info->swapchain : VK_NULL_HANDLE;
}

static vvl::Image::MemoryReqs GetMemoryRequirements(const vvl::DeviceState &dev_data, VkImage img,
                                                    const VkImageCreateInfo *create_info, bool disjoint, bool is_external_ahb) {
    vvl::Image::MemoryReqs result{};
    // Record the memory requirements in case they won't be queried
    // External AHB memory can't be queried until after memory is bound
    if (!is_external_ahb) {
        if (disjoint == false) {
            DispatchGetImageMemoryRequirements(dev_data.device, img, &result[0]);
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
                switch (dev_data.extensions.vk_khr_get_memory_requirements2) {
                    case kEnabledByApiLevel:
                        DispatchGetImageMemoryRequirements2(dev_data.device, &mem_req_info2, &mem_reqs2);
                        break;
                    case kEnabledByCreateinfo:
                        DispatchGetImageMemoryRequirements2KHR(dev_data.device, &mem_req_info2, &mem_reqs2);
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

static std::vector<VkSparseImageMemoryRequirements> GetSparseRequirements(const vvl::DeviceState &dev_data, VkImage img,
                                                                          bool sparse_residency) {
    std::vector<VkSparseImageMemoryRequirements> result;
    if (sparse_residency) {
        uint32_t count = 0;
        DispatchGetImageSparseMemoryRequirements(dev_data.device, img, &count, nullptr);
        result.resize(count);
        DispatchGetImageSparseMemoryRequirements(dev_data.device, img, &count, result.data());
    }
    return result;
}

static VkImageSubresourceRange MakeImageFullRange(const VkImageCreateInfo &create_info) {
    const VkFormat format = create_info.format;
    VkImageAspectFlags aspect_mask = 0;
    if (vkuFormatIsMultiplane(format)) {
        aspect_mask = NormalizeAspectMask(VK_IMAGE_ASPECT_COLOR_BIT, format);
    } else if (vkuFormatIsColor(format) || GetExternalFormat(create_info.pNext) != 0) {
        aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    } else {
        if (vkuFormatHasDepth(format)) {
            aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (vkuFormatHasStencil(format)) {
            aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    return VkImageSubresourceRange{aspect_mask, 0, create_info.mipLevels, 0, create_info.arrayLayers};
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

namespace vvl {

Image::Image(const vvl::DeviceState &dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo, VkFormatFeatureFlags2KHR ff)
    : Bindable(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      shared_presentable(false),
      layout_locked(false),
      ahb_format(GetExternalFormat(pCreateInfo->pNext)),
      full_range{MakeImageFullRange(create_info)},
      create_from_swapchain(GetSwapchain(pCreateInfo)),
      owned_by_swapchain(false),
      swapchain_image_index(0),
      format_features(ff),
      disjoint((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0),
      requirements(GetMemoryRequirements(dev_data, img, pCreateInfo, disjoint, IsExternalBuffer())),
      sparse_residency((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0),
      sparse_requirements(GetSparseRequirements(dev_data, img, sparse_residency)),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_image_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT)),
      metal_io_surface_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      subresource_encoder(GetSubresourceEncoderRange(dev_data, full_range)),
      store_device_as_workaround(dev_data.device),  // TODO REMOVE WHEN encoder can be const
      supported_video_profiles(dev_data.video_profile_cache_.Get(
          dev_data.physical_device, vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext))) {
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

Image::Image(const vvl::DeviceState &dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
             uint32_t swapchain_index, VkFormatFeatureFlags2KHR ff)
    : Bindable(img, kVulkanObjectTypeImage, (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      shared_presentable(false),
      layout_locked(false),
      ahb_format(GetExternalFormat(pCreateInfo->pNext)),
      full_range{MakeImageFullRange(create_info)},
      create_from_swapchain(swapchain),
      owned_by_swapchain(true),
      swapchain_image_index(swapchain_index),
      format_features(ff),
      disjoint((pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0),
      requirements{},
      sparse_residency(false),
      sparse_requirements{},
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_image_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT)),
      metal_io_surface_export(GetMetalExport(pCreateInfo, VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      subresource_encoder(GetSubresourceEncoderRange(dev_data, full_range)),
      store_device_as_workaround(dev_data.device),  // TODO REMOVE WHEN encoder can be const
      supported_video_profiles(dev_data.video_profile_cache_.Get(
          dev_data.physical_device, vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext))) {

    tracker_.emplace<BindableNoMemoryTracker>(requirements.data());
    SetMemoryTracker(&std::get<BindableNoMemoryTracker>(tracker_));
}

void Image::Destroy() {
    for (auto &item : sub_states_) {
        item.second->Destroy();
    }
    // NOTE: due to corner cases in aliased images, the layout_range_map MUST not be cleaned up here.
    // If it is, bad local entries could be created by vvl::CommandBuffer::GetOrCreateImageLayoutRegistry()
    // If an aliasing image was being destroyed (and layout_range_map was reset()), a nullptr keyed
    // entry could get put into vvl::CommandBuffer::aliased_image_layout_map.
    //
    if (bind_swapchain) {
        bind_swapchain->RemoveParent(this);
        bind_swapchain = nullptr;
    }
    Bindable::Destroy();
}

// Get buffer size from VkBufferImageCopy / VkBufferImageCopy2 structure, for a given format
template VkDeviceSize Image::GetBufferSizeFromCopyImage<VkBufferImageCopy>(const VkBufferImageCopy &) const;
template VkDeviceSize Image::GetBufferSizeFromCopyImage<VkBufferImageCopy2>(const VkBufferImageCopy2 &) const;

template <typename RegionType>
VkDeviceSize Image::GetBufferSizeFromCopyImage(const RegionType &region) const {
    VkDeviceSize buffer_size = 0;
    VkExtent3D copy_extent = region.imageExtent;
    VkDeviceSize buffer_width = (0 == region.bufferRowLength ? copy_extent.width : region.bufferRowLength);
    VkDeviceSize buffer_height = (0 == region.bufferImageHeight ? copy_extent.height : region.bufferImageHeight);
    uint32_t layer_count = region.imageSubresource.layerCount != VK_REMAINING_ARRAY_LAYERS
                               ? region.imageSubresource.layerCount
                               : create_info.arrayLayers - region.imageSubresource.baseArrayLayer;
    // VUID-VkImageCreateInfo-imageType-00961 prevents having both depth and layerCount ever both be greater than 1 together. Take
    // max to logic simple. This is the number of 'slices' to copy.
    const uint32_t z_copies = std::max(copy_extent.depth, layer_count);

    // Invalid if copy size is 0 and other validation checks will catch it. Returns zero as the caller should have fallback already
    // to ignore.
    if (copy_extent.width == 0 || copy_extent.height == 0 || copy_extent.depth == 0 || z_copies == 0) {
        return 0;
    }

    VkDeviceSize texel_block_size = 0;
    if (region.imageSubresource.aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
        // Spec in VkBufferImageCopy section list special cases for each format
        if (region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            texel_block_size = 1;
        } else {
            // VK_IMAGE_ASPECT_DEPTH_BIT
            switch (create_info.format) {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D16_UNORM_S8_UINT:
                    texel_block_size = 2;
                    break;
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // packed with the D24 value in the LSBs of the word, and undefined values in the eight MSBs
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                    texel_block_size = 4;
                    break;
                default:
                    // Any misuse of formats vs aspect mask should be caught before here
                    return 0;
            }
        }
    } else {
        const VkFormat compatible_format =
            vkuFormatIsMultiplane(create_info.format)
                ? vkuFindMultiplaneCompatibleFormat(create_info.format,
                                                    static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask))
                : create_info.format;
        texel_block_size = vkuFormatTexelBlockSize(compatible_format);
    }

    if (vkuFormatIsBlockedImage(create_info.format)) {
        // Switch to texel block units, rounding up for any partially-used blocks
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(create_info.format);
        buffer_width = (buffer_width + block_extent.width - 1) / block_extent.width;
        buffer_height = (buffer_height + block_extent.height - 1) / block_extent.height;

        copy_extent.width = (copy_extent.width + block_extent.width - 1) / block_extent.width;
        copy_extent.height = (copy_extent.height + block_extent.height - 1) / block_extent.height;
        copy_extent.depth = (copy_extent.depth + block_extent.depth - 1) / block_extent.depth;
    }

    // Calculate buffer offset of final copied byte, + 1.
    buffer_size = (z_copies - 1) * buffer_height * buffer_width;                   // offset to slice
    buffer_size += ((copy_extent.height - 1) * buffer_width) + copy_extent.width;  // add row,col
    buffer_size *= texel_block_size;                                               // convert to bytes
    return buffer_size;
}

void Image::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &item : sub_states_) {
        item.second->NotifyInvalidate(invalid_nodes, unlink);
    }
    Bindable::NotifyInvalidate(invalid_nodes, unlink);
    if (unlink) {
        bind_swapchain = nullptr;
    }
}

bool Image::IsCreateInfoEqual(const VkImageCreateInfo &other_create_info) const {
    bool is_equal = (create_info.sType == other_create_info.sType) && (create_info.flags == other_create_info.flags);
    is_equal = is_equal && IsImageTypeEqual(other_create_info) && IsFormatEqual(other_create_info);
    is_equal = is_equal && IsMipLevelsEqual(other_create_info) && IsArrayLayersEqual(other_create_info);
    is_equal = is_equal && IsUsageEqual(other_create_info) && IsInitialLayoutEqual(other_create_info);
    is_equal = is_equal && IsExtentEqual(other_create_info) && IsTilingEqual(other_create_info);
    is_equal = is_equal && IsSamplesEqual(other_create_info) && IsSharingModeEqual(other_create_info);
    return is_equal &&
           ((create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) ? IsQueueFamilyIndicesEqual(other_create_info) : true);
}

// Check image compatibility rules for VK_NV_dedicated_allocation_image_aliasing
bool Image::IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_create_info) const {
    bool is_compatible = (create_info.sType == other_create_info.sType) && (create_info.flags == other_create_info.flags);
    is_compatible = is_compatible && IsImageTypeEqual(other_create_info) && IsFormatEqual(other_create_info);
    is_compatible = is_compatible && IsMipLevelsEqual(other_create_info);
    is_compatible = is_compatible && IsUsageEqual(other_create_info) && IsInitialLayoutEqual(other_create_info);
    is_compatible = is_compatible && IsSamplesEqual(other_create_info) && IsSharingModeEqual(other_create_info);
    is_compatible = is_compatible &&
                    ((create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) ? IsQueueFamilyIndicesEqual(other_create_info) : true);
    is_compatible = is_compatible && IsTilingEqual(other_create_info);

    is_compatible = is_compatible && create_info.extent.width <= other_create_info.extent.width &&
                    create_info.extent.height <= other_create_info.extent.height &&
                    create_info.extent.depth <= other_create_info.extent.depth &&
                    create_info.arrayLayers <= other_create_info.arrayLayers;
    return is_compatible;
}

bool Image::IsCompatibleAliasing(const Image *other_image_state) const {
    if (!IsSwapchainImage() && !other_image_state->IsSwapchainImage() &&
        !(create_info.flags & other_image_state->create_info.flags & VK_IMAGE_CREATE_ALIAS_BIT)) {
        return false;
    }
    const auto binding = Binding();
    const auto other_binding = other_image_state->Binding();
    if ((create_from_swapchain == VK_NULL_HANDLE) && binding && other_binding &&
        (binding->memory_state == other_binding->memory_state) && (binding->memory_offset == other_binding->memory_offset) &&
        IsCreateInfoEqual(other_image_state->create_info)) {
        return true;
    }
    if (bind_swapchain && (bind_swapchain == other_image_state->bind_swapchain) &&
        (swapchain_image_index == other_image_state->swapchain_image_index)) {
        return true;
    }
    return false;
}

VkExtent3D Image::GetEffectiveSubresourceExtent(const VkImageSubresourceLayers &sub) const {
    return GetEffectiveExtent(create_info, sub.aspectMask, sub.mipLevel);
}

VkExtent3D Image::GetEffectiveSubresourceExtent(const VkImageSubresource &sub) const {
    return GetEffectiveExtent(create_info, sub.aspectMask, sub.mipLevel);
}

VkExtent3D Image::GetEffectiveSubresourceExtent(const VkImageSubresourceRange &range) const {
    return GetEffectiveExtent(create_info, range.aspectMask, range.baseMipLevel);
}

std::string Image::DescribeSubresourceLayers(const VkImageSubresourceLayers &subresource) const {
    std::stringstream ss;
    VkExtent3D subresource_extent = GetEffectiveSubresourceExtent(subresource);
    const VkFormat format = create_info.format;
    ss << "The " << string_VkImageType(create_info.imageType) << " VkImage was created with format " << string_VkFormat(format)
       << " and an extent of [" << string_VkExtent3D(create_info.extent) << "]\n";
    if (subresource.mipLevel != 0) {
        ss << "\tmipLevel " << subresource.mipLevel << " is [" << string_VkExtent3D(subresource_extent) << "]\n";
    }
    if (vkuFormatIsCompressed(format)) {
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(format);
        const VkExtent3D texel_blocks = GetTexelBlocks(subresource_extent, block_extent);
        ss << "\tThe compressed format block extent (" << string_VkExtent3D(block_extent) << ") represents miplevel "
           << subresource.mipLevel << " with a texel block extent [" << string_VkExtent3D(texel_blocks) << "]\n";
    } else if (vkuFormatIsMultiplane(format)) {
        assert(IsSingleBitSet(subresource.aspectMask));
        VkImageAspectFlagBits aspect_flag = static_cast<VkImageAspectFlagBits>(subresource.aspectMask);
        ss << "\tPlane " << vkuGetPlaneIndex(aspect_flag) << " (compatible format "
           << string_VkFormat(vkuFindMultiplaneCompatibleFormat(format, aspect_flag)) << ")";
        VkExtent2D divisors = vkuFindMultiplaneExtentDivisors(format, aspect_flag);
        if (divisors.width != 1 || divisors.height != 1) {
            ss << " has [widthDivisor = " << divisors.width << ", heightDivisor = " << divisors.height
               << "] which adjusts the extent to [" << string_VkExtent3D(subresource_extent) << "]";
        }
        ss << "\n";
    }
    return ss.str();
}

VkImageSubresourceRange Image::NormalizeSubresourceRange(const VkImageSubresourceRange &range) const {
    VkImageSubresourceRange norm = range;
    norm.levelCount = GetEffectiveLevelCount(range, create_info.mipLevels);
    norm.layerCount = GetEffectiveLayerCount(range, create_info.arrayLayers);
    norm.aspectMask = NormalizeAspectMask(range.aspectMask, create_info.format);
    return norm;
}

uint32_t Image::NormalizeLayerCount(const VkImageSubresourceLayers &resource) const {
    return (resource.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (create_info.arrayLayers - resource.baseArrayLayer)
                                                              : resource.layerCount;
}

VkImageSubresourceRange Image::GetSubresourceEncoderRange(const DeviceState &device_state,
                                                          const VkImageSubresourceRange &full_range) {
    VkImageSubresourceRange encoder_range = full_range;
    if (CanTransitionDepthSlices(device_state.extensions, create_info)) {
        encoder_range.layerCount = create_info.extent.depth;
    }
    return encoder_range;
}

void Image::SetInitialLayoutMap() {
    if (layout_map) {
        return;
    }

    std::shared_ptr<ImageLayoutMap> new_layout_map;
    std::shared_ptr<std::shared_mutex> new_layout_map_lock;

    auto get_layout_map = [&new_layout_map, &new_layout_map_lock](const Image &other_image) {
        new_layout_map = other_image.layout_map;
        new_layout_map_lock = other_image.layout_map_lock;
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

    // Set layout of each subresource as VkImageCreateInfo::initialLayout
    if (!new_layout_map) {
        new_layout_map = std::make_shared<ImageLayoutMap>(subresource_encoder.SubresourceCount());
        new_layout_map_lock = std::make_shared<std::shared_mutex>();

        for (auto range_gen = RangeGenerator(subresource_encoder); range_gen->non_empty(); ++range_gen) {
            new_layout_map->insert(new_layout_map->end(), std::make_pair(*range_gen, create_info.initialLayout));
        }
    }
    layout_map = std::move(new_layout_map);
    layout_map_lock = std::move(new_layout_map_lock);
}

void Image::SetImageLayout(const VkImageSubresourceRange &range, VkImageLayout layout) {
    using sparse_container::update_range_value;
    using sparse_container::value_precedence;
    RangeGenerator range_gen(subresource_encoder, NormalizeSubresourceRange(range));
    auto guard = LayoutMapWriteLock();
    for (; range_gen->non_empty(); ++range_gen) {
        update_range_value(*layout_map, *range_gen, layout, value_precedence::prefer_source);
    }
}

void Image::SetSwapchain(std::shared_ptr<vvl::Swapchain> &swapchain, uint32_t swapchain_index) {
    assert(IsSwapchainImage());
    bind_swapchain = swapchain;
    swapchain_image_index = swapchain_index;
    bind_swapchain->AddParent(this);
}

bool Image::CompareCreateInfo(const Image &other) const {
    bool valid_queue_family = true;
    if (create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (create_info.queueFamilyIndexCount != other.create_info.queueFamilyIndexCount) {
            valid_queue_family = false;
        } else {
            for (uint32_t i = 0; i < create_info.queueFamilyIndexCount; i++) {
                if (create_info.pQueueFamilyIndices[i] != other.create_info.pQueueFamilyIndices[i]) {
                    valid_queue_family = false;
                    break;
                }
            }
        }
    }

    // There are limitations what actually needs to be compared, so for simplicity (until found otherwise needed), we only need to
    // check the ExternalHandleType and not other pNext chains
    const bool valid_external = GetExternalHandleTypes(&create_info) == GetExternalHandleTypes(&other.create_info);

    return (create_info.flags == other.create_info.flags) && (create_info.imageType == other.create_info.imageType) &&
           (create_info.format == other.create_info.format) && (create_info.extent.width == other.create_info.extent.width) &&
           (create_info.extent.height == other.create_info.extent.height) &&
           (create_info.extent.depth == other.create_info.extent.depth) && (create_info.mipLevels == other.create_info.mipLevels) &&
           (create_info.arrayLayers == other.create_info.arrayLayers) && (create_info.samples == other.create_info.samples) &&
           (create_info.tiling == other.create_info.tiling) && (create_info.usage == other.create_info.usage) &&
           (create_info.initialLayout == other.create_info.initialLayout) && valid_queue_family && valid_external;
}

}  // namespace vvl

static VkSamplerYcbcrConversion GetSamplerConversion(const VkImageViewCreateInfo *ci) {
    auto *conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(ci->pNext);
    return conversion_info ? conversion_info->conversion : VK_NULL_HANDLE;
}

static VkImageUsageFlags GetInheritedUsage(const VkImageViewCreateInfo *ci, const vvl::Image &image_state) {
    auto usage_create_info = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(ci->pNext);
    return (usage_create_info) ? usage_create_info->usage : image_state.create_info.usage;
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

namespace vvl {

ImageView::ImageView(const DeviceState &device_state, const std::shared_ptr<vvl::Image> &image_state, VkImageView handle,
                     const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
                     const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props)
    : StateObject(handle, kVulkanObjectTypeImageView),
      safe_create_info(ci),
      create_info(*safe_create_info.ptr()),
      image_state(image_state),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_imageview_export(GetMetalExport(ci)),
#endif
      is_depth_sliced(IsDepthSliceView(image_state->create_info, create_info.viewType)),
      normalized_subresource_range(ImageView::NormalizeImageViewSubresourceRange(*image_state, create_info)),
      range_generator(image_state->subresource_encoder, GetRangeGeneratorRange(device_state.extensions)),
      samples(image_state->create_info.samples),
      samplerConversion(GetSamplerConversion(ci)),
      filter_cubic_props(cubic_props),
      min_lod(GetImageViewMinLod(ci)),
      format_features(ff),
      inherited_usage(GetInheritedUsage(ci, *image_state)) {
}

void ImageView::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &item : sub_states_) {
        item.second->NotifyInvalidate(invalid_nodes, unlink);
    }
    StateObject::NotifyInvalidate(invalid_nodes, unlink);
}

void ImageView::Destroy() {
    for (auto &item : sub_states_) {
        item.second->Destroy();
    }
    if (image_state) {
        image_state->RemoveParent(this);
        image_state = nullptr;
    }
    StateObject::Destroy();
}

uint32_t ImageView::GetAttachmentLayerCount() const {
    if (create_info.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS && !is_depth_sliced) {
        return image_state->create_info.arrayLayers;
    }
    return create_info.subresourceRange.layerCount;
}

VkImageSubresourceRange ImageView::NormalizeImageViewSubresourceRange(const Image &image_state,
                                                                      const VkImageViewCreateInfo &image_view_ci) {
    const VkImageCreateInfo &image_ci = image_state.create_info;

    VkImageSubresourceRange range = image_view_ci.subresourceRange;
    range.levelCount = GetEffectiveLevelCount(range, image_ci.mipLevels);
    range.aspectMask = NormalizeAspectMask(range.aspectMask, image_view_ci.format);

    if (image_view_ci.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        if (IsDepthSliceView(image_state.create_info, image_view_ci.viewType)) {
            const VkExtent3D extent = GetEffectiveExtent(image_ci, range.aspectMask, range.baseMipLevel);
            range.layerCount = extent.depth - image_view_ci.subresourceRange.baseArrayLayer;
        } else {
            range.layerCount = GetEffectiveLayerCount(range, image_ci.arrayLayers);
        }
    }
    return range;
}

VkImageSubresourceRange ImageView::GetRangeGeneratorRange(const DeviceExtensions &extensions) const {
    VkImageSubresourceRange subres_range = create_info.subresourceRange;

    // if we're mapping a 3D image to a 2d image view, convert the view's subresource range to be compatible with the
    // image's understanding of the world. From the VkImageSubresourceRange section of the Vulkan spec:
    //
    //     When the VkImageSubresourceRange structure is used to select a subset of the slices of a 3D image’s mip level in order to
    //     create a 2D or 2D array image view of a 3D image created with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT, baseArrayLayer and
    //     layerCount specify the first slice index and the number of slices to include in the created image view. Such an image
    //     view can be used as a framebuffer attachment that refers only to the specified range of slices of the selected mip level.
    //     If the maintenance9 feature is not enabled, any layout transitions performed on such an attachment view during a render
    //     pass instance still apply to the entire subresource referenced which includes all the slices of the selected mip level.
    //
    if (is_depth_sliced && !CanTransitionDepthSlices(extensions, image_state->create_info)) {
        subres_range.baseArrayLayer = 0;
        subres_range.layerCount = 1;
    }
    return image_state->NormalizeSubresourceRange(subres_range);
}

bool ImageView::OverlapSubresource(const ImageView &compare_view) const {
    if (VkHandle() == compare_view.VkHandle()) {
        return true;
    }
    if (image_state->VkHandle() != compare_view.image_state->VkHandle()) {
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

}  // namespace vvl
