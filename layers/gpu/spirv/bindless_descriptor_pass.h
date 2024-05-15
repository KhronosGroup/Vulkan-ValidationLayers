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

// Create a pass to instrument bindless descriptor checking
// This pass instruments all bindless references to check that descriptor
// array indices are inbounds, and if the descriptor indexing extension is
// enabled, that the descriptor has been initialized. If the reference is
// invalid, a record is written to the debug output buffer (if space allows)
// and a null value is returned.
class BindlessDescriptorPass : public Pass {
  public:
    BindlessDescriptorPass(Module& module) : Pass(module) {}

  private:
    bool AnalyzeInstruction(const Function& function, const Instruction& inst) final;
    uint32_t CreateFunctionCall(BasicBlock& block) final;
    void Reset() final;

    uint32_t FindTypeByteSize(uint32_t type_id, uint32_t matrix_stride = 0, bool col_major = false, bool in_matrix = false);
    uint32_t GetLastByte(BasicBlock& block);

    uint32_t link_function_id = 0;
    uint32_t GetLinkFunctionId();

    const Instruction* access_chain_inst_ = nullptr;
    const Instruction* var_inst_ = nullptr;
    const Instruction* image_inst_ = nullptr;

    const Instruction* target_instruction_ = nullptr;
    uint32_t descriptor_set_ = 0;
    uint32_t descriptor_binding_ = 0;
    uint32_t descriptor_index_id_ = 0;
    uint32_t descriptor_offset_id_ = 0;

    // < original ID, new CopyObject ID >
    vvl::unordered_map<uint32_t, uint32_t> copy_object_map_;
};

}  // namespace spirv
}  // namespace gpuav