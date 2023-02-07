/*
 * Copyright (c) 2019-2021, 2023 Valve Corporation
 * Copyright (c) 2019-2021, 2023 LunarG, Inc.
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
#include <vulkan/vulkan.h>
#include <string>

struct DeviceFeatures;

namespace sync_utils {

static constexpr VkQueueFlags kAllQueueTypes = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

VkPipelineStageFlags2KHR DisabledPipelineStages(const DeviceFeatures& features);

// Expand all pipeline stage bits. If queue_flags and disabled_feature_mask is provided, the expansion of ALL_COMMANDS_BIT
// and ALL_GRAPHICS_BIT will be limited to what is supported.
VkPipelineStageFlags2KHR ExpandPipelineStages(VkPipelineStageFlags2KHR stage_mask, VkQueueFlags queue_flags = kAllQueueTypes,
                                              const VkPipelineStageFlags2KHR disabled_feature_mask = 0);

VkAccessFlags2KHR ExpandAccessFlags(VkAccessFlags2KHR access_mask);

VkAccessFlags2KHR CompatibleAccessMask(VkPipelineStageFlags2KHR stage_mask);

VkPipelineStageFlags2KHR WithEarlierPipelineStages(VkPipelineStageFlags2KHR stage_mask);

VkPipelineStageFlags2KHR WithLaterPipelineStages(VkPipelineStageFlags2KHR stage_mask);

int GetGraphicsPipelineStageLogicalOrdinal(VkPipelineStageFlags2KHR flag);

VkPipelineStageFlags2KHR GetLogicallyEarliestGraphicsPipelineStage(VkPipelineStageFlags2KHR inflags);

VkPipelineStageFlags2KHR GetLogicallyLatestGraphicsPipelineStage(VkPipelineStageFlags2KHR inflags);

std::string StringPipelineStageFlags(VkPipelineStageFlags2KHR mask);

std::string StringAccessFlags(VkAccessFlags2KHR mask);

struct ExecScopes {
    VkPipelineStageFlags2KHR src;
    VkPipelineStageFlags2KHR dst;
};

ExecScopes GetGlobalStageMasks(const VkDependencyInfoKHR& dep_info);

}  // namespace sync_utils
