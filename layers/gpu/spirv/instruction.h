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
#include <stddef.h>
#include <vector>
#include "containers/custom_containers.h"
#include <spirv/unified1/spirv.hpp>

struct OperandInfo;

// Need to use until we have native std::span in c++20
using spirv_iterator = vvl::enumeration<const u32, const u32*>::iterator;

namespace gpu {
namespace spirv {

static constexpr u32 kLinkedInstruction = std::numeric_limits<u32>::max();

class Module;

// Represents a single Spv::Op instruction
struct Instruction {
    Instruction(spirv_iterator it, u32 position);

    // Assumes caller will fill remaining words
    Instruction(u32 length, spv::Op opcode);
    void Fill(const std::vector<u32>& words);

    void SetResultTypeIndex();

    // The word used to define the Instruction
    u32 Word(u32 index) const { return words_[index]; }
    // Skips pass any optional Result or Result Type word
    u32 Operand(u32 index) const { return words_[operand_index_ + index]; }

    u32 Length() const { return words_[0] >> 16; }

    u32 Opcode() const { return words_[0] & 0x0ffffu; }

    // operand id, return 0 if no result
    u32 ResultId() const { return (result_id_index_ == 0) ? 0 : words_[result_id_index_]; }
    // operand id, return 0 if no type
    u32 TypeId() const { return (type_id_index_ == 0) ? 0 : words_[type_id_index_]; }

    // Increments Lenght() as well
    void AppendWord(u32 word);

    void ReplaceResultId(u32 new_result_id);
    // searchs all operands to replace ID if found
    void ReplaceOperandId(u32 old_word, u32 new_word);
    void ReplaceLinkedId(vvl::unordered_map<u32, u32>& id_swap_map);

    bool IsArray() const { return (Opcode() == spv::OpTypeArray || Opcode() == spv::OpTypeRuntimeArray); }

    // SPIR-V spec: "A string is interpreted as a nul-terminated stream of characters"
    char const* GetAsString(u32 index) const {
        assert(index < Length());
        return (char const*)&words_[index];
    }

    void ToBinary(std::vector<u32>& out);

    bool operator==(Instruction const& other) const { return words_ == other.words_; }
    bool operator!=(Instruction const& other) const { return words_ != other.words_; }

    // Store minimal extra data
    u32 result_id_index_ = 0;
    u32 type_id_index_ = 0;
    u32 operand_index_ = 1;
    // used to find original position of instruction in shader, pre-instrumented
    u32 position_index_ = 0;
    const OperandInfo& operand_info_;

    // When this class was created, for SPIR-V Instructions that could be used in Vulkan,
    //   414 of 423 had 6 or less operands
    //   361 of 423 had 5 or less operands
    //   287 of 423 had 4 or less operands
    // An extra word is still needed because each insturction has one word prior to the operands
    static constexpr u32 word_vector_length = 7;

    // Max capacity needs to be u32 because an instruction can have a string operand that is (2^16)-1 bytes long
    small_vector<u32, word_vector_length, u32> words_;

    void UpdateDebugInfo();
#ifndef NDEBUG
    // Helping values to make debugging what is happening in a instruction easier
    spv::Op d_opcode_;
    // helps people new to using SPIR-V spec to understand Word()
    u32 d_length_;
    u32 d_result_id_;
    u32 d_type_id_;
    u32 d_words_[12];
#endif
};

void GenerateInstructions(const vvl::span<const u32>& spirv, std::vector<Instruction>& instructions);

}  // namespace spirv
}  // namespace gpu