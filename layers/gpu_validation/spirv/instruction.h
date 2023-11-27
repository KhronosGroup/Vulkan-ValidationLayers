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
#include <stddef.h>
#include <vector>
#include "containers/custom_containers.h"
#include <spirv/unified1/spirv.hpp>

namespace gpuav {
namespace spirv {

struct Module;

// Represents a single Spv::Op instruction
struct Instruction {
    Instruction(std::vector<uint32_t>::const_iterator it, uint32_t position = 0);
    Instruction(const uint32_t* words);

    // Assumes caller will fill remaining words
    Instruction(uint32_t length, spv::Op opcode);
    void Fill(const std::vector<uint32_t>& words) {
        for (uint32_t word : words) {
            words_.emplace_back(word);
        }
    }

    void SetResultTypeIndex();

    // The word used to define the Instruction
    uint32_t Word(uint32_t index) const { return words_[index]; }

    uint32_t Length() const { return words_[0] >> 16; }

    uint32_t Opcode() const { return words_[0] & 0x0ffffu; }

    // operand id, return 0 if no result
    uint32_t ResultId() const { return (result_id_index_ == 0) ? 0 : words_[result_id_index_]; }
    // operand id, return 0 if no type
    uint32_t TypeId() const { return (type_id_index_ == 0) ? 0 : words_[type_id_index_]; }

    // Increments Lenght() as well
    void AppendWord(uint32_t word);

    void ReplaceResultId(uint32_t new_result_id) { words_[result_id_index_] = new_result_id; }
    void ReplaceLinkedId(vvl::unordered_map<uint32_t, uint32_t>& id_swap_map);

    void ToBinary(std::vector<uint32_t>& out);

    // Store minimal extra data
    uint32_t result_id_index_ = 0;
    uint32_t type_id_index_ = 0;
    // used to find original position of instruction in shader, pre-instrumented
    uint32_t position_index_ = 0;

    // When this class was created, for SPIR-V Instructions that could be used in Vulkan,
    //   414 of 423 had 6 or less operands
    //   361 of 423 had 5 or less operands
    //   287 of 423 had 4 or less operands
    // An extra word is still needed because each insturction has one word prior to the operands
    static constexpr uint32_t word_vector_length = 7;

    // Max capacity needs to be uint32_t because an instruction can have a string operand that is (2^16)-1 bytes long
    small_vector<uint32_t, word_vector_length, uint32_t> words_;
};

}  // namespace spirv
}  // namespace gpuav