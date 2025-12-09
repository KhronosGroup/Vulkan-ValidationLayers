// Copyright (c) 2021-2025 The Khronos Group Inc.
// Copyright (c) 2021-2025 Valve Corporation
// Copyright (c) 2021-2025 LunarG, Inc.
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
// Values used between the GLSL shaders and the GPU-AV logic

// NOTE: This header is included by the instrumentation shaders and glslang doesn't support #pragma once
#ifndef BUILD_ACCELERATION_STRUCTURES_H
#define BUILD_ACCELERATION_STRUCTURES_H

#ifdef __cplusplus

#include <cstdint>

namespace gpuav {
namespace glsl {
using uint = uint32_t;
#else
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#else
#error No extension available for 64-bit integers.
#endif
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_scalar_block_layout : require
#endif

#ifdef __cplusplus
using PtrToBlasArray = uint64_t;
using PtrToBlasPtrArray = uint64_t;
using PtrToAccelerationStructureAddressArray = uint64_t;
using PtrToAccelerationStructureMetadataArray = uint64_t;
using PtrToAccelerationStructureBufferRangeArray = uint64_t;
using PtrtoPtrToAccelerationStructureArrays = uint64_t;
using PtrToBlasBuiltInCmd = uint64_t;

#else
struct VkAccelerationStructureInstance {
    mat3x4 transform;
    uint instanceCustomIndex_and_mask;
    uint instanceShaderBindingTableRecordOffset_and_flags;
    uint64_t accelerationStructureReference;
};

layout(buffer_reference, scalar) buffer VkAccelerationStructureInstancePtr { VkAccelerationStructureInstance as_instance; };

layout(buffer_reference, scalar) buffer PtrToAccelerationStructureAddressArray {
    uint count;
    uint pad_;
    uint64_t array[];
};

layout(buffer_reference, scalar) buffer PtrToAccelerationStructureMetadataArray { uint array[]; };

// Represent a [begin, end) range, where end is one past the last element held in range
struct Range {
    uint64_t begin;
    uint64_t end;
};

layout(buffer_reference, scalar) buffer PtrToAccelerationStructureBufferRangeArray { Range array[]; };
#endif

// "Struct of arrays" memory pattern to improve locality
struct AccelerationStructureArraysPtr {
    PtrToAccelerationStructureAddressArray addresses_ptr;
    PtrToAccelerationStructureMetadataArray metadata_ptr;
    PtrToAccelerationStructureBufferRangeArray buffer_ranges_ptr;
};

#ifndef __cplusplus
layout(buffer_reference, scalar) buffer PtrtoPtrToAccelerationStructureArrays { AccelerationStructureArraysPtr as_arrays_ptrs; };

layout(buffer_reference, scalar) buffer PtrToBlasArray { VkAccelerationStructureInstance blas_array[]; };

layout(buffer_reference, scalar) buffer PtrToBlasPtrArray { VkAccelerationStructureInstancePtr blas_ptr_array[]; };

layout(buffer_reference, scalar) buffer PtrToBlasBuiltInCmd { Range buffer_ranges[]; };
#endif

const uint kBuildASValidationMode_invalid_AS = 0;
const uint kBuildASValidationMode_memory_overlaps = 1;

// Case where arrayOfPointers is false
struct AccelerationStructureReferencePushData {
    PtrtoPtrToAccelerationStructureArrays ptr_to_ptr_to_accel_structs_arrays;
    uint64_t valid_dummy_blas_addr;

    // BLAS arrays to validate
    // Either an array of VkAccelerationStructureInstance,
    // or pointers to such structs.
    PtrToBlasArray blas_array_start_addr;
    PtrToBlasPtrArray blas_ptr_array_start_addr;
    uint blas_array_size;
    uint is_array_of_pointers;
    uint blas_array_i;  // Used to find back invalid build cmd pInfos if an error is found
    uint validation_mode;

    PtrToBlasBuiltInCmd blas_built_in_cmd_array_ptr;
    uint blas_built_in_cmd_array_size;
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
