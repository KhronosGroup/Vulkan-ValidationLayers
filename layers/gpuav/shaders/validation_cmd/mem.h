// Copyright (c) 2021-2026 The Khronos Group Inc.
// Copyright (c) 2021-2026 Valve Corporation
// Copyright (c) 2021-2026 LunarG, Inc.
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
// Values used between the Slang shaders and the GPU-AV logic

#pragma once

#include "build_acceleration_structures.h"

namespace gpuav {
namespace shader {

#define MEM_SHADER_WG_SIZE_X 64
struct MemcmpShaderPushData {
    AccelerationStructureGeometriesGPU** last_build_as_geometries_gpu;
    uint64_t update_time_indices;
    uint32_t uvec4_count;
    uint32_t u32_count;
    uint32_t u16_count;
    uint32_t geometry_i;
    uint32_t error_info_i;
};

struct MemcpyShaderPushData {
    uint64_t index_buffer_copy;
    uint64_t update_time_indices;
    uint32_t uvec4_count;
    uint32_t u32_count;
    uint32_t u16_count;
};

}  // namespace shader
}  // namespace gpuav
