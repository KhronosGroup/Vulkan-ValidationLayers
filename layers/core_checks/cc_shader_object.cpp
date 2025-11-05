/* Copyright (c) 2023-2025 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include <vulkan/vulkan_core.h>
#include <spirv/unified1/spirv.hpp>
#include "chassis/chassis_modification_state.h"
#include "core_validation.h"
#include "error_message/logging.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "generated/spirv_grammar_helper.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "containers/limits.h"
#include "utils/action_command_utils.h"
#include "utils/shader_utils.h"

// In order of how stages are linked together
static const std::array graphics_stages = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                           VK_SHADER_STAGE_FRAGMENT_BIT};
static const std::array mesh_stages = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_STAGE_FRAGMENT_BIT};

VkShaderStageFlags FindNextStage(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, VkShaderStageFlagBits stage) {
    uint32_t graphic_index = static_cast<uint32_t>(graphics_stages.size());
    uint32_t mesh_index = static_cast<uint32_t>(mesh_stages.size());
    for (uint32_t i = 0; i < graphics_stages.size(); ++i) {
        if (graphics_stages[i] == stage) {
            graphic_index = i;
            break;
        }
        if (i < mesh_stages.size() && mesh_stages[i] == stage) {
            mesh_index = i;
            break;
        }
    }

    if (graphic_index < graphics_stages.size()) {
        while (++graphic_index < graphics_stages.size()) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                if (pCreateInfos[i].stage == graphics_stages[graphic_index]) {
                    return graphics_stages[graphic_index];
                }
            }
        }
    } else {
        while (++mesh_index < mesh_stages.size()) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                if (pCreateInfos[i].stage == mesh_stages[mesh_index]) {
                    return mesh_stages[mesh_index];
                }
            }
        }
    }

    return 0;
}

bool CoreChecks::ValidateCreateShadersLinking(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                              const Location& loc) const {
    bool skip = false;

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
        const Location create_info_loc = loc.dot(Field::pCreateInfos, i);
        const VkShaderCreateInfoEXT& create_info = pCreateInfos[i];
        if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
            create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if (enabled_features.tessellationShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08419", device, create_info_loc.dot(Field::stage),
                                 "is %s, but the tessellationShader feature was not enabled.",
                                 string_VkShaderStageFlagBits(create_info.stage));
            }
        } else if (create_info.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if (enabled_features.geometryShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08420", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_GEOMETRY_BIT, but the geometryShader feature was not enabled.");
            }
        } else if (create_info.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if (enabled_features.taskShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08421", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TASK_BIT_EXT, but the taskShader feature was not enabled.");
            }
        } else if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if (enabled_features.meshShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08422", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_MESH_BIT_EXT, but the meshShader feature was not enabled.");
            }
        }

        if ((create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            const auto nextStage = FindNextStage(createInfoCount, pCreateInfos, create_info.stage);
            if (nextStage != 0 && create_info.nextStage != nextStage) {
                skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08409", device, create_info_loc.dot(Field::flags),
                                 "is %s, but nextStage (%s) does not equal the "
                                 "logically next stage (%s) which also has the VK_SHADER_CREATE_LINK_STAGE_BIT_EXT bit.",
                                 string_VkShaderCreateFlagsEXT(create_info.flags).c_str(),
                                 string_VkShaderStageFlags(create_info.nextStage).c_str(),
                                 string_VkShaderStageFlags(nextStage).c_str());
            }
            for (uint32_t j = i; j < createInfoCount; ++j) {
                if (i != j && create_info.stage == pCreateInfos[j].stage) {
                    skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08410", device, create_info_loc,
                                     "and pCreateInfos[%" PRIu32
                                     "] both contain VK_SHADER_CREATE_LINK_STAGE_BIT_EXT and have the stage %s.",
                                     j, string_VkShaderStageFlagBits(create_info.stage));
                }
            }

            linked_stage = i;
            if ((create_info.stage & VK_SHADER_STAGE_VERTEX_BIT) != 0) {
                linked_vert_stage = i;
            } else if ((create_info.stage & VK_SHADER_STAGE_TASK_BIT_EXT) != 0) {
                linked_task_mesh_stage = i;
                linked_task_stage = i;
            } else if ((create_info.stage & VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                linked_task_mesh_stage = i;
                if ((create_info.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0) {
                    linked_mesh_no_task_stage = i;
                }
            }
            if (create_info.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
                linked_spirv_index = i;
            } else if (create_info.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
                linked_binary_index = i;
            }
        } else if ((create_info.stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                         VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                         VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
            non_linked_graphics_stage = i;
        } else if ((create_info.stage & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) != 0) {
            non_linked_task_mesh_stage = i;
        }

        if (enabled_features.tessellationShader == VK_FALSE &&
            (create_info.nextStage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
             create_info.nextStage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08428", device, create_info_loc.dot(Field::nextStage),
                             "is %s, but tessellationShader feature was not enabled.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (enabled_features.geometryShader == VK_FALSE && create_info.nextStage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08429", device, create_info_loc.dot(Field::nextStage),
                             "is VK_SHADER_STAGE_GEOMETRY_BIT, but tessellationShader feature was not enabled.");
        }
        if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT &&
            (create_info.nextStage & ~VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08430", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT &&
            (create_info.nextStage & ~(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08431", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_GEOMETRY_BIT && (create_info.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08433", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_GEOMETRY_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if ((create_info.stage == VK_SHADER_STAGE_FRAGMENT_BIT || create_info.stage == VK_SHADER_STAGE_COMPUTE_BIT) &&
            create_info.nextStage > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08434", device, create_info_loc.dot(Field::stage),
                             "is %s, but nextStage is %s.", string_VkShaderStageFlagBits(create_info.stage),
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_TASK_BIT_EXT && (create_info.nextStage & ~VK_SHADER_STAGE_MESH_BIT_EXT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08435", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TASK_BIT_EXT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT && (create_info.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08436", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_MESH_BIT_EXT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
    }

    if (linked_stage != invalid && non_linked_graphics_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08402", device,
                         loc.dot(Field::pCreateInfos, linked_stage).dot(Field::flags),
                         "contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but pCreateInfos[%" PRIu32
                         "].stage is %s and does not have VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         non_linked_graphics_stage, string_VkShaderStageFlagBits(pCreateInfos[non_linked_graphics_stage].stage));
    }
    if (linked_stage != invalid && non_linked_task_mesh_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08403", device,
                         loc.dot(Field::pCreateInfos, linked_stage).dot(Field::flags),
                         "contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but pCreateInfos[%" PRIu32
                         "] stage is %s and does not have VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         non_linked_task_mesh_stage, string_VkShaderStageFlagBits(pCreateInfos[non_linked_task_mesh_stage].stage));
    }
    if (linked_vert_stage != invalid && linked_task_mesh_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08404", device,
                         loc.dot(Field::pCreateInfos, linked_vert_stage).dot(Field::stage),
                         "is %s and pCreateInfos[%" PRIu32 "].stage is %s, but both contain VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         string_VkShaderStageFlagBits(pCreateInfos[linked_vert_stage].stage), linked_task_mesh_stage,
                         string_VkShaderStageFlagBits(pCreateInfos[linked_task_mesh_stage].stage));
    }
    if (linked_task_stage != invalid && linked_mesh_no_task_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08405", device, loc.dot(Field::pCreateInfos, linked_task_stage),
                         "is a linked task shader, but pCreateInfos[%" PRIu32
                         "] is a linked mesh shader with VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT flag.",
                         linked_mesh_no_task_stage);
    }
    if (linked_spirv_index != invalid && linked_binary_index != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08411", device, loc.dot(Field::pCreateInfos, linked_spirv_index),
                         "is a linked shader with codeType VK_SHADER_CODE_TYPE_SPIRV_EXT, but pCreateInfos[%" PRIu32
                         "] is a linked shader with codeType VK_SHADER_CODE_TYPE_BINARY_EXT.",
                         linked_binary_index);
    }

    return skip;
}

bool CoreChecks::ValidateCreateShadersMesh(const VkShaderCreateInfoEXT& create_info, const spirv::Module& spirv,
                                           const Location& create_info_loc) const {
    bool skip = false;
    if (create_info.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) return skip;
    if (spirv.static_data_.has_builtin_draw_index) {
        skip |= LogError(
            "VUID-vkCreateShadersEXT-pCreateInfos-09632", device, create_info_loc,
            "the mesh Shader Object being created uses DrawIndex (gl_DrawID) which will be an undefined value when reading.");
    }
    return skip;
}

bool CoreChecks::ValidateCreateShadersSpirv(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                            const Location& loc, chassis::ShaderObject& chassis_state) const {
    bool skip = false;
    // If user has VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT, just skip all things related to creating the shader object
    if (disabled[shader_validation]) {
        return skip;
    }

    struct Tesc {
        bool is_linked = false;
        uint32_t subdivision = 0u;
        uint32_t orientation = 0u;
        bool point_mode = false;
        uint32_t spacing = 0u;
        uint32_t patch_size = 0u;
    } tesc;

    struct Tese {
        bool is_linked = false;
        uint32_t subdivision = 0u;
        uint32_t orientation = 0u;
        bool point_mode = false;
        uint32_t spacing = 0u;
        uint32_t patch_size = 0u;
    } tese;

    // Currently we don't provide a way for apps to supply their own cache for shader object
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3570
    ValidationCache* cache = CastFromHandle<ValidationCache*>(core_validation_cache);

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const VkShaderCreateInfoEXT& create_info = pCreateInfos[i];
        // Will be empty if not VK_SHADER_CODE_TYPE_SPIRV_EXT
        const std::shared_ptr<spirv::Module> spirv = chassis_state.module_states[i];

        if (!spirv || create_info.codeType != VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            continue;
        }

        const Location create_info_loc = loc.dot(Field::pCreateInfos, i);

        // Will do the "stateless" validation we also do at vkCreateShaderModule time
        skip |= stateless_spirv_validator.Validate(*spirv, chassis_state.stateless_data[i], create_info_loc);

        // Then we can run spirv-val
        spv_const_binary_t binary{static_cast<const uint32_t*>(create_info.pCode), create_info.codeSize / sizeof(uint32_t)};
        skip |= RunSpirvValidation(binary, create_info_loc, cache);

        // We need to do this here because we have not created the vvl::ShaderObject state yet
        vvl::DescriptorSetLayoutList set_layouts(create_info.setLayoutCount);
        for (uint32_t j = 0; j < create_info.setLayoutCount; ++j) {
            set_layouts.list[j] = Get<vvl::DescriptorSetLayout>(create_info.pSetLayouts[j]);
        }

        // Finally, we have "pipeline" level information and can do validation we normally do at pipeline creation time
        vku::safe_VkShaderCreateInfoEXT safe_create_info = vku::safe_VkShaderCreateInfoEXT(&pCreateInfos[i]);
        const ShaderStageState stage_state(nullptr, &safe_create_info, &set_layouts, nullptr, spirv, VK_NULL_HANDLE);
        skip |= ValidateShaderStage(stage_state, nullptr, create_info_loc);

        if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            skip |= ValidateCreateShadersMesh(create_info, *spirv, create_info_loc);
        }

        // We need to look for both tessellation stages to get information outside pCreateInfos loop
        if (stage_state.entrypoint && (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                                       create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            const spirv::ExecutionModeSet& execution_mode = stage_state.entrypoint->execution_mode;

            const uint32_t tessellation_subdivision = execution_mode.GetTessellationSubdivision();

            if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                if (tessellation_subdivision == spirv::kInvalidValue) {
                    skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-08872", device, create_info_loc.dot(Field::stage),
                                     "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but subdivision is not specified "
                                     "(Triangles/Quads/IsoLines).");
                }
            } else if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                if (execution_mode.output_vertices == spirv::kInvalidValue) {
                    skip |=
                        LogError("VUID-VkShaderCreateInfoEXT-codeType-08875", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but OutputVertices (patch size) is not specified.");
                } else if (execution_mode.output_vertices == 0u ||
                           execution_mode.output_vertices > phys_dev_props.limits.maxTessellationPatchSize) {
                    skip |= LogError("VUID-VkShaderCreateInfoEXT-pCode-08453", device, create_info_loc.dot(Field::pCode),
                                     "is using OutputVertices (patch size) %" PRIu32
                                     ", which is not between 1 and maxTessellationPatchSize (%" PRIu32 ").",
                                     execution_mode.output_vertices, phys_dev_props.limits.maxTessellationPatchSize);
                }
            }

            if ((create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0u) {
                if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                    tesc.is_linked = true;
                    tesc.subdivision = tessellation_subdivision;
                    tesc.orientation = execution_mode.GetTessellationOrientation();
                    tesc.point_mode = execution_mode.Has(spirv::ExecutionModeSet::point_mode_bit);
                    tesc.spacing = execution_mode.GetTessellationSpacing();
                    tesc.patch_size = execution_mode.output_vertices;
                } else if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                    tese.is_linked = true;
                    tese.subdivision = tessellation_subdivision;
                    tese.orientation = execution_mode.GetTessellationOrientation();
                    tese.point_mode = execution_mode.Has(spirv::ExecutionModeSet::point_mode_bit);
                    tese.spacing = execution_mode.GetTessellationSpacing();
                    tese.patch_size = execution_mode.output_vertices;
                }
            }
        }
    }

    if (tesc.is_linked && tese.is_linked) {
        if (tesc.subdivision != spirv::kInvalidValue && tese.subdivision != spirv::kInvalidValue &&
            tesc.subdivision != tese.subdivision) {
            skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08867", device, loc,
                             "The subdivision specified in tessellation control shader (%s) does not match the subdivision in "
                             "tessellation evaluation shader (%s).",
                             string_SpvExecutionMode(tesc.subdivision), string_SpvExecutionMode(tese.subdivision));
        }
        if (tesc.orientation != spirv::kInvalidValue && tese.orientation != spirv::kInvalidValue &&
            tesc.orientation != tese.orientation) {
            skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08868", device, loc,
                             "The orientation specified in tessellation control shader (%s) does not match the orientation in "
                             "tessellation evaluation shader (%s).",
                             string_SpvExecutionMode(tesc.orientation), string_SpvExecutionMode(tese.orientation));
        }
        if (tesc.spacing != spirv::kInvalidValue && tese.spacing != spirv::kInvalidValue && tesc.spacing != tese.spacing) {
            skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08870", device, loc,
                             "The spacing specified in tessellation control shader (%s) does not match the spacing in "
                             "tessellation evaluation shader (%s).",
                             string_SpvExecutionMode(tesc.spacing), string_SpvExecutionMode(tese.spacing));
        }
        if (tesc.patch_size != spirv::kInvalidValue && tese.patch_size != spirv::kInvalidValue &&
            tesc.patch_size != tese.patch_size) {
            skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08871", device, loc,
                             "The OutputVertices (patch size) specified in tessellation control shader (%" PRIu32
                             ") does not match the spacing in "
                             "tessellation evaluation shader (%" PRIu32 ").",
                             tesc.patch_size, tese.patch_size);
        }
    }

    return skip;
}

// This will only validate things that can be done without parsing the SPIR-V
bool CoreChecks::PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                 const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                 VkShaderEXT* pShaders, const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |=
            LogError("VUID-vkCreateShadersEXT-None-08400", device, error_obj.location, "the shaderObject feature was not enabled.");
    }

    skip |= ValidateCreateShadersLinking(createInfoCount, pCreateInfos, error_obj.location);

    bool has_compute = false;
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        has_compute = pCreateInfos[i].stage == VK_SHADER_STAGE_COMPUTE_BIT;
    }

    const VkQueueFlags queue_flag = has_compute ? VK_QUEUE_COMPUTE_BIT : VK_QUEUE_GRAPHICS_BIT;
    if ((physical_device_state->supported_queues & queue_flag) == 0) {
        // Used instead of calling ValidateDeviceQueueSupport()
        const char* vuid = has_compute ? "VUID-vkCreateShadersEXT-stage-09670" : "VUID-vkCreateShadersEXT-stage-09671";
        skip |=
            LogError(vuid, device, error_obj.location, "device only supports (%s) but require %s.",
                     string_VkQueueFlags(physical_device_state->supported_queues).c_str(), string_VkQueueFlags(queue_flag).c_str());
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

    if (const auto shader_state = Get<vvl::ShaderObject>(shader)) {
        skip |= ValidateObjectNotInUse(shader_state.get(), error_obj.location.dot(Field::shader),
                                       "VUID-vkDestroyShaderEXT-shader-08482");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                  const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    const auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-None-08462", commandBuffer, error_obj.location,
                         "the shaderObject feature was not enabled.");
    }

    uint32_t vertex_stage_index = stageCount;
    uint32_t task_stage_index = stageCount;
    uint32_t mesh_stage_index = stageCount;
    for (uint32_t i = 0; i < stageCount; ++i) {
        const Location stage_loc = error_obj.location.dot(Field::pStages, i);
        const VkShaderStageFlagBits& stage = pStages[i];
        VkShaderEXT shader = pShaders ? pShaders[i] : VK_NULL_HANDLE;

        for (uint32_t j = i; j < stageCount; ++j) {
            if (i != j && stage == pStages[j]) {
                skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08463", commandBuffer, stage_loc,
                                 "and pStages[%" PRIu32 "] are both %s.", j, string_VkShaderStageFlagBits(stage));
            }
        }

        if (stage == VK_SHADER_STAGE_VERTEX_BIT && shader != VK_NULL_HANDLE) {
            vertex_stage_index = i;
        } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT && shader != VK_NULL_HANDLE) {
            task_stage_index = i;
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && shader != VK_NULL_HANDLE) {
            mesh_stage_index = i;
        } else if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_COMPUTE_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError(
                    "VUID-vkCmdBindShadersEXT-pShaders-08476", objlist, stage_loc,
                    "is VK_SHADER_STAGE_COMPUTE_BIT, but %s was allocated from %s that does not support compute operations (%s).",
                    FormatHandle(commandBuffer).c_str(), FormatHandle(cb_state->command_pool->Handle()).c_str(),
                    string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) >
            0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08477", objlist, stage_loc,
                                 "is %s, but %s was allocated from %s that does not support graphics operations (%s).",
                                 string_VkShaderStageFlagBits(stage), FormatHandle(commandBuffer).c_str(),
                                 FormatHandle(cb_state->command_pool->Handle()).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) > 0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08478", objlist, stage_loc,
                                 "is %s, but %s was allocated from %s that does not support graphics operations (%s).",
                                 string_VkShaderStageFlagBits(stage), FormatHandle(commandBuffer).c_str(),
                                 FormatHandle(cb_state->command_pool->Handle()).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if (stage == VK_SHADER_STAGE_ALL_GRAPHICS || stage == VK_SHADER_STAGE_ALL) {
            // Use string_VkShaderStageFlags to print special flag names
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08464", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlags(stage).c_str());
        }
        if (stage & kShaderStageAllRayTracing) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08465", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08467", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08468", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (shader != VK_NULL_HANDLE) {
            const auto shader_state = Get<vvl::ShaderObject>(shader);
            if (shader_state && shader_state->create_info.stage != stage) {
                const LogObjectList objlist(commandBuffer, shader);
                skip |=
                    LogError("VUID-vkCmdBindShadersEXT-pShaders-08469", objlist, stage_loc,
                             "is %s, but pShaders[%" PRIu32 "] was created with shader stage %s.",
                             string_VkShaderStageFlagBits(stage), i, string_VkShaderStageFlagBits(shader_state->create_info.stage));
            }
        }
    }

    if (vertex_stage_index != stageCount && task_stage_index != stageCount) {
        const auto vertex_state = Get<vvl::ShaderObject>(pShaders[vertex_stage_index]);
        const auto task_state = Get<vvl::ShaderObject>(pShaders[task_stage_index]);
        const LogObjectList objlist(commandBuffer, vertex_state->Handle(), task_state->Handle());
        skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08470", objlist, error_obj.location,
                         "pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                         "] is VK_SHADER_STAGE_TASK_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                         "] are VK_NULL_HANDLE.",
                         vertex_stage_index, task_stage_index, vertex_stage_index, task_stage_index);
    }
    if (vertex_stage_index != stageCount && mesh_stage_index != stageCount) {
        const auto vertex_state = Get<vvl::ShaderObject>(pShaders[vertex_stage_index]);
        const auto mesh_state = Get<vvl::ShaderObject>(pShaders[mesh_stage_index]);
        const LogObjectList objlist(commandBuffer, vertex_state->Handle(), mesh_state->Handle());
        skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08471", objlist, error_obj.location,
                         "pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                         "] is VK_SHADER_STAGE_MESH_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                         "] are VK_NULL_HANDLE.",
                         vertex_stage_index, mesh_stage_index, vertex_stage_index, mesh_stage_index);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |= LogError("VUID-vkGetShaderBinaryDataEXT-None-08461", shader, error_obj.location,
                         "the shaderObject feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectNextStage(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;

    const auto& stages = last_bound_state.IsValidShaderBound(ShaderObjectStage::VERTEX)
                             ? vvl::span<const VkShaderStageFlagBits>(graphics_stages)
                             : vvl::span<const VkShaderStageFlagBits>(mesh_stages);
    VkShaderStageFlagBits previous_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    for (const auto stage : stages) {
        const ShaderObjectStage shader_object_stage = VkShaderStageToShaderObjectStage(stage);
        if (!last_bound_state.IsValidShaderBound(shader_object_stage)) {
            continue;
        }
        if (previous_stage != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM) {
            const auto previous_state = last_bound_state.GetShaderStateIfValid(VkShaderStageToShaderObjectStage(previous_stage));
            ASSERT_AND_CONTINUE(previous_state);
            if ((previous_state->create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) == 0 &&
                (previous_state->create_info.nextStage & stage) == 0) {
                const auto state = last_bound_state.GetShaderStateIfValid(shader_object_stage);
                const LogObjectList objlist(last_bound_state.cb_state.Handle(), previous_state->Handle(), state->Handle());
                skip |= LogError(vuid.next_stage_10745, objlist, vuid.loc(),
                                 "The combination of graphic shader objects bound is invalid, because "
                                 "shader stages %s (%s) and %s (%s) are bound with no other stages between them.\nThe %s shader "
                                 "was created with nextStage of %s but needs to be %s.",
                                 string_VkShaderStageFlagBits(previous_stage), FormatHandle(previous_state->Handle()).c_str(),
                                 string_VkShaderStageFlagBits(stage), FormatHandle(state->Handle()).c_str(),
                                 string_VkShaderStageFlagBits(previous_stage),
                                 string_VkShaderStageFlags(previous_state->create_info.nextStage).c_str(),
                                 string_VkShaderStageFlagBits(stage));
                return skip;  // only report on a single error
            }
        }
        previous_stage = stage;
    }
    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectBoundShader(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    if (!last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::VERTEX)) {
        const bool tried_mesh = last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::MESH);
        skip |= LogError(
            vuid.vertex_shader_08684, cb_state.Handle(), vuid.loc(),
            "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
            "VK_SHADER_STAGE_VERTEX_BIT%s.",
            tried_mesh ? " (Even if you are using a mesh shader, a VK_NULL_HANDLE must be bound to the vertex stage)" : "");
    }
    if (enabled_features.tessellationShader &&
        !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_CONTROL)) {
        skip |= LogError(vuid.tessellation_control_shader_08685, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT. (If the tessellationShader is enabled, the stage "
                         "needs to be provided, it can be bound with VK_NULL_HANDLE)");
    }
    if (enabled_features.tessellationShader &&
        !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_EVALUATION)) {
        skip |= LogError(vuid.tessellation_evaluation_shader_08686, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT. (If the tessellationShader is enabled, the stage "
                         "needs to be provided, it can be bound with VK_NULL_HANDLE)");
    }
    if (enabled_features.geometryShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::GEOMETRY)) {
        skip |= LogError(vuid.geometry_shader_08687, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_GEOMETRY_BIT. (If the geometryShader is enabled, the stage needs to be provided, it can "
                         "be bound with VK_NULL_HANDLE)");
    }
    if (!last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::FRAGMENT)) {
        skip |= LogError(vuid.fragment_shader_08688, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_FRAGMENT_BIT (Even if you are trying to a vertex/mesh only draw, a VK_NULL_HANDLE must "
                         "be bound to the fragment stage)");
    }
    if (enabled_features.taskShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TASK)) {
        skip |= LogError(vuid.task_shader_08689, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_TASK_BIT. (If the taskShader is enabled, the stage needs to be provided, it can be bound "
                         "with VK_NULL_HANDLE)");
    }
    if (enabled_features.meshShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::MESH)) {
        skip |= LogError(vuid.mesh_shader_08690, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_MESH_BIT. (If the meshShader is enabled, the stage needs to be provided, it can be bound "
                         "with VK_NULL_HANDLE)");
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObject(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    if (const vvl::RenderPass* rp_state = cb_state.active_render_pass.get()) {
        if (!rp_state->UsesDynamicRendering()) {
            skip |= LogError(vuid.render_pass_began_08876, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                             "Shader objects must be used with dynamic rendering, but VkRenderPass %s is active.",
                             FormatHandle(rp_state->Handle()).c_str());
        }
        if (rp_state->has_multiview_enabled) {
            const LogObjectList objlist(cb_state.Handle(), rp_state->Handle());
            skip |= LogError(vuid.shader_object_multiview_10772, objlist, vuid.loc(),
                             "render pass instance has multiview enabled, which is not allowed when using shader objects.");
        }
    }

    skip |= ValidateDrawShaderObjectNextStage(last_bound_state, vuid);
    skip |= ValidateDrawShaderObjectBoundShader(last_bound_state, vuid);
    skip |= ValidateDrawShaderObjectLinking(last_bound_state, vuid);
    skip |= ValidateDrawShaderObjectPushConstantAndLayout(last_bound_state, vuid);
    skip |= ValidateDrawShaderObjectMesh(last_bound_state, vuid);
    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectLinking(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const Location loc = vuid.loc();

    for (uint32_t i = 0; i < kShaderObjectStageCount; ++i) {
        if (i == static_cast<uint32_t>(ShaderObjectStage::COMPUTE) || !last_bound_state.shader_object_states[i]) {
            continue;
        }

        for (const auto& linked_shader : last_bound_state.shader_object_states[i]->linked_shaders) {
            bool found = false;
            for (uint32_t j = 0; j < kShaderObjectStageCount; ++j) {
                if (linked_shader == last_bound_state.GetShader(static_cast<ShaderObjectStage>(j))) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                const VkShaderEXT bound_shader = last_bound_state.GetShader(static_cast<ShaderObjectStage>(i));
                const auto missing_shader = Get<vvl::ShaderObject>(linked_shader);
                const LogObjectList objlist(cb_state.Handle(), bound_shader, missing_shader->Handle());
                skip |=
                    LogError(vuid.linked_shaders_08698, cb_state.Handle(), loc,
                             "Shader %s (%s) was created with VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but the linked %s "
                             "shader (%s) is not bound.",
                             FormatHandle(bound_shader).c_str(),
                             string_VkShaderStageFlagBits(last_bound_state.shader_object_states[i]->create_info.stage),
                             FormatHandle(linked_shader).c_str(), string_VkShaderStageFlagBits(missing_shader->create_info.stage));
                break;
            }
        }
    }

    VkShaderStageFlagBits prev_stage = VK_SHADER_STAGE_ALL;
    VkShaderStageFlagBits next_stage = VK_SHADER_STAGE_ALL;
    const vvl::ShaderObject* producer = nullptr;
    const vvl::ShaderObject* consumer = nullptr;

    for (const auto stage : graphics_stages) {
        if (skip) break;
        consumer = last_bound_state.GetShaderState(VkShaderStageToShaderObjectStage(stage));
        if (!consumer) continue;
        if (next_stage != VK_SHADER_STAGE_ALL && consumer->create_info.stage != next_stage) {
            const LogObjectList objlist(cb_state.Handle(), consumer->Handle());
            skip |= LogError(vuid.linked_shaders_08699, cb_state.Handle(), loc,
                             "Shaders %s and %s were created with VK_SHADER_CREATE_LINK_STAGE_BIT_EXT without intermediate "
                             "stage %s linked, but %s shader is bound.",
                             string_VkShaderStageFlagBits(prev_stage), string_VkShaderStageFlagBits(next_stage),
                             string_VkShaderStageFlagBits(stage), string_VkShaderStageFlagBits(stage));
            break;
        }

        next_stage = VK_SHADER_STAGE_ALL;
        if (!consumer->linked_shaders.empty()) {
            prev_stage = stage;
            for (const auto& linked_shader : consumer->linked_shaders) {
                const auto& linked_state = Get<vvl::ShaderObject>(linked_shader);
                if (linked_state && linked_state->create_info.stage == consumer->create_info.nextStage) {
                    next_stage = static_cast<VkShaderStageFlagBits>(consumer->create_info.nextStage);
                    break;
                }
            }
        }

        if (producer && consumer->spirv && producer->spirv && consumer->entrypoint && producer->entrypoint) {
            skip |= ValidateInterfaceBetweenStages(*producer->spirv, *producer->entrypoint, *consumer->spirv, *consumer->entrypoint,
                                                   loc);
        }
        producer = consumer;
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectPushConstantAndLayout(const LastBound& last_bound_state,
                                                               const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    const vvl::ShaderObject* first = nullptr;
    for (const auto shader_state : last_bound_state.shader_object_states) {
        if (!shader_state || !shader_state->IsGraphicsShaderState()) {
            continue;
        }
        if (!first) {
            first = shader_state;
            continue;
        }

        if (first->create_info.pushConstantRangeCount != shader_state->create_info.pushConstantRangeCount) {
            const LogObjectList objlist(cb_state.Handle(), first->Handle(), shader_state->Handle());
            skip |= LogError(vuid.shaders_push_constants_08878, objlist, vuid.loc(),
                             "The bound %s shader was created with a pushConstantRangeCount of %" PRIu32
                             " which doesn't match the bound %s shader create with a pushConstantRangeCount of %" PRIu32 "",
                             string_VkShaderStageFlagBits(first->create_info.stage), first->create_info.pushConstantRangeCount,
                             string_VkShaderStageFlagBits(shader_state->create_info.stage),
                             shader_state->create_info.pushConstantRangeCount);
        } else {
            bool found = false;  // find duplicate push constant ranges
            for (uint32_t i = 0; i < shader_state->create_info.pushConstantRangeCount; ++i) {
                for (uint32_t j = 0; j < first->create_info.pushConstantRangeCount; ++j) {
                    if (shader_state->create_info.pPushConstantRanges[i] == first->create_info.pPushConstantRanges[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    const LogObjectList objlist(cb_state.Handle(), first->Handle(), shader_state->Handle());
                    skip |= LogError(vuid.shaders_push_constants_08878, objlist, vuid.loc(),
                                     "The bound %s and %s shader have different push constant ranges.",
                                     string_VkShaderStageFlagBits(first->create_info.stage),
                                     string_VkShaderStageFlagBits(shader_state->create_info.stage));
                    break;
                }
            }
        }

        if (first->create_info.setLayoutCount != shader_state->create_info.setLayoutCount) {
            const LogObjectList objlist(cb_state.Handle(), first->Handle(), shader_state->Handle());
            skip |=
                LogError(vuid.shaders_descriptor_layouts_08879, objlist, vuid.loc(),
                         "The bound %s shader was created with a setLayoutCount of %" PRIu32
                         " which doesn't match the bound %s shader create with a setLayoutCount of %" PRIu32 "",
                         string_VkShaderStageFlagBits(first->create_info.stage), first->create_info.setLayoutCount,
                         string_VkShaderStageFlagBits(shader_state->create_info.stage), shader_state->create_info.setLayoutCount);
        } else {
            bool found = false;  // find duplicate set layouts
            for (uint32_t i = 0; i < shader_state->create_info.setLayoutCount; ++i) {
                for (uint32_t j = 0; j < first->create_info.setLayoutCount; ++j) {
                    if (shader_state->create_info.pSetLayouts[i] == first->create_info.pSetLayouts[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    const LogObjectList objlist(cb_state.Handle(), first->Handle(), shader_state->Handle());
                    skip |= LogError(vuid.shaders_descriptor_layouts_08879, objlist, vuid.loc(),
                                     "The bound %s and %s shader have different descriptor set layouts.",
                                     string_VkShaderStageFlagBits(first->create_info.stage),
                                     string_VkShaderStageFlagBits(shader_state->create_info.stage));
                    break;
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectMesh(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    const VkShaderEXT vertex_shader_handle = last_bound_state.GetShader(ShaderObjectStage::VERTEX);
    const VkShaderEXT task_shader_handle = last_bound_state.GetShader(ShaderObjectStage::TASK);
    const VkShaderEXT mesh_shader_handle = last_bound_state.GetShader(ShaderObjectStage::MESH);
    const bool has_vertex_shader = vertex_shader_handle != VK_NULL_HANDLE;
    const bool has_task_shader = task_shader_handle != VK_NULL_HANDLE;
    const bool has_mesh_shader = mesh_shader_handle != VK_NULL_HANDLE;

    const bool is_mesh_command = vvl::IsCommandDrawMesh(vuid.function);

    if (has_task_shader || has_mesh_shader) {
        auto print_mesh_task = [this, has_task_shader, has_mesh_shader, mesh_shader_handle, task_shader_handle]() {
            std::stringstream msg;
            if (has_task_shader && has_mesh_shader) {
                msg << "Task shader (" << FormatHandle(task_shader_handle).c_str() << ") and mesh shader ("
                    << FormatHandle(mesh_shader_handle).c_str() << ") are";
            } else if (has_task_shader) {
                msg << "Task shader (" << FormatHandle(task_shader_handle).c_str() << ") is";
            } else {
                msg << "Mesh shader (" << FormatHandle(mesh_shader_handle).c_str() << ") is";
            }
            return msg.str();
        };

        if (!is_mesh_command) {
            skip |= LogError(vuid.draw_shaders_no_task_mesh_08885, cb_state.Handle(), vuid.loc(),
                             "%s bound, but this is not a vkCmdDrawMeshTasks* call.", print_mesh_task().c_str());
        }
        if (has_vertex_shader) {
            skip |= LogError(vuid.vert_task_mesh_shader_08696, cb_state.Handle(), vuid.loc(),
                             "Vertex shader (%s) is bound, but %s bound as well.", FormatHandle(mesh_shader_handle).c_str(),
                             print_mesh_task().c_str());
        }
    }

    if (enabled_features.taskShader || enabled_features.meshShader) {
        if (has_vertex_shader && has_mesh_shader) {
            skip |= LogError(vuid.vert_mesh_shader_08693, cb_state.Handle(), vuid.loc(),
                             "Both vertex shader (%s) and mesh shader (%s) are bound, but only %s should be bound (the other needs "
                             "to be set to VK_NULL_HANDLE).",
                             FormatHandle(vertex_shader_handle).c_str(), FormatHandle(mesh_shader_handle).c_str(),
                             is_mesh_command ? "mesh" : "vertex");
        } else if (!has_vertex_shader && !has_mesh_shader) {
            // TODO - should have a dedicated VU or rework 08693
            skip |= LogError(vuid.vert_mesh_shader_08693, cb_state.Handle(), vuid.loc(),
                             "Neither vertex shader nor mesh shader are bound (for %s you need %s)", String(vuid.function),
                             is_mesh_command ? "mesh" : "vertex");
        }
    }

    if (enabled_features.taskShader && enabled_features.meshShader && is_mesh_command && has_mesh_shader) {
        if (const auto mesh_state = last_bound_state.GetShaderState(ShaderObjectStage::MESH)) {
            const bool no_task_shader_flag = (mesh_state->create_info.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0;

            if (!no_task_shader_flag && !has_task_shader) {
                skip |= LogError(
                    vuid.task_mesh_shader_08694, cb_state.Handle(), vuid.loc(),
                    "Mesh shader (%s) was created without VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT, but no task shader is bound.",
                    FormatHandle(mesh_shader_handle).c_str());
            } else if (no_task_shader_flag && has_task_shader) {
                skip |= LogError(
                    vuid.task_mesh_shader_08695, cb_state.Handle(), vuid.loc(),
                    "Mesh shader (%s) was created with VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT, but a task shader (%s) is bound.",
                    FormatHandle(mesh_shader_handle).c_str(), FormatHandle(task_shader_handle).c_str());
            }

            if (const vvl::ShaderObject* task_state = last_bound_state.GetShaderState(ShaderObjectStage::TASK)) {
                if (task_state->spirv && mesh_state->entrypoint) {
                    skip |= ValidateTaskPayload(*task_state->spirv, *mesh_state->entrypoint, vuid.loc());
                }
            }
        }
    }

    if (is_mesh_command) {
        const VkShaderEXT tesc_shader_handle = last_bound_state.GetShader(ShaderObjectStage::TESSELLATION_CONTROL);
        const VkShaderEXT tese_shader_handle = last_bound_state.GetShader(ShaderObjectStage::TESSELLATION_EVALUATION);
        const VkShaderEXT geom_shader_handle = last_bound_state.GetShader(ShaderObjectStage::GEOMETRY);
        const bool has_tesc_shader = tesc_shader_handle != VK_NULL_HANDLE;
        const bool has_tese_shader = tese_shader_handle != VK_NULL_HANDLE;
        const bool has_geom_shader = geom_shader_handle != VK_NULL_HANDLE;
        if (has_vertex_shader || has_tesc_shader || has_tese_shader || has_geom_shader) {
            std::stringstream msg;
            if (has_vertex_shader) msg << "Vertex shader: " << FormatHandle(vertex_shader_handle) << '\n';
            if (has_tese_shader) msg << "Tessellation Eval shader: " << FormatHandle(tese_shader_handle) << '\n';
            if (has_tesc_shader) msg << "Tessellation Control shader: " << FormatHandle(tesc_shader_handle) << '\n';
            if (has_geom_shader) msg << "Geometry shader: " << FormatHandle(geom_shader_handle) << '\n';
            skip |=
                LogError(vuid.bound_non_mesh_10680, cb_state.Handle(), vuid.loc(),
                         "Calling a mesh draw call, but the following stages are bound, but need to be bound as VK_NULL_HANDLE\n%s",
                         msg.str().c_str());
        }
    }
    return skip;
}
