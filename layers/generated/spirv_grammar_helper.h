// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_gramar_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2021 The Khronos Group Inc.
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
 * Author: Spencer Fricke <s.fricke@samsung.com>
 *
 * This file is related to anything that is found in the SPIR-V grammar
 * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
 *
 ****************************************************************************/

#pragma once
#include <cstdint>

bool AtomicOperation(uint32_t opcode);

bool GroupOperation(uint32_t opcode);

uint32_t MemoryScopeParam(uint32_t opcode);
uint32_t ExecutionScopeParam(uint32_t opcode);

