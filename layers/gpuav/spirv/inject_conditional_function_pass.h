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

#include "pass.h"

namespace gpuav {
namespace spirv {

// A type of common pass that will inject a function call and link it up later,
// We will have wrap the checks to be safe from bad values crashing things
// For OpStore we will just ignore the store if it is invalid, example:
// Before:
//     bda.data[index] = value;
// After:
//    if (isValid(bda.data, index)) {
//         bda.data[index] = value;
//    }
//
// For OpLoad we replace the value with Zero (via Phi node) if it is invalid, example
// Before:
//     int X = bda.data[index];
//     int Y = bda.data[X];
// After:
//    if (isValid(bda.data, index)) {
//         int X = bda.data[index];
//    } else {
//         int X = 0;
//    }
//    if (isValid(bda.data, X)) {
//         int Y = bda.data[X];
//    } else {
//         int Y = 0;
//    }
class InjectConditionalFunctionPass : public Pass {
  public:
    bool Instrument() final;

  protected:
    InjectConditionalFunctionPass(Module& module);

    // This is metadata tied to a single instruction gathered during RequiresInstrumentation() to be used later
    // TODO - Currently this is shared between the various children parents of InjectConditionalFunctionPass and we should refactor
    // InjectConditionalFunctionPass to have each class manage their looping of functions/blocks so they can only use their own
    // variables
    struct InstructionMeta {
        const Instruction* target_instruction = nullptr;

        // BufferDeviceAddressPass
        // ---
        uint32_t alignment_literal = 0;
        uint32_t type_length = 0;

        // DescriptorIndexingOOBPass
        // ---
        const Instruction* var_inst = nullptr;
        const Instruction* image_inst = nullptr;
        uint32_t descriptor_set = 0;
        uint32_t descriptor_binding = 0;
        uint32_t descriptor_index_id = 0;
        bool is_combined_image_sampler = false;
        // Duplicate values if dealing with SAMPLED_IMAGE and SAMPLER together
        const Instruction* sampler_var_inst = nullptr;
        uint32_t sampler_descriptor_set = 0;
        uint32_t sampler_descriptor_binding = 0;
        uint32_t sampler_descriptor_index_id = 0;

        void Reset() {
            target_instruction = nullptr;

            alignment_literal = 0;
            type_length = 0;

            var_inst = nullptr;
            image_inst = nullptr;
            descriptor_set = 0;
            descriptor_binding = 0;
            descriptor_index_id = 0;
            is_combined_image_sampler = false;
            sampler_var_inst = nullptr;
            sampler_descriptor_set = 0;
            sampler_descriptor_binding = 0;
            sampler_descriptor_index_id = 0;
        }
    };

    BasicBlockIt InjectFunction(Function* function, BasicBlockIt block_it, InstructionIt inst_it,
                                const InjectionData& injection_data, const InstructionMeta& meta);

    // Each pass decides if the instruction should needs to have its function check injected
    virtual bool RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) = 0;
    // A callback from the function injection logic.
    // Each pass creates a OpFunctionCall and returns its result id.
    // If |inst_it| is not null, it will update it to instruction post OpFunctionCall
    virtual uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data,
                                        const InstructionMeta& meta) = 0;

    // Optional notification that a new block is being passed
    virtual void NewBlock(const BasicBlock&){};
};

}  // namespace spirv
}  // namespace gpuav