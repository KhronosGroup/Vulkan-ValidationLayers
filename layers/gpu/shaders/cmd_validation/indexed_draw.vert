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

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndirectBuffer) buffer DrawBuffer {
    uint draw_indexed_indirect_cmds[];
};

// CountBuffer won't be bound for non-count draws
layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_CountBuffer) buffer CountBuffer {
    uint count_buffer;
};

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndexBuffer) buffer IndexBuffer32 {
    uint index_buffer[];
};

// This function can cause OOB accesses per the descrptor size when dealing with 
// 16 or 8 bits indices (eg if dealing with 3 16 bits indices). 
// In practice it should be fine, as index buffer size
// is always aligned to 4 bytes (see Validator::PreCallRecordCreateBuffer in gpuav_record.cpp)
uint get_vertex_index(uint i) {
    if (push_data.index_width == 32) {
        return index_buffer[i];
    } else if (push_data.index_width == 16) {
		uint load_i = i / 2;
		uint packed_16_16 = index_buffer[load_i];
		// if (i % 2) == 0, take first 16 bits, else last 16 bits
		uint shift = (i % 2) * 16;
		return (packed_16_16 >> shift) & 0xFFFF;	
	} else if (push_data.index_width == 8) {
        uint load_i = i / 4;
		uint packed_8_8_8_8 = index_buffer[load_i];
		// if (i % 4) == 0, take first 8 bits, if == 1 take second set of 8 bits, etc...
		uint shift = (i % 4) * 8;
		return (packed_8_8_8_8 >> shift) & 0xFF;	
    }
    return 0;
}

bool check_index_buffer(uint index_count, uint first_index, uint vertex_offset) {
    if ((first_index + index_count) > push_data.index_buffer_max_count) {
        GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawIndexBuffer, first_index, index_count);
        return false;
    }

    for (uint index_i = 0; index_i < index_count; index_i++) {
        uint vert_index = get_vertex_index(first_index + index_i) + vertex_offset;
        if (vert_index >= push_data.smallest_vertex_attributes_count) {
            GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawVertexIndex, first_index + index_i, vert_index);
            return false;
        }
    }
    return true;
}

void main() {
    if (gl_VertexIndex == 0) {
        uint draw_count = 0;
        uint draw_index_count = 0;

        if ((push_data.flags & kPreDrawValidationFlags_CountBuffer) != 0) {
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
        if ((push_data.flags & kPreDrawValidationFlags_FirstInstance) != 0) {
            // Validate firstInstances
            uint draw_index = 0;
            for (uint i = 0; i < draw_count; i++) {
                if (draw_indexed_indirect_cmds[draw_index + kFirstInstance] != 0) {
                    GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawFirstInstance, i, i);
                    break;
                }
                draw_index += push_data.draw_stride;
            }

        }
        if ((push_data.flags & kPreDrawValidationFlags_IndexBuffer) != 0) {
            if ((push_data.flags & kPreDrawValidationFlags_DrawBuffer) != 0) {
                uint draw_index = 0;
                for (uint i = 0; i < draw_count; i++) {
                    uint index_count = draw_indexed_indirect_cmds[draw_index + kIndexCount];
                    uint first_index = draw_indexed_indirect_cmds[draw_index + kFirstIndex];
                    uint vertex_offset = draw_indexed_indirect_cmds[draw_index + kVertexOffset];

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
