/* Copyright (c) 2021-2022 The Khronos Group Inc.
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
 * Author: Spencer Fricke <s.fricke@samsung.com>
 */

#include "shader_module.h"

#include <sstream>
#include <string>

#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "pipeline_state.h"
#include "descriptor_sets.h"
#include "spirv_grammar_helper.h"

void decoration_set::merge(decoration_set const &other) {
    if (other.flags & location_bit) location = other.location;
    if (other.flags & component_bit) component = other.component;
    if (other.flags & input_attachment_index_bit) input_attachment_index = other.input_attachment_index;
    if (other.flags & descriptor_set_bit) descriptor_set = other.descriptor_set;
    if (other.flags & binding_bit) binding = other.binding;
    if (other.flags & builtin_bit) builtin = other.builtin;
    flags |= other.flags;
}

void decoration_set::add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationLocation:
            flags |= location_bit;
            location = value;
            break;
        case spv::DecorationPatch:
            flags |= patch_bit;
            break;
        case spv::DecorationRelaxedPrecision:
            flags |= relaxed_precision_bit;
            break;
        case spv::DecorationBlock:
            flags |= block_bit;
            break;
        case spv::DecorationBufferBlock:
            flags |= buffer_block_bit;
            break;
        case spv::DecorationComponent:
            flags |= component_bit;
            component = value;
            break;
        case spv::DecorationInputAttachmentIndex:
            flags |= input_attachment_index_bit;
            input_attachment_index = value;
            break;
        case spv::DecorationDescriptorSet:
            flags |= descriptor_set_bit;
            descriptor_set = value;
            break;
        case spv::DecorationBinding:
            flags |= binding_bit;
            binding = value;
            break;
        case spv::DecorationNonWritable:
            flags |= nonwritable_bit;
            break;
        case spv::DecorationBuiltIn:
            flags |= builtin_bit;
            builtin = value;
            break;
        case spv::DecorationNonReadable:
            flags |= nonreadable_bit;
            break;
        case spv::DecorationPerVertexNV:
            flags |= per_vertex_bit;
            break;
        case spv::DecorationPassthroughNV:
            flags |= passthrough_bit;
            break;
        case spv::DecorationAliased:
            flags |= aliased_bit;
            break;
    }
}

std::string shader_struct_member::GetLocationDesc(uint32_t index_used_bytes) const {
    std::string desc = "";
    if (array_length_hierarchy.size() > 0) {
        desc += " index:";
        for (const auto block_size : array_block_size) {
            desc += "[";
            desc += std::to_string(index_used_bytes / (block_size * size));
            desc += "]";
            index_used_bytes = index_used_bytes % (block_size * size);
        }
    }
    const int struct_members_size = static_cast<int>(struct_members.size());
    if (struct_members_size > 0) {
        desc += " member:";
        for (int i = struct_members_size - 1; i >= 0; --i) {
            if (index_used_bytes > struct_members[i].offset) {
                desc += std::to_string(i);
                desc += struct_members[i].GetLocationDesc(index_used_bytes - struct_members[i].offset);
                break;
            }
        }
    } else {
        desc += " offset:";
        desc += std::to_string(index_used_bytes);
    }
    return desc;
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

// For some analyses, we need to know about all ids referenced by the static call tree of a particular entrypoint. This is
// important for identifying the set of shader resources actually used by an entrypoint, for example.
// Note: we only explore parts of the image which might actually contain ids we care about for the above analyses.
//  - NOT the shader input/output interfaces.
//
// TODO: The set of interesting opcodes here was determined by eyeballing the SPIRV spec. It might be worth
// converting parts of this to be generated from the machine-readable spec instead.
layer_data::unordered_set<uint32_t> SHADER_MODULE_STATE::MarkAccessibleIds(spirv_inst_iter entrypoint) const {
    layer_data::unordered_set<uint32_t> ids;
    if (entrypoint == end() || !has_valid_spirv) {
        return ids;
    }
    layer_data::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.word(2));

    while (!worklist.empty()) {
        auto id_iter = worklist.begin();
        auto id = *id_iter;
        worklist.erase(id_iter);

        auto insn = get_def(id);
        if (insn == end()) {
            // ID is something we didn't collect in SpirvStaticData. that's OK -- we'll stumble across all kinds of things here
            // that we may not care about.
            continue;
        }

        // Try to add to the output set
        if (!ids.insert(id).second) {
            continue;  // If we already saw this id, we don't want to walk it again.
        }

        switch (insn.opcode()) {
            case spv::OpFunction:
                // Scan whole body of the function, enlisting anything interesting
                while (++insn, insn.opcode() != spv::OpFunctionEnd) {
                    switch (insn.opcode()) {
                        case spv::OpLoad:
                            worklist.insert(insn.word(3));  // ptr
                            break;
                        case spv::OpStore:
                            worklist.insert(insn.word(1));  // ptr
                            break;
                        case spv::OpAccessChain:
                        case spv::OpInBoundsAccessChain:
                            worklist.insert(insn.word(3));  // base ptr
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
                            worklist.insert(insn.word(3));  // Image or sampled image
                            break;
                        case spv::OpImageWrite:
                            worklist.insert(insn.word(1));  // Image -- different operand order to above
                            break;
                        case spv::OpFunctionCall:
                            for (uint32_t i = 3; i < insn.len(); i++) {
                                worklist.insert(insn.word(i));  // fn itself, and all args
                            }
                            break;

                        case spv::OpExtInst:
                            for (uint32_t i = 5; i < insn.len(); i++) {
                                worklist.insert(insn.word(i));  // Operands to ext inst
                            }
                            break;

                        default: {
                            if (AtomicOperation(insn.opcode())) {
                                if (insn.opcode() == spv::OpAtomicStore) {
                                    worklist.insert(insn.word(1));  // ptr
                                } else {
                                    worklist.insert(insn.word(3));  // ptr
                                }
                            }
                            break;
                        }
                    }
                }
                break;
        }
    }

    return ids;
}

layer_data::optional<VkPrimitiveTopology> SHADER_MODULE_STATE::GetTopology(const spirv_inst_iter &entrypoint) const {
    layer_data::optional<VkPrimitiveTopology> result;

    auto entrypoint_id = entrypoint.word(2);
    bool is_point_mode = false;

    auto it = static_data_.execution_mode_inst.find(entrypoint_id);
    if (it != static_data_.execution_mode_inst.end()) {
        for (auto insn : it->second) {
            switch (insn.word(2)) {
                case spv::ExecutionModePointMode:
                    // In tessellation shaders, PointMode is separate and trumps the tessellation topology.
                    is_point_mode = true;
                    break;

                case spv::ExecutionModeOutputPoints:
                    result.emplace(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
                    break;

                case spv::ExecutionModeIsolines:
                case spv::ExecutionModeOutputLineStrip:
                case spv::ExecutionModeOutputLinesNV:
                    result.emplace(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
                    break;

                case spv::ExecutionModeTriangles:
                case spv::ExecutionModeQuads:
                case spv::ExecutionModeOutputTriangleStrip:
                case spv::ExecutionModeOutputTrianglesNV:
                    result.emplace(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
                    break;
            }
        }
    }

    if (is_point_mode) {
        result.emplace(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    }

    return result;
}

layer_data::optional<VkPrimitiveTopology> SHADER_MODULE_STATE::GetTopology() const {
    if (static_data_.entry_points.size() > 0) {
        const auto entrypoint = static_data_.entry_points.cbegin()->second;
        return GetTopology(get_def(entrypoint.offset));
    }
    return {};
}

SHADER_MODULE_STATE::SpirvStaticData::SpirvStaticData(const SHADER_MODULE_STATE &module_state) {
    for (auto insn : module_state) {
        const uint32_t result_word = OpcodeResultWord(insn.opcode());
        if (result_word != 0) {
            def_index[insn.word(result_word)] = insn.offset();
        }

        switch (insn.opcode()) {
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
                auto target_id = insn.word(1);
                decorations[target_id].add(insn.word(2), insn.len() > 3u ? insn.word(3) : 0u);
                decoration_inst.push_back(insn);
                if (insn.word(2) == spv::DecorationBuiltIn) {
                    builtin_decoration_list.emplace_back(insn.offset(), static_cast<spv::BuiltIn>(insn.word(3)));
                } else if (insn.word(2) == spv::DecorationSpecId) {
                    spec_const_map[insn.word(3)] = target_id;
                }

            } break;
            case spv::OpGroupDecorate: {
                auto const &src = decorations[insn.word(1)];
                for (auto i = 2u; i < insn.len(); i++) decorations[insn.word(i)].merge(src);
                has_group_decoration = true;
            } break;
            case spv::OpDecorationGroup:
            case spv::OpGroupMemberDecorate: {
                has_group_decoration = true;
            } break;
            case spv::OpMemberDecorate: {
                member_decoration_inst.push_back(insn);
                if (insn.word(3) == spv::DecorationBuiltIn) {
                    builtin_decoration_list.emplace_back(insn.offset(), static_cast<spv::BuiltIn>(insn.word(4)));
                }
            } break;

            case spv::OpCapability:
                capability_list.push_back(static_cast<spv::Capability>(insn.word(1)));
                break;

            case spv::OpVariable:
                variable_inst.push_back(insn);
                break;

            // Execution Mode
            case spv::OpExecutionMode:
            case spv::OpExecutionModeId: {
                execution_mode_inst[insn.word(1)].push_back(insn);
            } break;
            // Listed from vkspec.html#ray-tracing-repack
            case spv::OpTraceRayKHR:
            case spv::OpTraceRayMotionNV:
            case spv::OpReportIntersectionKHR:
            case spv::OpExecuteCallableKHR:
                has_invocation_repack_instruction = true;
                break;

            default:
                if (AtomicOperation(insn.opcode()) == true) {
                    // All atomics have a pointer referenced
                    spirv_inst_iter access;
                    if (insn.opcode() == spv::OpAtomicStore) {
                        access = module_state.get_def(insn.word(1));
                    } else {
                        access = module_state.get_def(insn.word(3));
                    }

                    atomic_instruction atomic;

                    auto pointer = module_state.get_def(access.word(1));
                    // spirv-val should catch if not pointer
                    assert(pointer.opcode() == spv::OpTypePointer);
                    atomic.storage_class = pointer.word(2);

                    auto data_type = module_state.get_def(pointer.word(3));
                    atomic.type = data_type.opcode();

                    // TODO - Should have a proper GetBitWidth like spirv-val does
                    assert(data_type.opcode() == spv::OpTypeFloat || data_type.opcode() == spv::OpTypeInt);
                    atomic.bit_width = data_type.word(2);

                    atomic_inst[insn.offset()] = atomic;
                }
                // We don't care about any other defs for now.
                break;
        }
    }

    entry_points = SHADER_MODULE_STATE::ProcessEntryPoints(module_state);
    multiple_entry_points = entry_points.size() > 1;
}

// static
std::unordered_multimap<std::string, SHADER_MODULE_STATE::EntryPoint> SHADER_MODULE_STATE::ProcessEntryPoints(
    const SHADER_MODULE_STATE &module_state) {
    std::unordered_multimap<std::string, SHADER_MODULE_STATE::EntryPoint> entry_points;
    function_set func_set = {};
    EntryPoint *entry_point = nullptr;

    for (auto insn : module_state) {
        // offset is not 0, it means it's updated and the offset is in a Function.
        if (func_set.offset) {
            func_set.op_lists.emplace(insn.opcode(), insn.offset());
        } else if (entry_point) {
            entry_point->decorate_list.emplace(insn.opcode(), insn.offset());
        }

        switch (insn.opcode()) {
            // Functions
            case spv::OpFunction:
                func_set.id = insn.word(2);
                func_set.offset = insn.offset();
                func_set.op_lists.clear();
                break;

            // Entry points ... add to the entrypoint table
            case spv::OpEntryPoint: {
                // Entry points do not have an id (the id is the function id) and thus need their own table
                auto entrypoint_name = reinterpret_cast<char const *>(&insn.word(3));
                auto execution_model = insn.word(1);
                auto entrypoint_stage = ExecutionModelToShaderStageFlagBits(execution_model);
                entry_points.emplace(entrypoint_name,
                                     EntryPoint{insn.offset(), static_cast<VkShaderStageFlagBits>(entrypoint_stage)});

                auto range = entry_points.equal_range(entrypoint_name);
                for (auto it = range.first; it != range.second; ++it) {
                    if (it->second.offset == insn.offset()) {
                        entry_point = &(it->second);
                        break;
                    }
                }
                assert(entry_point != nullptr);
                break;
            }
            case spv::OpFunctionEnd: {
                assert(entry_point != nullptr);
                func_set.length = insn.offset() - func_set.offset;
                entry_point->function_set_list.emplace_back(func_set);
                break;
            }
        }
    }

    SHADER_MODULE_STATE::SetPushConstantUsedInShader(module_state, entry_points);
    return entry_points;
}

void SHADER_MODULE_STATE::PreprocessShaderBinary(const spv_target_env env) {
    if (static_data_.has_group_decoration) {
        spvtools::Optimizer optimizer(env);
        optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
        std::vector<uint32_t> optimized_binary;
        // Run optimizer to flatten decorations only, set skip_validation so as to not re-run validator
        auto result = optimizer.Run(words.data(), words.size(), &optimized_binary, spvtools::ValidatorOptions(), true);

        if (result) {
            // NOTE: We need to update words with the result from the spirv-tools optimizer.
            // **THIS ONLY HAPPENS ON INITIALIZATION**. words should remain const for the lifetime
            // of the SHADER_MODULE_STATE instance.
            *const_cast<std::vector<uint32_t> *>(&words) = std::move(optimized_binary);
            // Will need to update static data now the words have changed or else the def_index will not align
            // It is really rare this will get here as Group Decorations have been deprecated and before this was added no one ever
            // raised an issue for a bug that would crash the layers that was around for many releases
            *const_cast<SpirvStaticData *>(&static_data_) = SpirvStaticData(*this);
        }
    }
}

void SHADER_MODULE_STATE::DescribeTypeInner(std::ostringstream &ss, uint32_t type) const {
    auto insn = get_def(type);
    assert(insn != end());

    switch (insn.opcode()) {
        case spv::OpTypeBool:
            ss << "bool";
            break;
        case spv::OpTypeInt:
            ss << (insn.word(3) ? 's' : 'u') << "int" << insn.word(2);
            break;
        case spv::OpTypeFloat:
            ss << "float" << insn.word(2);
            break;
        case spv::OpTypeVector:
            ss << "vec" << insn.word(3) << " of ";
            DescribeTypeInner(ss, insn.word(2));
            break;
        case spv::OpTypeMatrix:
            ss << "mat" << insn.word(3) << " of ";
            DescribeTypeInner(ss, insn.word(2));
            break;
        case spv::OpTypeArray:
            ss << "arr[" << GetConstantValueById(insn.word(3)) << "] of ";
            DescribeTypeInner(ss, insn.word(2));
            break;
        case spv::OpTypeRuntimeArray:
            ss << "runtime arr[] of ";
            DescribeTypeInner(ss, insn.word(2));
            break;
        case spv::OpTypePointer:
            ss << "ptr to " << string_SpvStorageClass(insn.word(2)) << " ";
            DescribeTypeInner(ss, insn.word(3));
            break;
        case spv::OpTypeStruct: {
            ss << "struct of (";
            for (uint32_t i = 2; i < insn.len(); i++) {
                DescribeTypeInner(ss, insn.word(i));
                if (i == insn.len() - 1) {
                    ss << ")";
                } else {
                    ss << ", ";
                }
            }
            break;
        }
        case spv::OpTypeSampler:
            ss << "sampler";
            break;
        case spv::OpTypeSampledImage:
            ss << "sampler+";
            DescribeTypeInner(ss, insn.word(2));
            break;
        case spv::OpTypeImage:
            ss << "image(dim=" << insn.word(3) << ", sampled=" << insn.word(7) << ")";
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
    DescribeTypeInner(ss, type);
    return ss.str();
}

std::string SHADER_MODULE_STATE::DescribeInstruction(const spirv_inst_iter &insn) const {
    std::ostringstream ss;
    const uint32_t opcode = insn.opcode();
    uint32_t operand_offset = 1;  // where to start printing operands
    // common disassembled for SPIR-V is
    // %result = Opcode %result_type %operands
    if (OpcodeHasResult(opcode)) {
        operand_offset++;
        ss << "%" << (OpcodeHasType(opcode) ? insn.word(2) : insn.word(1)) << " = ";
    }

    ss << string_SpvOpcode(opcode);

    if (OpcodeHasType(opcode)) {
        operand_offset++;
        ss << " %" << insn.word(1);
    }

    // TODO - For now don't list the '%' for any operands since they are only for reference IDs. Without generating a table of each
    // instructions operand types and covering the many edge cases (such as optional, paired, or variable operands) this is the
    // simplest way to print the instruction and give the developer something to look into when an error occurs.
    //
    // For now this safely should be able to assume it will never come across a LiteralString such as in OpExtInstImport or
    // OpEntryPoint
    for (uint32_t i = operand_offset; i < insn.len(); i++) {
        ss << " " << insn.word(i);
    }
    return ss.str();
}

const SHADER_MODULE_STATE::EntryPoint *SHADER_MODULE_STATE::FindEntrypointStruct(char const *name,
                                                                                 VkShaderStageFlagBits stageBits) const {
    auto range = static_data_.entry_points.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.stage == stageBits) {
            return &(it->second);
        }
    }
    return nullptr;
}

spirv_inst_iter SHADER_MODULE_STATE::FindEntrypoint(char const *name, VkShaderStageFlagBits stageBits) const {
    auto range = static_data_.entry_points.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.stage == stageBits) {
            return at(it->second.offset);
        }
    }
    return end();
}

// Because the following is legal, need the entry point
//    OpEntryPoint GLCompute %main "name_a"
//    OpEntryPoint GLCompute %main "name_b"
// Assumes shader module contains no spec constants used to set the local size values
bool SHADER_MODULE_STATE::FindLocalSize(const spirv_inst_iter &entrypoint, uint32_t &local_size_x, uint32_t &local_size_y,
                                        uint32_t &local_size_z) const {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    for (const auto &builtin : static_data_.builtin_decoration_list) {
        if (builtin.builtin == spv::BuiltInWorkgroupSize) {
            const uint32_t workgroup_size_id = at(builtin.offset).word(1);
            auto composite_def = get_def(workgroup_size_id);
            if (composite_def.opcode() == spv::OpConstantComposite) {
                // VUID-WorkgroupSize-WorkgroupSize-04427 makes sure this is a OpTypeVector of int32
                local_size_x = GetConstantValueById(composite_def.word(3));
                local_size_y = GetConstantValueById(composite_def.word(4));
                local_size_z = GetConstantValueById(composite_def.word(5));
                return true;
            }
        }
    }

    auto entrypoint_id = entrypoint.word(2);
    auto it = static_data_.execution_mode_inst.find(entrypoint_id);
    if (it != static_data_.execution_mode_inst.end()) {
        for (auto insn : it->second) {
            if (insn.opcode() == spv::OpExecutionMode && insn.word(2) == spv::ExecutionModeLocalSize) {
                local_size_x = insn.word(3);
                local_size_y = insn.word(4);
                local_size_z = insn.word(5);
                return true;
            } else if (insn.opcode() == spv::OpExecutionModeId && insn.word(2) == spv::ExecutionModeLocalSizeId) {
                local_size_x = GetConstantValueById(insn.word(3));
                local_size_y = GetConstantValueById(insn.word(4));
                local_size_z = GetConstantValueById(insn.word(5));
                return true;
            }
        }
    }
    return false;  // not found
}

// If the instruction at id is a constant or copy of a constant, returns a valid iterator pointing to that instruction.
// Otherwise, returns src->end().
spirv_inst_iter SHADER_MODULE_STATE::GetConstantDef(uint32_t id) const {
    auto value = get_def(id);

    // If id is a copy, see where it was copied from
    if ((end() != value) && ((value.opcode() == spv::OpCopyObject) || (value.opcode() == spv::OpCopyLogical))) {
        id = value.word(3);
        value = get_def(id);
    }

    if ((end() != value) && (value.opcode() == spv::OpConstant)) {
        return value;
    }
    return end();
}

// While simple, function name provides a more human readable description why word(3) is used
uint32_t SHADER_MODULE_STATE::GetConstantValue(const spirv_inst_iter &itr) const {
    // This should be a OpConstant (not a OpSpecConstant), if this asserts then 2 things are happening
    // 1. This function is being used where we don't actually know it is a constant and is a bug in the validation layers
    // 2. The CreateFoldSpecConstantOpAndCompositePass didn't fully fold everything and is a bug in spirv-opt
    assert(itr.opcode() == spv::OpConstant);
    return itr.word(3);
}

// Either returns the constant value described by the instruction at id, or 1
uint32_t SHADER_MODULE_STATE::GetConstantValueById(uint32_t id) const {
    auto value = GetConstantDef(id);

    if (end() == value) {
        // TODO: Either ensure that the specialization transform is already performed on a module we're
        //       considering here, OR -- specialize on the fly now.
        return 1;
    }

    return GetConstantValue(value);
}

// Returns an int32_t corresponding to the spv::Dim of the given resource, when positive, and corresponding to an unknown type, when
// negative.
int32_t SHADER_MODULE_STATE::GetShaderResourceDimensionality(const interface_var &resource) const {
    auto type = get_def(resource.type_id);
    while (true) {
        switch (type.opcode()) {
            case spv::OpTypeSampledImage:
                type = get_def(type.word(2));
                break;
            case spv::OpTypePointer:
                type = get_def(type.word(3));
                break;
            case spv::OpTypeImage:
                return type.word(3);
            default:
                return -1;
        }
    }
}

uint32_t SHADER_MODULE_STATE::GetLocationsConsumedByType(uint32_t type, bool strip_array_level) const {
    auto insn = get_def(type);
    assert(insn != end());

    switch (insn.opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetLocationsConsumedByType(insn.word(3), strip_array_level);
        case spv::OpTypeArray:
            if (strip_array_level) {
                return GetLocationsConsumedByType(insn.word(2), false);
            } else {
                return GetConstantValueById(insn.word(3)) * GetLocationsConsumedByType(insn.word(2), false);
            }
        case spv::OpTypeMatrix:
            // Num locations is the dimension * element size
            return insn.word(3) * GetLocationsConsumedByType(insn.word(2), false);
        case spv::OpTypeVector: {
            auto scalar_type = get_def(insn.word(2));
            auto bit_width =
                (scalar_type.opcode() == spv::OpTypeInt || scalar_type.opcode() == spv::OpTypeFloat) ? scalar_type.word(2) : 32;

            // Locations are 128-bit wide; 3- and 4-component vectors of 64 bit types require two.
            return (bit_width * insn.word(3) + 127) / 128;
        }
        default:
            // Everything else is just 1.
            return 1;

            // TODO: extend to handle 64bit scalar types, whose vectors may need multiple locations.
    }
}

uint32_t SHADER_MODULE_STATE::GetComponentsConsumedByType(uint32_t type, bool strip_array_level) const {
    auto insn = get_def(type);
    assert(insn != end());

    switch (insn.opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetComponentsConsumedByType(insn.word(3), strip_array_level);
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            for (uint32_t i = 2; i < insn.len(); i++) {  // i=2 to skip word(0) and word(1)=ID of struct
                sum += GetComponentsConsumedByType(insn.word(i), false);
            }
            return sum;
        }
        case spv::OpTypeArray:
            if (strip_array_level) {
                return GetComponentsConsumedByType(insn.word(2), false);
            } else {
                return GetConstantValueById(insn.word(3)) * GetComponentsConsumedByType(insn.word(2), false);
            }
        case spv::OpTypeMatrix:
            // Num locations is the dimension * element size
            return insn.word(3) * GetComponentsConsumedByType(insn.word(2), false);
        case spv::OpTypeVector: {
            auto scalar_type = get_def(insn.word(2));
            auto bit_width =
                (scalar_type.opcode() == spv::OpTypeInt || scalar_type.opcode() == spv::OpTypeFloat) ? scalar_type.word(2) : 32;
            // One component is 32-bit
            return (bit_width * insn.word(3) + 31) / 32;
        }
        case spv::OpTypeFloat: {
            auto bit_width = insn.word(2);
            return (bit_width + 31) / 32;
        }
        case spv::OpTypeInt: {
            auto bit_width = insn.word(2);
            return (bit_width + 31) / 32;
        }
        case spv::OpConstant:
            return GetComponentsConsumedByType(insn.word(1), false);
        default:
            return 0;
    }
}

// characterizes a SPIR-V type appearing in an interface to a FF stage, for comparison to a VkFormat's characterization above.
// also used for input attachments, as we statically know their format.
uint32_t SHADER_MODULE_STATE::GetFundamentalType(uint32_t type) const {
    auto insn = get_def(type);
    assert(insn != end());

    switch (insn.opcode()) {
        case spv::OpTypeInt:
            return insn.word(3) ? FORMAT_TYPE_SINT : FORMAT_TYPE_UINT;
        case spv::OpTypeFloat:
            return FORMAT_TYPE_FLOAT;
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeImage:
            return GetFundamentalType(insn.word(2));
        case spv::OpTypePointer:
            return GetFundamentalType(insn.word(3));

        default:
            return 0;
    }
}

spirv_inst_iter SHADER_MODULE_STATE::GetStructType(spirv_inst_iter def, bool is_array_of_verts) const {
    while (true) {
        if (def.opcode() == spv::OpTypePointer) {
            def = get_def(def.word(3));
        } else if (def.opcode() == spv::OpTypeArray && is_array_of_verts) {
            def = get_def(def.word(2));
            is_array_of_verts = false;
        } else if (def.opcode() == spv::OpTypeStruct) {
            return def;
        } else {
            return end();
        }
    }
}

void SHADER_MODULE_STATE::DefineStructMember(const spirv_inst_iter &it, const std::vector<uint32_t> &member_decorate_offsets,
                                             shader_struct_member &data) const {
    const auto struct_it = GetStructType(it, false);
    assert(struct_it != end());
    data.size = 0;

    shader_struct_member data1;
    uint32_t i = 2;
    uint32_t local_offset = 0;
    std::vector<uint32_t> offsets;
    offsets.resize(struct_it.len() - i);

    // The members of struct in SPRIV_R aren't always sort, so we need to know their order.
    for (const auto offset : member_decorate_offsets) {
        const auto member_decorate = at(offset);
        if (member_decorate.word(1) != struct_it.word(1)) {
            continue;
        }

        offsets[member_decorate.word(2)] = member_decorate.word(4);
    }

    for (const auto offset : offsets) {
        local_offset = offset;
        data1 = {};
        data1.root = data.root;
        data1.offset = local_offset;
        auto def_member = get_def(struct_it.word(i));

        // Array could be multi-dimensional
        while (def_member.opcode() == spv::OpTypeArray) {
            const auto len_id = def_member.word(3);
            const auto def_len = get_def(len_id);
            data1.array_length_hierarchy.emplace_back(def_len.word(3));  // array length
            def_member = get_def(def_member.word(2));
        }

        if (def_member.opcode() == spv::OpTypeStruct) {
            DefineStructMember(def_member, member_decorate_offsets, data1);
        } else if (def_member.opcode() == spv::OpTypePointer) {
            if (def_member.word(2) == spv::StorageClassPhysicalStorageBuffer) {
                // If it's a pointer with PhysicalStorageBuffer class, this member is essentially a uint64_t containing an address
                // that "points to something."
                data1.size = 8;
            } else {
                // If it's OpTypePointer. it means the member is a buffer, the type will be TypePointer, and then struct
                DefineStructMember(def_member, member_decorate_offsets, data1);
            }
        } else {
            if (def_member.opcode() == spv::OpTypeMatrix) {
                data1.array_length_hierarchy.emplace_back(def_member.word(3));  // matrix's columns. matrix's row is vector.
                def_member = get_def(def_member.word(2));
            }

            if (def_member.opcode() == spv::OpTypeVector) {
                data1.array_length_hierarchy.emplace_back(def_member.word(3));  // vector length
                def_member = get_def(def_member.word(2));
            }

            // Get scalar type size. The value in SPRV-R is bit. It needs to translate to byte.
            data1.size = (def_member.word(2) / 8);
        }
        const auto array_length_hierarchy_szie = data1.array_length_hierarchy.size();
        if (array_length_hierarchy_szie > 0) {
            data1.array_block_size.resize(array_length_hierarchy_szie, 1);

            for (int i2 = static_cast<int>(array_length_hierarchy_szie - 1); i2 > 0; --i2) {
                data1.array_block_size[i2 - 1] = data1.array_length_hierarchy[i2] * data1.array_block_size[i2];
            }
        }
        data.struct_members.emplace_back(data1);
        ++i;
    }
    uint32_t total_array_length = 1;
    for (const auto length : data1.array_length_hierarchy) {
        total_array_length *= length;
    }
    data.size = local_offset + data1.size * total_array_length;
}

static uint32_t UpdateOffset(uint32_t offset, const std::vector<uint32_t> &array_indices, const shader_struct_member &data) {
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

static void SetUsedBytes(uint32_t offset, const std::vector<uint32_t> &array_indices, const shader_struct_member &data) {
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
                                       spirv_inst_iter &access_chain_it, const shader_struct_member &data) const {
    if (access_chain_word_index < access_chain_it.len()) {
        if (data.array_length_hierarchy.size() > array_indices.size()) {
            auto def_it = get_def(access_chain_it.word(access_chain_word_index));
            ++access_chain_word_index;

            if (def_it != end() && def_it.opcode() == spv::OpConstant) {
                array_indices.emplace_back(def_it.word(3));
                RunUsedArray(offset, array_indices, access_chain_word_index, access_chain_it, data);
            } else {
                // If it is a variable, set the all array is used.
                if (access_chain_word_index < access_chain_it.len()) {
                    uint32_t array_length = data.array_length_hierarchy[array_indices.size()];
                    for (uint32_t i = 0; i < array_length; ++i) {
                        auto array_indices2 = array_indices;
                        array_indices2.emplace_back(i);
                        RunUsedArray(offset, array_indices2, access_chain_word_index, access_chain_it, data);
                    }
                } else {
                    SetUsedBytes(offset, array_indices, data);
                }
            }
        } else {
            offset = UpdateOffset(offset, array_indices, data);
            RunUsedStruct(offset, access_chain_word_index, access_chain_it, data);
        }
    } else {
        SetUsedBytes(offset, array_indices, data);
    }
}

void SHADER_MODULE_STATE::RunUsedStruct(uint32_t offset, uint32_t access_chain_word_index, spirv_inst_iter &access_chain_it,
                                        const shader_struct_member &data) const {
    std::vector<uint32_t> array_indices_emptry;

    if (access_chain_word_index < access_chain_it.len()) {
        auto strcut_member_index = GetConstantValueById(access_chain_it.word(access_chain_word_index));
        ++access_chain_word_index;

        auto data1 = data.struct_members[strcut_member_index];
        RunUsedArray(offset + data1.offset, array_indices_emptry, access_chain_word_index, access_chain_it, data1);
    }
}

void SHADER_MODULE_STATE::SetUsedStructMember(const uint32_t variable_id, const std::vector<function_set> &function_set_list,
                                              const shader_struct_member &data) const {
    for (const auto &func_set : function_set_list) {
        auto range = func_set.op_lists.equal_range(spv::OpAccessChain);
        for (auto it = range.first; it != range.second; ++it) {
            auto access_chain = at(it->second);
            if (access_chain.word(3) == variable_id) {
                RunUsedStruct(0, 4, access_chain, data);
            }
        }
    }
}

// static
void SHADER_MODULE_STATE::SetPushConstantUsedInShader(
    const SHADER_MODULE_STATE &module_state, std::unordered_multimap<std::string, SHADER_MODULE_STATE::EntryPoint> &entry_points) {
    for (auto &entrypoint : entry_points) {
        auto range = entrypoint.second.decorate_list.equal_range(spv::OpVariable);
        for (auto it = range.first; it != range.second; ++it) {
            const auto def_insn = module_state.at(it->second);

            if (def_insn.word(3) == spv::StorageClassPushConstant) {
                spirv_inst_iter type = module_state.get_def(def_insn.word(1));
                const auto range2 = entrypoint.second.decorate_list.equal_range(spv::OpMemberDecorate);
                std::vector<uint32_t> offsets;

                for (auto it2 = range2.first; it2 != range2.second; ++it2) {
                    auto member_decorate = module_state.at(it2->second);
                    if (member_decorate.len() == 5 && member_decorate.word(3) == spv::DecorationOffset) {
                        offsets.emplace_back(member_decorate.offset());
                    }
                }
                entrypoint.second.push_constant_used_in_shader.root = &entrypoint.second.push_constant_used_in_shader;
                module_state.DefineStructMember(type, offsets, entrypoint.second.push_constant_used_in_shader);
                module_state.SetUsedStructMember(def_insn.word(2), entrypoint.second.function_set_list,
                                                 entrypoint.second.push_constant_used_in_shader);
            }
        }
    }
}

uint32_t SHADER_MODULE_STATE::DescriptorTypeToReqs(uint32_t type_id) const {
    auto type = get_def(type_id);

    while (true) {
        switch (type.opcode()) {
            case spv::OpTypeArray:
            case spv::OpTypeRuntimeArray:
            case spv::OpTypeSampledImage:
                type = get_def(type.word(2));
                break;
            case spv::OpTypePointer:
                type = get_def(type.word(3));
                break;
            case spv::OpTypeImage: {
                auto dim = type.word(3);
                auto arrayed = type.word(5);
                auto msaa = type.word(6);

                uint32_t bits = 0;
                switch (GetFundamentalType(type.word(2))) {
                    case FORMAT_TYPE_FLOAT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
                        break;
                    case FORMAT_TYPE_UINT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
                        break;
                    case FORMAT_TYPE_SINT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT;
                        break;
                    default:
                        break;
                }

                switch (dim) {
                    case spv::Dim1D:
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_1D;
                        return bits;
                    case spv::Dim2D:
                        bits |= msaa ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_2D;
                        return bits;
                    case spv::Dim3D:
                        bits |= DESCRIPTOR_REQ_VIEW_TYPE_3D;
                        return bits;
                    case spv::DimCube:
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_CUBE;
                        return bits;
                    case spv::DimSubpassData:
                        bits |= msaa ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                        return bits;
                    default:  // buffer, etc.
                        return bits;
                }
            }
            default:
                return 0;
        }
    }
}

// For some built-in analysis we need to know if the variable decorated with as the built-in was actually written to.
// This function examines instructions in the static call tree for a write to this variable.
bool SHADER_MODULE_STATE::IsBuiltInWritten(spirv_inst_iter builtin_instr, spirv_inst_iter entrypoint) const {
    auto type = builtin_instr.opcode();
    uint32_t target_id = builtin_instr.word(1);
    bool init_complete = false;
    uint32_t target_member_offset = 0;

    if (type == spv::OpMemberDecorate) {
        // Built-in is part of a structure -- examine instructions up to first function body to get initial IDs
        auto insn = entrypoint;
        while (!init_complete && (insn.opcode() != spv::OpFunction)) {
            switch (insn.opcode()) {
                case spv::OpTypePointer:
                    if (insn.word(2) == spv::StorageClassOutput) {
                        const auto type_id = insn.word(3);
                        if (type_id == target_id) {
                            target_id = insn.word(1);
                        } else {
                            // If the output is an array, check if the element type is what we're looking for
                            const auto type_insn = get_def(type_id);
                            if ((type_insn.opcode() == spv::OpTypeArray) && (type_insn.word(2) == target_id)) {
                                target_id = insn.word(1);
                                target_member_offset = 1;
                            }
                        }
                    }
                    break;
                case spv::OpVariable:
                    if (insn.word(1) == target_id) {
                        target_id = insn.word(2);
                        init_complete = true;
                    }
                    break;
            }
            insn++;
        }
    }

    if (!init_complete && (type == spv::OpMemberDecorate)) return false;

    bool found_write = false;
    layer_data::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.word(2));

    // Follow instructions in call graph looking for writes to target
    while (!worklist.empty() && !found_write) {
        auto id_iter = worklist.begin();
        auto id = *id_iter;
        worklist.erase(id_iter);

        auto insn = get_def(id);
        if (insn == end()) {
            continue;
        }

        if (insn.opcode() == spv::OpFunction) {
            // Scan body of function looking for other function calls or items in our ID chain
            while (++insn, (insn.opcode() != spv::OpFunctionEnd) && !found_write) {
                switch (insn.opcode()) {
                    case spv::OpAccessChain:
                    case spv::OpInBoundsAccessChain:
                        if (insn.word(3) == target_id) {
                            if (type == spv::OpMemberDecorate) {
                                // Get the target member of the struct
                                // NOTE: this will only work for structs and arrays of structs. Deeper levels of nesting (e.g.,
                                // arrays of structs of structs) is not currently supported.
                                const auto value_itr = GetConstantDef(insn.word(4 + target_member_offset));
                                if (value_itr != end()) {
                                    auto value = GetConstantValue(value_itr);
                                    if (value == builtin_instr.word(2)) {
                                        target_id = insn.word(2);
                                    }
                                }
                            } else {
                                target_id = insn.word(2);
                            }
                        }
                        break;
                    case spv::OpStore:
                        if (insn.word(1) == target_id) {
                            found_write = true;
                        }
                        break;
                    case spv::OpFunctionCall:
                        worklist.insert(insn.word(3));
                        break;
                }
            }
        }
    }
    return found_write;
}

// Used by the collection functions to help aid in state tracking
struct shader_module_used_operators {
    bool updated;
    std::vector<uint32_t> image_read_load_ids;
    std::vector<uint32_t> image_write_load_ids;
    std::vector<uint32_t> atomic_pointer_ids;
    std::vector<uint32_t> store_pointer_ids;
    std::vector<uint32_t> atomic_store_pointer_ids;
    std::vector<uint32_t> sampler_load_ids;  // tracks all sampling operations
    std::vector<uint32_t> sampler_implicitLod_dref_proj_load_ids;
    std::vector<uint32_t> sampler_bias_offset_load_ids;
    std::vector<uint32_t> image_dref_load_ids;
    std::vector<std::pair<uint32_t, uint32_t>> sampled_image_load_ids;                       // <image, sampler>
    layer_data::unordered_map<uint32_t, uint32_t> load_members;                              // <result id, pointer>
    layer_data::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> accesschain_members;  // <result id, <base,index[0]>>
    layer_data::unordered_map<uint32_t, uint32_t> image_texel_pointer_members;               // <result id, image>

    shader_module_used_operators() : updated(false) {}

    bool CheckImageOperandsBiasOffset(uint32_t type) {
        return type & (spv::ImageOperandsBiasMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask |
                       spv::ImageOperandsConstOffsetsMask)
                   ? true
                   : false;
    }

    void update(SHADER_MODULE_STATE const *module_state) {
        if (updated) return;
        updated = true;

        for (auto insn : *module_state) {
            switch (insn.opcode()) {
                case spv::OpImageSampleImplicitLod:
                case spv::OpImageSampleProjImplicitLod:
                case spv::OpImageSampleProjExplicitLod:
                case spv::OpImageSparseSampleImplicitLod:
                case spv::OpImageSparseSampleProjImplicitLod:
                case spv::OpImageSparseSampleProjExplicitLod: {
                    // combined image samples are just OpLoad, but also can be separate image and sampler
                    auto id = module_state->get_def(insn.word(3));  // <id> Sampled Image
                    auto load_id = (id.opcode() == spv::OpSampledImage) ? id.word(4) : insn.word(3);
                    sampler_load_ids.emplace_back(load_id);
                    sampler_implicitLod_dref_proj_load_ids.emplace_back(load_id);
                    // ImageOperands in index: 5
                    if (insn.len() > 5 && CheckImageOperandsBiasOffset(insn.word(5))) {
                        sampler_bias_offset_load_ids.emplace_back(load_id);
                    }
                    break;
                }
                case spv::OpImageDrefGather:
                case spv::OpImageSparseDrefGather: {
                    // combined image samples are just OpLoad, but also can be separate image and sampler
                    auto id = module_state->get_def(insn.word(3));  // <id> Sampled Image
                    auto load_id = (id.opcode() == spv::OpSampledImage) ? id.word(3) : insn.word(3);
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
                    auto id = module_state->get_def(insn.word(3));  // <id> Sampled Image
                    auto sampler_load_id = (id.opcode() == spv::OpSampledImage) ? id.word(4) : insn.word(3);
                    auto image_load_id = (id.opcode() == spv::OpSampledImage) ? id.word(3) : insn.word(3);

                    image_dref_load_ids.emplace_back(image_load_id);
                    sampler_load_ids.emplace_back(sampler_load_id);
                    sampler_implicitLod_dref_proj_load_ids.emplace_back(sampler_load_id);
                    // ImageOperands in index: 6
                    if (insn.len() > 6 && CheckImageOperandsBiasOffset(insn.word(6))) {
                        sampler_bias_offset_load_ids.emplace_back(sampler_load_id);
                    }
                    break;
                }
                case spv::OpImageSampleExplicitLod:
                case spv::OpImageSparseSampleExplicitLod: {
                    // ImageOperands in index: 5
                    if (insn.len() > 5 && CheckImageOperandsBiasOffset(insn.word(5))) {
                        // combined image samples are just OpLoad, but also can be separate image and sampler
                        auto id = module_state->get_def(insn.word(3));  // <id> Sampled Image
                        auto load_id = (id.opcode() == spv::OpSampledImage) ? id.word(4) : insn.word(3);
                        sampler_load_ids.emplace_back(load_id);
                        sampler_bias_offset_load_ids.emplace_back(load_id);
                    }
                    break;
                }
                case spv::OpStore: {
                    store_pointer_ids.emplace_back(insn.word(1));  // object id or AccessChain id
                    break;
                }
                case spv::OpImageRead:
                case spv::OpImageSparseRead: {
                    image_read_load_ids.emplace_back(insn.word(3));
                    break;
                }
                case spv::OpImageWrite: {
                    image_write_load_ids.emplace_back(insn.word(1));
                    break;
                }
                case spv::OpSampledImage: {
                    // 3: image load id, 4: sampler load id
                    sampled_image_load_ids.emplace_back(std::pair<uint32_t, uint32_t>(insn.word(3), insn.word(4)));
                    break;
                }
                case spv::OpLoad: {
                    // 2: Load id, 3: object id or AccessChain id
                    load_members.emplace(insn.word(2), insn.word(3));
                    break;
                }
                case spv::OpAccessChain:
                case spv::OpInBoundsAccessChain: {
                    if (insn.len() == 4) {
                        // If it is for struct, the length is only 4.
                        // 2: AccessChain id, 3: object id
                        accesschain_members.emplace(insn.word(2), std::pair<uint32_t, uint32_t>(insn.word(3), 0));
                    } else {
                        // 2: AccessChain id, 3: object id, 4: object id of array index
                        accesschain_members.emplace(insn.word(2), std::pair<uint32_t, uint32_t>(insn.word(3), insn.word(4)));
                    }
                    break;
                }
                case spv::OpImageTexelPointer: {
                    // 2: ImageTexelPointer id, 3: object id
                    image_texel_pointer_members.emplace(insn.word(2), insn.word(3));
                    break;
                }
                default: {
                    if (AtomicOperation(insn.opcode())) {
                        if (insn.opcode() == spv::OpAtomicStore) {
                            atomic_store_pointer_ids.emplace_back(insn.word(1));
                            atomic_pointer_ids.emplace_back(insn.word(1));
                        } else {
                            atomic_pointer_ids.emplace_back(insn.word(3));
                        }
                    }
                    break;
                }
            }
        }
    }
};

static bool CheckObjectIDFromOpLoad(uint32_t object_id, const std::vector<uint32_t> &operator_members,
                                    const layer_data::unordered_map<uint32_t, uint32_t> &load_members,
                                    const layer_data::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> &accesschain_members) {
    for (auto load_id : operator_members) {
        if (object_id == load_id) return true;
        auto load_it = load_members.find(load_id);
        if (load_it == load_members.end()) {
            continue;
        }
        if (load_it->second == object_id) {
            return true;
        }

        auto accesschain_it = accesschain_members.find(load_it->second);
        if (accesschain_it == accesschain_members.end()) {
            continue;
        }
        if (accesschain_it->second.first == object_id) {
            return true;
        }
    }
    return false;
}

// Takes a OpVariable and looks at the the descriptor type it uses. This will find things such as if the variable is writable, image
// atomic operation, matching images to samplers, etc
void SHADER_MODULE_STATE::IsSpecificDescriptorType(const spirv_inst_iter &id_it, bool is_storage_buffer, bool is_check_writable,
                                                   interface_var &out_interface_var) const {
    shader_module_used_operators used_operators;
    uint32_t type_id = id_it.word(1);
    uint32_t id = id_it.word(2);

    auto type = get_def(type_id);

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type.opcode() == spv::OpTypeArray || type.opcode() == spv::OpTypePointer || type.opcode() == spv::OpTypeRuntimeArray ||
           type.opcode() == spv::OpTypeSampledImage) {
        if (type.opcode() == spv::OpTypeArray || type.opcode() == spv::OpTypeRuntimeArray ||
            type.opcode() == spv::OpTypeSampledImage) {
            type = get_def(type.word(2));  // Element type
        } else {
            type = get_def(type.word(3));  // Pointer type
        }
    }

    switch (type.opcode()) {
        case spv::OpTypeImage: {
            auto dim = type.word(3);
            if (dim != spv::DimSubpassData) {
                used_operators.update(this);

                // Sampled == 2 indicates used without a sampler (a storage image)
                bool is_image_without_format = false;
                if (type.word(7) == 2) is_image_without_format = type.word(8) == spv::ImageFormatUnknown;

                if (CheckObjectIDFromOpLoad(id, used_operators.image_write_load_ids, used_operators.load_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_writable = true;
                    if (is_image_without_format) out_interface_var.is_write_without_format = true;
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.image_read_load_ids, used_operators.load_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_readable = true;
                    if (is_image_without_format) out_interface_var.is_read_without_format = true;
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.sampler_load_ids, used_operators.load_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_sampler_sampled = true;
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.sampler_implicitLod_dref_proj_load_ids, used_operators.load_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_sampler_implicitLod_dref_proj = true;
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.sampler_bias_offset_load_ids, used_operators.load_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_sampler_bias_offset = true;
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.atomic_pointer_ids, used_operators.image_texel_pointer_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_atomic_operation = true;
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.image_dref_load_ids, used_operators.load_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_dref_operation = true;
                }

                for (auto &itp_id : used_operators.sampled_image_load_ids) {
                    // Find if image id match.
                    uint32_t image_index = 0;
                    auto load_it = used_operators.load_members.find(itp_id.first);
                    if (load_it == used_operators.load_members.end()) {
                        continue;
                    } else {
                        if (load_it->second != id) {
                            auto accesschain_it = used_operators.accesschain_members.find(load_it->second);
                            if (accesschain_it == used_operators.accesschain_members.end()) {
                                continue;
                            } else {
                                if (accesschain_it->second.first != id) {
                                    continue;
                                }

                                const auto const_itr = GetConstantDef(accesschain_it->second.second);
                                if (const_itr == end()) {
                                    // access chain index not a constant, skip.
                                    break;
                                }
                                image_index = GetConstantValue(const_itr);
                            }
                        }
                    }
                    // Find sampler's set binding.
                    load_it = used_operators.load_members.find(itp_id.second);
                    if (load_it == used_operators.load_members.end()) {
                        continue;
                    } else {
                        uint32_t sampler_id = load_it->second;
                        uint32_t sampler_index = 0;
                        auto accesschain_it = used_operators.accesschain_members.find(load_it->second);

                        if (accesschain_it != used_operators.accesschain_members.end()) {
                            const auto const_itr = GetConstantDef(accesschain_it->second.second);
                            if (const_itr == end()) {
                                // access chain index representing sampler index is not a constant, skip.
                                break;
                            }
                            sampler_id = const_itr.offset();
                            sampler_index = GetConstantValue(const_itr);
                        }
                        auto sampler_dec = get_decorations(sampler_id);
                        if (image_index >= out_interface_var.samplers_used_by_image.size()) {
                            out_interface_var.samplers_used_by_image.resize(image_index + 1);
                        }

                        // Need to check again for these properties in case not using a combined image sampler
                        if (CheckObjectIDFromOpLoad(sampler_id, used_operators.sampler_load_ids, used_operators.load_members,
                                                    used_operators.accesschain_members)) {
                            out_interface_var.is_sampler_sampled = true;
                        }
                        if (CheckObjectIDFromOpLoad(sampler_id, used_operators.sampler_implicitLod_dref_proj_load_ids,
                                                    used_operators.load_members, used_operators.accesschain_members)) {
                            out_interface_var.is_sampler_implicitLod_dref_proj = true;
                        }
                        if (CheckObjectIDFromOpLoad(sampler_id, used_operators.sampler_bias_offset_load_ids,
                                                    used_operators.load_members, used_operators.accesschain_members)) {
                            out_interface_var.is_sampler_bias_offset = true;
                        }

                        out_interface_var.samplers_used_by_image[image_index].emplace(
                            SamplerUsedByImage{DescriptorSlot{sampler_dec.descriptor_set, sampler_dec.binding}, sampler_index});
                    }
                }
            }
            return;
        }

        case spv::OpTypeStruct: {
            layer_data::unordered_set<uint32_t> nonwritable_members;
            if (get_decorations(type.word(1)).flags & decoration_set::buffer_block_bit) is_storage_buffer = true;
            for (auto insn : static_data_.member_decoration_inst) {
                if (insn.word(1) == type.word(1) && insn.word(3) == spv::DecorationNonWritable) {
                    nonwritable_members.insert(insn.word(2));
                }
            }

            // A buffer is writable if it's either flavor of storage buffer, and has any member not decorated
            // as nonwritable.
            if (is_storage_buffer && nonwritable_members.size() != type.len() - 2) {
                used_operators.update(this);

                for (auto oid : used_operators.store_pointer_ids) {
                    if (id == oid) {
                        out_interface_var.is_writable = true;
                        return;
                    }
                    auto accesschain_it = used_operators.accesschain_members.find(oid);
                    if (accesschain_it == used_operators.accesschain_members.end()) {
                        continue;
                    }
                    if (accesschain_it->second.first == id) {
                        out_interface_var.is_writable = true;
                        return;
                    }
                }
                if (CheckObjectIDFromOpLoad(id, used_operators.atomic_store_pointer_ids, used_operators.image_texel_pointer_members,
                                            used_operators.accesschain_members)) {
                    out_interface_var.is_writable = true;
                    return;
                }
            }
        }
    }
}

std::vector<std::pair<DescriptorSlot, interface_var>> SHADER_MODULE_STATE::CollectInterfaceByDescriptorSlot(
    layer_data::unordered_set<uint32_t> const &accessible_ids) const {
    std::vector<std::pair<DescriptorSlot, interface_var>> out;

    for (auto id : accessible_ids) {
        auto insn = get_def(id);
        assert(insn != end());

        if (insn.opcode() == spv::OpVariable &&
            (insn.word(3) == spv::StorageClassUniform ||
             insn.word(3) == spv::StorageClassUniformConstant ||
             insn.word(3) == spv::StorageClassStorageBuffer)) {
            auto d = get_decorations(insn.word(2));
            uint32_t set = d.descriptor_set;
            uint32_t binding = d.binding;

            interface_var v = {};
            v.id = insn.word(2);
            v.type_id = insn.word(1);

            IsSpecificDescriptorType(insn, insn.word(3) == spv::StorageClassStorageBuffer,
                                     !(d.flags & decoration_set::nonwritable_bit), v);
            out.emplace_back(DescriptorSlot{set, binding}, v);
        }
    }

    return out;
}

layer_data::unordered_set<uint32_t> SHADER_MODULE_STATE::CollectWritableOutputLocationinFS(
    const spirv_inst_iter &entrypoint) const {
    layer_data::unordered_set<uint32_t> location_list;
    const auto outputs = CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, false);
    layer_data::unordered_set<uint32_t> store_pointer_ids;
    layer_data::unordered_map<uint32_t, uint32_t> accesschain_members;

    for (auto insn : *this) {
        switch (insn.opcode()) {
            case spv::OpStore:
            case spv::OpAtomicStore: {
                store_pointer_ids.insert(insn.word(1));  // object id or AccessChain id
                break;
            }
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain: {
                // 2: AccessChain id, 3: object id
                if (insn.word(3)) accesschain_members.emplace(insn.word(2), insn.word(3));
                break;
            }
            default:
                break;
        }
    }
    if (store_pointer_ids.empty()) {
        return location_list;
    }
    for (auto output : outputs) {
        auto store_it = store_pointer_ids.find(output.second.id);
        if (store_it != store_pointer_ids.end()) {
            location_list.insert(output.first.first);
            store_pointer_ids.erase(store_it);
            continue;
        }
        store_it = store_pointer_ids.begin();
        while (store_it != store_pointer_ids.end()) {
            auto accesschain_it = accesschain_members.find(*store_it);
            if (accesschain_it == accesschain_members.end()) {
                ++store_it;
                continue;
            }
            if (accesschain_it->second == output.second.id) {
                location_list.insert(output.first.first);
                store_pointer_ids.erase(store_it);
                accesschain_members.erase(accesschain_it);
                break;
            }
            ++store_it;
        }
    }
    return location_list;
}

bool SHADER_MODULE_STATE::CollectInterfaceBlockMembers(std::map<location_t, interface_var> *out, bool is_array_of_verts,
                                                       uint32_t id, uint32_t type_id, bool is_patch,
                                                       uint32_t /*first_location*/) const {
    // Walk down the type_id presented, trying to determine whether it's actually an interface block.
    auto type = GetStructType(get_def(type_id), is_array_of_verts && !is_patch);
    if (type == end() || !(get_decorations(type.word(1)).flags & decoration_set::block_bit)) {
        // This isn't an interface block.
        return false;
    }

    layer_data::unordered_map<uint32_t, uint32_t> member_components;
    layer_data::unordered_map<uint32_t, uint32_t> member_relaxed_precision;
    layer_data::unordered_map<uint32_t, uint32_t> member_patch;

    // Walk all the OpMemberDecorate for type's result id -- first pass, collect components.
    for (auto insn : static_data_.member_decoration_inst) {
        if (insn.word(1) == type.word(1)) {
            uint32_t member_index = insn.word(2);

            if (insn.word(3) == spv::DecorationComponent) {
                uint32_t component = insn.word(4);
                member_components[member_index] = component;
            }

            if (insn.word(3) == spv::DecorationRelaxedPrecision) {
                member_relaxed_precision[member_index] = 1;
            }

            if (insn.word(3) == spv::DecorationPatch) {
                member_patch[member_index] = 1;
            }
        }
    }

    // TODO: correctly handle location assignment from outside

    // Second pass -- produce the output, from Location decorations
    for (auto insn : static_data_.member_decoration_inst) {
        if (insn.word(1) == type.word(1)) {
            uint32_t member_index = insn.word(2);
            uint32_t member_type_id = type.word(2 + member_index);

            if (insn.word(3) == spv::DecorationLocation) {
                uint32_t location = insn.word(4);
                uint32_t num_locations = GetLocationsConsumedByType(member_type_id, false);
                auto component_it = member_components.find(member_index);
                uint32_t component = component_it == member_components.end() ? 0 : component_it->second;
                bool is_relaxed_precision = member_relaxed_precision.find(member_index) != member_relaxed_precision.end();
                bool member_is_patch = is_patch || member_patch.count(member_index) > 0;

                for (uint32_t offset = 0; offset < num_locations; offset++) {
                    interface_var v = {};
                    v.id = id;
                    // TODO: member index in interface_var too?
                    v.type_id = member_type_id;
                    v.offset = offset;
                    v.is_patch = member_is_patch;
                    v.is_block_member = true;
                    v.is_relaxed_precision = is_relaxed_precision;
                    (*out)[std::make_pair(location + offset, component)] = v;
                }
            }
        }
    }

    return true;
}

std::map<location_t, interface_var> SHADER_MODULE_STATE::CollectInterfaceByLocation(spirv_inst_iter entrypoint,
                                                                                    spv::StorageClass sinterface,
                                                                                    bool is_array_of_verts) const {
    // TODO: handle index=1 dual source outputs from FS -- two vars will have the same location, and we DON'T want to clobber.

    std::map<location_t, interface_var> out;

    for (uint32_t iid : FindEntrypointInterfaces(entrypoint)) {
        auto insn = get_def(iid);
        assert(insn != end());
        assert(insn.opcode() == spv::OpVariable);

        const auto d = get_decorations(iid);
        bool passthrough = sinterface == spv::StorageClassOutput && insn.word(3) == spv::StorageClassInput &&
                           (d.flags & decoration_set::passthrough_bit) != 0;
        if (insn.word(3) == static_cast<uint32_t>(sinterface) || passthrough) {
            uint32_t id = insn.word(2);
            uint32_t type = insn.word(1);

            auto location = d.location;
            int builtin = d.builtin;
            uint32_t component = d.component;
            bool is_patch = (d.flags & decoration_set::patch_bit) != 0;
            bool is_relaxed_precision = (d.flags & decoration_set::relaxed_precision_bit) != 0;
            bool is_per_vertex = (d.flags & decoration_set::per_vertex_bit) != 0;

            if (builtin != -1) {
                continue;
            } else if (!CollectInterfaceBlockMembers(&out, is_array_of_verts, id, type, is_patch, location) ||
                       location != decoration_set::kInvalidValue) {
                // A user-defined interface variable, with a location. Where a variable occupied multiple locations, emit
                // one result for each.
                uint32_t num_locations = GetLocationsConsumedByType(type, (is_array_of_verts && !is_patch) || is_per_vertex);
                for (uint32_t offset = 0; offset < num_locations; offset++) {
                    interface_var v = {};
                    v.id = id;
                    v.type_id = type;
                    v.offset = offset;
                    v.is_patch = is_patch;
                    v.is_relaxed_precision = is_relaxed_precision;
                    out[std::make_pair(location + offset, component)] = v;
                }
            }
        }
    }

    return out;
}

std::vector<uint32_t> SHADER_MODULE_STATE::CollectBuiltinBlockMembers(spirv_inst_iter entrypoint, uint32_t storageClass) const {
    // Find all interface variables belonging to the entrypoint and matching the storage class
    std::vector<uint32_t> variables;
    for (uint32_t id : FindEntrypointInterfaces(entrypoint)) {
        auto def = get_def(id);
        assert(def != end());
        assert(def.opcode() == spv::OpVariable);

        if (def.word(3) == storageClass) variables.push_back(def.word(1));
    }

    // Find all members belonging to the builtin block selected
    std::vector<uint32_t> builtin_block_members;
    for (auto &var : variables) {
        auto def = get_def(get_def(var).word(3));

        // It could be an array of IO blocks. The element type should be the struct defining the block contents
        if (def.opcode() == spv::OpTypeArray) def = get_def(def.word(2));

        // Now find all members belonging to the struct defining the IO block
        if (def.opcode() == spv::OpTypeStruct) {
            for (auto set : static_data_.builtin_decoration_list) {
                auto insn = at(set.offset);
                if ((insn.opcode() == spv::OpMemberDecorate) && (def.word(1) == insn.word(1))) {
                    // Start with undefined builtin for each struct member.
                    // But only when confirmed the struct is the built-in inteface block (can only be one per shader)
                    if (builtin_block_members.size() == 0) {
                        builtin_block_members.resize(def.len() - 2, spv::BuiltInMax);
                    }
                    auto struct_index = insn.word(2);
                    assert(struct_index < builtin_block_members.size());
                    builtin_block_members[struct_index] = insn.word(4);
                }
            }
        }
    }

    return builtin_block_members;
}

std::vector<std::pair<uint32_t, interface_var>> SHADER_MODULE_STATE::CollectInterfaceByInputAttachmentIndex(
    layer_data::unordered_set<uint32_t> const &accessible_ids) const {
    std::vector<std::pair<uint32_t, interface_var>> out;

    for (auto insn : static_data_.decoration_inst) {
        if (insn.word(2) == spv::DecorationInputAttachmentIndex) {
            auto attachment_index = insn.word(3);
            auto id = insn.word(1);

            if (accessible_ids.count(id)) {
                auto def = get_def(id);
                assert(def != end());
                if (def.opcode() == spv::OpVariable && def.word(3) == spv::StorageClassUniformConstant) {
                    auto num_locations = GetLocationsConsumedByType(def.word(1), false);
                    for (uint32_t offset = 0; offset < num_locations; offset++) {
                        interface_var v = {};
                        v.id = id;
                        v.type_id = def.word(1);
                        v.offset = offset;
                        out.emplace_back(attachment_index + offset, v);
                    }
                }
            }
        }
    }

    return out;
}

uint32_t SHADER_MODULE_STATE::GetNumComponentsInBaseType(const spirv_inst_iter &iter) const {
    const uint32_t opcode = iter.opcode();
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt) {
        return 1;
    } else if (opcode == spv::OpTypeVector) {
        const uint32_t component_count = iter.word(3);
        return component_count;
    } else if (opcode == spv::OpTypeMatrix) {
        const auto column_type = get_def(iter.word(2));
        const uint32_t vector_length = GetNumComponentsInBaseType(column_type);
        // Because we are calculating components for a single location we do not care about column count
        return vector_length;
    } else if (opcode == spv::OpTypeArray) {
        const auto element_type = get_def(iter.word(2));
        const uint32_t element_length = GetNumComponentsInBaseType(element_type);
        return element_length;
    } else if (opcode == spv::OpTypeStruct) {
        uint32_t total_size = 0;
        for (uint32_t i = 2; i < iter.len(); ++i) {
            total_size += GetNumComponentsInBaseType(get_def(iter.word(i)));
        }
        return total_size;
    } else if (opcode == spv::OpTypePointer) {
        const auto type = get_def(iter.word(3));
        return GetNumComponentsInBaseType(type);
    }
    return 0;
}

uint32_t SHADER_MODULE_STATE::GetTypeBitsSize(const spirv_inst_iter &iter) const {
    const uint32_t opcode = iter.opcode();
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt) {
        return iter.word(2);
    } else if (opcode == spv::OpTypeVector) {
        const auto component_type = get_def(iter.word(2));
        uint32_t scalar_width = GetTypeBitsSize(component_type);
        uint32_t component_count = iter.word(3);
        return scalar_width * component_count;
    } else if (opcode == spv::OpTypeMatrix) {
        const auto column_type = get_def(iter.word(2));
        uint32_t vector_width = GetTypeBitsSize(column_type);
        uint32_t column_count = iter.word(3);
        return vector_width * column_count;
    } else if (opcode == spv::OpTypeArray) {
        const auto element_type = get_def(iter.word(2));
        uint32_t element_width = GetTypeBitsSize(element_type);
        const auto length_type = get_def(iter.word(3));
        uint32_t length = GetConstantValue(length_type);
        return element_width * length;
    } else if (opcode == spv::OpTypeStruct) {
        uint32_t total_size = 0;
        for (uint32_t i = 2; i < iter.len(); ++i) {
            total_size += GetTypeBitsSize(get_def(iter.word(i)));
        }
        return total_size;
    } else if (opcode == spv::OpTypePointer) {
        const auto type = get_def(iter.word(3));
        return GetTypeBitsSize(type);
    } else if (opcode == spv::OpVariable) {
        const auto type = get_def(iter.word(1));
        return GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeBool) {
        // The Spec states:
        // "Boolean values considered as 32-bit integer values for the purpose of this calculation"
        // when getting the size for the limits
        return 32;
    }
    return 0;
}

uint32_t SHADER_MODULE_STATE::GetTypeBytesSize(const spirv_inst_iter &iter) const { return GetTypeBitsSize(iter) / 8; }

// Returns the base type (float, int or unsigned int) or struct (can have multiple different base types inside)
// Will return 0 if it can not be determined
uint32_t SHADER_MODULE_STATE::GetBaseType(const spirv_inst_iter &iter) const {
    const uint32_t opcode = iter.opcode();
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt || opcode == spv::OpTypeBool || opcode == spv::OpTypeStruct) {
        // point to itself as its the base type (or a struct that needs to be traversed still)
        return iter.word(1);
    } else if (opcode == spv::OpTypeVector) {
        const auto& component_type = get_def(iter.word(2));
        return GetBaseType(component_type);
    } else if (opcode == spv::OpTypeMatrix) {
        const auto& column_type = get_def(iter.word(2));
        return GetBaseType(column_type);
    } else if (opcode == spv::OpTypeArray || opcode == spv::OpTypeRuntimeArray) {
        const auto& element_type = get_def(iter.word(2));
        return GetBaseType(element_type);
    } else if (opcode == spv::OpTypePointer) {
        const auto &storage_class = iter.word(2);
        const auto& type = get_def(iter.word(3));
        if (storage_class == spv::StorageClassPhysicalStorageBuffer && type.opcode() == spv::OpTypeStruct) {
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

// Returns type_id if id has type or zero otherwise
uint32_t SHADER_MODULE_STATE::GetTypeId(uint32_t id) const {
    const auto type = get_def(id);
    return OpcodeHasType(type.opcode()) ? type.word(1) : 0;
}

std::vector<uint32_t> FindEntrypointInterfaces(const spirv_inst_iter &entrypoint) {
    assert(entrypoint.opcode() == spv::OpEntryPoint);

    std::vector<uint32_t> interfaces;
    // Find the end of the entrypoint's name string. additional zero bytes follow the actual null terminator, to fill out the
    // rest of the word - so we only need to look at the last byte in the word to determine which word contains the terminator.
    uint32_t word = 3;
    while (entrypoint.word(word) & 0xff000000u) {
        ++word;
    }
    ++word;

    for (; word < entrypoint.len(); word++) interfaces.push_back(entrypoint.word(word));

    return interfaces;
}
