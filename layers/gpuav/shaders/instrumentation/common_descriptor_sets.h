// Copyright (c) 2024-2025 The Khronos Group Inc.
// Copyright (c) 2024-2025 Valve Corporation
// Copyright (c) 2024-2025 LunarG, Inc.
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

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#else
#error No extension available for 64-bit integers.
#endif

#include "gpuav_error_header.h"
#include "gpuav_shaders_constants.h"

layout(set = kInstDefaultDescriptorSet, binding = kBindingInstErrorBuffer, std430) buffer OutputBuffer {
    uint flags;
    uint written_count;
    uint data[];
}
inst_errors_buffer;

layout(set = kInstDefaultDescriptorSet, binding = kBindingInstActionIndex, std430) buffer ActionIndexBuffer { uint index[]; }
inst_action_index_buffer;

layout(set = kInstDefaultDescriptorSet, binding = kBindingInstCmdResourceIndex, std430) buffer CmdResourceIndexBuffer {
    uint index[];
}
inst_cmd_resource_index_buffer;

layout(set = kInstDefaultDescriptorSet, binding = kBindingInstCmdErrorsCount, std430) buffer CmdErrorsCountBuffer {
    uint errors_count[];
}
inst_cmd_errors_count_buffer;
