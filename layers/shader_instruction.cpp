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
    words_.push_back(*it++);
    for (uint32_t i = 1; i < length(); i++) {
        words_.push_back(*it++);
    }

    const bool has_result = OpcodeHasResult(opcode());
    if (OpcodeHasType(opcode())) {
        type_id_ = 1;
        if (has_result) {
            result_id_ = 2;
        }
    } else if (has_result) {
        result_id_ = 1;
    }
}

atomic_instruction_info Instruction::GetAtomicInfo(const SHADER_MODULE_STATE& module_state) const {
    atomic_instruction_info info;

    // All atomics have a pointer referenced
    const uint32_t pointer_index = opcode() == spv::OpAtomicStore ? 1 : 3;
    const Instruction* access = module_state.FindDef(word(pointer_index));

    // spirv-val will catch if not OpTypePointer
    const Instruction* pointer = module_state.FindDef(access->word(1));
    info.storage_class = pointer->word(2);

    const Instruction* data_type = module_state.FindDef(pointer->word(3));
    info.type = data_type->opcode();

    // TODO - Should have a proper GetBitWidth like spirv-val does
    assert(data_type->opcode() == spv::OpTypeFloat || data_type->opcode() == spv::OpTypeInt);
    info.bit_width = data_type->word(2);

    return info;
}

spv::BuiltIn Instruction::GetBuiltIn() const {
    if (opcode() == spv::OpDecorate) {
        return static_cast<spv::BuiltIn>(word(3));
    } else if (opcode() == spv::OpMemberDecorate) {
        return static_cast<spv::BuiltIn>(word(4));
    } else {
        assert(false); // non valid opcode
        return spv::BuiltInMax;
    }
}