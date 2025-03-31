/* Copyright (c) 2024-2025 LunarG, Inc.
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
    // Swap the private variable with private_variable_id
    SwapPrivateVariable = 0x00000002,
};

// This struct is for things that are generated and therefor can be created once statically
struct OfflineLinkInfo {
    // SPIR-V module to link in
    const uint32_t* words;
    const uint32_t word_count;

    // used for debugging
    const char* opname;

    // Optional things to be done when linking
    const uint32_t flags = 0;
};

struct LinkInfo {
    // If it's not in generated offline, it will change each pass
    const OfflineLinkInfo& offline;

    // The result ID of OpFunction
    const uint32_t function_id;

    // Used when SwapPrivateVariable is used
    uint32_t private_variable_id = 0;
};

}  // namespace spirv
}  // namespace gpuav