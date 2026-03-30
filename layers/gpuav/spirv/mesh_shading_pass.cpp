/* Copyright (c) 2025-2026 LunarG, Inc.
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

#include "mesh_shading_pass.h"
#include <vulkan/vulkan_core.h>
#include "containers/container_utils.h"
#include "module.h"
#include <cassert>
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_mesh_shading_comp, instrumentation_mesh_shading_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction[2] = {
    {"inst_set_mesh_output", instrumentation_mesh_shading_comp_function_0_offset},
    {"inst_task_payload_always", instrumentation_mesh_shading_comp_function_1_offset},
};

MeshShading::MeshShading(Module& module) : Pass(module, kOfflineModule) {}

uint32_t MeshShading::GetLinkFunctionId(const InstructionMeta& meta) {
    return GetLinkFunction(link_function_id_[meta.function_id], kOfflineFunction[meta.function_id]);
}

uint32_t MeshShading::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId(meta);

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    if (meta.function_id == SET_MESH_OUTPUT) {
        const uint32_t vertex_count = meta.target_instruction->Word(1);
        const uint32_t primitive_count = meta.target_instruction->Word(2);
        assert(output_vertices_id_ != 0 && output_primitives_id_ != 0);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, vertex_count, primitive_count,
                                 output_vertices_id_, output_primitives_id_},
                                inst_it);
    } else if (meta.function_id == TASK_PAYLOAD_ALWAYS) {
        block.CreateInstruction(spv::OpFunctionCall, {bool_type, function_result, function_def, inst_position_id}, inst_it);
    }
    module_.need_log_error_ = true;
    return function_result;
}

bool MeshShading::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    const spv::Op opcode = (spv::Op)inst.Opcode();
    if (opcode == spv::OpSetMeshOutputsEXT) {
        meta.target_instruction = &inst;
        meta.function_id = SET_MESH_OUTPUT;
        return true;
    } else if (guard_all_task_payloads_ && (IsValueIn(opcode, {spv::OpLoad, spv::OpStore}) || AtomicOperation(opcode))) {
        // |Operand 0| works for both Store/Load
        const uint32_t ptr_id = inst.Operand(0);

        // Unlike descriptors, TaskPayload can be a scalar that does a direct variable access, not using an access chain
        const Variable* variable = type_manager_.FindVariableById(ptr_id);
        const Instruction* next_access_chain = function.FindInstruction(inst.Operand(0));
        // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
        while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
            const uint32_t access_chain_base_id = next_access_chain->Operand(0);
            variable = type_manager_.FindVariableById(access_chain_base_id);
            if (variable) {
                break;  // found
            }
            next_access_chain = function.FindInstruction(access_chain_base_id);
        }
        if (!variable) {
            return false;
        } else if (variable->StorageClass() != spv::StorageClassTaskPayloadWorkgroupEXT) {
            return false;
        }

        meta.target_instruction = &inst;
        meta.function_id = TASK_PAYLOAD_ALWAYS;
        return true;
    }

    return false;
}

bool MeshShading::Instrument() {
    if (module_.interface_.entry_point_stage != VK_SHADER_STAGE_MESH_BIT_EXT) {
        return false;
    }

    // Note - OpExecutionModeId can't be used if the extra operands are marked as "literal"
    for (const auto& execution_mode_inst : module_.execution_modes_) {
        if (execution_mode_inst->Word(1) != module_.target_entry_point_id_) {
            continue;
        }

        const spv::ExecutionMode mode = (spv::ExecutionMode)execution_mode_inst->Word(2);
        if (mode == spv::ExecutionModeOutputVertices) {
            output_vertices_id_ = type_manager_.CreateConstantUInt32(execution_mode_inst->Word(3)).Id();
        } else if (mode == spv::ExecutionModeOutputPrimitivesEXT) {
            output_primitives_id_ = type_manager_.CreateConstantUInt32(execution_mode_inst->Word(3)).Id();
        }
    }

    if (!module_.interface_.has_task_shader && !type_manager_.GetTaskPayloadVariables().empty()) {
        // This means there is a task payload in the Mesh Shader that is not allowed to be used
        // https://github.com/KhronosGroup/Vulkan-Docs/issues/2677
        guard_all_task_payloads_ = true;
    }

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

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
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

void MeshShading::PrintDebugInfo() const { std::cout << "MeshShading instrumentation count: " << instrumentations_count_ << '\n'; }

}  // namespace spirv
}  // namespace gpuav