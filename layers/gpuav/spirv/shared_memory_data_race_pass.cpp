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
#include "type_manager.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_shared_memory_data_race_comp,
                                             instrumentation_shared_memory_data_race_comp_size,
                                             UseErrorPayloadVariable | SharedMemoryDataRace};

const static OfflineFunction kOfflineFunction[] = {
    {"init_shadow", instrumentation_shared_memory_data_race_comp_function_0_offset},
    {"do_store", instrumentation_shared_memory_data_race_comp_function_1_offset},
    {"do_load", instrumentation_shared_memory_data_race_comp_function_2_offset},
    {"do_atomic", instrumentation_shared_memory_data_race_comp_function_3_offset},
    {"do_coopmat_load", instrumentation_shared_memory_data_race_comp_function_4_offset},
    {"do_coopmat_store", instrumentation_shared_memory_data_race_comp_function_5_offset},
};

SharedMemoryDataRacePass::SharedMemoryDataRacePass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t SharedMemoryDataRacePass::GetLinkFunctionId(const InstructionMeta& meta) {
    return GetLinkFunction(link_function_id_[meta.function_idx], kOfflineFunction[meta.function_idx]);
}

// The goal of Function::FindInstruction is you should know the instruction is in the Function.
// For walking the indexes of an access chain in this pass, we want a global lookup
const Instruction* SharedMemoryDataRacePass::FindInstructionGlobal(const Function& function, uint32_t id) const {
    if (auto ret = function.FindInstruction(id)) {
        return ret;
    }
    if (auto ret = module_.type_manager_.FindConstantById(id)) {
        return &ret->inst_;
    }
    if (auto ret = module_.type_manager_.FindVariableById(id)) {
        return &ret->inst_;
    }
    return nullptr;
}

void SharedMemoryDataRacePass::CreateFunctionCall(const Function& function, BasicBlock& block, InstructionIt* inst_it,
                                                  const InstructionMeta& meta) {
    const uint32_t function_def = GetLinkFunctionId(meta);
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    if (meta.function_idx == INIT_SHADOW) {
        const uint32_t function_result = module_.TakeNextId();
        const uint32_t length_id = type_manager_.GetConstantUInt32(num_slots_).Id();
        block.CreateInstruction(spv::OpFunctionCall, {void_type, function_result, function_def, length_id, work_group_size_id_},
                                inst_it);
    } else {
        const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
        const uint32_t inst_position_id = type_manager_.GetConstantUInt32(inst_position).Id();

        const uint32_t variable_idx_id = type_manager_.GetConstantUInt32(meta.variable_idx).Id();
        for (uint32_t i = 0; i < meta.num_elements; ++i) {
            const uint32_t function_result = module_.TakeNextId();
            uint32_t start = type_manager_.GetConstantUInt32(meta.start + i).Id();

            // Compute additional parameters for coopmat load/store
            if (meta.function_idx == DO_COOPMAT_LOAD || meta.function_idx == DO_COOPMAT_STORE) {
                bool store = meta.function_idx == DO_COOPMAT_STORE;
                const Instruction& inst = *((*inst_it)->get());
                // get coopmat type either from resulttype id or object id
                const Type* coopmat_type;
                if (store) {
                    const uint32_t object_id = inst.Word(2);
                    const Instruction* object_inst = FindInstructionGlobal(function, object_id);
                    coopmat_type = type_manager_.FindTypeById(object_inst->TypeId());
                } else {
                    coopmat_type = type_manager_.FindTypeById(inst.TypeId());
                }
                const uint32_t scope_id = type_manager_.GetConstantUInt32FromId(coopmat_type->inst_.Word(3));
                uint32_t rows_id = type_manager_.GetConstantUInt32FromId(coopmat_type->inst_.Word(4));
                uint32_t cols_id = type_manager_.GetConstantUInt32FromId(coopmat_type->inst_.Word(5));
                const uint32_t memory_layout_id = type_manager_.GetConstantUInt32FromId(inst.Word(store ? 3 : 4));
                uint32_t stride_id = type_manager_.GetConstantUInt32FromId(inst.Word(store ? 4 : 5));

                const uint32_t ptr_id = inst.Operand(0);  // works with both store and loads

                // get type of pointee
                auto ptr_inst = FindInstructionGlobal(function, ptr_id);
                const Type* ptr_type = type_manager_.FindTypeById(ptr_inst->TypeId());
                const Type* ptr_elem_type = type_manager_.FindChildType(*ptr_type, 0);
                const Type* scalar_elem_type = ptr_elem_type;

                const Type& uint32_type = type_manager_.GetTypeInt(32, false);

                // if the pointer is to a vector type, scale stride_id by the vector size
                if (ptr_elem_type->VectorSize()) {
                    uint32_t vec_size = ptr_elem_type->VectorSize();

                    uint32_t next_id = module_.TakeNextId();
                    block.CreateInstruction(spv::OpIMul,
                                            {uint32_type.Id(), next_id, stride_id, type_manager_.GetConstantUInt32(vec_size).Id()},
                                            inst_it);
                    stride_id = next_id;

                    scalar_elem_type = type_manager_.FindChildType(*ptr_elem_type, 0);
                }

                // If the size of the scalar element type doesn't match the component size,
                // scale the rows or cols up or down to touch the correct number of elements
                uint32_t access_size = type_manager_.GetTypeBytesSize(*scalar_elem_type);
                uint32_t coopmat_component_size =
                    type_manager_.GetTypeBytesSize(*type_manager_.FindTypeById(coopmat_type->inst_.Word(2)));
                if (access_size != coopmat_component_size) {
                    uint32_t memory_layout = type_manager_.FindConstantById(memory_layout_id)->GetValueUint32();
                    uint32_t& rows_or_cols_id = memory_layout == spv::CooperativeMatrixLayoutRowMajorKHR ? cols_id : rows_id;
                    if (access_size > coopmat_component_size) {
                        uint32_t next_id = module_.TakeNextId();
                        block.CreateInstruction(spv::OpUDiv,
                                                {uint32_type.Id(), next_id, rows_or_cols_id,
                                                 type_manager_.GetConstantUInt32(access_size / coopmat_component_size).Id()},
                                                inst_it);
                        rows_or_cols_id = next_id;
                    } else {
                        uint32_t next_id = module_.TakeNextId();
                        block.CreateInstruction(spv::OpIMul,
                                                {uint32_type.Id(), next_id, rows_or_cols_id,
                                                 type_manager_.GetConstantUInt32(coopmat_component_size / access_size).Id()},
                                                inst_it);
                        rows_or_cols_id = next_id;
                    }
                }

                block.CreateInstruction(
                    spv::OpFunctionCall,
                    {void_type, function_result, function_def, start, meta.access_chain_idx_id, inst_position_id, variable_idx_id,
                     scope_id, rows_id, cols_id, memory_layout_id, stride_id, work_group_size_id_},
                    inst_it);
            } else {
                block.CreateInstruction(
                    spv::OpFunctionCall,
                    {void_type, function_result, function_def, start, meta.access_chain_idx_id, inst_position_id, variable_idx_id},
                    inst_it);
            }
        }

        // Only need logging when we add non init_shadow call
        module_.need_log_error_ = true;
    }
}

bool SharedMemoryDataRacePass::RequiresInstrumentation(const Function& function, BasicBlock& block, InstructionIt& inst_it,
                                                       const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();

    if (!AtomicOperation(opcode) && !IsValueIn(opcode, {spv::OpLoad, spv::OpStore, spv::OpControlBarrier,
                                                        spv::OpCooperativeMatrixLoadKHR, spv::OpCooperativeMatrixStoreKHR})) {
        return false;
    }
    meta.target_instruction = &inst;

    // For Barriers, we will need to call init_shadow again
    if (opcode == spv::OpControlBarrier) {
        // Must be scope=Workgroup, semantics=AcquireRelease
        const uint32_t scope_id = inst.Word(1);
        const uint32_t semantics_id = inst.Word(3);
        uint32_t scope = type_manager_.FindConstantById(scope_id)->GetValueUint32();
        uint32_t semantics = type_manager_.FindConstantById(semantics_id)->GetValueUint32();
        if (scope != spv::ScopeWorkgroup || (semantics & spv::MemorySemanticsAcquireReleaseMask) == 0) {
            return false;
        }
        meta.function_idx = INIT_SHADOW;
        return true;
    }

    const uint32_t ptr_id = inst.Operand(0);  // works with both store and loads

    std::vector<const Instruction*> access_chains;
    const Variable* variable = type_manager_.FindVariableById(ptr_id);
    const Instruction* access_chain_inst = FindInstructionGlobal(function, ptr_id);
    // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
    while (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
        // inserting in front allows us to walk over the loop from the front
        access_chains.insert(access_chains.begin(), access_chain_inst);
        const uint32_t access_chain_base_id = access_chain_inst->Operand(0);
        variable = type_manager_.FindVariableById(access_chain_base_id);
        if (variable) {
            break;  // found
        }
        access_chain_inst = FindInstructionGlobal(function, access_chain_base_id);
    }
    if (!variable) {
        return false;
    } else if (variable->StorageClass() != spv::StorageClassWorkgroup) {
        return false;
    }

    const Type& uint32_type = type_manager_.GetTypeInt(32, false);
    uint32_t offset_id = type_manager_.GetConstantZeroUint32().Id();

    // ptr_elem_type will point to the portion of the variable being accessed. Initialize it
    // to the variable's pointee type in case of no access chains.
    const Type* ptr_elem_type = type_manager_.FindChildType(*type_manager_.FindTypeById(variable->inst_.Word(1)), 0);
    for (auto ac : access_chains) {
        auto ptr = FindInstructionGlobal(function, ac->Word(3));
        const Type* base_ptr_type = type_manager_.FindTypeById(ptr->Word(1));

        // Get the base pointer pointee type.
        ptr_elem_type = type_manager_.FindChildType(*base_ptr_type, 0);

        for (uint32_t i = 4; i < ac->Length(); ++i) {
            uint32_t idx_id = ac->Word(i);
            auto idx_inst = FindInstructionGlobal(function, idx_id);
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
                    block.CreateInstruction(
                        spv::OpIAdd, {uint32_type.Id(), new_id, offset_id, type_manager_.GetConstantUInt32(off).Id()}, &inst_it);
                    offset_id = new_id;
                    ptr_elem_type = type_manager_.FindChildType(*ptr_elem_type, idx_u32);
                } break;
                default: {
                    // offset_id += (stride of first member) * idx
                    uint32_t stride = type_manager_.GetNumScalarElementsBeforeCompositeMember(*ptr_elem_type, 1);

                    uint32_t stride_times_idx_id = module_.TakeNextId();
                    block.CreateInstruction(
                        spv::OpIMul, {uint32_type.Id(), stride_times_idx_id, idx_id, type_manager_.GetConstantUInt32(stride).Id()},
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
    meta.start = slot_start_[variable->Id()];
    meta.num_elements = type_manager_.GetNumScalarElements(*ptr_elem_type);
    meta.variable_idx = variable->Id();

    switch (opcode) {
        case spv::OpLoad:
            meta.function_idx = DO_LOAD;
            break;
        case spv::OpStore:
            meta.function_idx = DO_STORE;
            break;
        case spv::OpCooperativeMatrixLoadKHR:
            meta.function_idx = DO_COOPMAT_LOAD;
            meta.num_elements = 1;
            break;
        case spv::OpCooperativeMatrixStoreKHR:
            meta.function_idx = DO_COOPMAT_STORE;
            meta.num_elements = 1;
            break;
        default:
            meta.function_idx = DO_ATOMIC;
            break;
    }

    return true;
}

// This is possible because all spec constant must have been frozen by now
uint32_t SharedMemoryDataRacePass::GetWorkgroupSize() {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(2) == spv::DecorationBuiltIn &&
            annotation->Word(3) == spv::BuiltInWorkgroupSize) {
            uint32_t wg_size_id = annotation->Word(1);
            const Constant* wg_constant = type_manager_.FindConstantById(wg_size_id);
            assert(wg_constant && wg_constant->inst_.Opcode() == spv::OpConstantComposite);
            uint32_t x = type_manager_.FindConstantById(wg_constant->inst_.Word(3))->GetValueUint32();
            uint32_t y = type_manager_.FindConstantById(wg_constant->inst_.Word(4))->GetValueUint32();
            uint32_t z = type_manager_.FindConstantById(wg_constant->inst_.Word(5))->GetValueUint32();
            return x * y * z;
        }
    }

    // if there's no WorkgroupSize, find LocalSize or LocalSizeId
    for (const auto& execution_mode_inst : module_.execution_modes_) {
        if (execution_mode_inst->Word(1) != module_.target_entry_point_id_) {
            continue;
        }
        const spv::ExecutionMode mode = (spv::ExecutionMode)execution_mode_inst->Word(2);

        if (mode == spv::ExecutionModeLocalSize) {
            uint32_t x = execution_mode_inst->Word(3);
            uint32_t y = execution_mode_inst->Word(4);
            uint32_t z = execution_mode_inst->Word(5);
            return x * y * z;
        } else if (mode == spv::ExecutionModeLocalSizeId) {
            uint32_t x = type_manager_.FindConstantById(execution_mode_inst->Word(3))->GetValueUint32();
            uint32_t y = type_manager_.FindConstantById(execution_mode_inst->Word(4))->GetValueUint32();
            uint32_t z = type_manager_.FindConstantById(execution_mode_inst->Word(5))->GetValueUint32();
            return x * y * z;
        }
    }
    assert(false);
    return 0;
}

bool SharedMemoryDataRacePass::Instrument() {
    if (module_.interface_.entry_point_stage != VK_SHADER_STAGE_COMPUTE_BIT) {
        return false;
    }

    // relies on some subgroup functionality
    const uint32_t spirv_version_1_3 = 0x00010300;
    if (module_.header_.version < spirv_version_1_3) {
        return false;
    }

    if (module_.HasCapability(spv::CapabilityWorkgroupMemoryExplicitLayoutKHR)) {
        return false;
    }

    const std::vector<const Variable*>& shmem_vars = type_manager_.GetSharedMemoryVariables();

    // Since we don't support WorkgroupMemoryExplicitLayoutKHR, we don't need to worry about Aliased blocks
    uint32_t shared_memory_size = 0;

    // Compute how much shared memory is needed.
    for (const Variable* v : shmem_vars) {
        const uint32_t v_id = v->Id();
        slot_start_[v_id] = num_slots_;
        const Type* pointee_type = v->PointerType(type_manager_);
        uint32_t num_scalar_elements = type_manager_.GetNumScalarElements(*pointee_type);
        assert(num_scalar_elements != 0);
        num_slots_ += num_scalar_elements;

        shared_memory_size += type_manager_.GetTypeBytesSize(*pointee_type);
    }

    if (num_slots_ == 0) {
        return false;  // no shared memory being used
    }

    // Bail if we would overflow the limit
    if (shared_memory_size + (num_slots_ * sizeof(uint32_t)) > module_.settings_.max_compute_shared_memory_size) {
        return false;
    }

    // Reserve the ID, will add OpVariable when confirmed shared memory is used
    module_.shared_memory_shadow_variable_id_ = module_.TakeNextId();

    // Need size to init the shadow memory
    const uint32_t work_group_size = GetWorkgroupSize();
    work_group_size_id_ = type_manager_.GetConstantUInt32(work_group_size).Id();

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
                meta.function_idx = INIT_SHADOW;

                CreateFunctionCall(function, current_block, &inst_it, meta);
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

                CreateFunctionCall(function, current_block, &inst_it, meta);
            }
        }
    }

    // Need to actutally add the shadow array variable type into the module
    if (instrumentations_count_ != 0) {
        const Type& uint32_type = type_manager_.GetTypeInt(32, false);
        const Constant& array_size = type_manager_.GetConstantUInt32(num_slots_);
        const Type& uint32_arr_type = type_manager_.GetTypeArray(uint32_type, array_size);
        const Type& uint32_arr_ptr_type = type_manager_.GetTypePointer(spv::StorageClassWorkgroup, uint32_arr_type);

        auto shadow_var = std::make_unique<Instruction>(4, spv::OpVariable);
        shadow_var->Fill({uint32_arr_ptr_type.Id(), module_.shared_memory_shadow_variable_id_, spv::StorageClassWorkgroup});
        type_manager_.AddVariable(std::move(shadow_var), uint32_arr_ptr_type);
    }

    return instrumentations_count_ != 0;
}

void SharedMemoryDataRacePass::PrintDebugInfo() const {
    std::cout << "SharedMemoryDataRacePass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
