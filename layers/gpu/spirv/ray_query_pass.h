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
#pragma once

#include <stdint.h>
#include "inject_conditional_function_pass.h"

namespace gpu {
namespace spirv {

// Create a pass to instrument SPV_KHR_ray_query instructions
class RayQueryPass : public InjectConditionalFunctionPass {
  public:
    RayQueryPass(Module& module) : InjectConditionalFunctionPass(module) {}
    void PrintDebugInfo() final;

  private:
    bool AnalyzeInstruction(const Function& function, const Instruction& inst) final;
    u32 CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data) final;
    void Reset() final;

    u32 link_function_id = 0;
    u32 GetLinkFunctionId();
};

}  // namespace spirv
}  // namespace gpu