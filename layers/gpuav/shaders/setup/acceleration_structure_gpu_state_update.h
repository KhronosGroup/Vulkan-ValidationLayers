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
#endif

#if defined(__cplusplus) || defined(__SLANG__)
namespace gpuav {
namespace shader {

typedef uint32_t* AsGpuStatePtr;
#endif

#if defined(__cplusplus)
using uint = uint32_t;
#elif !defined(__SLANG__)
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_scalar_block_layout : require

layout(buffer_reference, scalar) buffer AsGpuStatePtr { uint state; };
#endif

// Bits layout for AccelerationStructureGpuStateUpdateShaderPushData::state
// [0]   built or not (1 if built)
// [1]   build mode (VkBuildAccelerationStructureModeKHR)
// [2:3] AS type (VkAccelerationStructureTypeKHR)
const uint kAsGpuStateBuiltShift = 0;
const uint kAsGpuStateBuiltMask = 0x1;

const uint kBuildModeShift = 1;
const uint kBuildModeMask = 0x1;

const uint kAsTypeShift = 2;
const uint kAsTypeMask = 0x3 << kAsTypeShift;

struct AccelerationStructureGpuStateUpdateShaderPushData {
    AsGpuStatePtr dst_gpu_state_ptr;
    AsGpuStatePtr src_gpu_state_ptr;
    uint state;
};

#if defined(__cplusplus) || defined(__SLANG__)
}  // namespace shader
}  // namespace gpuav
#endif

#endif
