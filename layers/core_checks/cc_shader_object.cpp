/* Copyright (c) 2023 Nintendo
 * Copyright (c) 2023 LunarG, Inc.
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

#include "core_validation.h"

bool CoreChecks::PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                 const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                 VkShaderEXT* pShaders) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const VkShaderCreateInfoEXT& createInfo = pCreateInfos[i];
        if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
            createInfo.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if (enabled_features.core.tessellationShader == VK_FALSE) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08419",
                                 "vkCreateShadersEXT(): tessellationShader feature is not enabled, but createInfo[%" PRIu32
                                 "].stage is %s.",
                                 i, string_VkShaderStageFlagBits(createInfo.stage));
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if (enabled_features.core.geometryShader == VK_FALSE) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08420",
                                 "vkCreateShadersEXT(): geometryShader feature is not enabled, but createInfo[%" PRIu32
                                 "].stage is VK_SHADER_STAGE_GEOMETRY_BIT.",
                                 i);
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if (enabled_features.mesh_shader_features.taskShader == VK_FALSE) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08421",
                                    "vkCreateShadersEXT(): taskShader feature is not enabled, but createInfo[%" PRIu32
                                    "].stage is VK_SHADER_STAGE_TASK_BIT_EXT.",
                                    i);
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if (enabled_features.mesh_shader_features.meshShader == VK_FALSE) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08422",
                                    "vkCreateShadersEXT(): meshShader feature is not enabled, but createInfo[%" PRIu32
                                    "].stage is VK_SHADER_STAGE_MESH_BIT_EXT.",
                                    i);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                 const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;

    if (enabled_features.shader_object_features.shaderObject == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkDestroyShaderEXT-None-08481", "vkDestroyShaderEXT(): shaderObject feature is not enabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                  const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders) const {
    bool skip = false;

    const auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);

    if (enabled_features.shader_object_features.shaderObject == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkCmdBindShadersEXT-None-08462", "vkCmdBindShadersEXT(): shaderObject feature is not enabled.");
    }

    uint32_t vertexStageIndex = stageCount;
    uint32_t taskStageIndex = stageCount;
    uint32_t meshStageIndex = stageCount;
    for (uint32_t i = 0; i < stageCount; ++i) {
        const VkShaderStageFlagBits& stage = pStages[i];
        VkShaderEXT shader = pShaders ? pShaders[i] : VK_NULL_HANDLE;

        for (uint32_t j = i; j < stageCount; ++j) {
            if (i != j && stage == pStages[j]) {
                skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pStages-08463",
                                 "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] and pStages[%" PRIu32 "] are both %s.", i, j,
                                 string_VkShaderStageFlagBits(stage));
            }
        }

        if (stage == VK_SHADER_STAGE_VERTEX_BIT && shader != VK_NULL_HANDLE) {
            vertexStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT && shader != VK_NULL_HANDLE) {
            taskStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && shader != VK_NULL_HANDLE) {
            meshStageIndex = i;
        } else if (enabled_features.core.tessellationShader == VK_FALSE &&
                   (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) &&
                   shader != VK_NULL_HANDLE) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08474",
                             "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s and pShaders[%" PRIu32
                             "] is not VK_NULL_HANDLE, but tessellationShader feature is not enabled.",
                             i, string_VkShaderStageFlagBits(stage), i);
        } else if (enabled_features.core.geometryShader == VK_FALSE && stage == VK_SHADER_STAGE_GEOMETRY_BIT &&
                   shader != VK_NULL_HANDLE) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08475",
                             "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is VK_SHADER_STAGE_GEOMETRY_BIT and pShaders[%" PRIu32
                             "] is not VK_NULL_HANDLE, but geometryShader feature is not enabled.",
                             i, i);
        } else if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_COMPUTE_BIT) == 0) {
                skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08476",
                                 "vkCmdBindShadersEXT(): pStages[%" PRIu32
                                 "] is VK_SHADER_STAGE_COMPUTE_BIT, but the command pool the command buffer %s was allocated from "
                                 "does not support compute operations (%s).",
                                 i, FormatHandle(cb_state->commandBuffer()).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) >
            0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08477",
                                 "vkCmdBindShadersEXT(): pStages[%" PRIu32
                                 "] is %s, but the command pool the command buffer %s was allocated from "
                                 "does not support graphics operations (%s).",
                                 i, string_VkShaderStageFlagBits(stage), FormatHandle(cb_state->commandBuffer()).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) > 0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08478",
                                 "vkCmdBindShadersEXT(): pStages[%" PRIu32
                                 "] is %s, but the command pool the command buffer %s was allocated from "
                                 "does not support graphics operations (%s).",
                                 i, string_VkShaderStageFlagBits(stage), FormatHandle(cb_state->commandBuffer()).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if (stage == VK_SHADER_STAGE_TASK_BIT_EXT && enabled_features.mesh_shader_features.taskShader == VK_FALSE &&
            shader != VK_NULL_HANDLE) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08490",
                                "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s and pShaders[%" PRIu32
                                "] is not VK_NULL_HANDLE, but taskShader feature is not enabled.",
                                i, string_VkShaderStageFlagBits(stage), i);
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && enabled_features.mesh_shader_features.meshShader == VK_FALSE &&
                    shader != VK_NULL_HANDLE) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08491",
                                "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s and pShaders[%" PRIu32
                                "] is not VK_NULL_HANDLE, but meshShader feature is not enabled.",
                                i, string_VkShaderStageFlagBits(stage), i);
        }
        if (stage == VK_SHADER_STAGE_ALL_GRAPHICS || stage == VK_SHADER_STAGE_ALL) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pStages-08464", "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s.",
                             i, string_VkShaderStageFlagBits(stage));
        }
        if ((stage & (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                        VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)) >
            0) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pStages-08465",
                                "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s.", i, string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pStages-08467", "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s.",
                             i, string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pStages-08468", "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is %s.",
                             i, string_VkShaderStageFlagBits(stage));
        }
    }

    if (vertexStageIndex != stageCount && taskStageIndex != stageCount) {
        skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08470",
                            "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                            "] is VK_SHADER_STAGE_TASK_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                            "] are VK_NULL_HANDLE.",
                            vertexStageIndex, taskStageIndex, vertexStageIndex, taskStageIndex);
    }
    if (vertexStageIndex != stageCount && meshStageIndex != stageCount) {
        skip |= LogError(device, "VUID-vkCmdBindShadersEXT-pShaders-08471",
                            "vkCmdBindShadersEXT(): pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                            "] is VK_SHADER_STAGE_MESH_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                            "] are VK_NULL_HANDLE.",
                            vertexStageIndex, meshStageIndex, vertexStageIndex, meshStageIndex);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) const {
    bool skip = false;

    if (enabled_features.shader_object_features.shaderObject == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkGetShaderBinaryDataEXT-None-08461", "vkGetShaderBinaryDataEXT(): shaderObject feature is not enabled.");
    }

    return skip;
}
