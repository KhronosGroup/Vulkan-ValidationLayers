/* Copyright (c) 2023 LunarG, Inc.
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

namespace gpuav {
namespace spirv {

struct Module;
struct Instruction;

// Core data structure of module.
// The vector acts as our linked list to iterator and make occasional insertions.
// The unique_ptr allows us to create instructions outside module scope and bring them back.
using InstructionList = std::vector<std::unique_ptr<Instruction>>;
using InstructionIt = InstructionList::iterator;

// Since CFG analysis/manipulation is not a main focus, Blocks/Funcitons are just simple containers for ordering Instructions
struct BasicBlock {
    BasicBlock(std::unique_ptr<Instruction> label) {
        // Used when loading initial SPIR-V
        instructions_.push_back(std::move(label));  // OpLabel
    }
    BasicBlock(Module& module);

    void ToBinary(std::vector<uint32_t>& out);

    const Instruction& GetLabel() { return *instructions_[0].get(); }

    const Instruction& CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words);

    InstructionList instructions_;
};

using BasicBlockList = std::vector<std::unique_ptr<BasicBlock>>;
using BasicBlockIt = BasicBlockList::iterator;

struct Function {
    Function(Module& module, std::unique_ptr<Instruction> function_insn) : module_(module) {
        // Used when loading initial SPIR-V
        pre_block_inst_.push_back(std::move(function_insn));  // OpFunction
    }
    Function(Module& module) : module_(module) {}

    void ToBinary(std::vector<uint32_t>& out);

    const Instruction& GetDef() { return *pre_block_inst_[0].get(); }

    // Adds a new block after and returns reference to it
    BasicBlockIt InsertNewBlock(BasicBlockIt it);

    Module& module_;
    // OpFunction and parameters
    InstructionList pre_block_inst_;
    // All basic blocks inside this function in specification order
    BasicBlockList blocks_;
    // normally just OpFunctionEnd, but could be non-semantic
    InstructionList post_block_inst_;
};

using FunctionList = std::vector<std::unique_ptr<Function>>;
using FunctionIt = FunctionList::iterator;

}  // namespace spirv
}  // namespace gpuav