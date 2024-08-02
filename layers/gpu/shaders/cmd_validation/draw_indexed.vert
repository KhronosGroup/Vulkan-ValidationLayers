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
    IndexedDrawPushData pc;
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

void ValidateVertexIndex(uint index_buffer_offset, uint vertex_offset, uint smallest_vertex_attributes_count) {
    const uint vertex_index = get_vertex_index(index_buffer_offset) + vertex_offset;

    if (vertex_index >= smallest_vertex_attributes_count) {
        GpuavLogError4(kErrorGroupGpuPreDraw, kErrorSubCode_OobVertexBuffer, index_buffer_offset, vertex_offset, vertex_index, 0);
    }
}

// Supposed to be called on a plain vkCmdDraw (not indexed, not indirect)
// Goal is to do indexed call job manually, so that we can both retrieve the value of the faulty vertex index,
// and its offset in the index buffer
void main() {
    gl_PointSize = 1.0;

    const uint index_buffer_offset = gl_VertexIndex;
    ValidateVertexIndex(index_buffer_offset, pc.vertex_offset, pc.smallest_vertex_attributes_count);
}
