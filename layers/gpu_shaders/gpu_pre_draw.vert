// Copyright (c) 2021-2022 The Khronos Group Inc.
// Copyright (c) 2021-2023 Valve Corporation
// Copyright (c) 2021-2023 LunarG, Inc.
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
#define VAL_OUT_RECORD_SZ 10
#extension GL_GOOGLE_include_directive : enable
#include "gpu_shaders_constants.h"

// used when testing count buffer
#define count_limit push_constant_word_0
#define max_writes push_constant_word_1
#define count_offset push_constant_word_2

// used when testing draw buffer
#define draw_count push_constant_word_1
#define first_instance_offset push_constant_word_2
#define draw_stride push_constant_word_3

layout(set = 0, binding = 0) buffer OutputBuffer {
    uint flags;
    uint output_buffer_count;
    uint output_buffer[];
};

// Input will be only testing one buffer depending on the command
// Alias to same binding with different names
layout(set = 0, binding = 1) buffer CountBuffer { uint count_buffer[]; };
layout(set = 0, binding = 1) buffer DrawBuffer { uint draws_buffer[]; };

layout(push_constant) uniform UniformInfo {
    uint push_constant_word_0;
    uint push_constant_word_1;
    uint push_constant_word_2;
    uint push_constant_word_3;
} u_info;

void valErrorOut(uint error, uint count) {
    uint vo_idx = atomicAdd(output_buffer_count, VAL_OUT_RECORD_SZ);
    if (vo_idx + VAL_OUT_RECORD_SZ > output_buffer.length())
        return;
    output_buffer[vo_idx + _kInstValidationOutError] = _kInstErrorPreDrawValidate;
    output_buffer[vo_idx + _kInstValidationOutError + 1] = error;
    output_buffer[vo_idx + _kInstValidationOutError + 2] = count;
}

void main() {
    if (gl_VertexIndex == 0) {
        if (u_info.count_limit > 0) {
            // Validate count buffer
            uint count_in = count_buffer[u_info.count_offset];
            if (count_in > u_info.max_writes) {
                valErrorOut(pre_draw_count_exceeds_bufsize_error, count_in);
            }
            else if (count_in > u_info.count_limit) {
                valErrorOut(pre_draw_count_exceeds_limit_error, count_in);
            }
        } else {
            // Validate firstInstances
            uint fi_index = u_info.first_instance_offset;
            for (uint i = 0; i < u_info.draw_count; i++) {
                if (draws_buffer[fi_index] != 0) {
                    valErrorOut(pre_draw_first_instance_error, i);
                    break;
				}
                fi_index += u_info.draw_stride;
            }
        }
    }
}
