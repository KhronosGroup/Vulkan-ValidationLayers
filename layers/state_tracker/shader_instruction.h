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
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "vk_layer_data.h"
#include <spirv/unified1/spirv.hpp>

struct SHADER_MODULE_STATE;

struct AtomicInstructionInfo {
    uint32_t storage_class;
    uint32_t bit_width;
    uint32_t type;  // ex. OpTypeInt
};

// Holds information about a single SPIR-V instruction
// Provides easy access to len, opcode, and content words without the caller needing to care too much about the physical SPIRV
// module layout.
//
// For more information of the physical module layout to help understand this struct:
// https://github.com/KhronosGroup/SPIRV-Guide/blob/main/chapters/parsing_instructions.md
class Instruction {
  public:
    Instruction(std::vector<uint32_t>::const_iterator it);
    ~Instruction() = default;

    // The word used to define the Instruction
    uint32_t Word(uint32_t index) const { return words_[index]; }

    uint32_t Length() const { return words_[0] >> 16; }

    uint32_t Opcode() const { return words_[0] & 0x0ffffu; }

    // operand id index, return 0 if no result
    uint32_t ResultId() const { return result_id_; }
    // operand id index, return 0 if no type
    uint32_t TypeId() const { return type_id_; }

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
    AtomicInstructionInfo GetAtomicInfo(const SHADER_MODULE_STATE& module_state) const;
    spv::BuiltIn GetBuiltIn() const;

    // Auto-generated helper functions
    spv::StorageClass StorageClass() const;

    bool operator==(Instruction const& other) const { return words_ == other.words_; }
    bool operator!=(Instruction const& other) const { return words_ != other.words_; }

  private:
    // When this class was created, for SPIR-V Instructions that could be used in Vulkan,
    //   414 of 423 had 6 or less operands
    //   361 of 423 had 5 or less operands
    //   287 of 423 had 4 or less operands
    // An extra word is still needed because each insturction has one word prior to the operands
    static constexpr uint32_t word_vector_length = 7;

    // Max capacity needs to be uint32_t because an instruction can have a string operand that is (2^16)-1 bytes long
    small_vector<uint32_t, word_vector_length, uint32_t> words_;
    uint32_t result_id_;
    uint32_t type_id_;
};
