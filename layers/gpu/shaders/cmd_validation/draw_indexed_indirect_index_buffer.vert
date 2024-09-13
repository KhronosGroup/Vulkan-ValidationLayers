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

layout(push_constant) uniform UniformInfo {
    DrawIndexedIndirectIndexBufferPushData pc;
};

/*
struct VkDrawIndexedIndirectCommand {
uint indexCount;
uint instanceCount;
uint firstIndex;
uint vertexOffset;
uint firstInstance;
};
*/

const uint kIndexCount = 0;
const uint kInstancekCount = 1;
const uint kFirstIndex = 2;
const uint kVertexOffset = 3;
const uint kFirstInstance = 4;

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndirectBuffer) 
readonly buffer DrawBuffer {
    uint draw_indexed_indirect_cmds[];
};

// CountBuffer won't be bound for non-count draws
layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_CountBuffer) 
readonly buffer CountBuffer {
    uint count_from_buffer;
};

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndexBuffer)
readonly buffer IndexBuffer32 {
    uint index_buffer[];
};

// Supposed to be called on a plain vkCmdDraw (not indexed, not indirect)
// vertexCount is the number of indices found in the bound index buffer.
// Goal is to do indexed call job manually, so that we can both retrieve the value of the faulty vertex index,
// and its offset in the index buffer
void main() {
    gl_PointSize = 1.0;

    uint draw_count = 0;
    if ((pc.flags & kIndexedIndirectDrawFlags_DrawCountFromBuffer) == 0) {
        draw_count = pc.cpu_draw_count;
    } else {
        draw_count = count_from_buffer;
    }

    for (uint draw_i = 0; draw_i < draw_count; ++draw_i) {
        const uint draw_i_index_count = draw_indexed_indirect_cmds[draw_i * pc.draw_cmds_stride_dwords + kIndexCount];
        const uint draw_i_first_index = draw_indexed_indirect_cmds[draw_i * pc.draw_cmds_stride_dwords + kFirstIndex];
    
        if (draw_i_first_index + draw_i_index_count > pc.bound_index_buffer_indices_count) {
            GpuavLogError4(kErrorGroupGpuPreDraw, kErrorSubCode_OobIndexBuffer, draw_i, draw_i_first_index, draw_i_index_count, 0);
        }
    }
}
