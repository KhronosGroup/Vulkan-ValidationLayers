/* Copyright (c) 2019-2025 The Khronos Group Inc.
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

#include "sync_utils.h"
#include "generated/device_features.h"
#include "generated/enum_flag_bits.h"
#include "generated/sync_validation_types.h"
#include "generated/vk_extension_helper.h"
#include "utils/hash_vk_types.h"
#include <vulkan/vk_enum_string_helper.h>

size_t QFOImageTransferBarrier::hash() const {
    // Ignoring the layout information for the purpose of the hash, as we're interested in QFO release/acquisition w.r.t.
    // the subresource affected, an layout transitions are current validated on another path
    auto hc = base_hash_combiner() << subresourceRange;
    return hc.Value();
}

bool QFOImageTransferBarrier::operator==(const QFOImageTransferBarrier &rhs) const {
    // Ignoring layout w.r.t. equality. See comment in hash above.
    return (static_cast<BaseType>(*this) == static_cast<BaseType>(rhs)) && (subresourceRange == rhs.subresourceRange);
}

size_t QFOBufferTransferBarrier::hash() const {
    auto hc = base_hash_combiner() << offset << size;
    return hc.Value();
}

bool QFOBufferTransferBarrier::operator==(const QFOBufferTransferBarrier &rhs) const {
    return (static_cast<BaseType>(*this) == static_cast<BaseType>(rhs)) && (offset == rhs.offset) && (size == rhs.size);
}

namespace sync_utils {
// IMPORTANT: the features listed here should also be reflected in GetFeatureNameMap()
VkPipelineStageFlags2 DisabledPipelineStages(const DeviceFeatures &features, const DeviceExtensions &device_extensions) {
    VkPipelineStageFlags2 result = 0;
    if (!features.geometryShader) {
        result |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
    }
    if (!features.tessellationShader) {
        result |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (!features.conditionalRendering) {
        result |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT;
    }
    if (!features.fragmentDensityMap) {
        result |= VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
    }
    if (!features.transformFeedback) {
        result |= VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT;
    }
    if (!features.meshShader) {
        result |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
    }
    if (!features.taskShader) {
        result |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
    }
    if (!features.attachmentFragmentShadingRate && !features.shadingRateImage) {
        result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }
    if (!features.subpassShading) {
        result |= VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI;
    }
    if (!features.invocationMask) {
        result |= VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI;
    }
    if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) && !features.rayTracingPipeline) {
        result |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    }
    // The NV extension includes the accelerationStructure implicitly
    if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing) && !features.accelerationStructure) {
        result |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    }
    if (!features.rayTracingMaintenance1) {
        result |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
    }
    if (!features.micromap) {
        result |= VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT;
    }
    return result;
}

VkAccessFlags2 DisabledAccesses(const DeviceExtensions &device_extensions) {
    VkAccessFlags2 result = 0;
    if (!IsExtEnabled(device_extensions.vk_qcom_tile_shading)) {
        result |= VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM | VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM;
    }
    return result;
}

// Helpers to try to print the shortest string description of masks.
// If the bitmask doesn't use a synchronization2 specific flag, we'll
// print the old strings. There are common code paths where we need
// to print masks as strings and this makes the output less confusing
// for people not using synchronization2.
//
// TODO 2025: It is also confusing for people using sync2 that some masks
// printed as STAGE_and some as STAGE_2_ for the same app. The suggestion
// to rework this so the function itself prefers a single style - sync2, so
// it covers all cases, but maybe introduce overload or additional parameter
// so the caller can specify which version to use (if we really need this).
std::string StringPipelineStageFlags(VkPipelineStageFlags2 mask) {
    VkPipelineStageFlags sync1_mask = static_cast<VkPipelineStageFlags>(mask & AllVkPipelineStageFlagBits);
    if (sync1_mask) {
        return string_VkPipelineStageFlags(sync1_mask);
    }
    return string_VkPipelineStageFlags2(mask);
}

VkPipelineStageFlags2 ExpandPipelineStages(VkPipelineStageFlags2 stage_mask, VkQueueFlags queue_flags,
                                           const VkPipelineStageFlags2 disabled_feature_mask) {
    VkPipelineStageFlags2 expanded = stage_mask;

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
    if (VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        expanded |= kAllTransferExpandBits;
    }
    if (VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
        expanded |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT | VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    }
    if (VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT & stage_mask) {
        expanded &= ~VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
        expanded |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
    }

    return expanded;
}

VkAccessFlags2 CompatibleAccessMask(VkPipelineStageFlags2 stage_mask) {
    static constexpr uint32_t kNumPipelineStageBits = sizeof(VkPipelineStageFlags2) * 8;
    VkAccessFlags2 result = 0;
    stage_mask = ExpandPipelineStages(stage_mask, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
    for (size_t i = 0; i < kNumPipelineStageBits; i++) {
        VkPipelineStageFlags2 bit = 1ULL << i;
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
        result |= VK_ACCESS_2_SHADER_READ_BIT;
    }

    if (result & kShaderWriteExpandBits) {
        result |= VK_ACCESS_2_SHADER_WRITE_BIT;
    }

    return result;
}

std::string StringAccessFlags(VkAccessFlags2 mask) {
    VkAccessFlags sync1_mask = static_cast<VkAccessFlags>(mask & AllVkAccessFlagBits);
    if (sync1_mask) {
        return string_VkAccessFlags(sync1_mask);
    }
    return string_VkAccessFlags2(mask);
}

ExecScopes GetExecScopes(const VkDependencyInfo &dep_info) {
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

}  // namespace sync_utils
