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
#pragma once

#include <stdint.h>
#include "pass.h"
#include "function_basic_block.h"

namespace gpuav {
namespace spirv {

struct Module;

class RayTracingRuntimePass : public Pass {
  public:
    RayTracingRuntimePass(Module& module) : Pass(module) {}
    void Run() override;

  private:
    BasicBlockIt InjectFunctionCheck(Function* function, BasicBlockIt block_it, InstructionIt insn_it);

    struct Parameters {
        // OpTraceRayKHR
        uint32_t ray_flags_id;
        uint32_t ray_origin_id;
        uint32_t ray_tmin_id;
        uint32_t ray_direction_id;
        uint32_t ray_tmax_id;

        // OpReportIntersectionKHR
        uint32_t hit_kind;
    } params_;

    uint32_t link_function_trace_ray_id = 0;
    uint32_t link_function_report_intersection_id = 0;
    uint32_t GetLinkFunctionId(uint32_t opcode);
};

}  // namespace spirv
}  // namespace gpuav