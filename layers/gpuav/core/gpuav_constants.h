/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include <cstdint>

namespace gpuav {
// constant
namespace cst {

// Number of indices held in the buffer used to index commands and validation resources
inline constexpr uint32_t indices_count = 1u << 13;  // 8192
// If we hit our limit, we will use this to signal to the app what they are seeing is likely garbage.
// This is required because we still need to bind our descriptors regardless.
inline constexpr uint32_t invalid_index_command = indices_count - 1;

// TODO - Need to develop a proper way to gather ALL binding between shader instrumentation and command validation
// For now we get enough for DebugPrintf
inline constexpr uint32_t total_internal_descriptors = 64;

// Stream Output Buffer Offsets
//
// The following values provide offsets into the output buffer struct
// ------------------------------

// The 1st member of the debug output buffer contains a set of flags
// controlling the behavior of instrumentation code.
inline constexpr uint32_t stream_output_flags_offset = 0;

// Values stored at output_flags_offset
inline constexpr uint32_t inst_buffer_oob_enabled = 0x1;

// The 2nd member of the debug output buffer contains the next available word
// in the data stream to be written. Shaders will atomically read and update
// this value so as not to overwrite each others records. This value must be
// initialized to zero
inline constexpr uint32_t stream_output_size_offset = 1;

// The 3rd member of the output buffer is the start of the stream of records
// written by the instrumented shaders. Each record represents a validation
// error. The format of the records is documented below.
inline constexpr uint32_t stream_output_data_offset = 2;

}  // namespace cst
}  // namespace gpuav
