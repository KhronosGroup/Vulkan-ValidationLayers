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

namespace gpu {
namespace spirv {

class Module;
struct Variable;
struct BasicBlock;

// Info we know is the same regardless what pass is consuming the CreateFunctionCall()
struct InjectionData {
    u32 stage_info_id;
    u32 inst_position_id;
};

// Common helpers for all passes
class Pass {
  public:
    // Return false if nothing was changed
    virtual bool Run() { return false; }

    virtual void PrintDebugInfo() {}

    // Finds (and creates if needed) decoration and returns the OpVariable it points to
    const Variable& GetBuiltinVariable(u32 built_in);

    // Returns the ID for OpCompositeConstruct it creates
    u32 GetStageInfo(Function& function, BasicBlockIt target_block_it, InstructionIt& target_inst_it);

    const Instruction* GetDecoration(u32 id, spv::Decoration decoration);
    const Instruction* GetMemeberDecoration(u32 id, u32 member_index, spv::Decoration decoration);

    u32 GetLastByte(const Instruction& var_inst, const Instruction& access_chain_inst, BasicBlock& block, InstructionIt* inst_it);
    // Generate SPIR-V needed to help convert things to be uniformly u32
    // If no inst_it is passed in, any new instructions will be added to end of the Block
    u32 ConvertTo32(u32 id, BasicBlock& block, InstructionIt* inst_it);
    u32 CastToUint32(u32 id, BasicBlock& block, InstructionIt* inst_it);

  protected:
    Pass(Module& module) : module_(module) {}
    Module& module_;

    // clear values between instrumented instructions
    virtual void Reset() = 0;

    // As various things are modifiying the instruction streams, we need to get back to where we were.
    // (normally set in the AnalyzeInstruction call)
    const Instruction* target_instruction_ = nullptr;
    InstructionIt FindTargetInstruction(BasicBlock& block) const;

    u32 instrumented_count_ = 0;
};

}  // namespace spirv
}  // namespace gpu