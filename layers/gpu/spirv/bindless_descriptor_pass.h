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

// Create a pass to instrument bindless descriptor checking
// This pass instruments all bindless references to check that descriptor
// array indices are inbounds, and if the descriptor indexing extension is
// enabled, that the descriptor has been initialized.
class BindlessDescriptorPass : public InjectConditionalFunctionPass {
  public:
    BindlessDescriptorPass(Module& module) : InjectConditionalFunctionPass(module) {}
    void PrintDebugInfo() final;

  private:
    bool AnalyzeInstruction(const Function& function, const Instruction& inst) final;
    u32 CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data) final;
    void Reset() final;

    u32 link_function_id = 0;
    u32 GetLinkFunctionId();

    const Instruction* access_chain_inst_ = nullptr;
    const Instruction* var_inst_ = nullptr;
    const Instruction* image_inst_ = nullptr;

    u32 descriptor_set_ = 0;
    u32 descriptor_binding_ = 0;
    u32 descriptor_index_id_ = 0;
    u32 descriptor_offset_id_ = 0;

    // < original ID, new CopyObject ID >
    vvl::unordered_map<u32, u32> copy_object_map_;
};

}  // namespace spirv
}  // namespace gpu