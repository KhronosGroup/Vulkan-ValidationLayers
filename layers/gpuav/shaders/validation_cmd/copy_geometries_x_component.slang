// Copyright (c) 2022-2026 The Khronos Group Inc.
// Copyright (c) 2022-2026 Valve Corporation
// Copyright (c) 2022-2026 LunarG, Inc.
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

#include "common.h"
#include "copy_geometries_x_component.h"

using namespace gpuav::shader;

[vk::push_constant]
ConstantBuffer<CopyGeometriesXComponentShaderPushData> pc;

// Vertex format assumed to be this
static const uint VK_FORMAT_R32G32B32_SFLOAT = 106;

// Reference:
// https://docs.vulkan.org/refpages/latest/refpages/source/VkAccelerationStructureBuildRangeInfoKHR.html#_description
float LoadVertexX(uint64_t vertices, uint64_t stride, uint vertex_i) {
    float vertex_x = 0;
    float* array_f32 = (float*)(vertices + vertex_i * stride);
    vertex_x = array_f32[0];
    return vertex_x;
}

[shader("compute")]
[numthreads(COPY_GEOMETRIES_X_COMPONENT_SHADER_WG_SIZE_X, 1, 1)]
void main(uint3 thread_id: SV_DispatchThreadID) {
    const uint vertex_i = thread_id.x;

    if (vertex_i >= pc.count) {
        return;
    }

    const float vertex_x = LoadVertexX(
        pc.build_cmd_vertex,
        pc.stride, vertex_i);

    pc.geometries_x_component_copy[vertex_i] = vertex_x;
}
