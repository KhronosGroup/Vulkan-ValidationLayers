// Copyright (c) 2021-2026 The Khronos Group Inc.
// Copyright (c) 2021-2026 Valve Corporation
// Copyright (c) 2021-2026 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// Values used between the GLSL shaders and the GPU-AV logic

// NOTE: This header is included by the instrumentation shaders and glslang doesn't support #pragma once
#ifndef MEM_COMP_H
#define MEM_COMP_H

#ifdef __cplusplus

#include <cstdint>

namespace gpuav {
namespace glsl {
using uint = uint32_t;
#else
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#else
#error No extension available for 64-bit integers.
#endif
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_scalar_block_layout : require
#endif

// Used to know the origin of the index difference
const uint kMemcmp_uvec4_diff = 0;
const uint kMemcmp_u32_diff = 1;
const uint kMemcmp_u16_diff = 2;
const uint kMemcmp_u8_diff = 3;

struct MemShaderPushData {
    uint64_t updated_indices;
    uint64_t original_indices;
    uint uvec4_count;
    uint u32_count;
    uint u16_count;
    uint u8_count;
    uint error_info_i;
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
