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
#include "state_tracker/shader_object_state.h"
#include "generated/spirv_grammar_helper.h"

VkShaderStageFlags FindNextStage(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, VkShaderStageFlagBits stage) {
    constexpr uint32_t graphicsStagesCount = 5;
    constexpr uint32_t meshStagesCount = 3;
    const VkShaderStageFlagBits graphicsStages[graphicsStagesCount] = {
        VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderStageFlagBits meshStages[meshStagesCount] = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT,
                                                               VK_SHADER_STAGE_FRAGMENT_BIT};

    uint32_t graphicsIndex = graphicsStagesCount;
    uint32_t meshIndex = meshStagesCount;
    for (uint32_t i = 0; i < graphicsStagesCount; ++i) {
        if (graphicsStages[i] == stage) {
            graphicsIndex = i;
            break;
        }
        if (i < meshStagesCount && meshStages[i] == stage) {
            meshIndex = i;
            break;
        }
    }

    if (graphicsIndex < graphicsStagesCount) {
        while (++graphicsIndex < graphicsStagesCount) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                if (pCreateInfos[i].stage == graphicsStages[graphicsIndex]) {
                    return graphicsStages[graphicsIndex];
                }
            }
        }
    } else {
        while (++meshIndex < meshStagesCount) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                if (pCreateInfos[i].stage == meshStages[meshIndex]) {
                    return meshStages[meshIndex];
                }
            }
        }
    }

    return 0;
}

bool CoreChecks::PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                 const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                 VkShaderEXT* pShaders, const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |=
            LogError("VUID-vkCreateShadersEXT-None-08400", device, error_obj.location, "the shaderObject feature was not enabled.");
    }

    const uint32_t invalid = createInfoCount;
    uint32_t linked_stage = invalid;
    uint32_t non_linked_graphics_stage = invalid;
    uint32_t non_linked_task_mesh_stage = invalid;
    uint32_t linked_task_mesh_stage = invalid;
    uint32_t linked_vert_stage = invalid;
    uint32_t linked_task_stage = invalid;
    uint32_t linked_mesh_no_task_stage = invalid;
    uint32_t linked_spirv_index = invalid;
    uint32_t linked_binary_index = invalid;
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkShaderCreateInfoEXT& createInfo = pCreateInfos[i];
        if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
            createInfo.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if (enabled_features.tessellationShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08419", device, create_info_loc.dot(Field::stage),
                                 "is %s, but the tessellationShader feature was not enabled.",
                                 string_VkShaderStageFlagBits(createInfo.stage));
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if (enabled_features.geometryShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08420", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_GEOMETRY_BIT, but the geometryShader feature was not enabled.");
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if (enabled_features.taskShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08421", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TASK_BIT_EXT, but the taskShader feature was not enabled.");
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if (enabled_features.meshShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08422", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_MESH_BIT_EXT, but the meshShader feature was not enabled.");
            }
        }

        if ((createInfo.flags & VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT) != 0 &&
            enabled_features.attachmentFragmentShadingRate == VK_FALSE) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08487", device, create_info_loc.dot(Field::flags),
                             "is %s, but the attachmentFragmentShadingRate feature was not enabled.",
                             string_VkShaderCreateFlagsEXT(createInfo.flags).c_str());
        }
        if ((createInfo.flags & VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) != 0 &&
            enabled_features.fragmentDensityMap == VK_FALSE) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08489", device, create_info_loc.dot(Field::flags),
                             "is %s, but the fragmentDensityMap feature was not enabled.",
                             string_VkShaderCreateFlagsEXT(createInfo.flags).c_str());
        }

        if ((createInfo.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0 && createInfoCount == 1) {
            skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08401", device, create_info_loc.dot(Field::flags),
                             "is %s, but createInfoCount is 1.", string_VkShaderCreateFlagsEXT(createInfo.flags).c_str());
        }
        if ((createInfo.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            const auto nextStage = FindNextStage(createInfoCount, pCreateInfos, createInfo.stage);
            if (nextStage != 0 && createInfo.nextStage != nextStage) {
                skip |=
                    LogError("VUID-vkCreateShadersEXT-pCreateInfos-08409", device, create_info_loc.dot(Field::flags),
                             "is %s, but nextStage (%s) does not equal the "
                             "logically next stage (%s) which also has the VK_SHADER_CREATE_LINK_STAGE_BIT_EXT bit.",
                             string_VkShaderCreateFlagsEXT(createInfo.flags).c_str(),
                             string_VkShaderStageFlags(createInfo.nextStage).c_str(), string_VkShaderStageFlags(nextStage).c_str());
            }
            for (uint32_t j = i; j < createInfoCount; ++j) {
                if (i != j && createInfo.stage == pCreateInfos[j].stage) {
                    skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08410", device, create_info_loc,
                                     "and pCreateInfos[%" PRIu32
                                     "] both contain VK_SHADER_CREATE_LINK_STAGE_BIT_EXT and have the stage %s.",
                                     j, string_VkShaderStageFlagBits(createInfo.stage));
                }
            }

            linked_stage = i;
            if ((createInfo.stage & VK_SHADER_STAGE_VERTEX_BIT) != 0) {
                linked_vert_stage = i;
            } else if ((createInfo.stage & VK_SHADER_STAGE_TASK_BIT_EXT) != 0) {
                linked_task_mesh_stage = i;
                linked_task_stage = i;
            } else if ((createInfo.stage & VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                linked_task_mesh_stage = i;
                if ((createInfo.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0) {
                    linked_mesh_no_task_stage = i;
                }
            }
            if (createInfo.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
                linked_spirv_index = i;
            } else if (createInfo.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
                linked_binary_index = i;
            }
        } else if ((createInfo.stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                        VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
            non_linked_graphics_stage = i;
        } else if ((createInfo.stage & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) != 0) {
            non_linked_task_mesh_stage = i;
        }

        if (enabled_features.tessellationShader == VK_FALSE &&
            (createInfo.nextStage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
             createInfo.nextStage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08428", device, create_info_loc.dot(Field::nextStage),
                             "is %s, but tessellationShader feature was not enabled.",
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }
        if (enabled_features.geometryShader == VK_FALSE && createInfo.nextStage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08429", device, create_info_loc.dot(Field::nextStage),
                             "is VK_SHADER_STAGE_GEOMETRY_BIT, but tessellationShader feature was not enabled.");
        }
        if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT &&
            (createInfo.nextStage & ~VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08430", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }
        if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT &&
            (createInfo.nextStage & ~(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08431", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }
        if (createInfo.stage == VK_SHADER_STAGE_GEOMETRY_BIT && (createInfo.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08433", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_GEOMETRY_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }
        if ((createInfo.stage == VK_SHADER_STAGE_FRAGMENT_BIT || createInfo.stage == VK_SHADER_STAGE_COMPUTE_BIT) &&
            createInfo.nextStage > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08434", device, create_info_loc.dot(Field::stage),
                             "is %s, but nextStage is %s.", string_VkShaderStageFlagBits(createInfo.stage),
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }
        if (createInfo.stage == VK_SHADER_STAGE_TASK_BIT_EXT && (createInfo.nextStage & ~VK_SHADER_STAGE_MESH_BIT_EXT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08435", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TASK_BIT_EXT, but nextStage is %s.",
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }
        if (createInfo.stage == VK_SHADER_STAGE_MESH_BIT_EXT && (createInfo.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08436", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_MESH_BIT_EXT, but nextStage is %s.",
                             string_VkShaderStageFlags(createInfo.nextStage).c_str());
        }

        if ((createInfo.flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 &&
            enabled_features.subgroupSizeControl == VK_FALSE) {
            skip |= LogError(
                kVUID_Core_Shader_AllowVaryingSubgroupSize, device, create_info_loc.dot(Field::flags),
                "contains VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, but subgroupSizeControl feature is not enabled.");
        }
        if ((createInfo.flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0 &&
            enabled_features.computeFullSubgroups == VK_FALSE) {
            skip |= LogError(
                kVUID_Core_Shader_RequireFullSubgroups, device, create_info_loc.dot(Field::flags),
                "contains VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but computeFullSubgroups feature is not enabled.");
        }
    }

    if (linked_stage != invalid && non_linked_graphics_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08402", device, error_obj.location,
                         "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                         "].flags contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but pCreateInfos[%" PRIu32
                         "] stage is %s and does not have VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         linked_stage, non_linked_graphics_stage,
                         string_VkShaderStageFlagBits(pCreateInfos[non_linked_graphics_stage].stage));
    }
    if (linked_stage != invalid && non_linked_task_mesh_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08403", device, error_obj.location,
                         "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                         "].flags contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but pCreateInfos[%" PRIu32
                         "] stage is %s and does not have VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         linked_stage, non_linked_task_mesh_stage,
                         string_VkShaderStageFlagBits(pCreateInfos[non_linked_task_mesh_stage].stage));
    }
    if (linked_vert_stage != invalid && linked_task_mesh_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08404", device, error_obj.location,
                         "pCreateInfos[%" PRIu32 "].stage is %s and pCreateInfos[%" PRIu32
                         "].stage is %s, but both contain VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         linked_vert_stage, string_VkShaderStageFlagBits(pCreateInfos[linked_vert_stage].stage),
                         linked_task_mesh_stage, string_VkShaderStageFlagBits(pCreateInfos[linked_task_mesh_stage].stage));
    }
    if (linked_task_stage != invalid && linked_mesh_no_task_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08405", device, error_obj.location,
                         "pCreateInfos[%" PRIu32 "] is a linked task shader, but pCreateInfos[%" PRIu32
                         "] is a linked mesh shader with VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT flag.",
                         linked_task_stage, linked_mesh_no_task_stage);
    }
    if (linked_spirv_index != invalid && linked_binary_index != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08411", device, error_obj.location,
                         "pCreateInfos[%" PRIu32
                         "] is a linked shader with codeType VK_SHADER_CODE_TYPE_SPIRV_EXT, but pCreateInfos[%" PRIu32
                         "] is a linked shader with codeType VK_SHADER_CODE_TYPE_BINARY_EXT.",
                         linked_spirv_index, linked_binary_index);
    }

    uint32_t tesc_linked_subdivision = 0u;
    uint32_t tese_linked_subdivision = 0u;
    uint32_t tesc_linked_orientation = 0u;
    uint32_t tese_linked_orientation = 0u;
    bool tesc_linked_point_mode = false;
    bool tese_linked_point_mode = false;
    uint32_t tesc_linked_spacing = 0u;
    uint32_t tese_linked_spacing = 0u;
    uint32_t tesc_output_patch_size = 0u;
    uint32_t tese_output_patch_size = 0u;
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (pCreateInfos[i].codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
            const StageCreateInfo stage_create_info(pCreateInfos[i]);
            const auto spirv =
                std::make_shared<SPIRV_MODULE_STATE>(pCreateInfos[i].codeSize, static_cast<const uint32_t*>(pCreateInfos[i].pCode));
            safe_VkShaderCreateInfoEXT safe_create_info = safe_VkShaderCreateInfoEXT(&pCreateInfos[i]);
            const PipelineStageState stage_state(nullptr, &safe_create_info, nullptr, spirv);
            skip |= ValidatePipelineShaderStage(stage_create_info, stage_state, create_info_loc);

            if (pCreateInfos[i].flags == VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) {
                if (pCreateInfos[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                    tesc_linked_subdivision = stage_state.entrypoint->execution_mode.tessellation_subdivision;
                    tesc_linked_orientation = stage_state.entrypoint->execution_mode.tessellation_orientation;
                    tesc_linked_point_mode = stage_state.entrypoint->execution_mode.flags & ExecutionModeSet::point_mode_bit;
                    tesc_linked_spacing = stage_state.entrypoint->execution_mode.tessellation_spacing;
                    tesc_output_patch_size = stage_state.entrypoint->execution_mode.output_vertices;
                } else if (pCreateInfos[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                    tese_linked_subdivision = stage_state.entrypoint->execution_mode.tessellation_subdivision;
                    tese_linked_orientation = stage_state.entrypoint->execution_mode.tessellation_orientation;
                    tese_linked_point_mode = stage_state.entrypoint->execution_mode.flags & ExecutionModeSet::point_mode_bit;
                    tese_linked_spacing = stage_state.entrypoint->execution_mode.tessellation_spacing;
                    tese_output_patch_size = stage_state.entrypoint->execution_mode.output_vertices;
                }
            }
        }
    }

    if (tesc_linked_subdivision != 0 && tese_linked_subdivision != 0 && tesc_linked_subdivision != tese_linked_subdivision) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08867", device, error_obj.location,
                         "The subdivision specified in tessellation control shader (%s) does not match the subdivision in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_linked_subdivision), string_SpvExecutionMode(tese_linked_subdivision));
    }
    if (tesc_linked_orientation != 0 && tese_linked_orientation != 0 && tesc_linked_orientation != tese_linked_orientation) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08868", device, error_obj.location,
                         "The orientation specified in tessellation control shader (%s) does not match the orientation in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_linked_orientation), string_SpvExecutionMode(tese_linked_orientation));
    }
    if (tesc_linked_point_mode && !tese_linked_point_mode) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08869", device, error_obj.location,
                         "The tessellation control shader specifies execution mode point mode, but the tessellation evaluation "
                         "shader does not.");
    }
    if (tesc_linked_spacing != 0 && tese_linked_spacing != 0 && tesc_linked_spacing != tese_linked_spacing) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08870", device, error_obj.location,
                         "The spacing specified in tessellation control shader (%s) does not match the spacing in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_linked_spacing), string_SpvExecutionMode(tese_linked_spacing));
    }
    if (tesc_output_patch_size != tese_output_patch_size && tese_output_patch_size != 0) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08871", device, error_obj.location,
                         "The output patch size in tessellation control shader (%" PRIu32
                         ") does not match the output patch size in tessellation evaluation shader (%" PRIu32 ").",
                         tesc_output_patch_size, tese_output_patch_size);
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |=
            LogError("VUID-vkDestroyShaderEXT-None-08481", device, error_obj.location, "the shaderObject feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                  const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;

    const auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-None-08462", device, error_obj.location,
                         "the shaderObject feature was not enabled.");
    }

    uint32_t vertexStageIndex = stageCount;
    uint32_t taskStageIndex = stageCount;
    uint32_t meshStageIndex = stageCount;
    for (uint32_t i = 0; i < stageCount; ++i) {
        const Location stage_loc = error_obj.location.dot(Field::pStages, i);
        const VkShaderStageFlagBits& stage = pStages[i];
        VkShaderEXT shader = pShaders ? pShaders[i] : VK_NULL_HANDLE;

        for (uint32_t j = i; j < stageCount; ++j) {
            if (i != j && stage == pStages[j]) {
                skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08463", device, stage_loc,
                                 "and pStages[%" PRIu32 "] are both %s.", j, string_VkShaderStageFlagBits(stage));
            }
        }

        if (stage == VK_SHADER_STAGE_VERTEX_BIT && shader != VK_NULL_HANDLE) {
            vertexStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT && shader != VK_NULL_HANDLE) {
            taskStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && shader != VK_NULL_HANDLE) {
            meshStageIndex = i;
        } else if (enabled_features.tessellationShader == VK_FALSE &&
                   (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) &&
                   shader != VK_NULL_HANDLE) {
            skip |=
                LogError("VUID-vkCmdBindShadersEXT-pShaders-08474", device, stage_loc,
                         "is %s and pShaders[%" PRIu32 "] is not VK_NULL_HANDLE, but tessellationShader feature was not enabled.",
                         string_VkShaderStageFlagBits(stage), i);
        } else if (enabled_features.geometryShader == VK_FALSE && stage == VK_SHADER_STAGE_GEOMETRY_BIT &&
                   shader != VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08475", device, stage_loc,
                             "is VK_SHADER_STAGE_GEOMETRY_BIT and pShaders[%" PRIu32
                             "] is not VK_NULL_HANDLE, but geometryShader feature was not enabled.",
                             i);
        } else if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_COMPUTE_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |=
                    LogError("VUID-vkCmdBindShadersEXT-pShaders-08476", objlist, stage_loc,
                             "is VK_SHADER_STAGE_COMPUTE_BIT, but the command pool the command buffer (%s) was allocated from "
                             "does not support compute operations (%s).",
                             FormatHandle(commandBuffer).c_str(), string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) >
            0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08477", objlist, stage_loc,
                                 "is %s, but the command pool the command buffer %s was allocated from "
                                 "does not support graphics operations (%s).",
                                 string_VkShaderStageFlagBits(stage), FormatHandle(commandBuffer).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) > 0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08478", objlist, stage_loc,
                                 "is %s, but the command pool the command buffer %s was allocated from "
                                 "does not support graphics operations (%s).",
                                 string_VkShaderStageFlagBits(stage), FormatHandle(commandBuffer).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if (stage == VK_SHADER_STAGE_TASK_BIT_EXT && enabled_features.taskShader == VK_FALSE && shader != VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08490", device, stage_loc,
                             "is %s and pShaders[%" PRIu32 "] is not VK_NULL_HANDLE, but taskShader feature was not enabled.",
                             string_VkShaderStageFlagBits(stage), i);
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && enabled_features.meshShader == VK_FALSE && shader != VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08491", device, stage_loc,
                             "is %s and pShaders[%" PRIu32 "] is not VK_NULL_HANDLE, but meshShader feature was not enabled.",
                             string_VkShaderStageFlagBits(stage), i);
        }
        if (stage == VK_SHADER_STAGE_ALL_GRAPHICS || stage == VK_SHADER_STAGE_ALL) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08464", device, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if ((stage & (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                      VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)) >
            0) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08465", device, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08467", device, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08468", device, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (shader != VK_NULL_HANDLE) {
            const auto shader_state = Get<SHADER_OBJECT_STATE>(shader);
            if (shader_state->create_info.stage != stage) {
                skip |=
                    LogError("VUID-vkCmdBindShadersEXT-pShaders-08469", device, stage_loc,
                             "is %s, but pShaders[%" PRIu32 "] was created with shader stage %s.",
                             string_VkShaderStageFlagBits(stage), i, string_VkShaderStageFlagBits(shader_state->create_info.stage));
            }
        }
    }

    if (vertexStageIndex != stageCount && taskStageIndex != stageCount) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08470", device, error_obj.location,
                         "pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                         "] is VK_SHADER_STAGE_TASK_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                         "] are VK_NULL_HANDLE.",
                         vertexStageIndex, taskStageIndex, vertexStageIndex, taskStageIndex);
    }
    if (vertexStageIndex != stageCount && meshStageIndex != stageCount) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08471", device, error_obj.location,
                         "pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                         "] is VK_SHADER_STAGE_MESH_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                         "] are VK_NULL_HANDLE.",
                         vertexStageIndex, meshStageIndex, vertexStageIndex, meshStageIndex);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |= LogError("VUID-vkGetShaderBinaryDataEXT-None-08461", device, error_obj.location,
                         "the shaderObject feature was not enabled.");
    }

    return skip;
}
