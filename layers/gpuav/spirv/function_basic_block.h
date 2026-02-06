/* Copyright (c) 2024-2026 LunarG, Inc.
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
#include <vector>
#include <list>
#include <memory>
#include <spirv/unified1/spirv.hpp>
#include "state_tracker/shader_instruction.h"

namespace gpuav {
namespace spirv {

class Module;
struct Function;

// Core data structure of module.
// We use the vector for Instruction because inside a block these should be together in memory. We should rarly need to update
// randomly in a block, for those cases, we just need to manage the iterator The unique_ptr allows us to create instructions outside
// module scope and bring them back.
using Instruction = ::spirv::Instruction;
using InstructionList = std::vector<std::unique_ptr<Instruction>>;
using InstructionIt = InstructionList::iterator;

// Since CFG analysis/manipulation is not a main focus, Blocks/Funcitons are just simple containers for ordering Instructions
struct BasicBlock {
    // Used when loading initial SPIR-V
    BasicBlock(std::unique_ptr<Instruction> label);
    // Used for times we need to inject a new block
    BasicBlock(Module& module, Function* function);

    void ToBinary(std::vector<uint32_t>& out) const;

    uint32_t GetLabelId() const;

    // "All OpVariable instructions in a function must be the first instructions in the first block"
    // So need to get the first valid location in block.
    InstructionIt GetFirstInjectableInstrution();
    // Finds instruction before the Block Termination Instruction.
    InstructionIt GetLastInjectableInstrution();

    // Creates an instruction and inserts it before an optional target inst_it.
    // If an InstructionIt is provided, inst_it will be updated to still point at the original target instruction.
    // Otherwise, the new instruction will be created at block end.
    void CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words, InstructionIt* inst_it = nullptr);

    InstructionList instructions_;

    // Set after all functions are discovered (so can assume will be non-null when needed)
    Function* function_ = nullptr;

    // For blocks that are a Loop hader, points to the Merge Target
    uint32_t loop_header_merge_target_ = 0;
    bool IsLoopHeader() const { return loop_header_merge_target_ != 0; }

    // If block terminates with OpBranchConditional/OpSwtich, mark the ID they point to
    uint32_t selection_merge_target_ = 0;
    uint32_t branch_conditional_true_ = 0;
    uint32_t branch_conditional_false_ = 0;
    uint32_t switch_default_ = 0;
    std::vector<uint32_t> switch_cases_;
};

// Control Flow can be tricky, so having this as a List allows use to easily add/remove/edit blocks around without worrying about
// the iterator breaking from under us.
using BasicBlockList = std::list<std::unique_ptr<BasicBlock>>;
using BasicBlockIt = BasicBlockList::iterator;

struct Function {
    // Used to add functions building up SPIR-V the first time
    Function(Module& module, std::unique_ptr<Instruction> function_inst);
    // Used to link in new functions
    Function(Module& module) : id_(0), module_(module) {}

    // Allow copying when we expend the FunctionList
    // Note we only will emplace_back, never move/delete things from the list
    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;
    Function(Function&& other) noexcept = default;
    Function& operator=(Function&& other) noexcept = delete;

    void ToBinary(std::vector<uint32_t>& out) const;

    BasicBlock& GetFirstBlock() { return *blocks_.front(); }

    // Adds a new block after and returns reference to it
    BasicBlockIt InsertNewBlock(BasicBlockIt it);
    BasicBlock& InsertNewBlockEnd();

    void ReplaceAllUsesWith(uint32_t old_word, uint32_t new_word);

    // Result ID of OpFunction (zero if injected function from GPU-AV as we shouldn't need it)
    const uint32_t id_;
    bool AddedFromInstrumentation() const { return id_ == 0; }

    Module& module_;
    // OpFunction and parameters
    InstructionList pre_block_inst_;
    // All basic blocks inside this function in specification order
    BasicBlockList blocks_;
    // normally just OpFunctionEnd, but could be non-semantic
    InstructionList post_block_inst_;

    vvl::unordered_map<uint32_t, const Instruction*> inst_map_;
    const Instruction* FindInstruction(uint32_t id) const;

    // A slower version of BasicBlock::CreateInstruction() that will search the entire function for |id| and then inject the
    // instruction after. Only to be used if you need to suddenly walk back to find an instruction, but normally instructions should
    // be added as you go forward only.
    void CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words, uint32_t id);

    // This is the uvec4 most consumers will need
    uint32_t stage_info_id_ = 0;
    // The individual IDs making up the uvec4
    uint32_t stage_info_x_id_ = 0;
    uint32_t stage_info_y_id_ = 0;
    uint32_t stage_info_z_id_ = 0;
    uint32_t stage_info_w_id_ = 0;

    // Will be updated once all functions are made and know if called.
    // Lets us know if the function is never going to be called, therefore skipping instrumentation.
    //
    // While spirv-opt should remove unused functions, this is for 2 cases
    // 1. When using multiple entry points, we only want to instrument the functions for this target
    // 2. Some real debug workflows will not have ran spirv-opt 100% and have lingering functions
    bool called_from_target_ = false;
};

// We can keep a list of Structs because we only grow the function
// 1. When we first create the Module and find them all
// 2. When we link them in
// We shouldn't need to store pointers and can loop the list if we need to find a function quickly
using FunctionList = std::vector<Function>;
using FunctionIt = FunctionList::iterator;

}  // namespace spirv
}  // namespace gpuav