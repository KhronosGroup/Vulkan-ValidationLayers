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
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/validation_cmd/push_data.h"
#include "gpuav/shaders/validation_cmd/build_acceleration_structures.h"
#include "generated/gpuav_offline_spirv.h"
#include "error_message/error_strings.h"
#include "containers/limits.h"
#include "utils/math_utils.h"
#include "utils/ray_tracing_utils.h"

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

struct AccelerationStructuresAddrToBufferMap {
    vvl::concurrent_unordered_map<VkDeviceAddress, std::shared_ptr<vvl::Buffer>> map;
};

void RecordGetAccelerationStructureDeviceAddress(Validator& gpuav, VkAccelerationStructureKHR as, VkDeviceAddress as_addr) {
    if (!gpuav.gpuav_settings.validate_build_acceleration_structures) {
        return;
    }

    auto& as_addr_to_as_buffer = gpuav.shared_resources_manager.GetOrCreate<AccelerationStructuresAddrToBufferMap>();

    if (as_addr == 0) {
        return;
    }
    if (auto as_state = gpuav.Get<vvl::AccelerationStructureKHR>(as)) {
        as_state->acceleration_structure_address = as_addr;
        as_addr_to_as_buffer.map.insert(as_addr, as_state->buffer_state);
    }
}

void RemoveAccelerationStrutureDeviceAddress(Validator& gpuav, VkAccelerationStructureKHR as) {
    if (!gpuav.gpuav_settings.validate_build_acceleration_structures) {
        return;
    }

    if (auto as_state = gpuav.Get<vvl::AccelerationStructureKHR>(as)) {
        if (as_state->acceleration_structure_address != 0) {
            auto* as_addr_to_as_buffer = gpuav.shared_resources_manager.TryGet<AccelerationStructuresAddrToBufferMap>();
            if (as_addr_to_as_buffer) {
                as_addr_to_as_buffer->map.erase(as_state->acceleration_structure_address);
                as_state->acceleration_structure_address = 0;
            }
        }
    }
}

void BuildAccelerationStructures(Validator& gpuav, const Location& loc, CommandBufferSubState& cb_state,
                                 const LastBound& last_bound, uint32_t info_count,
                                 const VkAccelerationStructureBuildGeometryInfoKHR* infos,
                                 const VkAccelerationStructureBuildRangeInfoKHR* const* build_ranges_infos,
                                 chassis::BuildAccelerationStructures& chassis_state) {
    if (!gpuav.gpuav_settings.validate_build_acceleration_structures) {
        return;
    }

    if (!gpuav.modified_features.shaderInt64) {
        return;
    }

    struct BlasArray {
        uint32_t flattened_build_range_start_i = 0;
        uint32_t count = 0;
        VkDeviceAddress array_start_addr = 0;
    };

    uint32_t geometry_counts_accum = 0;
    std::vector<BlasArray> blas_arrays;
    for (const auto [info_i, info] : vvl::enumerate(infos, info_count)) {
        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& geom = rt::GetGeometry(info, geom_i);
            if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                // #ARNO_TODO handle arrayOfPointers
                if (!geom.geometry.instances.arrayOfPointers) {
                    BlasArray blas_array;
                    blas_array.flattened_build_range_start_i = geometry_counts_accum + geom_i;
                    blas_array.count = build_ranges_infos[info_i][geom_i].primitiveCount;
                    blas_array.array_start_addr =
                        geom.geometry.instances.data.deviceAddress + build_ranges_infos[info_i][geom_i].primitiveOffset;
                    blas_arrays.emplace_back(blas_array);
                }
            }
        }

        geometry_counts_accum += info.geometryCount;
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

    chassis_state.app_dispatch = false;

    vko::BufferRange ptr_to_accel_structs_metadata_buffer =
        cb_state.gpu_resources_manager.GetDeviceLocalBufferRange(sizeof(VkDeviceAddress));
    BuildAccelerationStructuresValidationShader shader_resources;
    shader_resources.push_constants.ptr_to_ptr_to_accel_structs_metadata = ptr_to_accel_structs_metadata_buffer.offset_address;

    cb_state.on_pre_cb_submission_functions.emplace_back(
        [ptr_to_accel_structs_metadata_buffer](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
            // #ARNO_TODO Refacto the "copy to buffer" part
            auto* as_addr_to_as_buffer = gpuav.shared_resources_manager.TryGet<AccelerationStructuresAddrToBufferMap>();
            if (!as_addr_to_as_buffer) {
                assert(false);
                gpuav.InternalError(LogObjectList(), Location(vvl::Func::vkQueueSubmit),
                                    "Failed to get AccelerationStructuresAddrToBufferMap.");
                return;
            }
            // #ARNO_TODO Definitely can see this become a big perf bottleneck
            auto as_addr_to_as_buffer_snapshot = as_addr_to_as_buffer->map.snapshot();

            vko::BufferRange accel_structs_metadata_buffer = cb.gpu_resources_manager.GetHostCoherentBufferRange(
                as_addr_to_as_buffer_snapshot.size() * sizeof(AccelerationStructureMetadata));
            auto accel_structs_metadata_buffer_u32_ptr = (uint32_t*)accel_structs_metadata_buffer.offset_mapped_ptr;

            *accel_structs_metadata_buffer_u32_ptr = (uint32_t)as_addr_to_as_buffer_snapshot.size();

            auto as_metadata_ptr = (AccelerationStructureMetadata*)(accel_structs_metadata_buffer_u32_ptr + 1);
            uint32_t written_count = 0;
            for (const auto& [device_addr, as_buffer] : as_addr_to_as_buffer_snapshot) {
                as_metadata_ptr[written_count++] = {uint32_t(device_addr), uint32_t(device_addr >> 32u),
                                                    uint32_t(as_buffer && !as_buffer->Destroyed())};
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

    // Setup buffer for build acceleration structures build command into an indirect one.
    // Doing so allows to modify invalid build instructions on the GPU when they are detected,
    // preventing a GPU crash.
    // ---
    {
        std::vector<VkDeviceAddress> indirect_device_addresses;
        std::vector<uint32_t> indirect_strides;
        std::vector<std::vector<uint32_t>> max_primitive_countss;
        std::vector<uint32_t*> max_primitive_countss_pointers;

        std::vector<uint32_t> geometry_counts(info_count);
        for (const auto& [info_i, info] : vvl::enumerate(infos, info_count)) {
            geometry_counts[info_i] = info.geometryCount;
        }
        VkDeviceSize indirect_buffer_size = 0;
        for (uint32_t geom_count : geometry_counts) {
            // Given sizeof(VkAccelerationStructureBuildRangeInfoKHR) == 16, don't worry about being aligned to 4
            static_assert(sizeof(VkAccelerationStructureBuildRangeInfoKHR) == 16);
            indirect_buffer_size += geom_count * sizeof(VkAccelerationStructureBuildRangeInfoKHR);
        }

        // Turn build command into an indirect one, to be able to remove bad AS references on the GPU
        // ---
        vko::BufferRange build_indirect_buffer =
            cb_state.gpu_resources_manager.GetHostCoherentIndirectBufferRange(indirect_buffer_size);
        VkDeviceAddress indirect_buffer_bda = build_indirect_buffer.offset_address;

        auto indirect_buffer_ptr = (VkAccelerationStructureBuildRangeInfoKHR*)build_indirect_buffer.offset_mapped_ptr;
        for (const auto& [build_ranges_info_i, build_ranges_info] : vvl::enumerate(build_ranges_infos, info_count)) {
            std::memcpy(indirect_buffer_ptr, build_ranges_info,
                        geometry_counts[build_ranges_info_i] * sizeof(VkAccelerationStructureBuildRangeInfoKHR));
            indirect_device_addresses.emplace_back(indirect_buffer_bda);

            indirect_buffer_ptr += geometry_counts[build_ranges_info_i];
            indirect_buffer_bda += geometry_counts[build_ranges_info_i] * sizeof(VkAccelerationStructureBuildRangeInfoKHR);

            std::vector<uint32_t> max_primitive_counts(geometry_counts[build_ranges_info_i]);
            for (size_t geom_i = 0; geom_i < geometry_counts[build_ranges_info_i]; ++geom_i) {
                max_primitive_counts[geom_i] = build_ranges_info[geom_i].primitiveCount;
            }
            max_primitive_countss.emplace_back(std::move(max_primitive_counts));
        }

        max_primitive_countss_pointers.reserve(max_primitive_countss.size());
        for (std::vector<uint32_t>& max_primitive_count : max_primitive_countss) {
            max_primitive_countss_pointers.emplace_back(max_primitive_count.data());
        }

        indirect_strides = std::vector<uint32_t>(info_count, sizeof(VkDeviceAddress));

        assert((uint8_t*)indirect_buffer_ptr == ((uint8_t*)build_indirect_buffer.offset_mapped_ptr + indirect_buffer_size));

        shader_resources.push_constants.ptr_to_accel_struct_build_range_infos = build_indirect_buffer.offset_address;

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        // Sync indirect buffer writes - the same command buffer could be executed concurrently
        // for all we know
        {
            VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
            barrier_write_after_read.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            barrier_write_after_read.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier_write_after_read.buffer = build_indirect_buffer.buffer;
            barrier_write_after_read.offset = build_indirect_buffer.offset;
            barrier_write_after_read.size = build_indirect_buffer.size;

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                       nullptr);
        }

        // Validation dispatch, one for each TLAS build
        // ---
        for (size_t tlas_build_i = 0; tlas_build_i < blas_arrays.size(); ++tlas_build_i) {
            shader_resources.push_constants.blas_array_start_addr = blas_arrays[tlas_build_i].array_start_addr;
            shader_resources.push_constants.flattened_build_range_i = blas_arrays[tlas_build_i].flattened_build_range_start_i;

            const bool bind_error_logging_desc_set = tlas_build_i == 0;
            if (!BindShaderResources(validation_pipeline, gpuav, cb_state, cb_state.compute_index, cb_state.GetErrorLoggerIndex(),
                                     shader_resources, bind_error_logging_desc_set)) {
                return;
            }

            const uint32_t as_instances_count = blas_arrays[tlas_build_i].count;
            const uint32_t work_group_count = as_instances_count / 32 + uint32_t(as_instances_count % 32 > 0);
            DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);
        }

        // Sync indirect buffer reads
        {
            VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
            barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier_read_after_write.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            barrier_read_after_write.buffer = build_indirect_buffer.buffer;
            barrier_read_after_write.offset = build_indirect_buffer.offset;
            barrier_read_after_write.size = build_indirect_buffer.size;

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1,
                                       &barrier_read_after_write, 0, nullptr);
        }

        DispatchCmdBuildAccelerationStructuresIndirectKHR(cb_state.VkHandle(), info_count, infos, indirect_device_addresses.data(),
                                                          indirect_strides.data(), max_primitive_countss_pointers.data());
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger =
        [&gpuav](const uint32_t* error_record, const Location& loc_with_debug_region, const LogObjectList& objlist) {
            bool skip = false;
            using namespace glsl;

            const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
            if (error_group != kErrorGroupGpuPreBuildAccelerationStructures) {
                return skip;
            }

            const uint64_t accel_struct_addr_low = error_record[kValCmdErrorPayloadDword_0];
            const uint64_t accel_struct_addr_high = error_record[kValCmdErrorPayloadDword_1];
            const uint64_t accel_struct_addr = accel_struct_addr_low | (accel_struct_addr_high << 32u);

            const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
            switch (error_sub_code) {
                case kErrorSubCode_PreBuildAccelerationStructures_InvalidAS: {
                    skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-06707", objlist, loc_with_debug_region,
                                           "Invalid acceleration structure reference: 0x%" PRIx64 "", accel_struct_addr);
                    break;
                }
                case kErrorSubCode_PreBuildAccelerationStructures_DestroyedASBuffer: {
                    skip |= gpuav.LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-06707", objlist, loc_with_debug_region,
                                           "Invalid acceleration structure reference: 0x%" PRIx64
                                           " - underlying buffer has been destroyed.",
                                           accel_struct_addr);
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
