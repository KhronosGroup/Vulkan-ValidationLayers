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
#include <vector>
#include <memory>
#include <spirv/unified1/spirv.hpp>
#include "containers/custom_containers.h"

namespace gpu {
namespace spirv {

class Module;
struct Function;
struct Instruction;

// Core data structure of module.
// The vector acts as our linked list to iterator and make occasional insertions.
// The unique_ptr allows us to create instructions outside module scope and bring them back.
using InstructionList = std::vector<std::unique_ptr<Instruction>>;
using InstructionIt = InstructionList::iterator;

// Since CFG analysis/manipulation is not a main focus, Blocks/Funcitons are just simple containers for ordering Instructions
struct BasicBlock {
    // Used when loading initial SPIR-V
    BasicBlock(std::unique_ptr<Instruction> label, Function& function);
    BasicBlock(Module& module, Function& function);

    void ToBinary(std::vector<uint32_t>& out);

    uint32_t GetLabelId();

    // "All OpVariable instructions in a function must be the first instructions in the first block"
    // So need to get the first valid location in block.
    InstructionIt GetFirstInjectableInstrution();
    // Finds instruction before the Block Termination Instruction.
    InstructionIt GetLastInjectableInstrution();

    // Creates instruction and inserts it before the Instruction, updates poistion after new instruciton.
    // If no InstructionIt is provided, it will add it to the end of the block.
    void CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words, InstructionIt* inst_it = nullptr);

    InstructionList instructions_;
    Function& function_;

    bool loop_header_ = false;
};

using BasicBlockList = std::vector<std::unique_ptr<BasicBlock>>;
using BasicBlockIt = BasicBlockList::iterator;

struct Function {
    Function(Module& module, std::unique_ptr<Instruction> function_inst);
    Function(Module& module) : module_(module) {}

    void ToBinary(std::vector<uint32_t>& out);

    const Instruction& GetDef() { return *pre_block_inst_[0].get(); }
    BasicBlock& GetFirstBlock() { return *blocks_[0]; }

    // Adds a new block after and returns reference to it
    BasicBlockIt InsertNewBlock(BasicBlockIt it);
    void InitBlocks(uint32_t count);

    void ReplaceAllUsesWith(uint32_t old_word, uint32_t new_word);

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
};

using FunctionList = std::vector<std::unique_ptr<Function>>;
using FunctionIt = FunctionList::iterator;

}  // namespace spirv
}  // namespace gpu