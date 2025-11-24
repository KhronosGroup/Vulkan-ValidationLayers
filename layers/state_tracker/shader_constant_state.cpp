/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 */

#include "shader_constant_state.h"
#include <vulkan/vulkan_core.h>
#include "shader_module.h"

namespace spirv {

ConstantState::ConstantState(const spirv::Module* module_state, const VkSpecializationInfo* spec_info)
    : module_state(module_state), spec_info(spec_info) {}

bool ConstantState::GetBooleanValue(const spirv::Instruction& insn, bool* value) const {
    const spirv::Instruction* type_id = module_state->FindDef(insn.Word(1));
    if (type_id->Opcode() != spv::OpTypeBool) {
        return false;
    }

    if (insn.Opcode() == spv::OpConstantFalse) {
        *value = false;
        return true;
    } else if (insn.Opcode() == spv::OpConstantTrue) {
        *value = true;
        return true;
    } else if (insn.Opcode() == spv::OpSpecConstantTrue || insn.Opcode() == spv::OpSpecConstantFalse) {
        *value = insn.Opcode() == spv::OpSpecConstantTrue;  // default value
        const uint32_t spec_id = module_state->static_data_.id_to_spec_id.at(insn.Word(2));
        if (spec_info && spec_id < spec_info->mapEntryCount) {
            memcpy(value, (uint8_t*)spec_info->pData + spec_info->pMapEntries[spec_id].offset, 1);
        }
        return true;
    }

    // This means the value is not known until runtime and will need to be checked in GPU-AV
    return false;
}

bool ConstantState::GetInt32Value(const spirv::Instruction& insn, uint32_t* value) const {
    const spirv::Instruction* type_id = module_state->FindDef(insn.Word(1));
    if (type_id->Opcode() != spv::OpTypeInt || type_id->Word(2) != 32) {
        return false;
    }

    if (insn.Opcode() == spv::OpConstant) {
        *value = insn.Word(3);
        return true;
    } else if (insn.Opcode() == spv::OpSpecConstant) {
        *value = insn.Word(3);  // default value
        const uint32_t spec_id = module_state->static_data_.id_to_spec_id.at(insn.Word(2));
        if (spec_info && spec_id < spec_info->mapEntryCount) {
            memcpy(value, (uint8_t*)spec_info->pData + spec_info->pMapEntries[spec_id].offset,
                   spec_info->pMapEntries[spec_id].size);
        }
        return true;
    }

    // This means the value is not known until runtime and will need to be checked in GPU-AV
    return false;
}

}  // namespace spirv