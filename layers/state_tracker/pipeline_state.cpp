/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/pipeline_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/shader_module.h"
#include "enum_flag_bits.h"

static bool WrotePrimitiveShadingRate(VkShaderStageFlagBits stage_flag, const Instruction &entrypoint,
                                      const SHADER_MODULE_STATE *module_state) {
    bool primitive_rate_written = false;
    if (stage_flag == VK_SHADER_STAGE_VERTEX_BIT || stage_flag == VK_SHADER_STAGE_GEOMETRY_BIT ||
        stage_flag == VK_SHADER_STAGE_MESH_BIT_NV) {
        for (const Instruction *inst : module_state->GetBuiltinDecorationList()) {
            if (inst->GetBuiltIn() == spv::BuiltInPrimitiveShadingRateKHR) {
                primitive_rate_written = module_state->IsBuiltInWritten(inst, entrypoint);
            }
            if (primitive_rate_written) {
                break;
            }
        }
    }
    return primitive_rate_written;
}

PipelineStageState::PipelineStageState(const safe_VkPipelineShaderStageCreateInfo *stage,
                                       std::shared_ptr<const SHADER_MODULE_STATE> &module_state)
    : module_state(module_state),
      create_info(stage),
      stage_flag(stage->stage),
      entrypoint(module_state->FindEntrypoint(stage->pName, stage->stage)),
      writes_to_gl_layer(module_state->WritesToGlLayer()),
      has_input_attachment_capability(module_state->HasCapability(spv::CapabilityInputAttachment)) {
    if (entrypoint) {
        descriptor_variables = module_state->GetResourceInterfaceVariable(*entrypoint);
        if (descriptor_variables) {
            has_writable_descriptor = std::any_of(descriptor_variables->begin(), descriptor_variables->end(),
                                                  [](const auto &variable) { return variable.is_writable; });

            has_atomic_descriptor = std::any_of(descriptor_variables->begin(), descriptor_variables->end(),
                                                [](const auto &variable) { return variable.is_atomic_operation; });
        }
        wrote_primitive_shading_rate = WrotePrimitiveShadingRate(stage_flag, *entrypoint, module_state.get());
    }
}

// static
PIPELINE_STATE::StageStateVec PIPELINE_STATE::GetStageStates(const ValidationStateTracker &state_data,
                                                             const PIPELINE_STATE &pipe_state,
                                                             CreateShaderModuleStates *csm_states) {
    PIPELINE_STATE::StageStateVec stage_states;
    // shader stages need to be recorded in pipeline order
    const auto stages = pipe_state.GetShaderStages();

    // stages such as VK_SHADER_STAGE_ALL are find as this code is only looking for exact matches, not bool logic
    for (const auto &stage : AllVkShaderStageFlags) {
        bool stage_found = false;
        for (const auto &shader_stage : stages) {
            if (shader_stage.stage == stage) {
                auto module = state_data.Get<SHADER_MODULE_STATE>(shader_stage.module);
                if (!module) {
                    // See if the module is referenced in a library sub state
                    module = pipe_state.GetSubStateShader(shader_stage.stage);
                }

                if (!module) {
                    // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
                    // module is part of a library and the state must be created
                    const auto shader_ci = LvlFindInChain<VkShaderModuleCreateInfo>(shader_stage.pNext);
                    const uint32_t unique_shader_id = (csm_states) ? (*csm_states)[stage].unique_shader_id : 0;
                    if (shader_ci) {
                        module = state_data.CreateShaderModuleState(*shader_ci, unique_shader_id);
                    } else {
                        // shader_module_identifier could legally provide a null module handle
                        VkShaderModuleCreateInfo dummy_module_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
                        dummy_module_ci.pCode = &unique_shader_id;  // Ensure tripping invalid spirv
                        module = state_data.CreateShaderModuleState(dummy_module_ci, unique_shader_id);
                    }
                }

                stage_states.emplace_back(&shader_stage, module);
                stage_found = true;
            }
        }
        if (!stage_found) {
            // Check if stage has been supplied by a library
            switch (stage) {
                case VK_SHADER_STAGE_VERTEX_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->vertex_shader) {
                        stage_states.emplace_back(pipe_state.pre_raster_state->vertex_shader_ci,
                                                  pipe_state.pre_raster_state->vertex_shader);
                    }
                    break;
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->tessc_shader) {
                        stage_states.emplace_back(pipe_state.pre_raster_state->tessc_shader_ci,
                                                  pipe_state.pre_raster_state->tessc_shader);
                    }
                    break;
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->tesse_shader) {
                        stage_states.emplace_back(pipe_state.pre_raster_state->tesse_shader_ci,
                                                  pipe_state.pre_raster_state->tesse_shader);
                    }
                    break;
                case VK_SHADER_STAGE_GEOMETRY_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->geometry_shader) {
                        stage_states.emplace_back(pipe_state.pre_raster_state->geometry_shader_ci,
                                                  pipe_state.pre_raster_state->geometry_shader);
                    }
                    break;
                case VK_SHADER_STAGE_TASK_BIT_EXT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->task_shader) {
                        stage_states.emplace_back(pipe_state.pre_raster_state->task_shader_ci,
                                                  pipe_state.pre_raster_state->task_shader);
                    }
                    break;
                case VK_SHADER_STAGE_MESH_BIT_EXT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->mesh_shader) {
                        stage_states.emplace_back(pipe_state.pre_raster_state->mesh_shader_ci,
                                                  pipe_state.pre_raster_state->mesh_shader);
                    }
                    break;
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    if (pipe_state.fragment_shader_state && pipe_state.fragment_shader_state->fragment_shader) {
                        stage_states.emplace_back(pipe_state.fragment_shader_state->fragment_shader_ci.get(),
                                                  pipe_state.fragment_shader_state->fragment_shader);
                    }
                    break;
                default:
                    // no-op
                    break;
            }
        }
    }
    return stage_states;
}

// static
PIPELINE_STATE::ActiveSlotMap PIPELINE_STATE::GetActiveSlots(const StageStateVec &stage_states) {
    PIPELINE_STATE::ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        if (!stage.entrypoint || !stage.descriptor_variables) {
            continue;
        }
        // Capture descriptor uses for the pipeline
        for (const auto &variable : *stage.descriptor_variables) {
            // While validating shaders capture which slots are used by the pipeline
            auto &entry = active_slots[variable.decorations.set][variable.decorations.binding];
            entry.is_writable |= variable.is_writable;

            auto &reqs = entry.reqs;
            reqs |= stage.module_state->DescriptorTypeToReqs(variable.type_id);
            if (variable.is_atomic_operation) reqs |= DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION;
            if (variable.is_sampler_sampled) reqs |= DESCRIPTOR_REQ_SAMPLER_SAMPLED;
            if (variable.is_sampler_implicitLod_dref_proj) reqs |= DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ;
            if (variable.is_sampler_bias_offset) reqs |= DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET;
            if (variable.is_read_without_format) reqs |= DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT;
            if (variable.is_write_without_format) reqs |= DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT;
            if (variable.is_dref_operation) reqs |= DESCRIPTOR_REQ_IMAGE_DREF;

            if (!variable.samplers_used_by_image.empty()) {
                if (variable.samplers_used_by_image.size() > entry.samplers_used_by_image.size()) {
                    entry.samplers_used_by_image.resize(variable.samplers_used_by_image.size());
                }
                uint32_t image_index = 0;
                for (const auto &samplers : variable.samplers_used_by_image) {
                    for (const auto &sampler : samplers) {
                        entry.samplers_used_by_image[image_index].emplace(sampler);
                    }
                    ++image_index;
                }
            }
            entry.write_without_formats_component_count_list = variable.write_without_formats_component_count_list;
            entry.image_sampled_type_width = variable.image_sampled_type_width;
        }
    }
    return active_slots;
}

static uint32_t GetMaxActiveSlot(const PIPELINE_STATE::ActiveSlotMap &active_slots) {
    uint32_t max_active_slot = 0;
    for (const auto &entry : active_slots) {
        max_active_slot = std::max(max_active_slot, entry.first);
    }
    return max_active_slot;
}

static uint32_t GetActiveShaders(const PIPELINE_STATE::StageStateVec &stages) {
    uint32_t result = 0;
    for (const auto &stage : stages) {
        result |= stage.stage_flag;
    }
    return result;
}

static bool UsesPipelineRobustness(const void *pNext, const PIPELINE_STATE::StageStateVec &stages) {
    bool result = false;
    const auto robustness_info = LvlFindInChain<VkPipelineRobustnessCreateInfoEXT>(pNext);
    if (!robustness_info) {
        return false;
    }
    result |= (robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT) ||
              (robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT);
    result |= (robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT) ||
              (robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT);
    if (!result) {
        for (const auto &stage : stages) {
            const auto stage_robustness_info = LvlFindInChain<VkPipelineRobustnessCreateInfoEXT>(stage.create_info->pNext);
            if (stage_robustness_info) {
                result |=
                    (stage_robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT) ||
                    (stage_robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT);
                result |=
                    (stage_robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT) ||
                    (stage_robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT);
            }
        }
    }
    return result;
}

static bool UsesShaderModuleId(const PIPELINE_STATE::StageStateVec &stages) {
    bool result = false;
    for (const auto &stage : stages) {
        const auto module_id_info = LvlFindInChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(stage.create_info->pNext);
        if (module_id_info) result |= (module_id_info->identifierSize > 0);
    }
    return result;
}

static vvl::unordered_set<uint32_t> GetFSOutputLocations(const PIPELINE_STATE::StageStateVec &stage_states) {
    vvl::unordered_set<uint32_t> result;
    for (const auto &stage : stage_states) {
        if (!stage.entrypoint) {
            continue;
        }
        if (stage.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT) {
            result = stage.module_state->CollectWritableOutputLocationinFS(*(stage.entrypoint));
            break;
        }
    }
    return result;
}

static VkPrimitiveTopology GetTopologyAtRasterizer(const PIPELINE_STATE &pipeline) {
    auto result = (pipeline.vertex_input_state && pipeline.vertex_input_state->input_assembly_state)
                      ? pipeline.vertex_input_state->input_assembly_state->topology
                      : VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    for (const auto &stage : pipeline.stage_state) {
        if (!stage.entrypoint) {
            continue;
        }
        auto stage_topo = stage.module_state->GetTopology(*(stage.entrypoint));
        if (stage_topo) {
            result = *stage_topo;
        }
    }
    return result;
}

// static
std::shared_ptr<VertexInputState> PIPELINE_STATE::CreateVertexInputState(const PIPELINE_STATE &p,
                                                                         const ValidationStateTracker &state,
                                                                         const safe_VkGraphicsPipelineCreateInfo &create_info) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) {  // Vertex input graphics library
        return std::make_shared<VertexInputState>(p, create_info);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    } else {
        if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
            return std::make_shared<VertexInputState>(p, create_info);
        }
    }

    // We shouldn't get here...
    return {};
}

// static
std::shared_ptr<PreRasterState> PIPELINE_STATE::CreatePreRasterState(const PIPELINE_STATE &p, const ValidationStateTracker &state,
                                                                     const safe_VkGraphicsPipelineCreateInfo &create_info,
                                                                     const std::shared_ptr<const RENDER_PASS_STATE> &rp) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {  // Pre-raster graphics library
        return std::make_shared<PreRasterState>(p, state, create_info, rp);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    } else {
        if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
            return std::make_shared<PreRasterState>(p, state, create_info, rp);
        }
    }

    // We shouldn't get here...
    return {};
}

// static
std::shared_ptr<FragmentShaderState> PIPELINE_STATE::CreateFragmentShaderState(
    const PIPELINE_STATE &p, const ValidationStateTracker &state, const VkGraphicsPipelineCreateInfo &create_info,
    const safe_VkGraphicsPipelineCreateInfo &safe_create_info, const std::shared_ptr<const RENDER_PASS_STATE> &rp) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {  // Fragment shader graphics library
        return std::make_shared<FragmentShaderState>(p, state, create_info, rp);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    } else {
        if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
            return std::make_shared<FragmentShaderState>(p, state, safe_create_info, rp);
        }
    }

    // We shouldn't get here...
    return {};
}

// static
// Pointers that should be ignored have been set to null in safe_create_info, but if this is a graphics library we need the "raw"
// create_info.
std::shared_ptr<FragmentOutputState> PIPELINE_STATE::CreateFragmentOutputState(
    const PIPELINE_STATE &p, const ValidationStateTracker &state, const VkGraphicsPipelineCreateInfo &create_info,
    const safe_VkGraphicsPipelineCreateInfo &safe_create_info, const std::shared_ptr<const RENDER_PASS_STATE> &rp) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {  // Fragment output graphics library
        return std::make_shared<FragmentOutputState>(p, create_info, rp);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    } else {
        if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
            return std::make_shared<FragmentOutputState>(p, safe_create_info, rp);
        }
    }

    // We shouldn't get here...
    return {};
}

template <typename Substate>
void AppendDynamicStateFromSubstate(const Substate &substate, std::vector<VkDynamicState> &dyn_states,
                                    VkPipelineDynamicStateCreateFlags &flags) {
    if (substate) {
        const auto *dyn_state = substate->parent.DynamicState();
        if (dyn_state) {
            flags |= dyn_state->flags;
            for (uint32_t i = 0; i < dyn_state->dynamicStateCount; ++i) {
                const auto itr = std::find(dyn_states.cbegin(), dyn_states.cend(), dyn_state->pDynamicStates[i]);
                if (itr == dyn_states.cend()) {
                    dyn_states.emplace_back(dyn_state->pDynamicStates[i]);
                }
            }
        }
    }
}

std::vector<std::shared_ptr<const PIPELINE_LAYOUT_STATE>> PIPELINE_STATE::PipelineLayoutStateUnion() const {
    std::vector<std::shared_ptr<const PIPELINE_LAYOUT_STATE>> ret;
    ret.reserve(2);
    // Only need to check pre-raster _or_ fragment shader layout; if either one is not merged_graphics_layout, then
    // merged_graphics_layout is a union
    if (pre_raster_state) {
        if (pre_raster_state->pipeline_layout != fragment_shader_state->pipeline_layout) {
            return {pre_raster_state->pipeline_layout, fragment_shader_state->pipeline_layout};
        } else {
            return {pre_raster_state->pipeline_layout};
        }
    }
    return {merged_graphics_layout};
}

template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkGraphicsPipelineCreateInfo>() const {
    assert(create_info.graphics.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    return create_info.graphics.basePipelineHandle;
}
template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkComputePipelineCreateInfo>() const {
    assert(create_info.compute.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
    return create_info.compute.basePipelineHandle;
}
template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkRayTracingPipelineCreateInfoKHR>() const {
    assert(create_info.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR);
    return create_info.raytracing.basePipelineHandle;
}
template <>
VkPipeline PIPELINE_STATE::BasePipeline<VkRayTracingPipelineCreateInfoNV>() const {
    assert(create_info.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV);
    return create_info.raytracing.basePipelineHandle;
}

template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkGraphicsPipelineCreateInfo>() const {
    assert(create_info.graphics.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    return create_info.graphics.basePipelineIndex;
}
template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkComputePipelineCreateInfo>() const {
    assert(create_info.compute.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
    return create_info.compute.basePipelineIndex;
}
template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkRayTracingPipelineCreateInfoKHR>() const {
    assert(create_info.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR);
    return create_info.raytracing.basePipelineIndex;
}
template <>
int32_t PIPELINE_STATE::BasePipelineIndex<VkRayTracingPipelineCreateInfoNV>() const {
    assert(create_info.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV);
    return create_info.raytracing.basePipelineIndex;
}

template <>
VkShaderModule PIPELINE_STATE::PIPELINE_STATE::GetShaderModuleByCIIndex<VkGraphicsPipelineCreateInfo>(uint32_t i) {
    assert(create_info.graphics.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    return create_info.graphics.pStages[i].module;
}
template <>
VkShaderModule PIPELINE_STATE::GetShaderModuleByCIIndex<VkComputePipelineCreateInfo>(uint32_t) {
    assert(create_info.compute.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
    return create_info.compute.stage.module;
}
template <>
VkShaderModule PIPELINE_STATE::GetShaderModuleByCIIndex<VkRayTracingPipelineCreateInfoKHR>(uint32_t i) {
    assert(create_info.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR);
    return create_info.raytracing.pStages[i].module;
}
template <>
VkShaderModule PIPELINE_STATE::GetShaderModuleByCIIndex<VkRayTracingPipelineCreateInfoNV>(uint32_t i) {
    assert(create_info.raytracing.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV);
    return create_info.raytracing.pStages[i].module;
}

// TODO (ncesario) this needs to be automated. As a first step, need to leverage SubState::ValidShaderStages()
std::shared_ptr<const SHADER_MODULE_STATE> PIPELINE_STATE::GetSubStateShader(VkShaderStageFlagBits state) const {
    switch (state) {
        case VK_SHADER_STAGE_VERTEX_BIT: {
            const auto sub_state =
                PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->vertex_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
            const auto sub_state =
                PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->tessc_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
            const auto sub_state =
                PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->tesse_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_GEOMETRY_BIT: {
            const auto sub_state =
                PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->geometry_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_TASK_BIT_EXT: {
            const auto sub_state =
                PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->task_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_MESH_BIT_EXT: {
            const auto sub_state =
                PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->mesh_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_FRAGMENT_BIT: {
            const auto sub_state = PIPELINE_STATE::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(*this);
            return (sub_state) ? sub_state->fragment_shader : nullptr;
            break;
        };
        default:
            return {};
    }
}

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                               uint32_t create_index, std::shared_ptr<const RENDER_PASS_STATE> &&rpstate,
                               std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      rp_state(rpstate),
      create_info(pCreateInfo, rpstate),
      create_index(create_index),
      graphics_lib_type(GetGraphicsLibType(create_info.graphics)),
      vertex_input_state(CreateVertexInputState(*this, *state_data, create_info.graphics)),
      pre_raster_state(CreatePreRasterState(*this, *state_data, create_info.graphics, rpstate)),
      fragment_shader_state(CreateFragmentShaderState(*this, *state_data, *pCreateInfo, create_info.graphics, rpstate)),
      fragment_output_state(CreateFragmentOutputState(*this, *state_data, *pCreateInfo, create_info.graphics, rpstate)),
      rendering_create_info(LvlFindInChain<VkPipelineRenderingCreateInfo>(PNext())),
      stage_state(GetStageStates(*state_data, *this, csm_states)),
      fragmentShader_writable_output_location_list(GetFSOutputLocations(stage_state)),
      active_slots(GetActiveSlots(stage_state)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      active_shaders(GetActiveShaders(stage_state)),
      topology_at_rasterizer(GetTopologyAtRasterizer(*this)),
      uses_shader_module_id(UsesShaderModuleId(stage_state)),
      descriptor_buffer_mode((create_info.graphics.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), stage_state)),
      csm_states(csm_states) {
    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(PNext());
    if (link_info) {
        // accumulate dynamic state
        // TODO is this correct?
        auto *dyn_state_ci = const_cast<safe_VkPipelineDynamicStateCreateInfo *>(create_info.graphics.pDynamicState);
        std::vector<VkDynamicState> dyn_states;
        VkPipelineDynamicStateCreateFlags dyn_flags = 0;
        if (create_info.graphics.pDynamicState) {
            std::copy(dyn_state_ci->pDynamicStates, dyn_state_ci->pDynamicStates + dyn_state_ci->dynamicStateCount,
                      std::back_inserter(dyn_states));
            dyn_flags = dyn_state_ci->flags;
        }
        AppendDynamicStateFromSubstate(vertex_input_state, dyn_states, dyn_flags);
        AppendDynamicStateFromSubstate(pre_raster_state, dyn_states, dyn_flags);
        AppendDynamicStateFromSubstate(fragment_shader_state, dyn_states, dyn_flags);
        AppendDynamicStateFromSubstate(fragment_output_state, dyn_states, dyn_flags);
        if (dyn_states.size() > 0) {
            // We have dynamic state
            if (!dyn_state_ci || (dyn_state_ci->dynamicStateCount < dyn_states.size())) {
                // There is dynamic state defined in libraries that the is not included in this pipeline's create info
                if (!dyn_state_ci) {
                    // *All* dynamic state defined is coming from graphics libraries
                    // NOTE: heap allocation cleaned up in ~safe_VkGraphicsPipelineCreateInfo
                    dyn_state_ci = new safe_VkPipelineDynamicStateCreateInfo;
                    const_cast<safe_VkGraphicsPipelineCreateInfo *>(&create_info.graphics)->pDynamicState = dyn_state_ci;
                }
                dyn_state_ci->flags = dyn_flags;
                dyn_state_ci->dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
                // NOTE: heap allocation cleaned up in ~safe_VkPipelineDynamicStateCreateInfo
                dyn_state_ci->pDynamicStates = new VkDynamicState[dyn_states.size()];
                std::copy(&dyn_states.front(), &dyn_states.front() + dyn_states.size(),
                          const_cast<VkDynamicState *>(dyn_state_ci->pDynamicStates));
            }
        }

        const auto &exe_layout_state = state_data->Get<PIPELINE_LAYOUT_STATE>(create_info.graphics.layout);
        const auto *exe_layout = exe_layout_state.get();
        const auto *pre_raster_layout =
            (pre_raster_state && pre_raster_state->pipeline_layout) ? pre_raster_state->pipeline_layout.get() : nullptr;
        const auto *fragment_shader_layout = (fragment_shader_state && fragment_shader_state->pipeline_layout)
                                                 ? fragment_shader_state->pipeline_layout.get()
                                                 : nullptr;
        std::array<decltype(exe_layout), 3> layouts;
        layouts[0] = exe_layout;
        layouts[1] = fragment_shader_layout;
        layouts[2] = pre_raster_layout;
        merged_graphics_layout = std::make_shared<PIPELINE_LAYOUT_STATE>(layouts);

        // TODO Could store the graphics_lib_type in the sub-state rather than searching for it again here.
        //      Or, could store a pointer back to the owning PIPELINE_STATE.
        for (uint32_t i = 0; i < link_info->libraryCount; ++i) {
            const auto &state = state_data->Get<PIPELINE_STATE>(link_info->pLibraries[i]);
            if (state) {
                graphics_lib_type |= state->graphics_lib_type;
            }
        }
    }
}

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo,
                               uint32_t create_index, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                               CreateShaderModuleStates *csm_states)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      create_index(create_index),
      stage_state(GetStageStates(*state_data, *this, csm_states)),
      active_slots(GetActiveSlots(stage_state)),
      active_shaders(GetActiveShaders(stage_state)),
      uses_shader_module_id(UsesShaderModuleId(stage_state)),
      descriptor_buffer_mode((create_info.compute.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), stage_state)),
      csm_states(csm_states),
      merged_graphics_layout(layout) {
    assert(active_shaders == VK_SHADER_STAGE_COMPUTE_BIT);
}

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
                               uint32_t create_index, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                               CreateShaderModuleStates *csm_states)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      create_index(create_index),
      stage_state(GetStageStates(*state_data, *this, csm_states)),
      active_slots(GetActiveSlots(stage_state)),
      active_shaders(GetActiveShaders(stage_state)),
      uses_shader_module_id(UsesShaderModuleId(stage_state)),
      descriptor_buffer_mode((create_info.raytracing.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), stage_state)),
      csm_states(csm_states),
      merged_graphics_layout(std::move(layout)) {
    assert(0 == (active_shaders &
                 ~(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                   VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)));
}

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo,
                               uint32_t create_index, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout,
                               CreateShaderModuleStates *csm_states)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      create_index(create_index),
      stage_state(GetStageStates(*state_data, *this, csm_states)),
      active_slots(GetActiveSlots(stage_state)),
      active_shaders(GetActiveShaders(stage_state)),
      uses_shader_module_id(UsesShaderModuleId(stage_state)),
      descriptor_buffer_mode((create_info.graphics.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), stage_state)),
      csm_states(csm_states),
      merged_graphics_layout(std::move(layout)) {
    assert(0 == (active_shaders &
                 ~(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                   VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)));
}

void LAST_BOUND_STATE::UnbindAndResetPushDescriptorSet(std::shared_ptr<cvdescriptorset::DescriptorSet> &&ds) {
    if (push_descriptor_set) {
        for (auto &ps : per_set) {
            if (ps.bound_descriptor_set == push_descriptor_set) {
                cb_state.RemoveChild(ps.bound_descriptor_set);
                ps.bound_descriptor_set.reset();
            }
        }
    }
    cb_state.AddChild(ds);
    push_descriptor_set = std::move(ds);
}

void LAST_BOUND_STATE::Reset() {
    pipeline_state = nullptr;
    pipeline_layout = VK_NULL_HANDLE;
    if (push_descriptor_set) {
        cb_state.RemoveChild(push_descriptor_set);
        push_descriptor_set->Destroy();
    }
    push_descriptor_set.reset();
    per_set.clear();
}
