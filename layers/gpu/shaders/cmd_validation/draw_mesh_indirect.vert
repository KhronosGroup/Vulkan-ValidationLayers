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

#version 450
#extension GL_GOOGLE_include_directive : enable

#include "common.h"
#include "draw_push_data.h"

layout (push_constant)
uniform UniformInfo {
    DrawMeshPushData pc;
};

/*
struct VkDrawMeshTasksIndirectCommandEXT {
    uint32_t    groupCountX;
    uint32_t    groupCountY;
    uint32_t    groupCountZ;
};
*/

const uint kGroupCountX = 0;
const uint kGroupCountY = 1;
const uint kGroupCountZ = 2;

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndirectBuffer)
readonly buffer DrawBuffer {
    uint draw_buffer[];
};

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_CountBuffer) 
readonly buffer CountBuffer {
    uint count_buffer;
};

void main() {
    gl_PointSize = 1.0;

    if ((pc.flags & kDrawMeshFlags_DrawCountFromBuffer) != 0) {
        if (gl_VertexIndex >= count_buffer) return;
    }

    uint x = draw_buffer[gl_VertexIndex * pc.draw_cmds_stride_dwords + kGroupCountX];
    uint y = draw_buffer[gl_VertexIndex * pc.draw_cmds_stride_dwords + kGroupCountY];
    uint z = draw_buffer[gl_VertexIndex * pc.draw_cmds_stride_dwords + kGroupCountZ];
    if (x > pc.max_workgroup_count_x) {
        GpuavLogError2(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountX, x, gl_VertexIndex);
    }
    if (y > pc.max_workgroup_count_y) {
        GpuavLogError2(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountY, y, gl_VertexIndex);
    } 
    if (z > pc.max_workgroup_count_z) {
        GpuavLogError2(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountZ, z, gl_VertexIndex);
    }

    uint total = x * y * z;
    if (total > pc.max_workgroup_total_count) {
        GpuavLogError2(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountTotal, total, gl_VertexIndex);
    }
}
