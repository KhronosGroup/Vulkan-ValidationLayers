/* Copyright (c) 2018-2026 The Khronos Group Inc.
 * Copyright (c) 2018-2026 Valve Corporation
 * Copyright (c) 2018-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_format_utils.h>
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/validation_cmd/memcmp.h"
#include "gpuav/shaders/validation_cmd/push_data.h"
#include "gpuav/shaders/validation_cmd/build_acceleration_structures.h"
#include "generated/gpuav_offline_spirv.h"
#include "error_message/error_strings.h"
#include "containers/limits.h"
#include "utils/math_utils.h"
#include "utils/ray_tracing_utils.h"

#include "profiling/profiling.h"

namespace gpuav {
namespace valcmd {

struct TraceRaysValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_trace_rays_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return validation_cmd_trace_rays_comp; }

    glsl::TraceRaysPushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

void TraceRaysIndirect(Validator& gpuav, const Location& loc, CommandBufferSubState& cb_state, const LastBound& last_bound,
                       VkDeviceAddress indirect_data_address) {
    if (!gpuav.gpuav_settings.validate_indirect_trace_rays_buffers) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    ValidationCommandsGpuavState& val_cmd_gpuav_state =
        gpuav.shared_resources_cache.GetOrCreate<ValidationCommandsGpuavState>(gpuav, loc);
    valpipe::ComputePipeline<TraceRaysValidationShader>& validation_pipeline =
        gpuav.shared_resources_cache.GetOrCreate<valpipe::ComputePipeline<TraceRaysValidationShader>>(
            gpuav, loc, val_cmd_gpuav_state.error_logging_desc_set_layout_);
    if (!validation_pipeline.valid) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Failed to create TraceRaysValidationShader.");
        return;
    }

    // Setup shader resources
    // ---
    {
        const uint64_t ray_query_dimension_max_width =
            static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
            static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[0]);
        const uint64_t ray_query_dimension_max_height =
            static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
            static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[1]);
        const uint64_t ray_query_dimension_max_depth =
            static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
            static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[2]);

        TraceRaysValidationShader shader_resources;
        shader_resources.push_constants.indirect_data = indirect_data_address;
        shader_resources.push_constants.trace_rays_width_limit =
            static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_width, vvl::kU32Max));
        shader_resources.push_constants.trace_rays_height_limit =
            static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_height, vvl::kU32Max));
        shader_resources.push_constants.trace_rays_depth_limit =
            static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_depth, vvl::kU32Max));
        shader_resources.push_constants.max_ray_dispatch_invocation_count =
            gpuav.phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount;

        if (!BindShaderResources(validation_pipeline, gpuav, cb_state, cb_state.compute_index, cb_state.GetErrorLoggerIndex(),
                                 shader_resources)) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Failed to GetManagedDescriptorSet in BindShaderResources");
            return;
        }
    }

    // Setup validation pipeline
    // ---
    {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger =
        [&gpuav](const uint32_t* error_record, const Location& loc_with_debug_region, const LogObjectList& objlist) {
            bool skip = false;
            using namespace glsl;

            if (GetErrorGroup(error_record) != kErrorGroup_GpuPreTraceRays) {
                return skip;
            }

            const uint32_t error_sub_code = GetSubError(error_record);
            switch (error_sub_code) {
                case kErrorSubCode_PreTraceRays_LimitWidth: {
                    const uint32_t width = error_record[kValCmd_ErrorPayloadDword_0];
                    skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-width-03638", objlist, loc_with_debug_region,
                                           "Indirect trace rays of VkTraceRaysIndirectCommandKHR::width of %" PRIu32
                                           " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] * "
                                           "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0] limit of %" PRIu64 ".",
                                           width,
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                               static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[0]));
                    break;
                }
                case kErrorSubCode_PreTraceRays_LimitHeight: {
                    const uint32_t height = error_record[kValCmd_ErrorPayloadDword_0];
                    skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-height-03639", objlist, loc_with_debug_region,
                                           "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                                           " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] * "
                                           "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1] limit of %" PRIu64 ".",
                                           height,
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                               static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[1]));
                    break;
                }
                case kErrorSubCode_PreTraceRays_LimitDepth: {
                    const uint32_t depth = error_record[kValCmd_ErrorPayloadDword_0];
                    skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-depth-03640", objlist, loc_with_debug_region,
                                           "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                                           " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] * "
                                           "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2] limit of %" PRIu64 ".",
                                           depth,
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                               static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[2]));
                    break;
                }
                case kErrorSubCode_PreTraceRays_LimitVolume: {
                    const VkExtent3D trace_rays_extent = {error_record[kValCmd_ErrorPayloadDword_0],
                                                          error_record[kValCmd_ErrorPayloadDword_1],
                                                          error_record[kValCmd_ErrorPayloadDword_2]};
                    const uint64_t rays_volume = trace_rays_extent.width * trace_rays_extent.height * trace_rays_extent.depth;
                    skip |= gpuav.LogError(
                        "VUID-VkTraceRaysIndirectCommandKHR-width-03641", objlist, loc_with_debug_region,
                        "Indirect trace rays of volume %" PRIu64
                        " (%s) would exceed VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayDispatchInvocationCount "
                        "limit of %" PRIu32 ".",
                        rays_volume, string_VkExtent3D(trace_rays_extent).c_str(),
                        gpuav.phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount);
                    break;
                }
                default:
                    break;
            }

            return skip;
        };

    cb_state.AddCommandErrorLogger(loc, &last_bound, std::move(error_logger));
}

struct BuildAccelerationStructuresValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_tlas_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return validation_cmd_tlas_comp; }

    glsl::TLASValidationShaderPushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

class DummyBLAS {
  public:
    DummyBLAS(Validator& gpuav, CommandBufferSubState& cb_state)
        : device(gpuav.device), vertex_buffer(gpuav), transform_buffer(gpuav), scratch_buffer(gpuav), blas_buffer(gpuav) {
        {
            VkBufferCreateInfo vertex_buffer_ci = vku::InitStructHelper();
            vertex_buffer_ci.size = 3 * 3 * sizeof(float);
            vertex_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
            alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            const VkResult result = vertex_buffer.Create(&vertex_buffer_ci, &alloc_ci);
            if (result != VK_SUCCESS) {
                gpuav.InternalVmaError(LogObjectList(), result, "Failed to create dummy BLAS's vertex buffer.");
                return;
            }
            constexpr std::array vertices = {// Vertex 0
                                             10.0f, 10.0f, 0.0f,
                                             // Vertex 1
                                             -10.0f, 10.0f, 0.0f,
                                             // Vertex 2
                                             0.0f, -10.0f, 0.0f};
            auto vertex_buffer_ptr = static_cast<float*>(vertex_buffer.GetMappedPtr());
            std::copy(vertices.begin(), vertices.end(), vertex_buffer_ptr);
        }

        {
            VkBufferCreateInfo transform_buffer_ci = vku::InitStructHelper();
            transform_buffer_ci.size = sizeof(VkTransformMatrixKHR) + 16;
            transform_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
            alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            const VkResult result = transform_buffer.Create(&transform_buffer_ci, &alloc_ci);
            if (result != VK_SUCCESS) {
                gpuav.InternalVmaError(LogObjectList(), result, "Failed to create dummy BLAS's transform buffer.");
                return;
            }
            // clang-format off
            VkTransformMatrixKHR transform_matrix = {{
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
            }};
            // clang-format on
            auto transform_buffer_ptr = static_cast<VkTransformMatrixKHR*>(transform_buffer.GetMappedPtr());
            std::memcpy(transform_buffer_ptr, &transform_matrix, sizeof(transform_matrix));
        }

        triangle = vku::InitStructHelper();
        triangle.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        triangle.geometry.triangles = vku::InitStructHelper();
        triangle.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        triangle.geometry.triangles.vertexData.deviceAddress = vertex_buffer.Address();
        triangle.geometry.triangles.vertexStride = 3 * sizeof(float);
        triangle.geometry.triangles.maxVertex = 2;
        triangle.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
        triangle.geometry.triangles.indexData.deviceAddress = 0;
        triangle.geometry.triangles.transformData.deviceAddress = Align<VkDeviceAddress>(transform_buffer.Address(), 16);

        as_build_geom_info = vku::InitStructHelper();
        as_build_geom_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        as_build_geom_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        as_build_geom_info.srcAccelerationStructure = VK_NULL_HANDLE;
        as_build_geom_info.dstAccelerationStructure = VK_NULL_HANDLE;
        as_build_geom_info.geometryCount = 1;
        as_build_geom_info.pGeometries = &triangle;
        as_build_geom_info.scratchData.deviceAddress = 0;
        const uint32_t max_prim_count = triangle.geometry.triangles.maxVertex;
        VkAccelerationStructureBuildSizesInfoKHR build_sizes_info = vku::InitStructHelper();
        DispatchGetAccelerationStructureBuildSizesKHR(gpuav.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                      &as_build_geom_info, &max_prim_count, &build_sizes_info);

        {
            VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
            scratch_buffer_ci.size = build_sizes_info.buildScratchSize +
                                     gpuav.phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment;
            scratch_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            if (gpuav.IsAllDeviceLocalMappable()) {
                alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            }
            const VkResult result = scratch_buffer.Create(&scratch_buffer_ci, &alloc_ci);
            if (result != VK_SUCCESS) {
                gpuav.InternalVmaError(LogObjectList(), result, "Failed to create dummy BLAS's scratch buffer.");
                return;
            }
        }
        {
            VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
            blas_buffer_ci.size = build_sizes_info.accelerationStructureSize;
            blas_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            if (gpuav.IsAllDeviceLocalMappable()) {
                alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            }
            const VkResult result = blas_buffer.Create(&blas_buffer_ci, &alloc_ci);
            if (result != VK_SUCCESS) {
                gpuav.InternalVmaError(LogObjectList(), result, "Failed to create dummy BLAS buffer.");
                return;
            }
        }

        as_build_geom_info.scratchData.deviceAddress = Align<VkDeviceAddress>(
            scratch_buffer.Address(), gpuav.phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment);
        VkAccelerationStructureCreateInfoKHR as_ci = vku::InitStructHelper();
        as_ci.buffer = blas_buffer.VkHandle();
        as_ci.offset = 0;
        as_ci.size = blas_buffer.Size();
        as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        DispatchCreateAccelerationStructureKHR(gpuav.device, &as_ci, nullptr, &blas_handle);
        as_build_geom_info.dstAccelerationStructure = blas_handle;

        VkAccelerationStructureDeviceAddressInfoKHR addr_info = vku::InitStructHelper();
        addr_info.accelerationStructure = blas_handle;
        blas_address = DispatchGetAccelerationStructureDeviceAddressKHR(gpuav.device, &addr_info);

        cb_state.on_pre_cb_submission_functions.emplace_back(
            [this](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
                VkAccelerationStructureBuildRangeInfoKHR triangle_build_range{};
                triangle_build_range.primitiveCount = 1;
                triangle_build_range.primitiveOffset = 0;
                triangle_build_range.firstVertex = 0;
                triangle_build_range.transformOffset = 0;

                std::array build_range_infos = {&triangle_build_range};
                DispatchCmdBuildAccelerationStructuresKHR(per_submission_cb, 1, &as_build_geom_info, build_range_infos.data());

                VkBufferMemoryBarrier barrier_blas_build = vku::InitStructHelper();
                barrier_blas_build.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
                barrier_blas_build.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                barrier_blas_build.buffer = blas_buffer.VkHandle();
                barrier_blas_build.offset = 0;
                barrier_blas_build.size = blas_buffer.Size();

                DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                           VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                           &barrier_blas_build, 0, nullptr);
            });
    }

    ~DummyBLAS() {
        if (blas_handle) {
            DispatchDestroyAccelerationStructureKHR(device, blas_handle, nullptr);
        }
        vertex_buffer.Destroy();
        transform_buffer.Destroy();
        scratch_buffer.Destroy();
        blas_buffer.Destroy();
    }

  public:
    VkDeviceAddress blas_address = 0;

  private:
    VkDevice device = VK_NULL_HANDLE;
    VkAccelerationStructureKHR blas_handle = VK_NULL_HANDLE;
    vko::Buffer vertex_buffer;
    vko::Buffer transform_buffer;
    vko::Buffer scratch_buffer;
    vko::Buffer blas_buffer;
    VkAccelerationStructureGeometryKHR triangle{};
    VkAccelerationStructureBuildGeometryInfoKHR as_build_geom_info{};
};

void TLAS(Validator& gpuav, const Location& loc, CommandBufferSubState& cb_state, const LastBound& last_bound, uint32_t info_count,
          const VkAccelerationStructureBuildGeometryInfoKHR* infos,
          const VkAccelerationStructureBuildRangeInfoKHR* const* build_ranges_infos) {
    VVL_ZoneScoped;
    if (!gpuav.gpuav_settings.validate_acceleration_structures_builds) {
        return;
    }

    struct BlasArray {
        VkDeviceAddress array_start_addr = 0;
        uint32_t size = 0;
        uint32_t is_array_of_pointers = 0;
        uint32_t info_i = 0;
        uint32_t geom_i = 0;
    };

    struct BlasBuiltInCmd {
        std::shared_ptr<vvl::AccelerationStructureKHR> blas = {};
        size_t p_info_i = 0;
    };
    std::vector<BlasArray> blas_arrays;
    std::vector<BlasBuiltInCmd> blas_built_in_cmd_array;
    for (const auto [info_i, info] : vvl::enumerate(infos, info_count)) {
        if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
            for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
                const VkAccelerationStructureGeometryKHR& geom = rt::GetGeometry(info, geom_i);
                const uint32_t primitive_count = build_ranges_infos[info_i][geom_i].primitiveCount;
                if (primitive_count > 0 && geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    BlasArray blas_array;
                    blas_array.size = primitive_count;
                    blas_array.array_start_addr =
                        geom.geometry.instances.data.deviceAddress + build_ranges_infos[info_i][geom_i].primitiveOffset;
                    blas_array.is_array_of_pointers = uint32_t(geom.geometry.instances.arrayOfPointers);
                    blas_array.info_i = info_i;
                    blas_array.geom_i = geom_i;
                    blas_arrays.emplace_back(blas_array);
                }
            }
        } else if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
            auto blas = gpuav.Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);
            if (blas) {
                BlasBuiltInCmd blas_build_in_cmd{};
                blas_build_in_cmd.blas = std::move(blas);
                blas_build_in_cmd.p_info_i = info_i;
                blas_built_in_cmd_array.emplace_back(blas_build_in_cmd);
            }
        }
    }

    // No TLAS built in command, so no validation to perform
    if (blas_arrays.empty()) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    ValidationCommandsGpuavState& val_cmd_gpuav_state =
        gpuav.shared_resources_cache.GetOrCreate<ValidationCommandsGpuavState>(gpuav, loc);
    valpipe::ComputePipeline<BuildAccelerationStructuresValidationShader>& validation_pipeline =
        gpuav.shared_resources_cache.GetOrCreate<valpipe::ComputePipeline<BuildAccelerationStructuresValidationShader>>(
            gpuav, loc, val_cmd_gpuav_state.error_logging_desc_set_layout_);
    if (!validation_pipeline.valid) {
        return;
    }

    vko::BufferRange as_arrays_ptr_buffer =
        cb_state.gpu_resources_manager.GetDeviceLocalBufferRange(sizeof(glsl::AccelerationStructureArraysPtr));

    cb_state.on_pre_cb_submission_functions.emplace_back(
        [as_arrays_ptr_buffer](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
            VVL_ZoneScopedN("validate_as_builds_pre_submit");

            ReadLockGuard lock(gpuav.device_state->as_with_addresses.array_mutex);

            // valid AS addresses buffer
            vko::BufferRange as_addresses_buffer = cb.gpu_resources_manager.GetHostCoherentBufferRange(
                2 * sizeof(uint32_t) + gpuav.device_state->as_with_addresses.array.size() * sizeof(uint64_t));
            auto accel_struct_addresses_buffer_u32_ptr = (uint32_t*)as_addresses_buffer.offset_mapped_ptr;

            *accel_struct_addresses_buffer_u32_ptr = (uint32_t)gpuav.device_state->as_with_addresses.array.size();

            auto as_addresses_ptr = (uint64_t*)(accel_struct_addresses_buffer_u32_ptr + 2);

            // valid AS metadata buffer
            vko::BufferRange as_metadatas_buffer = cb.gpu_resources_manager.GetHostCachedBufferRange(
                gpuav.device_state->as_with_addresses.array.size() * sizeof(uint32_t));
            auto as_metadatas_ptr = (uint32_t*)(as_metadatas_buffer.offset_mapped_ptr);

            // valid AS buffer address ranges buffer
            vko::BufferRange as_buffer_addr_ranges_buffer = cb.gpu_resources_manager.GetHostCoherentBufferRange(
                gpuav.device_state->as_with_addresses.array.size() * (2 * sizeof(uint64_t)));
            auto as_buffer_addr_ranges_ptr = (uint64_t*)(as_buffer_addr_ranges_buffer.offset_mapped_ptr);

            uint32_t written_count = 0;
            for (const vvl::AccelerationStructureKHR* as : gpuav.device_state->as_with_addresses.array) {
                as_addresses_ptr[written_count] = as->GetAccelerationStructureAddress();
                uint32_t metadata = 0;
                const auto as_buf = as->GetFirstValidBuffer(*gpuav.device_state);
                const bool is_buffer_alive = as_buf && !as_buf.state->Destroyed();
                const bool is_buffer_bound_to_memory = is_buffer_alive && as_buf.state->IsMemoryBound();
                metadata |= SET_BUILD_AS_METADATA_BUFFER_STATUS(is_buffer_alive);
                metadata |= SET_BUILD_AS_METADATA_AS_TYPE(as->GetType());
                metadata |= SET_BUILD_AS_METADATA_BUFFER_MEMORY_STATUS(is_buffer_bound_to_memory);
                as_metadatas_ptr[written_count] = metadata;
                const vvl::range<VkDeviceAddress> as_buffer_addr_range = as->GetVvlEffectiveDeviceAddressRange();
                as_buffer_addr_ranges_ptr[2 * written_count] = as_buffer_addr_range.begin;
                as_buffer_addr_ranges_ptr[2 * written_count + 1] = as_buffer_addr_range.end;

                ++written_count;
            }

            // Fill a GPU buffer with a pointer to the AS metadata
            vko::BufferRange submit_time_ptr_to_accel_structs_metadata_buffer =
                cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(glsl::AccelerationStructureArraysPtr));
            auto submit_time_ptr_to_accel_structs_metadata_buffer_ptr =
                (glsl::AccelerationStructureArraysPtr*)submit_time_ptr_to_accel_structs_metadata_buffer.offset_mapped_ptr;

            submit_time_ptr_to_accel_structs_metadata_buffer_ptr->addresses_ptr = as_addresses_buffer.offset_address;
            submit_time_ptr_to_accel_structs_metadata_buffer_ptr->metadata_ptr = as_metadatas_buffer.offset_address;
            submit_time_ptr_to_accel_structs_metadata_buffer_ptr->buffer_ranges_ptr = as_buffer_addr_ranges_buffer.offset_address;

            vko::CmdSynchronizedCopyBufferRange(per_submission_cb, as_arrays_ptr_buffer,
                                                submit_time_ptr_to_accel_structs_metadata_buffer);
        });

    // Setup Validation pipeline
    // ---
    {
        DummyBLAS& dummy_blas = gpuav.shared_resources_cache.GetOrCreate<DummyBLAS>(gpuav, cb_state);

        // Fill a buffer with BLAS built in this cmd
        vko::BufferRange blas_built_in_cmd_buffer;
        if (!blas_built_in_cmd_array.empty()) {
            blas_built_in_cmd_buffer =
                cb_state.gpu_resources_manager.GetHostCachedBufferRange(blas_built_in_cmd_array.size() * (2 * sizeof(uint64_t)));
            auto blas_built_in_cmd_buffer_ptr = (uint64_t*)(blas_built_in_cmd_buffer.offset_mapped_ptr);
            for (const auto [i, blas_built_in_cmd] : vvl::enumerate(blas_built_in_cmd_array)) {
                const vvl::range<VkDeviceAddress> blas_built_in_cmd_buffer_addr_range =
                    blas_built_in_cmd.blas->GetVvlEffectiveDeviceAddressRange();
                blas_built_in_cmd_buffer_ptr[2 * i] = blas_built_in_cmd_buffer_addr_range.begin;
                blas_built_in_cmd_buffer_ptr[2 * i + 1] = blas_built_in_cmd_buffer_addr_range.end;
            }
        }

        BuildAccelerationStructuresValidationShader shader_resources;
        shader_resources.push_constants.ptr_to_ptr_to_accel_structs_arrays = as_arrays_ptr_buffer.offset_address;
        shader_resources.push_constants.valid_dummy_blas_addr = dummy_blas.blas_address;
        shader_resources.push_constants.blas_built_in_cmd_array_ptr = blas_built_in_cmd_buffer.offset_address;
        shader_resources.push_constants.blas_built_in_cmd_array_size = (uint32_t)blas_built_in_cmd_array.size();

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        // Validation dispatch, one for each TLAS build
        // ---
        for (size_t blas_array_i = 0; blas_array_i < blas_arrays.size(); ++blas_array_i) {
            const auto blas_array_buffers = gpuav.GetBuffersByAddress(blas_arrays[blas_array_i].array_start_addr);
            if (blas_array_buffers.empty()) {
                assert(false);
            } else {
                VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
                barrier_write_after_read.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                barrier_write_after_read.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier_write_after_read.buffer = blas_array_buffers[0]->VkHandle();
                barrier_write_after_read.offset = 0;
                barrier_write_after_read.size = VK_WHOLE_SIZE;

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                           nullptr);
            }

            shader_resources.push_constants.validation_mode = glsl::kBuildASValidationMode_invalid_AS;
            const uint32_t is_array_of_pointers = blas_arrays[blas_array_i].is_array_of_pointers;
            if (is_array_of_pointers == 0) {
                shader_resources.push_constants.blas_array_start_addr = blas_arrays[blas_array_i].array_start_addr;
                shader_resources.push_constants.blas_ptr_array_start_addr = 0;
            } else {
                shader_resources.push_constants.blas_ptr_array_start_addr = blas_arrays[blas_array_i].array_start_addr;
                shader_resources.push_constants.blas_array_start_addr = 0;
            }

            shader_resources.push_constants.blas_array_size = blas_arrays[blas_array_i].size;
            shader_resources.push_constants.is_array_of_pointers = is_array_of_pointers;
            shader_resources.push_constants.blas_array_i = (uint32_t)blas_array_i;

            const bool bind_error_logging_desc_set = blas_array_i == 0;
            if (!BindShaderResources(validation_pipeline, gpuav, cb_state, cb_state.compute_index, cb_state.GetErrorLoggerIndex(),
                                     shader_resources, bind_error_logging_desc_set)) {
                assert(false);
                return;
            }

            constexpr uint32_t wg_size_x = 8;
            constexpr uint32_t wg_size_y = 8;

            const uint32_t as_instances_count = blas_arrays[blas_array_i].size;
            const uint32_t wg_count_x = as_instances_count / wg_size_x + uint32_t(as_instances_count % wg_size_x > 0);
            DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, 1, 1);

            shader_resources.push_constants.validation_mode = glsl::kBuildASValidationMode_memory_overlaps;

            BindShaderPushConstants(validation_pipeline, gpuav, cb_state, shader_resources);

            const uint32_t wg_count_y =
                (uint32_t)blas_built_in_cmd_array.size() / wg_size_y + uint32_t(blas_built_in_cmd_array.size() % wg_size_y > 0);
            DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, wg_count_y, 1);

            if (!blas_array_buffers.empty()) {
                VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier_read_after_write.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                barrier_read_after_write.buffer = blas_array_buffers[0]->VkHandle();
                barrier_read_after_write.offset = 0;
                barrier_read_after_write.size = VK_WHOLE_SIZE;

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                           &barrier_read_after_write, 0, nullptr);
            }
        }
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, blas_arrays = std::move(blas_arrays),
                                                           blas_built_in_cmd_array = std::move(blas_built_in_cmd_array)](
                                                              const uint32_t* error_record, const Location& loc_with_debug_region,
                                                              const LogObjectList& objlist) {
        bool skip = false;
        using namespace glsl;

        if (GetErrorGroup(error_record) != kErrorGroup_GpuPreBuildAccelerationStructures) {
            return skip;
        }

        const uint64_t blas_in_tlas_addr = glsl::GetUint64(error_record + kValCmd_ErrorPayloadDword_0);
        const uint32_t as_instance_i = error_record[kValCmd_ErrorPayloadDword_2];
        const uint32_t blas_array_i = error_record[kValCmd_ErrorPayloadDword_3];

        // Gather error info
        // ---
        const char* vvl_bug_msg =
            "this is most likely a validation layer bug. Please file an issue at "
            "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues ";
        const auto as_found_it =
            std::find_if(gpuav.device_state->as_with_addresses.array.begin(), gpuav.device_state->as_with_addresses.array.end(),
                         [blas_in_tlas_addr](vvl::AccelerationStructureKHR* as) {
                             return as->GetAccelerationStructureAddress() == blas_in_tlas_addr;
                         });
        std::stringstream ss_as;
        std::stringstream ss_as_buffer;
        if (as_found_it != gpuav.device_state->as_with_addresses.array.end()) {
            ss_as << "Acceleration structure corresponding to reference: " << gpuav.FormatHandle((*as_found_it)->VkHandle());
            if (const auto as_buffer = (*as_found_it)->GetFirstValidBuffer(*gpuav.device_state)) {
                ss_as_buffer << "(" << gpuav.FormatHandle(as_buffer.state->VkHandle()) << ") ";
            }
        } else {
            ss_as << "Could not map acceleration structure reference to its corresponding handle, " << vvl_bug_msg;
        }
        const std::string ss_as_str = ss_as.str();
        const std::string ss_buffer_str = ss_as_buffer.str();
        const BlasArray blas_array = blas_arrays[blas_array_i];
        std::ostringstream invalid_blas_loc;
        invalid_blas_loc << "pInfos[" << blas_array.info_i << "].pGeometries[" << blas_array.geom_i
                         << "].geometry.instances<VkAccelerationStructureInstance" << (blas_array.is_array_of_pointers ? " *" : "")
                         << ">[" << as_instance_i << ']' << (blas_array.is_array_of_pointers ? "->" : ".")
                         << "accelerationStructureReference (0x" << std::hex << blas_in_tlas_addr << ")";
        const std::string invalid_blas_loc_str = invalid_blas_loc.str();

        // Log error
        // ---
        const uint32_t error_sub_code = GetSubError(error_record);
        switch (error_sub_code) {
            case kErrorSubCode_PreBuildAccelerationStructures_BlasAddrAlignment: {
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03717", objlist, loc_with_debug_region,
                                       "%s is not aligned to 16 bytes.", invalid_blas_loc_str.c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_InvalidAS: {
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12281", objlist, loc_with_debug_region,
                                       "%s is an invalid acceleration structure reference.", invalid_blas_loc_str.c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_DestroyedASBuffer: {
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12281", objlist, loc_with_debug_region,
                                       "%s is an invalid acceleration structure reference - underlying buffer %swas already "
                                       "destroyed when build command started execution. %s.",
                                       invalid_blas_loc_str.c_str(), ss_buffer_str.c_str(), ss_as_str.c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_InvalidASType: {
                std::stringstream ss_as_type;
                if (as_found_it != gpuav.device_state->as_with_addresses.array.end()) {
                    ss_as_type << ", but has type " << string_VkAccelerationStructureTypeKHR((*as_found_it)->GetType()) << ". ";
                }
                const std::string ss_as_type_str = ss_as_type.str();
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12281", objlist, loc_with_debug_region,
                                       "%s is not a bottom level acceleration structure%s%s.", invalid_blas_loc_str.c_str(),
                                       ss_as_type_str.c_str(), ss_as_str.c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_DestroyedASMemory: {
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03709", objlist, loc_with_debug_region,
                                       "%s is an invalid acceleration structure reference - underlying buffer %s was not bound to "
                                       "memory anymore when build command started execution. Memory was probably destroyed. %s.",
                                       invalid_blas_loc_str.c_str(), ss_buffer_str.c_str(), ss_as_str.c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_BlasMemoryOverlap: {
                const uint32_t blas_built_in_cmd_i = error_record[kValCmd_ErrorPayloadDword_4];
                const BlasBuiltInCmd& blas_built_in_cmd = blas_built_in_cmd_array[blas_built_in_cmd_i];
                std::stringstream error_ss;
                if (as_found_it != gpuav.device_state->as_with_addresses.array.end()) {
                    const vvl::range<VkDeviceAddress> blas_in_tlas_buffer_addr_range =
                        (*as_found_it)->GetVvlEffectiveDeviceAddressRange();
                    const vvl::range<VkDeviceAddress> blas_built_in_cmd_buffer_addr_range =
                        blas_built_in_cmd.blas->GetVvlEffectiveDeviceAddressRange();
                    const vvl::range<VkDeviceAddress> overlap =
                        blas_in_tlas_buffer_addr_range & blas_built_in_cmd_buffer_addr_range;
                    assert(overlap.non_empty());
                    const VkAccelerationStructureKHR blas_built_in_cmd_handle = blas_built_in_cmd.blas->VkHandle();
                    const VkAccelerationStructureKHR blas_in_tlas_handle = (*as_found_it)->VkHandle();
                    const auto blas_cmd_as_buffer = blas_built_in_cmd.blas->GetFirstValidBuffer(*gpuav.device_state);
                    const auto blas_tlas_as_buffer = (*as_found_it)->GetFirstValidBuffer(*gpuav.device_state);
                    if (blas_built_in_cmd_handle != blas_in_tlas_handle) {
                        if (!blas_cmd_as_buffer || !blas_tlas_as_buffer) {
                            error_ss << "Could not retrieve buffer information, " << vvl_bug_msg;
                        } else {
                            error_ss << "pInfos[" << blas_built_in_cmd.p_info_i << "].dstAccelerationStructure ("
                                     << gpuav.FormatHandle(blas_built_in_cmd.blas->VkHandle()) << "), backed by buffer ("
                                     << gpuav.FormatHandle(blas_cmd_as_buffer.state->VkHandle())
                                     << "), overlaps on buffer address range " << vvl::string_range_hex(overlap) << " with buffer ("
                                     << gpuav.FormatHandle(blas_tlas_as_buffer.state->VkHandle()) << ") of BLAS ("
                                     << gpuav.FormatHandle((*as_found_it)->VkHandle()) << "), referenced in "
                                     << invalid_blas_loc_str;
                        }
                    } else {
                        error_ss << "pInfos[" << blas_built_in_cmd.p_info_i << "].dstAccelerationStructure ("
                                 << gpuav.FormatHandle(blas_built_in_cmd.blas->VkHandle())
                                 << ") is also referenced in a TLAS built in the same command, through " << invalid_blas_loc_str;
                    }
                } else {
                    error_ss << "Could not retrieve error information, " << vvl_bug_msg;
                }
                const std::string error_str = error_ss.str();
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03706", objlist,
                                       loc_with_debug_region, "%s.", error_str.c_str());
                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.AddCommandErrorLogger(loc, &last_bound, std::move(error_logger));
}

struct BLASValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_blas_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return validation_cmd_blas_comp; }

    glsl::BLASValidationShaderPushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

struct MemcmpShader {
    static size_t GetSpirvSize() { return validation_cmd_memcmp_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return validation_cmd_memcmp_comp; }

    glsl::MemShaderPushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }
    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

void BLAS(Validator& gpuav, const Location& loc, CommandBufferSubState& cb_state, const LastBound& last_bound, uint32_t info_count,
          const VkAccelerationStructureBuildGeometryInfoKHR* infos,
          const VkAccelerationStructureBuildRangeInfoKHR* const* pp_build_ranges_infos) {
    VVL_ZoneScoped;
    if (!gpuav.gpuav_settings.validate_acceleration_structures_builds) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    ValidationCommandsGpuavState& val_cmd_gpuav_state =
        gpuav.shared_resources_cache.GetOrCreate<ValidationCommandsGpuavState>(gpuav, loc);

    valpipe::ComputePipeline<BLASValidationShader>& blas_pipeline =
        gpuav.shared_resources_cache.GetOrCreate<valpipe::ComputePipeline<BLASValidationShader>>(
            gpuav, loc, val_cmd_gpuav_state.error_logging_desc_set_layout_);
    if (!blas_pipeline.valid) {
        return;
    }

    valpipe::ComputePipeline<MemcmpShader>& memcmp_pipeline =
        gpuav.shared_resources_cache.GetOrCreate<valpipe::ComputePipeline<MemcmpShader>>(
            gpuav, loc, val_cmd_gpuav_state.error_logging_desc_set_layout_);
    if (!memcmp_pipeline.valid) {
        return;
    }

    struct ErrorInfo {
        uint32_t info_i{};
        uint32_t geom_i{};
        VkAccelerationStructureBuildGeometryInfoKHR info;
        VkGeometryTypeKHR geom_type;
        VkAccelerationStructureGeometryDataKHR geom;
        VkAccelerationStructureBuildRangeInfoKHR build_range_info{};
    };

    std::vector<ErrorInfo> error_infos;
    // Setup Validation pipeline
    // ---
    {
        for (uint32_t info_i = 0; info_i < info_count; ++info_i) {
            const VkAccelerationStructureBuildGeometryInfoKHR& info = infos[info_i];

            for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
                const VkAccelerationStructureGeometryKHR& geom_data = rt::GetGeometry(info, geom_i);

                const bool setup_triangles_validation = geom_data.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                const bool setup_aabbs_validation = geom_data.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR;
                const bool setup_transform_validation = geom_data.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR &&
                                                        geom_data.geometry.triangles.transformData.deviceAddress != 0;

                if (!setup_triangles_validation && !setup_aabbs_validation && !setup_transform_validation) {
                    continue;
                }

                auto dst_as_state = gpuav.Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);
                if (!dst_as_state) {
                    gpuav.InternalError(info.dstAccelerationStructure, loc,
                                        "gpuav::valcmd::BLAS(): Unrecognized destination acceleration structure.");
                    return;
                }
                AccelerationStructureKHRSubState& dst_as_gpuav_state = SubState(*dst_as_state);

                const VkAccelerationStructureBuildRangeInfoKHR& build_range_info = pp_build_ranges_infos[info_i][geom_i];

                ErrorInfo& error_info = error_infos.emplace_back();
                error_info.info = info;
                error_info.info_i = info_i;
                error_info.geom_i = geom_i;
                error_info.geom_type = geom_data.geometryType;
                error_info.geom = geom_data.geometry;
                error_info.build_range_info = build_range_info;

                glsl::BLASValidationShaderPushData blash_shader_common_pc{};
                blash_shader_common_pc.first_vertex = build_range_info.firstVertex;
                blash_shader_common_pc.primitive_offset = build_range_info.primitiveOffset;
                blash_shader_common_pc.primitive_count = build_range_info.primitiveCount;
                blash_shader_common_pc.error_info_i = uint32_t(error_infos.size() - 1);

                constexpr uint32_t shader_wg_size_x = 64;

                auto copy_geom_buffer = [&gpuav, &cb_state](vko::BufferRange& dst_buffer_range, VkDeviceSize byte_size,
                                                            VkDeviceAddress src_addr, vvl::Buffer* const src_buffer) {
                    if (dst_buffer_range.buffer != VK_NULL_HANDLE) {
                        gpuav.gpu_resources_manager_.ReturnDeviceLocalBufferRange(dst_buffer_range);
                    }
                    dst_buffer_range = gpuav.gpu_resources_manager_.GetDeviceLocalBufferRange(byte_size);

                    VkBufferCopy region{};
                    region.srcOffset = src_addr - src_buffer->deviceAddress;
                    region.dstOffset = dst_buffer_range.offset;
                    region.size = byte_size;
                    DispatchCmdCopyBuffer(cb_state.VkHandle(), src_buffer->VkHandle(), dst_buffer_range.buffer, 1, &region);

                    VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                    barrier_read_after_write.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier_read_after_write.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                    barrier_read_after_write.buffer = dst_buffer_range.buffer;
                    barrier_read_after_write.offset = 0;
                    barrier_read_after_write.size = VK_WHOLE_SIZE;

                    DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                               &barrier_read_after_write, 0, nullptr);
                };

                // By definition, a buffer containing the full vertex/index buffers range should exist.
                // Just use the biggest buffer at least containing this full index buffer range
                // for validation purposes.
                auto get_buffer_with_max_addr_range = [](vvl::span<vvl::Buffer* const> buffers, VkDeviceAddress addr) {
                    size_t max_buffer_addr_range_i = 0;
                    VkDeviceSize max_buffer_addr_range = (buffers[0]->deviceAddress + buffers[0]->GetSize()) - addr;
                    {
                        for (const auto [buffer_i, buffer] : vvl::enumerate(buffers.data(), buffers.size())) {
                            const VkDeviceSize buffer_range =
                                (buffers[buffer_i]->deviceAddress + buffers[buffer_i]->GetSize()) - addr;
                            if (buffer_range > max_buffer_addr_range) {
                                max_buffer_addr_range = buffer_range;
                                max_buffer_addr_range_i = buffer_i;
                            }
                        }
                    }

                    return max_buffer_addr_range_i;
                };

                if (setup_triangles_validation) {
                    const bool as_build_allowing_updates = (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR) &&
                                                           (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
                    const bool as_update = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
                    const VkFormat vertex_format = geom_data.geometry.triangles.vertexFormat;
                    const bool validated_vertex_format = vertex_format == VK_FORMAT_R32G32B32_SFLOAT;
                    const bool validate_vertex_buffer_consistency = gpuav.gpuav_settings.ray_tracing_buffers_consistency &&
                                                                    validated_vertex_format &&
                                                                    (as_build_allowing_updates || as_update);

                    if (validate_vertex_buffer_consistency) {
                        const auto vertex_buffers =
                            gpuav.GetBuffersByAddress(geom_data.geometry.triangles.vertexData.deviceAddress);
                        if (vertex_buffers.empty()) {
                            return;
                        }

                        const size_t max_buffer_addr_range_i =
                            get_buffer_with_max_addr_range(vertex_buffers, geom_data.geometry.triangles.vertexData.deviceAddress);

                        // const uint32_t format_byte_size = vkuFormatTexelBlockSize(vertex_format);
                        const VkDeviceSize vertex_buffer_byte_size =
                            (geom_data.geometry.triangles.maxVertex + 1) * geom_data.geometry.triangles.vertexStride;

                        if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR) {
                            // Copy vertex buffer
                            dst_as_gpuav_state.geometry_buffer_copies.resize(info.geometryCount);
                            AccelerationStructureKHRSubState::GeometryBufferRange& vertex_buffer_copy =
                                dst_as_gpuav_state.geometry_buffer_copies[geom_i];
                            vertex_buffer_copy.stride = geom_data.geometry.triangles.vertexStride;
                            VkDeviceAddress vertices_addr = geom_data.geometry.triangles.vertexData.deviceAddress;
                            vertices_addr += build_range_info.primitiveOffset +
                                             geom_data.geometry.triangles.vertexStride * build_range_info.firstVertex;
                            copy_geom_buffer(vertex_buffer_copy.range, vertex_buffer_byte_size, vertices_addr,
                                             vertex_buffers[max_buffer_addr_range_i]);
                        } else if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
                                   (geom_i < dst_as_gpuav_state.geometry_buffer_copies.size()) &&
                                   (vertex_buffer_byte_size == dst_as_gpuav_state.geometry_buffer_copies[geom_i].range.size)) {
                            // Validate that vertex buffer supplied during last build still has the same inactive primitives at the
                            // same positions
                            AccelerationStructureKHRSubState::GeometryBufferRange& vertex_buffer_copy =
                                dst_as_gpuav_state.geometry_buffer_copies[geom_i];
                            BLASValidationShader blas_shader_resources{};
                            blas_shader_resources.push_constants = blash_shader_common_pc;
                            blas_shader_resources.push_constants.validation_mode = glsl::kBLASValidationMode_active_triangles;
                            blas_shader_resources.push_constants.address = geom_data.geometry.triangles.indexData.deviceAddress;
                            blas_shader_resources.push_constants.address_2 = geom_data.geometry.triangles.vertexData.deviceAddress;
                            blas_shader_resources.push_constants.address_3 = vertex_buffer_copy.range.offset_address;
                            blas_shader_resources.push_constants.update_time_stride = geom_data.geometry.triangles.vertexStride;
                            blas_shader_resources.push_constants.build_time_stride = vertex_buffer_copy.stride;
                            blas_shader_resources.push_constants.index_type = geom_data.geometry.triangles.indexType;
                            blas_shader_resources.push_constants.vertex_format = vertex_format;
                            blas_shader_resources.push_constants.first_vertex = build_range_info.firstVertex;
                            blas_shader_resources.push_constants.primitive_offset = build_range_info.primitiveOffset;
                            blas_shader_resources.push_constants.primitive_count = build_range_info.primitiveCount;
                            blas_shader_resources.push_constants.error_info_i = uint32_t(error_infos.size() - 1);

                            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, blas_pipeline.pipeline);

                            if (!BindShaderResources(blas_pipeline, gpuav, cb_state, cb_state.compute_index,
                                                     cb_state.GetErrorLoggerIndex(), blas_shader_resources)) {
                                assert(false);
                                return;
                            }

                            const uint32_t wg_count_x = (3 * build_range_info.primitiveCount) / shader_wg_size_x +
                                                        uint32_t(((3 * build_range_info.primitiveCount) % shader_wg_size_x) > 0);
                            DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, 1, 1);

                            VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                            barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                            barrier_read_after_write.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                            barrier_read_after_write.buffer = vertex_buffers[max_buffer_addr_range_i]->VkHandle();
                            barrier_read_after_write.offset = 0;
                            barrier_read_after_write.size = VK_WHOLE_SIZE;

                            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                                       &barrier_read_after_write, 0, nullptr);
                        }
                    }

                    const bool setup_triangle_indices_validation = geom_data.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR &&
                                                                   geom_data.geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR;
                    if (setup_triangle_indices_validation) {
                        const auto index_buffers = gpuav.GetBuffersByAddress(geom_data.geometry.triangles.indexData.deviceAddress);
                        if (index_buffers.empty()) {
                            return;
                        }

                        const size_t max_buffer_addr_range_i =
                            get_buffer_with_max_addr_range(index_buffers, geom_data.geometry.triangles.indexData.deviceAddress);

                        const bool validate_index_buffer_consistency =
                            gpuav.gpuav_settings.ray_tracing_buffers_consistency && (as_build_allowing_updates || as_update);
                        if (validate_index_buffer_consistency) {
                            const VkDeviceSize index_buffer_byte_size = VkDeviceSize(3) * build_range_info.primitiveCount *
                                                                        IndexTypeByteSize(geom_data.geometry.triangles.indexType);
                            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR) {
                                dst_as_gpuav_state.index_buffer_copies.resize(info.geometryCount);
                                vko::BufferRange& index_buffer_copy = dst_as_gpuav_state.index_buffer_copies[geom_i];
                                VkDeviceAddress indices_addr = geom_data.geometry.triangles.indexData.deviceAddress;
                                indices_addr += build_range_info.primitiveOffset;
                                // #ARNO_TODO add test playing on primitiveOffset
                                copy_geom_buffer(index_buffer_copy, index_buffer_byte_size, indices_addr,
                                                 index_buffers[max_buffer_addr_range_i]);
                            }
                            // Core checks already validates that index count is the same as last build
                            else if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
                                     (geom_i < dst_as_gpuav_state.index_buffer_copies.size()) &&
                                     (index_buffer_byte_size == dst_as_gpuav_state.index_buffer_copies[geom_i].size)) {
                                MemcmpShader memcmp_shader_resources;
                                memcmp_shader_resources.push_constants.update_time_indices_addr =
                                    geom_data.geometry.triangles.indexData.deviceAddress + build_range_info.primitiveOffset;
                                memcmp_shader_resources.push_constants.build_time_indices_addr =
                                    dst_as_gpuav_state.index_buffer_copies[geom_i].offset_address;
                                memcmp_shader_resources.push_constants.uvec4_count =
                                    uint32_t(index_buffer_byte_size / (4 * sizeof(uint32_t)));
                                uint32_t index_buffer_bytes_leftover =
                                    uint32_t(index_buffer_byte_size -
                                             (memcmp_shader_resources.push_constants.uvec4_count * 4 * sizeof(uint32_t)));
                                memcmp_shader_resources.push_constants.u32_count = index_buffer_bytes_leftover / sizeof(uint32_t);
                                index_buffer_bytes_leftover -= memcmp_shader_resources.push_constants.u32_count * sizeof(uint32_t);
                                memcmp_shader_resources.push_constants.u16_count = index_buffer_bytes_leftover / sizeof(uint16_t);
                                index_buffer_bytes_leftover -= memcmp_shader_resources.push_constants.u16_count * sizeof(uint16_t);
                                (void)index_buffer_bytes_leftover;
                                assert(index_buffer_bytes_leftover == 0);

                                memcmp_shader_resources.push_constants.error_info_i = uint32_t(error_infos.size() - 1);

                                DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE,
                                                        memcmp_pipeline.pipeline);

                                if (!BindShaderResources(memcmp_pipeline, gpuav, cb_state, cb_state.compute_index,
                                                         cb_state.GetErrorLoggerIndex(), memcmp_shader_resources)) {
                                    assert(false);
                                    return;
                                }

                                const uint32_t shader_threads_count = memcmp_shader_resources.push_constants.uvec4_count +
                                                                      memcmp_shader_resources.push_constants.u32_count +
                                                                      memcmp_shader_resources.push_constants.u16_count;
                                const uint32_t wg_count_x = shader_threads_count / shader_wg_size_x +
                                                            uint32_t((shader_threads_count % shader_wg_size_x) > 0);
                                DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, 1, 1);
                            }
                        }

                        BLASValidationShader blas_shader_resources{};
                        blas_shader_resources.push_constants = blash_shader_common_pc;
                        blas_shader_resources.push_constants.validation_mode = glsl::kBLASValidationMode_triangles_indices;
                        blas_shader_resources.push_constants.address = geom_data.geometry.triangles.indexData.deviceAddress;
                        blas_shader_resources.push_constants.index_type = geom_data.geometry.triangles.indexType;
                        blas_shader_resources.push_constants.max_vertex = geom_data.geometry.triangles.maxVertex;

                        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, blas_pipeline.pipeline);

                        if (!BindShaderResources(blas_pipeline, gpuav, cb_state, cb_state.compute_index,
                                                 cb_state.GetErrorLoggerIndex(), blas_shader_resources)) {
                            assert(false);
                            return;
                        }

                        const uint32_t wg_count_x = (3 * build_range_info.primitiveCount) / shader_wg_size_x +
                                                    uint32_t(((3 * build_range_info.primitiveCount) % shader_wg_size_x) > 0);
                        DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, 1, 1);

                        VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                        barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        barrier_read_after_write.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                        barrier_read_after_write.buffer = index_buffers[max_buffer_addr_range_i]->VkHandle();
                        barrier_read_after_write.offset = 0;
                        barrier_read_after_write.size = VK_WHOLE_SIZE;

                        DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                   VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                                   &barrier_read_after_write, 0, nullptr);
                    }
                }

                if (setup_aabbs_validation) {
                    const auto aabb_buffers = gpuav.GetBuffersByAddress(geom_data.geometry.aabbs.data.deviceAddress);
                    if (aabb_buffers.empty()) {
                        return;
                    }

                    const size_t max_buffer_addr_range_i =
                        get_buffer_with_max_addr_range(aabb_buffers, geom_data.geometry.aabbs.data.deviceAddress);
                    const VkDeviceSize aabb_buffer_size = build_range_info.primitiveCount * geom_data.geometry.aabbs.stride;
                    if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR) {
                        dst_as_gpuav_state.geometry_buffer_copies.resize(info.geometryCount);
                        AccelerationStructureKHRSubState::GeometryBufferRange& aabb_buffer_copy =
                            dst_as_gpuav_state.geometry_buffer_copies[geom_i];
                        aabb_buffer_copy.stride = geom_data.geometry.aabbs.stride;
                        VkDeviceAddress aabbs_addr = geom_data.geometry.aabbs.data.deviceAddress;
                        aabbs_addr += build_range_info.primitiveOffset;
                        copy_geom_buffer(aabb_buffer_copy.range, aabb_buffer_size, aabbs_addr,
                                         aabb_buffers[max_buffer_addr_range_i]);
                    } else if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
                               (geom_i < dst_as_gpuav_state.geometry_buffer_copies.size()) &&
                               (aabb_buffer_size == dst_as_gpuav_state.geometry_buffer_copies[geom_i].range.size)) {
                        AccelerationStructureKHRSubState::GeometryBufferRange& aabb_buffer_copy =
                            dst_as_gpuav_state.geometry_buffer_copies[geom_i];
                        BLASValidationShader blas_shader_resources{};
                        blas_shader_resources.push_constants = blash_shader_common_pc;
                        blas_shader_resources.push_constants.validation_mode = glsl::kBLASValidationMode_aabbs;
                        blas_shader_resources.push_constants.address = geom_data.geometry.aabbs.data.deviceAddress;
                        blas_shader_resources.push_constants.address_2 = aabb_buffer_copy.range.offset_address;
                        blas_shader_resources.push_constants.update_time_stride = geom_data.geometry.aabbs.stride;
                        blas_shader_resources.push_constants.build_time_stride = aabb_buffer_copy.stride;
                        blas_shader_resources.push_constants.primitive_offset = build_range_info.primitiveOffset;
                        blas_shader_resources.push_constants.primitive_count = build_range_info.primitiveCount;

                        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, blas_pipeline.pipeline);

                        if (!BindShaderResources(blas_pipeline, gpuav, cb_state, cb_state.compute_index,
                                                 cb_state.GetErrorLoggerIndex(), blas_shader_resources)) {
                            assert(false);
                            return;
                        }

                        const uint32_t wg_count_x = build_range_info.primitiveCount / shader_wg_size_x +
                                                    uint32_t((build_range_info.primitiveCount % shader_wg_size_x) > 0);
                        DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, 1, 1);
                    }

                    BLASValidationShader blas_shader_resources{};
                    blas_shader_resources.push_constants = blash_shader_common_pc;
                    blas_shader_resources.push_constants.validation_mode = glsl::kBLASValidationMode_aabbs;
                    blas_shader_resources.push_constants.address = geom_data.geometry.aabbs.data.deviceAddress;
                    blas_shader_resources.push_constants.update_time_stride = geom_data.geometry.aabbs.stride;

                    DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, blas_pipeline.pipeline);

                    if (!BindShaderResources(blas_pipeline, gpuav, cb_state, cb_state.compute_index, cb_state.GetErrorLoggerIndex(),
                                             blas_shader_resources)) {
                        assert(false);
                        return;
                    }

                    const uint32_t wg_count_x = (build_range_info.primitiveCount) / shader_wg_size_x +
                                                uint32_t(((build_range_info.primitiveCount) % shader_wg_size_x) > 0);
                    DispatchCmdDispatch(cb_state.VkHandle(), wg_count_x, 1, 1);

                    VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                    barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    barrier_read_after_write.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                    barrier_read_after_write.buffer = aabb_buffers[max_buffer_addr_range_i]->VkHandle();
                    barrier_read_after_write.offset = 0;
                    barrier_read_after_write.size = VK_WHOLE_SIZE;

                    DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                               &barrier_read_after_write, 0, nullptr);
                }

                if (setup_transform_validation) {
                    BLASValidationShader blas_shader_resources{};
                    blas_shader_resources.push_constants = blash_shader_common_pc;
                    blas_shader_resources.push_constants.validation_mode = glsl::kBLASValidationMode_transform_matrix;
                    blas_shader_resources.push_constants.primitive_offset = build_range_info.transformOffset;
                    blas_shader_resources.push_constants.address = geom_data.geometry.triangles.transformData.deviceAddress;

                    DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, blas_pipeline.pipeline);

                    if (!BindShaderResources(blas_pipeline, gpuav, cb_state, cb_state.compute_index, cb_state.GetErrorLoggerIndex(),
                                             blas_shader_resources)) {
                        assert(false);
                        return;
                    }

                    DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);

                    if (const auto transform_buffers =
                            gpuav.GetBuffersByAddress(geom_data.geometry.triangles.transformData.deviceAddress);
                        !transform_buffers.empty()) {
                        VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                        barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        barrier_read_after_write.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                        barrier_read_after_write.buffer = transform_buffers[0]->VkHandle();
                        barrier_read_after_write.offset = 0;
                        barrier_read_after_write.size = VK_WHOLE_SIZE;

                        DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                   VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                                   &barrier_read_after_write, 0, nullptr);
                    }
                }
            }
        }
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, error_infos = std::move(error_infos)](
                                                              const uint32_t* error_record, const Location& loc_with_debug_region,
                                                              const LogObjectList& objlist) {
        bool skip = false;
        using namespace glsl;

        if (GetErrorGroup(error_record) != kErrorGroup_GpuPreBuildAccelerationStructures) {
            return skip;
        }

        const uint32_t error_info_i = error_record[kValCmd_ErrorPayloadDword_0];
        const uint32_t gid = error_record[kValCmd_ErrorPayloadDword_1];

        assert(error_info_i < error_infos.size());
        const ErrorInfo& error_info = error_infos[error_info_i];

        // Log error
        // ---
        const uint32_t error_sub_code = GetSubError(error_record);
        switch (error_sub_code) {
            case kErrorSubCode_PreBuildAccelerationStructures_MaxFetchedIndex: {
                const uint32_t index = error_record[kValCmd_ErrorPayloadDword_2];
                const uint32_t index_type_byte_size = IndexTypeByteSize(error_info.geom.triangles.indexType);

                skip |= gpuav.LogError(
                    "VUID-VkAccelerationStructureBuildRangeInfoKHR-maxVertex-10774", objlist, loc_with_debug_region,
                    "Index out of bounds.\n"
                    "Index (%" PRIu32 ") + firstVertex (%" PRIu32 ") = %" PRIu32
                    " but VkAccelerationStructureGeometryTrianglesDataKHR::maxVertex is only %" PRIu32
                    ".\n"
                    "Index buffer starts at VkDeviceAddress indexData (0x%" PRIx64 ") + primitiveOffset (%" PRIu32 ") = 0x%" PRIx64
                    "\n"
                    "Given index type of %s, and that starting address, that's IndexBuffer[%" PRIu32
                    "] (VkDeviceAddress: 0x%" PRIx64
                    ")\n\n"

                    "Corresponding BLAS build command info:\n"
                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32 "]::VkAccelerationStructureGeometryKHR[%" PRIu32
                    "]::VkAccelerationStructureGeometryTrianglesDataKHR was:\n%s\n"

                    "VkAccelerationStructureBuildRangeInfoKHR[%" PRIu32 "][%" PRIu32 "] was:\n%s\n",

                    index, error_info.build_range_info.firstVertex, index + error_info.build_range_info.firstVertex,
                    error_info.geom.triangles.maxVertex, error_info.geom.triangles.indexData.deviceAddress,
                    error_info.build_range_info.primitiveOffset,
                    error_info.geom.triangles.indexData.deviceAddress + error_info.build_range_info.primitiveOffset,
                    string_VkIndexType(error_info.geom.triangles.indexType), gid,
                    error_info.geom.triangles.indexData.deviceAddress + error_info.build_range_info.primitiveOffset +
                        gid * index_type_byte_size,

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureGeometryTrianglesDataKHR(*gpuav.device_state, error_info.geom.triangles).c_str(),
                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureBuildRangeInfoKHR(error_info.build_range_info).c_str());
                break;
            }

            case kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_X:
            case kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Y:
            case kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Z: {
                // Should use std::bit_cast but requires c++20
                const float min = *(float*)(error_record + kValCmd_ErrorPayloadDword_2);
                const float max = *(float*)(error_record + kValCmd_ErrorPayloadDword_3);
                vvl::Field min_field{};
                vvl::Field max_field{};
                const char* vuid = "";
                switch (error_sub_code) {
                    case kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_X:
                        min_field = vvl::Field::minX;
                        max_field = vvl::Field::maxX;
                        vuid = "VUID-VkAabbPositionsKHR-minX-03546";
                        break;
                    case kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Y:
                        min_field = vvl::Field::minY;
                        max_field = vvl::Field::maxY;
                        vuid = "VUID-VkAabbPositionsKHR-minY-03547";
                        break;
                    case kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Z:
                        min_field = vvl::Field::minZ;
                        max_field = vvl::Field::maxZ;
                        vuid = "VUID-VkAabbPositionsKHR-minZ-03548";
                        break;
                    default:
                        break;
                }
                skip |= gpuav.LogError(
                    vuid, objlist, loc_with_debug_region,
                    "Ill formed AABB at primitive index %" PRIu32
                    ".\n"
                    "%s (%f) > %s (%f)\n"
                    "AABB was found at VkDeviceAddress aabbs.data (0x%" PRIx64 ") + primitiveOffset (%" PRIu32
                    ") + primitive index (%" PRIu32 ") * stride (%" PRIu64 ") = 0x%" PRIx64
                    "\n"

                    "Corresponding BLAS build command info:\n"
                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32 "]::VkAccelerationStructureGeometryKHR[%" PRIu32
                    "]::VkAccelerationStructureGeometryAabbsDataKHR was:\n%s\n"

                    "VkAccelerationStructureBuildRangeInfoKHR[%" PRIu32 "][%" PRIu32 "] was:\n%s\n",

                    gid, vvl::String(min_field), min, vvl::String(max_field), max, error_info.geom.aabbs.data.deviceAddress,
                    error_info.build_range_info.primitiveOffset, gid, error_info.geom.aabbs.stride,
                    error_info.geom.aabbs.data.deviceAddress + error_info.build_range_info.primitiveOffset +
                        gid * error_info.geom.aabbs.stride,

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureGeometryAabbsDataKHR(*gpuav.device_state, error_info.geom.aabbs).c_str(),

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureBuildRangeInfoKHR(error_info.build_range_info).c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_Transform: {
                skip |= gpuav.LogError(
                    "VUID-VkTransformMatrixKHR-matrix-03799", objlist, loc_with_debug_region,
                    "Transform matrix's first three columns do not define an invertible 3x3 matrix.\n"
                    "Corresponding BLAS build command info:\n"
                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32 "]::VkAccelerationStructureGeometryKHR[%" PRIu32
                    "]::VkAccelerationStructureGeometryTrianglesDataKHR was:\n%s\n"

                    "VkAccelerationStructureBuildRangeInfoKHR[%" PRIu32 "][%" PRIu32 "] was:\n%s\n",

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureGeometryTrianglesDataKHR(*gpuav.device_state, error_info.geom.triangles).c_str(),
                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureBuildRangeInfoKHR(error_info.build_range_info).c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_IndexBufferUpdated: {
                const uint32_t u32_diff_byte_offset = error_record[kValCmd_ErrorPayloadDword_1];
                const uint32_t memcmp_diff_type = error_record[kValCmd_ErrorPayloadDword_2];
                // #ARNO_TODO Do I actually don't need that? Or did I mess up?
                (void)memcmp_diff_type;
                const uint32_t update_time_index_dword = error_record[kValCmd_ErrorPayloadDword_3];
                const uint32_t build_time_index_dword = error_record[kValCmd_ErrorPayloadDword_4];

                // Get first differing byte in differing indices dwords
                uint32_t diff_byte_i = 0;
                for (; diff_byte_i < 4; ++diff_byte_i) {
                    const uint32_t update_time_byte_i = (update_time_index_dword >> (8u * diff_byte_i)) & 0xff;
                    const uint32_t build_time_byte_i = (build_time_index_dword >> (8u * diff_byte_i)) & 0xff;
                    if (update_time_byte_i != build_time_byte_i) {
                        break;
                    }
                }

                // Now based on index type, get:
                // - Position in index buffer
                // - Corresponding VkDeviceAddress
                // - Build time index value
                // - Update time index value
                uint32_t index_buffer_pos = 0;
                VkDeviceAddress index_address = 0;
                uint32_t update_time_index = 0;
                uint32_t build_time_index = 0;
                switch (error_info.geom.triangles.indexType) {
                    case VK_INDEX_TYPE_UINT16: {
                        index_buffer_pos = u32_diff_byte_offset / sizeof(uint16_t) + (diff_byte_i / sizeof(uint16_t));
                        index_address = error_info.geom.triangles.indexData.deviceAddress +
                                        error_info.build_range_info.primitiveOffset + u32_diff_byte_offset +
                                        diff_byte_i / sizeof(uint16_t);
                        update_time_index =
                            (update_time_index_dword >> ((diff_byte_i / sizeof(uint16_t)) * 8 * sizeof(uint16_t))) & 0xffff;
                        build_time_index =
                            (build_time_index_dword >> ((diff_byte_i / sizeof(uint16_t)) * 8 * sizeof(uint16_t))) & 0xffff;
                        break;
                    }
                    case VK_INDEX_TYPE_UINT32: {
                        index_buffer_pos = u32_diff_byte_offset / sizeof(uint32_t);
                        index_address = error_info.geom.triangles.indexData.deviceAddress +
                                        error_info.build_range_info.primitiveOffset + u32_diff_byte_offset;
                        update_time_index = update_time_index_dword;
                        build_time_index = build_time_index_dword;
                        break;
                    }
                    default:
                        break;
                }

                skip |= gpuav.LogError(
                    "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03768", objlist, loc_with_debug_region,
                    "Index buffer value(s) updated between acceleration structure build and update.\n"
                    "Index type is %s. At offset %" PRIu32 " index supplied at acceleration structure build time was %" PRIu32
                    ", but at update time index at same offset is %" PRIu32 " (found at VkDeviceAddress: 0x%" PRIx64
                    ").\n"
                    "Updated index buffer starts at VkDeviceAddress indexData (0x%" PRIx64 ") + primitiveOffset (%" PRIu32
                    ") = 0x%" PRIx64
                    ".\n\n"

                    "Corresponding BLAS build command info:\n"
                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32
                    "] was:\n%s\n"

                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32 "]::VkAccelerationStructureGeometryKHR[%" PRIu32
                    "]::VkAccelerationStructureGeometryTrianglesDataKHR was:\n%s\n"

                    "VkAccelerationStructureBuildRangeInfoKHR[%" PRIu32 "][%" PRIu32 "] was:\n%s\n",

                    string_VkIndexType(error_info.geom.triangles.indexType), index_buffer_pos, build_time_index, update_time_index,
                    index_address, error_info.geom.triangles.indexData.deviceAddress, error_info.build_range_info.primitiveOffset,
                    error_info.geom.triangles.indexData.deviceAddress + error_info.build_range_info.primitiveOffset,

                    error_info.info_i, string_VkAccelerationStructureBuildGeometryInfoKHR(gpuav, error_info.info).c_str(),

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureGeometryTrianglesDataKHR(*gpuav.device_state, error_info.geom.triangles).c_str(),

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureBuildRangeInfoKHR(error_info.build_range_info).c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_VertexBufferActiveStatusUpdated: {
                const uint32_t index = error_record[kValCmd_ErrorPayloadDword_2];
                const uint32_t is_build_time_vertex_nan = error_record[kValCmd_ErrorPayloadDword_3];

                const char* vuid = is_build_time_vertex_nan ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03663"
                                                            : "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03664";
                const char* error_type = is_build_time_vertex_nan ? "inactive to active" : "active to inactive";
                skip |= gpuav.LogError(
                    vuid, objlist, loc_with_debug_region,
                    "Primitive status changed from %s between acceleration structure build and update. In terms of vertices of "
                    "format %s strided by %" PRIu64 " bytes, primitive was found at index %" PRIu32
                    " in vertex buffer. "
                    "Corresponding vertex buffer address is 0x%" PRIx64
                    ".\n\n"

                    "Corresponding BLAS build command info:\n"
                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32
                    "] was:\n%s\n"

                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32 "]::VkAccelerationStructureGeometryKHR[%" PRIu32
                    "]::VkAccelerationStructureGeometryTrianglesDataKHR was:\n%s\n"

                    "VkAccelerationStructureBuildRangeInfoKHR[%" PRIu32 "][%" PRIu32 "] was:\n%s\n",

                    error_type, string_VkFormat(error_info.geom.triangles.vertexFormat), error_info.geom.triangles.vertexStride,
                    index, error_info.geom.triangles.vertexData.deviceAddress + index * error_info.geom.triangles.vertexStride,

                    error_info.info_i, string_VkAccelerationStructureBuildGeometryInfoKHR(gpuav, error_info.info).c_str(),

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureGeometryTrianglesDataKHR(*gpuav.device_state, error_info.geom.triangles).c_str(),

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureBuildRangeInfoKHR(error_info.build_range_info).c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_AabbBufferActiveStatusUpdated: {
                const uint32_t is_build_time_aabb_nan = error_record[kValCmd_ErrorPayloadDword_2];

                const char* vuid = is_build_time_aabb_nan ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03663"
                                                          : "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03664";
                const char* error_type = is_build_time_aabb_nan ? "inactive to active" : "active to inactive";
                skip |= gpuav.LogError(
                    vuid, objlist, loc_with_debug_region,
                    "Primitive status changed from %s between acceleration structure build and update. AABB primitive at index "
                    "%" PRIu32 " strided by %" PRIu64 " bytes was found at VkDeviceAddress aabbs.data (0x%" PRIx64
                    ") + primitiveOffset (%" PRIu32 ") + primitive index (%" PRIu32 ") * stride (%" PRIu64 ") = 0x%" PRIx64
                    ".\n\n"

                    "Corresponding BLAS build command info:\n"
                    "VkAccelerationStructureBuildGeometryInfoKHR[%" PRIu32 "]::VkAccelerationStructureGeometryKHR[%" PRIu32
                    "]::VkAccelerationStructureGeometryAabbsDataKHR was:\n%s\n"

                    "VkAccelerationStructureBuildRangeInfoKHR[%" PRIu32 "][%" PRIu32 "] was:\n%s\n",

                    error_type, gid, error_info.geom.aabbs.stride, error_info.geom.aabbs.data.deviceAddress,
                    error_info.build_range_info.primitiveOffset, gid, error_info.geom.aabbs.stride,
                    error_info.geom.aabbs.data.deviceAddress + error_info.build_range_info.primitiveOffset +
                        gid * error_info.geom.aabbs.stride,

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureGeometryAabbsDataKHR(*gpuav.device_state, error_info.geom.aabbs).c_str(),

                    error_info.info_i, error_info.geom_i,
                    string_VkAccelerationStructureBuildRangeInfoKHR(error_info.build_range_info).c_str());
                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.AddCommandErrorLogger(loc, &last_bound, std::move(error_logger));
}

}  // namespace valcmd
}  // namespace gpuav
