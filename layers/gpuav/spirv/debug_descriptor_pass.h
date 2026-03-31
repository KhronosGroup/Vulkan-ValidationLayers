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

#include <stdint.h>
#include "pass.h"

namespace gpuav {
namespace spirv {

struct Type;

// Create a pass to instrument NonSemantic.DebugDebug (GL_EXT_debug_descriptor) instructions
class DebugDescriptorPass : public Pass {
  public:
    DebugDescriptorPass(Module& module, uint32_t binding_slot) : Pass(module, kNullOffline), binding_slot_(binding_slot) {}
    const char* Name() const final { return "DebugDescriptorPass"; }

    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        bool dump_all = false;
    };

    bool RequiresInstrumentation(const Instruction& inst, InstructionMeta& meta);
    void CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    bool Validate(const Function& current_function, const InstructionMeta& meta);

    const uint32_t binding_slot_;
    uint32_t ext_import_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
