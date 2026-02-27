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
#include "pass.h"
#include <map>

namespace gpuav {
namespace spirv {

class SharedMemoryDataRacePass : public Pass {
  public:
    SharedMemoryDataRacePass(Module& module);
    const char* Name() const final { return "SharedMemoryDataRacePass"; }
    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;
        // linked function index
        uint32_t function_idx;
        // id of element number computed from the access chain
        uint32_t access_chain_idx_id;
        // start of this variable in the shadow memory
        uint32_t start;
        // number of scalar elements touched by this access
        uint32_t num_elements;
        // given the set of workgroup storage opvariables in the order they appear in the program,
        // this is the index into that set for the variable that is the base of the access chain.
        uint32_t variable_idx;
    };

    bool RequiresInstrumentation(const Function& function, BasicBlock& block, InstructionIt& inst_it, const Instruction& inst,
                                 InstructionMeta& meta);
    void CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta);

    uint32_t GetLinkFunctionId(const InstructionMeta& meta);

    // Function IDs to link in
    uint32_t link_function_id_[4]{};
    // Map shared memory variables to start offset in shadow memory
    std::map<const Variable*, uint32_t> slot_start;
    // index of variable in GetSharedMemoryVariables
    std::map<const Variable*, uint32_t> variable_to_idx;
    // number of slots in shadow memory
    uint32_t num_slots{};
};

}  // namespace spirv
}  // namespace gpuav
