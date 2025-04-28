/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "pass.h"
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include "function_basic_block.h"
#include "generated/spirv_grammar_helper.h"
#include "link.h"
#include "state_tracker/shader_instruction.h"
#include "module.h"
#include "gpuav/shaders/gpuav_error_codes.h"

namespace gpuav {
namespace spirv {

bool Pass::Run() {
    const bool modified = Instrument();
    if (module_.settings_.print_debug_info) {
        PrintDebugInfo();
    }

    // Detect if any functions were applied that we need to add now
    if (modified && !link_info_.functions.empty()) {
        module_.link_infos_.emplace_back(link_info_);
    }
    return modified;
}

const Variable& Pass::GetBuiltinVariable(uint32_t built_in) {
    uint32_t variable_id = 0;
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(2) == spv::DecorationBuiltIn &&
            annotation->Word(3) == built_in) {
            variable_id = annotation->Word(1);
            break;
        }
    }

    if (variable_id == 0) {
        variable_id = module_.TakeNextId();
        auto new_inst = std::make_unique<Instruction>(4, spv::OpDecorate);
        new_inst->Fill({variable_id, spv::DecorationBuiltIn, built_in});
        module_.annotations_.emplace_back(std::move(new_inst));
    }

    // Currently we only ever needed Input variables and the built-ins we are using are not those that can be used by both Input and
    // Output storage classes
    const Variable* built_in_variable = module_.type_manager_.FindVariableById(variable_id);
    if (!built_in_variable) {
        const Type& pointer_type = module_.type_manager_.GetTypePointerBuiltInInput(spv::BuiltIn(built_in));
        auto new_inst = std::make_unique<Instruction>(4, spv::OpVariable);
        new_inst->Fill({pointer_type.Id(), variable_id, spv::StorageClassInput});
        built_in_variable = &module_.type_manager_.AddVariable(std::move(new_inst), pointer_type);
        module_.AddInterfaceVariables(built_in_variable->Id(), spv::StorageClassInput);
    }

    return *built_in_variable;
}

// To reduce having to load this information everytime we do a OpFunctionCall, instead just create it once per Function block and
// reference it each time
uint32_t Pass::GetStageInfo(Function& function, const BasicBlock& target_block_it, InstructionIt& out_inst_it) {
    // Cached so only need to compute this once
    if (function.stage_info_id_ != 0) {
        return function.stage_info_id_;
    }

    // Save original for later to restore
    const Instruction& target_instruction = *out_inst_it->get();

    BasicBlock& block = function.GetFirstBlock();
    InstructionIt inst_it = block.GetFirstInjectableInstrution();

    // Stage info is always passed in as a uvec4
    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);
    const Type& uvec4_type = module_.type_manager_.GetTypeVector(uint32_type, 4);
    const uint32_t uint32_0_id = module_.type_manager_.GetConstantZeroUint32().Id();
    uint32_t stage_info[4] = {uint32_0_id, uint32_0_id, uint32_0_id, uint32_0_id};

    if (module_.entry_points_.size() > 1) {
        // For Multi Entry Points it currently a lot of work to scan every function to see where it will be called from
        // For now we will just report it is "unknown" and skip printing that part of the error message
        stage_info[0] = module_.type_manager_.GetConstantUInt32(glsl::kExecutionModelMultiEntryPoint).Id();
    } else {
        spv::ExecutionModel execution_model = spv::ExecutionModel(module_.entry_points_.begin()->get()->Operand(0));

        // Need to map how GenerateStageMessage() will consume it
        uint32_t normalized_execution_model = execution_model;
        if (execution_model == spv::ExecutionModelTaskNV) {
            normalized_execution_model = glsl::kExecutionModelTaskNV;
        } else if (execution_model == spv::ExecutionModelMeshNV) {
            normalized_execution_model = glsl::kExecutionModelMeshNV;
        } else if (execution_model == spv::ExecutionModelRayGenerationKHR) {
            normalized_execution_model = glsl::kExecutionModelRayGenerationKHR;
        } else if (execution_model == spv::ExecutionModelIntersectionKHR) {
            normalized_execution_model = glsl::kExecutionModelIntersectionKHR;
        } else if (execution_model == spv::ExecutionModelAnyHitKHR) {
            normalized_execution_model = glsl::kExecutionModelAnyHitKHR;
        } else if (execution_model == spv::ExecutionModelClosestHitKHR) {
            normalized_execution_model = glsl::kExecutionModelClosestHitKHR;
        } else if (execution_model == spv::ExecutionModelMissKHR) {
            normalized_execution_model = glsl::kExecutionModelMissKHR;
        } else if (execution_model == spv::ExecutionModelCallableKHR) {
            normalized_execution_model = glsl::kExecutionModelCallableKHR;
        } else if (execution_model == spv::ExecutionModelCallableKHR) {
            normalized_execution_model = glsl::kExecutionModelCallableKHR;
        } else if (execution_model == spv::ExecutionModelTaskEXT) {
            normalized_execution_model = glsl::kExecutionModelTaskEXT;
        } else if (execution_model == spv::ExecutionModelMeshEXT) {
            normalized_execution_model = glsl::kExecutionModelMeshEXT;
        }
        stage_info[0] = module_.type_manager_.GetConstantUInt32(normalized_execution_model).Id();

        // Gets BuiltIn variable and creates a valid OpLoad of it
        auto create_load = [this, &block, &inst_it](spv::BuiltIn built_in) {
            const Variable& variable = GetBuiltinVariable(built_in);
            const Type* pointer_type = variable.PointerType(module_.type_manager_);
            const uint32_t load_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpLoad, {pointer_type->Id(), load_id, variable.Id()}, &inst_it);
            return load_id;
        };

        switch (execution_model) {
            case spv::ExecutionModelVertex: {
                uint32_t load_id = create_load(spv::BuiltInVertexIndex);
                stage_info[1] = CastToUint32(load_id, block, &inst_it);
                load_id = create_load(spv::BuiltInInstanceIndex);
                stage_info[2] = CastToUint32(load_id, block, &inst_it);
            } break;
            case spv::ExecutionModelFragment: {
                const uint32_t load_id = create_load(spv::BuiltInFragCoord);
                // convert vec4 to uvec4
                const uint32_t bitcast_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpBitcast, {uvec4_type.Id(), bitcast_id, load_id}, &inst_it);

                for (uint32_t i = 0; i < 2; i++) {
                    const uint32_t extract_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, bitcast_id, i}, &inst_it);
                    stage_info[i + 1] = extract_id;
                }
            } break;
            case spv::ExecutionModelRayGenerationKHR:
            case spv::ExecutionModelIntersectionKHR:
            case spv::ExecutionModelAnyHitKHR:
            case spv::ExecutionModelClosestHitKHR:
            case spv::ExecutionModelMissKHR:
            case spv::ExecutionModelCallableKHR: {
                const uint32_t load_id = create_load(spv::BuiltInLaunchIdKHR);

                for (uint32_t i = 0; i < 3; i++) {
                    const uint32_t extract_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, load_id, i}, &inst_it);
                    stage_info[i + 1] = extract_id;
                }
            } break;
            case spv::ExecutionModelGLCompute:
            case spv::ExecutionModelTaskNV:
            case spv::ExecutionModelMeshNV:
            case spv::ExecutionModelTaskEXT:
            case spv::ExecutionModelMeshEXT: {
                // This can be both a uvec3 or ivec3 so need to cast if ivec3
                const Variable& variable = GetBuiltinVariable(spv::BuiltInGlobalInvocationId);
                const Type* pointer_type = variable.PointerType(module_.type_manager_);
                const uint32_t load_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpLoad, {pointer_type->Id(), load_id, variable.Id()}, &inst_it);
                uint32_t final_load_id = load_id;

                if (pointer_type->IsIVec3(module_.type_manager_)) {
                    const Type& vec3_type = module_.type_manager_.GetTypeVector(uint32_type, 3);
                    final_load_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpBitcast, {vec3_type.Id(), final_load_id, load_id}, &inst_it);
                }

                for (uint32_t i = 0; i < 3; i++) {
                    const uint32_t extract_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, final_load_id, i}, &inst_it);
                    stage_info[i + 1] = extract_id;
                }
            } break;
            case spv::ExecutionModelGeometry: {
                const uint32_t primitive_id = create_load(spv::BuiltInPrimitiveId);
                stage_info[1] = CastToUint32(primitive_id, block, &inst_it);
                const uint32_t load_id = create_load(spv::BuiltInInvocationId);
                stage_info[2] = CastToUint32(load_id, block, &inst_it);
            } break;
            case spv::ExecutionModelTessellationControl: {
                const uint32_t load_id = create_load(spv::BuiltInInvocationId);
                stage_info[1] = CastToUint32(load_id, block, &inst_it);
                const uint32_t primitive_id = create_load(spv::BuiltInPrimitiveId);
                stage_info[2] = CastToUint32(primitive_id, block, &inst_it);
            } break;
            case spv::ExecutionModelTessellationEvaluation: {
                const uint32_t primitive_id = create_load(spv::BuiltInPrimitiveId);
                stage_info[1] = CastToUint32(primitive_id, block, &inst_it);

                // convert vec3 to uvec3
                const Type& uvec3_type = module_.type_manager_.GetTypeVector(uint32_type, 3);
                const uint32_t load_id = create_load(spv::BuiltInTessCoord);
                const uint32_t bitcast_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpBitcast, {uvec3_type.Id(), bitcast_id, load_id}, &inst_it);

                // TessCoord.uv values from it
                for (uint32_t i = 0; i < 2; i++) {
                    const uint32_t extract_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, bitcast_id, i}, &inst_it);
                    stage_info[i + 2] = extract_id;
                }
            } break;
            default:
                module_.InternalError(Name(), "GetStageInfo has unsupported stage");
                break;
        }
    }

    function.stage_info_id_ = module_.TakeNextId();
    block.CreateInstruction(spv::OpCompositeConstruct,
                            {uvec4_type.Id(), function.stage_info_id_, stage_info[0], stage_info[1], stage_info[2], stage_info[3]},
                            &inst_it);

    function.stage_info_x_id_ = stage_info[0];
    function.stage_info_y_id_ = stage_info[1];
    function.stage_info_z_id_ = stage_info[2];
    function.stage_info_w_id_ = stage_info[3];

    // because we are injecting things in the first block, there is a chance we just destroyed the iterator if the target
    // instruction was also in the first block, so need to regain it for the caller
    if (target_block_it.GetLabelId() == block.GetLabelId()) {
        out_inst_it = FindTargetInstruction(block, target_instruction);
    }

    return function.stage_info_id_;
}

const Instruction* Pass::GetDecoration(uint32_t id, spv::Decoration decoration) const {
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == id &&
            spv::Decoration(annotation->Word(2)) == decoration) {
            return annotation.get();
        }
    }
    return nullptr;
}

const Instruction* Pass::GetMemberDecoration(uint32_t id, uint32_t member_index, spv::Decoration decoration) const {
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpMemberDecorate && annotation->Word(1) == id && annotation->Word(2) == member_index &&
            spv::Decoration(annotation->Word(3)) == decoration) {
            return annotation.get();
        }
    }
    return nullptr;
}

// In an ideal world, this would be baked into the Type class when we construct it. The core issue is OpTypeMatrix size can be
// different depending where it is used. Because of this, we need to have a higher level view what is going on in order to correctly
// figure out the size of a given type.
uint32_t Pass::FindTypeByteSize(uint32_t type_id, uint32_t matrix_stride, bool col_major, bool in_matrix) const {
    const Type& type = *module_.type_manager_.FindTypeById(type_id);
    switch (type.spv_type_) {
        case SpvType::kPointer:
            return 8;  // Assuming PhysicalStorageBuffer pointer
            break;
        case SpvType::kMatrix: {
            if (matrix_stride == 0) {
                module_.InternalError("FindTypeByteSize", "missing matrix stride");
            }
            if (col_major) {
                return type.inst_.Word(3) * matrix_stride;
            } else {
                const Type* vector_type = module_.type_manager_.FindTypeById(type.inst_.Word(2));
                return vector_type->inst_.Word(3) * matrix_stride;
            }
        }
        case SpvType::kVector: {
            uint32_t size = type.inst_.Word(3);
            const Type* component_type = module_.type_manager_.FindTypeById(type.inst_.Word(2));
            // if vector in row major matrix, the vector is strided so return the number of bytes spanned by the vector
            if (in_matrix && !col_major && matrix_stride > 0) {
                return (size - 1) * matrix_stride + FindTypeByteSize(component_type->Id());
            } else if (component_type->spv_type_ == SpvType::kFloat || component_type->spv_type_ == SpvType::kInt) {
                const uint32_t width = component_type->inst_.Word(2);
                size *= width;
            } else {
                module_.InternalError("FindTypeByteSize", "unexpected vector type");
            }
            return size / 8;
        }
        case SpvType::kFloat:
        case SpvType::kInt: {
            const uint32_t width = type.inst_.Word(2);
            return width / 8;
        }
        case SpvType::kArray: {
            const uint32_t array_stride = GetDecoration(type_id, spv::DecorationArrayStride)->Word(3);
            const Constant* count = module_.type_manager_.FindConstantById(type.inst_.Operand(1));
            // TODO - Need to handle spec constant here, for now return one to have things not blowup
            assert(count && !count->is_spec_constant_);
            const uint32_t array_length = (count && !count->is_spec_constant_) ? count->inst_.Operand(0) : 1;
            return array_length * array_stride;
        }
        case SpvType::kStruct: {
            const uint32_t struct_length = type.inst_.Length() - 2;
            const uint32_t struct_id = type.inst_.ResultId();
            // We do our best to find the "size" of the struct (see https://gitlab.khronos.org/spirv/SPIR-V/-/issues/763)
            uint32_t highest_element_index = 0;
            uint32_t highest_element_offset = 0;

            for (uint32_t i = 0; i < struct_length; i++) {
                for (const auto& annotation : module_.annotations_) {
                    if (annotation->Opcode() == spv::OpMemberDecorate && annotation->Word(1) == struct_id &&
                        annotation->Word(2) == i && spv::Decoration(annotation->Word(3)) == spv::DecorationOffset) {
                        const uint32_t member_offset = annotation->Word(4);
                        if (member_offset > highest_element_offset) {
                            highest_element_index = i;
                            highest_element_offset = member_offset;
                        }
                        break;
                    }
                }
            }

            const uint32_t last_offset_id = type.inst_.Operand(highest_element_index);
            const Type* last_offset_type = module_.type_manager_.FindTypeById(last_offset_id);
            uint32_t highest_element_size = 0;
            if (last_offset_type->spv_type_ == SpvType::kMatrix) {
                // TODO - We need a better way to handle Matrix at the end of structs
                const Instruction* decoration_matrix_stride =
                    GetMemberDecoration(struct_id, highest_element_index, spv::DecorationMatrixStride);
                matrix_stride = decoration_matrix_stride ? decoration_matrix_stride->Word(4) : 0;
                const Instruction* decoration_col_major =
                    GetMemberDecoration(struct_id, highest_element_index, spv::DecorationColMajor);
                col_major = decoration_col_major != nullptr;
                highest_element_size = FindTypeByteSize(last_offset_id, matrix_stride, col_major, true);
            } else {
                highest_element_size = FindTypeByteSize(last_offset_id);
            }
            return highest_element_offset + highest_element_size;
        }
        default:
            break;
    }
    return 1;
}

// Find outermost buffer type and its access chain index.
// Because access chains indexes can be runtime values, we need to build arithmetic logic in the SPIR-V to get the runtime value of
// the indexing
uint32_t Pass::GetLastByte(const Type& descriptor_type, const std::vector<const Instruction*>& access_chain_insts,
                           BasicBlock& block, InstructionIt* inst_it) {
    assert(!access_chain_insts.empty());
    uint32_t current_type_id = 0;
    const uint32_t reset_ac_word = 4;  // points to first "Index" operand of an OpAccessChain
    uint32_t ac_word_index = reset_ac_word;

    if (descriptor_type.IsArray()) {
        current_type_id = descriptor_type.inst_.Operand(0);
        ac_word_index++;  // this jumps over the array of descriptors so we first start on the descriptor itself
    } else if (descriptor_type.spv_type_ == SpvType::kStruct) {
        current_type_id = descriptor_type.Id();
    } else {
        module_.InternalError(Name(), "GetLastByte has unexpected descriptor type");
        return 0;
    }

    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);

    // instruction that will have calculated the sum of the byte offset
    uint32_t sum_id = 0;

    uint32_t matrix_stride = 0;
    bool col_major = false;
    uint32_t matrix_stride_id = 0;
    bool in_matrix = false;

    // This loop gets use to the last element, so if we have something like
    //
    // Struct foo {
    //   uint a; // 4 bytes
    //   vec4 b; // 16 bytes
    //   float c; <--- accessing
    // }
    //
    // it will get us to 20 bytes
    auto access_chain_iter = access_chain_insts.rbegin();

    // This occurs in things like Slang where they have a single OpAccessChain for the descriptor
    // (GLSL/HLSL will combine 2 indexes into the last OpAccessChain)
    if (ac_word_index >= (*access_chain_iter)->Length()) {
        ++access_chain_iter;
        ac_word_index = reset_ac_word;
    }

    while (access_chain_iter != access_chain_insts.rend()) {
        const uint32_t ac_index_id = (*access_chain_iter)->Word(ac_word_index);
        uint32_t current_offset_id = 0;

        const Type* current_type = module_.type_manager_.FindTypeById(current_type_id);
        switch (current_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kRuntimeArray: {
                // Get array stride and multiply by current index
                const uint32_t array_stride = GetDecoration(current_type_id, spv::DecorationArrayStride)->Word(3);
                const uint32_t array_stride_id = module_.type_manager_.GetConstantUInt32(array_stride).Id();
                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block, inst_it);

                current_offset_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, array_stride_id, ac_index_id_32},
                                        inst_it);

                // Get element type for next step
                current_type_id = current_type->inst_.Operand(0);
            } break;
            case SpvType::kMatrix: {
                if (matrix_stride == 0) {
                    module_.InternalError(Name(), "GetLastByte is missing matrix stride");
                }
                matrix_stride_id = module_.type_manager_.GetConstantUInt32(matrix_stride).Id();
                uint32_t vec_type_id = current_type->inst_.Operand(0);

                // If column major, multiply column index by matrix stride, otherwise by vector component size and save matrix
                // stride for vector (row) index
                uint32_t col_stride_id = 0;
                if (col_major) {
                    col_stride_id = matrix_stride_id;
                } else {
                    const uint32_t component_type_id = module_.type_manager_.FindTypeById(vec_type_id)->inst_.Operand(0);
                    const uint32_t col_stride = FindTypeByteSize(component_type_id);
                    col_stride_id = module_.type_manager_.GetConstantUInt32(col_stride).Id();
                }

                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block, inst_it);
                current_offset_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, col_stride_id, ac_index_id_32}, inst_it);

                // Get element type for next step
                current_type_id = vec_type_id;
                in_matrix = true;
            } break;
            case SpvType::kVector: {
                // If inside a row major matrix type, multiply index by matrix stride,
                // else multiply by component size
                const uint32_t component_type_id = current_type->inst_.Operand(0);
                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block, inst_it);
                if (in_matrix && !col_major) {
                    current_offset_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, matrix_stride_id, ac_index_id_32},
                                            inst_it);
                } else {
                    const uint32_t component_type_size = FindTypeByteSize(component_type_id);
                    const uint32_t size_id = module_.type_manager_.GetConstantUInt32(component_type_size).Id();

                    current_offset_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, size_id, ac_index_id_32}, inst_it);
                }
                // Get element type for next step
                current_type_id = component_type_id;
            } break;
            case SpvType::kStruct: {
                // Get buffer byte offset for the referenced member
                const Constant* member_constant = module_.type_manager_.FindConstantById(ac_index_id);
                assert(!member_constant->is_spec_constant_);
                uint32_t member_index = member_constant->inst_.Operand(0);
                uint32_t member_offset = GetMemberDecoration(current_type_id, member_index, spv::DecorationOffset)->Word(4);
                current_offset_id = module_.type_manager_.GetConstantUInt32(member_offset).Id();

                // Look for matrix stride for this member if there is one. The matrix
                // stride is not on the matrix type, but in a OpMemberDecorate on the
                // enclosing struct type at the member index. If none found, reset
                // stride to 0.
                const Instruction* decoration_matrix_stride =
                    GetMemberDecoration(current_type_id, member_index, spv::DecorationMatrixStride);
                matrix_stride = decoration_matrix_stride ? decoration_matrix_stride->Word(4) : 0;

                const Instruction* decoration_col_major =
                    GetMemberDecoration(current_type_id, member_index, spv::DecorationColMajor);
                col_major = decoration_col_major != nullptr;

                // Get element type for next step
                current_type_id = current_type->inst_.Operand(member_index);
            } break;
            default: {
                module_.InternalError(Name(), "GetLastByte has unexpected non-composite type");
            } break;
        }

        if (sum_id == 0) {
            sum_id = current_offset_id;
        } else {
            const uint32_t new_sum_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpIAdd, {uint32_type.Id(), new_sum_id, sum_id, current_offset_id}, inst_it);
            sum_id = new_sum_id;
        }

        ac_word_index++;
        if (ac_word_index >= (*access_chain_iter)->Length()) {
            ++access_chain_iter;
            ac_word_index = reset_ac_word;
        }
    }

    // Add in offset of last byte of referenced object
    const uint32_t accessed_type_size = FindTypeByteSize(current_type_id, matrix_stride, col_major, in_matrix);
    const uint32_t last_byte_index = accessed_type_size - 1;

    const uint32_t last_byte_index_id = module_.type_manager_.GetConstantUInt32(last_byte_index).Id();

    const uint32_t new_sum_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpIAdd, {uint32_type.Id(), new_sum_id, sum_id, last_byte_index_id}, inst_it);
    return new_sum_id;
}

// Finds the upper bound offset into the struct an instruction would access
// If it is a non-constant value, will return zero to indicate its a runtime value
//
// If shader looks for 'b' in a descriptor like
//
// struct X {
//    uint a;
//    uint b;
//    uint c;
// }
//
// it will return `7` because it covers [4, 7] bytes of the descriptor
// (This matches the GetLastByte() check)
uint32_t Pass::FindOffsetInStruct(uint32_t struct_id, bool is_descriptor_array,
                                  const std::vector<const Instruction*>& access_chain_insts) const {
    assert(!access_chain_insts.empty());
    uint32_t last_byte_offset = 0;
    const uint32_t reset_ac_word = 4;  // points to first "Index" operand of an OpAccessChain
    uint32_t ac_word_index = reset_ac_word;

    if (is_descriptor_array) {
        ac_word_index++;  // this jumps over the array of descriptors so we first start on the descriptor itself
    }

    uint32_t matrix_stride = 0;
    bool col_major = false;
    bool in_matrix = false;

    auto access_chain_iter = access_chain_insts.rbegin();

    // This occurs in things like Slang where they have a single OpAccessChain for the descriptor
    // (GLSL/HLSL will combine 2 indexes into the last OpAccessChain)
    if (ac_word_index >= (*access_chain_iter)->Length()) {
        ++access_chain_iter;
        ac_word_index = reset_ac_word;
    }

    uint32_t current_type_id = struct_id;
    // Walk down access chains to build up the offset
    while (access_chain_iter != access_chain_insts.rend()) {
        const uint32_t ac_index_id = (*access_chain_iter)->Word(ac_word_index);
        const Constant* index_constant = module_.type_manager_.FindConstantById(ac_index_id);
        if (!index_constant || index_constant->inst_.Opcode() != spv::OpConstant) {
            return 0;  // Access Chain has dynamic value
        }
        const uint32_t constant_value = index_constant->GetValueUint32();

        uint32_t current_offset = 0;

        const Type* current_type = module_.type_manager_.FindTypeById(current_type_id);
        switch (current_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kRuntimeArray: {
                // Get array stride and multiply by current index
                const uint32_t array_stride = GetDecoration(current_type_id, spv::DecorationArrayStride)->Word(3);
                current_offset = constant_value * array_stride;

                current_type_id = current_type->inst_.Operand(0);  // Get element type for next step
            } break;
            case SpvType::kMatrix: {
                if (matrix_stride == 0) {
                    module_.InternalError(Name(), "FindOffsetInStruct is missing matrix stride");
                }
                in_matrix = true;
                uint32_t vec_type_id = current_type->inst_.Operand(0);

                // If column major, multiply column index by matrix stride, otherwise by vector component size and save matrix
                // stride for vector (row) index
                uint32_t col_stride = 0;
                if (col_major) {
                    col_stride = matrix_stride;
                } else {
                    const uint32_t component_type_id = module_.type_manager_.FindTypeById(vec_type_id)->inst_.Operand(0);
                    col_stride = FindTypeByteSize(component_type_id);
                }

                current_offset = constant_value * col_stride;

                current_type_id = vec_type_id;  // Get element type for next step
            } break;
            case SpvType::kVector: {
                // If inside a row major matrix type, multiply index by matrix stride,
                // else multiply by component size
                const uint32_t component_type_id = current_type->inst_.Operand(0);

                if (in_matrix && !col_major) {
                    current_offset = constant_value * matrix_stride;
                } else {
                    const uint32_t component_type_size = FindTypeByteSize(component_type_id);
                    current_offset = constant_value * component_type_size;
                }

                current_type_id = component_type_id;  // Get element type for next step
            } break;
            case SpvType::kStruct: {
                // Get buffer byte offset for the referenced member
                current_offset = GetMemberDecoration(current_type_id, constant_value, spv::DecorationOffset)->Word(4);

                // Look for matrix stride for this member if there is one. The matrix
                // stride is not on the matrix type, but in a OpMemberDecorate on the
                // enclosing struct type at the member index. If none is found, reset
                // stride to 0.
                const Instruction* decoration_matrix_stride =
                    GetMemberDecoration(current_type_id, constant_value, spv::DecorationMatrixStride);
                matrix_stride = decoration_matrix_stride ? decoration_matrix_stride->Word(4) : 0;

                const Instruction* decoration_col_major =
                    GetMemberDecoration(current_type_id, constant_value, spv::DecorationColMajor);
                col_major = decoration_col_major != nullptr;

                current_type_id = current_type->inst_.Operand(constant_value);  // Get element type for next step
            } break;
            default: {
                module_.InternalError(Name(), "FindOffsetInStruct has unexpected non-composite type");
            } break;
        }

        last_byte_offset += current_offset;

        ac_word_index++;
        if (ac_word_index >= (*access_chain_iter)->Length()) {
            ++access_chain_iter;
            ac_word_index = reset_ac_word;
        }
    }

    // Add in offset of last byte of referenced object
    const uint32_t accessed_type_size = FindTypeByteSize(current_type_id, matrix_stride, col_major, in_matrix);
    const uint32_t last_byte_index = accessed_type_size - 1;
    last_byte_offset += last_byte_index;

    return last_byte_offset;
}

// Generate code to convert integer id to 32bit, if needed.
uint32_t Pass::ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) const {
    // Find type doing the indexing into the access chain
    const Type* type = nullptr;
    const Constant* constant = module_.type_manager_.FindConstantById(id);
    if (constant) {
        type = &constant->type_;
    } else {
        const Instruction* inst = block.function_.FindInstruction(id);
        if (inst) {
            type = module_.type_manager_.FindTypeById(inst->TypeId());
        }
    }
    if (!type) {
        return id;
    }
    assert(type->spv_type_ == SpvType::kInt);
    if (type->inst_.Word(2) == 32) {
        return id;
    }

    const bool is_signed = type->inst_.Word(3) != 0;
    const uint32_t new_id = module_.TakeNextId();
    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);
    if (is_signed) {
        block.CreateInstruction(spv::OpSConvert, {uint32_type.Id(), new_id, id}, inst_it);
    } else {
        block.CreateInstruction(spv::OpUConvert, {uint32_type.Id(), new_id, id}, inst_it);
    }
    return new_id;  // Return an id to the 32bit equivalent.
}

// Generate code to cast integer it to 32bit unsigned, if needed.
uint32_t Pass::CastToUint32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) const {
    // Convert value to 32-bit if necessary
    uint32_t int32_id = ConvertTo32(id, block, inst_it);

    const Type* type = nullptr;
    const Constant* constant = module_.type_manager_.FindConstantById(int32_id);
    if (constant) {
        type = &constant->type_;
    } else {
        const Instruction* inst = block.function_.FindInstruction(int32_id);
        if (inst) {
            type = module_.type_manager_.FindTypeById(inst->TypeId());
        }
    }
    if (!type) {
        return int32_id;
    }
    assert(type->spv_type_ == SpvType::kInt);
    const bool is_signed = type->inst_.Word(3) != 0;
    if (!is_signed) {
        return int32_id;
    }

    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);
    const uint32_t new_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpBitcast, {uint32_type.Id(), new_id, int32_id}, inst_it);
    return new_id;  // Return an id to the Uint equivalent.
}

InstructionIt Pass::FindTargetInstruction(BasicBlock& block, const Instruction& target_instruction) const {
    const uint32_t target_id = target_instruction.ResultId();
    for (auto inst_it = block.instructions_.begin(); inst_it != block.instructions_.end(); ++inst_it) {
        // This has to re-loop the entire block to find the instruction, using the ResultID, we can quickly compare
        if ((*inst_it)->ResultId() == target_id) {
            // Things like OpStore will have a result id of zero, so need to do deep instruction comparison
            if (*(*inst_it) == target_instruction) {
                return inst_it;
            }
        }
    }

    module_.InternalError(Name(), "failed to find instruction");
    return block.instructions_.end();
}

bool Pass::IsMaxInstrumentationsCount() const {
    return (module_.settings_.max_instrumentations_count != 0) &&
           (instrumentations_count_ >= module_.settings_.max_instrumentations_count);
}

// A type of common pass that will inject a function call and link it up later,
// We will have wrap the checks to be safe from bad values crashing things
// For OpStore we will just ignore the store if it is invalid, example:
// Before:
//     bda.data[index] = value;
// After:
//    if (isValid(bda.data, index)) {
//         bda.data[index] = value;
//    }
//
// For OpLoad we replace the value with Zero (via Phi node) if it is invalid, example
// Before:
//     int X = bda.data[index];
//     int Y = bda.data[X];
// After:
//    if (isValid(bda.data, index)) {
//         int X = bda.data[index];
//    } else {
//         int X = 0;
//    }
//    if (isValid(bda.data, X)) {
//         int Y = bda.data[X];
//    } else {
//         int Y = 0;
//    }
InjectConditionalData Pass::InjectFunctionPre(Function& function, const BasicBlockIt original_block_it, InstructionIt inst_it) {
    // We turn the block into 4 separate blocks
    BasicBlock& original_block = **original_block_it;
    const uint32_t original_label = original_block.GetLabelId();

    // Where we call targeted instruction if it is valid
    BasicBlockIt valid_block_it = function.InsertNewBlock(original_block_it);
    BasicBlock& valid_block = **valid_block_it;
    const uint32_t valid_block_label = valid_block.GetLabelId();

    // will be an empty block, used for the Phi node, even if no result, create for simplicity
    BasicBlockIt invalid_block_it = function.InsertNewBlock(valid_block_it);
    BasicBlock& invalid_block = **invalid_block_it;
    const uint32_t invalid_block_label = invalid_block.GetLabelId();

    // All the remaining block instructions after targeted instruction
    BasicBlockIt merge_block_it = function.InsertNewBlock(invalid_block_it);
    BasicBlock& merge_block = **merge_block_it;
    const uint32_t merge_block_label = merge_block.GetLabelId();

    // need to preserve the control-flow of how things, like a OpPhi, are accessed from a predecessor block
    function.ReplaceAllUsesWith(original_label, merge_block_label);

    // Move the targeted instruction to a valid block
    const Instruction& target_inst = *valid_block.instructions_.emplace_back(std::move(*inst_it));
    inst_it = original_block.instructions_.erase(inst_it);
    valid_block.CreateInstruction(spv::OpBranch, {merge_block_label});

    // If thre is a result, we need to create an additional BasicBlock to hold the |else| case, then after we create a Phi node to
    // hold the result
    const uint32_t target_inst_id = target_inst.ResultId();
    if (target_inst_id != 0) {
        const uint32_t phi_id = module_.TakeNextId();
        const Type& phi_type = *module_.type_manager_.FindTypeById(target_inst.TypeId());
        uint32_t null_id = 0;
        // Can't create ConstantNull of pointer type, so convert uint64 zero to pointer
        if (phi_type.spv_type_ == SpvType::kPointer) {
            const Type& uint64_type = module_.type_manager_.GetTypeInt(64, false);
            const Constant& null_constant = module_.type_manager_.GetConstantNull(uint64_type);
            null_id = module_.TakeNextId();
            // We need to put any intermittent instructions here so Phi is first in the merge block
            invalid_block.CreateInstruction(spv::OpConvertUToPtr, {phi_type.Id(), null_id, null_constant.Id()});
            module_.AddCapability(spv::CapabilityInt64);
        } else {
            if ((phi_type.spv_type_ == SpvType::kInt || phi_type.spv_type_ == SpvType::kFloat) && phi_type.inst_.Word(2) < 32) {
                // You can't make a constant of a 8-int, 16-int, 16-float without having the capability
                // The only way this situation occurs if they use something like
                //     OpCapability StorageBuffer8BitAccess
                // but there is not explicit Int8
                // It should be more than safe to inject it for them
                spv::Capability capability = (phi_type.spv_type_ == SpvType::kFloat) ? spv::CapabilityFloat16
                                             : (phi_type.inst_.Word(2) == 16)        ? spv::CapabilityInt16
                                                                                     : spv::CapabilityInt8;
                module_.AddCapability(capability);
            }

            null_id = module_.type_manager_.GetConstantNull(phi_type).Id();
        }

        // replace before creating instruction, otherwise will over-write itself
        function.ReplaceAllUsesWith(target_inst_id, phi_id);
        merge_block.CreateInstruction(spv::OpPhi,
                                      {phi_type.Id(), phi_id, target_inst_id, valid_block_label, null_id, invalid_block_label});
    }

    // When skipping some instructions, we need something valid to replace it
    if (target_inst.Opcode() == spv::OpRayQueryInitializeKHR) {
        // Currently assume the RayQuery and AS object were valid already
        const uint32_t uint32_0_id = module_.type_manager_.GetConstantZeroUint32().Id();
        const uint32_t float32_0_id = module_.type_manager_.GetConstantZeroFloat32().Id();
        const uint32_t vec3_0_id = module_.type_manager_.GetConstantZeroVec3().Id();
        invalid_block.CreateInstruction(spv::OpRayQueryInitializeKHR,
                                        {target_inst.Operand(0), target_inst.Operand(1), uint32_0_id, uint32_0_id, vec3_0_id,
                                         float32_0_id, vec3_0_id, float32_0_id});
    }

    invalid_block.CreateInstruction(spv::OpBranch, {merge_block_label});

    // move all remaining instructions to the newly created merge block
    merge_block.instructions_.insert(merge_block.instructions_.end(), std::make_move_iterator(inst_it),
                                     std::make_move_iterator(original_block.instructions_.end()));
    original_block.instructions_.erase(inst_it, original_block.instructions_.end());

    return InjectConditionalData{merge_block_label, valid_block_label, invalid_block_label, 0, merge_block_it};
}

void Pass::InjectFunctionPost(BasicBlock& original_block, const InjectConditionalData& ic_data) {
    original_block.CreateInstruction(spv::OpSelectionMerge, {ic_data.merge_block_label, spv::SelectionControlMaskNone});
    original_block.CreateInstruction(spv::OpBranchConditional,
                                     {ic_data.function_result_id, ic_data.valid_block_label, ic_data.invalid_block_label});
}

void Pass::ControlFlow::Update(const BasicBlock& block) {
    if (in_loop) {
        if (block.GetLabelId() == merge_target_id) {
            in_loop = false;
            merge_target_id = 0;
        }
    } else if (block.IsLoopHeader()) {
        in_loop = true;
        merge_target_id = block.loop_header_merge_target_;
    }
}

// Helper for passes with multiple linked functions they may grab
// Pass in cached link_function_id and only update it the first time
uint32_t Pass::GetLinkFunction(uint32_t& link_function_id, const OfflineFunction& offline) {
    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info_.functions.emplace_back(LinkFunction{offline, link_function_id});
    }
    return link_function_id;
}

void DescriptroIndexPushConstantAccess::Update(const Module& module, InstructionIt inst_it) {
    if (!(*inst_it)->IsNonPtrAccessChain()) {
        return;
    }

    const Variable* pc_variable = module.type_manager_.FindPushConstantVariable();
    if (!pc_variable) {
        return;  // shader doesn't use Push Constant
    }

    if ((*inst_it)->Operand(0) != pc_variable->Id()) {
        return;  // Access chain is not aimmed at the Push Constant
    }

    const Constant* member_index_constant = module.type_manager_.FindConstantById((*inst_it)->Operand(1));
    if (!member_index_constant) {
        return;  // dynamic access into Push Constant (which is crazy and not likely)
    }
    const uint32_t found_member_index = member_index_constant->Id();

    // We save memory/time tracking every instruction and know from viewing SPIR-V this pattern always will look like
    // %a = OpAccessChain %ptr %pc %uint_x
    // %b = OpLoad %uint %a
    // %c = OpIAdd %uint %b %uint_y (optional)
    //
    // We use this and just do a quick look ahead for load
    const uint32_t access_chain_id = (*inst_it)->ResultId();
    inst_it++;
    if ((*inst_it)->Opcode() != spv::OpLoad || (*inst_it)->Operand(0) != access_chain_id) {
        return;
    }

    const Type* access_type = module.type_manager_.FindTypeById((*inst_it)->TypeId());
    if (!access_type || access_type->spv_type_ != SpvType::kInt) {
        return;  // might be grabbing a uvec2 or float instead we want to ignore
    }

    uint32_t found_descriptor_index_id = (*inst_it)->ResultId();
    uint32_t found_add_id_value = 0;
    inst_it++;

    if ((*inst_it)->Opcode() == spv::OpIAdd) {
        const uint32_t add_0_id = (*inst_it)->Operand(0);
        const uint32_t add_1_id = (*inst_it)->Operand(1);
        // Might be (pc + constant) or (constant + pc)
        if (add_0_id == found_descriptor_index_id) {
            found_add_id_value = add_1_id;
        } else if (add_1_id == found_descriptor_index_id) {
            found_add_id_value = add_0_id;
        } else {
            return;  // we have hit a strange case and rather be safe and exit
        }
        found_descriptor_index_id = (*inst_it)->ResultId();
    }

    next_alias_id = found_descriptor_index_id;
    if (add_id_value != found_add_id_value || member_index != found_member_index) {
        // First time seeing the Push Constant, set starting values.
        // Also if found a new uint being used, need to reset.
        descriptor_index_id = found_descriptor_index_id;
        add_id_value = found_add_id_value;
        member_index = found_member_index;
    }
}

bool FunctionDuplicateTracker::FindAndUpdate(BlockDuplicateTracker& block, uint32_t hash) {
    // Subtle, but important, if you have
    //
    // inst_post_process(hash) A
    // if (x)
    //   inst_post_process(hash) B
    //   if (x)
    //     inst_post_process(hash) C
    //
    // A, B, and C are the same, we will be adding the hash here still for B, but never add the actual OpFunctionCall, then C will
    // detect the block B is in and also do the same. This means we create a Post-Dominated chain effect without having to store any
    // list of some sort.
    auto insert_pair = block.hashes.insert(hash);
    if (!insert_pair.second) {
        return true;  // found in this block
    }

    // Here we look back and see if this block is post-dominated by something with same instrumentation already
    if (block.merge_select_predecessor != 0) {
        BlockDuplicateTracker& predecessor_tracker = blocks_[block.merge_select_predecessor];
        if (predecessor_tracker.hashes.find(hash) != predecessor_tracker.hashes.end()) {
            return true;
        }
    }
    if (block.branch_conditional_predecessor != 0) {
        BlockDuplicateTracker& predecessor_tracker = blocks_[block.branch_conditional_predecessor];
        if (predecessor_tracker.hashes.find(hash) != predecessor_tracker.hashes.end()) {
            return true;
        }
    }
    if (block.switch_cases_predecessor != 0) {
        BlockDuplicateTracker& predecessor_tracker = blocks_[block.switch_cases_predecessor];
        if (predecessor_tracker.hashes.find(hash) != predecessor_tracker.hashes.end()) {
            return true;
        }
    }

    return false;
}

// If the block is terminating, mark the post-dominated blocks
BlockDuplicateTracker& FunctionDuplicateTracker::GetAndUpdate(BasicBlock& block) {
    const uint32_t current_block_id = block.GetLabelId();

    if (block.selection_merge_target_) {
        blocks_[block.selection_merge_target_].merge_select_predecessor = current_block_id;
    }

    if (block.branch_conditional_true_) {
        blocks_[block.branch_conditional_true_].branch_conditional_predecessor = current_block_id;
    }
    if (block.branch_conditional_false_) {
        blocks_[block.branch_conditional_false_].branch_conditional_predecessor = current_block_id;
    }

    if (block.switch_default_) {
        blocks_[block.switch_default_].switch_cases_predecessor = current_block_id;
    }
    for (uint32_t switch_case_id : block.switch_cases_) {
        blocks_[switch_case_id].switch_cases_predecessor = current_block_id;
    }

    return blocks_[current_block_id];
}

}  // namespace spirv
}  // namespace gpuav