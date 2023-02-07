// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_gramar_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2021-2023 The Khronos Group Inc.
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
 * This file is related to anything that is found in the SPIR-V grammar
 * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
 *
 ****************************************************************************/

#pragma once
#include <cstdint>
#include <spirv/unified1/spirv.hpp>


bool AtomicOperation(uint32_t opcode);

bool GroupOperation(uint32_t opcode);


bool ImageGatherOperation(uint32_t opcode);
bool ImageFetchOperation(uint32_t opcode);
bool ImageSampleOperation(uint32_t opcode);

bool OpcodeHasType(uint32_t opcode);
bool OpcodeHasResult(uint32_t opcode);

uint32_t OpcodeMemoryScopePosition(uint32_t opcode);
uint32_t OpcodeExecutionScopePosition(uint32_t opcode);
uint32_t OpcodeImageOperandsPosition(uint32_t opcode);

uint32_t ImageOperandsParamCount(uint32_t opcode);

const char* string_SpvOpcode(uint32_t opcode);
const char* string_SpvStorageClass(uint32_t storage_class);
const char* string_SpvExecutionModel(uint32_t execution_model);

