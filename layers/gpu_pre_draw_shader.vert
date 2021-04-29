// Copyright (c) 2021 The Khronos Group Inc.
// Copyright (c) 2021 Valve Corporation
// Copyright (c) 2021 LunarG, Inc.
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
//
// * Author: Tony Barbour <tony@lunarg.com>
//

#version 450
#define VAL_OUT_RECORD_SZ 10
#extension GL_GOOGLE_include_directive : enable
#include "gpu_pre_draw_constants.h"

layout(set = 0, binding = 0) buffer OutputBuffer { 
    uint count;
    uint data[];
} Output;
layout(set = 0, binding = 1) buffer CountBuffer { uint data[]; } Count;
layout(push_constant) uniform ufoo {
    uint countLimit;
    uint maxWrites;
    uint countOffset;
} u_info;
void valErrorOut(uint error, uint count) {
    uint vo_idx = atomicAdd(Output.count, VAL_OUT_RECORD_SZ);
    if (vo_idx + VAL_OUT_RECORD_SZ > Output.data.length())
        return;
    Output.data[vo_idx + _kInstValidationOutError] = _kInstErrorPreDrawValidate;
    Output.data[vo_idx + _kInstValidationOutError + 1] = error;
    Output.data[vo_idx + _kInstValidationOutError + 2] = count;
}
void main() {
    if (gl_VertexIndex == 0) {
        if (u_info.countLimit > 0) {
            uint countIn = Count.data[u_info.countOffset];
            if (countIn > u_info.maxWrites) {
                valErrorOut(pre_draw_count_exceeds_bufsize_error, countIn);
            }
            else if (countIn > u_info.countLimit) {
                valErrorOut(pre_draw_count_exceeds_limit_error, countIn);
            }
        }
    }
}
