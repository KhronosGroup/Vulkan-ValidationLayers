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

layout(push_constant)
uniform PushConstants {
	FirstInstancePushData pc;
};


layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_IndirectBuffer)
readonly buffer DrawBuffer {
    uint draw_indexed_indirect_cmds[];
};

layout(set = kDiagPerCmdDescriptorSet, binding = kPreDrawBinding_CountBuffer) 
readonly buffer CountBuffer {
    uint count_buffer;
};

// Validate firstInstance member from indirect draw commands
void main() {
    gl_PointSize = 1.0;

    if ((pc.flags & kFirstInstanceFlags_DrawCountFromBuffer) != 0) {
        if (gl_VertexIndex >= count_buffer) return;
    }

    const uint first_instance = draw_indexed_indirect_cmds[gl_VertexIndex * pc.draw_cmds_stride_dwords + pc.first_instance_member_pos];
    if (first_instance != 0) {
           GpuavLogError2(kErrorGroupGpuPreDraw, kErrorSubCodePreDrawFirstInstance, gl_VertexIndex, first_instance);
    }
}
