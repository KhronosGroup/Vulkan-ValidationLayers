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

namespace gpu {
namespace spirv {

// Will make sure Buffers (Storage and Uniform Buffers) that are non bindless are not OOB Uses robustBufferAccess to ensure if we
// are OOB that it won't crash and we will return the error safely
class NonBindlessOOBBufferPass : public Pass {
  public:
    NonBindlessOOBBufferPass(Module& module);
    void PrintDebugInfo() final;
    const char* Name() const final { return "NonBindlessOOBBufferPass"; }
    bool Run() final;

  private:
    bool AnalyzeInstruction(const Function& function, const Instruction& inst);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data);
    void Reset() final;

    uint32_t link_function_id = 0;
    uint32_t GetLinkFunctionId();

    const Instruction* access_chain_inst_ = nullptr;
    const Instruction* var_inst_ = nullptr;

    uint32_t descriptor_array_size_id_ = 0;  // the size of the descriptor array (size is 1 if no array)
    uint32_t descriptor_set_ = 0;
    uint32_t descriptor_binding_ = 0;
    uint32_t descriptor_index_id_ = 0;  // index input the descriptor array
    uint32_t descriptor_offset_id_ = 0;
};

}  // namespace spirv
}  // namespace gpu