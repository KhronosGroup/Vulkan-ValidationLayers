/*
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
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
#include "sync/sync_utils.h"
#include "state_tracker/state_tracker.h"
#include "generated/enum_flag_bits.h"

namespace sync_utils {
static constexpr uint32_t kNumPipelineStageBits = sizeof(VkPipelineStageFlags2KHR) * 8;

VkPipelineStageFlags2KHR DisabledPipelineStages(const DeviceFeatures &features) {
    VkPipelineStageFlags2KHR result = 0;
    if (!features.geometryShader) {
        result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if (!features.tessellationShader) {
        result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (!features.conditionalRendering) {
        result |= VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
    }
    if (!features.fragmentDensityMap) {
        result |= VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
    }
    if (!features.transformFeedback) {
        result |= VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
    }
    if (!features.meshShader) {
        result |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
    }
    if (!features.taskShader) {
        result |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
    }
    if (!features.attachmentFragmentShadingRate && !features.shadingRateImage) {
        result |= VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }
    // TODO: VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
    // TODO: VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR
    return result;
}

VkPipelineStageFlags2KHR ExpandPipelineStages(VkPipelineStageFlags2KHR stage_mask, VkQueueFlags queue_flags,
                                              const VkPipelineStageFlags2KHR disabled_feature_mask) {
    VkPipelineStageFlags2KHR expanded = stage_mask;

    if (VK_PIPELINE_STAGE_ALL_COMMANDS_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        for (const auto &all_commands : syncAllCommandStagesByQueueFlags()) {
            if (all_commands.first & queue_flags) {
                expanded |= all_commands.second & ~disabled_feature_mask;
            }
        }
    }
    if (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        // Make sure we don't pull in the HOST stage from expansion, but keep it if set by the caller.
        // The syncAllCommandStagesByQueueFlags table includes HOST for all queue types since it is
        // allowed but it shouldn't be part of ALL_GRAPHICS
        expanded |=
            syncAllCommandStagesByQueueFlags().at(VK_QUEUE_GRAPHICS_BIT) & ~disabled_feature_mask & ~VK_PIPELINE_STAGE_HOST_BIT;
    }
    if (VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR;
        expanded |= VK_PIPELINE_STAGE_2_COPY_BIT_KHR | VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR | VK_PIPELINE_STAGE_2_BLIT_BIT_KHR |
                    VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR;
    }
    if (VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR;
        expanded |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT_KHR | VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR;
    }
    if (VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR;
        expanded |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR |
                    VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR;
    }

    return expanded;
}

static const auto kShaderReadExpandBits =
    VK_ACCESS_2_UNIFORM_READ_BIT_KHR | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR;
static const auto kShaderWriteExpandBits = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR;

VkAccessFlags2KHR ExpandAccessFlags(VkAccessFlags2KHR access_mask) {
    VkAccessFlags2KHR expanded = access_mask;

    if (VK_ACCESS_2_SHADER_READ_BIT_KHR & access_mask) {
        expanded = expanded & ~VK_ACCESS_2_SHADER_READ_BIT_KHR;
        expanded |= kShaderReadExpandBits;
    }

    if (VK_ACCESS_2_SHADER_WRITE_BIT_KHR & access_mask) {
        expanded = expanded & ~VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
        expanded |= kShaderWriteExpandBits;
    }

    return expanded;
}

VkAccessFlags2KHR CompatibleAccessMask(VkPipelineStageFlags2KHR stage_mask) {
    VkAccessFlags2KHR result = 0;
    stage_mask = ExpandPipelineStages(stage_mask);
    for (size_t i = 0; i < kNumPipelineStageBits; i++) {
        VkPipelineStageFlags2KHR bit = 1ULL << i;
        if (stage_mask & bit) {
            auto access_rec = syncDirectStageToAccessMask().find(bit);
            if (access_rec != syncDirectStageToAccessMask().end()) {
                result |= access_rec->second;
                continue;
            }
        }
    }

    // put the meta-access bits back on
    if (result & kShaderReadExpandBits) {
        result |= VK_ACCESS_2_SHADER_READ_BIT_KHR;
    }

    if (result & kShaderWriteExpandBits) {
        result |= VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
    }

    return result;
}

VkPipelineStageFlags2KHR RelatedPipelineStages(VkPipelineStageFlags2KHR stage_mask,
                                               const std::map<VkPipelineStageFlags2KHR, VkPipelineStageFlags2KHR> &map) {
    VkPipelineStageFlags2KHR unscanned = stage_mask;
    VkPipelineStageFlags2KHR related = 0;
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

VkPipelineStageFlags2KHR WithEarlierPipelineStages(VkPipelineStageFlags2KHR stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyEarlierStages());
}

VkPipelineStageFlags2KHR WithLaterPipelineStages(VkPipelineStageFlags2KHR stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyLaterStages());
}

// helper to extract the union of the stage masks in all of the barriers
ExecScopes GetGlobalStageMasks(const VkDependencyInfoKHR &dep_info) {
    ExecScopes result{};
    for (uint32_t i = 0; i < dep_info.memoryBarrierCount; i++) {
        result.src |= dep_info.pMemoryBarriers[i].srcStageMask;
        result.dst |= dep_info.pMemoryBarriers[i].dstStageMask;
    }
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        result.src |= dep_info.pBufferMemoryBarriers[i].srcStageMask;
        result.dst |= dep_info.pBufferMemoryBarriers[i].dstStageMask;
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        result.src |= dep_info.pImageMemoryBarriers[i].srcStageMask;
        result.dst |= dep_info.pImageMemoryBarriers[i].dstStageMask;
    }
    return result;
}

// Helpers to try to print the shortest string description of masks.
// If the bitmask doesn't use a synchronization2 specific flag, we'll
// print the old strings. There are common code paths where we need
// to print masks as strings and this makes the output less confusing
// for people not using synchronization2.
std::string StringPipelineStageFlags(VkPipelineStageFlags2KHR mask) {
    VkPipelineStageFlags sync1_mask = static_cast<VkPipelineStageFlags>(mask & AllVkPipelineStageFlagBits);
    if (sync1_mask) {
        return string_VkPipelineStageFlags(sync1_mask);
    }
    return string_VkPipelineStageFlags2(mask);
}

std::string StringAccessFlags(VkAccessFlags2KHR mask) {
    VkAccessFlags sync1_mask = static_cast<VkAccessFlags>(mask & AllVkAccessFlagBits);
    if (sync1_mask) {
        return string_VkAccessFlags(sync1_mask);
    }
    return string_VkAccessFlags2(mask);
}

ShaderStageAccesses GetShaderStageAccesses(VkShaderStageFlagBits shader_stage) {
    static const std::map<VkShaderStageFlagBits, ShaderStageAccesses> map = {
        // clang-format off
        {VK_SHADER_STAGE_VERTEX_BIT, {
            SYNC_VERTEX_SHADER_SHADER_SAMPLED_READ,
            SYNC_VERTEX_SHADER_SHADER_STORAGE_READ,
            SYNC_VERTEX_SHADER_SHADER_STORAGE_WRITE,
            SYNC_VERTEX_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, {
            SYNC_TESSELLATION_CONTROL_SHADER_SHADER_SAMPLED_READ,
            SYNC_TESSELLATION_CONTROL_SHADER_SHADER_STORAGE_READ,
            SYNC_TESSELLATION_CONTROL_SHADER_SHADER_STORAGE_WRITE,
            SYNC_TESSELLATION_CONTROL_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, {
            SYNC_TESSELLATION_EVALUATION_SHADER_SHADER_SAMPLED_READ,
            SYNC_TESSELLATION_EVALUATION_SHADER_SHADER_STORAGE_READ,
            SYNC_TESSELLATION_EVALUATION_SHADER_SHADER_STORAGE_WRITE,
            SYNC_TESSELLATION_EVALUATION_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_GEOMETRY_BIT, {
            SYNC_GEOMETRY_SHADER_SHADER_SAMPLED_READ,
            SYNC_GEOMETRY_SHADER_SHADER_STORAGE_READ,
            SYNC_GEOMETRY_SHADER_SHADER_STORAGE_WRITE,
            SYNC_GEOMETRY_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_FRAGMENT_BIT, {
            SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ,
            SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ,
            SYNC_FRAGMENT_SHADER_SHADER_STORAGE_WRITE,
            SYNC_FRAGMENT_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_COMPUTE_BIT, {
            SYNC_COMPUTE_SHADER_SHADER_SAMPLED_READ,
            SYNC_COMPUTE_SHADER_SHADER_STORAGE_READ,
            SYNC_COMPUTE_SHADER_SHADER_STORAGE_WRITE,
            SYNC_COMPUTE_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_RAYGEN_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_ANY_HIT_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_MISS_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_INTERSECTION_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_CALLABLE_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_TASK_BIT_EXT, {
            SYNC_TASK_SHADER_EXT_SHADER_SAMPLED_READ,
            SYNC_TASK_SHADER_EXT_SHADER_STORAGE_READ,
            SYNC_TASK_SHADER_EXT_SHADER_STORAGE_WRITE,
            SYNC_TASK_SHADER_EXT_UNIFORM_READ
        }},
        {VK_SHADER_STAGE_MESH_BIT_EXT, {
            SYNC_MESH_SHADER_EXT_SHADER_SAMPLED_READ,
            SYNC_MESH_SHADER_EXT_SHADER_STORAGE_READ,
            SYNC_MESH_SHADER_EXT_SHADER_STORAGE_WRITE,
            SYNC_MESH_SHADER_EXT_UNIFORM_READ
        }},
        // clang-format on
    };
    auto it = map.find(shader_stage);
    assert(it != map.end());
    return it->second;
}

const std::shared_ptr<const BUFFER_STATE> BufferBarrier::GetResourceState(const ValidationStateTracker &state_tracker) const {
    return state_tracker.Get<BUFFER_STATE>(buffer);
}

const std::shared_ptr<const IMAGE_STATE> ImageBarrier::GetResourceState(const ValidationStateTracker &state_tracker) const {
    return state_tracker.Get<IMAGE_STATE>(image);
}

}  // namespace sync_utils
