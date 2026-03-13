/* Copyright (c) 2026 LunarG, Inc.
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

#include "shared_memory_oob_pass.h"
#include "module.h"
#include <iostream>
#include <spirv/unified1/spirv.hpp>

#include "generated/gpuav_offline_spirv.h"
#include "type_manager.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_shared_memory_oob_comp, instrumentation_shared_memory_oob_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_shared_memory_oob",
                                                 instrumentation_shared_memory_oob_comp_function_0_offset};

SharedMemoryOobPass::SharedMemoryOobPass(Module& module) : Pass(module, kOfflineModule) {}

void SharedMemoryOobPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_def = GetLinkFunction(link_function_id_, kOfflineFunction);
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();
    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.GetConstantUInt32(inst_position).Id();
    const uint32_t variable_id_const = type_manager_.GetConstantUInt32(meta.variable_id).Id();

    for (const auto& check : meta.checks) {
        const uint32_t function_result = module_.TakeNextId();
        const uint32_t bound_id = type_manager_.GetConstantUInt32(check.bound).Id();

        block.CreateInstruction(
            spv::OpFunctionCall,
            {void_type, function_result, function_def, check.index_id, bound_id, inst_position_id, variable_id_const}, inst_it);
    }

    module_.need_log_error_ = true;
}

bool SharedMemoryOobPass::RequiresInstrumentation(const Function& function, BasicBlock& block, InstructionIt& inst_it,
                                                  const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();

    if (opcode == spv::OpVectorExtractDynamic || opcode == spv::OpVectorInsertDynamic) {
        meta.target_instruction = &inst;

        const uint32_t vector_id = inst.Operand(0);
        const Instruction* vector_inst = function.FindInstruction(vector_id);
        if (!vector_inst) return false;

        // OpVectorExtract/InsertDynamic operate on values, not pointers, so the vector operand
        // will be an OpLoad from the shared memory variable. We trace through the OpLoad to find
        // the source pointer and verify it originates from workgroup storage. Even though the OOB
        // occurs on the loaded value, we still report it because the vector's bounds are determined
        // by the shared memory declaration and the index is checked before the access.
        if (vector_inst->Opcode() != spv::OpLoad) return false;
        const uint32_t ptr_id = vector_inst->Operand(0);

        // Walk access chains to find the base variable
        const Variable* variable = type_manager_.FindVariableById(ptr_id);
        const Instruction* access_chain_inst = function.FindInstruction(ptr_id);
        while (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
            const uint32_t base_id = access_chain_inst->Operand(0);
            variable = type_manager_.FindVariableById(base_id);
            if (variable) break;
            access_chain_inst = function.FindInstruction(base_id);
        }
        if (!variable || variable->StorageClass() != spv::StorageClassWorkgroup) return false;

        const Type* vector_type = type_manager_.FindTypeById(vector_inst->TypeId());
        if (!vector_type || (vector_type->spv_type_ != SpvType::kVector && vector_type->spv_type_ != SpvType::kVectorIdEXT))
            return false;

        uint32_t index_id;
        if (opcode == spv::OpVectorExtractDynamic) {
            index_id = inst.Operand(1);
        } else {
            index_id = inst.Operand(2);
        }

        meta.variable_id = variable->Id();
        index_id = CastToUint32(index_id, block, &inst_it);
        meta.checks.push_back({index_id, vector_type->meta_.vector.component_count});
        return !meta.checks.empty();
    }

    if (!inst.IsNonPtrAccessChain()) return false;
    meta.target_instruction = &inst;

    // Walk chained access chains to find the base OpVariable
    const uint32_t base_id = inst.Operand(0);
    const Variable* variable = type_manager_.FindVariableById(base_id);
    const Instruction* chain_inst = function.FindInstruction(base_id);
    while (chain_inst && chain_inst->IsNonPtrAccessChain()) {
        const uint32_t chain_base_id = chain_inst->Operand(0);
        variable = type_manager_.FindVariableById(chain_base_id);
        if (variable) break;
        chain_inst = function.FindInstruction(chain_base_id);
    }

    if (!variable || variable->StorageClass() != spv::StorageClassWorkgroup) return false;
    meta.variable_id = variable->Id();

    // Start the type walk from the direct base operand's pointee type, not the root variable's type,
    // because Slang (and potentially other compilers) can chain access chains.
    const Instruction* base_inst = function.FindInstruction(base_id);
    const Type* base_ptr_type = base_inst ? type_manager_.FindTypeById(base_inst->TypeId()) : nullptr;
    if (!base_ptr_type) {
        assert(variable->Id() == base_id);
        base_ptr_type = type_manager_.FindTypeById(variable->inst_.TypeId());
    }
    assert(base_ptr_type);
    const Type* pointee_type = type_manager_.FindChildType(*base_ptr_type, 0);

    // Word(3) is the base pointer, Word(4..Length()-1) are indices
    for (uint32_t i = 4; i < inst.Length(); ++i) {
        uint32_t idx_id = inst.Word(i);

        switch (pointee_type->spv_type_) {
            case SpvType::kArray:
            case SpvType::kVector:
            case SpvType::kVectorIdEXT: {
                idx_id = CastToUint32(idx_id, block, &inst_it);
                const uint32_t bound = (pointee_type->spv_type_ == SpvType::kArray) ? pointee_type->meta_.array.length
                                                                                    : pointee_type->meta_.vector.component_count;
                meta.checks.push_back({idx_id, bound});
                pointee_type = type_manager_.FindChildType(*pointee_type, 0);
            } break;
            case SpvType::kStruct: {
                // Struct member indices are always constant -- advance type but no check needed
                auto idx_c = type_manager_.FindConstantById(idx_id);
                uint32_t member_idx = idx_c->GetValueUint32();
                pointee_type = type_manager_.FindChildType(*pointee_type, member_idx);
            } break;
            case SpvType::kMatrix: {
                // Matrix column indices -- advance type, skip check
                pointee_type = type_manager_.FindChildType(*pointee_type, 0);
            } break;
            default:
                pointee_type = type_manager_.FindChildType(*pointee_type, 0);
                break;
        }
    }

    return !meta.checks.empty();
}

bool SharedMemoryOobPass::Instrument() {
    if (module_.interface_.entry_point_stage != VK_SHADER_STAGE_COMPUTE_BIT &&
        module_.interface_.entry_point_stage != VK_SHADER_STAGE_TASK_BIT_EXT &&
        module_.interface_.entry_point_stage != VK_SHADER_STAGE_MESH_BIT_EXT) {
        return false;
    }

    const std::vector<const Variable*>& shmem_vars = type_manager_.GetSharedMemoryVariables();
    if (shmem_vars.empty()) {
        return false;
    }

    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }
        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            if (current_block.IsLoopHeader()) {
                continue;
            }

            auto& block_instructions = current_block.instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                const Instruction& inst = *(inst_it->get());

                InstructionMeta meta;
                if (!RequiresInstrumentation(function, current_block, inst_it, inst, meta)) {
                    continue;
                }

                if (IsMaxInstrumentationsCount()) {
                    continue;
                }
                instrumentations_count_++;

                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    return instrumentations_count_ != 0;
}

void SharedMemoryOobPass::PrintDebugInfo() const {
    std::cout << "SharedMemoryOobPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
