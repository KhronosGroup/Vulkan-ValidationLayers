/* Copyright (c) 2021-2023 The Khronos Group Inc.
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

#include "state_tracker/shader_module.h"

#include <sstream>
#include <string>

#include "state_tracker/pipeline_state.h"
#include "state_tracker/descriptor_sets.h"
#include "generated/spirv_grammar_helper.h"

void DecorationBase::Add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationLocation:
            location = value;
            break;
        case spv::DecorationPatch:
            flags |= patch_bit;
            break;
        case spv::DecorationBlock:
            flags |= block_bit;
            break;
        case spv::DecorationBufferBlock:
            flags |= buffer_block_bit;
            break;
        case spv::DecorationComponent:
            component = value;
            break;
        case spv::DecorationNonWritable:
            flags |= nonwritable_bit;
            break;
        case spv::DecorationBuiltIn:
            assert(builtin == kInvalidValue);  // being over written - not valid
            builtin = value;
            break;
        case spv::DecorationNonReadable:
            flags |= nonreadable_bit;
            break;
        case spv::DecorationPerVertexKHR:  // VK_KHR_fragment_shader_barycentric
            flags |= per_vertex_bit;
            break;
        case spv::DecorationPassthroughNV:  // VK_NV_geometry_shader_passthrough
            flags |= passthrough_bit;
            break;
        case spv::DecorationAliased:
            flags |= aliased_bit;
            break;
        case spv::DecorationPerTaskNV:  // VK_NV_mesh_shader
            flags |= per_task_nv;
            break;
        case spv::DecorationPerPrimitiveEXT:  // VK_EXT_mesh_shader
            flags |= per_primitive_ext;
            break;
        default:
            break;
    }
}

// Some decorations are only avaiable for variables, so can't be in OpMemberDecorate
void DecorationSet::Add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationDescriptorSet:
            set = value;
            break;
        case spv::DecorationBinding:
            binding = value;
            break;
        case spv::DecorationInputAttachmentIndex:
            flags |= input_attachment_bit;
            input_attachment_index_start = value;
            break;
        default:
            DecorationBase::Add(decoration, value);
    }
}

bool DecorationSet::HasBuiltIn() const {
    if (kInvalidValue != builtin) {
        return true;
    } else if (!member_decorations.empty()) {
        for (const auto& member : member_decorations) {
            if (kInvalidValue != member.second.builtin) {
                return true;
            }
        }
    }
    return false;
}

bool DecorationSet::HasInMember(FlagBit flag_bit) const {
    for (const auto& decoration : member_decorations) {
        if (decoration.second.Has(flag_bit)) {
            return true;
        }
    }
    return false;
}

bool DecorationSet::AllMemberHave(FlagBit flag_bit) const {
    for (const auto& decoration : member_decorations) {
        if (!decoration.second.Has(flag_bit)) {
            return false;
        }
    }
    return true;
}

void ExecutionModeSet::Add(const Instruction& insn) {
    const uint32_t execution_mode = insn.Word(2);
    const uint32_t value = insn.Length() > 3u ? insn.Word(3) : 0u;
    switch (execution_mode) {
        case spv::ExecutionModeOutputPoints:  // for geometry shaders
            flags |= output_points_bit;
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case spv::ExecutionModePointMode:  // for tessellation shaders
            flags |= point_mode_bit;
            break;
        case spv::ExecutionModePostDepthCoverage:  // VK_EXT_post_depth_coverage
            flags |= post_depth_coverage_bit;
            break;
        case spv::ExecutionModeIsolines:  // Tessellation
            flags |= iso_lines_bit;
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case spv::ExecutionModeOutputLineStrip:
        case spv::ExecutionModeOutputLinesNV:
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case spv::ExecutionModeTriangles:
        case spv::ExecutionModeQuads:
        case spv::ExecutionModeOutputTriangleStrip:
        case spv::ExecutionModeOutputTrianglesNV:
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case spv::ExecutionModeLocalSizeId:
            flags |= local_size_id_bit;
            // Store ID here, will use flag to know to pull then out
            local_size_x = insn.Word(3);
            local_size_y = insn.Word(4);
            local_size_z = insn.Word(5);
            break;
        case spv::ExecutionModeLocalSize:
            flags |= local_size_bit;
            local_size_x = insn.Word(3);
            local_size_y = insn.Word(4);
            local_size_z = insn.Word(5);
            break;
        case spv::ExecutionModeOutputVertices:
            output_vertices = value;
            break;
        case spv::ExecutionModeOutputPrimitivesEXT:  // alias ExecutionModeOutputPrimitivesNV
            output_primitives = value;
            break;
        case spv::ExecutionModeXfb:  // TransformFeedback
            flags |= xfb_bit;
            break;
        case spv::ExecutionModeInvocations:
            invocations = value;
            break;
        case spv::ExecutionModeSignedZeroInfNanPreserve:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= signed_zero_inf_nan_preserve_width_16;
            } else if (value == 32) {
                flags |= signed_zero_inf_nan_preserve_width_32;
            } else if (value == 64) {
                flags |= signed_zero_inf_nan_preserve_width_64;
            }
            break;
        case spv::ExecutionModeDenormPreserve:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= denorm_preserve_width_16;
            } else if (value == 32) {
                flags |= denorm_preserve_width_32;
            } else if (value == 64) {
                flags |= denorm_preserve_width_64;
            }
            break;
        case spv::ExecutionModeDenormFlushToZero:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= denorm_flush_to_zero_width_16;
            } else if (value == 32) {
                flags |= denorm_flush_to_zero_width_32;
            } else if (value == 64) {
                flags |= denorm_flush_to_zero_width_64;
            }
            break;
        case spv::ExecutionModeRoundingModeRTE:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= rounding_mode_rte_width_16;
            } else if (value == 32) {
                flags |= rounding_mode_rte_width_32;
            } else if (value == 64) {
                flags |= rounding_mode_rte_width_64;
            }
            break;
        case spv::ExecutionModeRoundingModeRTZ:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= rounding_mode_rtz_width_16;
            } else if (value == 32) {
                flags |= rounding_mode_rtz_width_32;
            } else if (value == 64) {
                flags |= rounding_mode_rtz_width_64;
            }
            break;
        case spv::ExecutionModeEarlyFragmentTests:
            flags |= early_fragment_test_bit;
            break;
        case spv::ExecutionModeSubgroupUniformControlFlowKHR:  // VK_KHR_shader_subgroup_uniform_control_flow
            flags |= subgroup_uniform_control_flow_bit;
            break;
        default:
            break;
    }
}

static uint32_t ExecutionModelToShaderStageFlagBits(uint32_t mode) {
    switch (mode) {
        case spv::ExecutionModelVertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelTessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGeometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelFragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case spv::ExecutionModelGLCompute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        case spv::ExecutionModelRayGenerationKHR:
            return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case spv::ExecutionModelAnyHitKHR:
            return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case spv::ExecutionModelClosestHitKHR:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case spv::ExecutionModelMissKHR:
            return VK_SHADER_STAGE_MISS_BIT_KHR;
        case spv::ExecutionModelIntersectionKHR:
            return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case spv::ExecutionModelCallableKHR:
            return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        case spv::ExecutionModelTaskNV:
            return VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelMeshNV:
            return VK_SHADER_STAGE_MESH_BIT_NV;
        case spv::ExecutionModelTaskEXT:
            return VK_SHADER_STAGE_TASK_BIT_EXT;
        case spv::ExecutionModelMeshEXT:
            return VK_SHADER_STAGE_MESH_BIT_EXT;
        default:
            return 0;
    }
}

// TODO: The set of interesting opcodes here was determined by eyeballing the SPIRV spec. It might be worth
// converting parts of this to be generated from the machine-readable spec instead.
static void FindPointersAndObjects(const Instruction& insn, vvl::unordered_set<uint32_t>& result) {
    switch (insn.Opcode()) {
        case spv::OpLoad:
            result.insert(insn.Word(3));  // ptr
            break;
        case spv::OpStore:
            result.insert(insn.Word(1));  // ptr
            break;
        case spv::OpAccessChain:
        case spv::OpInBoundsAccessChain:
            result.insert(insn.Word(3));  // base ptr
            break;
        case spv::OpSampledImage:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQueryFormat:
        case spv::OpImageQueryOrder:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageTexelPointer:
            // Note: we only explore parts of the image which might actually contain ids we care about for the above analyses.
            //  - NOT the shader input/output interfaces.
            result.insert(insn.Word(3));  // Image or sampled image
            break;
        case spv::OpImageWrite:
            result.insert(insn.Word(1));  // Image -- different operand order to above
            break;
        case spv::OpFunctionCall:
            for (uint32_t i = 3; i < insn.Length(); i++) {
                result.insert(insn.Word(i));  // fn itself, and all args
            }
            break;

        case spv::OpExtInst:
            for (uint32_t i = 5; i < insn.Length(); i++) {
                result.insert(insn.Word(i));  // Operands to ext inst
            }
            break;

        default: {
            if (AtomicOperation(insn.Opcode())) {
                if (insn.Opcode() == spv::OpAtomicStore) {
                    result.insert(insn.Word(1));  // ptr
                } else {
                    result.insert(insn.Word(3));  // ptr
                }
            }
            break;
        }
    }
}

vvl::unordered_set<uint32_t> EntryPoint::GetAccessibleIds(const SHADER_MODULE_STATE& module_state, EntryPoint& entrypoint) {
    vvl::unordered_set<uint32_t> result_ids;
    if (!module_state.has_valid_spirv) {
        return result_ids;
    }

    // For some analyses, we need to know about all ids referenced by the static call tree of a particular entrypoint.
    // This is important for identifying the set of shader resources actually used by an entrypoint.
    vvl::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.id);

    while (!worklist.empty()) {
        auto worklist_id_iter = worklist.begin();
        auto worklist_id = *worklist_id_iter;
        worklist.erase(worklist_id_iter);

        const Instruction* next_insn = module_state.FindDef(worklist_id);
        if (!next_insn) {
            // ID is something we didn't collect in SpirvStaticData. that's OK -- we'll stumble across all kinds of things here
            // that we may not care about.
            continue;
        }

        // Try to add to the output set
        if (!result_ids.insert(worklist_id).second) {
            continue;  // If we already saw this id, we don't want to walk it again.
        }

        if (next_insn->Opcode() == spv::OpFunction) {
            // Scan whole body of the function
            while (++next_insn, next_insn->Opcode() != spv::OpFunctionEnd) {
                const auto& insn = *next_insn;
                // Build up list of accessible ID
                FindPointersAndObjects(insn, worklist);

                // Gather any instructions info that is only for the EntryPoint and not whole module
                switch (insn.Opcode()) {
                    case spv::OpEmitVertex:
                    case spv::OpEmitStreamVertex:
                        entrypoint.emit_vertex_geometry = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return result_ids;
}

std::vector<StageInteraceVariable> EntryPoint::GetStageInterfaceVariables(const SHADER_MODULE_STATE& module_state,
                                                                          const EntryPoint& entrypoint) {
    std::vector<StageInteraceVariable> variables;
    if (!module_state.has_valid_spirv) {
        return variables;
    }

    // spirv-val validates that any Input/Output used in the entrypoint is listed in as interface IDs
    uint32_t word = 3;  // operand Name operand starts
    // Find the end of the entrypoint's name string. additional zero bytes follow the actual null terminator, to fill out
    // the rest of the word - so we only need to look at the last byte in the word to determine which word contains the
    // terminator.
    while (entrypoint.entrypoint_insn.Word(word) & 0xff000000u) {
        ++word;
    }
    ++word;

    vvl::unordered_set<uint32_t> unique_interface_id;
    for (; word < entrypoint.entrypoint_insn.Length(); word++) {
        const uint32_t interface_id = entrypoint.entrypoint_insn.Word(word);
        if (unique_interface_id.insert(interface_id).second == false) {
            continue;  // Before SPIR-V 1.4 duplicates of these IDs are allowed
        };
        // guaranteed by spirv-val to be a OpVariable
        const Instruction& insn = *module_state.FindDef(interface_id);

        if (insn.Word(3) != spv::StorageClassInput && insn.Word(3) != spv::StorageClassOutput) {
            continue;  // Only checking for input/output here
        }
        variables.emplace_back(module_state, insn, entrypoint.stage);
    }
    return variables;
}

std::vector<ResourceInterfaceVariable> EntryPoint::GetResourceInterfaceVariables(const SHADER_MODULE_STATE& module_state,
                                                                                 const EntryPoint& entrypoint) {
    std::vector<ResourceInterfaceVariable> variables;
    if (!module_state.has_valid_spirv) {
        return variables;
    }

    // Now that the accessible_ids list is known, fill in any information that can be statically known per EntryPoint
    for (const auto& accessible_id : entrypoint.accessible_ids) {
        const Instruction& insn = *module_state.FindDef(accessible_id);
        if (insn.Opcode() != spv::OpVariable) {
            continue;
        }
        const uint32_t storage_class = insn.StorageClass();
        // These are the only storage classes that interface with a descriptor
        // see vkspec.html#interfaces-resources-descset
        if (storage_class == spv::StorageClassUniform || storage_class == spv::StorageClassUniformConstant ||
            storage_class == spv::StorageClassStorageBuffer) {
            variables.emplace_back(module_state, entrypoint, insn);
        }
    }
    return variables;
}

EntryPoint::EntryPoint(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint_insn)
    : entrypoint_insn(entrypoint_insn),
      execution_model(spv::ExecutionModel(entrypoint_insn.Word(1))),
      stage(static_cast<VkShaderStageFlagBits>(ExecutionModelToShaderStageFlagBits(execution_model))),
      id(entrypoint_insn.Word(2)),
      name(entrypoint_insn.GetAsString(3)),
      execution_mode(module_state.GetExecutionModeSet(id)),
      emit_vertex_geometry(false),
      accessible_ids(GetAccessibleIds(module_state, *this)),
      resource_interface_variables(GetResourceInterfaceVariables(module_state, *this)),
      stage_interface_variables(GetStageInterfaceVariables(module_state, *this)) {
    if (module_state.has_valid_spirv) {
        // After all variables are made, can get references from them
        // Also can set per-Entrypoint values now
        for (const auto& variable : stage_interface_variables) {
            if (variable.is_per_task_nv) {
                continue;  // SPV_NV_mesh_shader has a PerTaskNV which is not a builtin or interface
            }
            has_passthrough |= variable.decorations.Has(DecorationSet::passthrough_bit);

            if (variable.is_builtin) {
                built_in_variables.push_back(&variable);

                if (variable.storage_class == spv::StorageClassInput) {
                    builtin_input_components += variable.total_builtin_components;
                } else if (variable.storage_class == spv::StorageClassOutput) {
                    builtin_output_components += variable.total_builtin_components;
                }
            } else {
                user_defined_interface_variables.push_back(&variable);

                // After creating, make lookup table
                if (variable.interface_slots.empty()) {
                    continue;
                }
                for (const auto& slot : variable.interface_slots) {
                    if (variable.storage_class == spv::StorageClassInput) {
                        input_interface_slots[slot] = &variable;
                        if (!max_input_slot || slot.slot > max_input_slot->slot) {
                            max_input_slot = &slot;
                            max_input_slot_variable = &variable;
                        }
                    } else if (variable.storage_class == spv::StorageClassOutput) {
                        output_interface_slots[slot] = &variable;
                        if (!max_output_slot || slot.slot > max_output_slot->slot) {
                            max_output_slot = &slot;
                            max_output_slot_variable = &variable;
                        }
                    }
                }
            }
        }

        for (const Instruction* decoration_inst : module_state.static_data_.builtin_decoration_inst) {
            if ((decoration_inst->GetBuiltIn() == spv::BuiltInPointSize) && module_state.IsBuiltInWritten(decoration_inst, *this)) {
                written_builtin_point_size = true;
            }
            if ((decoration_inst->GetBuiltIn() == spv::BuiltInPrimitiveShadingRateKHR) &&
                module_state.IsBuiltInWritten(decoration_inst, *this)) {
                written_builtin_primitive_shading_rate_khr = true;
            }
            if ((decoration_inst->GetBuiltIn() == spv::BuiltInViewportIndex) &&
                module_state.IsBuiltInWritten(decoration_inst, *this)) {
                written_builtin_viewport_index = true;
            }
            if ((decoration_inst->GetBuiltIn() == spv::BuiltInViewportMaskNV) &&
                module_state.IsBuiltInWritten(decoration_inst, *this)) {
                written_builtin_viewport_mask_nv = true;
            }
        }
    }
}

std::optional<VkPrimitiveTopology> SHADER_MODULE_STATE::GetTopology(const EntryPoint& entrypoint) const {
    std::optional<VkPrimitiveTopology> result;

    // In tessellation shaders, PointMode is separate and trumps the tessellation topology.
    if (entrypoint.execution_mode.Has(ExecutionModeSet::point_mode_bit)) {
        result.emplace(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    } else if (entrypoint.execution_mode.primitive_topology != VK_PRIMITIVE_TOPOLOGY_MAX_ENUM) {
        result.emplace(entrypoint.execution_mode.primitive_topology);
    }

    return result;
}

static inline bool IsImageOperandsBiasOffset(uint32_t type) {
    return (type & (spv::ImageOperandsBiasMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask |
                    spv::ImageOperandsConstOffsetsMask)) != 0;
}

SHADER_MODULE_STATE::StaticData::StaticData(const SHADER_MODULE_STATE& module_state) {
    // Parse the words first so we have instruction class objects to use
    {
        std::vector<uint32_t>::const_iterator it = module_state.words_.cbegin();
        it += 5;  // skip first 5 word of header
        while (it != module_state.words_.cend()) {
            Instruction insn(it);
            const uint32_t opcode = insn.Opcode();

            // Check for opcodes that would require reparsing of the words
            if (opcode == spv::OpGroupDecorate || opcode == spv::OpDecorationGroup || opcode == spv::OpGroupMemberDecorate) {
                assert(has_group_decoration == false);  // if assert, spirv-opt didn't flatten it
                has_group_decoration = true;
                return;  // no need to continue parsing
            }

            instructions.push_back(insn);
            it += insn.Length();
        }
        instructions.shrink_to_fit();
    }

    // These have their own object class, but need entire module parsed first
    std::vector<const Instruction*> entry_point_instructions;
    std::vector<const Instruction*> type_struct_instructions;

    // Loop through once and build up the static data
    // Also process the entry points
    for (const Instruction& insn : instructions) {
        // Build definition list
        if (insn.ResultId() != 0) {
            definitions[insn.Word(insn.ResultId())] = &insn;
        }

        switch (insn.Opcode()) {
            // Specialization constants
            case spv::OpSpecConstantTrue:
            case spv::OpSpecConstantFalse:
            case spv::OpSpecConstant:
            case spv::OpSpecConstantComposite:
            case spv::OpSpecConstantOp:
                has_specialization_constants = true;
                break;

            // Decorations
            case spv::OpDecorate: {
                const uint32_t target_id = insn.Word(1);
                decorations[target_id].Add(insn.Word(2), insn.Length() > 3u ? insn.Word(3) : 0u);
                decoration_inst.push_back(&insn);
                if (insn.Word(2) == spv::DecorationBuiltIn) {
                    builtin_decoration_inst.push_back(&insn);
                } else if (insn.Word(2) == spv::DecorationSpecId) {
                    spec_const_map[insn.Word(3)] = target_id;
                }
            } break;
            case spv::OpMemberDecorate: {
                const uint32_t target_id = insn.Word(1);
                const uint32_t member_index = insn.Word(2);
                decorations[target_id].member_decorations[member_index].Add(insn.Word(3), insn.Length() > 4u ? insn.Word(4) : 0u);
                member_decoration_inst.push_back(&insn);
                if (insn.Word(3) == spv::DecorationBuiltIn) {
                    builtin_decoration_inst.push_back(&insn);
                }
            } break;

            case spv::OpCapability:
                capability_list.push_back(static_cast<spv::Capability>(insn.Word(1)));
                break;

            case spv::OpVariable:
                variable_inst.push_back(&insn);
                break;

            case spv::OpEmitStreamVertex:
            case spv::OpEndStreamPrimitive:
                transform_feedback_stream_inst.push_back(&insn);
                break;

            // Execution Mode
            case spv::OpExecutionMode:
            case spv::OpExecutionModeId: {
                execution_modes[insn.Word(1)].Add(insn);
            } break;
            // Listed from vkspec.html#ray-tracing-repack
            case spv::OpTraceRayKHR:
            case spv::OpTraceRayMotionNV:
            case spv::OpReportIntersectionKHR:
            case spv::OpExecuteCallableKHR:
                has_invocation_repack_instruction = true;
                break;

            // Entry points
            case spv::OpEntryPoint: {
                entry_point_instructions.push_back(&insn);
                break;
            }

            // Access operations
            case spv::OpImageSampleImplicitLod:
            case spv::OpImageSampleProjImplicitLod:
            case spv::OpImageSampleProjExplicitLod:
            case spv::OpImageSparseSampleImplicitLod:
            case spv::OpImageSparseSampleProjImplicitLod:
            case spv::OpImageSparseSampleProjExplicitLod: {
                // combined image samples are just OpLoad, but also can be separate image and sampler
                const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                auto load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(4) : insn.Word(3);
                sampler_load_ids.emplace_back(load_id);
                sampler_implicitLod_dref_proj_load_ids.emplace_back(load_id);
                // ImageOperands in index: 5
                if (insn.Length() > 5 && IsImageOperandsBiasOffset(insn.Word(5))) {
                    sampler_bias_offset_load_ids.emplace_back(load_id);
                }
                break;
            }
            case spv::OpImageDrefGather:
            case spv::OpImageSparseDrefGather: {
                // combined image samples are just OpLoad, but also can be separate image and sampler
                const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                auto load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(3) : insn.Word(3);
                image_dref_load_ids.emplace_back(load_id);
                break;
            }
            case spv::OpImageSampleDrefImplicitLod:
            case spv::OpImageSampleDrefExplicitLod:
            case spv::OpImageSampleProjDrefImplicitLod:
            case spv::OpImageSampleProjDrefExplicitLod:
            case spv::OpImageSparseSampleDrefImplicitLod:
            case spv::OpImageSparseSampleDrefExplicitLod:
            case spv::OpImageSparseSampleProjDrefImplicitLod:
            case spv::OpImageSparseSampleProjDrefExplicitLod: {
                // combined image samples are just OpLoad, but also can be separate image and sampler
                const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                auto sampler_load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(4) : insn.Word(3);
                auto image_load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(3) : insn.Word(3);

                image_dref_load_ids.emplace_back(image_load_id);
                sampler_load_ids.emplace_back(sampler_load_id);
                sampler_implicitLod_dref_proj_load_ids.emplace_back(sampler_load_id);
                // ImageOperands in index: 6
                if (insn.Length() > 6 && IsImageOperandsBiasOffset(insn.Word(6))) {
                    sampler_bias_offset_load_ids.emplace_back(sampler_load_id);
                }
                break;
            }
            case spv::OpImageSampleExplicitLod:
            case spv::OpImageSparseSampleExplicitLod: {
                // ImageOperands in index: 5
                if (insn.Length() > 5 && IsImageOperandsBiasOffset(insn.Word(5))) {
                    // combined image samples are just OpLoad, but also can be separate image and sampler
                    const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                    auto load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(4) : insn.Word(3);
                    sampler_load_ids.emplace_back(load_id);
                    sampler_bias_offset_load_ids.emplace_back(load_id);
                }
                break;
            }
            case spv::OpStore: {
                store_pointer_ids.emplace_back(insn.Word(1));  // object id or AccessChain id
                break;
            }
            case spv::OpImageRead:
            case spv::OpImageSparseRead: {
                image_read_load_ids.emplace_back(insn.Word(3));
                break;
            }
            case spv::OpImageWrite: {
                image_write_load_ids.emplace_back(insn.Word(1));
                image_write_load_id_map.emplace(&insn, insn.Word(1));
                break;
            }
            case spv::OpSampledImage: {
                // 3: image load id, 4: sampler load id
                sampled_image_load_ids.emplace_back(std::pair<uint32_t, uint32_t>(insn.Word(3), insn.Word(4)));
                break;
            }
            case spv::OpLoad: {
                // 2: Load id, 3: object id or AccessChain id
                load_members.emplace(insn.Word(2), insn.Word(3));
                break;
            }
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain: {
                if (insn.Length() == 4) {
                    // If it is for struct, the length is only 4.
                    // 2: AccessChain id, 3: object id
                    accesschain_members.emplace(insn.Word(2), std::pair<uint32_t, uint32_t>(insn.Word(3), 0));
                } else {
                    // 2: AccessChain id, 3: object id, 4: object id of array index
                    accesschain_members.emplace(insn.Word(2), std::pair<uint32_t, uint32_t>(insn.Word(3), insn.Word(4)));
                }
                break;
            }
            case spv::OpImageTexelPointer: {
                // 2: ImageTexelPointer id, 3: object id
                image_texel_pointer_members.emplace(insn.Word(2), insn.Word(3));
                break;
            }
            case spv::OpTypeStruct: {
                type_struct_instructions.push_back(&insn);
                break;
            }
            case spv::OpReadClockKHR: {
                read_clock_inst.push_back(&insn);
                break;
            }

            default:
                if (AtomicOperation(insn.Opcode())) {
                    atomic_inst.push_back(&insn);
                    if (insn.Opcode() == spv::OpAtomicStore) {
                        atomic_store_pointer_ids.emplace_back(insn.Word(1));
                        atomic_pointer_ids.emplace_back(insn.Word(1));
                    } else {
                        atomic_pointer_ids.emplace_back(insn.Word(3));
                    }
                }
                if (GroupOperation(insn.Opcode())) {
                    group_inst.push_back(&insn);
                }
                // We don't care about any other defs for now.
                break;
        }
    }

    for (const Instruction* decoration_inst : builtin_decoration_inst) {
        if (decoration_inst->GetBuiltIn() == spv::BuiltInLayer) {
            has_builtin_layer = true;
        } else if (decoration_inst->GetBuiltIn() == spv::BuiltInFullyCoveredEXT) {
            has_builtin_fully_covered = true;
        } else if (decoration_inst->GetBuiltIn() == spv::BuiltInWorkgroupSize) {
            has_builtin_workgroup_size = true;
            builtin_workgroup_size_id = decoration_inst->Word(1);
        }
    }

    // Need to get struct first and EntryPoint's variables depend on it
    for (const auto& insn : type_struct_instructions) {
        auto new_struct = type_structs.emplace_back(std::make_shared<TypeStructInfo>(module_state, *insn));
        type_struct_map[new_struct->id] = new_struct;
    }

    // Need to build the definitions table for FindDef before looking for which instructions each entry point uses
    for (const auto& insn : entry_point_instructions) {
        entry_points.emplace_back(std::make_shared<EntryPoint>(module_state, *insn));
    }

    SHADER_MODULE_STATE::SetPushConstantUsedInShader(module_state, entry_points);
}

void SHADER_MODULE_STATE::DescribeTypeInner(std::ostringstream& ss, uint32_t type, uint32_t indent) const {
    const Instruction* insn = FindDef(type);
    for (uint32_t i = 0; i < indent; i++) {
        ss << "\t";
    }

    switch (insn->Opcode()) {
        case spv::OpTypeBool:
            ss << "bool";
            break;
        case spv::OpTypeInt:
            ss << (insn->Word(3) ? 's' : 'u') << "int" << insn->Word(2);
            break;
        case spv::OpTypeFloat:
            ss << "float" << insn->Word(2);
            break;
        case spv::OpTypeVector:
            ss << "vec" << insn->Word(3) << " of ";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeMatrix:
            ss << "mat" << insn->Word(3) << " of ";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeArray:
            ss << "array[" << GetConstantValueById(insn->Word(3)) << "] of ";
            DescribeTypeInner(ss, insn->Word(2), 0);  // if struct, has pointer in between
            break;
        case spv::OpTypeRuntimeArray:
            ss << "runtime array[] of ";
            DescribeTypeInner(ss, insn->Word(2), 0);  // if struct, has pointer in between
            break;
        case spv::OpTypePointer:
            ss << "pointer to " << string_SpvStorageClass(insn->Word(2)) << " ->\n";
            indent++;
            DescribeTypeInner(ss, insn->Word(3), indent);
            break;
        case spv::OpTypeStruct: {
            ss << "struct of {\n";
            indent++;
            for (uint32_t i = 2; i < insn->Length(); i++) {
                DescribeTypeInner(ss, insn->Word(i), indent);
                ss << "\n";
            }
            indent--;
            for (uint32_t i = 0; i < indent; i++) {
                ss << "\t";
            }
            ss << "}";
            break;
        }
        case spv::OpTypeSampler:
            ss << "sampler";
            break;
        case spv::OpTypeSampledImage:
            ss << "sampler+";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeImage:
            ss << "image(dim=" << insn->Word(3) << ", sampled=" << insn->Word(7) << ")";
            break;
        case spv::OpTypeAccelerationStructureNV:
            ss << "accelerationStruture";
            break;
        default:
            ss << "oddtype";
            break;
    }
}

std::string SHADER_MODULE_STATE::DescribeType(uint32_t type) const {
    std::ostringstream ss;
    DescribeTypeInner(ss, type, 0);
    return ss.str();
}

const StructInfo* SHADER_MODULE_STATE::FindEntrypointPushConstant(char const* name, VkShaderStageFlagBits stageBits) const {
    for (const auto& entry_point : static_data_.entry_points) {
        if (entry_point->name.compare(name) == 0 && entry_point->stage == stageBits) {
            return &(entry_point->push_constant_used_in_shader);
        }
    }
    return nullptr;
}

std::shared_ptr<const EntryPoint> SHADER_MODULE_STATE::FindEntrypoint(char const* name, VkShaderStageFlagBits stageBits) const {
    for (const auto& entry_point : static_data_.entry_points) {
        if (entry_point->name.compare(name) == 0 && entry_point->stage == stageBits) {
            return entry_point;
        }
    }
    return nullptr;
}

// Because the following is legal, need the entry point
//    OpEntryPoint GLCompute %main "name_a"
//    OpEntryPoint GLCompute %main "name_b"
// Assumes shader module contains no spec constants used to set the local size values
bool SHADER_MODULE_STATE::FindLocalSize(const EntryPoint& entrypoint, uint32_t& local_size_x, uint32_t& local_size_y,
                                        uint32_t& local_size_z) const {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    if (static_data_.has_builtin_workgroup_size) {
        const Instruction* composite_def = FindDef(static_data_.builtin_workgroup_size_id);
        if (composite_def->Opcode() == spv::OpConstantComposite) {
            // VUID-WorkgroupSize-WorkgroupSize-04427 makes sure this is a OpTypeVector of int32
            local_size_x = GetConstantValueById(composite_def->Word(3));
            local_size_y = GetConstantValueById(composite_def->Word(4));
            local_size_z = GetConstantValueById(composite_def->Word(5));
            return true;
        }
    }

    if (entrypoint.execution_mode.Has(ExecutionModeSet::local_size_bit)) {
        local_size_x = entrypoint.execution_mode.local_size_x;
        local_size_y = entrypoint.execution_mode.local_size_y;
        local_size_z = entrypoint.execution_mode.local_size_z;
        return true;
    } else if (entrypoint.execution_mode.Has(ExecutionModeSet::local_size_id_bit)) {
        // Uses ExecutionModeLocalSizeId so need to resolve ID value
        local_size_x = GetConstantValueById(entrypoint.execution_mode.local_size_x);
        local_size_y = GetConstantValueById(entrypoint.execution_mode.local_size_y);
        local_size_z = GetConstantValueById(entrypoint.execution_mode.local_size_z);
        return true;
    }

    return false;  // not found
}

uint32_t SHADER_MODULE_STATE::CalculateComputeSharedMemory() const {
    uint32_t total_shared_size = 0;
    // when using WorkgroupMemoryExplicitLayoutKHR
    // either all or none the structs are decorated with Block,
    // if using block, all must decorated with Aliased.
    // In this case we want to find the MAX not ADD the block sizes
    bool find_max_block = false;

    for (const Instruction* insn : static_data_.variable_inst) {
        // StorageClass Workgroup is shared memory
        if (insn->StorageClass() == spv::StorageClassWorkgroup) {
            if (GetDecorationSet(insn->Word(2)).Has(DecorationSet::aliased_bit)) {
                find_max_block = true;
            }

            const uint32_t result_type_id = insn->Word(1);
            const Instruction* result_type = FindDef(result_type_id);
            const Instruction* type = FindDef(result_type->Word(3));
            const uint32_t variable_shared_size = GetTypeBytesSize(type);

            if (find_max_block) {
                total_shared_size = std::max(total_shared_size, variable_shared_size);
            } else {
                total_shared_size += variable_shared_size;
            }
        }
    }
    return total_shared_size;
}

// If the instruction at id is a constant or copy of a constant, returns a valid iterator pointing to that instruction.
// Otherwise, returns src->end().
const Instruction* SHADER_MODULE_STATE::GetConstantDef(uint32_t id) const {
    const Instruction* value = FindDef(id);

    // If id is a copy, see where it was copied from
    if (value && ((value->Opcode() == spv::OpCopyObject) || (value->Opcode() == spv::OpCopyLogical))) {
        id = value->Word(3);
        value = FindDef(id);
    }

    if (value && (value->Opcode() == spv::OpConstant)) {
        return value;
    }
    return nullptr;
}

// Either returns the constant value described by the instruction at id, or 1
uint32_t SHADER_MODULE_STATE::GetConstantValueById(uint32_t id) const {
    const Instruction* value = GetConstantDef(id);

    if (!value) {
        // TODO: Either ensure that the specialization transform is already performed on a module we're
        //       considering here, OR -- specialize on the fly now.
        // If using to get index into array, this could be hit if using VK_EXT_descriptor_indexing
        return 1;
    }

    return value->GetConstantValue();
}

// Returns the number of Location slots used for a given ID reference to a OpType*
uint32_t SHADER_MODULE_STATE::GetLocationsConsumedByType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetLocationsConsumedByType(insn->Word(3));
        case spv::OpTypeArray: {
            // Spec: "If an array of size n and each element takes m locations,
            // it will be assigned m × n consecutive locations starting with the location specified"
            const uint32_t locations = GetLocationsConsumedByType(insn->Word(2));
            const uint32_t array_size = GetConstantValueById(insn->Word(3));
            return locations * array_size;
        }
        case spv::OpTypeMatrix: {
            // Spec: "if n × m matrix, the number of locations assigned for each matrix will be the same as for an n-element array
            // of m-component vectors"
            const uint32_t column_type = insn->Word(2);
            const uint32_t column_count = insn->Word(3);
            return column_count * GetLocationsConsumedByType(column_type);
        }
        case spv::OpTypeVector: {
            const Instruction* scalar_type = FindDef(insn->Word(2));
            const uint32_t width = scalar_type->GetByteWidth();
            const uint32_t vector_length = insn->Word(3);
            const uint32_t components = width * vector_length;
            // Locations are 128-bit wide (4 components)
            // 3- and 4-component vectors of 64 bit types require two.
            return (components / 5) + 1;
        }
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            // first 2 words of struct are not the elements to check
            for (uint32_t i = 2; i < insn->Length(); i++) {
                sum += GetLocationsConsumedByType(insn->Word(i));
            }
            return sum;
        }
        default:
            // Scalars (Int, Float, Bool, etc) are are just 1.
            return 1;
    }
}

// Returns the number of Components slots used for a given ID reference to a OpType*
uint32_t SHADER_MODULE_STATE::GetComponentsConsumedByType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetComponentsConsumedByType(insn->Word(3));
        case spv::OpTypeArray:
            // Skip array as each array element is a whole new Location and we care only about the base type
            return GetComponentsConsumedByType(insn->Word(2));
        case spv::OpTypeMatrix: {
            const uint32_t column_type = insn->Word(2);
            const uint32_t column_count = insn->Word(3);
            return column_count * GetComponentsConsumedByType(column_type);
        }
        case spv::OpTypeVector: {
            const Instruction* scalar_type = FindDef(insn->Word(2));
            const uint32_t width = scalar_type->GetByteWidth();
            const uint32_t vector_length = insn->Word(3);
            return width * vector_length;  // One component is 32-bit
        }
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            // first 2 words of struct are not the elements to check
            for (uint32_t i = 2; i < insn->Length(); i++) {
                sum += GetComponentsConsumedByType(insn->Word(i));
            }
            return sum;
        }
        default:
            // Int, Float, Bool, etc
            return insn->GetByteWidth();
    }
}

// characterizes a SPIR-V type appearing in an interface to a FF stage, for comparison to a VkFormat's characterization above.
// also used for input attachments, as we statically know their format.
NumericType SHADER_MODULE_STATE::GetNumericType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypeInt:
            return insn->Word(3) ? NumericTypeSint : NumericTypeUint;
        case spv::OpTypeFloat:
            return NumericTypeFloat;
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeImage:
            return GetNumericType(insn->Word(2));
        case spv::OpTypePointer:
            return GetNumericType(insn->Word(3));
        default:
            return NumericTypeUnknown;
    }
}

const Instruction* SHADER_MODULE_STATE::GetStructType(const Instruction* insn) const {
    while (true) {
        if (insn->Opcode() == spv::OpTypePointer) {
            insn = FindDef(insn->Word(3));
        } else if (insn->Opcode() == spv::OpTypeArray) {
            insn = FindDef(insn->Word(2));
        } else if (insn->Opcode() == spv::OpTypeStruct) {
            return insn;
        } else {
            return nullptr;
        }
    }
}

void SHADER_MODULE_STATE::DefineStructMember(const Instruction* insn, StructInfo& data) const {
    const Instruction* struct_type = GetStructType(insn);
    data.size = 0;

    StructInfo data1;
    uint32_t element_index = 2;  // offset where first element in OpTypeStruct is
    uint32_t local_offset = 0;
    // offsets into struct
    std::vector<uint32_t> offsets;
    offsets.resize(struct_type->Length() - element_index);

    for (const Instruction* member_decorate : static_data_.member_decoration_inst) {
        if ((member_decorate->Word(1) == struct_type->Word(1)) && (member_decorate->Length() == 5) &&
            (member_decorate->Word(3) == spv::DecorationOffset)) {
            // The members of struct in SPRIV_R aren't always sort, so we need to know their order.
            offsets[member_decorate->Word(2)] = member_decorate->Word(4);
        }
    }

    for (const uint32_t offset : offsets) {
        local_offset = offset;
        data1 = {};
        data1.root = data.root;
        data1.offset = local_offset;
        const Instruction* def_member = FindDef(struct_type->Word(element_index));

        // Array could be multi-dimensional
        while (def_member->Opcode() == spv::OpTypeArray) {
            const auto len_id = def_member->Word(3);
            const Instruction* def_len = FindDef(len_id);
            data1.array_length_hierarchy.emplace_back(def_len->Word(3));  // array length
            def_member = FindDef(def_member->Word(2));
        }

        if (def_member->Opcode() == spv::OpTypeStruct) {
            DefineStructMember(def_member, data1);
        } else if (def_member->Opcode() == spv::OpTypePointer) {
            if (def_member->StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
                // If it's a pointer with PhysicalStorageBuffer class, this member is essentially a uint64_t containing an address
                // that "points to something."
                data1.size = 8;
            } else {
                // If it's OpTypePointer. it means the member is a buffer, the type will be TypePointer, and then struct
                DefineStructMember(def_member, data1);
            }
        } else {
            if (def_member->Opcode() == spv::OpTypeMatrix) {
                data1.array_length_hierarchy.emplace_back(def_member->Word(3));  // matrix's columns. matrix's row is vector.
                def_member = FindDef(def_member->Word(2));
            }

            if (def_member->Opcode() == spv::OpTypeVector) {
                data1.array_length_hierarchy.emplace_back(def_member->Word(3));  // vector length
                def_member = FindDef(def_member->Word(2));
            }

            // Get scalar type size. The value in SPRV-R is bit. It needs to translate to byte.
            data1.size = (def_member->Word(2) / 8);
        }
        const auto array_length_hierarchy_szie = data1.array_length_hierarchy.size();
        if (array_length_hierarchy_szie > 0) {
            data1.array_block_size.resize(array_length_hierarchy_szie, 1);

            for (int i2 = static_cast<int>(array_length_hierarchy_szie - 1); i2 > 0; --i2) {
                data1.array_block_size[i2 - 1] = data1.array_length_hierarchy[i2] * data1.array_block_size[i2];
            }
        }
        data.struct_members.emplace_back(data1);
        ++element_index;
    }
    uint32_t total_array_length = 1;
    for (const auto length : data1.array_length_hierarchy) {
        total_array_length *= length;
    }
    data.size = local_offset + data1.size * total_array_length;
}

uint32_t SHADER_MODULE_STATE::UpdateOffset(uint32_t offset, const std::vector<uint32_t>& array_indices,
                                           const StructInfo& data) const {
    int array_indices_size = static_cast<int>(array_indices.size());
    if (array_indices_size) {
        uint32_t array_index = 0;
        uint32_t i = 0;
        for (const auto index : array_indices) {
            array_index += (data.array_block_size[i] * index);
            ++i;
        }
        offset += (array_index * data.size);
    }
    return offset;
}

void SHADER_MODULE_STATE::SetUsedBytes(uint32_t offset, const std::vector<uint32_t>& array_indices, const StructInfo& data) const {
    int array_indices_size = static_cast<int>(array_indices.size());
    uint32_t block_memory_size = data.size;
    for (uint32_t i = static_cast<int>(array_indices_size); i < data.array_length_hierarchy.size(); ++i) {
        block_memory_size *= data.array_length_hierarchy[i];
    }

    offset = UpdateOffset(offset, array_indices, data);

    uint32_t end = offset + block_memory_size;
    auto used_bytes = data.GetUsedbytes();
    if (used_bytes->size() < end) {
        used_bytes->resize(end, 0);
    }
    std::memset(used_bytes->data() + offset, true, static_cast<std::size_t>(block_memory_size));
}

void SHADER_MODULE_STATE::RunUsedArray(uint32_t offset, std::vector<uint32_t> array_indices, uint32_t access_chain_word_index,
                                       const Instruction* access_chain, const StructInfo& data) const {
    if (access_chain_word_index < access_chain->Length()) {
        if (data.array_length_hierarchy.size() > array_indices.size()) {
            const Instruction* def = FindDef(access_chain->Word(access_chain_word_index));
            ++access_chain_word_index;

            if (def && def->Opcode() == spv::OpConstant) {
                array_indices.emplace_back(def->Word(3));
                RunUsedArray(offset, array_indices, access_chain_word_index, access_chain, data);
            } else {
                // If it is a variable, set the all array is used.
                if (access_chain_word_index < access_chain->Length()) {
                    uint32_t array_length = data.array_length_hierarchy[array_indices.size()];
                    for (uint32_t i = 0; i < array_length; ++i) {
                        auto array_indices2 = array_indices;
                        array_indices2.emplace_back(i);
                        RunUsedArray(offset, array_indices2, access_chain_word_index, access_chain, data);
                    }
                } else {
                    SetUsedBytes(offset, array_indices, data);
                }
            }
        } else {
            offset = UpdateOffset(offset, array_indices, data);
            RunUsedStruct(offset, access_chain_word_index, access_chain, data);
        }
    } else {
        SetUsedBytes(offset, array_indices, data);
    }
}

void SHADER_MODULE_STATE::RunUsedStruct(uint32_t offset, uint32_t access_chain_word_index, const Instruction* access_chain,
                                        const StructInfo& data) const {
    std::vector<uint32_t> array_indices_emptry;

    if (access_chain_word_index < access_chain->Length()) {
        auto strcut_member_index = GetConstantValueById(access_chain->Word(access_chain_word_index));
        ++access_chain_word_index;

        auto data1 = data.struct_members[strcut_member_index];
        RunUsedArray(offset + data1.offset, array_indices_emptry, access_chain_word_index, access_chain, data1);
    }
}

void SHADER_MODULE_STATE::SetUsedStructMember(const uint32_t variable_id, const vvl::unordered_set<uint32_t>& accessible_ids,
                                              const StructInfo& data) const {
    for (const auto& id : accessible_ids) {
        const Instruction* insn = FindDef(id);
        if (insn->Opcode() == spv::OpAccessChain) {
            if (insn->Word(3) == variable_id) {
                RunUsedStruct(0, 4, insn, data);
            }
        }
    }
}

void SHADER_MODULE_STATE::SetPushConstantUsedInShader(const SHADER_MODULE_STATE& module_state,
                                                      std::vector<std::shared_ptr<EntryPoint>>& entry_points) {
    for (auto& entrypoint : entry_points) {
        for (const Instruction* var_insn : module_state.static_data_.variable_inst) {
            if (var_insn->StorageClass() == spv::StorageClassPushConstant) {
                const Instruction* type = module_state.FindDef(var_insn->Word(1));
                entrypoint->push_constant_used_in_shader.root = &entrypoint->push_constant_used_in_shader;
                module_state.DefineStructMember(type, entrypoint->push_constant_used_in_shader);
                module_state.SetUsedStructMember(var_insn->Word(2), entrypoint->accessible_ids,
                                                 entrypoint->push_constant_used_in_shader);
            }
        }
    }
}

// For some built-in analysis we need to know if the variable decorated with as the built-in was actually written to.
// This function examines instructions in the static call tree for a write to this variable.
bool SHADER_MODULE_STATE::IsBuiltInWritten(const Instruction* builtin_insn, const EntryPoint& entrypoint) const {
    auto type = builtin_insn->Opcode();
    uint32_t target_id = builtin_insn->Word(1);
    bool init_complete = false;
    uint32_t target_member_offset = 0;

    if (type == spv::OpMemberDecorate) {
        // Built-in is part of a structure -- examine instructions up to first function body to get initial IDs
        for (const Instruction& insn : GetInstructions()) {
            if (insn.Opcode() == spv::OpFunction) {
                break;
            }
            switch (insn.Opcode()) {
                case spv::OpTypePointer:
                    if (insn.StorageClass() == spv::StorageClassOutput) {
                        const auto type_id = insn.Word(3);
                        if (type_id == target_id) {
                            target_id = insn.Word(1);
                        } else {
                            // If the output is an array, check if the element type is what we're looking for
                            const Instruction* type_def = FindDef(type_id);
                            if ((type_def->Opcode() == spv::OpTypeArray) && (type_def->Word(2) == target_id)) {
                                target_id = insn.Word(1);
                                target_member_offset = 1;
                            }
                        }
                    }
                    break;
                case spv::OpVariable:
                    if (insn.Word(1) == target_id) {
                        target_id = insn.Word(2);
                        init_complete = true;
                    }
                    break;
            }
        }
    }

    if (!init_complete && (type == spv::OpMemberDecorate)) return false;

    bool found_write = false;
    vvl::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.id);

    // Follow instructions in call graph looking for writes to target
    while (!worklist.empty() && !found_write) {
        auto worklist_id_iter = worklist.begin();
        auto worklist_id = *worklist_id_iter;
        worklist.erase(worklist_id_iter);

        const Instruction* insn = FindDef(worklist_id);
        if (!insn) {
            continue;
        }

        if (insn->Opcode() == spv::OpFunction) {
            // Scan body of function looking for other function calls or items in our ID chain
            while (++insn, (insn->Opcode() != spv::OpFunctionEnd) && !found_write) {
                switch (insn->Opcode()) {
                    case spv::OpAccessChain:
                    case spv::OpInBoundsAccessChain:
                        if (insn->Word(3) == target_id) {
                            if (type == spv::OpMemberDecorate) {
                                // Get the target member of the struct
                                // NOTE: this will only work for structs and arrays of structs. Deeper levels of nesting (e.g.,
                                // arrays of structs of structs) is not currently supported.
                                const Instruction* value_def = GetConstantDef(insn->Word(4 + target_member_offset));
                                if (value_def) {
                                    auto value = value_def->GetConstantValue();
                                    if (value == builtin_insn->Word(2)) {
                                        target_id = insn->Word(2);
                                    }
                                }
                            } else {
                                target_id = insn->Word(2);
                            }
                        }
                        break;
                    case spv::OpStore:
                        if (insn->Word(1) == target_id) {
                            found_write = true;
                        }
                        break;
                    case spv::OpFunctionCall:
                        worklist.insert(insn->Word(3));
                        break;
                }
            }
        }
    }
    return found_write;
}

// Takes a OpVariable ID and searches all ways it can be accessed from the access id lists
// Example:
//    %a = OpVariable
//    %b = OpLoad %a
//    %c = OpSampledImage %b
//
// %a == variable_id
// %c == access_ids
// %b == return value
const std::vector<const Instruction*> SHADER_MODULE_STATE::FindVariableAccesses(uint32_t variable_id,
                                                                                const std::vector<uint32_t>& access_ids,
                                                                                bool atomic) const {
    std::vector<const Instruction*> accessed_instructions;
    for (auto access_id : access_ids) {
        // The only time a direct access to a OpVariable is possible is in a Workgroup storage class
        // But this is checking resource variables which are not Workgroup
        assert(variable_id != access_id);

        // Atomic are accessed by OpImageTexelPointer instead of OpLoad
        // Currently for Atomics, just need to know if it was accessed or not
        uint32_t access_chain_load = 0;
        if (atomic) {
            // non image atomic operations (ex. OpAtomicIAdd) go straight to an OpAccessChain
            auto access_chain_it = static_data_.accesschain_members.find(access_id);
            if ((access_chain_it != static_data_.accesschain_members.end()) && (access_chain_it->second.first == variable_id)) {
                accessed_instructions.emplace_back(FindDef(access_chain_it->first));
                continue;
            }
            auto pointer_it = static_data_.image_texel_pointer_members.find(access_id);
            if (pointer_it == static_data_.image_texel_pointer_members.end()) {
                continue;  // if not here, won't be in AccessChain neither
            }
            if (pointer_it->second == variable_id) {
                accessed_instructions.emplace_back(FindDef(pointer_it->first));
                continue;
            } else {
                access_chain_load = pointer_it->second;
            }
        } else {
            auto load_it = static_data_.load_members.find(access_id);
            if (load_it == static_data_.load_members.end()) {
                continue;  // if not here, won't be in AccessChain neither
            }
            if (load_it->second == variable_id) {
                accessed_instructions.emplace_back(FindDef(load_it->first));
                continue;
            } else {
                access_chain_load = load_it->second;
            }
        }

        auto access_chain_it = static_data_.accesschain_members.find(access_chain_load);
        if ((access_chain_it != static_data_.accesschain_members.end()) && (access_chain_it->second.first == variable_id)) {
            accessed_instructions.emplace_back(FindDef(access_chain_it->first));
            continue;
        }
    }
    return accessed_instructions;
}

std::string InterfaceSlot::Describe() const {
    std::stringstream msg;
    msg << "Location = " << Location() << " | Component = " << Component() << " | Type = " << string_SpvOpcode(type) << " "
        << bit_width << " bits";
    return msg.str();
}

uint32_t GetFormatType(VkFormat format) {
    if (FormatIsSINT(format)) return NumericTypeSint;
    if (FormatIsUINT(format)) return NumericTypeUint;
    // Formats such as VK_FORMAT_D16_UNORM_S8_UINT are both
    if (FormatIsDepthAndStencil(format)) return NumericTypeFloat | NumericTypeUint;
    if (format == VK_FORMAT_UNDEFINED) return NumericTypeUnknown;
    // everything else -- UNORM/SNORM/FLOAT/USCALED/SSCALED is all float in the shader.
    return NumericTypeFloat;
}

char const* string_NumericType(uint32_t type) {
    if (type == NumericTypeSint) return "SINT";
    if (type == NumericTypeUint) return "UINT";
    if (type == NumericTypeFloat) return "FLOAT";
    return "(none)";
}

VariableBase::VariableBase(const SHADER_MODULE_STATE& module_state, const Instruction& insn, VkShaderStageFlagBits stage)
    : id(insn.Word(2)),
      type_id(insn.Word(1)),
      storage_class(static_cast<spv::StorageClass>(insn.Word(3))),
      decorations(module_state.GetDecorationSet(id)),
      type_struct_info(module_state.GetTypeStructInfo(&insn)),
      stage(stage) {
    assert(insn.Opcode() == spv::OpVariable);
}

bool StageInteraceVariable::IsPerTaskNV(const StageInteraceVariable& variable) {
    // will always be in a struct member
    if (variable.type_struct_info &&
        (variable.stage == VK_SHADER_STAGE_MESH_BIT_EXT || variable.stage == VK_SHADER_STAGE_TASK_BIT_EXT)) {
        return variable.type_struct_info->decorations.HasInMember(DecorationSet::per_task_nv);
    }
    return false;
}

// Some cases there is an array that is there to be "per-vertex" (or related)
// We want to remove this as it is not part of the Location caluclation or true type of variable
bool StageInteraceVariable::IsArrayInterface(const StageInteraceVariable& variable) {
    switch (variable.stage) {
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return variable.storage_class == spv::StorageClassInput;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return !variable.is_patch;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return !variable.is_patch && (variable.storage_class == spv::StorageClassInput);
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return variable.is_per_vertex && (variable.storage_class == spv::StorageClassInput);
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return !variable.is_per_task_nv && (variable.storage_class == spv::StorageClassOutput);
        default:
            break;
    }
    return false;
}

const Instruction& StageInteraceVariable::FindBaseType(const StageInteraceVariable& variable,
                                                       const SHADER_MODULE_STATE& module_state) {
    // base type is always just grabbing the type of the OpTypePointer tied to the variables
    // This is allowed only here because interface variables are never Phyiscal pointers
    const Instruction& base_type = *module_state.FindDef(module_state.FindDef(variable.type_id)->Word(3));

    // Strip away the array if one
    if (variable.is_array_interface && (base_type.Opcode() == spv::OpTypeArray || base_type.Opcode() == spv::OpTypeRuntimeArray)) {
        const uint32_t type_id = base_type.Word(2);
        return *module_state.FindDef(type_id);
    }
    // Most times won't be anything to strip
    return base_type;
}

bool StageInteraceVariable::IsBuiltin(const StageInteraceVariable& variable, const SHADER_MODULE_STATE& module_state) {
    const auto decoration_set = module_state.GetDecorationSet(variable.id);
    // If OpTypeStruct, will grab it's own decoration set
    return decoration_set.HasBuiltIn() || (variable.type_struct_info && variable.type_struct_info->decorations.HasBuiltIn());
}

std::vector<InterfaceSlot> StageInteraceVariable::GetInterfaceSlots(StageInteraceVariable& variable,
                                                                    const SHADER_MODULE_STATE& module_state) {
    std::vector<InterfaceSlot> slots;
    if (variable.is_builtin || variable.is_per_task_nv) {
        // SPV_NV_mesh_shader has a PerTaskNV which is not a builtin or interface
        return slots;
    }

    if (variable.type_struct_info) {
        // Structs has two options being labeled
        // 1. The block is given a Location, need to walk though and add up starting for that value
        // 2. The block is NOT given a Location, each member has dedicated decoration
        const bool block_decorated_with_location = variable.decorations.location != DecorationSet::kInvalidValue;
        if (block_decorated_with_location) {
            // In case of option 1, need to keep track as we go
            uint32_t base_location = variable.decorations.location;
            for (const uint32_t member_id : variable.type_struct_info->member_ids) {
                const uint32_t components = module_state.GetComponentsConsumedByType(member_id);

                // Info needed to test type matching later
                const Instruction* numerical_type = module_state.GetBaseTypeInstruction(member_id);
                const uint32_t numerical_type_opcode = numerical_type->Opcode();
                // TODO - Handle nested structs
                if (numerical_type_opcode == spv::OpTypeStruct) {
                    variable.nested_struct = true;
                    break;
                }
                const uint32_t numerical_type_width = numerical_type->GetBitWidth();

                for (uint32_t j = 0; j < components; j++) {
                    slots.emplace_back(base_location, j, numerical_type_opcode, numerical_type_width);
                }
                base_location++;  // If using, each members starts a new Location
            }
        } else {
            // Option 2
            for (uint32_t i = 0; i < variable.type_struct_info->length; i++) {
                const uint32_t member_id = variable.type_struct_info->member_ids[i];
                // Location/Components cant be decorated in nested structs, so no need to keep checking further
                // The spec says all or non of the member variables must have Location
                const auto member_decoration = variable.type_struct_info->decorations.member_decorations.at(i);
                const uint32_t location = member_decoration.location;
                const uint32_t starting_componet = member_decoration.component;
                const uint32_t components = module_state.GetComponentsConsumedByType(member_id);

                // Info needed to test type matching later
                const Instruction* numerical_type = module_state.GetBaseTypeInstruction(member_id);
                const uint32_t numerical_type_opcode = numerical_type->Opcode();
                const uint32_t numerical_type_width = numerical_type->GetBitWidth();

                for (uint32_t j = 0; j < components; j++) {
                    slots.emplace_back(location, starting_componet + j, numerical_type_opcode, numerical_type_width);
                }
            }
        }
    } else {
        uint32_t array_size = 1;
        uint32_t locations = 0;
        uint32_t sub_type_id = variable.type_id;
        // If there is a 64vec3[], each array index takes 2 location, so easier to pull it out here
        if (variable.base_type.Opcode() == spv::OpTypeArray) {
            array_size = module_state.GetConstantValueById(variable.base_type.Word(3));
            sub_type_id = variable.base_type.Word(2);
        }
        locations = module_state.GetLocationsConsumedByType(sub_type_id);

        // Info needed to test type matching later
        const Instruction* numerical_type = module_state.GetBaseTypeInstruction(sub_type_id);
        const uint32_t numerical_type_opcode = numerical_type->Opcode();
        const uint32_t numerical_type_width = numerical_type->GetBitWidth();

        const uint32_t starting_location = variable.decorations.location;
        const uint32_t starting_componet = variable.decorations.component;
        for (uint32_t array_index = 0; array_index < array_size; array_index++) {
            // offet into array if there is one
            const uint32_t location = starting_location + (locations * array_index);
            const uint32_t components = module_state.GetComponentsConsumedByType(sub_type_id);
            for (uint32_t component = 0; component < components; component++) {
                slots.emplace_back(location, component + starting_componet, numerical_type_opcode, numerical_type_width);
            }
        }
    }
    return slots;
}

std::vector<uint32_t> StageInteraceVariable::GetBuiltinBlock(const StageInteraceVariable& variable,
                                                             const SHADER_MODULE_STATE& module_state) {
    // Built-in Location slot will always be [zero, size]
    std::vector<uint32_t> slots;
    // Only check block built-ins - many builtin are non-block and not used between shaders
    if (!variable.is_builtin || !variable.type_struct_info) {
        return slots;
    }

    const auto& decoration_set = variable.type_struct_info->decorations;
    if (decoration_set.Has(DecorationSet::block_bit)) {
        for (uint32_t i = 0; i < variable.type_struct_info->length; i++) {
            slots.push_back(decoration_set.member_decorations.at(i).builtin);
        }
    }
    return slots;
}

uint32_t StageInteraceVariable::GetBuiltinComponents(const StageInteraceVariable& variable,
                                                     const SHADER_MODULE_STATE& module_state) {
    uint32_t count = 0;
    if (!variable.is_builtin) {
        return count;
    }
    if (variable.type_struct_info) {
        for (const uint32_t member_id : variable.type_struct_info->member_ids) {
            count += module_state.GetComponentsConsumedByType(member_id);
        }
    } else {
        const uint32_t base_type_id = variable.base_type.Word(variable.base_type.ResultId());
        count += module_state.GetComponentsConsumedByType(base_type_id);
    }
    return count;
}

StageInteraceVariable::StageInteraceVariable(const SHADER_MODULE_STATE& module_state, const Instruction& insn,
                                             VkShaderStageFlagBits stage)
    : VariableBase(module_state, insn, stage),
      is_patch(decorations.Has(DecorationSet::patch_bit)),
      is_per_vertex(decorations.Has(DecorationSet::per_vertex_bit)),
      is_per_task_nv(IsPerTaskNV(*this)),
      is_array_interface(IsArrayInterface(*this)),
      base_type(FindBaseType(*this, module_state)),
      is_builtin(IsBuiltin(*this, module_state)),
      nested_struct(false),
      interface_slots(GetInterfaceSlots(*this, module_state)),
      builtin_block(GetBuiltinBlock(*this, module_state)),
      total_builtin_components(GetBuiltinComponents(*this, module_state)) {}

const Instruction& ResourceInterfaceVariable::FindBaseType(ResourceInterfaceVariable& variable,
                                                           const SHADER_MODULE_STATE& module_state) {
    // Takes a OpVariable and looks at the the descriptor type it uses. This will find things such as if the variable is writable,
    // image atomic operation, matching images to samplers, etc
    const Instruction* type = module_state.FindDef(variable.type_id);

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type->Opcode() == spv::OpTypeArray || type->Opcode() == spv::OpTypePointer ||
           type->Opcode() == spv::OpTypeRuntimeArray || type->Opcode() == spv::OpTypeSampledImage) {
        if (type->Opcode() == spv::OpTypeArray || type->Opcode() == spv::OpTypeRuntimeArray ||
            type->Opcode() == spv::OpTypeSampledImage) {
            // currently just tracks 1D arrays
            if (type->Opcode() == spv::OpTypeArray && variable.array_length == 0) {
                variable.array_length = module_state.GetConstantValueById(type->Word(3));
            }
            if (type->Opcode() == spv::OpTypeRuntimeArray) {
                variable.runtime_array = true;
            }

            type = module_state.FindDef(type->Word(2));  // Element type
        } else {
            type = module_state.FindDef(type->Word(3));  // Pointer type
        }
    }
    return *type;
}

NumericType ResourceInterfaceVariable::FindImageFormatType(const SHADER_MODULE_STATE& module_state, const Instruction& base_type) {
    return (base_type.Opcode() == spv::OpTypeImage) ? module_state.GetNumericType(base_type.Word(2)) : NumericTypeUnknown;
}

bool ResourceInterfaceVariable::IsStorageBuffer(const ResourceInterfaceVariable& variable) {
    // before VK_KHR_storage_buffer_storage_class Storage Buffer were a Uniform storage class
    const bool physical_storage_buffer = variable.storage_class == spv::StorageClassPhysicalStorageBuffer;
    const bool storage_buffer = variable.storage_class == spv::StorageClassStorageBuffer;
    const bool uniform = variable.storage_class == spv::StorageClassUniform;
    const bool buffer_block = variable.decorations.Has(DecorationSet::buffer_block_bit);
    const bool block = variable.decorations.Has(DecorationSet::block_bit);
    return ((uniform && buffer_block) || ((storage_buffer || physical_storage_buffer) && block));
}

ResourceInterfaceVariable::ResourceInterfaceVariable(const SHADER_MODULE_STATE& module_state, const EntryPoint& entrypoint,
                                                     const Instruction& insn)
    : VariableBase(module_state, insn, entrypoint.stage),
      base_type(FindBaseType(*this, module_state)),
      image_format_type(FindImageFormatType(module_state, base_type)),
      image_dim(base_type.FindImageDim()),
      is_image_array(base_type.IsArrayed()),
      is_multisampled(base_type.IsMultisampled()),
      is_storage_buffer(IsStorageBuffer(*this)) {
    const auto& static_data_ = module_state.static_data_;
    // Handle anything specific to the base type
    switch (base_type.Opcode()) {
        case spv::OpTypeImage: {
            image_sampled_type_width = module_state.GetTypeBitsSize(&base_type);

            const bool is_sampled_without_sampler = base_type.Word(7) == 2;  // Word(7) == Sampled
            const spv::Dim image_dim = spv::Dim(base_type.Word(3));
            if (is_sampled_without_sampler) {
                if (image_dim == spv::DimSubpassData) {
                    is_input_attachment = true;
                    input_attachment_index_read.resize(array_length);  // is zero if runtime array
                } else if (image_dim == spv::DimBuffer) {
                    is_storage_texel_buffer = true;
                } else {
                    is_storage_image = true;
                }
            }

            const bool is_image_without_format = ((is_sampled_without_sampler) && (base_type.Word(8) == spv::ImageFormatUnknown));

            auto image_write_loads = module_state.FindVariableAccesses(id, static_data_.image_write_load_ids, false);
            if (!image_write_loads.empty()) {
                is_written_to = true;
                if (is_written_to && is_image_without_format) {
                    is_write_without_format = true;
                    for (const auto& entry : static_data_.image_write_load_id_map) {
                        for (const Instruction* insn : image_write_loads) {
                            if (insn->Word(insn->ResultId()) == entry.second) {
                                const uint32_t texel_component_count = module_state.GetTexelComponentCount(*entry.first);
                                write_without_formats_component_count_list.emplace_back(*entry.first, texel_component_count);
                            }
                        }
                    }
                }
            }

            auto image_read_loads = module_state.FindVariableAccesses(id, static_data_.image_read_load_ids, false);
            if (!image_read_loads.empty()) {
                is_read_from = true;
                if (is_image_without_format) {
                    is_read_without_format = true;
                }

                // If accessed in an array, track which indexes were read, if not runtime array
                if (is_input_attachment && !runtime_array) {
                    for (const Instruction* insn : image_read_loads) {
                        if (insn->Opcode() == spv::OpAccessChain) {
                            // TODO 5465 - Better way to check for descriptor indexing
                            const Instruction* const_def = module_state.GetConstantDef(insn->Word(4));
                            if (const_def) {
                                const uint32_t access_index = const_def->GetConstantValue();
                                input_attachment_index_read[access_index] = true;
                            }
                        } else if (insn->Opcode() == spv::OpLoad) {
                            // if InputAttachment is accessed from load, just a single, non-array, index
                            input_attachment_index_read.resize(1);
                            input_attachment_index_read[0] = true;
                        }
                    }
                }
            }
            if (!module_state.FindVariableAccesses(id, static_data_.sampler_load_ids, false).empty()) {
                is_sampler_sampled = true;
            }
            if (!module_state.FindVariableAccesses(id, static_data_.sampler_implicitLod_dref_proj_load_ids, false).empty()) {
                is_sampler_implicitLod_dref_proj = true;
            }
            if (!module_state.FindVariableAccesses(id, static_data_.sampler_bias_offset_load_ids, false).empty()) {
                is_sampler_bias_offset = true;
            }
            if (!module_state.FindVariableAccesses(id, static_data_.image_dref_load_ids, false).empty()) {
                is_dref_operation = true;
            }

            // Search all <image, sampler> combos (if not CombinedImageSampler)
            for (auto& sampled_image_load_id : static_data_.sampled_image_load_ids) {
                std::vector<uint32_t> image_load_ids = {sampled_image_load_id.first};
                auto image_loads = module_state.FindVariableAccesses(id, image_load_ids, false);
                if (image_loads.empty()) {
                    continue;
                }

                uint32_t image_index = 0;
                // If Image is an array, need to get the index
                // Currently just need to care about the first image_loads because the above loop will have combos to
                // image-to-samplers for us
                if (image_loads[0]->Opcode() == spv::OpAccessChain) {
                    // TODO 5465 - Can this just be GetConstantValueById() - is a spec const possible here?
                    const Instruction* const_def = module_state.GetConstantDef(image_loads[0]->Word(4));
                    if (!const_def) {
                        continue;  // access chain index not a constant
                    }
                    image_index = const_def->GetConstantValue();
                }

                // Find sampler's set binding.
                auto load_it = static_data_.load_members.find(sampled_image_load_id.second);
                if (load_it == static_data_.load_members.end()) {
                    continue;
                } else {
                    uint32_t sampler_id = load_it->second;
                    uint32_t sampler_index = 0;
                    auto accesschain_it = static_data_.accesschain_members.find(load_it->second);

                    if (accesschain_it != static_data_.accesschain_members.end()) {
                        const Instruction* const_def = module_state.GetConstantDef(accesschain_it->second.second);
                        if (!const_def) {
                            // access chain index representing sampler index is not a constant, skip.
                            break;
                        }
                        sampler_id = const_def->Word(const_def->ResultId());
                        sampler_index = const_def->GetConstantValue();
                    }
                    const auto sampler_dec = module_state.GetDecorationSet(sampler_id);
                    if (image_index >= samplers_used_by_image.size()) {
                        samplers_used_by_image.resize(image_index + 1);
                    }

                    // Need to check again for these properties in case not using a combined image sampler
                    if (!module_state.FindVariableAccesses(sampler_id, static_data_.sampler_load_ids, false).empty()) {
                        is_sampler_sampled = true;
                    }
                    if (!module_state.FindVariableAccesses(sampler_id, static_data_.sampler_implicitLod_dref_proj_load_ids, false)
                             .empty()) {
                        is_sampler_implicitLod_dref_proj = true;
                    }
                    if (!module_state.FindVariableAccesses(sampler_id, static_data_.sampler_bias_offset_load_ids, false).empty()) {
                        is_sampler_bias_offset = true;
                    }

                    samplers_used_by_image[image_index].emplace(
                        SamplerUsedByImage{DescriptorSlot{sampler_dec.set, sampler_dec.binding}, sampler_index});
                }
            }
            break;
        }

        case spv::OpTypeStruct: {
            // A buffer is writable if it's either flavor of storage buffer, and has any member not decorated
            // as nonwritable.
            if (is_storage_buffer && !type_struct_info->decorations.AllMemberHave(DecorationSet::nonwritable_bit)) {
                if (!module_state.FindVariableAccesses(id, static_data_.store_pointer_ids, false).empty()) {
                    is_written_to = true;
                    break;
                }
                if (!module_state.FindVariableAccesses(id, static_data_.atomic_store_pointer_ids, true).empty()) {
                    is_written_to = true;
                    break;
                }
            }

            break;
        }
        default:
            break;
    }

    // Type independent checks
    if (!module_state.FindVariableAccesses(id, static_data_.atomic_pointer_ids, true).empty()) {
        is_atomic_operation = true;
    }
}

TypeStructInfo::TypeStructInfo(const SHADER_MODULE_STATE& module_state, const Instruction& struct_insn)
    : id(struct_insn.Word(1)), length(struct_insn.Length() - 2), decorations(module_state.GetDecorationSet(id)) {
    member_ids.resize(length);
    for (uint32_t i = 0; i < length; i++) {
        member_ids[i] = struct_insn.Word(2 + i);
    }
}

uint32_t SHADER_MODULE_STATE::GetNumComponentsInBaseType(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    uint32_t component_count = 0;
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt) {
        component_count = 1;
    } else if (opcode == spv::OpTypeVector) {
        component_count = insn->Word(3);
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        // Because we are calculating components for a single location we do not care about column count
        component_count = GetNumComponentsInBaseType(column_type);  // vector length
    } else if (opcode == spv::OpTypeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        component_count = GetNumComponentsInBaseType(element_type);  // element length
    } else if (opcode == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); ++i) {
            component_count += GetNumComponentsInBaseType(FindDef(insn->Word(i)));
        }
    } else if (opcode == spv::OpTypePointer) {
        const Instruction* type = FindDef(insn->Word(3));
        component_count = GetNumComponentsInBaseType(type);
    }
    return component_count;
}

// Returns the total size in 'bits' of any OpType*
uint32_t SHADER_MODULE_STATE::GetTypeBitsSize(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    uint32_t bit_size = 0;
    if (opcode == spv::OpTypeVector) {
        const Instruction* component_type = FindDef(insn->Word(2));
        uint32_t scalar_width = GetTypeBitsSize(component_type);
        uint32_t component_count = insn->Word(3);
        bit_size = scalar_width * component_count;
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        uint32_t vector_width = GetTypeBitsSize(column_type);
        uint32_t column_count = insn->Word(3);
        bit_size = vector_width * column_count;
    } else if (opcode == spv::OpTypeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        uint32_t element_width = GetTypeBitsSize(element_type);
        const Instruction* length_type = FindDef(insn->Word(3));
        uint32_t length = length_type->GetConstantValue();
        bit_size = element_width * length;
    } else if (opcode == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); ++i) {
            bit_size += GetTypeBitsSize(FindDef(insn->Word(i)));
        }
    } else if (opcode == spv::OpTypePointer) {
        const Instruction* type = FindDef(insn->Word(3));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpVariable) {
        const Instruction* type = FindDef(insn->Word(1));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeImage) {
        const Instruction* type = FindDef(insn->Word(2));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeVoid) {
        // Sampled Type of OpTypeImage can be a void
        bit_size = 0;
    } else {
        bit_size = insn->GetBitWidth();
    }

    return bit_size;
}

// Returns the total size in 'bytes' of any OpType*
uint32_t SHADER_MODULE_STATE::GetTypeBytesSize(const Instruction* insn) const { return GetTypeBitsSize(insn) / 8; }

// Returns the base type (float, int or unsigned int) or struct (can have multiple different base types inside)
// Will return 0 if it can not be determined
uint32_t SHADER_MODULE_STATE::GetBaseType(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt || opcode == spv::OpTypeBool || opcode == spv::OpTypeStruct) {
        // point to itself as its the base type (or a struct that needs to be traversed still)
        return insn->Word(1);
    } else if (opcode == spv::OpTypeVector) {
        const Instruction* component_type = FindDef(insn->Word(2));
        return GetBaseType(component_type);
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        return GetBaseType(column_type);
    } else if (opcode == spv::OpTypeArray || opcode == spv::OpTypeRuntimeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        return GetBaseType(element_type);
    } else if (opcode == spv::OpTypePointer) {
        const auto& storage_class = insn->StorageClass();
        const Instruction* type = FindDef(insn->Word(3));
        if (storage_class == spv::StorageClassPhysicalStorageBuffer && type->Opcode() == spv::OpTypeStruct) {
            // A physical storage buffer to a struct has a chance to point to itself and can't resolve a baseType
            // GLSL example:
            // layout(buffer_reference) buffer T1 {
            //     T1 b[2];
            // };
            return 0;
        }
        return GetBaseType(type);
    }
    // If we assert here, we are missing a valid base type that must be handled. Without this assert, a return value of 0 will
    // produce a hard bug to track
    assert(false);
    return 0;
}

const Instruction* SHADER_MODULE_STATE::GetBaseTypeInstruction(uint32_t type) const {
    const Instruction* insn = FindDef(type);
    const uint32_t base_insn_id = GetBaseType(insn);
    // Will return end() if an invalid/unknown base_insn_id is returned
    return FindDef(base_insn_id);
}

// Returns type_id if id has type or zero otherwise
uint32_t SHADER_MODULE_STATE::GetTypeId(uint32_t id) const {
    const Instruction* type = FindDef(id);
    return type ? type->Word(type->TypeId()) : 0;
}

// Return zero if nothing is found
uint32_t SHADER_MODULE_STATE::GetTexelComponentCount(const Instruction& insn) const {
    uint32_t texel_component_count = 0;
    switch (insn.Opcode()) {
        case spv::OpImageWrite: {
            const Instruction* texel_def = FindDef(insn.Word(3));
            const Instruction* texel_type = FindDef(texel_def->Word(1));
            texel_component_count = (texel_type->Opcode() == spv::OpTypeVector) ? texel_type->Word(3) : 1;
            break;
        }
        default:
            break;
    }
    return texel_component_count;
}
