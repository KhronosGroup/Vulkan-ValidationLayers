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
namespace cst {

// Number of indices held in the buffer used to index commands and validation resources
inline constexpr uint32_t indices_count = 16384;

// Error Output Buffer Offsets
// ---

// Error output buffer size
inline constexpr uint32_t error_output_buffer_size_offset = 0;

// Written words count in error output buffer.
// Shaders will atomically read and update this value so as not to overwrite each others records. This value must be  initialized to
// zero
inline constexpr uint32_t error_output_buffer_written_words_count_offset = 1;

// Error output buffer
inline constexpr uint32_t error_output_buffer_error_records_offset = 2;

}  // namespace cst
}  // namespace gpuav
