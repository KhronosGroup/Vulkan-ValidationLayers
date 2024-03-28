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
#include <spirv/unified1/spirv.hpp>
#include "function_basic_block.h"

namespace gpuav {
namespace spirv {

class Module;
struct Variable;
struct BasicBlock;

// Common helpers for all passes
class Pass {
  public:
    void Run();

    // Finds (and creates if needed) decoration and returns the OpVariable it points to
    const Variable& GetBuiltinVariable(uint32_t built_in);

    // Returns the ID for OpCompositeConstruct it creates
    uint32_t GetStageInfo(Function& function);

    const Instruction* GetDecoration(uint32_t id, spv::Decoration decoration);
    const Instruction* GetMemeberDecoration(uint32_t id, uint32_t member_index, spv::Decoration decoration);

    // Generate SPIR-V needed to help convert things to be uniformly uint32_t
    // If no inst_it is passed in, any new instructions will be added to end of the Block
    uint32_t ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it = nullptr);
    uint32_t CastToUint32(uint32_t id, BasicBlock& block, InstructionIt* inst_it = nullptr);

  protected:
    Pass(Module& module) : module_(module) {}
    Module& module_;

    BasicBlockIt InjectFunctionCheck(Function* function, BasicBlockIt block_it, InstructionIt inst_it);

    // Each pass decides if the instruction should needs to have its function check injected
    virtual bool AnalyzeInstruction(const Function& function, const Instruction& inst) = 0;
    // A callback from the function injection logic.
    // Each pass creates a OpFunctionCall and returns its result id.
    virtual uint32_t CreateFunctionCall(BasicBlock& block) = 0;
    virtual void Reset() = 0;
};

}  // namespace spirv
}  // namespace gpuav