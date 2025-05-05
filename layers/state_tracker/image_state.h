/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include <variant>

#include "state_tracker/device_memory_state.h"
#include "state_tracker/image_layout_map.h"
#include "utils/vk_layer_utils.h"

namespace vvl {
class DeviceState;
class ImageSubState;
class ImageViewSubState;
class Swapchain;
class VideoProfileDesc;
}  // namespace vvl

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    return (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
}

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

namespace vvl {

// State for VkImage objects.
// Parent -> child relationships in the object usage tree:
// 1. Normal images:
//    vvl::Image [1] -> [1] vvl::DeviceMemory
//
// 2. Sparse images:
//    vvl::Image [1] -> [N] vvl::DeviceMemory
//
// 3. VK_IMAGE_CREATE_ALIAS_BIT images:
//    vvl::Image [N] -> [1] vvl::DeviceMemory
//    All other images using the same device memory are in the aliasing_images set.
//
// 4. Swapchain images
//    vvl::Image [N] -> [1] vvl::Swapchain
//    All other images using the same swapchain and swapchain_image_index are in the aliasing_images set.
//    Note that the images for *every* image_index will show up as parents of the swapchain,
//    so swapchain_image_index values must be compared.
//
class Image : public Bindable, public SubStateManager<ImageSubState> {
  public:
    const vku::safe_VkImageCreateInfo safe_create_info;
    const VkImageCreateInfo &create_info;
    bool shared_presentable;                   // True for a front-buffered swapchain image
    bool layout_locked;                        // A front-buffered image that has been presented can never have layout transitioned
    const uint64_t ahb_format;                 // External Android format, if provided
    const VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers, and aspects
    const VkSwapchainKHR create_from_swapchain;
    const bool owned_by_swapchain;
    std::shared_ptr<vvl::Swapchain> bind_swapchain;
    uint32_t swapchain_image_index;
    const VkFormatFeatureFlags2KHR format_features;
    // Need to memory requirements for each plane if image is disjoint
    const bool disjoint;  // True if image was created with VK_IMAGE_CREATE_DISJOINT_BIT
    static constexpr int kMaxPlanes = 3;
    using MemoryReqs = std::array<VkMemoryRequirements, kMaxPlanes>;
    const MemoryReqs requirements;

    const bool sparse_residency;
    const std::vector<VkSparseImageMemoryRequirements> sparse_requirements;

    VkImageFormatProperties image_format_properties = {};
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_image_export;
    const bool metal_io_surface_export;
#endif  // VK_USE_PLATFORM_METAL

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const

    // This map is used to validate/update image layouts during submit time processing.
    // Record time validation can't use this, because global image layout is undefined
    // at record time (depends on the previous submissions, which are unknown at record time).
    std::shared_ptr<ImageLayoutRangeMap> layout_range_map;

    // If there is no aliasing this mutex protects this->layout_range_map.
    // With aliasing one of the images shares a mutex with other aliases,
    // so for some aliased images this mutex can be unused.
    mutable std::shared_mutex layout_range_map_lock;

    vvl::unordered_set<std::shared_ptr<const vvl::VideoProfileDesc>> supported_video_profiles;

    Image(const DeviceState &dev_data, VkImage handle, const VkImageCreateInfo *pCreateInfo, VkFormatFeatureFlags2KHR features);
    Image(const DeviceState &dev_data, VkImage handle, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
          uint32_t swapchain_index, VkFormatFeatureFlags2KHR features);
    Image(Image const &rh_obj) = delete;
    std::shared_ptr<const Image> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<Image> shared_from_this() { return SharedFromThisImpl(this); }

    VkImage VkHandle() const { return handle_.Cast<VkImage>(); }

    bool HasAHBFormat() const { return ahb_format != 0; }
    bool IsCompatibleAliasing(const Image *other_image_state) const;

    // returns true if this image could be using the same memory as another image
    bool HasAliasFlag() const { return 0 != (create_info.flags & VK_IMAGE_CREATE_ALIAS_BIT); }
    bool CanAlias() const { return HasAliasFlag() || bind_swapchain; }

    bool IsCreateInfoEqual(const VkImageCreateInfo &other_create_info) const;
    bool IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_create_info) const;

    bool IsSwapchainImage() const { return create_from_swapchain != VK_NULL_HANDLE; }

    // TODO - need to understand if VkBindImageMemorySwapchainInfoKHR counts as "bound"
    bool HasBeenBound() const { return (MemoryState() != nullptr) || (bind_swapchain); }

    inline bool IsImageTypeEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.imageType == other_create_info.imageType;
    }
    inline bool IsFormatEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.format == other_create_info.format;
    }
    inline bool IsMipLevelsEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.mipLevels == other_create_info.mipLevels;
    }
    inline bool IsUsageEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.usage == other_create_info.usage;
    }
    inline bool IsSamplesEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.samples == other_create_info.samples;
    }
    inline bool IsTilingEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.tiling == other_create_info.tiling;
    }
    inline bool IsArrayLayersEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.arrayLayers == other_create_info.arrayLayers;
    }
    inline bool IsInitialLayoutEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.initialLayout == other_create_info.initialLayout;
    }
    inline bool IsSharingModeEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.sharingMode == other_create_info.sharingMode;
    }
    inline bool IsExtentEqual(const VkImageCreateInfo &other_create_info) const {
        return (create_info.extent.width == other_create_info.extent.width) &&
               (create_info.extent.height == other_create_info.extent.height) &&
               (create_info.extent.depth == other_create_info.extent.depth);
    }
    inline bool IsQueueFamilyIndicesEqual(const VkImageCreateInfo &other_create_info) const {
        return (create_info.queueFamilyIndexCount == other_create_info.queueFamilyIndexCount) &&
               (create_info.queueFamilyIndexCount == 0 ||
                memcmp(create_info.pQueueFamilyIndices, other_create_info.pQueueFamilyIndices,
                       create_info.queueFamilyIndexCount * sizeof(create_info.pQueueFamilyIndices[0])) == 0);
    }

    ~Image() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void SetSwapchain(std::shared_ptr<vvl::Swapchain> &swapchain, uint32_t swapchain_index);

    void Destroy() override;

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresourceLayers &sub) const {
        return GetEffectiveExtent(create_info, sub.aspectMask, sub.mipLevel);
    }

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresource &sub) const {
        return GetEffectiveExtent(create_info, sub.aspectMask, sub.mipLevel);
    }

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresourceRange &range) const {
        return GetEffectiveExtent(create_info, range.aspectMask, range.baseMipLevel);
    }

    VkImageSubresourceRange NormalizeSubresourceRange(const VkImageSubresourceRange &range) const;
    uint32_t NormalizeLayerCount(const VkImageSubresourceLayers &resource) const;

    void SetInitialLayoutMap();
    void SetImageLayout(const VkImageSubresourceRange &range, VkImageLayout layout);

    // This function is only used for comparing Imported External Dedicated Memory
    bool CompareCreateInfo(const Image &other) const;

    template <typename UnaryPredicate>
    bool AnyAliasBindingOf(const StateObject::NodeMap &bindings, const UnaryPredicate &pred) const {
        for (auto &entry : bindings) {
            if (entry.first.type == kVulkanObjectTypeImage) {
                auto state_object = entry.second.lock();
                if (state_object) {
                    auto other_image = static_cast<Image *>(state_object.get());
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

    template <typename RegionType>
    VkDeviceSize GetBufferSizeFromCopyImage(const RegionType &region) const;

  protected:
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

  private:
    VkImageSubresourceRange MakeImageFullRange();

    std::variant<std::monostate, BindableNoMemoryTracker, BindableLinearMemoryTracker, BindableSparseMemoryTracker,
                 BindableMultiplanarMemoryTracker>
        tracker_;
};

class ImageSubState {
  public:
    explicit ImageSubState(Image &img) : base(img) {}
    ImageSubState(const ImageSubState &) = delete;
    ImageSubState &operator=(const ImageSubState &) = delete;
    virtual ~ImageSubState() {}
    virtual void Destroy() {}
    virtual void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {}

    Image &base;
};

// State for VkImageView objects.
// Parent -> child relationships in the object usage tree:
//    ImageView [N] -> [1] vv::Image
class ImageView : public StateObject, public SubStateManager<ImageViewSubState> {
  public:
    const vku::safe_VkImageViewCreateInfo safe_create_info;
    const VkImageViewCreateInfo &create_info;

    std::shared_ptr<vvl::Image> image_state;

#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_imageview_export;
#endif  // VK_USE_PLATFORM_METAL_EXT

    const bool is_depth_sliced;
    const VkImageSubresourceRange normalized_subresource_range;
    const image_layout_map::RangeGenerator range_generator;
    const VkSampleCountFlagBits samples;
    const VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    const VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props;
    const float min_lod;
    const VkFormatFeatureFlags2KHR format_features;
    const VkImageUsageFlags inherited_usage;  // from spec #resources-image-inherited-usage

    ImageView(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle, const VkImageViewCreateInfo *ci,
              VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    ImageView(const ImageView &rh_obj) = delete;
    VkImageView VkHandle() const { return handle_.Cast<VkImageView>(); }

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        image_state->AddParent(this);
    }

    virtual ~ImageView() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    bool OverlapSubresource(const ImageView &compare_view) const;

    void Destroy() override;
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    uint32_t GetAttachmentLayerCount() const;

    bool Invalid() const override { return Destroyed() || !image_state || image_state->Invalid(); }

  private:
    VkImageSubresourceRange NormalizeSubresourceRange() const;
    bool IsDepthSliced();
};

class ImageViewSubState {
  public:
    explicit ImageViewSubState(ImageView &view) : base(view) {}
    ImageViewSubState(const ImageViewSubState &) = delete;
    ImageViewSubState &operator=(const ImageViewSubState &) = delete;
    virtual ~ImageViewSubState() {}
    virtual void Destroy() {}
    virtual void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {}

    ImageView &base;
};
}  // namespace vvl
