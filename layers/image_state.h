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
#pragma once

#include "device_memory_state.h"
#include "image_layout_map.h"
#include "vk_format_utils.h"

class ValidationStateTracker;
class SURFACE_STATE;
class SWAPCHAIN_NODE;

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    bool is_equal = (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
    return is_equal;
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info, const VkImageSubresourceRange &range);
uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels);
uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers);

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
    bool valid;               // If this is a swapchain image backing memory track valid here as it doesn't have DEVICE_MEMORY_STATE
    bool acquired;            // If this is a swapchain image, has it been acquired by the app.
    bool shared_presentable;  // True for a front-buffered swapchain image
    bool layout_locked;       // A front-buffered image that has been presented can never have layout transitioned
    bool get_sparse_reqs_called;         // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_required;       // Track if sparse metadata aspect is required for this image
    bool sparse_metadata_bound;          // Track if sparse metadata aspect is bound to this image
    const uint64_t ahb_format;           // External Android format, if provided
    const VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers (slices), and aspects
    const VkSwapchainKHR create_from_swapchain;
    std::shared_ptr<SWAPCHAIN_NODE> bind_swapchain;
    uint32_t swapchain_image_index;
    image_layout_map::Encoder range_encoder;
    const VkFormatFeatureFlags format_features;
    // Need to memory requirments for each plane if image is disjoint
    bool disjoint;  // True if image was created with VK_IMAGE_CREATE_DISJOINT_BIT
    static const int MAX_PLANES = 3;
    std::array<VkMemoryRequirements, MAX_PLANES> requirements;
    std::array<bool, MAX_PLANES> memory_requirements_checked;

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    std::unique_ptr<const subresource_adapter::ImageRangeEncoder> fragment_encoder;  // Fragment resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const

    std::vector<VkSparseImageMemoryRequirements> sparse_requirements;
    layer_data::unordered_set<IMAGE_STATE *> aliasing_images;

    IMAGE_STATE(VkDevice dev, VkImage img, const VkImageCreateInfo *pCreateInfo, VkFormatFeatureFlags features);
    IMAGE_STATE(VkDevice dev, VkImage img, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain, uint32_t swapchain_index,
                VkFormatFeatureFlags features);
    IMAGE_STATE(IMAGE_STATE const &rh_obj) = delete;

    VkImage image() const { return handle_.Cast<VkImage>(); }

    bool HasAHBFormat() const { return ahb_format != 0; }
    bool IsCompatibleAliasing(IMAGE_STATE *other_image_state) const;

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

    void SetMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, VkDeviceSize memory_offset) override;

    void SetSwapchain(std::shared_ptr<SWAPCHAIN_NODE> &swapchain, uint32_t swapchain_index);

    VkDeviceSize GetFakeBaseAddress() const override;

    void Destroy() override;

    VkExtent3D GetSubresourceExtent(const VkImageSubresourceLayers &subresource) const;

    VkImageSubresourceRange NormalizeSubresourceRange(const VkImageSubresourceRange &range) const {
        return ::NormalizeSubresourceRange(createInfo, range);
    }

  protected:
    void AddAliasingImage(IMAGE_STATE *bound_image);
    void Unlink();
    void NotifyInvalidate(const LogObjectList &invalid_handles, bool unlink) override;
};

// State for VkImageView objects.
// Parent -> child relationships in the object usage tree:
//    IMAGE_VIEW_STATE [N] -> [1] IMAGE_STATE
class IMAGE_VIEW_STATE : public BASE_NODE {
  public:
    const VkImageViewCreateInfo create_info;
    const VkImageSubresourceRange normalized_subresource_range;
    const image_layout_map::RangeGenerator range_generator;
    const VkSampleCountFlagBits samples;
    const unsigned descriptor_format_bits;
    const VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    const VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props;
    const VkFormatFeatureFlags format_features;
    const VkImageUsageFlags inherited_usage;  // from spec #resources-image-inherited-usage
    std::shared_ptr<IMAGE_STATE> image_state;

    IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                     VkFormatFeatureFlags ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    IMAGE_VIEW_STATE(const IMAGE_VIEW_STATE &rh_obj) = delete;

    VkImageView image_view() const { return handle_.Cast<VkImageView>(); }

    virtual ~IMAGE_VIEW_STATE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    bool OverlapSubresource(const IMAGE_VIEW_STATE &compare_view) const;

    void Destroy() override;
};

struct SWAPCHAIN_IMAGE {
    IMAGE_STATE *image_state = nullptr;
    VkDeviceSize fake_base_address = 0;
};

// State for VkSwapchainKHR objects.
// Parent -> child relationships in the object usage tree:
//    SWAPCHAIN_NODE [N] -> [1] SURFACE_STATE
//    However, only 1 swapchain for each surface can be !retired.
class SWAPCHAIN_NODE : public BASE_NODE {
  public:
    safe_VkSwapchainCreateInfoKHR createInfo;
    std::vector<SWAPCHAIN_IMAGE> images;
    bool retired;
    const bool shared_presentable;
    uint32_t get_swapchain_image_count;
    uint64_t max_present_id;
    const safe_VkImageCreateInfo image_create_info;
    std::shared_ptr<SURFACE_STATE> surface;
    ValidationStateTracker *dev_data;

    SWAPCHAIN_NODE(ValidationStateTracker *dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR swapchain);

    ~SWAPCHAIN_NODE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSwapchainKHR swapchain() const { return handle_.Cast<VkSwapchainKHR>(); }

    void PresentImage(uint32_t image_index);

    void AcquireImage(uint32_t image_index);

    void Destroy() override;

    const BindingsType &ObjectBindings() const { return parent_nodes_; }

  protected:
    void NotifyInvalidate(const LogObjectList &invalid_handles, bool unlink) override;
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
    SWAPCHAIN_NODE *swapchain;
    layer_data::unordered_map<GpuQueue, bool> gpu_queue_support;

    SURFACE_STATE(VkSurfaceKHR s) : BASE_NODE(s, kVulkanObjectTypeSurfaceKHR), swapchain(nullptr), gpu_queue_support() {}

    ~SURFACE_STATE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSurfaceKHR surface() const { return handle_.Cast<VkSurfaceKHR>(); }

    void Destroy() override;

    VkImageCreateInfo GetImageCreateInfo() const;

    void RemoveParent(BASE_NODE *parent_node) override;
};
