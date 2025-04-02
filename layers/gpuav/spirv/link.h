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
#include <vector>

namespace gpuav {
namespace spirv {

enum LinkFlags {
    // GLSL can't use the optional Initializer operand for OpVariables
    // This will make all private variables set to zero
    // Currently only does Uint32, but could expand to be all types if we find more usecases
    ZeroInitializeUintPrivateVariables = 0x00000001,
    // Swap the private ErrorPayload struct variable with error_payload_variable_id
    UseErrorPayloadVariable = 0x00000002,
};

// SPIR-V module to link in
struct OfflineModule {
    const uint32_t* words;
    const uint32_t word_count;

    // Optional things to be done when linking
    const uint32_t flags = 0;  // LinkFlags
};

struct OfflineFunction {
    // used for debugging
    const char* opname;
    // Number of bytes into module the OpFunction starts
    const uint32_t offset;
};

struct LinkFunction {
    // Information about the function known offline
    const OfflineFunction& offline;
    // The result ID of OpFunction, different each pass
    const uint32_t id;
};

struct LinkInfo {
    // This struct is for things that are generated and therefor can be created once statically
    // If it's not in generated offline, it will change each pass
    const OfflineModule& module;

    std::vector<LinkFunction> functions;

    // This is created once per pass and handed to the module to be linked afterwards
    explicit LinkInfo(const OfflineModule& module) : module(module) {}
};

}  // namespace spirv
}  // namespace gpuav