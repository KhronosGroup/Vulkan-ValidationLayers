/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
#include <vector>
#include <string>
#include "state_tracker/shader_instruction.h"

// When producing error messages for SPIR-V related items and the user generated the shader with debug information, we can use these
// helpers to print out information from their High Level source instead of some cryptic SPIR-V jargon

const spirv::Instruction *FindOpString(const std::vector<spirv::Instruction> &instructions, uint32_t string_id);

void ReadOpSource(const std::vector<spirv::Instruction> &instructions, const uint32_t reported_file_id,
                  std::vector<std::string> &out_opsource_lines);

void ReadDebugSource(const std::vector<spirv::Instruction> &instructions, const uint32_t debug_source_id,
                     uint32_t &out_file_string_id, std::vector<std::string> &out_opsource_lines);

bool GetLineAndFilename(const std::string &string, uint32_t *linenumber, std::string &filename);