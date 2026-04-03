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

#include "trace_ray_pass.h"

#include "containers/container_utils.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_trace_ray_comp, instrumentation_trace_ray_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kTraceRayValidationFunction = {"inst_trace_ray", instrumentation_trace_ray_comp_function_0_offset};
const static OfflineFunction kRayHitObjectValidationFunction = {"inst_ray_hit_object",
                                                                instrumentation_trace_ray_comp_function_1_offset};
const static OfflineFunction kRayHitObjectSbtIndexValidationFunction = {"inst_ray_hit_object_sbt_index_check",
                                                                        instrumentation_trace_ray_comp_function_2_offset};

TraceRayPass::TraceRayPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

std::vector<uint32_t> TraceRayPass::GetTraceRayValidationFunctionCallInstructions(InstructionIt* trace_ray_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(trace_ray_link_function_id_, kTraceRayValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t ray_flags_id = (*trace_ray_inst_it)->get()->Operand(1);

    const uint32_t inst_position = (*trace_ray_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    return {bool_type, function_result, function_def, inst_position_id, ray_flags_id};
}

std::vector<uint32_t> TraceRayPass::GetRayHitObjectValidationFunctionCallInstructions(InstructionIt* ray_hit_object_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(hit_object_link_function_id_, kRayHitObjectValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t opcode = (*ray_hit_object_inst_it)->get()->Opcode();

    // All HitObject opcodes have ray parameters at the same positions
    const uint32_t ray_flags_id = (*ray_hit_object_inst_it)->get()->Operand(2);
    const uint32_t ray_origin_id = (*ray_hit_object_inst_it)->get()->Operand(7);
    const uint32_t ray_tmin_id = (*ray_hit_object_inst_it)->get()->Operand(8);
    const uint32_t ray_direction_id = (*ray_hit_object_inst_it)->get()->Operand(9);
    const uint32_t ray_tmax_id = (*ray_hit_object_inst_it)->get()->Operand(10);

    uint32_t time_id = 0;
    if (opcode == spv::OpHitObjectTraceRayMotionEXT || opcode == spv::OpHitObjectTraceMotionReorderExecuteEXT) {
        time_id = (*ray_hit_object_inst_it)->get()->Operand(11);
    }

    const uint32_t inst_position = (*ray_hit_object_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t opcode_type_id = type_manager_.CreateConstantUInt32(opcode).Id();

    const uint32_t pipeline_flags = (module_.interface_.instrumentation_dsl.pipeline_has_skip_aabbs_flag ? 1u : 0u) |
                                    (module_.interface_.instrumentation_dsl.pipeline_has_skip_triangles_flag ? 2u : 0u);
    const uint32_t pipeline_flags_id = type_manager_.CreateConstantUInt32(pipeline_flags).Id();

    // For non-motion opcodes, pass 0.0 as time (valid value, won't trigger error)
    if (time_id == 0) {
        time_id = type_manager_.GetConstantZeroFloat32().Id();
    }

    return {bool_type,     function_result, function_def,     inst_position_id, opcode_type_id,    ray_flags_id,
            ray_origin_id, ray_tmin_id,     ray_direction_id, ray_tmax_id,      pipeline_flags_id, time_id};
}

std::vector<uint32_t> TraceRayPass::GetRayHitObjectSbtIndexValidationFunctionCallInstructions(
    InstructionIt* ray_hit_object_sbt_index_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(hit_object_sbt_index_link_function_id_, kRayHitObjectSbtIndexValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t sbt_index_id = (*ray_hit_object_sbt_index_inst_it)->get()->Operand(1);

    const uint32_t inst_position = (*ray_hit_object_sbt_index_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    // maxShaderBindingTableRecordIndex
    const uint32_t max_sbt_index = module_.interface_.instrumentation_dsl.max_shader_binding_table_record_index;
    const uint32_t max_sbt_index_id = type_manager_.CreateConstantUInt32(max_sbt_index).Id();

    return {bool_type, function_result, function_def, inst_position_id, sbt_index_id, max_sbt_index_id};
}

uint32_t TraceRayPass::AddFunctionCall(BasicBlock& block, std::vector<uint32_t>&& instructions, InstructionIt* inst_it) {
    const uint32_t function_result = instructions[1];
    block.CreateInstruction(spv::OpFunctionCall, std::move(instructions), inst_it);
    module_.need_log_error_ = true;
    return function_result;
}

bool TraceRayPass::Instrument() {
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
                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }

                const spv::Op opcode = spv::Op(inst_it->get()->Opcode());

                std::vector<uint32_t> instrumentation_instructions;
                if (opcode == spv::OpTraceRayKHR) {
                    instrumentation_instructions = GetTraceRayValidationFunctionCallInstructions(&inst_it);
                } else if (opcode == spv::OpHitObjectSetShaderBindingTableRecordIndexEXT) {
                    instrumentation_instructions = GetRayHitObjectSbtIndexValidationFunctionCallInstructions(&inst_it);
                } else if (IsValueIn(opcode, {spv::OpHitObjectTraceRayEXT, spv::OpHitObjectTraceReorderExecuteEXT,
                                              spv::OpHitObjectTraceRayMotionEXT, spv::OpHitObjectTraceMotionReorderExecuteEXT})) {
                    instrumentation_instructions = GetRayHitObjectValidationFunctionCallInstructions(&inst_it);
                }

                if (instrumentation_instructions.empty()) {
                    continue;
                }

                ++instrumentations_count_;

                if (!module_.settings_.safe_mode) {
                    AddFunctionCall(current_block, std::move(instrumentation_instructions), &inst_it);

                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    ic_data.function_result_id = AddFunctionCall(current_block, std::move(instrumentation_instructions), nullptr);
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    ++block_it;
                    ++block_it;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void TraceRayPass::PrintDebugInfo() const {
    std::cout << "TraceRayPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
