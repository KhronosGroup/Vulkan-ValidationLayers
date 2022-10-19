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
 */

#include "shader_instruction.h"
#include "shader_module.h"
#include "spirv_grammar_helper.h"

Instruction::Instruction(std::vector<uint32_t>::const_iterator it) : result_id_(0), type_id_(0) {
    words_.emplace_back(*it++);
    words_.reserve(Length());
    for (uint32_t i = 1; i < Length(); i++) {
        words_.emplace_back(*it++);
    }

    const bool has_result = OpcodeHasResult(Opcode());
    if (OpcodeHasType(Opcode())) {
        type_id_ = 1;
        if (has_result) {
            result_id_ = 2;
        }
    } else if (has_result) {
        result_id_ = 1;
    }
}

std::string Instruction::Describe() const {
    std::ostringstream ss;
    const uint32_t opcode = Opcode();
    uint32_t operand_offset = 1;  // where to start printing operands
    // common disassembled for SPIR-V is
    // %result = Opcode %result_type %operands
    if (OpcodeHasResult(opcode)) {
        operand_offset++;
        ss << "%" << (OpcodeHasType(opcode) ? Word(2) : Word(1)) << " = ";
    }

    ss << string_SpvOpcode(opcode);

    if (OpcodeHasType(opcode)) {
        operand_offset++;
        ss << " %" << Word(1);
    }

    // TODO - For now don't list the '%' for any operands since they are only for reference IDs. Without generating a table of each
    // instructions operand types and covering the many edge cases (such as optional, paired, or variable operands) this is the
    // simplest way to print the instruction and give the developer something to look into when an error occurs.
    //
    // For now this safely should be able to assume it will never come across a LiteralString such as in OpExtInstImport or
    // OpEntryPoint
    for (uint32_t i = operand_offset; i < Length(); i++) {
        ss << " " << Word(i);
    }
    return ss.str();
}

// While simple, function name provides a more human readable description why Word(3) is used
uint32_t Instruction::GetConstantValue() const {
    // This should be a OpConstant (not a OpSpecConstant), if this asserts then 2 things are happening
    // 1. This function is being used where we don't actually know it is a constant and is a bug in the validation layers
    // 2. The CreateFoldSpecConstantOpAndCompositePass didn't fully fold everything and is a bug in spirv-opt
    assert(Opcode() == spv::OpConstant);
    return Word(3);
}

AtomicInstructionInfo Instruction::GetAtomicInfo(const SHADER_MODULE_STATE& module_state) const {
    AtomicInstructionInfo info;

    // All atomics have a pointer referenced
    const uint32_t pointer_index = Opcode() == spv::OpAtomicStore ? 1 : 3;
    const Instruction* access = module_state.FindDef(Word(pointer_index));

    // spirv-val will catch if not OpTypePointer
    const Instruction* pointer = module_state.FindDef(access->Word(1));
    info.storage_class = pointer->Word(2);

    const Instruction* data_type = module_state.FindDef(pointer->Word(3));
    info.type = data_type->Opcode();

    // TODO - Should have a proper GetBitWidth like spirv-val does
    assert(data_type->Opcode() == spv::OpTypeFloat || data_type->Opcode() == spv::OpTypeInt);
    info.bit_width = data_type->Word(2);

    return info;
}

spv::BuiltIn Instruction::GetBuiltIn() const {
    if (Opcode() == spv::OpDecorate) {
        return static_cast<spv::BuiltIn>(Word(3));
    } else if (Opcode() == spv::OpMemberDecorate) {
        return static_cast<spv::BuiltIn>(Word(4));
    } else {
        assert(false);  // non valid Opcode
        return spv::BuiltInMax;
    }
}