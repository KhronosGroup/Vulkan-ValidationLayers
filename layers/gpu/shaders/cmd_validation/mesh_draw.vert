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
    DrawMeshPushData push_data;
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

// array of VkDrawMeshTasksIndirectCommandEXT structs with caller defined stride
layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndirectBuffer) buffer DrawBuffer {
    uint draw_buffer[];
};

// CountBuffer won't be bound for non-count draws
layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_CountBuffer) buffer CountBuffer {
    uint count_buffer;
};

void main() {
    if (gl_VertexIndex == 0) {
        // if there's a count buffer error draw_count = 0 will skip validating the draws 
        uint draw_count = 0;
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

        // Validate mesh draw buffer
        uint draw_index = 0;
        for (uint i = 0; i < draw_count; i++){
            uint x = draw_buffer[draw_index + kGroupCountX];
            uint y = draw_buffer[draw_index + kGroupCountY];
            uint z = draw_buffer[draw_index + kGroupCountZ];

            if (x > push_data.max_workgroup_count_x) {
                GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountX, x, i);
            } else if (y > push_data.max_workgroup_count_y) {
                GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountY, y, i);
            } else if (z > push_data.max_workgroup_count_z) {
                GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountZ, z, i);
            } else {
                uint total = x * y * z;
                if (total > push_data.max_workgroup_total_count) {
                    GpuavLogError(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawGroupCountTotal, total, i);
                }
            }
            draw_index += push_data.draw_stride;
        }
    }
}
