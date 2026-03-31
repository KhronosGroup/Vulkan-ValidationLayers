/* Copyright (c) 2024-2026 LunarG, Inc.
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

#include "descriptor_heap_pass.h"
#include "generated/spirv_grammar_helper.h"
#include "containers/container_utils.h"
#include "state_tracker/shader_instruction.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/shaders/gpuav_error_codes.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_heap_comp, instrumentation_descriptor_heap_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunctionPostProcess = {"inst_descriptor_heap",
                                                            instrumentation_descriptor_heap_comp_function_0_offset};

DescriptorHeapPass::DescriptorHeapPass(Module& module, VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props,
                                       VkPhysicalDeviceDescriptorHeapTensorPropertiesARM heap_tensor_props)
    : Pass(module, kOfflineModule), heap_props_(heap_props), heap_tensor_props_(heap_tensor_props) {
    module.use_bda_ = true;
}

void DescriptorHeapPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    if (meta.post_process) {
        const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
        const uint32_t inst_position_id = module_.type_manager_.CreateConstantUInt32(inst_position).Id();

        const uint32_t function_result = module_.TakeNextId();
        const uint32_t function_def = GetLinkFunction(link_function_id_, kOfflineFunctionPostProcess);
        const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();

        const uint32_t zero = module_.type_manager_.CreateConstantUInt32(0).Id();
        const uint32_t descriptor_offset_id =
            meta.access_chain_insts.empty()
                ? zero
                : GetLastByte(*meta.descriptor_type, meta.access_chain_insts, meta.coop_mat_access, block, inst_it);

        const Constant& slot_index = type_manager_.CreateConstantUInt32(slot_index_++);
        if (slot_index_ > glsl::kDebugMaxDescSetAndBindings) {
            return;
        }

        const Constant& variable_id_constant = type_manager_.GetConstantUInt32(meta.variable_id);
        block.CreateInstruction(spv::OpFunctionCall,
                                {void_type, function_result, function_def, inst_position_id, descriptor_offset_id, slot_index.Id(),
                                 variable_id_constant.Id()},
                                inst_it);
    }

    // Temporary, to mute warnings until validation that requires these fields is added
    (void)heap_props_;
    (void)heap_tensor_props_;

    module_.need_log_error_ = true;
}

bool DescriptorHeapPass::HeapPointerRequiresInstrumentation(const Function&, const Instruction&, InstructionMeta&) { return false; }

bool DescriptorHeapPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();

    // Save information to be used to make the Function
    meta.target_instruction = &inst;

    bool requires_instrumentation = HeapPointerRequiresInstrumentation(function, inst, meta);

    if (!IsValueIn(spv::Op(opcode), {spv::OpLoad, spv::OpStore, spv::OpAtomicStore, spv::OpAtomicLoad, spv::OpAtomicExchange})) {
        return requires_instrumentation;
    }

    const Variable* variable = nullptr;
    const Instruction* next_access_chain = function.FindInstruction(inst.Operand(0));
    if (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
        meta.access_chain_insts.push_back(next_access_chain);
        // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
        while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
            const uint32_t access_chain_base_id = next_access_chain->Operand(0);
            variable = module_.type_manager_.FindVariableById(access_chain_base_id);
            if (variable) {
                break;  // found
            }
            next_access_chain = function.FindInstruction(access_chain_base_id);
        }
    } else {
        variable = module_.type_manager_.FindVariableById(inst.Operand(0));
    }

    if (!variable) {
        return requires_instrumentation;
    }
    meta.variable_id = variable->inst_.ResultId();
    meta.descriptor_type = variable->PointerType(type_manager_);

    bool is_coop_mat = opcode == spv::OpCooperativeMatrixLoadKHR || opcode == spv::OpCooperativeMatrixStoreKHR;
    if (is_coop_mat) {
        meta.coop_mat_access = GetCooperativeMatrixAccess(inst, function);
    }

    bool set_found = false;
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable->Id()) {
            if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                set_found = true;
            }
        }
    }

    if (!set_found) {
        return requires_instrumentation;
    }

    meta.post_process = true;
    return true;
}

void DescriptorHeapPass::PrintDebugInfo() const {
    std::cout << "DescriptorHeapPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Created own Instrument() because need to control finding the largest offset in a given block
bool DescriptorHeapPass::Instrument() {
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) continue;

            auto& block_instructions = current_block.instructions_;

            // < Descriptor SSA ID, Highest offset byte that will be accessed >
            vvl::unordered_map<uint32_t, uint32_t> block_highest_offset_map;

            if (!module_.settings_.safe_mode) {
                // Pre-pass loop the Block to get the highest offset accessed (statically known)
                // Do here before we inject instructions into the block list below
                for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                    InstructionMeta meta;
                    // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                    if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) continue;
                }
            }

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) continue;

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    if (instrumentations_count_ > 75) {
        module_.InternalWarning(
            "GPUAV-Compile-time-descriptor-heap",
            "This shader will be very slow to compile and runtime performance may also be slow. This is due to the number of OOB "
            "checks for storage/uniform "
            "buffers. Turn on the |gpuav_force_on_robustness| setting to skip these checks and improve GPU-AV performance.");
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav
