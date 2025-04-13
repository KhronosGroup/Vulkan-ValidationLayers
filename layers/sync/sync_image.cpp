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
#include "sync_image.h"
#include "state_tracker/state_tracker.h"

syncval_state::ImageSubState::ImageSubState(vvl::Image &image) : vvl::ImageSubState(image), fragment_encoder(image) {}

bool syncval_state::ImageSubState::IsSimplyBound() const {
    bool simple = SimpleBinding(base) || base.IsSwapchainImage() || base.bind_swapchain;
    return simple;
}

void syncval_state::ImageSubState::SetOpaqueBaseAddress(vvl::DeviceState &dev_data) {
    // This is safe to call if already called to simplify caller logic
    // NOTE: Not asserting IsTiled, as there could in future be other reasons for opaque representations
    if (opaque_base_address_) return;

    VkDeviceSize opaque_base = 0U;  // Fakespace Allocator starts > 0
    auto get_opaque_base = [&opaque_base](const vvl::Image &other) {
        const auto &other_sync = syncval_state::SubState(other);
        opaque_base = other_sync.opaque_base_address_;
        return true;
    };
    if (base.IsSwapchainImage()) {
        base.AnyAliasBindingOf(base.bind_swapchain->ObjectBindings(), get_opaque_base);
    } else {
        base.AnyImageAliasOf(get_opaque_base);
    }
    if (!opaque_base) {
        // The size of the opaque range is based on the SyncVal *internal* representation of the tiled resource, unrelated
        // to the acutal size of the the resource in device memory. If differing representations become possible, the allocated
        // size would need to be changed to those representation's size requirements.
        opaque_base = dev_data.AllocFakeMemory(fragment_encoder.TotalSize());
    }
    opaque_base_address_ = opaque_base;
}

VkDeviceSize syncval_state::ImageSubState::GetResourceBaseAddress() const {
    if (HasOpaqueMapping()) {
        return GetOpaqueBaseAddress();
    }
    return base.GetFakeBaseAddress();
}

ImageRangeGen syncval_state::ImageSubState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                              bool is_depth_sliced) const {
    if (!IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    ImageRangeGen range_gen(fragment_encoder, subresource_range, base_address, is_depth_sliced);
    return range_gen;
}

ImageRangeGen syncval_state::ImageSubState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                              const VkOffset3D &offset, const VkExtent3D &extent,
                                                              bool is_depth_sliced) const {
    if (!IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    subresource_adapter::ImageRangeGenerator range_gen(fragment_encoder, subresource_range, offset, extent, base_address,
                                                       is_depth_sliced);
    return range_gen;
}

ImageRangeGen syncval_state::MakeImageRangeGen(const vvl::ImageView &view) {
    const auto &sub_state = SubState(*view.image_state);
    return sub_state.MakeImageRangeGen(view.normalized_subresource_range, view.is_depth_sliced);
}

ImageRangeGen syncval_state::MakeImageRangeGen(const vvl::ImageView &view, const VkOffset3D &offset, const VkExtent3D &extent,
                                               VkImageAspectFlags override_depth_stencil_aspect_mask) {
    if (view.Invalid()) ImageRangeGen();

    VkImageSubresourceRange subresource_range = view.normalized_subresource_range;

    if (override_depth_stencil_aspect_mask != 0) {
        assert((override_depth_stencil_aspect_mask & kDepthStencilAspects) == override_depth_stencil_aspect_mask);
        subresource_range.aspectMask = override_depth_stencil_aspect_mask;
    }
    const auto &sub_state = SubState(*view.image_state);
    return sub_state.MakeImageRangeGen(subresource_range, offset, extent, view.is_depth_sliced);
}
