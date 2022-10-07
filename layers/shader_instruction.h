/* Copyright (c) 2022 The Khronos Group Inc.
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
 * Author: Spencer Fricke <spencerfricke@gmail.com>
 *
 * The Shader Instruction file is in charge of holding instruction information
 */
#ifndef VULKAN_SHADER_INSTRUCTION_H
#define VULKAN_SHADER_INSTRUCTION_H

#include <stddef.h>  // size_t for gcc
#include <stdint.h>
#include <vector>
#include <spirv/unified1/spirv.hpp>

struct SHADER_MODULE_STATE;

struct atomic_instruction_info {
    uint32_t storage_class;
    uint32_t bit_width;
    uint32_t type;  // ex. OpTypeInt
};

// Holds information about a single SPIR-V instruction
// Provides easy access to len, opcode, and content words without the caller needing to care too much about the physical SPIRV module layout.
//
// For more information of the physical module layout to help understand this struct:
// https://github.com/KhronosGroup/SPIRV-Guide/blob/master/chapters/parsing_instructions.md
class Instruction {
    public:
    Instruction(std::vector<uint32_t>::const_iterator it);
    ~Instruction() {}

    // The word used to define the Instruction
    uint32_t word(size_t index) const { return words_[index]; }

    // The words used to define the Instruction
    const std::vector<uint32_t>& words() const { return words_; }

    uint32_t length() const { return words_[0] >> 16; }

    uint32_t opcode() const { return words_[0] & 0x0ffffu; }

    // operand id index, return 0 if no result
    uint32_t ResultId() const { return result_id_; }
    // operand id index, return 0 if no type
    uint32_t TypeId() const { return type_id_; }

    char const * GetAsString(uint32_t operand) const { return (char const *)&words_.at(operand); }
    atomic_instruction_info GetAtomicInfo(const SHADER_MODULE_STATE& module_state) const;
    spv::BuiltIn GetBuiltIn() const;

    private:
        std::vector<uint32_t> words_;
        uint32_t result_id_;
        uint32_t type_id_;
};

#endif  // VULKAN_SHADER_INSTRUCTION_H