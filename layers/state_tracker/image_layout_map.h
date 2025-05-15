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

#include "containers/subresource_adapter.h"
#include "containers/container_utils.h"
#include "utils/vk_layer_utils.h"
#include "error_message/logging.h"

namespace vvl {
class Image;
}  // namespace vvl

constexpr VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

struct LayoutEntry {
    // This tracks current subresource layout as we progress through the command buffer
    VkImageLayout current_layout;

    // This tracks the first known layout of the subresource in the command buffer (that's why initial).
    // This value is tracked based on the expected layout parameters from various API functions.
    // For example, for vkCmdCopyImageToBuffer the expected layout is the srcImageLayout parameter,
    // and for image barrier it is the oldLayout.
    VkImageLayout initial_layout;

    // For relaxed matching rules
    VkImageAspectFlags aspect_mask;
};

class CommandBufferImageLayoutMap : public subresource_adapter::BothRangeMap<LayoutEntry, 16> {
  public:
    CommandBufferImageLayoutMap(const vvl::Image& image_state);
    const uint32_t image_id;
};

using ImageLayoutMap = subresource_adapter::BothRangeMap<VkImageLayout, 16>;

using CommandBufferImageLayoutRegistry = vvl::unordered_map<VkImage, std::shared_ptr<CommandBufferImageLayoutMap>>;
using ImageLayoutRegistry = vvl::unordered_map<const vvl::Image*, std::optional<ImageLayoutMap>>;

// Set current layout. If API also defines the expected layout it can be specified too.
bool UpdateCurrentLayout(CommandBufferImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& range_gen,
                         VkImageLayout layout, VkImageLayout expected_layout = kInvalidLayout);

// Tracks image first layout in the command buffer. This is usually used by the APIs that do not perform
// layout transitions but just manifest the expected layout, e.g. srcImageLayout parameter in vkCmdCopyImageToBuffer.
// The aspect mask is used if API additionally restricts subresource to specific aspect (descriptor image views).
void TrackInitialLayout(CommandBufferImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& range_gen,
                        VkImageLayout expected_layout, VkImageAspectFlags aspect_mask = 0);

// TODO: document and rename me
bool AnyInRange(const CommandBufferImageLayoutMap& image_layout_map, const vvl::Image& image_state,
                const VkImageSubresourceRange& normalized_subresource_range,
                std::function<bool(const subresource_adapter::IndexRange& range, const LayoutEntry& state)>&& func);
bool AnyInRange(const CommandBufferImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator&& gen,
                std::function<bool(const subresource_adapter::IndexRange& range, const LayoutEntry& state)>&& func);
bool AnyInRange(const ImageLayoutMap& image_layout_map, subresource_adapter::RangeGenerator& gen,
                std::function<bool(const subresource_adapter::IndexRange& range, VkImageLayout image_layout)>&& func);
