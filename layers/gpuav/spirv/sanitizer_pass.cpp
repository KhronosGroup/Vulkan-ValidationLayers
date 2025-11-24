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

#include "sanitizer_pass.h"
#include "containers/container_utils.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_sanitizer_comp, instrumentation_sanitizer_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_sanitizer_divide_by_zero", instrumentation_sanitizer_comp_function_0_offset};

SanitizerPass::SanitizerPass(Module& module) : Pass(module, kOfflineModule) {}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t SanitizerPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

// We do the check for zero in C++ to make it easier to handle the various cases of signed/unsigned/64bit/etc
// Returns an ID of type OpTypeBool
uint32_t SanitizerPass::DivideByZeroCheck(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const Type& bool_type = type_manager_.GetTypeBool();
    const uint32_t vector_size = meta.result_type->VectorSize();
    const uint32_t divisor_id = meta.target_instruction->Word(4);

    if (vector_size == 0) {
        const uint32_t null_type_id = type_manager_.GetConstantNull(*meta.result_type).Id();
        const uint32_t compare_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpIEqual, {bool_type.Id(), compare_id, null_type_id, divisor_id}, inst_it);
        return compare_id;
    } else {
        const Type& bool_vector_type = type_manager_.GetTypeVector(bool_type, vector_size);
        const uint32_t zero_id = type_manager_.GetConstantZeroVector(*meta.result_type).Id();

        const uint32_t compare_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpIEqual, {bool_vector_type.Id(), compare_id, zero_id, divisor_id}, inst_it);

        const uint32_t any_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpAny, {bool_type.Id(), any_id, compare_id}, inst_it);
        return any_id;
    }
}

uint32_t SanitizerPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t is_valid_id = DivideByZeroCheck(block, inst_it, meta);

    const uint32_t opcode_id = type_manager_.CreateConstantUInt32(meta.target_instruction->Opcode()).Id();
    const uint32_t vector_size_id = type_manager_.CreateConstantUInt32(meta.result_type->VectorSize()).Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, is_valid_id, inst_position_id, opcode_id, vector_size_id},
                            inst_it);
    module_.need_log_error_ = true;
    return function_result;
}

bool SanitizerPass::IsConstantZero(const Constant& constant) const {
    if (constant.is_spec_constant_) {
        // TODO - We have the spec constants information, we just need to pipe it into the passes
        return false;
    }
    const spv::Op opcode = (spv::Op)constant.inst_.Opcode();
    if (opcode == spv::OpConstantNull || opcode == spv::OpConstantFalse) {
        return true;
    } else if (opcode == spv::OpConstant) {
        // This works for signed ints and floats because zero is same for all
        if (constant.type_.Is64Bit()) {
            return constant.inst_.Word(3) == 0 && constant.inst_.Word(4) == 0;
        } else {
            return constant.inst_.Word(3) == 0;
        }
    } else if (opcode == spv::OpConstantComposite) {
        const size_t constituent_count = constant.inst_.Length() - 3;
        for (uint32_t i = 0; i < constituent_count; i++) {
            const Constant* component_constant = type_manager_.FindConstantById(constant.inst_.Operand(i));
            if (component_constant && IsConstantZero(*component_constant)) {
                return true;
            }
        }
    }
    return false;
}

bool SanitizerPass::RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();

    if (IsValueIn(opcode, {spv::OpUDiv, spv::OpSDiv, spv::OpUMod, spv::OpSMod, spv::OpSRem})) {
        // Note - It is valid to divide by zero for a float (you get NaN), but invalid for an int.
        if (const Constant* constant = type_manager_.FindConstantById(inst.Word(4))) {
            // If its a constant, no reason to instrument, unless its a constant value of zero,
            // then it is only invalid if executed on the GPU.
            // (Tried to add to spirv-val, decided it should be a warning at most in VVL)
            if (!IsConstantZero(*constant)) {
                return false;
            }
        }
        meta.result_type = type_manager_.FindTypeById(inst.TypeId());
        meta.target_instruction = &inst;
        return true;
    }

    return false;
}

bool SanitizerPass::Instrument() {
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
                if (!RequiresInstrumentation(*(inst_it->get()), meta)) continue;

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(*function.get(), block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta);
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

void SanitizerPass::PrintDebugInfo() const {
    std::cout << "SanitizerPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav