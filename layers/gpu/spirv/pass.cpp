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

namespace gpuav {
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
            default: {
                assert(false && "unsupported stage");
            } break;
        }
    }

    function.stage_info_id_ = module_.TakeNextId();
    block.CreateInstruction(spv::OpCompositeConstruct,
                            {uvec4_type.Id(), function.stage_info_id_, stage_info[0], stage_info[1], stage_info[2], stage_info[3]},
                            &inst_it);

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

// Generate code to convert integer id to 32bit, if needed.
uint32_t Pass::ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) {
    // Find type doing the indexing into the access chain
    const Type* type = nullptr;
    const Constant* constant = module_.type_manager_.FindConstantById(id);
    if (constant) {
        type = &constant->type_;
    } else {
        const Instruction* inst = block.function_.FindInstruction(id);
        type = module_.type_manager_.FindTypeById(inst->TypeId());
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
        type = module_.type_manager_.FindTypeById(inst->TypeId());
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
    assert(false);
    return block.instructions_.end();
}

}  // namespace spirv
}  // namespace gpuav