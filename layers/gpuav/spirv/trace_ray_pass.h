/* Copyright (c) 2024-2026 LunarG, Inc.
 * Copyright (c) 2020-2026 Valve Corporation
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
#pragma once

#include <stdint.h>
#include <vector>
#include "pass.h"

namespace gpuav {
namespace spirv {

class TraceRayPass : public Pass {
  public:
    TraceRayPass(Module& module);
    const char* Name() const final { return "TraceRayPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    std::vector<uint32_t> GetTlasValidationFunctionCallInstructions(const Function& function, uint32_t tlas_operand_pos,
                                                                    uint32_t error_sub_code, BasicBlock& block,
                                                                    InstructionIt* trace_ray_inst_it);
    std::vector<uint32_t> GetTraceRayValidationFunctionCallInstructions(InstructionIt* trace_ray_inst_it);
    std::vector<uint32_t> GetRayHitObjectValidationFunctionCallInstructions(InstructionIt* ray_hit_object_inst_it);
    std::vector<uint32_t> GetRayHitObjectSbtIndexValidationFunctionCallInstructions(
        InstructionIt* ray_hit_object_sbt_index_inst_it);
    std::vector<uint32_t> GetRayQueryInitializeValidationFunctionCallInstructions(InstructionIt* ray_query_init_inst_it);
    std::vector<uint32_t> GetReportIntersectionValidationFunctionCallInstructions(InstructionIt* report_intersection_inst_it);
    uint32_t AddFunctionCall(BasicBlock& block, std::vector<uint32_t>&& instructions, InstructionIt* inst_it);
    // Function IDs to link in
    uint32_t trace_ray_link_function_id_ = 0;
    uint32_t trace_ray_as_link_function_id_ = 0;
    uint32_t hit_object_link_function_id_ = 0;
    uint32_t hit_object_sbt_index_link_function_id_ = 0;
    uint32_t ray_query_initialize_function_id_ = 0;
    uint32_t report_intersection_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
