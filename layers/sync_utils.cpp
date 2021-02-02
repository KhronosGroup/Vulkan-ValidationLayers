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
#include "sync_utils.h"
#include "state_tracker.h"
#include "descriptor_sets.h"
#include "core_validation_types.h"
#include "synchronization_validation_types.h"

namespace sync_utils {
static constexpr uint32_t kNumPipelineStageBits = sizeof(VkPipelineStageFlags) * 8;

VkPipelineStageFlags DisabledPipelineStages(const DeviceFeatures &features) {
    VkPipelineStageFlags result = 0;
    if (!features.core.geometryShader) {
        result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if (!features.core.tessellationShader) {
        result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (!features.conditional_rendering.conditionalRendering) {
        result |= VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
    }
    if (!features.fragment_density_map_features.fragmentDensityMap) {
        result |= VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
    }
    if (!features.transform_feedback_features.transformFeedback) {
        result |= VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
    }
    if (!features.mesh_shader.meshShader) {
        result |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;
    }
    if (!features.mesh_shader.taskShader) {
        result |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
    }
    if (!features.fragment_shading_rate_features.pipelineFragmentShadingRate && !features.shading_rate_image.shadingRateImage) {
        result |= VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }
    // TODO: VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
    // TODO: VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR
    return result;
}

VkPipelineStageFlags ExpandPipelineStages(VkPipelineStageFlags stage_mask, VkQueueFlags queue_flags,
                                          const VkPipelineStageFlags disabled_feature_mask) {
    VkPipelineStageFlags expanded = stage_mask;
    if (VK_PIPELINE_STAGE_ALL_COMMANDS_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        for (const auto &all_commands : syncAllCommandStagesByQueueFlags) {
            if (all_commands.first & queue_flags) {
                expanded |= all_commands.second & ~disabled_feature_mask;
            }
        }
    }
    if (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        expanded |= syncAllCommandStagesByQueueFlags.at(VK_QUEUE_GRAPHICS_BIT) & ~disabled_feature_mask;
        expanded &= ~VK_PIPELINE_STAGE_HOST_BIT;
    }
    return expanded;
}

VkAccessFlags CompatibleAccessMask(VkPipelineStageFlags stage_mask) {
    VkAccessFlags result = 0;
    stage_mask = ExpandPipelineStages(stage_mask);
    for (size_t i = 0; i < kNumPipelineStageBits; i++) {
        VkPipelineStageFlags bit = 1ULL << i;
        if (stage_mask & bit) {
            auto access_rec = syncDirectStageToAccessMask.find(bit);
            if (access_rec != syncDirectStageToAccessMask.end()) {
                result |= access_rec->second;
                continue;
            }
        }
    }
    return result;
}

VkPipelineStageFlags RelatedPipelineStages(VkPipelineStageFlags stage_mask,
                                           const std::map<VkPipelineStageFlagBits, VkPipelineStageFlags> &map) {
    VkPipelineStageFlags unscanned = stage_mask;
    VkPipelineStageFlags related = 0;
    for (const auto &entry : map) {
        const auto &stage = entry.first;
        if (stage & unscanned) {
            related = related | entry.second;
            unscanned = unscanned & ~stage;
            if (!unscanned) break;
        }
    }
    return related;
}

VkPipelineStageFlags WithEarlierPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyEarlierStages);
}

VkPipelineStageFlags WithLaterPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyLaterStages);
}

int GetGraphicsPipelineStageLogicalOrdinal(VkPipelineStageFlags flag) {
    const auto &rec = syncStageOrder.find(static_cast<VkPipelineStageFlagBits>(flag));
    if (rec == syncStageOrder.end()) {
        return -1;
    }
    return rec->second;
}

// The following two functions technically have O(N^2) complexity, but it's for a value of O that's largely
// stable and also rather tiny - this could definitely be rejigged to work more efficiently, but the impact
// on runtime is currently negligible, so it wouldn't gain very much.
// If we add a lot more graphics pipeline stages, this set of functions should be rewritten to accomodate.
VkPipelineStageFlags GetLogicallyEarliestGraphicsPipelineStage(VkPipelineStageFlags inflags) {
    VkPipelineStageFlags earliest_bit = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    int earliest_bit_order = GetGraphicsPipelineStageLogicalOrdinal(earliest_bit);

    inflags = ExpandPipelineStages(inflags);
    for (std::size_t i = 0; i < kNumPipelineStageBits; ++i) {
        VkPipelineStageFlags current_flag = (inflags & 0x1ull) << i;
        if (current_flag) {
            int new_order = GetGraphicsPipelineStageLogicalOrdinal(current_flag);
            if (new_order != -1 && new_order < earliest_bit_order) {
                earliest_bit_order = new_order;
                earliest_bit = current_flag;
            }
        }
        inflags = inflags >> 1;
    }
    return earliest_bit;
}

VkPipelineStageFlags GetLogicallyLatestGraphicsPipelineStage(VkPipelineStageFlags inflags) {
    VkPipelineStageFlags latest_bit = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    int latest_bit_order = GetGraphicsPipelineStageLogicalOrdinal(latest_bit);

    inflags = ExpandPipelineStages(inflags);
    for (std::size_t i = 0; i < kNumPipelineStageBits; ++i) {
        VkPipelineStageFlags current_flag = (inflags & 0x1ull) << i;
        if (current_flag) {
            int new_order = GetGraphicsPipelineStageLogicalOrdinal(current_flag);
            if (new_order != -1 && new_order > latest_bit_order) {
                latest_bit_order = new_order;
                latest_bit = current_flag;
            }
        }
        inflags = inflags >> 1;
    }
    return latest_bit;
}

}  // namespace sync_utils
