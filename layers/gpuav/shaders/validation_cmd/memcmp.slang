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
ConstantBuffer<MemcmpShaderPushData> pc;

static const uint sizeof_u32 = 4;
static const uint sizeof_u16 = 2;
static const uint sizeof_uvec4 = 4 * sizeof_u32;

// From VkIndexType
static const uint VK_INDEX_TYPE_UINT16 = 0;
static const uint VK_INDEX_TYPE_UINT32 = 1;

uint IndexTypeByteSize(uint index_type) {
    if (index_type == VK_INDEX_TYPE_UINT16) {
        return 2;
    } else if (index_type == VK_INDEX_TYPE_UINT32) {
        return 4;
    }
    return 0;
}

// CPU will try to dispatch `primitive_count` threads
[shader("compute")]
[numthreads(MEM_SHADER_WG_SIZE_X, 1, 1)]
void main(uint3 thread_id: SV_DispatchThreadID) {
    const uint gid = thread_id.x;

    if (pc.last_build_as_geometries_gpu == nullptr) {
        return;
    }

    AccelerationStructureGeometriesGPU* last_build = *(pc.last_build_as_geometries_gpu);
    // Can happen is no VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR was issued (deserialized AS)
    if (last_build == nullptr) {
        return;
    }

    if (pc.geometry_i >= last_build->count) {
        return;
    }

    uint64_t build_times_indices = last_build->array[pc.geometry_i].index_buffer_copies;

    // No index snapshot was taken at build time
    if (build_times_indices == 0) {
        return;
    }

    if (pc.update_time_indices == 0) {
        return;
    }

    const uint build_time_indices_byte_size =
        3 * last_build->array[pc.geometry_i].primitive_count * IndexTypeByteSize(last_build->array[pc.geometry_i].index_type);

    // First work on as many uvec4 as possible
    if (gid < pc.uvec4_count) {
        if ((gid + 1) * sizeof_uvec4 > build_time_indices_byte_size) {
            return;
        }
        uint4* update_time_indices_uvec4_array = (uint4*)pc.update_time_indices;
        uint4* build_time_indices_uvec4_array = (uint4*)build_times_indices;
        const uint4 update_time_indices_uvec4 = update_time_indices_uvec4_array[gid];
        const uint4 build_time_indices_uvec4 = build_time_indices_uvec4_array[gid];
        if (any(update_time_indices_uvec4 != build_time_indices_uvec4)) {
            uint update_time_indices_diff = 0;
            uint build_time_indices_diff = 0;
            uint uint_diff_i = 0;
            for (uint i = 0; i < 4; ++i) {
                if (update_time_indices_uvec4[i] != build_time_indices_uvec4[i]) {
                    update_time_indices_diff = update_time_indices_uvec4[i];
                    build_time_indices_diff = build_time_indices_uvec4[i];
                    uint_diff_i = i;
                }
            }

            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_IndexBufferUpdated,
                           pc.error_info_i,
                           gid * sizeof_uvec4 + uint_diff_i * sizeof_u32,
                           update_time_indices_diff,
                           build_time_indices_diff);
        }
        return;
    }

    // Then look at trailing u32(s)
    if (gid < (pc.u32_count + pc.uvec4_count)) {
        const uint u32_i = gid - pc.uvec4_count;
        const uint u32_start_byte_offset = pc.uvec4_count * sizeof_uvec4;
        if (u32_start_byte_offset + (u32_i + 1) * sizeof_u32 > build_time_indices_byte_size) {
            return;
        }
        uint* update_time_indices_uint_array = (uint*)(pc.update_time_indices + u32_start_byte_offset);
        uint* build_time_indices_uint_array = (uint*)(build_times_indices + u32_start_byte_offset);
        const uint update_time_indices_u32 = update_time_indices_uint_array[u32_i];
        const uint build_time_indices_u32 = build_time_indices_uint_array[u32_i];

        if (update_time_indices_u32 != build_time_indices_u32) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_IndexBufferUpdated,
                           pc.error_info_i,
                           u32_start_byte_offset + u32_i * sizeof_u32,
                           update_time_indices_u32,
                           build_time_indices_u32);
        }
        return;
    }

    // Then look at trailing u16
    if (gid < (pc.u32_count + pc.uvec4_count + pc.u16_count)) {
        const uint u16_i = gid - pc.uvec4_count - pc.u32_count;
        const uint u16_start_byte_offset = pc.uvec4_count * sizeof_uvec4 + pc.u32_count * sizeof_u32;
        if (u16_start_byte_offset + (u16_i + 1) * sizeof_u16 > build_time_indices_byte_size) {
            return;
        }
        uint16_t* update_time_indices_u16_array = (uint16_t*)(pc.update_time_indices + u16_start_byte_offset);
        uint16_t* build_time_indices_u16_array = (uint16_t*)(build_times_indices + u16_start_byte_offset);
        const uint16_t update_time_indices_u16 = update_time_indices_u16_array[u16_i];
        const uint16_t build_time_indices_u16 = build_time_indices_u16_array[u16_i];
        if (update_time_indices_u16 != build_time_indices_u16) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_IndexBufferUpdated,
                           pc.error_info_i,
                           u16_start_byte_offset + u16_i * sizeof_u16,
                           uint(update_time_indices_u16),
                           uint(build_time_indices_u16));
        }
        return;
    }

    if (gid >= (pc.u32_count + pc.uvec4_count + pc.u16_count)) {
        return;
    }
}
