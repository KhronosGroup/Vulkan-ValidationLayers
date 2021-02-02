/*
 * Copyright (c) 2019-2021 Valve Corporation
 * Copyright (c) 2019-2021 LunarG, Inc.
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
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Locke Lin <locke@lunarg.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */

#pragma once
#include <vulkan/vulkan.h>
#include <array>

struct DeviceFeatures;

namespace sync_utils {

static constexpr VkQueueFlags kAllQueueTypes = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

VkPipelineStageFlags DisabledPipelineStages(const DeviceFeatures& features);

// Expand all pipeline stage bits. If queue_flags and disabled_feature_mask is provided, the expansion of ALL_COMMANDS_BIT
// and ALL_GRAPHICS_BIT will be limited to what is supported.
VkPipelineStageFlags ExpandPipelineStages(VkPipelineStageFlags stage_mask, VkQueueFlags queue_flags = kAllQueueTypes,
                                          const VkPipelineStageFlags disabled_feature_mask = 0);

VkAccessFlags CompatibleAccessMask(VkPipelineStageFlags stage_mask);

VkPipelineStageFlags WithEarlierPipelineStages(VkPipelineStageFlags stage_mask);

VkPipelineStageFlags WithLaterPipelineStages(VkPipelineStageFlags stage_mask);

int GetGraphicsPipelineStageLogicalOrdinal(VkPipelineStageFlags flag);

VkPipelineStageFlags GetLogicallyEarliestGraphicsPipelineStage(VkPipelineStageFlags inflags);

VkPipelineStageFlags GetLogicallyLatestGraphicsPipelineStage(VkPipelineStageFlags inflags);

}  // namespace sync_utils
