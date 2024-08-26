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

namespace spirv {

// Holds information about a single SPIR-V instruction
// Provides easy access to len, opcode, and content words without the caller needing to care too much about the physical SPIRV
// module layout.
//
// For more information of the physical module layout to help understand this struct:
// https://github.com/KhronosGroup/SPIRV-Guide/blob/main/chapters/parsing_instructions.md
class Instruction {
  public:
    Instruction(std::vector<u32>::const_iterator it);
    Instruction(const u32* it);
    ~Instruction() = default;

    // The word used to define the Instruction
    u32 Word(u32 index) const { return words_[index]; }

    u32 Length() const { return words_[0] >> 16; }

    u32 Opcode() const { return words_[0] & 0x0ffffu; }

    // operand id, return 0 if no result
    u32 ResultId() const { return (result_id_index_ == 0) ? 0 : words_[result_id_index_]; }
    // operand id, return 0 if no type
    u32 TypeId() const { return (type_id_index_ == 0) ? 0 : words_[type_id_index_]; }

    // Used when need to print information for an error message
    std::string Describe() const;

    // Only used to get strings in SPIR-V instructions
    // SPIR-V spec (and spirv-val) ensure:
    // "A string is interpreted as a nul-terminated stream of characters"
    char const* GetAsString(u32 operand) const {
        assert(operand < Length());
        return (char const*)&words_[operand];
    }

    u32 GetConstantValue() const;
    u32 GetBitWidth() const;
    u32 GetByteWidth() const { return (GetBitWidth() + 31) / 32; }
    spv::BuiltIn GetBuiltIn() const;

    bool IsArray() const;
    // Helpers for OpTypeImage
    spv::Dim FindImageDim() const;
    bool IsImageArray() const;
    bool IsImageMultisampled() const;

    // Auto-generated helper functions
    spv::StorageClass StorageClass() const;

    bool operator==(Instruction const& other) const { return words_ == other.words_; }
    bool operator!=(Instruction const& other) const { return words_ != other.words_; }

  private:
    void SetResultTypeIndex();
    void UpdateDebugInfo();

    // When this class was created, for SPIR-V Instructions that could be used in Vulkan,
    //   414 of 423 had 6 or less operands
    //   361 of 423 had 5 or less operands
    //   287 of 423 had 4 or less operands
    // An extra word is still needed because each insturction has one word prior to the operands
    static constexpr u32 word_vector_length = 7;

    // Max capacity needs to be u32 because an instruction can have a string operand that is (2^16)-1 bytes long
    small_vector<u32, word_vector_length, u32> words_;
    u32 result_id_index_ = 0;
    u32 type_id_index_ = 0;

#ifndef NDEBUG
    // Helping values to make debugging what is happening in a instruction easier
    std::string d_opcode_;
    u32 d_length_;
    u32 d_result_id_;
    u32 d_type_id_;
    // helps people new to using SPIR-V spec to understand Word()
    u32 d_words_[12];
#endif
};

}  // namespace spirv
