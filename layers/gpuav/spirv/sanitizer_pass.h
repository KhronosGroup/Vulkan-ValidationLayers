/* Copyright (c) 2025 LunarG, Inc.
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

// This pass catches things that are not a dedicated VU, things such as, but not limited to
// - dividing by zero (for an int)
// - overflows
// - undefined casting
// - reaching OpUnreachable
// - etc
class SanitizerPass : public Pass {
  public:
    SanitizerPass(Module& module);
    const char* Name() const final { return "SanitizerPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Type* result_type = nullptr;
        const Instruction* target_instruction = nullptr;
    };

    bool RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta);

    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    uint32_t GetLinkFunctionId();

    // Function IDs to link in
    uint32_t link_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav