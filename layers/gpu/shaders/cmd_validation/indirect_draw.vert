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
    DrawIndirectPushData push_data;
};

/*
struct VkDrawIndirectCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};
*/
const uint kFirstInstance = 3;

// array of VkDrawIndirectCommand structs with caller defined stride
layout(set = kDiagPerCmdDescriptorSet, binding = 0) buffer DrawBuffer {
    uint draw_buffer[];
};

// CountBuffer won't be bound for non-count draws
layout(set = kDiagPerCmdDescriptorSet, binding = 1) buffer CountBuffer {
    uint count_buffer;
};

// binding 2 is for the index buffer which isn't used

void main() {
    if (gl_VertexIndex == 0) {
        uint draw_count = 0;

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
    }
}
