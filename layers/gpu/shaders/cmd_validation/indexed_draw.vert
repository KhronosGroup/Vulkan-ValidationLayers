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
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_shader_8bit_storage : enable

#include "common.h"
#include "draw_push_data.h"
layout(push_constant) uniform UniformInfo {
    DrawIndexedPushData push_data;
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

layout(set = kDiagPerCmdDescriptorSet, binding = 0) buffer DrawBuffer {
    uint draw_buffer[];
};

// CountBuffer won't be bound for non-count draws
layout(set = kDiagPerCmdDescriptorSet, binding = 1) buffer CountBuffer {
    uint count_buffer;
};

layout(set = kDiagPerCmdDescriptorSet, binding = 2) buffer IndexBuffer32 {
    uint index_buffer_32[];
};

layout(set = kDiagPerCmdDescriptorSet, binding = 2) buffer IndexBuffer16 {
    uint16_t index_buffer_16[];
};

layout(set = kDiagPerCmdDescriptorSet, binding = 2) buffer IndexBuffer8 {
    uint8_t index_buffer_8[];
};

uint get_vertex_index(uint i) {
    uint value = 0;
    if (push_data.index_width == 32) {
        value = index_buffer_32[i];
    } else if (push_data.index_width == 16) {
        value = uint(index_buffer_16[i]);
    } else if (push_data.index_width == 8) {
        value = uint(index_buffer_8[i]);
    }
    return value;
}

bool check_index_buffer(uint ic, uint fi, uint vo) {
    uint index_len = 0;
    if (push_data.index_width == 32) {
        index_len = index_buffer_32.length();
    } else if (push_data.index_width == 16) {
        index_len = index_buffer_16.length();
    } else if (push_data.index_width == 8) {
        index_len = index_buffer_8.length();
    } else {
        // host should have caught this
        return false;
    }

    if ((fi + ic) > index_len) {
        GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawIndexBuffer, fi, ic);
        return false;
    }

    for (uint j = 0; j < ic; j++) {
        uint vert_index = get_vertex_index(fi + j) + vo;
        if (vert_index >= push_data.vertex_buffer_size) {
            GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawVertexIndex, fi + j, vert_index);
            return false;
        }
    }
    return true;
}

void main() {
    if (gl_VertexIndex == 0) {
        uint draw_count = 0;
        uint draw_index_count = 0;

        if ((push_data.flags & kPreDrawSelectCountBuffer) != 0) {
            // Validate count buffer
            uint count_in = count_buffer;
            if (count_in > push_data.buffer_count_limit) {
                GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawBufferSize, count_in, 0);
            } else if (count_in > push_data.prop_count_limit) {
                GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawCountLimit, count_in, 0);
            } else {
                draw_count = count_in;
            }
        } else {
            draw_count = push_data.draw_count;
        }
        if ((push_data.flags & kPreDrawSelectFirstInstance) != 0) {
            // Validate firstInstances
            uint draw_index = 0;
            for (uint i = 0; i < draw_count; i++) {
                if (draw_buffer[draw_index + kFirstInstance] != 0) {
                    GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawFirstInstance, i, i);
                    break;
                }
                draw_index += push_data.draw_stride;
            }

        }
        if ((push_data.flags & kPreDrawSelectIndexBuffer) != 0) {
            if ((push_data.flags & kPreDrawSelectDrawBuffer) != 0) {
                uint draw_index = 0;
                for (uint i = 0; i < draw_count; i++) {
                    uint index_count = draw_buffer[draw_index + kIndexCount];
                    uint first_index = draw_buffer[draw_index + kFirstIndex];
                    uint vertex_offset = draw_buffer[draw_index + kVertexOffset];

                    if (!check_index_buffer(index_count, first_index, vertex_offset)) {
                        break;
                    }

                    draw_index += push_data.draw_stride;
                }
            } else {
                check_index_buffer(push_data.index_count, push_data.first_index, push_data.vertex_offset);
            }
        }
    }
}
