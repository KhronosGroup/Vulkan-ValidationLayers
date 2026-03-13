/* Copyright (c) 2026 LunarG, Inc.
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

#include <vulkan/vulkan.h>
#include <stdint.h>
#include <vector>
#include "pass.h"

namespace gpuav {
namespace spirv {

class SharedMemoryOobPass : public Pass {
  public:
    SharedMemoryOobPass(Module& module);
    const char* Name() const final { return "SharedMemoryOobPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    struct IndexCheck {
        uint32_t index_id;
        uint32_t bound;
        uint32_t error_subcode;
    };

    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        uint32_t variable_id = 0;
        std::vector<IndexCheck> checks;
    };

    bool RequiresInstrumentation(const Function& function, BasicBlock& block, InstructionIt& inst_it, const Instruction& inst,
                                 InstructionMeta& meta);
    void CreateFunctionCall(const Function& function, BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    uint32_t link_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
