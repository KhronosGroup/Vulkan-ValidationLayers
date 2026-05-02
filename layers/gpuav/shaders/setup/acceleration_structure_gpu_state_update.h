// Copyright (c) 2024-2026 The Khronos Group Inc.
// Copyright (c) 2024-2026 Valve Corporation
// Copyright (c) 2024-2026 LunarG, Inc.
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

#ifndef ACCELERATION_STRUCTURE_GPU_STATE_UPDATE_H
#define ACCELERATION_STRUCTURE_GPU_STATE_UPDATE_H

#ifdef __cplusplus

#include <cstdint>

namespace gpuav {
namespace glsl {
using uint = uint32_t;
#else

#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_scalar_block_layout : require
#endif

#ifdef __cplusplus
using GpuStatePtr = uint64_t;
#else

layout(buffer_reference, scalar) buffer GpuStatePtr { uint state; };
#endif

// Bits layout for AccelerationStructureGpuStateUpdateShaderPushData::state
// [0]   Valid state or not (1 if valid)
// [1]   build mode (VkBuildAccelerationStructureModeKHR)
// [2:3] AS type (VkAccelerationStructureTypeKHR)
const uint kAsGpuStateValidShift = 0;
const uint kAsGpuStateValidMask = 0x1;

const uint kBuildModeShift = 1;
const uint kBuildModeMask = 0x1;

const uint kAsTypeShift = 2;
const uint kAsTypeMask = 0x3 << kAsTypeShift;

struct AccelerationStructureGpuStateUpdateShaderPushData {
    GpuStatePtr gpu_state_ptr;
    uint state;
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif

#endif
