/* Copyright (c) 2024 LunarG, Inc.
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
#include "module.h"
#include "gpu/shaders/gpu_error_codes.h"

namespace gpu {
namespace spirv {

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
uint32_t Pass::GetStageInfo(Function& function, BasicBlockIt target_block_it, InstructionIt& target_inst_it) {
    // Cached so only need to compute this once
    if (function.stage_info_id_ != 0) {
        return function.stage_info_id_;
    }

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
        stage_info[0] = module_.type_manager_.GetConstantUInt32(gpuav::glsl::kHeaderStageIdMultiEntryPoint).Id();
    } else {
        spv::ExecutionModel execution_model = spv::ExecutionModel(module_.entry_points_.begin()->get()->Operand(0));
        stage_info[0] = module_.type_manager_.GetConstantUInt32(execution_model).Id();

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
                const uint32_t load_id = create_load(spv::BuiltInGlobalInvocationId);

                for (uint32_t i = 0; i < 3; i++) {
                    const uint32_t extract_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, load_id, i}, &inst_it);
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
                const Type& vec3_type = module_.type_manager_.GetTypeVector(uint32_type, 3);
                const uint32_t load_id = create_load(spv::BuiltInTessCoord);
                const uint32_t bitcast_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpBitcast, {vec3_type.Id(), bitcast_id, load_id}, &inst_it);

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
    if ((*target_block_it)->GetLabelId() == block.GetLabelId()) {
        target_inst_it = FindTargetInstruction(block);
    }

    return function.stage_info_id_;
}

const Instruction* Pass::GetDecoration(uint32_t id, spv::Decoration decoration) {
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == id &&
            spv::Decoration(annotation->Word(2)) == decoration) {
            return annotation.get();
        }
    }
    return nullptr;
}

const Instruction* Pass::GetMemeberDecoration(uint32_t id, uint32_t member_index, spv::Decoration decoration) {
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpMemberDecorate && annotation->Word(1) == id && annotation->Word(2) == member_index &&
            spv::Decoration(annotation->Word(3)) == decoration) {
            return annotation.get();
        }
    }
    return nullptr;
}

// Find outermost buffer type and its access chain index.
// Because access chains indexes can be runtime values, we need to build arithmetic logic in the SPIR-V to get the runtime value of
// the indexing
uint32_t Pass::GetLastByte(const Instruction& var_inst, const Instruction& access_chain_inst, BasicBlock& block,
                           InstructionIt* inst_it) {
    const Type* pointer_type = module_.type_manager_.FindTypeById(var_inst.TypeId());
    const Type* descriptor_type = module_.type_manager_.FindTypeById(pointer_type->inst_.Word(3));

    uint32_t current_type_id = 0;
    uint32_t ac_word_index = 4;

    if (descriptor_type->spv_type_ == SpvType::kArray || descriptor_type->spv_type_ == SpvType::kRuntimeArray) {
        current_type_id = descriptor_type->inst_.Operand(0);
        ac_word_index++;
    } else if (descriptor_type->spv_type_ == SpvType::kStruct) {
        current_type_id = descriptor_type->Id();
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

    while (ac_word_index < access_chain_inst.Length()) {
        const uint32_t ac_index_id = access_chain_inst.Word(ac_word_index);
        uint32_t current_offset_id = 0;

        const Type* current_type = module_.type_manager_.FindTypeById(current_type_id);
        switch (current_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kRuntimeArray: {
                // Get array stride and multiply by current index
                uint32_t arr_stride = GetDecoration(current_type_id, spv::DecorationArrayStride)->Word(3);
                const uint32_t arr_stride_id = module_.type_manager_.GetConstantUInt32(arr_stride).Id();
                const uint32_t ac_index_id_32 = ConvertTo32(ac_index_id, block, inst_it);

                current_offset_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpIMul, {uint32_type.Id(), current_offset_id, arr_stride_id, ac_index_id_32}, inst_it);

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
                    const uint32_t col_stride = module_.type_manager_.FindTypeByteSize(component_type_id);
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
                    const uint32_t component_type_size = module_.type_manager_.FindTypeByteSize(component_type_id);
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
                uint32_t member_index = member_constant->inst_.Operand(0);
                uint32_t member_offset = GetMemeberDecoration(current_type_id, member_index, spv::DecorationOffset)->Word(4);
                current_offset_id = module_.type_manager_.GetConstantUInt32(member_offset).Id();

                // Look for matrix stride for this member if there is one. The matrix
                // stride is not on the matrix type, but in a OpMemberDecorate on the
                // enclosing struct type at the member index. If none found, reset
                // stride to 0.
                const Instruction* decoration_matrix_stride =
                    GetMemeberDecoration(current_type_id, member_index, spv::DecorationMatrixStride);
                matrix_stride = decoration_matrix_stride ? decoration_matrix_stride->Word(4) : 0;

                const Instruction* decoration_col_major =
                    GetMemeberDecoration(current_type_id, member_index, spv::DecorationColMajor);
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
    }

    // Add in offset of last byte of referenced object
    uint32_t bsize = module_.type_manager_.FindTypeByteSize(current_type_id, matrix_stride, col_major, in_matrix);
    uint32_t last = bsize - 1;

    const uint32_t last_id = module_.type_manager_.GetConstantUInt32(last).Id();

    const uint32_t new_sum_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpIAdd, {uint32_type.Id(), new_sum_id, sum_id, last_id}, inst_it);
    return new_sum_id;
}

// Generate code to convert integer id to 32bit, if needed.
uint32_t Pass::ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) {
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
uint32_t Pass::CastToUint32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) {
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

InstructionIt Pass::FindTargetInstruction(BasicBlock& block) const {
    const uint32_t target_id = target_instruction_->ResultId();
    for (auto inst_it = block.instructions_.begin(); inst_it != block.instructions_.end(); ++inst_it) {
        // This has to re-loop the entire block to find the instruction, using the ResultID, we can quickly compare
        if ((*inst_it)->ResultId() == target_id) {
            // Things like OpStore will have a result id of zero, so need to do deep instruction comparison
            if (*(*inst_it) == *target_instruction_) {
                return inst_it;
            }
        }
    }

    module_.InternalError(Name(), "failed to find instruction");
    return block.instructions_.end();
}

}  // namespace spirv
}  // namespace gpu