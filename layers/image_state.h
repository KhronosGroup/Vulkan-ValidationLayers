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
#pragma once

#include "device_memory_state.h"
#include "image_layout_map.h"
#include "vk_format_utils.h"
#include "vk_layer_utils.h"

class ValidationStateTracker;
class SURFACE_STATE;
class SWAPCHAIN_NODE;

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    bool is_equal = (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
    return is_equal;
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info, const VkImageSubresourceRange &range);
VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info,
                                                  const VkImageViewCreateInfo &view_create_info);

// Transfer VkImageSubresourceRange into VkImageSubresourceLayers struct
static inline VkImageSubresourceLayers LayersFromRange(const VkImageSubresourceRange &subresource_range) {
    VkImageSubresourceLayers subresource_layers;
    subresource_layers.aspectMask = subresource_range.aspectMask;
    subresource_layers.baseArrayLayer = subresource_range.baseArrayLayer;
    subresource_layers.layerCount = subresource_range.layerCount;
    subresource_layers.mipLevel = subresource_range.baseMipLevel;
    return subresource_layers;
}

// Transfer VkImageSubresourceLayers into VkImageSubresourceRange struct
static inline VkImageSubresourceRange RangeFromLayers(const VkImageSubresourceLayers &subresource_layers) {
    VkImageSubresourceRange subresource_range;
    subresource_range.aspectMask = subresource_layers.aspectMask;
    subresource_range.baseArrayLayer = subresource_layers.baseArrayLayer;
    subresource_range.layerCount = subresource_layers.layerCount;
    subresource_range.baseMipLevel = subresource_layers.mipLevel;
    subresource_range.levelCount = 1;
    return subresource_range;
}

class GlobalImageLayoutRangeMap : public subresource_adapter::BothRangeMap<VkImageLayout, 16> {
  public:
    GlobalImageLayoutRangeMap(index_type index) : BothRangeMap<VkImageLayout, 16>(index) {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

  private:
    mutable ReadWriteLock lock_;
};

// State for VkImage objects.
// Parent -> child relationships in the object usage tree:
// 1. Normal images:
//    IMAGE_STATE [1] -> [1] DEVICE_MEMORY_STATE
//
// 2. Sparse images:
//    IMAGE_STATE [1] -> [N] DEVICE_MEMORY_STATE
//
// 3. VK_IMAGE_CREATE_ALIAS_BIT images:
//    IMAGE_STATE [N] -> [1] DEVICE_MEMORY_STATE
//    All other images using the same device memory are in the aliasing_images set.
//
// 4. Swapchain images
//    IMAGE_STATE [N] -> [1] SWAPCHAIN_NODE
//    All other images using the same swapchain and swapchain_image_index are in the aliasing_images set.
//    Note that the images for *every* image_index will show up as parents of the swapchain,
//    so swapchain_image_index values must be compared.
//
class IMAGE_STATE : public BINDABLE {
  public:
    const safe_VkImageCreateInfo safe_create_info;
    const VkImageCreateInfo &createInfo;
    bool shared_presentable;  // True for a front-buffered swapchain image
    bool layout_locked;       // A front-buffered image that has been presented can never have layout transitioned
    const uint64_t ahb_format;           // External Android format, if provided
    const VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers, and aspects
    const VkSwapchainKHR create_from_swapchain;
    const bool owned_by_swapchain;
    std::shared_ptr<SWAPCHAIN_NODE> bind_swapchain;
    uint32_t swapchain_image_index;
    const VkFormatFeatureFlags2KHR format_features;
    // Need to memory requirments for each plane if image is disjoint
    const bool disjoint;  // True if image was created with VK_IMAGE_CREATE_DISJOINT_BIT
    static constexpr int MAX_PLANES = 3;
    using MemoryReqs = std::array<VkMemoryRequirements, MAX_PLANES>;
    const MemoryReqs requirements;
    const VkMemoryRequirements *const memory_requirements_pointer = &requirements[0];
    std::array<bool, MAX_PLANES> memory_requirements_checked;
    using SparseReqs = std::vector<VkSparseImageMemoryRequirements>;
    const SparseReqs sparse_requirements;
    const bool sparse_metadata_required;  // Track if sparse metadata aspect is required for this image
    bool get_sparse_reqs_called;          // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_bound;           // Track if sparse metadata aspect is bound to this image
    VkImageFormatProperties image_format_properties = {};
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_image_export;
    const bool metal_io_surface_export;
#endif // VK_USE_PLATFORM_METAL

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    std::unique_ptr<const subresource_adapter::ImageRangeEncoder> fragment_encoder;  // Fragment resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const

    std::shared_ptr<GlobalImageLayoutRangeMap> layout_range_map;

    IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
                VkFormatFeatureFlags2KHR features);
    IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
                uint32_t swapchain_index, VkFormatFeatureFlags2KHR features);
    IMAGE_STATE(IMAGE_STATE const &rh_obj) = delete;

    VkImage image() const { return handle_.Cast<VkImage>(); }

    bool HasAHBFormat() const { return ahb_format != 0; }
    bool IsCompatibleAliasing(IMAGE_STATE *other_image_state) const;

    // returns true if this image could be using the same memory as another image
    bool CanAlias() const { return ((createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT) != 0) || bind_swapchain; }

    bool IsCreateInfoEqual(const VkImageCreateInfo &other_createInfo) const;
    bool IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_createInfo) const;

    bool IsSwapchainImage() const { return create_from_swapchain != VK_NULL_HANDLE; }

    inline bool IsImageTypeEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.imageType == other_createInfo.imageType;
    }
    inline bool IsFormatEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.format == other_createInfo.format;
    }
    inline bool IsMipLevelsEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.mipLevels == other_createInfo.mipLevels;
    }
    inline bool IsUsageEqual(const VkImageCreateInfo &other_createInfo) const { return createInfo.usage == other_createInfo.usage; }
    inline bool IsSamplesEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.samples == other_createInfo.samples;
    }
    inline bool IsTilingEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.tiling == other_createInfo.tiling;
    }
    inline bool IsArrayLayersEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.arrayLayers == other_createInfo.arrayLayers;
    }
    inline bool IsInitialLayoutEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.initialLayout == other_createInfo.initialLayout;
    }
    inline bool IsSharingModeEqual(const VkImageCreateInfo &other_createInfo) const {
        return createInfo.sharingMode == other_createInfo.sharingMode;
    }
    inline bool IsExtentEqual(const VkImageCreateInfo &other_createInfo) const {
        return (createInfo.extent.width == other_createInfo.extent.width) &&
               (createInfo.extent.height == other_createInfo.extent.height) &&
               (createInfo.extent.depth == other_createInfo.extent.depth);
    }
    inline bool IsQueueFamilyIndicesEqual(const VkImageCreateInfo &other_createInfo) const {
        return (createInfo.queueFamilyIndexCount == other_createInfo.queueFamilyIndexCount) &&
               (createInfo.queueFamilyIndexCount == 0 ||
                memcmp(createInfo.pQueueFamilyIndices, other_createInfo.pQueueFamilyIndices,
                       createInfo.queueFamilyIndexCount * sizeof(createInfo.pQueueFamilyIndices[0])) == 0);
    }

    ~IMAGE_STATE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void SetSwapchain(std::shared_ptr<SWAPCHAIN_NODE> &swapchain, uint32_t swapchain_index);

    VkDeviceSize GetFakeBaseAddress() const override;

    void Destroy() override;

    VkExtent3D GetSubresourceExtent(VkImageAspectFlags aspect_mask, uint32_t mip_level) const;
    VkExtent3D GetSubresourceExtent(const VkImageSubresourceLayers &subresource) const;

    VkImageSubresourceRange NormalizeSubresourceRange(const VkImageSubresourceRange &range) const {
        return ::NormalizeSubresourceRange(createInfo, range);
    }

    void SetInitialLayoutMap();

  protected:
    void NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) override;
};

using IMAGE_STATE_NO_BINDING = MEMORY_TRACKED_RESOURCE_STATE<IMAGE_STATE, BindableNoMemoryTracker>;
using IMAGE_STATE_LINEAR = MEMORY_TRACKED_RESOURCE_STATE<IMAGE_STATE, BindableLinearMemoryTracker>;
template <bool IS_RESIDENT>
using IMAGE_STATE_SPARSE = MEMORY_TRACKED_RESOURCE_STATE<IMAGE_STATE, BindableSparseMemoryTracker<IS_RESIDENT>>;
template <unsigned PLANE_COUNT>
using IMAGE_STATE_MULTIPLANAR = MEMORY_TRACKED_RESOURCE_STATE<IMAGE_STATE, BindableMultiplanarMemoryTracker<PLANE_COUNT>>;

// State for VkImageView objects.
// Parent -> child relationships in the object usage tree:
//    IMAGE_VIEW_STATE [N] -> [1] IMAGE_STATE
class IMAGE_VIEW_STATE : public BASE_NODE {
  public:
    const safe_VkImageViewCreateInfo safe_create_info;
    const VkImageViewCreateInfo &create_info;
    const VkImageSubresourceRange normalized_subresource_range;
    const image_layout_map::RangeGenerator range_generator;
    const VkSampleCountFlagBits samples;
    const unsigned descriptor_format_bits;
    const VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    const VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props;
    const float min_lod;
    const VkFormatFeatureFlags2KHR format_features;
    const VkImageUsageFlags inherited_usage;  // from spec #resources-image-inherited-usage
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_imageview_export;
#endif // VK_USE_PLATFORM_METAL_EXT
    std::shared_ptr<IMAGE_STATE> image_state;

    IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                     VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    IMAGE_VIEW_STATE(const IMAGE_VIEW_STATE &rh_obj) = delete;
    VkImageView image_view() const { return handle_.Cast<VkImageView>(); }

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        image_state->AddParent(this);
    }

    virtual ~IMAGE_VIEW_STATE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    bool OverlapSubresource(const IMAGE_VIEW_STATE &compare_view) const;

    void Destroy() override;

    bool IsDepthSliced() const;

    VkOffset3D GetOffset() const;
    VkExtent3D GetExtent() const;
    uint32_t GetAttachmentLayerCount() const;

    bool Invalid() const override { return Destroyed() || !image_state || image_state->Invalid(); }
};

struct SWAPCHAIN_IMAGE {
    IMAGE_STATE *image_state = nullptr;
    VkDeviceSize fake_base_address = 0;
    bool acquired = false;
};

// State for VkSwapchainKHR objects.
// Parent -> child relationships in the object usage tree:
//    SWAPCHAIN_NODE [N] -> [1] SURFACE_STATE
//    However, only 1 swapchain for each surface can be !retired.
class SWAPCHAIN_NODE : public BASE_NODE {
  public:
    const safe_VkSwapchainCreateInfoKHR createInfo;
    std::vector<SWAPCHAIN_IMAGE> images;
    bool retired = false;
    const bool shared_presentable;
    uint32_t get_swapchain_image_count = 0;
    uint64_t max_present_id = 0;
    const safe_VkImageCreateInfo image_create_info;

    std::shared_ptr<SURFACE_STATE> surface;
    ValidationStateTracker *dev_data;
    uint32_t acquired_images = 0;

    SWAPCHAIN_NODE(ValidationStateTracker *dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR swapchain);

    ~SWAPCHAIN_NODE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSwapchainKHR swapchain() const { return handle_.Cast<VkSwapchainKHR>(); }

    void PresentImage(uint32_t image_index, uint64_t present_id);

    void AcquireImage(uint32_t image_index);

    void Destroy() override;

  protected:
    void NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) override;
};

struct GpuQueue {
    VkPhysicalDevice gpu;
    uint32_t queue_family_index;
};

inline bool operator==(GpuQueue const &lhs, GpuQueue const &rhs) {
    return (lhs.gpu == rhs.gpu && lhs.queue_family_index == rhs.queue_family_index);
}

namespace std {
template <>
struct hash<GpuQueue> {
    size_t operator()(GpuQueue gq) const throw() {
        return hash<uint64_t>()((uint64_t)(gq.gpu)) ^ hash<uint32_t>()(gq.queue_family_index);
    }
};
}  // namespace std

// State for VkSurfaceKHR objects.
// Parent -> child relationships in the object usage tree:
//    SURFACE_STATE -> nothing
class SURFACE_STATE : public BASE_NODE {
  public:
    SURFACE_STATE(VkSurfaceKHR s) : BASE_NODE(s, kVulkanObjectTypeSurfaceKHR) {}

    ~SURFACE_STATE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSurfaceKHR surface() const { return handle_.Cast<VkSurfaceKHR>(); }

    void Destroy() override;

    VkImageCreateInfo GetImageCreateInfo() const;

    void RemoveParent(BASE_NODE *parent_node) override;

    void SetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi, bool supported);
    bool GetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi) const;

    void SetPresentModes(VkPhysicalDevice phys_dev, std::vector<VkPresentModeKHR> &&modes);
    std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice phys_dev) const;

    void SetFormats(VkPhysicalDevice phys_dev, std::vector<VkSurfaceFormatKHR> &&fmts);
    std::vector<VkSurfaceFormatKHR> GetFormats(VkPhysicalDevice phys_dev) const;

    void SetCapabilities(VkPhysicalDevice phys_dev, const VkSurfaceCapabilitiesKHR &caps);
    VkSurfaceCapabilitiesKHR GetCapabilities(VkPhysicalDevice phys_dev) const;

    SWAPCHAIN_NODE *swapchain{nullptr};

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }
    mutable std::mutex lock_;
    mutable layer_data::unordered_map<GpuQueue, bool> gpu_queue_support_;
    mutable layer_data::unordered_map<VkPhysicalDevice, std::vector<VkPresentModeKHR>> present_modes_;
    mutable layer_data::unordered_map<VkPhysicalDevice, std::vector<VkSurfaceFormatKHR>> formats_;
    mutable layer_data::unordered_map<VkPhysicalDevice, VkSurfaceCapabilitiesKHR> capabilities_;
};
