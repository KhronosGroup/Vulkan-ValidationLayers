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
#include <spirv/unified1/spirv.hpp>
#include "function_basic_block.h"

namespace gpuav {
namespace spirv {

class Module;
struct Variable;
struct BasicBlock;
struct Type;

// Info we know is the same regardless what pass is consuming the CreateFunctionCall()
struct InjectionData {
    uint32_t stage_info_id;
    uint32_t inst_position_id;
};

// The goal is to have the complex conditional inject control flow to be in a single spot
// To allow this, we create all the pieces for Pass, let it make its own function call, then use this data to apply the final
// control flow logic
struct InjectConditionalData {
    uint32_t merge_block_label;
    uint32_t valid_block_label;
    uint32_t invalid_block_label;
    uint32_t function_result_id;
    BasicBlockIt merge_block_it;
};

// Common helpers for all passes
// The pass takes the Module object and modifies it as needed
class Pass {
  public:
    // Needed to know where an error/warning comes from
    virtual const char* Name() const = 0;
    // Return true if code was instrumented/modified in anyway
    virtual bool Instrument() = 0;
    // Requiring because this becomes important/helpful while debugging
    virtual void PrintDebugInfo() const = 0;
    // Wrapper that each pass can use to start
    bool Run();

    // Finds (and creates if needed) decoration and returns the OpVariable it points to
    const Variable& GetBuiltinVariable(uint32_t built_in);

    // Returns the ID for OpCompositeConstruct it creates
    uint32_t GetStageInfo(Function& function, const BasicBlock& target_block_it, InstructionIt& out_inst_it);
    InjectionData GetInjectionData(Function& function, const BasicBlock& target_block_it, InstructionIt& out_inst_it,
                                   const Instruction& target_instruction);

    const Instruction* GetDecoration(uint32_t id, spv::Decoration decoration) const;
    const Instruction* GetMemberDecoration(uint32_t id, uint32_t member_index, spv::Decoration decoration) const;

    uint32_t FindTypeByteSize(uint32_t type_id, uint32_t matrix_stride = 0, bool col_major = false, bool in_matrix = false) const;
    uint32_t GetLastByte(const Type& descriptor_type, const std::vector<const Instruction*>& access_chain_insts, BasicBlock& block,
                         InstructionIt* inst_it);
    // Generate SPIR-V needed to help convert things to be uniformly uint32_t
    // If no inst_it is passed in, any new instructions will be added to end of the Block
    uint32_t ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) const;
    uint32_t CastToUint32(uint32_t id, BasicBlock& block, InstructionIt* inst_it) const;

    bool IsMaxInstrumentationsCount() const;

    InjectConditionalData InjectFunctionPre(Function& function, const BasicBlockIt original_block_it, InstructionIt inst_it);
    void InjectFunctionPost(BasicBlock& original_block, const InjectConditionalData& ic_data);

  protected:
    Pass(Module& module) : module_(module) {}
    Module& module_;

    // As various things are modifiying the instruction streams, we need to get back to where we were.
    // (normally set in the RequiresInstrumentation call)
    InstructionIt FindTargetInstruction(BasicBlock& block, const Instruction& target_instruction) const;

    uint32_t instrumentations_count_ = 0;
};

}  // namespace spirv
}  // namespace gpuav