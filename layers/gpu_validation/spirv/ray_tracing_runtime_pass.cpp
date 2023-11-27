/* Copyright (c) 2023 LunarG, Inc.
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

#include "ray_tracing_runtime_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>

#include "generated/inst_ray_tracing_runtime_rgen.h"
#include "generated/inst_ray_tracing_runtime_rint.h"

namespace gpuav {
namespace spirv {

// There are 2 possible functions that we might need to link in, by appending the LinkInfo, it will attempt at linking stage to add
// the function.
uint32_t RayTracingRuntimePass::GetLinkFunctionId(uint32_t opcode) {
    if (opcode == spv::OpTraceRayKHR) {
        if (link_function_trace_ray_id == 0) {
            link_function_trace_ray_id = module_.TakeNextId();
            module_.link_info_.push_back({inst_ray_tracing_runtime_rgen, sizeof(inst_ray_tracing_runtime_rgen) / sizeof(uint32_t),
                                          LinkFunctions::inst_op_trace_ray, link_function_trace_ray_id});
        }
        return link_function_trace_ray_id;
    } else if (opcode == spv::OpReportIntersectionKHR) {
        if (link_function_report_intersection_id == 0) {
            link_function_report_intersection_id = module_.TakeNextId();
            module_.link_info_.push_back({inst_ray_tracing_runtime_rint, sizeof(inst_ray_tracing_runtime_rint) / sizeof(uint32_t),
                                          LinkFunctions::inst_op_report_intersection, link_function_report_intersection_id});
        }
        return link_function_report_intersection_id;
    }
    assert(false);
    return 0;
}

BasicBlockIt RayTracingRuntimePass::InjectFunctionCheck(Function* function, BasicBlockIt block_it, InstructionIt insn_it) {
    const uint32_t insn_position = (*insn_it)->position_index_;
    const uint32_t opcode = (*insn_it)->Opcode();

    // We turn the block into 3 separate blocks
    block_it = function->InsertNewBlock(block_it);
    block_it = function->InsertNewBlock(block_it);
    BasicBlock& original_block = **(std::prev(block_it, 2));
    // Where we call OpTraceRayKHR if it is valid
    BasicBlock& valid_block = **(std::prev(block_it, 1));
    // All the remaining block instructions after OpTraceRayKHR
    BasicBlock& merge_block = **block_it;

    // Move OpTraceRayKHR to valid block
    valid_block.instructions_.emplace_back(std::move(*insn_it));
    insn_it = original_block.instructions_.erase(insn_it);

    // move all remaining instructions to the newly created merge block
    while (*insn_it && insn_it != original_block.instructions_.end()) {
        merge_block.instructions_.emplace_back(std::move(*insn_it));
        insn_it = original_block.instructions_.erase(insn_it);
    }

    const uint32_t valid_block_label = valid_block.GetLabel().ResultId();
    const uint32_t merge_block_label = merge_block.GetLabel().ResultId();

    valid_block.CreateInstruction(spv::OpBranch, {merge_block_label});

    // Add any debug information to pass into the function call
    const uint32_t stage_info_id = CreateStageInfo(spv::ExecutionModelRayGenerationKHR, original_block);
    auto insn_position_constant = module_.type_manager_.CreateConstantUInt32(insn_position);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId(opcode);
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    if (opcode == spv::OpTraceRayKHR) {
        original_block.CreateInstruction(
            spv::OpFunctionCall,
            {bool_type, function_result, function_def, insn_position_constant.Id(), stage_info_id, params_.ray_flags_id,
             params_.ray_origin_id, params_.ray_tmin_id, params_.ray_direction_id, params_.ray_tmax_id});
    } else if (opcode == spv::OpReportIntersectionKHR) {
        original_block.CreateInstruction(spv::OpFunctionCall, {bool_type, function_result, function_def,
                                                               insn_position_constant.Id(), stage_info_id, params_.hit_kind});
    }

    original_block.CreateInstruction(spv::OpSelectionMerge, {merge_block_label, spv::SelectionControlMaskNone});
    original_block.CreateInstruction(spv::OpBranchConditional, {function_result, valid_block_label, merge_block_label});

    return block_it;
}

void RayTracingRuntimePass::Run() {
    for (const auto& function : module_.functions_) {
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            for (auto insn_it = (*block_it)->instructions_.begin(); insn_it != (*block_it)->instructions_.end(); ++insn_it) {
                auto& insn = *insn_it;
                const uint32_t opcode = insn->Opcode();
                if (opcode == spv::OpTraceRayKHR) {
                    params_.ray_flags_id = insn->Word(2);
                    params_.ray_origin_id = insn->Word(7);
                    params_.ray_tmin_id = insn->Word(8);
                    params_.ray_direction_id = insn->Word(9);
                    params_.ray_tmax_id = insn->Word(10);

                    block_it = InjectFunctionCheck(function.get(), block_it, insn_it);

                    // will start searching again from newly split merge block
                    block_it--;
                    break;
                } else if (opcode == spv::OpReportIntersectionKHR) {
                    params_.hit_kind = insn->Word(4);

                    block_it = InjectFunctionCheck(function.get(), block_it, insn_it);

                    // will start searching again from newly split merge block
                    block_it--;
                    break;
                }
            }
        }
    }
}

}  // namespace spirv
}  // namespace gpuav