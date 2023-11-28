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
#extension GL_GOOGLE_include_directive : enable
#include "gpu_pre_action.h"

#define validation_select push_constant_word_0

// used when testing count buffer
#define count_limit push_constant_word_1
#define max_writes push_constant_word_2
#define count_offset push_constant_word_3

// used when testing draw buffer
#define draw_count push_constant_word_1
#define first_instance_offset push_constant_word_2
#define draw_stride push_constant_word_3

// used when validating mesh draw buffer
// words 0-3 could be used to validate count
#define mesh_draw_buffer_offset push_constant_word_4
#define mesh_draw_buffer_num_draws push_constant_word_5
#define mesh_draw_buffer_stride push_constant_word_6
#define max_workgroup_count_x push_constant_word_7
#define max_workgroup_count_y push_constant_word_8
#define max_workgroup_count_z push_constant_word_9
#define max_workgroup_total_count push_constant_word_10

// CountBuffer won't be bound for non-count draws
layout(set = 0, binding = 1) buffer CountBuffer { uint count_buffer[]; };
layout(set = 0, binding = 2) buffer DrawBuffer { uint draws_buffer[]; };

layout(push_constant) uniform UniformInfo {
    uint push_constant_word_0;
    uint push_constant_word_1;
    uint push_constant_word_2;
    uint push_constant_word_3;
    uint push_constant_word_4;
    uint push_constant_word_5;
    uint push_constant_word_6;
    uint push_constant_word_7;
    uint push_constant_word_8;
    uint push_constant_word_9;
    uint push_constant_word_10;
} u_info;


void main() {
    if (gl_VertexIndex == 0) {
        if (u_info.validation_select == pre_draw_select_count_buffer ||
            u_info.validation_select == pre_draw_select_mesh_count_buffer) {
            // Validate count buffer
            uint count_in = count_buffer[u_info.count_offset];
            if (count_in > u_info.max_writes) {
                gpuavLogError(kInstErrorPreDrawValidate, pre_draw_count_exceeds_bufsize_error, count_in, 0);
            }
            else if (count_in > u_info.count_limit) {
                gpuavLogError(kInstErrorPreDrawValidate, pre_draw_count_exceeds_limit_error, count_in, 0);
            }
        } else if (u_info.validation_select == pre_draw_select_draw_buffer) {
            // Validate firstInstances
            uint fi_index = u_info.first_instance_offset;
            for (uint i = 0; i < u_info.draw_count; i++) {
                if (draws_buffer[fi_index] != 0) {
                    gpuavLogError(kInstErrorPreDrawValidate, pre_draw_first_instance_error, i, i);
                    break;
				}
                fi_index += u_info.draw_stride;
            }
        }

        if (u_info.validation_select == pre_draw_select_mesh_count_buffer ||
            u_info.validation_select == pre_draw_select_mesh_no_count) {
            // Validate mesh draw buffer
            uint draw_buffer_index = u_info.mesh_draw_buffer_offset;
            uint stride = u_info.mesh_draw_buffer_stride;
            uint draw_count;
            if (u_info.validation_select == pre_draw_select_mesh_count_buffer)
                draw_count = count_buffer[u_info.count_offset];
            else
                draw_count = u_info.mesh_draw_buffer_num_draws;
            for (uint i = 0; i < draw_count; i++){
                uint count_x_in = draws_buffer[draw_buffer_index];
                uint count_y_in = draws_buffer[draw_buffer_index + 1];
                uint count_z_in = draws_buffer[draw_buffer_index + 2];
                if (count_x_in > u_info.max_workgroup_count_x) {
                    gpuavLogError(kInstErrorPreDrawValidate, pre_draw_group_count_exceeds_limit_x_error, count_x_in, i);
                }
                if (count_y_in > u_info.max_workgroup_count_y) {
                    gpuavLogError(kInstErrorPreDrawValidate, pre_draw_group_count_exceeds_limit_y_error, count_y_in, i);
                }
                if (count_z_in > u_info.max_workgroup_count_z) {
                    gpuavLogError(kInstErrorPreDrawValidate, pre_draw_group_count_exceeds_limit_z_error, count_z_in, i);
                }
                uint total = count_x_in * count_y_in * count_z_in;
                if (total > u_info.max_workgroup_total_count) {
                    gpuavLogError(kInstErrorPreDrawValidate, pre_draw_group_count_exceeds_total_error, total, i);
                }
                draw_buffer_index += stride;
            }
        }
    }
}
