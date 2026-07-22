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

#define BUILD_AS_METADATA_VALID_BUFFER 1u
#define GET_BUILD_AS_METADATA_BUFFER_STATUS(metadata) ((metadata & (0x1 << 0u)) >> 0u)
#define SET_BUILD_AS_METADATA_BUFFER_STATUS(is_buffer_destroyed) ((uint32_t(is_buffer_destroyed) & 0x1) << 0u)

#define BUILD_AS_METADATA_AS_TYPE_BLAS 1u
#define GET_BUILD_AS_METADATA_AS_TYPE(metadata) ((metadata & (0x3 << 1u)) >> 1u)
#define SET_BUILD_AS_METADATA_AS_TYPE(as_type) ((uint32_t(as_type) & 0x3) << 1u)

#define BUILD_AS_METADATA_VALID_BUFFER_MEMORY 1u
#define GET_BUILD_AS_METADATA_BUFFER_MEMORY_STATUS(metadata) ((metadata & (0x1 << 3u)) >> 3u)
#define SET_BUILD_AS_METADATA_BUFFER_MEMORY_STATUS(is_memory_destroyed) ((uint32_t(is_memory_destroyed) & 0x1) << 3u)

#ifdef __cplusplus
#include <cstdint>
#endif

namespace gpuav {
namespace shader {

struct VkTransformMatrix {
    float matrix[3][4];
};

struct VkAccelerationStructureInstance {
    VkTransformMatrix transform;
    uint32_t instanceCustomIndex_and_mask;
    uint32_t instanceShaderBindingTableRecordOffset_and_flags;
    uint64_t accelerationStructureReference;
};

// Represent a [begin, end) range, where end is one past the last element held in range
struct Range {
    uint64_t begin;
    uint64_t end;
};

struct AccelerationStructureAddressArray {
    uint32_t count;
    uint32_t pad_;
#ifndef __cplusplus
    uint64_t array[];
#endif
};

// "Struct of arrays" memory pattern to improve locality
struct AccelerationStructureArraysPtr {
    AccelerationStructureAddressArray* addresses_ptr;
    uint32_t* metadata_ptr;
    Range* buffer_ranges_ptr;
};

static const uint32_t kBuildASValidationMode_invalid_AS = 0;
static const uint32_t kBuildASValidationMode_memory_overlaps = 1;

static const uint32_t tlas_validation_shader_wg_x = 8;
static const uint32_t tlas_validation_shader_wg_y = 8;
struct TLASValidationShaderPushData {
    AccelerationStructureArraysPtr* ptr_to_ptr_to_accel_structs_arrays;
    uint64_t valid_dummy_blas_addr;

    // BLAS arrays to validate
    // Either an array of VkAccelerationStructureInstance,
    // or pointers to such structs.
    VkAccelerationStructureInstance* blas_array_start_addr;
    VkAccelerationStructureInstance** blas_ptr_array_start_addr;
    uint32_t blas_array_size;
    uint32_t is_array_of_pointers;
    uint32_t blas_array_i;  // Used to find back invalid build cmd pInfos if an error is found
    uint32_t validation_mode;

    Range* blas_built_in_cmd_array_ptr;
    uint32_t blas_built_in_cmd_array_size;
};

static const uint32_t kBLASValidationMode_triangles_indices = 0;
static const uint32_t kBLASValidationMode_active_triangles = 1;
static const uint32_t kBLASValidationMode_aabbs = 2;
static const uint32_t kBLASValidationMode_transform_matrix = 3;

// GPU representation of a single VkAccelerationStructureGeometryKHR
struct AccelerationStructureGeometryGPU {
    uint64_t stride;
    uint64_t index_buffer;
    uint64_t index_buffer_copies;
    // WARNING: Read start at offset primitive_offset
    uint64_t geometry_buffer;
    float* geometry_x_components_copies;
    // WARNING: Read start at offset transform_offset
    uint64_t transform;
    uint32_t is_update_build;
    uint32_t geometry_type;
    uint32_t index_type;
    uint32_t vertex_format;
    uint32_t max_vertex;
    uint32_t is_array_of_pointers;

    uint32_t primitive_offset;
    uint32_t primitive_count;
    uint32_t first_vertex;
    uint32_t transform_offset;

    uint32_t geometry_i;
    uint32_t error_info_i;  // put that somewhere else?
};

struct AccelerationStructureGeometriesGPU {
    uint32_t count;
    uint32_t pad_;
#ifndef __cplusplus
    AccelerationStructureGeometryGPU array[];
#endif
};

static const uint32_t blas_validation_shader_wg_x = 64;
struct BLASValidationShaderPushData {
    // #ARNO_TODO So I think now I can just turn this into a pointer to dst_as_gpu_state
    AccelerationStructureGeometriesGPU** last_build_as_geometries_gpu;
    AccelerationStructureGeometryGPU* as_geometry_gpu;
    uint32_t validation_mode;
};

}  // namespace shader
}  // namespace gpuav
