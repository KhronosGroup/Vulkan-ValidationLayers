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

#include "log_error_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "utils/vk_layer_utils.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_log_error_comp, instrumentation_log_error_comp_size,
                                             UseErrorPayloadVariable};
const static OfflineFunction kOfflineFunction = {"inst_log_error", instrumentation_log_error_comp_function_0_offset};

LogErrorPass::LogErrorPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

void LogErrorPass::PrintDebugInfo() const {
    std::cout << "LogErrorPass instrumentation count: " << instrumentations_count_ << '\n';
}

void LogErrorPass::ClearErrorPayloadVariable(Function& function) {
    // First time we clear, we add the private variable to the global scope
    if (module_.error_payload_variable_id_ == 0) {
        module_.error_payload_variable_id_ = module_.TakeNextId();
        const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);

        const uint32_t struct_type_id = module_.TakeNextId();
        auto new_struct_inst = std::make_unique<Instruction>(7, spv::OpTypeStruct);
        new_struct_inst->Fill({
            struct_type_id,
            uint32_type.Id(),  // uint inst_num;
            uint32_type.Id(),  // uint shader_error_encoding;
            uint32_type.Id(),  // uint parameter_0;
            uint32_type.Id(),  // uint parameter_1;
            uint32_type.Id(),  // uint parameter_2;
        });
        const Type& struct_type = module_.type_manager_.AddType(std::move(new_struct_inst), SpvType::kStruct);
        module_.type_manager_.AddStructTypeForLinking(&struct_type);
        const Type& pointer_type = module_.type_manager_.GetTypePointer(spv::StorageClassPrivate, struct_type);
        {
            auto new_inst = std::make_unique<Instruction>(4, spv::OpVariable);
            new_inst->Fill({pointer_type.Id(), module_.error_payload_variable_id_, spv::StorageClassPrivate});
            module_.type_manager_.AddVariable(std::move(new_inst), pointer_type);
        }

        const uint32_t uint32_0_id = module_.type_manager_.GetConstantZeroUint32().Id();
        const uint32_t constant_id = module_.TakeNextId();
        {
            auto new_inst = std::make_unique<Instruction>(8, spv::OpConstantComposite);
            new_inst->Fill({struct_type.Id(), constant_id, uint32_0_id, uint32_0_id, uint32_0_id, uint32_0_id, uint32_0_id});
            const Constant& clear_constant = module_.type_manager_.AddConstant(std::move(new_inst), struct_type);
            error_payload_variable_clear_ = clear_constant.Id();
        }
    }

    BasicBlock& block = function.GetFirstBlock();
    InstructionIt inst_it = block.GetFirstInjectableInstrution();

    block.CreateInstruction(spv::OpStore, {module_.error_payload_variable_id_, error_payload_variable_clear_}, &inst_it);
}

void LogErrorPass::CreateFunctionCallLogError(Function& function, BasicBlock& block, InstructionIt* inst_it) {
    GetLinkFunction(link_function_id_, kOfflineFunction);

    const uint32_t stage_info_id = GetStageInfo(function, block, *inst_it);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();
    block.CreateInstruction(spv::OpFunctionCall, {void_type, function_result, link_function_id_, stage_info_id}, inst_it);
}

// This is a hard thing to fully get right, as things like OpTerminateInvocation/OpKill can end the invocation, but not the shader.
// The main thing we are trying to find here is if this is our last chance to report an error message
bool LogErrorPass::IsShaderExiting(const Instruction& inst) const {
    return IsValueIn((spv::Op)inst.Opcode(), {spv::OpReturn, spv::OpReturnValue, spv::OpEmitMeshTasksEXT});
}

bool LogErrorPass::Instrument() {
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;
        // Only need to clear when we know we are starting a new entry into the shader
        if (!function->is_entry_point_) continue;

        // This makes (a reasonably safe) assumption one entrypoint is not calling into another entrypoint after it has caused an
        // error. If we really care, we could build CFA to detect that, but likely not worth the effort currently.
        ClearErrorPayloadVariable(*function);

        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            auto& block_instructions = current_block.instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (IsShaderExiting(*(inst_it->get()))) {
                    CreateFunctionCallLogError(*function, current_block, &inst_it);
                    instrumentations_count_++;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav