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

#include "buffer_device_address_pass.h"
#include "link.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>
#include "utils/math_utils.h"
#include "gpuav/shaders/gpuav_error_header.h"

#include "generated/instrumentation_buffer_device_address_comp.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_buffer_device_address_comp,
                                             instrumentation_buffer_device_address_comp_size,
                                             ZeroInitializeUintPrivateVariables | UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunctionRange = {"inst_buffer_device_address_range",
                                                      instrumentation_buffer_device_address_comp_function_0_offset};
const static OfflineFunction kOfflineFunctionAlign = {"inst_buffer_device_address_align",
                                                      instrumentation_buffer_device_address_comp_function_1_offset};

BufferDeviceAddressPass::BufferDeviceAddressPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t BufferDeviceAddressPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                                     bool safe_mode) {
    // The Pointer ID Operand is always the first operand for Load/Store/Atomics
    // We can just take it and cast to a uint64 here to examine the ptr value
    const uint32_t pointer_id = meta.target_instruction->Operand(0);

    // Convert reference pointer to uint64
    const Type& uint64_type = module_.type_manager_.GetTypeInt(64, 0);
    const uint32_t address_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpConvertPtrToU, {uint64_type.Id(), address_id, pointer_id}, inst_it);

    const Constant& length_constant = module_.type_manager_.GetConstantUInt32(meta.type_length);
    const uint32_t opcode = meta.target_instruction->Opcode();

    uint32_t access_type_value = 0;
    if (opcode == spv::OpStore) {
        access_type_value |= 1 << glsl::kInstBuffAddrAccessPayloadShiftIsWrite;
    }
    if (meta.type_is_struct) {
        access_type_value |= 1 << glsl::kInstBuffAddrAccessPayloadShiftIsStruct;
    }
    const Constant& access_type = module_.type_manager_.GetConstantUInt32(access_type_value);
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    const uint32_t inst_position = meta.target_instruction->GetPositionIndex();
    const uint32_t inst_position_id = module_.type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t function_range_result = module_.TakeNextId();
    const uint32_t function_range_id = GetLinkFunction(function_range_id_, kOfflineFunctionRange);
    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_range_result, function_range_id, inst_position_id, address_id, access_type.Id(), length_constant.Id()},
        inst_it);

    const Constant& alignment_constant = module_.type_manager_.GetConstantUInt32(meta.alignment_literal);

    const uint32_t function_align_result = module_.TakeNextId();
    const uint32_t function_align_id = GetLinkFunction(function_align_id_, kOfflineFunctionAlign);
    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_align_result, function_align_id, inst_position_id, address_id, access_type.Id(),
                             alignment_constant.Id()},
                            inst_it);

    module_.need_log_error_ = true;

    // Will return bool that will look like (FuncRange() && FuncAlign()) { }
    if (safe_mode) {
        const uint32_t logical_and_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpLogicalAnd, {bool_type, logical_and_id, function_range_result, function_align_result},
                                inst_it);
        return logical_and_id;
    }
    return 0;  // unsafe mode, we don't care what this is
}

bool BufferDeviceAddressPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();
    if (opcode == spv::OpLoad || opcode == spv::OpStore) {
        // We only care if there is an Aligned Memory Operands
        // VUID-StandaloneSpirv-PhysicalStorageBuffer64-04708 requires there to be an Aligned operand
        const uint32_t memory_operand_index = opcode == spv::OpLoad ? 4 : 3;
        const uint32_t alignment_word_index = opcode == spv::OpLoad ? 5 : 4;  // OpStore is at [4]
        if (inst.Length() < alignment_word_index) {
            return false;
        }
        const uint32_t memory_operands = inst.Word(memory_operand_index);
        if ((memory_operands & spv::MemoryAccessAlignedMask) == 0) {
            return false;
        }
        // Even if they are other Memory Operands the spec says it is ordered by smallest bit first,
        // Luckily |Aligned| is the smallest bit that can have an operand so we know it is here
        meta.alignment_literal = inst.Word(alignment_word_index);

        // Aligned 0 was not being validated (https://github.com/KhronosGroup/glslang/issues/3893)
        // This is nonsense and we should skip (as it should be validated in spirv-val)
        if (!IsPowerOfTwo(meta.alignment_literal)) return false;
    } else if (AtomicOperation(opcode)) {
        // Atomics are naturally aligned and by setting this to 1, it will always pass the alignment check
        meta.alignment_literal = 1;
    } else {
        return false;
    }

    // While the Pointer Id might not be an OpAccessChain (can be OpLoad, OpCopyObject, etc), we can just examine its result type to
    // see if it is a PhysicalStorageBuffer pointer or not
    const uint32_t pointer_id = inst.Operand(0);
    const Instruction* pointer_inst = function.FindInstruction(pointer_id);
    if (!pointer_inst) {
        return false;  // Can be pointing to a Workgroup variable out of the function
    }

    // Get the OpTypePointer
    const Type* op_type_pointer = module_.type_manager_.FindTypeById(pointer_inst->TypeId());
    if (!op_type_pointer || op_type_pointer->spv_type_ != SpvType::kPointer ||
        op_type_pointer->inst_.Operand(0) != spv::StorageClassPhysicalStorageBuffer) {
        return false;
    }

    // The OpTypePointer's type
    uint32_t accessed_type_id = op_type_pointer->inst_.Operand(1);
    const Type* accessed_type = module_.type_manager_.FindTypeById(accessed_type_id);
    if (!accessed_type) {
        assert(false);
        return false;
    }

    // This might be an OpTypeStruct, even if some compilers are smart enough (know Mesa is) to detect only the first part of a
    // struct is loaded, we have to assume the entire struct is loaded and the entire memory is accessed (see
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8089)
    meta.type_length = module_.type_manager_.TypeLength(*accessed_type);
    // Will mark this is a struct acess to inform the user
    meta.type_is_struct = accessed_type->spv_type_ == SpvType::kStruct;

    meta.target_instruction = &inst;
    return true;
}

bool BufferDeviceAddressPass::Instrument() {
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) continue;

            if (current_block.IsLoopHeader()) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = current_block.instructions_;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*function, *(inst_it->get()), meta)) continue;

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                if (module_.settings_.unsafe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta, false);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(*function.get(), block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta, true);
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    block_it++;
                    block_it++;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void BufferDeviceAddressPass::PrintDebugInfo() const {
    std::cout << "BufferDeviceAddressPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav