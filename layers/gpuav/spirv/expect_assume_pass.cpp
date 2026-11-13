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

#include "expect_assume_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_expect_assume_comp, instrumentation_expect_assume_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_assume_true", instrumentation_expect_assume_comp_function_0_offset};

ExpectAssumePass::ExpectAssumePass(Module& module) : Pass(module, kOfflineModule) {}

uint32_t ExpectAssumePass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(link_function_id_, kOfflineFunction);
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    const uint32_t condition_id = meta.target_instruction->Word(1);
    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    block.CreateInstruction(spv::OpFunctionCall, {void_type, function_result, function_def, condition_id, inst_position_id},
                            inst_it);
    module_.need_log_error_ = true;
    return function_result;
}

bool ExpectAssumePass::RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta) {
    if (inst.Opcode() != spv::OpAssumeTrueKHR) {
        return false;
    }
    meta.target_instruction = &inst;
    return true;
}

bool ExpectAssumePass::Instrument() {
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

            // No new blocks are created, so unlike other passes, loop headers are fine to instrument
            auto& block_instructions = current_block.instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*(inst_it->get()), meta)) {
                    continue;
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                CreateFunctionCall(current_block, &inst_it, meta);

                // Subtle but important!
                //
                // We replaces the OpAssumeTrueKHR here with our function.
                // The goal is prevent a "Heisenbug" where our code changes the behavior originally seen without GPU-AV
                inst_it = block_instructions.erase(inst_it);
                --inst_it;
            }
        }
    }

    return instrumentations_count_ != 0;
}

void ExpectAssumePass::PrintDebugInfo() const {
    std::cout << "ExpectAssumePass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
