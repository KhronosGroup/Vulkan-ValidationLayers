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
#pragma once

#include <utility>
#include <variant>

#include <vulkan/utility/vk_format_utils.h>

#include "state_tracker/device_memory_state.h"
#include "state_tracker/image_layout_map.h"
#include "utils/vk_layer_utils.h"

class ValidationStateTracker;
class VideoProfileDesc;
class SURFACE_STATE;
class SWAPCHAIN_NODE;

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    return (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
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
    using RangeGenerator = image_layout_map::RangeGenerator;
    using RangeType = key_type;

    GlobalImageLayoutRangeMap(index_type index) : BothRangeMap<VkImageLayout, 16>(index) {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    bool AnyInRange(RangeGenerator &gen, std::function<bool(const key_type &range, const mapped_type &state)> &&func) const;

  private:
    mutable std::shared_mutex lock_;
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
    bool shared_presentable;                   // True for a front-buffered swapchain image
    bool layout_locked;                        // A front-buffered image that has been presented can never have layout transitioned
    const uint64_t ahb_format;                 // External Android format, if provided
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
    std::array<bool, MAX_PLANES> memory_requirements_checked = {};

    const bool sparse_residency;
    using SparseReqs = std::vector<VkSparseImageMemoryRequirements>;
    const SparseReqs sparse_requirements;
    const bool sparse_metadata_required;  // Track if sparse metadata aspect is required for this image
    bool get_sparse_reqs_called;          // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_bound;           // Track if sparse metadata aspect is bound to this image

    VkImageFormatProperties image_format_properties = {};
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_image_export;
    const bool metal_io_surface_export;
#endif  // VK_USE_PLATFORM_METAL

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    std::unique_ptr<const subresource_adapter::ImageRangeEncoder> fragment_encoder;  // Fragment resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const

    std::shared_ptr<GlobalImageLayoutRangeMap> layout_range_map;

    vvl::unordered_set<std::shared_ptr<const VideoProfileDesc>> supported_video_profiles;

    IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
                VkFormatFeatureFlags2KHR features);
    IMAGE_STATE(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
                uint32_t swapchain_index, VkFormatFeatureFlags2KHR features);
    IMAGE_STATE(IMAGE_STATE const &rh_obj) = delete;
    std::shared_ptr<const IMAGE_STATE> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<IMAGE_STATE> shared_from_this() { return SharedFromThisImpl(this); }

    VkImage image() const { return handle_.Cast<VkImage>(); }

    bool HasAHBFormat() const { return ahb_format != 0; }
    bool IsCompatibleAliasing(const IMAGE_STATE *other_image_state) const;

    // returns true if this image could be using the same memory as another image
    bool HasAliasFlag() const { return 0 != (createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT); }
    bool CanAlias() const { return HasAliasFlag() || bind_swapchain; }

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

    void Destroy() override;

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresourceLayers &sub) const {
        return ::GetEffectiveExtent(createInfo, sub.aspectMask, sub.mipLevel);
    }

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresource &sub) const {
        return ::GetEffectiveExtent(createInfo, sub.aspectMask, sub.mipLevel);
    }

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresourceRange &range) const {
        return ::GetEffectiveExtent(createInfo, range);
    }

    VkImageSubresourceRange NormalizeSubresourceRange(const VkImageSubresourceRange &range) const {
        return ::NormalizeSubresourceRange(createInfo, range);
    }

    void SetInitialLayoutMap();
    void SetImageLayout(const VkImageSubresourceRange &range, VkImageLayout layout);

  protected:
    void NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) override;

    template <typename UnaryPredicate>
    bool AnyAliasBindingOf(const BASE_NODE::NodeMap &bindings, const UnaryPredicate &pred) const {
        for (auto &entry : bindings) {
            if (entry.first.type == kVulkanObjectTypeImage) {
                auto base_node = entry.second.lock();
                if (base_node) {
                    auto other_image = static_cast<IMAGE_STATE *>(base_node.get());
                    if ((other_image != this) && other_image->IsCompatibleAliasing(this)) {
                        if (pred(*other_image)) return true;
                    }
                }
            }
        }
        return false;
    }

    template <typename UnaryPredicate>
    bool AnyImageAliasOf(const UnaryPredicate &pred) const {
        // Look for another aliasing image and
        // ObjectBindings() is thread safe since returns by value, and once
        // the weak_ptr is successfully locked, the other image state won't
        // be freed out from under us.
        for (auto const &memory_state : GetBoundMemoryStates()) {
            if (AnyAliasBindingOf(memory_state->ObjectBindings(), pred)) return true;
        }
        return false;
    }

  private:
    std::variant<std::monostate,
                 BindableNoMemoryTracker,
                 BindableLinearMemoryTracker,
                 BindableSparseMemoryTracker,
                 BindableMultiplanarMemoryTracker> tracker_;
};

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
#endif  // VK_USE_PLATFORM_METAL_EXT
    std::shared_ptr<IMAGE_STATE> image_state;
    const bool is_depth_sliced;

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

    bool IsDepthSliced() const { return is_depth_sliced; }

    uint32_t GetAttachmentLayerCount() const;

    bool Invalid() const override { return Destroyed() || !image_state || image_state->Invalid(); }
};

struct SWAPCHAIN_IMAGE {
    IMAGE_STATE *image_state = nullptr;
    bool acquired = false;
};

// State for VkSwapchainKHR objects.
// Parent -> child relationships in the object usage tree:
//    SWAPCHAIN_NODE [N] -> [1] SURFACE_STATE
//    However, only 1 swapchain for each surface can be !retired.
class SWAPCHAIN_NODE : public BASE_NODE {
  public:
    const safe_VkSwapchainCreateInfoKHR createInfo;
    std::vector<VkPresentModeKHR> present_modes;
    std::vector<SWAPCHAIN_IMAGE> images;
    bool retired = false;
    bool exclusive_full_screen_access;
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

    SWAPCHAIN_IMAGE GetSwapChainImage(uint32_t index) const;

    std::shared_ptr<const IMAGE_STATE> GetSwapChainImageShared(uint32_t index) const;

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

class ValidationObject;

// State for VkSurfaceKHR objects.
struct PresentModeState {
    VkSurfaceCapabilitiesKHR surface_capabilities_;
    VkSurfacePresentScalingCapabilitiesEXT scaling_capabilities_;
    std::vector<VkPresentModeKHR> compatible_present_modes_;
};

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
    VkPhysicalDeviceSurfaceInfo2KHR GetSurfaceInfo2(const void *surface_info2_pnext = nullptr) const {
        VkPhysicalDeviceSurfaceInfo2KHR surface_info2 = vku::InitStructHelper();
        surface_info2.pNext = surface_info2_pnext;
        surface_info2.surface = surface();
        return surface_info2;
    }

    void Destroy() override;

    void RemoveParent(BASE_NODE *parent_node) override;

    void SetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi, bool supported);
    bool GetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi) const;

    void SetPresentModes(VkPhysicalDevice phys_dev, vvl::span<const VkPresentModeKHR> modes);
    std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice phys_dev, const ValidationObject *validation_obj) const;

    void SetFormats(VkPhysicalDevice phys_dev, std::vector<safe_VkSurfaceFormat2KHR> &&fmts);
    vvl::span<const safe_VkSurfaceFormat2KHR> GetFormats(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                         const void *surface_info2_pnext,
                                                         const ValidationObject *validation_obj) const;

    void SetCapabilities(VkPhysicalDevice phys_dev, const safe_VkSurfaceCapabilities2KHR &caps);
    safe_VkSurfaceCapabilities2KHR GetCapabilities(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                   const void *surface_info2_pnext, const ValidationObject *validation_obj) const;

    void SetCompatibleModes(VkPhysicalDevice phys_dev, const VkPresentModeKHR present_mode,
                            vvl::span<const VkPresentModeKHR> compatible_modes);
    std::vector<VkPresentModeKHR> GetCompatibleModes(VkPhysicalDevice phys_dev, const VkPresentModeKHR present_mode) const;
    void SetPresentModeCapabilities(VkPhysicalDevice phys_dev, const VkPresentModeKHR present_mode,
                                    const VkSurfaceCapabilitiesKHR &caps,
                                    const VkSurfacePresentScalingCapabilitiesEXT &scaling_caps);
    VkSurfaceCapabilitiesKHR GetPresentModeSurfaceCapabilities(VkPhysicalDevice phys_dev,
                                                               const VkPresentModeKHR present_mode) const;
    VkSurfacePresentScalingCapabilitiesEXT GetPresentModeScalingCapabilities(VkPhysicalDevice phys_dev,
                                                                             const VkPresentModeKHR present_mode) const;
    SWAPCHAIN_NODE *swapchain{nullptr};

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }
    mutable std::mutex lock_;
    mutable vvl::unordered_map<GpuQueue, bool> gpu_queue_support_;
    mutable vvl::unordered_map<VkPhysicalDevice, std::vector<safe_VkSurfaceFormat2KHR>> formats_;
    mutable vvl::unordered_map<VkPhysicalDevice, safe_VkSurfaceCapabilities2KHR> capabilities_;
    mutable vvl::unordered_map<VkPhysicalDevice,
                                      vvl::unordered_map<VkPresentModeKHR, std::optional<std::shared_ptr<PresentModeState>>>>
        present_modes_data_;
};
