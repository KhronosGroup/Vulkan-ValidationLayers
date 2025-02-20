/* Copyright (c) 2024-205 LunarG, Inc.
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

#include <stdint.h>

namespace gpuav {
namespace spirv {

enum LinkFlags {
    // GLSL can't use the optional Initializer operand for OpVariables
    // This will make all private variables set to zero
    // Currently only does Uint32, but could expand to be all types if we find more usecases
    ZeroInitializeUintPrivateVariables = 0x00000001,
};

struct LinkInfo {
    // SPIR-V module to link in
    const uint32_t* words;
    const uint32_t word_count;

    // Information about the function it has
    uint32_t function_id;

    // used for debugging
    const char* opname;

    // Optional things to be done when linking
    uint32_t flags = 0;
};

}  // namespace spirv
}  // namespace gpuav