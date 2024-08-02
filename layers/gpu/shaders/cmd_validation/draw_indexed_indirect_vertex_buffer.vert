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
    DrawIndexedIndirectVertexBufferPushData pc;
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

uint get_vertex_index(uint i) {
    if (pc.index_width == 32) {
        return index_buffer[i];
    } else if (pc.index_width == 16) {
        uint load_i = i / 2;
        uint packed_16_16 = index_buffer[load_i];
        // if (i % 2) == 0, take first 16 bits, else last 16 bits
        uint shift = (i % 2) * 16;
        return (packed_16_16 >> shift) & 0xFFFF;	
    } else if (pc.index_width == 8) {
        uint load_i = i / 4;
        uint packed_8_8_8_8 = index_buffer[load_i];
        // if (i % 4) == 0, take first 8 bits, if == 1 take second set of 8 bits, etc...
        uint shift = (i % 4) * 8;
        return (packed_8_8_8_8 >> shift) & 0xFF;	
    }
    return 0;
}

// Validate that a vertex index is within the bounds of the smallest vertex buffer
void ValidateVertexIndex(uint draw_i, uint index_buffer_offset, int vertex_offset, uint smallest_vertex_attributes_count) {
    const uint vertex_index = get_vertex_index(index_buffer_offset) + vertex_offset;

    if (vertex_index >= smallest_vertex_attributes_count) {
        GpuavLogError4(kErrorGroupGpuPreDraw, kErrorSubCode_OobVertexBuffer, draw_i, index_buffer_offset, vertex_offset, vertex_index);
    }
}

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

    const uint index_buffer_offset = gl_VertexIndex;

    for (uint draw_i = 0; draw_i < draw_count; ++draw_i) {
        const uint draw_i_index_count = draw_indexed_indirect_cmds[draw_i * pc.draw_cmds_stride_dwords + kIndexCount];
        const uint draw_i_first_index = draw_indexed_indirect_cmds[draw_i * pc.draw_cmds_stride_dwords + kFirstIndex];

        // Is index within index buffer bounds?
        if (index_buffer_offset < pc.bound_index_buffer_indices_count) {
            // Is index within the range of indices to check?
            if (index_buffer_offset >= draw_i_first_index && index_buffer_offset < (draw_i_first_index + draw_i_index_count)) {
                const int vertex_offset = int(draw_indexed_indirect_cmds[draw_i * pc.draw_cmds_stride_dwords + kVertexOffset]);
                ValidateVertexIndex(draw_i, index_buffer_offset, vertex_offset, pc.smallest_vertex_attributes_count);
            }
        }
    }
}
