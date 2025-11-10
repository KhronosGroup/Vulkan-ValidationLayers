/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Copyright (C) 2019-2025 Google Inc.
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
 * John Zulauf <jzulauf@lunarg.com>
 *
 */
#pragma once

#include <functional>

#include "containers/custom_containers.h"
#include "containers/subresource_adapter.h"

constexpr VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

// Stores the image layout of each subresource of a single image.
// It is used to track the actual current layout (as opposed to record time tracking)
using ImageLayoutMap = subresource_adapter::BothRangeMap<VkImageLayout, 16>;

// Image layout state during command buffer recording
struct ImageLayoutState {
    VkImageLayout current_layout;

    // Tracks the first known layout of the subresource in the command buffer.
    // This value is tracked based on the expected layout parameters from various API functions.
    // For example, for vkCmdCopyImageToBuffer the expected layout is the srcImageLayout parameter,
    // and for image barrier it is the oldLayout.
    VkImageLayout first_layout;

    // Aspect mask is used to normalize layouts before comparison.
    // TODO: consider storing two additional normalized layouts instead of this field
    VkImageAspectFlags aspect_mask;

    // Submit time validation compares first_layout against image's current global layout.
    // If not null, this vuid is used to report an error.
    // If null, the current implementation will use VUID-vkCmdDraw-None-09600 until we fix all the places
    const char* submit_time_layout_mismatch_vuid;
};

// Tracks image layout state of each subresource of a single image during record time.
// Each command buffer has ImageLayoutRegistery that tracks all images.
class CommandBufferImageLayoutMap : public subresource_adapter::BothRangeMap<ImageLayoutState, 16> {
  public:
    CommandBufferImageLayoutMap(subresource_adapter::IndexType subresource_count, uint32_t image_id)
        : subresource_adapter::BothRangeMap<ImageLayoutState, 16>(subresource_count), image_id(image_id) {}
    const uint32_t image_id;
};
using ImageLayoutRegistry = vvl::unordered_map<VkImage, std::shared_ptr<CommandBufferImageLayoutMap>>;

// Update image layout state during command buffer recording phase.
// The VkImageLayout parameters must be unnormalized values (as defined by the API) so they can be used in the error messages.
bool UpdateCurrentLayout(CommandBufferImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& range_gen,
                         VkImageLayout layout, VkImageLayout expected_layout, VkImageAspectFlags aspect_mask);

// Track image layout at the beginning of the command buffer.
// Typically called by the APIs that specify the expected layout but do not perform a layout transition.
// The VkImageLayout parameter must be unnormalized value (as defined by the API) so it can be used in the error messages.
void TrackFirstLayout(CommandBufferImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& range_gen,
                      VkImageLayout expected_layout, VkImageAspectFlags aspect_mask, const char* submit_time_layout_mismatch_vuid);

// Iterate over layout map subresource ranges that intersect with the ranges defined by RangeGenerator.
// Runs the callback on each matching layout map range.
// Returns skip status (check todo in the implementation)
bool ForEachMatchingLayoutMapRange(
    const CommandBufferImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& gen,
    std::function<bool(const subresource_adapter::IndexRange& range, const ImageLayoutState& entry)>&& func);

bool ForEachMatchingLayoutMapRange(
    const ImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& gen,
    std::function<bool(const subresource_adapter::IndexRange& range, VkImageLayout image_layout)>&& func);
