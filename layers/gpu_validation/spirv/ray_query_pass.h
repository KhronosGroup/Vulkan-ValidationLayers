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
#include "pass.h"

namespace gpuav {
namespace spirv {

class Module;
struct Function;
struct BasicBlock;

// Create a pass to instrument SPV_KHR_ray_query instructions
class RayQueryPass : public Pass {
  public:
    RayQueryPass(Module& module) : Pass(module) {}

  private:
    bool AnalyzeInstruction(const Function& function, const Instruction& inst) final;
    uint32_t CreateFunctionCall(BasicBlock& block) final;
    void Reset() final;

    uint32_t link_function_id = 0;
    uint32_t GetLinkFunctionId();

    const Instruction* target_instruction_ = nullptr;
};

}  // namespace spirv
}  // namespace gpuav