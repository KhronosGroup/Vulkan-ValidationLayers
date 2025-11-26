/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "chassis/chassis_modification_state.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_error_header.h"
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

    if (!gpuav.modified_features.shaderInt64) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    ValidationCommandsCommon& val_cmd_common =
        cb_state.shared_resources_cache.GetOrCreate<ValidationCommandsCommon>(gpuav, cb_state, loc);
    valpipe::ComputePipeline<TraceRaysValidationShader>& validation_pipeline =
        gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<TraceRaysValidationShader>>(
            gpuav, loc, val_cmd_common.error_logging_desc_set_layout_);
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

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav](const uint32_t* error_record,
                                                                   const Location& loc_with_debug_region,
                                                                   const LogObjectList& objlist) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
        if (error_group != kErrorGroupGpuPreTraceRays) {
            return skip;
        }

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCodePreTraceRaysLimitWidth: {
                const uint32_t width = error_record[kValCmdErrorPayloadDword_0];
                skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-width-03638", objlist, loc_with_debug_region,
                                       "Indirect trace rays of VkTraceRaysIndirectCommandKHR::width of %" PRIu32
                                       " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] * "
                                       "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0] limit of %" PRIu64 ".",
                                       width,
                                       static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[0]));
                break;
            }
            case kErrorSubCodePreTraceRaysLimitHeight: {
                const uint32_t height = error_record[kValCmdErrorPayloadDword_0];
                skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-height-03639", objlist, loc_with_debug_region,
                                       "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                                       " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] * "
                                       "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1] limit of %" PRIu64 ".",
                                       height,
                                       static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[1]));
                break;
            }
            case kErrorSubCodePreTraceRaysLimitDepth: {
                const uint32_t depth = error_record[kValCmdErrorPayloadDword_0];
                skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-depth-03640", objlist, loc_with_debug_region,
                                       "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                                       " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] * "
                                       "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2] limit of %" PRIu64 ".",
                                       depth,
                                       static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[2]));
                break;
            }
            case kErrorSubCodePreTraceRaysLimitVolume: {
                const VkExtent3D trace_rays_extent = {error_record[kValCmdErrorPayloadDword_0],
                                                      error_record[kValCmdErrorPayloadDword_1],
                                                      error_record[kValCmdErrorPayloadDword_2]};
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
    static size_t GetSpirvSize() { return validation_cmd_build_acceleration_structures_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return validation_cmd_build_acceleration_structures_comp; }

    glsl::AccelerationStructureReferencePushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

struct AccelerationStructureMetadata {
    uint32_t address_low;
    uint32_t address_high;
    uint32_t buffer_status;
};

struct AccelerationStructuresAddrToStateObjectMap {
    vvl::concurrent_unordered_map<VkDeviceAddress, std::shared_ptr<vvl::AccelerationStructureKHR>> map;
};

void RecordGetAccelerationStructureDeviceAddress(Validator& gpuav, VkAccelerationStructureKHR as, VkDeviceAddress as_addr) {
    if (!gpuav.gpuav_settings.validate_acceleration_structures_builds) {
        return;
    }

    if (as_addr == 0) {
        return;
    }

    if (auto as_state = gpuav.Get<vvl::AccelerationStructureKHR>(as)) {
        as_state->acceleration_structure_address = as_addr;
        auto& as_addr_to_as_buffer = gpuav.shared_resources_manager.GetOrCreate<AccelerationStructuresAddrToStateObjectMap>();
        as_addr_to_as_buffer.map.insert(as_addr, as_state);
    }
}

void RemoveAccelerationStrutureDeviceAddress(Validator& gpuav, VkAccelerationStructureKHR as) {
    if (!gpuav.gpuav_settings.validate_acceleration_structures_builds) {
        return;
    }

    if (auto as_state = gpuav.Get<vvl::AccelerationStructureKHR>(as)) {
        if (as_state->acceleration_structure_address != 0) {
            auto* as_addr_to_as_buffer = gpuav.shared_resources_manager.TryGet<AccelerationStructuresAddrToStateObjectMap>();
            if (as_addr_to_as_buffer) {
                as_addr_to_as_buffer->map.erase(as_state->acceleration_structure_address);
                as_state->acceleration_structure_address = 0;
            }
        }
    }
}

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
            const bool success = vertex_buffer.Create(&vertex_buffer_ci, &alloc_ci);
            if (!success) {
                gpuav.InternalError(LogObjectList(), Location(vvl::Func::Empty), "Failed to create dummy BLAS's vertex buffer.");
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
            const bool success = transform_buffer.Create(&transform_buffer_ci, &alloc_ci);
            if (!success) {
                gpuav.InternalError(LogObjectList(), Location(vvl::Func::Empty), "Failed to create dummy BLAS's transform buffer.");
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
            const bool success = scratch_buffer.Create(&scratch_buffer_ci, &alloc_ci);
            if (!success) {
                gpuav.InternalError(LogObjectList(), Location(vvl::Func::Empty), "Failed to create dummy BLAS's scratch buffer.");
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
            const bool success = blas_buffer.Create(&blas_buffer_ci, &alloc_ci);
            if (!success) {
                gpuav.InternalError(LogObjectList(), Location(vvl::Func::Empty), "Failed to create dummy BLAS buffer.");
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

void BuildAccelerationStructures(Validator& gpuav, const Location& loc, CommandBufferSubState& cb_state,
                                 const LastBound& last_bound, uint32_t info_count,
                                 const VkAccelerationStructureBuildGeometryInfoKHR* infos,
                                 const VkAccelerationStructureBuildRangeInfoKHR* const* build_ranges_infos) {
    VVL_ZoneScoped;
    if (!gpuav.gpuav_settings.validate_acceleration_structures_builds) {
        return;
    }

    if (!gpuav.modified_features.shaderInt64) {
        return;
    }

    struct BlasArray {
        VkDeviceAddress array_start_addr = 0;
        uint32_t size = 0;
        uint32_t is_array_of_pointers = 0;
        uint32_t info_i = 0;
        uint32_t geom_i = 0;
    };

    std::vector<BlasArray> blas_arrays;
    for (const auto [info_i, info] : vvl::enumerate(infos, info_count)) {
        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& geom = rt::GetGeometry(info, geom_i);
            if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                BlasArray blas_array;
                blas_array.size = build_ranges_infos[info_i][geom_i].primitiveCount;
                blas_array.array_start_addr =
                    geom.geometry.instances.data.deviceAddress + build_ranges_infos[info_i][geom_i].primitiveOffset;
                blas_array.is_array_of_pointers = uint32_t(geom.geometry.instances.arrayOfPointers);
                blas_array.info_i = info_i;
                blas_array.geom_i = geom_i;
                blas_arrays.emplace_back(blas_array);
            }
        }
    }

    if (blas_arrays.empty()) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    ValidationCommandsCommon& val_cmd_common =
        cb_state.shared_resources_cache.GetOrCreate<ValidationCommandsCommon>(gpuav, cb_state, loc);
    valpipe::ComputePipeline<BuildAccelerationStructuresValidationShader>& validation_pipeline =
        gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<BuildAccelerationStructuresValidationShader>>(
            gpuav, loc, val_cmd_common.error_logging_desc_set_layout_);
    if (!validation_pipeline.valid) {
        return;
    }

    vko::BufferRange ptr_to_accel_structs_metadata_buffer =
        cb_state.gpu_resources_manager.GetDeviceLocalBufferRange(sizeof(VkDeviceAddress));

    DummyBLAS& dummy_blas = gpuav.shared_resources_manager.GetOrCreate<DummyBLAS>(gpuav, cb_state);

    BuildAccelerationStructuresValidationShader shader_resources;
    shader_resources.push_constants.ptr_to_ptr_to_accel_structs_metadata = ptr_to_accel_structs_metadata_buffer.offset_address;
    shader_resources.push_constants.valid_dummy_blas_addr = dummy_blas.blas_address;

    cb_state.on_pre_cb_submission_functions.emplace_back(
        [ptr_to_accel_structs_metadata_buffer](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
            VVL_ZoneScopedN("validate_as_builds_pre_submit");
            // #ARNO_TODO Refacto the "copy to buffer" part
            auto& as_addr_to_as_buffer = gpuav.shared_resources_manager.Get<AccelerationStructuresAddrToStateObjectMap>();
            // #ARNO_TODO Definitely can see this become a big perf bottleneck
            auto as_addr_to_as_buffer_snapshot = as_addr_to_as_buffer.map.snapshot();

            vko::BufferRange accel_structs_metadata_buffer = cb.gpu_resources_manager.GetHostCoherentBufferRange(
                as_addr_to_as_buffer_snapshot.size() * sizeof(AccelerationStructureMetadata));
            auto accel_structs_metadata_buffer_u32_ptr = (uint32_t*)accel_structs_metadata_buffer.offset_mapped_ptr;

            *accel_structs_metadata_buffer_u32_ptr = (uint32_t)as_addr_to_as_buffer_snapshot.size();

            auto as_metadata_ptr = (AccelerationStructureMetadata*)(accel_structs_metadata_buffer_u32_ptr + 1);
            uint32_t written_count = 0;
            for (const auto& [device_addr, as] : as_addr_to_as_buffer_snapshot) {
                as_metadata_ptr[written_count++] = {uint32_t(device_addr), uint32_t(device_addr >> 32u),
                                                    uint32_t(as->buffer_state && !as->buffer_state->Destroyed())};
            }

            // Fill a GPU buffer with a pointer to the AS metadata
            vko::BufferRange submit_time_ptr_to_accel_structs_metadata_buffer =
                cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(VkDeviceAddress));
            *(VkDeviceAddress*)submit_time_ptr_to_accel_structs_metadata_buffer.offset_mapped_ptr =
                accel_structs_metadata_buffer.offset_address;

            // Dispatch a copy command, copying the per CB submission AS metadata pointer to the AS metadata pointer created at
            // build acceleration structures time, so that CB submission accesses correct AS metadata snapshot.
            {
                VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
                barrier_write_after_read.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier_write_after_read.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_write_after_read.buffer = ptr_to_accel_structs_metadata_buffer.buffer;
                barrier_write_after_read.offset = ptr_to_accel_structs_metadata_buffer.offset;
                barrier_write_after_read.size = ptr_to_accel_structs_metadata_buffer.size;

                DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                           nullptr);

                VkBufferCopy copy;
                copy.srcOffset = submit_time_ptr_to_accel_structs_metadata_buffer.offset;
                copy.dstOffset = ptr_to_accel_structs_metadata_buffer.offset;
                copy.size = sizeof(VkDeviceAddress);
                DispatchCmdCopyBuffer(per_submission_cb, submit_time_ptr_to_accel_structs_metadata_buffer.buffer,
                                      ptr_to_accel_structs_metadata_buffer.buffer, 1, &copy);

                VkBufferMemoryBarrier barrier_read_before_write = vku::InitStructHelper();
                barrier_read_before_write.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_read_before_write.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier_read_before_write.buffer = ptr_to_accel_structs_metadata_buffer.buffer;
                barrier_read_before_write.offset = ptr_to_accel_structs_metadata_buffer.offset;
                barrier_read_before_write.size = ptr_to_accel_structs_metadata_buffer.size;

                DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                                           0, nullptr, 1, &barrier_read_before_write, 0, nullptr);
            }
        });

    // Setup Validation pipeline
    // ---
    {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        // Validation dispatch, one for each TLAS build
        // ---
        for (size_t blas_array_i = 0; blas_array_i < blas_arrays.size(); ++blas_array_i) {
            const auto blas_array_buffers = gpuav.GetBuffersByAddress(blas_arrays[blas_array_i].array_start_addr);
            if (blas_array_buffers.empty()) {
                assert(false);
            } else {
                VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
                barrier_write_after_read.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                barrier_write_after_read.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier_write_after_read.buffer = blas_array_buffers[0]->VkHandle();
                barrier_write_after_read.offset = 0;
                barrier_write_after_read.size = VK_WHOLE_SIZE;

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                           nullptr);
            }

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

            const uint32_t as_instances_count = blas_arrays[blas_array_i].size;
            const uint32_t work_group_count = as_instances_count / 32 + uint32_t(as_instances_count % 32 > 0);
            DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);

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

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, blas_arrays = std::move(blas_arrays)](
                                                              const uint32_t* error_record, const Location& loc_with_debug_region,
                                                              const LogObjectList& objlist) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
        if (error_group != kErrorGroupGpuPreBuildAccelerationStructures) {
            return skip;
        }

        const uint64_t accel_struct_addr = glsl::GetUint64(error_record + kValCmdErrorPayloadDword_0);
        const uint32_t as_instance_i = error_record[kValCmdErrorPayloadDword_2];
        const uint32_t blas_array_i = error_record[kValCmdErrorPayloadDword_3];

        const BlasArray blas_array = blas_arrays[blas_array_i];
        std::stringstream invalid_blas_loc;
        invalid_blas_loc << "pInfos[" << blas_array.info_i << "].pGeometries[" << blas_array.geom_i
                         << "].geometry.instances<VkAccelerationStructureInstance" << (blas_array.is_array_of_pointers ? " *" : "")
                         << ">[" << as_instance_i << "] (0x" << std::hex << accel_struct_addr
                         << ") is an invalid acceleration structure reference";
        const std::string invalid_blas_loc_str = invalid_blas_loc.str();

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCode_PreBuildAccelerationStructures_InvalidAS: {
                skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-06707", objlist, loc_with_debug_region,
                                       "%s.", invalid_blas_loc_str.c_str());
                break;
            }
            case kErrorSubCode_PreBuildAccelerationStructures_DestroyedASBuffer: {
                auto& as_addr_to_as_buffer = gpuav.shared_resources_manager.Get<AccelerationStructuresAddrToStateObjectMap>();
                auto found_as = as_addr_to_as_buffer.map.find(accel_struct_addr);
                std::stringstream ss;
                if (found_as != as_addr_to_as_buffer.map.end()) {
                    ss << "Corresponding acceleration structure: " << gpuav.FormatHandle(found_as->second->VkHandle());
                } else {
                    ss << "Could not map acceleration structure reference to its corresponding handle, this is most likely a "
                          "validation bug.";
                }
                const std::string ss_str = ss.str();
                skip |=
                    gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-06707", objlist, loc_with_debug_region,
                                   "%s - underlying buffer has been destroyed. %s", invalid_blas_loc_str.c_str(), ss_str.c_str());
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
