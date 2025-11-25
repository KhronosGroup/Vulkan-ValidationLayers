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

#pragma once
#include <vulkan/vulkan.h>

namespace spirv {
struct Module;
class Instruction;
// This helper is here to help get the constant values out of SPIR-V shaders
//
// The 2 main issues with Spec Constants for us are:
// 1. We create spirv::Module object that is tied to the SPIR-V prior to freezing the spec constants, if we need to lookup the
// constant value for a check, we need to have a way (in CoreCheck and GPU-AV) to do the Spec Constant value lookup.
// 2. If we need them later after the pipeline, we need to save the values to look up later
//
// TODO - Need to handle OpSpecConstantOp
class ConstantState {
  public:
    ConstantState(const spirv::Module* module_state, const VkSpecializationInfo* info);

    bool GetBooleanValue(const spirv::Instruction& insn, bool* value) const;
    bool GetInt32Value(const spirv::Instruction& insn, uint32_t* value) const;

    uint32_t GetSpecConstInt32Value(const spirv::Instruction& insn) const;

  private:
    const spirv::Module* module_state = nullptr;
    const VkSpecializationInfo* spec_info = nullptr;  // will be a copy of a safe_struct
};

}  // namespace spirv