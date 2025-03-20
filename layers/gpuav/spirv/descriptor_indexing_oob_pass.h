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
#include "inject_conditional_function_pass.h"

namespace gpuav {
namespace spirv {

// Create a pass to instrument descriptor indexing.
// This pass makes sure any index into an descriptor array is not OOB or uninitialized
class DescriptorIndexingOOBPass : public InjectConditionalFunctionPass {
  public:
    DescriptorIndexingOOBPass(Module& module) : InjectConditionalFunctionPass(module) {}
    const char* Name() const final { return "DescriptorIndexingOOBPass"; }
    bool EarlySkip() const final;
    void PrintDebugInfo() const final;

    void NewBlock(const BasicBlock& block, bool is_original_new_block) override;

  private:
    bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) final;
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data,
                                const InstructionMeta& meta) final;

    uint32_t link_function_id_bindless_ = 0;
    uint32_t link_function_id_bindless_combined_image_sampler_ = 0;
    uint32_t link_function_id_non_bindless_ = 0;
    uint32_t GetLinkFunctionId(bool is_combined_image_sampler);

    // < original ID, new CopyObject ID >
    vvl::unordered_map<uint32_t, uint32_t> copy_object_map_;

    // If the shader has a 'imageArray[]' the OpVariable will point to 'imageArray' then we only need to check (in unsafe mode) each
    // index into it once to see if the descriptor is valid or not . We marks which variables were already instrumented
    // < Variable ID, [descriptor index IDs accessed with this variable >
    vvl::unordered_map<uint32_t, vvl::unordered_set<uint32_t>> block_instrumented_table_;
};

}  // namespace spirv
}  // namespace gpuav