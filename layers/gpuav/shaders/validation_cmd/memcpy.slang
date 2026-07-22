// Copyright (c) 2022-2026 The Khronos Group Inc.
// Copyright (c) 2022-2026 Valve Corporation
// Copyright (c) 2022-2026 LunarG, Inc.
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

#include "common.h"
#include "mem.h"

using namespace gpuav::shader;

[vk::push_constant]
ConstantBuffer<MemcpyShaderPushData> pc;

static const uint sizeof_u32 = 4;
static const uint sizeof_u16 = 2;
static const uint sizeof_uvec4 = 4 * sizeof_u32;

// CPU will try to dispatch `primitive_count` threads
[shader("compute")]
[numthreads(MEM_SHADER_WG_SIZE_X, 1, 1)]
void main(uint3 thread_id: SV_DispatchThreadID) {
    const uint gid = thread_id.x;

    // First work on as many uvec4 as possible
    if (gid < pc.uvec4_count) {
        uint4* update_time_indices_uvec4_array = (uint4*)pc.update_time_indices;
        uint4* build_time_indices_uvec4_array = (uint4*)pc.index_buffer_copy;
        build_time_indices_uvec4_array[gid] = update_time_indices_uvec4_array[gid];
        return;
    }

    // Then look at trailing u32(s)
    if (gid < (pc.u32_count + pc.uvec4_count)) {
        const uint u32_i = gid - pc.uvec4_count;
        const uint u32_start_byte_offset = pc.uvec4_count * sizeof_uvec4;
        uint* update_time_indices_u32_array = (uint*)(pc.update_time_indices + u32_start_byte_offset);
        uint* build_time_indices_u32_array = (uint*)(pc.index_buffer_copy + u32_start_byte_offset);
        build_time_indices_u32_array[u32_i] = update_time_indices_u32_array[u32_i];
        return;
    }

    // Then look at trailing u16
    if (gid < (pc.u32_count + pc.uvec4_count + pc.u16_count)) {
        const uint u16_i = gid - pc.uvec4_count - pc.u32_count;
        const uint u16_start_byte_offset = pc.uvec4_count * sizeof_uvec4 + pc.u32_count * sizeof_u32;
        uint16_t* update_time_indices_u16_array = (uint16_t*)(pc.update_time_indices + u16_start_byte_offset);
        uint16_t* build_time_indices_u16_array = (uint16_t*)(pc.index_buffer_copy + u16_start_byte_offset);
        build_time_indices_u16_array[u16_i] = update_time_indices_u16_array[u16_i];
        return;
    }
}
