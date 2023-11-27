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
#include <spirv/unified1/spirv.hpp>

namespace gpuav {
namespace spirv {

struct Module;
struct Variable;
struct BasicBlock;

// Common helpers for all passes
class Pass {
  public:
    virtual void Run() = 0;

    // Finds (and creates if needed) decoration and returns the OpVariable it points to
    const Variable& GetBuiltinVariable(uint32_t built_in);

    // Returns the ID for OpCompositeConstruct it creates
    uint32_t CreateStageInfo(spv::ExecutionModel execution_model, BasicBlock& block);

  protected:
    Pass(Module& module) : module_(module) {}
    Module& module_;
};

}  // namespace spirv
}  // namespace gpuav