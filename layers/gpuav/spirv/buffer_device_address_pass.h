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
#include "containers/limits.h"
#include "pass.h"

namespace gpuav {
namespace spirv {

// Create a pass to instrument physical buffer address checking
// This pass instruments all physical buffer address references to check that
// all referenced bytes fall in a valid buffer.
class BufferDeviceAddressPass : public Pass {
  public:
    BufferDeviceAddressPass(Module& module);
    const char* Name() const final { return "BufferDeviceAddressPass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        const Instruction* pointer_inst = nullptr;
        uint32_t alignment_literal = 0;
        uint32_t access_size = 0;
        bool type_is_struct = false;
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    // Function IDs to link in
    uint32_t function_range_id_ = 0;
    uint32_t function_align_id_ = 0;

    // This is find the range of the statically accessed members in a BDA struct
    // We track for each struct, the offsets into it that are statically accessed.
    // From here, we can do the BDA range check just once.
    // (example https://godbolt.org/z/v6boos6Yr)
    struct Range {
        uint32_t min_instruction = 0;  // used to only instrument at the lowest offset
        uint32_t min_struct_offsets = vvl::kU32Max;
        uint32_t max_struct_offsets = 0;
    };
    vvl::unordered_map<uint32_t, Range> block_struct_range_map_;
    vvl::unordered_set<uint32_t> block_skip_list_;
};

}  // namespace spirv
}  // namespace gpuav