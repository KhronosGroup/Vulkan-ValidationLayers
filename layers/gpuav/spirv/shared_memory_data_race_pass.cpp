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

#include "shared_memory_data_race_pass.h"
#include "containers/container_utils.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "state_tracker/shader_module.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_shared_memory_data_race_comp,
                                             instrumentation_shared_memory_data_race_comp_size,
                                             UseErrorPayloadVariable | SharedMemoryDataRace};

const static OfflineFunction kOfflineFunction[4] = {
    {"init_shadow", instrumentation_shared_memory_data_race_comp_function_0_offset},
    {"do_store", instrumentation_shared_memory_data_race_comp_function_1_offset},
    {"do_load", instrumentation_shared_memory_data_race_comp_function_2_offset},
    {"do_atomic", instrumentation_shared_memory_data_race_comp_function_3_offset},
};

SharedMemoryDataRacePass::SharedMemoryDataRacePass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t SharedMemoryDataRacePass::GetLinkFunctionId(const InstructionMeta& meta) {
    return GetLinkFunction(link_function_id_[meta.function_idx], kOfflineFunction[meta.function_idx]);
}

void SharedMemoryDataRacePass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_def = GetLinkFunctionId(meta);
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.GetConstantUInt32(inst_position).Id();

    if (meta.function_idx == 0) {
        // workgroupsize can be a constant, so GetBuiltInVariable may insert a duplicate
        uint32_t wg_size_id = 0;
        for (const auto& annotation : module_.annotations_) {
            if (annotation->Opcode() == spv::OpDecorate && annotation->Word(2) == spv::DecorationBuiltIn &&
                annotation->Word(3) == spv::BuiltInWorkgroupSize) {
                wg_size_id = annotation->Word(1);
                break;
            }
        }

        // if there's no WorkgroupSize constant, create a constant from localsize
        if (!wg_size_id) {
            const Type& uint_32_type = type_manager_.GetTypeInt(32, 0);
            const Type& uvec3_type = type_manager_.GetTypeVector(uint_32_type, 3);
            uint32_t local_x = 1;
            uint32_t local_y = 1;
            uint32_t local_z = 1;

            for (const auto& execution_mode_inst : module_.execution_modes_) {
                if (execution_mode_inst->Word(1) != module_.target_entry_point_id_) {
                    continue;
                }

                const spv::ExecutionMode mode = (spv::ExecutionMode)execution_mode_inst->Word(2);
                if (mode == spv::ExecutionModeLocalSize) {
                    local_x = execution_mode_inst->Word(3);
                    local_y = execution_mode_inst->Word(4);
                    local_z = execution_mode_inst->Word(5);
                }
            }

            const uint32_t local_x_id = type_manager_.GetConstantUInt32(local_x).Id();
            const uint32_t local_y_id = type_manager_.GetConstantUInt32(local_y).Id();
            const uint32_t local_z_id = type_manager_.GetConstantUInt32(local_z).Id();

            wg_size_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(6, spv::OpConstantComposite);
            new_inst->Fill({uvec3_type.Id(), wg_size_id, local_x_id, local_y_id, local_z_id});
            type_manager_.AddConstant(std::move(new_inst), uvec3_type);
        }

        const uint32_t function_result = module_.TakeNextId();
        const uint32_t length_id = type_manager_.GetConstantUInt32(num_slots).Id();
        block.CreateInstruction(spv::OpFunctionCall, {void_type, function_result, function_def, length_id, wg_size_id}, inst_it);
    } else {
        const uint32_t variable_idx_id = type_manager_.GetConstantUInt32(meta.variable_idx).Id();
        for (uint32_t i = 0; i < meta.num_elements; ++i) {
            const uint32_t function_result = module_.TakeNextId();
            uint32_t start = type_manager_.GetConstantUInt32(meta.start + i).Id();
            block.CreateInstruction(
                spv::OpFunctionCall,
                {void_type, function_result, function_def, start, meta.access_chain_idx_id, inst_position_id, variable_idx_id},
                inst_it);
        }
    }
    module_.need_log_error_ = true;
}

bool SharedMemoryDataRacePass::RequiresInstrumentation(const Function& function, BasicBlock& block, InstructionIt& inst_it,
                                                       const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();

    if (!IsValueIn(opcode, {spv::OpLoad,        spv::OpStore,          spv::OpControlBarrier,   spv::OpAtomicLoad,
                            spv::OpAtomicStore, spv::OpAtomicExchange, spv::OpAtomicIIncrement, spv::OpAtomicIDecrement,
                            spv::OpAtomicIAdd,  spv::OpAtomicISub,     spv::OpAtomicSMin,       spv::OpAtomicUMin,
                            spv::OpAtomicSMax,  spv::OpAtomicUMax,     spv::OpAtomicAnd,        spv::OpAtomicOr,
                            spv::OpAtomicXor,   spv::OpAtomicFMinEXT,  spv::OpAtomicFMaxEXT,    spv::OpAtomicFAddEXT})) {
        return false;
    }
    meta.target_instruction = &inst;

    if (opcode != spv::OpControlBarrier) {
        uint32_t ptr_id = opcode == spv::OpStore ? inst.Word(1) : inst.Word(3);

        std::vector<const Instruction*> access_chains;
        const Variable* variable = type_manager_.FindVariableById(ptr_id);
        const Instruction* access_chain_inst = function.FindInstruction(ptr_id);
        // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
        while (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
            access_chains.insert(access_chains.begin(), access_chain_inst);
            const uint32_t access_chain_base_id = access_chain_inst->Operand(0);
            variable = type_manager_.FindVariableById(access_chain_base_id);
            if (variable) {
                break;  // found
            }
            access_chain_inst = function.FindInstruction(access_chain_base_id);
        }
        if (!variable) {
            return false;
        }

        const uint32_t storage_class = variable->StorageClass();
        if (storage_class != spv::StorageClassWorkgroup) {
            return false;
        }

        const Type& uint32_type = type_manager_.GetTypeInt(32, false);
        uint32_t offset_id = type_manager_.GetConstantUInt32(0).Id();

        // ptr_elem_type will point to the portion of the variable being accessed. Initialize it
        // to the variable's pointee type in case of no access chains.
        const Type* ptr_elem_type = type_manager_.FindChildType(*type_manager_.FindTypeById(variable->inst_.Word(1)), 0);
        for (auto ac : access_chains) {
            auto ptr = function.FindInstruction(ac->Word(3));
            const Type* base_ptr_type = type_manager_.FindTypeById(ptr->Word(1));

            // Get the base pointer pointee type.
            ptr_elem_type = type_manager_.FindChildType(*base_ptr_type, 0);

            for (uint32_t i = 4; i < access_chain_inst->Length(); ++i) {
                uint32_t idx_id = access_chain_inst->Word(i);
                auto idx_inst = function.FindInstruction(idx_id);
                auto idx_type = type_manager_.FindTypeById(idx_inst->Word(1));
                assert(idx_type->inst_.Opcode() == spv::OpTypeInt);
                // convert to u32 if needed
                if (idx_type->inst_.Word(2) != 32) {
                    uint32_t new_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpUConvert, {uint32_type.Id(), new_id, idx_id}, &inst_it);
                    idx_id = new_id;
                }

                switch (ptr_elem_type->spv_type_) {
                    case SpvType::kStruct: {
                        // offset_id += offsetof(struct member idx)
                        auto idx_c = type_manager_.FindConstantById(idx_id);
                        uint32_t idx_u32 = idx_c->GetValueUint32();
                        uint32_t off = type_manager_.GetNumScalarElementsBeforeCompositeMember(*ptr_elem_type, idx_u32);
                        uint32_t new_id = module_.TakeNextId();
                        block.CreateInstruction(spv::OpIAdd,
                                                {uint32_type.Id(), new_id, offset_id, type_manager_.GetConstantUInt32(off).Id()},
                                                &inst_it);
                        offset_id = new_id;
                        ptr_elem_type = type_manager_.FindChildType(*ptr_elem_type, idx_u32);
                    } break;
                    default: {
                        // offset_id += (stride of first member) * idx
                        uint32_t stride = type_manager_.GetNumScalarElementsBeforeCompositeMember(*ptr_elem_type, 1);

                        uint32_t stride_times_idx_id = module_.TakeNextId();
                        block.CreateInstruction(
                            spv::OpIMul,
                            {uint32_type.Id(), stride_times_idx_id, idx_id, type_manager_.GetConstantUInt32(stride).Id()},
                            &inst_it);

                        uint32_t new_id = module_.TakeNextId();
                        block.CreateInstruction(spv::OpIAdd, {uint32_type.Id(), new_id, offset_id, stride_times_idx_id}, &inst_it);
                        offset_id = new_id;
                        ptr_elem_type = type_manager_.FindChildType(*ptr_elem_type, 0);
                    } break;
                }
            }
        }

        meta.access_chain_idx_id = offset_id;
        meta.start = slot_start[variable];
        meta.num_elements = type_manager_.GetNumScalarElements(*ptr_elem_type);
        meta.variable_idx = variable_to_idx[variable];
    }

    if (opcode == spv::OpControlBarrier) {
        // Must be scope=Workgroup, semantics=AcquireRelease
        const uint32_t scope_id = inst.Word(1);
        const uint32_t semantics_id = inst.Word(3);
        uint32_t scope = type_manager_.FindConstantById(scope_id)->GetValueUint32();
        uint32_t semantics = type_manager_.FindConstantById(semantics_id)->GetValueUint32();
        if (scope != spv::ScopeWorkgroup || (semantics & spv::MemorySemanticsAcquireReleaseMask) == 0) {
            return false;
        }
    }

    switch (opcode) {
        case spv::OpLoad:
            meta.function_idx = 2;  // do_load
            break;
        case spv::OpStore:
            meta.function_idx = 1;  // do_store
            break;
        case spv::OpControlBarrier:
            meta.function_idx = 0;  // init_shadow
            break;
        default:
            meta.function_idx = 3;  // do_atomic
            break;
    }

    return true;
}

bool SharedMemoryDataRacePass::Instrument() {
    if (module_.interface_.entry_point_stage != VK_SHADER_STAGE_COMPUTE_BIT) {
        return false;
    }

    if (module_.HasCapability(spv::CapabilityWorkgroupMemoryExplicitLayoutKHR)) {
        return false;
    }

    const std::vector<const Variable*>& shmem_vars = type_manager_.GetSharedMemoryVariables();

    uint32_t variable_idx = 0;
    // Compute how much shared memory is needed.
    for (auto& v : shmem_vars) {
        const Type* pointee_type = v->PointerType(type_manager_);
        slot_start[v] = num_slots;
        variable_to_idx[v] = variable_idx++;
        uint32_t num_scalar_elements = type_manager_.GetNumScalarElements(*pointee_type);
        assert(num_scalar_elements != 0);
        num_slots += num_scalar_elements;
    }

    if (num_slots == 0) {
        return false;
    }

    const uint32_t shared_memory_size = module_.interface_.core_module->CalculateWorkgroupSharedMemory();
    // Bail if we would overflow the limit
    if (shared_memory_size + num_slots * sizeof(uint32_t) > module_.settings_.max_compute_shared_memory_size) {
        return false;
    }

    auto& uint32_ty = type_manager_.GetTypeInt(32, false);
    auto& uint32_arr_ty = type_manager_.GetTypeArray(uint32_ty, type_manager_.GetConstantUInt32(num_slots));
    auto& uint32_arr_ptr_ty = type_manager_.GetTypePointer(spv::StorageClassWorkgroup, uint32_arr_ty);

    module_.shared_memory_shadow_variable_id_ = module_.TakeNextId();

    // We've already checked that there are workgroup variables, they will
    // very likely be used, so just add the shadow memory now.
    auto shadow_var = std::make_unique<Instruction>(4, spv::OpVariable);
    shadow_var->Fill({uint32_arr_ptr_ty.Id(), module_.shared_memory_shadow_variable_id_, spv::StorageClassWorkgroup});
    type_manager_.AddVariable(std::move(shadow_var), uint32_arr_ptr_ty);

    // Can safely loop function list as there is no injecting of new Functions until linking time
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
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = current_block.instructions_;

            // Call init_shadow at the start of the entry point
            if (function.id_ == module_.target_entry_point_id_ && block_it == function.blocks_.begin()) {
                InstructionIt inst_it = current_block.GetFirstInjectableInstrution();

                InstructionMeta meta;
                meta.target_instruction = &*(inst_it->get());
                meta.function_idx = 0;  // init_shadow

                CreateFunctionCall(current_block, &inst_it, meta);
                instrumentations_count_++;
            }

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, current_block, inst_it, *(inst_it->get()), meta)) {
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
    return true;
}

void SharedMemoryDataRacePass::PrintDebugInfo() const {
    std::cout << "SharedMemoryDataRacePass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
