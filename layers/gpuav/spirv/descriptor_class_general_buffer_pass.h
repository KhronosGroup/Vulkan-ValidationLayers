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
#include "containers/custom_containers.h"
#include "pass.h"

namespace gpuav {
namespace spirv {

// Will make sure Buffers (Storage and Uniform Buffers) that are non bindless are not OOB Uses robustBufferAccess to ensure if we
// are OOB that it won't crash and we will return the error safely
class DescriptorClassGeneralBufferPass : public Pass {
  public:
    DescriptorClassGeneralBufferPass(Module& module);
    const char* Name() const final { return "DescriptorClassGeneralBufferPass"; }

    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    struct InstructionMeta {
        uint32_t descriptor_set = 0;
        uint32_t descriptor_binding = 0;
        uint32_t descriptor_index_id = 0;  // index input the descriptor array

        // The Type of the OpVariable that is being accessed
        const Type* descriptor_type = nullptr;

        // List of OpAccessChains fom the Store/Load down to the OpVariable
        // The front() will be closet to the exact spot accesssed
        // The back() will be closest to the OpVariable
        // (note GLSL will try to always create a single large OpAccessChain)
        std::vector<const Instruction*> access_chain_insts;

        void Reset() {
            descriptor_set = 0;
            descriptor_binding = 0;
            descriptor_index_id = 0;
            descriptor_type = nullptr;
            access_chain_insts.clear();
        }
    };

    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta, bool pre_pass);
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data,
                                const InstructionMeta& meta);
    void Reset() final;

    uint32_t link_function_id = 0;
    uint32_t GetLinkFunctionId();

    // Finds the offset into the SSBO/UBO
    uint32_t FindLastByteOffset(uint32_t descriptor_id, bool is_descriptor_array,
                                const std::vector<const Instruction*>& access_chain_insts) const;

    // If robustBufferAccess is turned on, we can use that to ensure the hardware will handle OOB accesses (in this case it is very
    // "safe" actually)
    uint32_t unsafe_mode_;
    // If there is a shader like
    //    if (x)
    //        ssbo.data[6] = 0;
    //    else
    //        ssbo.data[8] = 0;
    // We don't know which will actually execute, but by definition a Block must execute from start to finish
    //
    // < Descriptor SSA ID, Highest offset byte that will be accessed >
    vvl::unordered_map<uint32_t, uint32_t> block_highest_offset_map_;
};

}  // namespace spirv
}  // namespace gpuav