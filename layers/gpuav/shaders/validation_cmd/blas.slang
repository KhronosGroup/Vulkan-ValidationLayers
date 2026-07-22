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

// common.h is still written in GLSL, it is compiled thanks to the slangc -allow-glsl option
#include "common.h"
#include "build_acceleration_structures.h"

using namespace gpuav::shader;

[vk::push_constant]
ConstantBuffer<BLASValidationShaderPushData> pc;

// From VkIndexType
static const uint VK_INDEX_TYPE_UINT16 = 0;
static const uint VK_INDEX_TYPE_UINT32 = 1;
static const uint VK_INDEX_TYPE_UINT8 = 1000265000;
static const uint VK_INDEX_TYPE_NONE_KHR = 1000165000;

// From VkFormat
static const uint VK_FORMAT_R32G32B32_SFLOAT = 106;

uint LoadIndex(uint64_t index_buffer, uint index_type, uint primitive_offset, uint i) {
    if (index_type == VK_INDEX_TYPE_NONE_KHR) {
        return i;
    } else if (index_type == VK_INDEX_TYPE_UINT16) {
        uint16_t* array_u16 = (uint16_t*)(index_buffer + primitive_offset);
        return uint(array_u16[i]);
    } else if (index_type == VK_INDEX_TYPE_UINT32) {
        uint* array_u32 = (uint*)(index_buffer + primitive_offset);
        return array_u32[i];
    } else if (index_type == VK_INDEX_TYPE_UINT8) {
        uint8_t* array_u8 = (uint8_t*)(index_buffer + primitive_offset);
        return uint(array_u8[i]);
    } else {
        return 0;
    }
}

void StoreIndex(uint8_t* index_buffer, uint i, uint index_type, uint primitive_offset, uint value) {
    if (index_type == VK_INDEX_TYPE_UINT16) {
        uint16_t* array_u16 = (uint16_t*)(index_buffer + primitive_offset);
        array_u16[i] = uint16_t(value);
    } else if (index_type == VK_INDEX_TYPE_UINT32) {
        uint* array_u32 = (uint*)(index_buffer + primitive_offset);
        array_u32[i] = value;
    } else if (index_type == VK_INDEX_TYPE_UINT8) {
        uint8_t* array_u8 = index_buffer + primitive_offset;
        array_u8[i] = uint8_t(value);
    }
}

// Reference:
// https://docs.vulkan.org/refpages/latest/refpages/source/VkAccelerationStructureBuildRangeInfoKHR.html#_description
float LoadVertexX(uint64_t vertices, uint64_t stride, uint index, uint format) {
    float vertex_x = 0;

    if (format == VK_FORMAT_R32G32B32_SFLOAT) {
        float* array_f32 = (float*)(vertices + index * stride);
        vertex_x = array_f32[0];
    }

    return vertex_x;
}

struct VkAabbPositionsKHR {
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
};

// CPU will try to dispatch `primitive_count` threads
[shader("compute")]
[numthreads(blas_validation_shader_wg_x, 1, 1)]
void main(uint3 thread_id: SV_DispatchThreadID) {
    const uint gid = thread_id.x;

    const uint error_info_i = pc.as_geometry_gpu->error_info_i;
    const uint primitive_count = pc.as_geometry_gpu->primitive_count;

    if (pc.validation_mode == kBLASValidationMode_triangles_indices) {
        if (gid >= (3 * primitive_count)) {
            return;
        }

        uint64_t indices = pc.as_geometry_gpu->index_buffer;
        if (indices == 0) {
            return;
        }
        const uint index_type = pc.as_geometry_gpu->index_type;
        const uint primitive_offset = pc.as_geometry_gpu->primitive_offset;
        const uint first_vertex = pc.as_geometry_gpu->first_vertex;
        const uint max_vertex = pc.as_geometry_gpu->max_vertex;

        const uint fetched_index = LoadIndex(indices, index_type, primitive_offset, gid);
        if (max_vertex < (first_vertex + fetched_index)) {
            // In practice an invalid index does not cause a device loss, so don't bother changing its value.
            // Should someone add this back, a write barrier needs to be added on the CPU side.
            // StoreIndex(gid, 0);
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_MaxFetchedIndex,
                           error_info_i,
                           gid,
                           fetched_index,
                           0);
        }
    } else if (pc.validation_mode == kBLASValidationMode_active_triangles) {
        if (gid >= (3 * primitive_count)) {
            return;
        }

        if (pc.last_build_as_geometries_gpu == nullptr) {
            return;
        }

        AccelerationStructureGeometriesGPU* last_build = *(pc.last_build_as_geometries_gpu);
        // Can happen is no VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR was issued (deserialized AS)
        if (last_build == nullptr) {
            return;
        }

        const uint geom_i = pc.as_geometry_gpu->geometry_i;

        if (geom_i >= last_build->count) {
            return;
        }

        if (last_build->array[geom_i].geometry_type != pc.as_geometry_gpu->geometry_type) {
            return;
        }

        if (pc.as_geometry_gpu->vertex_format != VK_FORMAT_R32G32B32_SFLOAT) {
            return;
        }

        if (pc.as_geometry_gpu->geometry_buffer == 0) {
            return;
        }

        uint64_t last_build_indices = last_build->array[geom_i].index_buffer_copies;
        const uint last_build_index_type = last_build->array[geom_i].index_type;
        float* last_build_x_components = last_build->array[geom_i].geometry_x_components_copies;

        // No snapshot was taken at build time
        if (last_build_x_components == nullptr) {
            return;
        }
        if (last_build_index_type != VK_INDEX_TYPE_NONE_KHR && last_build_indices == 0) {
            return;
        }

        if (gid >= (3 * last_build->array[geom_i].primitive_count)) {
            return;
        }

        const uint fetched_index = LoadIndex(last_build_indices, last_build_index_type, 0, gid);

        if (fetched_index > last_build->array[geom_i].max_vertex) {
            return;
        }

        if (fetched_index > pc.as_geometry_gpu->max_vertex) {
            return;
        }

        const uint64_t update_time_stride = pc.as_geometry_gpu->stride;
        uint64_t update_time_vertex_buffer =
            pc.as_geometry_gpu->geometry_buffer + update_time_stride * pc.as_geometry_gpu->first_vertex;

        if (pc.as_geometry_gpu->index_type == VK_INDEX_TYPE_NONE_KHR) {
            update_time_vertex_buffer = update_time_vertex_buffer + pc.as_geometry_gpu->primitive_offset;
        }
        const float update_time_vertex_x = LoadVertexX(
            update_time_vertex_buffer,
            update_time_stride, fetched_index,
            pc.as_geometry_gpu->vertex_format);
        const bool is_update_time_vertex_nan = isnan(update_time_vertex_x);

        const float build_time_vertex_x = last_build_x_components[fetched_index];
        const bool is_build_time_vertex_nan = isnan(build_time_vertex_x);

        if (is_update_time_vertex_nan != is_build_time_vertex_nan) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_VertexBufferActiveStatusUpdated,
                           error_info_i,
                           gid,
                           fetched_index,
                           uint(is_build_time_vertex_nan));
        }
    } else if (pc.validation_mode == kBLASValidationMode_aabbs) {
        if (gid >= primitive_count) {
            return;
        }

        if (pc.as_geometry_gpu->geometry_buffer == 0) {
            return;
        }

        const uint64_t aabb_i = gid;
        VkAabbPositionsKHR* aabb = (VkAabbPositionsKHR*)(
            pc.as_geometry_gpu->geometry_buffer +
            uint64_t(pc.as_geometry_gpu->primitive_offset) + aabb_i * pc.as_geometry_gpu->stride);
        if (aabb->minX > aabb->maxX) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_X,
                           error_info_i,
                           gid,
                           asuint(aabb->minX),
                           asuint(aabb->maxX));
        }

        if (aabb->minY > aabb->maxY) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Y,
                           error_info_i,
                           gid,
                           asuint(aabb->minY),
                           asuint(aabb->maxY));
        }
        if (aabb->minZ > aabb->maxZ) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Z,
                           error_info_i,
                           gid,
                           asuint(aabb->minZ),
                           asuint(aabb->maxZ));
        }

        if (pc.as_geometry_gpu->is_update_build != 0) {
            if (pc.last_build_as_geometries_gpu == nullptr) {
                return;
            }

            AccelerationStructureGeometriesGPU* last_build = *(pc.last_build_as_geometries_gpu);
            // Can happen is no VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR was issued (deserialized AS)
            if (last_build == nullptr) {
                return;
            }

            const uint geom_i = pc.as_geometry_gpu->geometry_i;

            if (geom_i >= last_build->count) {
                return;
            }

            if (last_build->array[geom_i].geometry_type != pc.as_geometry_gpu->geometry_type) {
                return;
            }

            float* last_build_x_components = last_build->array[geom_i].geometry_x_components_copies;

            // No snapshot was taken at build time
            if (last_build_x_components == nullptr) {
                return;
            }

            // Make sure shader does not go OOB in last build data
            if (aabb_i >= last_build->array[geom_i].primitive_count) {
                return;
            }

            VkAabbPositionsKHR* update_time_aabb = aabb;
            const bool is_update_time_aabb_x_nan = isnan(update_time_aabb->minX);

            const float build_time_vertex_x = last_build_x_components[uint(aabb_i)];
            const bool is_build_time_aabb_x_nan = isnan(build_time_vertex_x);

            if (is_update_time_aabb_x_nan != is_build_time_aabb_x_nan) {
                GpuavLogError4(
                    kErrorGroup_GpuPreBuildAccelerationStructures,
                    kErrorSubCode_PreBuildAccelerationStructures_AabbBufferActiveStatusUpdated,
                    error_info_i,
                    gid,
                    uint(is_build_time_aabb_x_nan),
                    0);
            }

        }
    } else if (pc.validation_mode == kBLASValidationMode_transform_matrix) {
        if (gid > 0) {
            return;
        }

        VkTransformMatrix* transform = (VkTransformMatrix*)(
            pc.as_geometry_gpu->transform +
            pc.as_geometry_gpu->transform_offset);
        const float3x3 transform3x3 = float3x3(
            transform->matrix[0][0], transform->matrix[0][1], transform->matrix[0][2],
            transform->matrix[1][0], transform->matrix[1][1], transform->matrix[1][2],
            transform->matrix[2][0], transform->matrix[2][1], transform->matrix[2][2]
        );
        const float det = determinant(transform3x3);
        if (abs(det) < 1e-6) {
            GpuavLogError4(kErrorGroup_GpuPreBuildAccelerationStructures,
                           kErrorSubCode_PreBuildAccelerationStructures_Transform,
                           error_info_i,
                           gid,
                           0,
                           0);
        }
    }
}
