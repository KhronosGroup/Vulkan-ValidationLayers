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
#include "generated/enum_flag_bits.h"

PipelineStageState::PipelineStageState(const safe_VkPipelineShaderStageCreateInfo *create_info,
                                       std::shared_ptr<const SHADER_MODULE_STATE> &module_state,
                                       std::shared_ptr<const EntryPoint> &entrypoint)
    : module_state(module_state), create_info(create_info), entrypoint(entrypoint) {}

// static
PIPELINE_STATE::StageStateVec PIPELINE_STATE::GetStageStates(const ValidationStateTracker &state_data,
                                                             const PIPELINE_STATE &pipe_state,
                                                             CreateShaderModuleStates *csm_states) {
    PIPELINE_STATE::StageStateVec stage_states;

    // stages such as VK_SHADER_STAGE_ALL are find as this code is only looking for exact matches, not bool logic
    for (const auto &stage : AllVkShaderStageFlags) {
        bool stage_found = false;
        // shader stages need to be recorded in pipeline order
        for (const auto &stage_ci : pipe_state.shader_stages_ci) {
            if (stage_ci.stage == stage) {
                auto module = state_data.Get<SHADER_MODULE_STATE>(stage_ci.module);
                if (!module) {
                    // See if the module is referenced in a library sub state
                    module = pipe_state.GetSubStateShader(stage_ci.stage);
                }

                if (!module) {
                    // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
                    // module is part of a library and the state must be created
                    const auto shader_ci = LvlFindInChain<VkShaderModuleCreateInfo>(stage_ci.pNext);
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

                auto entrypoint = module->FindEntrypoint(stage_ci.pName, stage_ci.stage);
                stage_states.emplace_back(&stage_ci, module, entrypoint);
                stage_found = true;
            }
        }
        if (!stage_found) {
            // Check if stage has been supplied by a library
            std::shared_ptr<const SHADER_MODULE_STATE> module_state = nullptr;
            const safe_VkPipelineShaderStageCreateInfo *stage_ci = nullptr;
            switch (stage) {
                case VK_SHADER_STAGE_VERTEX_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->vertex_shader) {
                        module_state = pipe_state.pre_raster_state->vertex_shader;
                        stage_ci = pipe_state.pre_raster_state->vertex_shader_ci;
                    }
                    break;
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->tessc_shader) {
                        module_state = pipe_state.pre_raster_state->tessc_shader;
                        stage_ci = pipe_state.pre_raster_state->tessc_shader_ci;
                    }
                    break;
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->tesse_shader) {
                        module_state = pipe_state.pre_raster_state->tesse_shader;
                        stage_ci = pipe_state.pre_raster_state->tesse_shader_ci;
                    }
                    break;
                case VK_SHADER_STAGE_GEOMETRY_BIT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->geometry_shader) {
                        module_state = pipe_state.pre_raster_state->geometry_shader;
                        stage_ci = pipe_state.pre_raster_state->geometry_shader_ci;
                    }
                    break;
                case VK_SHADER_STAGE_TASK_BIT_EXT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->task_shader) {
                        module_state = pipe_state.pre_raster_state->task_shader;
                        stage_ci = pipe_state.pre_raster_state->task_shader_ci;
                    }
                    break;
                case VK_SHADER_STAGE_MESH_BIT_EXT:
                    if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->mesh_shader) {
                        module_state = pipe_state.pre_raster_state->mesh_shader;
                        stage_ci = pipe_state.pre_raster_state->mesh_shader_ci;
                    }
                    break;
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    if (pipe_state.fragment_shader_state && pipe_state.fragment_shader_state->fragment_shader) {
                        module_state = pipe_state.fragment_shader_state->fragment_shader;
                        stage_ci = pipe_state.fragment_shader_state->fragment_shader_ci.get();
                    }
                    break;
                default:
                    // no-op
                    break;
            }
            if (!stage_ci) {
                continue;
            }
            auto entrypoint = module_state->FindEntrypoint(stage_ci->pName, stage_ci->stage);
            stage_states.emplace_back(stage_ci, module_state, entrypoint);
        }
    }
    return stage_states;
}

// static
PIPELINE_STATE::ActiveSlotMap PIPELINE_STATE::GetActiveSlots(const StageStateVec &stage_states) {
    PIPELINE_STATE::ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        if (!stage.entrypoint) {
            continue;
        }
        // Capture descriptor uses for the pipeline
        for (const auto &variable : stage.entrypoint->resource_interface_variables) {
            // While validating shaders capture which slots are used by the pipeline
            auto &entry = active_slots[variable.decorations.set][variable.decorations.binding];
            entry.variable = &variable;

            auto &reqs = entry.reqs;
            if (variable.is_atomic_operation) reqs |= DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION;
            if (variable.is_sampler_sampled) reqs |= DESCRIPTOR_REQ_SAMPLER_SAMPLED;
            if (variable.is_sampler_implicitLod_dref_proj) reqs |= DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ;
            if (variable.is_sampler_bias_offset) reqs |= DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET;
            if (variable.is_read_without_format) reqs |= DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT;
            if (variable.is_write_without_format) reqs |= DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT;
            if (variable.is_dref) reqs |= DESCRIPTOR_REQ_IMAGE_DREF;

            if (variable.image_format_type == NumericTypeFloat) reqs |= DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
            if (variable.image_format_type == NumericTypeSint) reqs |= DESCRIPTOR_REQ_COMPONENT_TYPE_SINT;
            if (variable.image_format_type == NumericTypeUint) reqs |= DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;

            if (variable.image_dim == spv::Dim1D) {
                reqs |= (variable.is_image_array) ? DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_1D;
            }

            if (variable.image_dim == spv::Dim2D) {
                reqs |= (variable.is_multisampled) ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                reqs |= (variable.is_image_array) ? DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_2D;
            }

            if (variable.image_dim == spv::Dim3D) reqs |= DESCRIPTOR_REQ_VIEW_TYPE_3D;

            if (variable.image_dim == spv::DimCube) {
                reqs |= (variable.is_image_array) ? DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_CUBE;
            }
            if (variable.image_dim == spv::DimSubpassData) {
                reqs |= (variable.is_multisampled) ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
            }
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

static uint32_t GetCreateInfoShaders(const PIPELINE_STATE &pipe_state) {
    uint32_t result = 0;
    for (const auto &stage_ci : pipe_state.shader_stages_ci) {
        result |= stage_ci.stage;
    }
    return result;
}

static uint32_t GetLinkingShaders(const VkPipelineLibraryCreateInfoKHR *link_info, const ValidationStateTracker &state_data) {
    uint32_t result = 0;
    if (link_info) {
        for (uint32_t i = 0; i < link_info->libraryCount; ++i) {
            const auto &state = state_data.Get<PIPELINE_STATE>(link_info->pLibraries[i]);
            if (state) {
                result |= state->active_shaders;
            }
        }
    }
    return result;
}

static CBDynamicFlags GetGraphicsDynamicState(PIPELINE_STATE &pipe_state) {
    CBDynamicFlags flags = 0;

    // "Dynamic state values set via pDynamicState must be ignored if the state they correspond to is not otherwise statically set
    // by one of the state subsets used to create the pipeline."
    //
    // we only care here if the pipeline was created with the subset, not linked
    const bool has_vertex_input_state = pipe_state.OwnsSubState(pipe_state.vertex_input_state);
    const bool has_pre_raster_state = pipe_state.OwnsSubState(pipe_state.pre_raster_state);
    const bool has_fragment_shader_state = pipe_state.OwnsSubState(pipe_state.fragment_shader_state);
    const bool has_fragment_output_state = pipe_state.OwnsSubState(pipe_state.fragment_output_state);

    const auto *dynamic_state_ci = pipe_state.DynamicState();
    if (dynamic_state_ci) {
        for (uint32_t i = 0; i < dynamic_state_ci->dynamicStateCount; i++) {
            const VkDynamicState vk_dynamic_state = dynamic_state_ci->pDynamicStates[i];
            // Check if should ignore or not before converting and adding
            switch (vk_dynamic_state) {
                // VkPipelineVertexInputStateCreateInfo
                case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
                case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
                // VkPipelineInputAssemblyStateCreateInfo
                case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
                case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE: {
                    if (has_vertex_input_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineViewportStateCreateInfo
                case VK_DYNAMIC_STATE_VIEWPORT:
                case VK_DYNAMIC_STATE_SCISSOR:
                case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
                case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
                case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
                case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
                case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
                case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
                case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
                // VkPipelineRasterizationStateCreateInfo
                case VK_DYNAMIC_STATE_LINE_WIDTH:
                case VK_DYNAMIC_STATE_DEPTH_BIAS:
                case VK_DYNAMIC_STATE_CULL_MODE:
                case VK_DYNAMIC_STATE_FRONT_FACE:
                case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
                case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
                case VK_DYNAMIC_STATE_LINE_STIPPLE_EXT:
                case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
                case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
                case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
                case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
                case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
                case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
                // VkPipelineTessellationStateCreateInfo
                case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
                case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
                // VkPipelineDiscardRectangleStateCreateInfoEXT
                case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
                case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT: {
                    if (has_pre_raster_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineFragmentShadingRateStateCreateInfoKHR
                case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR: {
                    if (has_pre_raster_state || has_fragment_shader_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineDepthStencilStateCreateInfo
                case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
                case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
                case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
                case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
                case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
                case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
                case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
                case VK_DYNAMIC_STATE_STENCIL_OP:
                // VkPipelineRepresentativeFragmentTestStateCreateInfoNV
                case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV: {
                    if (has_fragment_shader_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                /// VkPipelineColorBlendStateCreateInfo
                case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
                case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
                case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
                case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
                case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT: {
                    if (has_fragment_output_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineMultisampleStateCreateInfo
                case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
                case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
                case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
                case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
                case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV: {
                    if (has_fragment_shader_state || has_fragment_output_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkRenderPass and subpass parameter
                case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT: {
                    if (has_pre_raster_state || has_fragment_shader_state || has_fragment_output_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // not valid and shouldn't see
                case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
                case VK_DYNAMIC_STATE_MAX_ENUM:
                    assert(false);
                    break;
            }
        }
    }

    // apply linked library's dynamic state
    if (!has_vertex_input_state && pipe_state.vertex_input_state) {
        flags |= pipe_state.vertex_input_state->parent.dynamic_state;
    }
    if (!has_pre_raster_state && pipe_state.pre_raster_state) {
        flags |= pipe_state.pre_raster_state->parent.dynamic_state;
    }
    if (!has_fragment_shader_state && pipe_state.fragment_shader_state) {
        flags |= pipe_state.fragment_shader_state->parent.dynamic_state;
    }
    if (!has_fragment_output_state && pipe_state.fragment_output_state) {
        flags |= pipe_state.fragment_output_state->parent.dynamic_state;
    }
    return flags;
}

static bool UsesPipelineRobustness(const void *pNext, const PIPELINE_STATE &pipe_state) {
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
        for (const auto &stage_ci : pipe_state.shader_stages_ci) {
            const auto stage_robustness_info = LvlFindInChain<VkPipelineRobustnessCreateInfoEXT>(stage_ci.pNext);
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

static bool IgnoreColorAttachments(const ValidationStateTracker *state_data, PIPELINE_STATE &pipe_state) {
    bool ignore = false;
    // If the libraries used to create this pipeline are ignoring color attachments, this pipeline should as well
    if (pipe_state.library_create_info) {
        for (uint32_t i = 0; i < pipe_state.library_create_info->libraryCount; i++) {
            const auto lib = state_data->Get<PIPELINE_STATE>(pipe_state.library_create_info->pLibraries[i]);
            if (lib->ignore_color_attachments) return true;
        }
    }
    // According to the spec, pAttachments is to be ignored if the pipeline is created with
    // VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT
    // and VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT dynamic states set
    if (pipe_state.ColorBlendState() && pipe_state.DynamicState()) {
        ignore = (pipe_state.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
                  pipe_state.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
                  pipe_state.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) &&
                  pipe_state.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT));
    }
    return ignore;
}

static bool UsesShaderModuleId(const PIPELINE_STATE &pipe_state) {
    for (const auto &stage_ci : pipe_state.shader_stages_ci) {
        const auto module_id_info = LvlFindInChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(stage_ci.pNext);
        if (module_id_info && (module_id_info->identifierSize > 0)) {
            return true;
        }
    }
    return false;
}

static vvl::unordered_set<uint32_t> GetFSOutputLocations(const PIPELINE_STATE::StageStateVec &stage_states) {
    vvl::unordered_set<uint32_t> result;
    for (const auto &stage_state : stage_states) {
        if (!stage_state.entrypoint) {
            continue;
        }
        if (stage_state.create_info->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            for (const auto *variable : stage_state.entrypoint->user_defined_interface_variables) {
                if ((variable->storage_class != spv::StorageClassOutput) || variable->interface_slots.empty()) {
                    continue;  // not an output interface
                }
                // It is not allowed to have Block Fragment or 64-bit vectors output in Frag shader
                // This means all Locations in slots will be the same
                result.insert(variable->interface_slots[0].Location());
            }
            break;  // found
        }
    }
    return result;
}

static VkPrimitiveTopology GetTopologyAtRasterizer(const PIPELINE_STATE &pipeline) {
    auto result = (pipeline.vertex_input_state && pipeline.vertex_input_state->input_assembly_state)
                      ? pipeline.vertex_input_state->input_assembly_state->topology
                      : VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    for (const auto &stage : pipeline.stage_states) {
        if (!stage.entrypoint) {
            continue;
        }
        auto stage_topo = stage.module_state->GetTopology(*stage.entrypoint);
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

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT>(state, *p.library_create_info);
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

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(state, *p.library_create_info);
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

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(state, *p.library_create_info);
        if (ss && EnablesRasterizationStates(p.pre_raster_state)) {
            return ss;
        }
    } else {
        if ((lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) &&  // Not a graphics library
            EnablesRasterizationStates(p.pre_raster_state)) {
            // No fragment shader _should_ imply no fragment shader state, however, for historical (GL) reasons, a pipeline _can_
            // be created with a VS but no FS and still have valid fragment shader state.
            // See https://gitlab.khronos.org/vulkan/vulkan/-/issues/3178 for more details.
            return std::make_shared<FragmentShaderState>(p, state, safe_create_info, rp);
        }
    }

    // The conditions for containing FS state were not met, so return null
    return {};
}

// static
// Pointers that should be ignored have been set to null in safe_create_info, but if this is a graphics library we need the "raw"
// create_info.
std::shared_ptr<FragmentOutputState> PIPELINE_STATE::CreateFragmentOutputState(
    const PIPELINE_STATE &p, const ValidationStateTracker &state, const VkGraphicsPipelineCreateInfo &create_info,
    const safe_VkGraphicsPipelineCreateInfo &safe_create_info, const std::shared_ptr<const RENDER_PASS_STATE> &rp) {
    // If this pipeline is being created a non-executable (i.e., does not contain complete state) pipeline with FO state, then
    // unconditionally set this pipeline's FO state.
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {  // Fragment output graphics library
        return std::make_shared<FragmentOutputState>(p, create_info, rp);
    }

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT>(state, *p.library_create_info);
        // If this pipeline is linking in a library that contains FO state, check to see if the FO state is valid before creating it
        // for this pipeline
        if (ss && EnablesRasterizationStates(p.pre_raster_state)) {
            return ss;
        }
    } else {
        // This is a complete pipeline that does not link to any graphics libraries. Check its create info to see if it has valid FO
        // state
        if ((lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) &&  // Not a graphics library
            EnablesRasterizationStates(p.pre_raster_state)) {
            return std::make_shared<FragmentOutputState>(p, safe_create_info, rp);
        }
    }

    // The conditions for containing FO state were not met, so return null
    return {};
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
      create_info(*pCreateInfo, rpstate, state_data),
      create_index(create_index),
      rendering_create_info(LvlFindInChain<VkPipelineRenderingCreateInfo>(PNext())),
      library_create_info(LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(PNext())),
      graphics_lib_type(GetGraphicsLibType(create_info.graphics)),
      pipeline_type(VK_PIPELINE_BIND_POINT_GRAPHICS),
      create_flags(create_info.graphics.flags),
      shader_stages_ci(create_info.graphics.pStages, create_info.graphics.stageCount),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      vertex_input_state(CreateVertexInputState(*this, *state_data, create_info.graphics)),
      pre_raster_state(CreatePreRasterState(*this, *state_data, create_info.graphics, rpstate)),
      fragment_shader_state(CreateFragmentShaderState(*this, *state_data, *pCreateInfo, create_info.graphics, rpstate)),
      fragment_output_state(CreateFragmentOutputState(*this, *state_data, *pCreateInfo, create_info.graphics, rpstate)),
      stage_states(GetStageStates(*state_data, *this, csm_states)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      linking_shaders(GetLinkingShaders(library_create_info, *state_data)),
      active_shaders(create_info_shaders | linking_shaders),
      fragmentShader_writable_output_location_list(GetFSOutputLocations(stage_states)),
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(GetGraphicsDynamicState(*this)),
      topology_at_rasterizer(GetTopologyAtRasterizer(*this)),
      descriptor_buffer_mode((create_info.graphics.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), *this)),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
      csm_states(csm_states) {
    if (library_create_info) {
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
        for (uint32_t i = 0; i < library_create_info->libraryCount; ++i) {
            const auto &state = state_data->Get<PIPELINE_STATE>(library_create_info->pLibraries[i]);
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
      pipeline_type(VK_PIPELINE_BIND_POINT_COMPUTE),
      create_flags(create_info.compute.flags),
      shader_stages_ci(&create_info.compute.stage, 1),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      stage_states(GetStageStates(*state_data, *this, csm_states)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      active_shaders(create_info_shaders),  // compute has no linking shaders
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(0),  // compute has no dynamic state
      descriptor_buffer_mode((create_info.compute.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), *this)),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
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
      pipeline_type(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
      create_flags(create_info.raytracing.flags),
      shader_stages_ci(create_info.raytracing.pStages, create_info.raytracing.stageCount),
      ray_tracing_library_ci(create_info.raytracing.pLibraryInfo),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      stage_states(GetStageStates(*state_data, *this, csm_states)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      active_shaders(create_info_shaders),  // RTX has no linking shaders
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(0),  // RTX has no dynamic states being validated
      descriptor_buffer_mode((create_info.raytracing.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), *this)),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
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
      pipeline_type(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV),
      create_flags(create_info.raytracing.flags),
      shader_stages_ci(create_info.raytracing.pStages, create_info.raytracing.stageCount),
      ray_tracing_library_ci(create_info.raytracing.pLibraryInfo),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      stage_states(GetStageStates(*state_data, *this, csm_states)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      active_shaders(create_info_shaders),  // RTX has no linking shaders
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(0),  // RTX has no dynamic states being validated
      descriptor_buffer_mode((create_info.graphics.flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(PNext(), *this)),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
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

bool LAST_BOUND_STATE::IsDepthTestEnable() const {
    return pipeline_state->IsDynamic(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) ? cb_state.dynamic_state_value.depth_test_enable
                                                                         : pipeline_state->DepthStencilState()->depthTestEnable;
}

bool LAST_BOUND_STATE::IsDepthWriteEnable() const {
    // "Depth writes are always disabled when depthTestEnable is VK_FALSE"
    if (!IsDepthTestEnable()) {
        return false;
    }
    return pipeline_state->IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) ? cb_state.dynamic_state_value.depth_write_enable
                                                                          : pipeline_state->DepthStencilState()->depthWriteEnable;
}

bool LAST_BOUND_STATE::IsStencilTestEnable() const {
    return pipeline_state->IsDynamic(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE) ? cb_state.dynamic_state_value.stencil_test_enable
                                                                           : pipeline_state->DepthStencilState()->stencilTestEnable;
}

VkStencilOpState LAST_BOUND_STATE::GetStencilOpStateFront() const {
    VkStencilOpState front = pipeline_state->DepthStencilState()->front;
    if (pipeline_state->IsDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
        front.writeMask = cb_state.dynamic_state_value.write_mask_front;
    }
    if (pipeline_state->IsDynamic(VK_DYNAMIC_STATE_STENCIL_OP)) {
        front.failOp = cb_state.dynamic_state_value.fail_op_front;
        front.passOp = cb_state.dynamic_state_value.pass_op_front;
        front.depthFailOp = cb_state.dynamic_state_value.depth_fail_op_front;
    }
    return front;
}

VkStencilOpState LAST_BOUND_STATE::GetStencilOpStateBack() const {
    VkStencilOpState back = pipeline_state->DepthStencilState()->back;
    if (pipeline_state->IsDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
        back.writeMask = cb_state.dynamic_state_value.write_mask_back;
    }
    if (pipeline_state->IsDynamic(VK_DYNAMIC_STATE_STENCIL_OP)) {
        back.failOp = cb_state.dynamic_state_value.fail_op_back;
        back.passOp = cb_state.dynamic_state_value.pass_op_back;
        back.depthFailOp = cb_state.dynamic_state_value.depth_fail_op_back;
    }
    return back;
}

VkSampleCountFlagBits LAST_BOUND_STATE::GetRasterizationSamples() const {
    // For given pipeline, return number of MSAA samples, or one if MSAA disabled
    VkSampleCountFlagBits rasterization_samples = VK_SAMPLE_COUNT_1_BIT;
    if (pipeline_state->IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        rasterization_samples = cb_state.dynamic_state_value.rasterization_samples;
    } else {
        const auto ms_state = pipeline_state->MultisampleState();
        if (ms_state) {
            rasterization_samples = ms_state->rasterizationSamples;
        }
    }
    return rasterization_samples;
}

bool LAST_BOUND_STATE::IsRasterizationDisabled() const {
    return pipeline_state->IsDynamic(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
               ? cb_state.dynamic_state_value.rasterizer_discard_enable
               : pipeline_state->RasterizationDisabled();
}