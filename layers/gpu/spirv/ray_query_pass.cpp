/* Copyright (c) 2024 LunarG, Inc.
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

#include "ray_query_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>

#include "generated/inst_ray_query_comp.h"

namespace gpuav {
namespace spirv {

static LinkInfo link_info = {inst_ray_query_comp, inst_ray_query_comp_size, LinkFunctions::inst_ray_query, 0, "inst_ray_query"};

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t RayQueryPass::GetLinkFunctionId() {
    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t RayQueryPass::CreateFunctionCall(BasicBlock& block) {
    // Add any debug information to pass into the function call
    const uint32_t stage_info_id = GetStageInfo(block.function_);
    const uint32_t inst_position = target_instruction_->position_index_;
    auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    const uint32_t ray_flags_id = target_instruction_->Operand(2);
    const uint32_t ray_origin_id = target_instruction_->Operand(4);
    const uint32_t ray_tmin_id = target_instruction_->Operand(5);
    const uint32_t ray_direction_id = target_instruction_->Operand(6);
    const uint32_t ray_tmax_id = target_instruction_->Operand(7);

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_constant.Id(), stage_info_id, ray_flags_id,
                             ray_origin_id, ray_tmin_id, ray_direction_id, ray_tmax_id});

    return function_result;
}

void RayQueryPass::Reset() { target_instruction_ = nullptr; }

bool RayQueryPass::AnalyzeInstruction(const Function& function, const Instruction& inst) {
    (void)function;
    const uint32_t opcode = inst.Opcode();
    if (opcode != spv::OpRayQueryInitializeKHR) {
        return false;
    }
    target_instruction_ = &inst;
    return true;
}

}  // namespace spirv
}  // namespace gpuav