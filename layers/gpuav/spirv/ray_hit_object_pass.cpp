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

#include "ray_hit_object_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_ray_hit_object_comp, instrumentation_ray_hit_object_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_ray_hit_object", instrumentation_ray_hit_object_comp_function_0_offset};
const static OfflineFunction kSBTIndexCheckFunction = {"inst_ray_hit_object_sbt_index_check", instrumentation_ray_hit_object_comp_function_1_offset};

RayHitObjectPass::RayHitObjectPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t RayHitObjectPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

uint32_t RayHitObjectPass::GetSBTIndexCheckFunctionId() { return GetLinkFunction(sbt_index_check_function_id_, kSBTIndexCheckFunction); }

uint32_t RayHitObjectPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t opcode = meta.target_instruction->Opcode();

    // Operand positions for ray parameters:
    // OpHitObjectTraceRayEXT:              HitObj, AS, RayFlags, CullMask, SBTOffset, SBTStride, MissIndex, Origin, TMin, Direction, TMax, Payload
    // OpHitObjectTraceRayMotionEXT:        HitObj, AS, RayFlags, CullMask, SBTOffset, SBTStride, MissIndex, Origin, TMin, Direction, TMax, Time, Payload
    // OpHitObjectTraceReorderExecuteEXT:   HitObj, AS, RayFlags, CullMask, SBTOffset, SBTStride, MissIndex, Origin, TMin, Direction, TMax, ...
    // OpHitObjectTraceMotionReorderExecuteEXT: HitObj, AS, RayFlags, CullMask, SBTOffset, SBTStride, MissIndex, Origin, TMin, Direction, TMax, Time, ...
    uint32_t ray_flags_id, ray_origin_id, ray_tmin_id, ray_direction_id, ray_tmax_id;
    uint32_t time_id = 0;

    // All HitObject opcodes have ray parameters at the same positions
    ray_flags_id = meta.target_instruction->Operand(2);
    ray_origin_id = meta.target_instruction->Operand(7);
    ray_tmin_id = meta.target_instruction->Operand(8);
    ray_direction_id = meta.target_instruction->Operand(9);
    ray_tmax_id = meta.target_instruction->Operand(10);
    // Motion opcodes have time at Operand(11)
    if (opcode == spv::OpHitObjectTraceRayMotionEXT || opcode == spv::OpHitObjectTraceMotionReorderExecuteEXT) {
        time_id = meta.target_instruction->Operand(11);
    }

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    // opcode_type: 0 = OpHitObjectTraceRayEXT, 1 = OpHitObjectTraceReorderExecuteEXT,
    //              2 = OpHitObjectTraceRayMotionEXT, 3 = OpHitObjectTraceMotionReorderExecuteEXT
    uint32_t opcode_type = 0;
    if (opcode == spv::OpHitObjectTraceRayEXT) {
        opcode_type = 0;
    } else if (opcode == spv::OpHitObjectTraceReorderExecuteEXT) {
        opcode_type = 1;
    } else if (opcode == spv::OpHitObjectTraceRayMotionEXT) {
        opcode_type = 2;
    } else if (opcode == spv::OpHitObjectTraceMotionReorderExecuteEXT) {
        opcode_type = 3;
    }
    const uint32_t opcode_type_id = type_manager_.CreateConstantUInt32(opcode_type).Id();

    // Pipeline flags for VUIDs 11886/11887
    const uint32_t pipeline_flags =
        (module_.settings_.pipeline_has_skip_aabbs_flag ? 1u : 0u) |
        (module_.settings_.pipeline_has_skip_triangles_flag ? 2u : 0u);
    const uint32_t pipeline_flags_id = type_manager_.CreateConstantUInt32(pipeline_flags).Id();

    // For non-motion opcodes, pass 0.0 as time (valid value, won't trigger error)
    if (time_id == 0) {
        time_id = type_manager_.GetConstantZeroFloat32().Id();
    }

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_id, opcode_type_id, ray_flags_id, ray_origin_id, ray_tmin_id,
                             ray_direction_id, ray_tmax_id, pipeline_flags_id, time_id},
                            inst_it);
    module_.need_log_error_ = true;
    return function_result;
}

// VUID-RuntimeSpirv-maxShaderBindingTableRecordIndex-11888
uint32_t RayHitObjectPass::CreateSBTIndexCheckFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetSBTIndexCheckFunctionId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    // OpHitObjectSetShaderBindingTableRecordIndexEXT operands: HitObject, RecordIndex
    const uint32_t sbt_index_id = meta.target_instruction->Operand(1);

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t max_sbt_index_id = type_manager_.CreateConstantUInt32(module_.settings_.max_shader_binding_table_record_index).Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_id, sbt_index_id, max_sbt_index_id},
                            inst_it);
    module_.need_log_error_ = true;
    return function_result;
}

bool RayHitObjectPass::RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();
    if (opcode == spv::OpHitObjectSetShaderBindingTableRecordIndexEXT) {
        meta.target_instruction = &inst;
        meta.is_sbt_index_check = true;
        return true;
    }
    if (opcode != spv::OpHitObjectTraceRayEXT && opcode != spv::OpHitObjectTraceReorderExecuteEXT &&
        opcode != spv::OpHitObjectTraceRayMotionEXT && opcode != spv::OpHitObjectTraceMotionReorderExecuteEXT) {
        return false;
    }
    meta.target_instruction = &inst;
    meta.is_sbt_index_check = false;
    return true;
}

bool RayHitObjectPass::Instrument() {
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) {
            continue;
        }
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
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
                if (!RequiresInstrumentation(*(inst_it->get()), meta)) {
                    continue;
                }

                if (IsMaxInstrumentationsCount()) {
                    continue;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    if (meta.is_sbt_index_check) {
                        CreateSBTIndexCheckFunctionCall(current_block, &inst_it, meta);
                    } else {
                        CreateFunctionCall(current_block, &inst_it, meta);
                    }
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(*function.get(), block_it, inst_it);
                    if (meta.is_sbt_index_check) {
                        ic_data.function_result_id = CreateSBTIndexCheckFunctionCall(current_block, nullptr, meta);
                    } else {
                        ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta);
                    }
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

void RayHitObjectPass::PrintDebugInfo() const {
    std::cout << "RayHitObjectPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
