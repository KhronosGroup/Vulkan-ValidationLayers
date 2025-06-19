// Copyright (c) 2021-2025 The Khronos Group Inc.
// Copyright (c) 2021-2025 Valve Corporation
// Copyright (c) 2021-2025 LunarG, Inc.
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
#ifndef GPU_SHADERS_PUSH_DATA_H
#define GPU_SHADERS_PUSH_DATA_H

#ifdef __cplusplus

#define BUFFER_ADDR_FWD_DECL(TypeName)
#define BUFFER_ADDR_DECL(TypeName) VkDeviceAddress

#include <cstdint>

namespace gpuav {
namespace glsl {
using uint = uint32_t;
#else
#define BUFFER_ADDR_FWD_DECL(TypeName) layout(buffer_reference) buffer TypeName;
#define BUFFER_ADDR_DECL(TypeName) TypeName

#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#else
#error No extension available for 64-bit integers.
#endif
#extension GL_EXT_buffer_reference : require
#endif

BUFFER_ADDR_FWD_DECL(CountBufferAddr)

BUFFER_ADDR_FWD_DECL(DrawIndexedIndirectCmdsBufferAddr)
BUFFER_ADDR_FWD_DECL(DipatchIndirectBufferAddr)
const uint kIndexedIndirectDrawFlags_DrawCountFromBuffer = uint(1) << 0;
// Most implementations have wave sizes of 32 or 64, so 64 is a good default
const uint DrawIndexedIndirect_LocalWorkGroupSizeX = 64;
struct DrawIndexedIndirectIndexBufferPushData {
    BUFFER_ADDR_DECL(DrawIndexedIndirectCmdsBufferAddr) draw_indexed_indirect_cmds_addr;
    BUFFER_ADDR_DECL(CountBufferAddr) count_buffer_addr;
    BUFFER_ADDR_DECL(DipatchIndirectBufferAddr) dispatch_indirect_addr;
    uint flags;
    uint api_stride_dwords;
    uint bound_index_buffer_indices_count;  // Number of indices in the index buffer, taking index type in account. NOT a byte size.
    uint api_draw_count;
    uint api_offset_dwords;
    uint api_count_buffer_offset_dwords;
};

const uint kDrawMeshFlags_DrawCountFromBuffer = uint(1) << 0;
BUFFER_ADDR_FWD_DECL(DrawMeshTasksIndirectCmdsBufferAddr)
struct DrawMeshPushData {
    BUFFER_ADDR_DECL(DrawMeshTasksIndirectCmdsBufferAddr) draw_mesh_task_indirect_cmds_addr;
    BUFFER_ADDR_DECL(CountBufferAddr) count_buffer_addr;
    uint flags;
    uint api_stride_dwords;
    uint api_draw_count;
    uint max_workgroup_count_x;
    uint max_workgroup_count_y;
    uint max_workgroup_count_z;
    uint max_workgroup_total_count;
    uint api_offset_dwords;
    uint api_count_buffer_offset_dwords;
};

BUFFER_ADDR_FWD_DECL(DrawIndexedIndirectCmdsAddr)
const uint kFirstInstanceFlags_DrawCountFromBuffer = uint(1) << 0;
struct FirstInstancePushData {
    BUFFER_ADDR_DECL(DrawIndexedIndirectCmdsAddr) draw_indexed_indirect_cmds_addr;
    BUFFER_ADDR_DECL(CountBufferAddr) count_buffer_addr;
    uint flags;
    uint api_stride_dwords;
    uint api_draw_count;
    uint first_instance_member_pos;  // position in the struct where the firstInstance member is
    uint api_offset_dwords;
    uint api_count_buffer_offset_dwords;
};

struct CountBufferPushData {
    BUFFER_ADDR_DECL(CountBufferAddr) count_buffer_addr;
    uint api_stride;
    uint64_t api_offset;
    uint64_t draw_buffer_size;
    uint api_struct_size_byte;
    uint device_limit_max_draw_indirect_count;
    uint api_count_buffer_offset_dwords;
};

BUFFER_ADDR_FWD_DECL(DispatchIndirectBufferAddr)
struct DispatchPushData {
    BUFFER_ADDR_DECL(DispatchIndirectBufferAddr) dispatch_indirect_buffer_addr;
    uint limit_x;
    uint limit_y;
    uint limit_z;
    uint indirect_x_offset;
};

BUFFER_ADDR_FWD_DECL(SrcBufferAddr)
BUFFER_ADDR_FWD_DECL(CopySrcRegionsAddr)
struct CopyBufferToImagePushData {
    BUFFER_ADDR_DECL(SrcBufferAddr) src_buffer_addr;
    BUFFER_ADDR_DECL(CopySrcRegionsAddr) copy_src_regions_addr;
};

#ifdef __cplusplus
using IndirectCommandReference = uint64_t;
#else
struct VkTraceRaysIndirectCommandKHR {
    uint width;
    uint height;
    uint depth;
};

layout(buffer_reference, std430) buffer IndirectCommandReference { VkTraceRaysIndirectCommandKHR trace_rays_dimensions; };
#endif

struct TraceRaysPushData {
    // Need to put the buffer reference first otherwise it is incorrect, probably an alignment issue
    IndirectCommandReference indirect_data;
    uint trace_rays_width_limit;
    uint trace_rays_height_limit;
    uint trace_rays_depth_limit;
    uint max_ray_dispatch_invocation_count;
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
