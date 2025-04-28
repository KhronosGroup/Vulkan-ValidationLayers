/* Copyright (c) 2024-2025 LunarG, Inc.
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

struct Type;

class PostProcessDescriptorIndexingPass : public Pass {
  public:
    PostProcessDescriptorIndexingPass(Module& module);
    const char* Name() const final { return "PostProcessDescriptorIndexingPass"; }

    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        uint32_t descriptor_set = 0;
        uint32_t descriptor_binding = 0;
        uint32_t descriptor_index_id = 0;
        uint32_t variable_id = 0;
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    void CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    uint32_t GetLinkFunctionId();

    // Function IDs to link in
    uint32_t link_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
