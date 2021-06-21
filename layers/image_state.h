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

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    bool is_equal = (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
    return is_equal;
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info, const VkImageSubresourceRange &range);
uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels);
uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers);

class IMAGE_STATE : public BINDABLE {
  public:
    safe_VkImageCreateInfo safe_create_info;
    VkImageCreateInfo &createInfo;
    bool valid;               // If this is a swapchain image backing memory track valid here as it doesn't have DEVICE_MEMORY_STATE
    bool acquired;            // If this is a swapchain image, has it been acquired by the app.
    bool shared_presentable;  // True for a front-buffered swapchain image
    bool layout_locked;       // A front-buffered image that has been presented can never have layout transitioned
    bool get_sparse_reqs_called;         // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_required;       // Track if sparse metadata aspect is required for this image
    bool sparse_metadata_bound;          // Track if sparse metadata aspect is bound to this image
    bool has_ahb_format;                 // True if image was created with an external Android format
    bool is_swapchain_image;             // True if image is a swapchain image
    uint64_t ahb_format;                 // External Android format, if provided
    VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers (slices), and aspects
    VkSwapchainKHR create_from_swapchain;
    VkSwapchainKHR bind_swapchain;
    uint32_t bind_swapchain_imageIndex;
    image_layout_map::Encoder range_encoder;
    VkFormatFeatureFlags format_features = 0;
    // Need to memory requirments for each plane if image is disjoint
    bool disjoint;  // True if image was created with VK_IMAGE_CREATE_DISJOINT_BIT
    static const int MAX_PLANES = 3;
    std::array<VkMemoryRequirements, MAX_PLANES> requirements;
    std::array<bool, MAX_PLANES> memory_requirements_checked;

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    std::unique_ptr<const subresource_adapter::ImageRangeEncoder> fragment_encoder;  // Fragment resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const
    VkDeviceSize swapchain_fake_address;  // Needed for swapchain syncval, since there is no VkDeviceMemory::fake_base_address

    std::vector<VkSparseImageMemoryRequirements> sparse_requirements;
    IMAGE_STATE(VkDevice dev, VkImage img, const VkImageCreateInfo *pCreateInfo);
    IMAGE_STATE(IMAGE_STATE const &rh_obj) = delete;

    VkImage image() const { return handle_.Cast<VkImage>(); }

    layer_data::unordered_set<IMAGE_STATE *> aliasing_images;
    bool IsCompatibleAliasing(IMAGE_STATE *other_image_state) const;

    bool IsCreateInfoEqual(const VkImageCreateInfo &other_createInfo) const;
    bool IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_createInfo) const;

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
        if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
            delete[] createInfo.pQueueFamilyIndices;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        Destroy();
    };

    void Destroy() override;

    void AddAliasingImage(IMAGE_STATE *bound_images);

    VkExtent3D GetSubresourceExtent(const VkImageSubresourceLayers &subresource) const;

    VkImageSubresourceRange NormalizeSubresourceRange(const VkImageSubresourceRange &range) const {
        return ::NormalizeSubresourceRange(createInfo, range);
    }

  protected:
    virtual void NotifyInvalidate(const LogObjectList &invalid_handles, bool unlink) override;
};

class IMAGE_VIEW_STATE : public BASE_NODE {
  public:
    VkImageViewCreateInfo create_info;
    const VkImageSubresourceRange normalized_subresource_range;
    const image_layout_map::RangeGenerator range_generator;
    VkSampleCountFlagBits samples;
    unsigned descriptor_format_bits;
    VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props;
    VkFormatFeatureFlags format_features;
    VkImageUsageFlags inherited_usage;  // from spec #resources-image-inherited-usage
    std::shared_ptr<IMAGE_STATE> image_state;
    IMAGE_VIEW_STATE(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci);
    IMAGE_VIEW_STATE(const IMAGE_VIEW_STATE &rh_obj) = delete;

    VkImageView image_view() const { return handle_.Cast<VkImageView>(); }

    virtual ~IMAGE_VIEW_STATE() { Destroy(); }

    bool OverlapSubresource(const IMAGE_VIEW_STATE &compare_view) const;

    void Destroy() override {
        if (image_state) {
            image_state->RemoveParent(this);
        }
        BASE_NODE::Destroy();
    }
};

struct SWAPCHAIN_IMAGE {
    IMAGE_STATE *image_state = nullptr;
    layer_data::unordered_set<IMAGE_STATE *> bound_images;
};

class SWAPCHAIN_NODE : public BASE_NODE {
  public:
    safe_VkSwapchainCreateInfoKHR createInfo;
    std::vector<SWAPCHAIN_IMAGE> images;
    bool retired = false;
    bool shared_presentable = false;
    uint32_t get_swapchain_image_count = 0;
    SWAPCHAIN_NODE(const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR swapchain)
        : BASE_NODE(swapchain, kVulkanObjectTypeSwapchainKHR), createInfo(pCreateInfo) {}

    VkSwapchainKHR swapchain() const { return handle_.Cast<VkSwapchainKHR>(); }
};
