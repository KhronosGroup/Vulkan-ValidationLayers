// Copyright (c) 2021-2024 The Khronos Group Inc.
// Copyright (c) 2021-2024 Valve Corporation
// Copyright (c) 2021-2024 LunarG, Inc.
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
#ifndef GPU_SHADERS_DRAW_PUSH_DATA_H
#define GPU_SHADERS_DRAW_PUSH_DATA_H

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
using uint = unsigned int;
#endif

// Bindings for all pre draw types
const uint kPreDrawBinding_IndirectBuffer = 0;
const uint kPreDrawBinding_CountBuffer = 1;
const uint kPreDrawBinding_IndexBuffer = 2;

// Flag values for all pre draw types

// Set if the count buffer is bound
const uint kPreDrawValidationFlags_CountBuffer = (1 << 0);
// Set if the draw buffer is bound
const uint kPreDrawValidationFlags_DrawBuffer = (1 << 1);
// Set if firstInstance fields of draw structs must be validated
const uint kPreDrawValidationFlags_FirstInstance = (1 << 2);
// Set if the index buffer is bound
const uint kPreDrawValidationFlags_IndexBuffer = (1 << 3);

struct DrawIndirectPushData {
    uint flags;
    uint prop_count_limit;
    uint buffer_count_limit;
    uint draw_count;
    uint draw_stride;
};

struct DrawIndexedPushData {
    uint flags;
    uint prop_count_limit;
    uint buffer_count_limit;
    uint draw_count;
    uint draw_stride;
    uint first_index;
    uint index_buffer_max_count;
    uint index_count;
    uint index_width;
    uint vertex_offset;
    uint smallest_vertex_attributes_count;
};

struct DrawMeshPushData {
    uint flags;
    uint prop_count_limit;
    uint buffer_count_limit;
    uint draw_count;
    uint draw_stride;
    uint max_workgroup_count_x;
    uint max_workgroup_count_y;
    uint max_workgroup_count_z;
    uint max_workgroup_total_count; 
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
