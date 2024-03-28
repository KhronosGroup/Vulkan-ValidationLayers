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
#include "type_manager.h"
#include "gpu_shaders/gpu_error_codes.h"
#include <spirv/unified1/spirv.hpp>

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

        for (auto& entry_point : module_.entry_points_) {
            entry_point->AppendWord(built_in_variable->Id());
        }
    }

    return *built_in_variable;
}

// To reduce having to load this information everytime we do a OpFunctionCall, instead just create it once per Function block and
// reference it each time
uint32_t Pass::GetStageInfo(Function& function) {
    // Cached so only need to compute this once
    if (function.stage_info_id_ != 0) {
        return function.stage_info_id_;
    }

    // Get the first block of function to add stage info
    BasicBlock& block = *function.blocks_[0];

    // All OpVariable instructions in a function must be the first instructions in the first block
    InstructionIt inst_it;
    for (inst_it = block.instructions_.begin(); inst_it != block.instructions_.end(); ++inst_it) {
        if ((*inst_it)->Opcode() != spv::OpLabel && (*inst_it)->Opcode() != spv::OpVariable) {
            break;
        }
    }

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

BasicBlockIt Pass::InjectFunctionCheck(Function* function, BasicBlockIt block_it, InstructionIt inst_it) {
    // We turn the block into 4 separate blocks
    block_it = function->InsertNewBlock(block_it);
    block_it = function->InsertNewBlock(block_it);
    block_it = function->InsertNewBlock(block_it);
    BasicBlock& original_block = **(std::prev(block_it, 3));
    // Where we call targeted instruction if it is valid
    BasicBlock& valid_block = **(std::prev(block_it, 2));
    // will be an empty block, used for the Phi node, even if no result, create for simplicity
    BasicBlock& invalid_block = **(std::prev(block_it, 1));
    // All the remaining block instructions after targeted instruction
    BasicBlock& merge_block = **block_it;

    const uint32_t original_label = original_block.GetLabelId();
    const uint32_t valid_block_label = valid_block.GetLabelId();
    const uint32_t invalid_block_label = invalid_block.GetLabelId();
    const uint32_t merge_block_label = merge_block.GetLabelId();

    // need to preserve the control-flow of how things, like a OpPhi, are accessed from a predecessor block
    function->ReplaceAllUsesWith(original_label, merge_block_label);

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
        function->ReplaceAllUsesWith(target_inst_id, phi_id);
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

    // Go back to original Block and add function call and branch from the bool result
    const uint32_t function_result = CreateFunctionCall(original_block);

    original_block.CreateInstruction(spv::OpSelectionMerge, {merge_block_label, spv::SelectionControlMaskNone});
    original_block.CreateInstruction(spv::OpBranchConditional, {function_result, valid_block_label, invalid_block_label});

    // clear values incase multiple calls are made
    Reset();

    return block_it;
}

void Pass::Run() {
    for (const auto& function : module_.functions_) {
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            if ((*block_it)->loop_header_) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = (*block_it)->instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (AnalyzeInstruction(*(function.get()), *(inst_it->get()))) {
                    block_it = InjectFunctionCheck(function.get(), block_it, inst_it);

                    // will start searching again from newly split merge block
                    block_it--;
                    break;
                }
            }
        }
    }
}

}  // namespace spirv
}  // namespace gpuav