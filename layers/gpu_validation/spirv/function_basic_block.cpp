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

#include "function_basic_block.h"
#include "instruction.h"
#include "module.h"

namespace gpuav {
namespace spirv {

void BasicBlock::ToBinary(std::vector<uint32_t>& out) {
    for (const auto& inst : instructions_) {
        inst->ToBinary(out);
    }
}

void Function::ToBinary(std::vector<uint32_t>& out) {
    for (const auto& inst : pre_block_inst_) {
        inst->ToBinary(out);
    }
    for (const auto& block : blocks_) {
        block->ToBinary(out);
    }
    for (const auto& inst : post_block_inst_) {
        inst->ToBinary(out);
    }
}

BasicBlock::BasicBlock(Module& module) {
    uint32_t new_label_id = module.TakeNextId();
    CreateInstruction(spv::OpLabel, {new_label_id});
}

const Instruction& BasicBlock::CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words) {
    // Add 1 as we need to reserve the first word for the opcode/length
    auto new_insn = std::make_unique<Instruction>(words.size() + 1, opcode);
    new_insn->Fill(words);
    return *instructions_.emplace_back(std::move(new_insn));
}

BasicBlockIt Function::InsertNewBlock(BasicBlockIt it) {
    auto new_block = std::make_unique<BasicBlock>(module_);
    it++;  // make sure it inserted after
    BasicBlockIt new_block_it = blocks_.insert(it, std::move(new_block));

    return new_block_it;
}

}  // namespace spirv
}  // namespace gpuav