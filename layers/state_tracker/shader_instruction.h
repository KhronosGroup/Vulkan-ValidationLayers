/* Copyright (c) 2022-2023 The Khronos Group Inc.
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
 *
 * The Shader Instruction file is in charge of holding instruction information
 */
#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include "containers/custom_containers.h"
#include <spirv/unified1/spirv.hpp>

struct OperandInfo;

// Need to use until we have native std::span in c++20
using spirv_iterator = vvl::enumeration<const uint32_t, const uint32_t*>::iterator;

namespace spirv {

// Holds information about a single SPIR-V instruction
// Provides easy access to len, opcode, and content words without the caller needing to care too much about the physical SPIRV
// module layout.
//
// For more information of the physical module layout to help understand this struct:
// https://github.com/KhronosGroup/SPIRV-Guide/blob/main/chapters/parsing_instructions.md
class Instruction {
  public:
    Instruction(std::vector<uint32_t>::const_iterator it);
    Instruction(const uint32_t* it);
    ~Instruction() = default;

    // The word used to define the Instruction
    uint32_t Word(uint32_t index) const { return words_[index]; }
    // Skips pass any optional Result or Result Type word
    uint32_t Operand(uint32_t index) const { return words_[operand_index_ + index]; }

    uint32_t Length() const { return words_[0] >> 16; }

    uint32_t Opcode() const { return words_[0] & 0x0ffffu; }

    // operand id, return 0 if no result
    uint32_t ResultId() const { return (result_id_index_ == 0) ? 0 : words_[result_id_index_]; }
    // operand id, return 0 if no type
    uint32_t TypeId() const { return (type_id_index_ == 0) ? 0 : words_[type_id_index_]; }

    // Used when need to print information for an error message
    std::string Describe() const;

    // Only used to get strings in SPIR-V instructions
    // SPIR-V spec (and spirv-val) ensure:
    // "A string is interpreted as a nul-terminated stream of characters"
    char const* GetAsString(uint32_t operand) const {
        assert(operand < Length());
        return (char const*)&words_[operand];
    }

    uint32_t GetConstantValue() const;
    uint32_t GetBitWidth() const;
    uint32_t GetByteWidth() const { return (GetBitWidth() + 31) / 32; }
    spv::BuiltIn GetBuiltIn() const;
    uint32_t GetPositionIndex() const { return position_index_; }
    bool IsArray() const;
    bool IsAccessChain() const;
    // Helpers for OpTypeImage
    spv::Dim FindImageDim() const;
    bool IsImageArray() const;
    bool IsImageMultisampled() const;

    // Auto-generated helper functions
    spv::StorageClass StorageClass() const;

    bool operator==(Instruction const& other) const { return words_ == other.words_; }
    bool operator!=(Instruction const& other) const { return words_ != other.words_; }

    // The following is only used for GPU-AV where we need to possibly update an Instruction
    Instruction(spirv_iterator it, uint32_t position);
    // Assumes caller will fill remaining words
    Instruction(uint32_t length, spv::Op opcode);
    void Fill(const std::vector<uint32_t>& words);
    void UpdateWord(uint32_t index, uint32_t data);
    void ToBinary(std::vector<uint32_t>& out);
    // Increments Length() as well
    void AppendWord(uint32_t word);
    void ReplaceResultId(uint32_t new_result_id);
    // searchs all operands to replace ID if found
    void ReplaceOperandId(uint32_t old_word, uint32_t new_word);
    void ReplaceLinkedId(vvl::unordered_map<uint32_t, uint32_t>& id_swap_map);

  private:
    void SetResultTypeIndex();
    void UpdateDebugInfo();

    // When this class was created, for SPIR-V Instructions that could be used in Vulkan,
    //   414 of 423 had 6 or less operands
    //   361 of 423 had 5 or less operands
    //   287 of 423 had 4 or less operands
    // An extra word is still needed because each insturction has one word prior to the operands
    static constexpr uint32_t word_vector_length = 7;

    // Max capacity needs to be uint32_t because an instruction can have a string operand that is (2^16)-1 bytes long
    small_vector<uint32_t, word_vector_length, uint32_t> words_;
    uint32_t result_id_index_ = 0;
    uint32_t type_id_index_ = 0;
    uint32_t operand_index_ = 1;

    // used to find original position of instruction in shader, pre-instrumented
    const uint32_t position_index_;
    const OperandInfo& operand_info_;

#ifndef NDEBUG
    // Helping values to make debugging what is happening in a instruction easier
    std::string d_opcode_;
    uint32_t d_length_;
    uint32_t d_result_id_;
    uint32_t d_type_id_;
    // helps people new to using SPIR-V spec to understand Word()
    uint32_t d_words_[12];
#endif
};

}  // namespace spirv
