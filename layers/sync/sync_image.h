/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include "sync/sync_submit.h"
#include "state_tracker/image_state.h"
#include "state_tracker/wsi_state.h"

namespace syncval_state {

class ImageSubState : public vvl::ImageSubState {
  public:
    ImageSubState(vvl::Image &image);

    bool IsLinear() const { return fragment_encoder.IsLinearImage(); }
    bool IsTiled() const { return !IsLinear(); }
    bool IsSimplyBound() const;

    void SetOpaqueBaseAddress(vvl::DeviceState &dev_data);

    VkDeviceSize GetOpaqueBaseAddress() const { return opaque_base_address_; }
    bool HasOpaqueMapping() const { return 0U != opaque_base_address_; }
    VkDeviceSize GetResourceBaseAddress() const;
    ImageRangeGen MakeImageRangeGen(const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const;
    ImageRangeGen MakeImageRangeGen(const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                    const VkExtent3D &extent, bool is_depth_sliced) const;

  protected:
    VkDeviceSize opaque_base_address_ = 0U;
    const subresource_adapter::ImageRangeEncoder fragment_encoder;
};

static inline ImageSubState &SubState(vvl::Image &img) {
    return *static_cast<ImageSubState *>(img.SubState(LayerObjectTypeSyncValidation));
}

static inline const ImageSubState &SubState(const vvl::Image &img) {
    return *static_cast<const ImageSubState *>(img.SubState(LayerObjectTypeSyncValidation));
}

ImageRangeGen MakeImageRangeGen(const vvl::ImageView &view);
ImageRangeGen MakeImageRangeGen(const vvl::ImageView &view, const VkOffset3D &offset, const VkExtent3D &extent,
                                VkImageAspectFlags override_depth_stencil_aspect_mask = 0);

class SwapchainSubState : public vvl::SwapchainSubState {
  public:
    SwapchainSubState(vvl::Swapchain &swapchain) : vvl::SwapchainSubState(swapchain) {}
    ~SwapchainSubState() { Destroy(); }
    void RecordPresentedImage(PresentedImage &&presented_images);
    PresentedImage MovePresentedImage(uint32_t image_index);
    void GetPresentBatches(std::vector<QueueBatchContext::Ptr> &batches) const;

  private:
    PresentedImages presented;  // Build this on demand
};

static inline SwapchainSubState &SubState(vvl::Swapchain &sc) {
    return *static_cast<SwapchainSubState *>(sc.SubState(LayerObjectTypeSyncValidation));
}

static inline const SwapchainSubState &SubState(const vvl::Swapchain &sc) {
    return *static_cast<const SwapchainSubState *>(sc.SubState(LayerObjectTypeSyncValidation));
}

}  // namespace syncval_state
