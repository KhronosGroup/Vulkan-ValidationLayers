/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <assert.h>
#include <string>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>
#include "core_validation.h"
#include "core_checks/cc_state_tracker.h"
#include "cc_buffer_address.h"
#include "error_message/logging.h"
#include "utils/math_utils.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/pipeline_state.h"
#include "containers/container_utils.h"

bool CoreChecks::PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkAccelerationStructureNV* pAccelerationStructure,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    if (pCreateInfo != nullptr && pCreateInfo->info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pCreateInfo->info.geometryCount; i++) {
            skip |= ValidateGeometryNV(pCreateInfo->info.pGeometries[i],
                                       error_obj.location.dot(Field::pCreateInfo).dot(Field::info).dot(Field::pGeometries, i));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const Location bind_info_loc = error_obj.location.dot(Field::pBindInfos, i);
        const VkBindAccelerationStructureMemoryInfoNV& info = pBindInfos[i];
        auto as_state = Get<vvl::AccelerationStructureNV>(info.accelerationStructure);
        ASSERT_AND_CONTINUE(as_state);

        if (as_state->HasFullRangeBound()) {
            skip |= LogError("VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620", info.accelerationStructure,
                             bind_info_loc.dot(Field::accelerationStructure), "must not already be backed by a memory object.");
        }

        // Validate bound memory range information
        auto mem_info = Get<vvl::DeviceMemory>(info.memory);
        if (mem_info) {
            skip |=
                ValidateInsertMemoryRange(VulkanTypedHandle(info.accelerationStructure, kVulkanObjectTypeAccelerationStructureNV),
                                          *mem_info, info.memoryOffset, bind_info_loc);
            skip |= ValidateMemoryTypes(*mem_info, as_state->memory_requirements.memoryTypeBits,
                                        bind_info_loc.dot(Field::accelerationStructure),
                                        "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
        }

        // Validate memory requirements alignment
        if (!IsIntegerMultipleOf(info.memoryOffset, as_state->memory_requirements.alignment)) {
            skip |= LogError("VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623", info.accelerationStructure,
                             bind_info_loc.dot(Field::memoryOffset),
                             "(%" PRIu64 ") must be a multiple of the alignment (%" PRIu64
                             ") member of the VkMemoryRequirements structure returned from "
                             "a call to vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure %s and type of "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV",
                             info.memoryOffset, as_state->memory_requirements.alignment,
                             FormatHandle(info.accelerationStructure).c_str());
        }

        if (mem_info) {
            // Validate memory requirements size
            if (as_state->memory_requirements.size > (mem_info->allocate_info.allocationSize - info.memoryOffset)) {
                skip |= LogError("VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624", info.accelerationStructure,
                                 bind_info_loc.dot(Field::memory),
                                 "required size (%" PRIu64 ") minus %s (%" PRIu64 ") is %" PRIu64
                                 ", but the size member of the VkMemoryRequirements structure returned from a call to "
                                 "vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure %s and type of "
                                 "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV is %" PRIu64 ".",
                                 as_state->memory_requirements.size, bind_info_loc.dot(Field::memoryOffset).Fields().c_str(),
                                 info.memoryOffset, mem_info->allocate_info.allocationSize - info.memoryOffset,
                                 FormatHandle(info.accelerationStructure).c_str(), as_state->memory_requirements.size);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                 size_t dataSize, void* pData, const ErrorObject& error_obj) const {
    bool skip = false;

    if (auto as_state = Get<vvl::AccelerationStructureNV>(accelerationStructure)) {
        skip |= VerifyBoundMemoryIsValid(as_state->MemoryState(), LogObjectList(accelerationStructure), as_state->Handle(),
                                         error_obj.location.dot(Field::accelerationStructure),
                                         "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (pInfo != nullptr && pInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
            skip |= ValidateGeometryNV(pInfo->pGeometries[i], error_obj.location.dot(Field::pInfo).dot(Field::pGeometries, i));
        }
    }

    if (pInfo != nullptr && pInfo->geometryCount > phys_dev_ext_props.ray_tracing_props_nv.maxGeometryCount) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241", commandBuffer, error_obj.location.dot(Field::pInfo).dot(Field::geometryCount),
                         "geometryCount [%" PRIu32
                         "] must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.",
                         pInfo->geometryCount);
    }

    auto dst_as_state = Get<vvl::AccelerationStructureNV>(dst);
    auto src_as_state = Get<vvl::AccelerationStructureNV>(src);

    if (dst_as_state && pInfo) {
        if (dst_as_state->create_info.info.type != pInfo->type) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::type"
                             "[%s] must be identical to build info VkAccelerationStructureInfoNV::type [%s].",
                             string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.info.type),
                             string_VkAccelerationStructureTypeKHR(pInfo->type));
        }
        if (dst_as_state->create_info.info.flags != pInfo->flags) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::flags"
                             "[%s] must be identical to build info VkAccelerationStructureInfoNV::flags [%s].",
                             string_VkBuildAccelerationStructureFlagsKHR(dst_as_state->create_info.info.flags).c_str(),
                             string_VkBuildAccelerationStructureFlagsKHR(pInfo->flags).c_str());
        }
        if (dst_as_state->create_info.info.instanceCount < pInfo->instanceCount) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::instanceCount "
                             "[%" PRIu32
                             "] must be greater than or equal to build info VkAccelerationStructureInfoNV::instanceCount [%" PRIu32
                             "].",
                             dst_as_state->create_info.info.instanceCount, pInfo->instanceCount);
        }
        if (dst_as_state->create_info.info.geometryCount < pInfo->geometryCount) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::geometryCount"
                             "[%" PRIu32
                             "] must be greater than or equal to build info VkAccelerationStructureInfoNV::geometryCount [%" PRIu32
                             "].",
                             dst_as_state->create_info.info.geometryCount, pInfo->geometryCount);
        } else {
            for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
                const VkGeometryDataNV& create_geometry_data = dst_as_state->create_info.info.pGeometries[i].geometry;
                const VkGeometryDataNV& build_geometry_data = pInfo->pGeometries[i].geometry;
                if (create_geometry_data.triangles.vertexCount < build_geometry_data.triangles.vertexCount) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                                     "create info pGeometries[%" PRIu32 "].geometry.triangles.vertexCount [%" PRIu32
                                     "]"
                                     "must be greater than or equal to build info pGeometries[%" PRIu32
                                     "].geometry.triangles.vertexCount [%" PRIu32 "].",
                                     i, create_geometry_data.triangles.vertexCount, i, build_geometry_data.triangles.vertexCount);
                    break;
                }
                if (create_geometry_data.triangles.indexCount < build_geometry_data.triangles.indexCount) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                                     "create info pGeometries[%" PRIu32 "].geometry.triangles.indexCount [%" PRIu32
                                     "]"
                                     "must be greater than or equal to build info pGeometries[%" PRIu32
                                     "].geometry.triangles.indexCount [%" PRIu32 "].",
                                     i, create_geometry_data.triangles.indexCount, i, build_geometry_data.triangles.indexCount);
                    break;
                }
                if (create_geometry_data.aabbs.numAABBs < build_geometry_data.aabbs.numAABBs) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                                     "create info pGeometries[%" PRIu32 "].geometry.aabbs.numAABBs [%" PRIu32
                                     "]"
                                     "must be greater than or equal to build info pGeometries[%" PRIu32
                                     "].geometry.aabbs.numAABBs [%" PRIu32 "].",
                                     i, create_geometry_data.aabbs.numAABBs, i, build_geometry_data.aabbs.numAABBs);
                    break;
                }
            }
        }
    }

    if (dst_as_state) {
        skip |= VerifyBoundMemoryIsValid(dst_as_state->MemoryState(), LogObjectList(commandBuffer, dst), dst_as_state->Handle(),
                                         error_obj.location.dot(Field::dst), "VUID-vkCmdBuildAccelerationStructureNV-dst-07787");
    }

    auto scratch_buffer_state = Get<vvl::Buffer>(scratch);
    if (update == VK_TRUE) {
        if (src == VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02489", commandBuffer, error_obj.location,
                             "If update is VK_TRUE, src must not be VK_NULL_HANDLE.");
        } else {
            if (!src_as_state || !src_as_state->built ||
                !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV)) {
                skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02490", commandBuffer, error_obj.location,
                                 "If update is VK_TRUE, src must have been built before "
                                 "with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV set in "
                                 "VkAccelerationStructureInfoNV::flags.");
            }
        }
        if (scratch_buffer_state && dst_as_state &&
            dst_as_state->update_scratch_memory_requirements.size > (scratch_buffer_state->create_info.size - scratchOffset)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02492", commandBuffer, error_obj.location,
                             "If update is VK_TRUE, The size member of the "
                             "VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV must be less than "
                             "or equal to the size of scratch minus scratchOffset");
        }
    } else {
        if (scratch_buffer_state && dst_as_state &&
            dst_as_state->build_scratch_memory_requirements.size > (scratch_buffer_state->create_info.size - scratchOffset)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02491", commandBuffer, error_obj.location,
                             "If update is VK_FALSE, The size member of the "
                             "VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV must be less than "
                             "or equal to the size of scratch minus scratchOffset");
        }
    }
    if (instanceData != VK_NULL_HANDLE) {
        if (auto buffer_state = Get<vvl::Buffer>(instanceData)) {
            skip |= ValidateBufferUsageFlags(
                LogObjectList(commandBuffer, instanceData), *buffer_state, VK_BUFFER_USAGE_2_RAY_TRACING_BIT_NV, true,
                "VUID-VkAccelerationStructureInfoNV-instanceData-02782", error_obj.location.dot(Field::instanceData));
        }
    }
    if (scratch_buffer_state) {
        skip |= ValidateBufferUsageFlags(
            LogObjectList(commandBuffer, scratch), *scratch_buffer_state, VK_BUFFER_USAGE_2_RAY_TRACING_BIT_NV, true,
            "VUID-VkAccelerationStructureInfoNV-scratch-02781", error_obj.location.dot(Field::scratch));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeNV mode,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);
    auto dst_as_state = Get<vvl::AccelerationStructureNV>(dst);
    auto src_as_state = Get<vvl::AccelerationStructureNV>(src);

    if (dst_as_state) {
        const LogObjectList objlist(commandBuffer, dst);
        skip |= VerifyBoundMemoryIsDeviceVisible(dst_as_state->MemoryState(), objlist, dst_as_state->Handle(),
                                                 error_obj.location.dot(Field::dst),
                                                 "VUID-vkCmdCopyAccelerationStructureNV-buffer-03719");
    }
    if (src_as_state) {
        const LogObjectList objlist(commandBuffer, src);
        skip |= VerifyBoundMemoryIsDeviceVisible(src_as_state->MemoryState(), objlist, src_as_state->Handle(),
                                                 error_obj.location.dot(Field::src),
                                                 "VUID-vkCmdCopyAccelerationStructureNV-buffer-03718");
    }

    if (mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV) {
        if (src_as_state &&
            (!src_as_state->built || !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV))) {
            skip |= LogError("VUID-vkCmdCopyAccelerationStructureNV-src-03411", commandBuffer, error_obj.location,
                             "src must have been built with "
                             "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV if mode is "
                             "VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV.");
        }
    }
    if (!(mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV || mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureNV-mode-03410", commandBuffer, error_obj.location,
                         "mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR"
                         "or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    if (auto as_state = Get<vvl::AccelerationStructureNV>(accelerationStructure)) {
        skip |= ValidateObjectNotInUse(as_state.get(), error_obj.location,
                                       "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    auto query_pool_state = Get<vvl::QueryPool>(queryPool);
    ASSERT_AND_RETURN_SKIP(query_pool_state);
    const auto& query_pool_ci = query_pool_state->create_info;
    if (query_pool_ci.queryType != queryType) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryPool-03755", commandBuffer,
                         error_obj.location.dot(Field::queryType),
                         "was created with %s which is differnent from the type queryPool was created with %s.",
                         string_VkQueryType(queryType), string_VkQueryType(query_pool_ci.queryType));
    }
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
            auto as_state = Get<vvl::AccelerationStructureNV>(pAccelerationStructures[i]);
            ASSERT_AND_CONTINUE(as_state);

            if (!(as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesNV-pAccelerationStructures-06215", commandBuffer,
                                 error_obj.location.dot(Field::pAccelerationStructures, i),
                                 "was built with %s, but queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR.",
                                 string_VkBuildAccelerationStructureFlagsKHR(as_state->build_info.flags).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV& triangles, const Location& loc) const {
    bool skip = false;

    auto vb_state = Get<vvl::Buffer>(triangles.vertexData);
    if (vb_state && vb_state->create_info.size <= triangles.vertexOffset) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-vertexOffset-02428", device, loc, "is invalid.");
    }

    auto ib_state = Get<vvl::Buffer>(triangles.indexData);
    if (ib_state && ib_state->create_info.size <= triangles.indexOffset) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-indexOffset-02431", device, loc, "is invalid.");
    }

    auto td_state = Get<vvl::Buffer>(triangles.transformData);
    if (td_state && td_state->create_info.size <= triangles.transformOffset) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-transformOffset-02437", device, loc, "is invalid.");
    }

    return skip;
}

bool CoreChecks::ValidateGeometryAABBNV(const VkGeometryAABBNV& aabbs, const Location& loc) const {
    bool skip = false;

    auto aabb_state = Get<vvl::Buffer>(aabbs.aabbData);
    if (aabb_state && aabb_state->create_info.size > 0 && aabb_state->create_info.size <= aabbs.offset) {
        skip |= LogError("VUID-VkGeometryAABBNV-offset-02439", device, loc, "is invalid.");
    }

    return skip;
}

bool CoreChecks::ValidateGeometryNV(const VkGeometryNV& geometry, const Location& loc) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip |= ValidateGeometryTrianglesNV(geometry.geometry.triangles, loc);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip |= ValidateGeometryAABBNV(geometry.geometry.aabbs, loc);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo,
    const ErrorObject& error_obj) const {
    bool skip = false;

    if (!enabled_features.partitionedAccelerationStructure) {
        skip |= LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-partitionedAccelerationStructure-10536", commandBuffer,
                         error_obj.location, "partitionedAccelerationStructure feature was not enabled.");
    }
    // Get build size info here for memory size check
    VkAccelerationStructureBuildSizesInfoKHR build_size_info = vku::InitStructHelper();
    const VkPartitionedAccelerationStructureInstancesInputNV input = pBuildInfo->input;
    DispatchGetPartitionedAccelerationStructuresBuildSizesNV(device, &input, &build_size_info);

    skip |= ValidateBuildPartitionedAccelerationStructureInfoNV(*pBuildInfo, error_obj.location.dot(Field::pBuildInfo),
                                                                build_size_info.buildScratchSize,
                                                                build_size_info.accelerationStructureSize);

    if (!IsPointerAligned(pBuildInfo->srcAccelerationStructureData, 256)) {
        skip |= LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10544", commandBuffer,
                         error_obj.location.dot(Field::pBuildInfo).dot(Field::srcAccelerationStructureData),
                         "(0x%" PRIx64 ") must be aligned to 256 bytes", pBuildInfo->srcAccelerationStructureData);
    }

    if (!IsPointerAligned(pBuildInfo->dstAccelerationStructureData, 256)) {
        skip |= LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10545", commandBuffer,
                         error_obj.location.dot(Field::pBuildInfo).dot(Field::dstAccelerationStructureData),
                         "(0x%" PRIx64 ") must be aligned to 256 bytes", pBuildInfo->dstAccelerationStructureData);
    }

    if (!IsPointerAligned(pBuildInfo->scratchData, 256)) {
        skip |= LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10542", commandBuffer,
                         error_obj.location.dot(Field::pBuildInfo).dot(Field::scratchData),
                         "(0x%" PRIx64 ") must be aligned to 256 bytes", pBuildInfo->scratchData);
    }

    {
        BufferAddressValidation<2> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10550",
               [](const vvl::Buffer& buffer_state) { return (buffer_state.usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0; },
               []() { return "The following buffers are missing VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT"; }, kUsageErrorMsgBuffer},
              {"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10541",
               [&build_size_info](const vvl::Buffer& buffer_state) {
                   return buffer_state.requirements.size < build_size_info.buildScratchSize;
               },
               [&build_size_info]() {
                   return "The buildScratchSize (" + std::to_string(build_size_info.buildScratchSize) +
                          ") does not fit in any buffer";
               },
               kEmptyErrorMsgBuffer}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, error_obj.location.dot(Field::pBuildInfo).dot(Field::scratchData), LogObjectList(commandBuffer),
            pBuildInfo->scratchData, build_size_info.buildScratchSize);
    }

    {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10551",
               [](const vvl::Buffer& buffer_state) {
                   return (buffer_state.usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
               },
               []() {
                   return "The following buffers are missing "
                          "VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
               },
               kUsageErrorMsgBuffer}}}};

        skip |=
            buffer_address_validator.ValidateDeviceAddress(*this, error_obj.location.dot(Field::pBuildInfo).dot(Field::srcInfos),
                                                           LogObjectList(commandBuffer), pBuildInfo->srcInfos);
    }

    {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10551",
               [](const vvl::Buffer& buffer_state) {
                   return (buffer_state.usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
               },
               []() {
                   return "The following buffers are missing "
                          "VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
               },
               kUsageErrorMsgBuffer}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(*this,
                                                               error_obj.location.dot(Field::pBuildInfo).dot(Field::srcInfosCount),
                                                               LogObjectList(commandBuffer), pBuildInfo->srcInfosCount);
    }

    {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10552",
               [](const vvl::Buffer& buffer_state) {
                   return (buffer_state.usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) == 0;
               },
               []() { return "The following buffers are missing VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR"; },
               kUsageErrorMsgBuffer}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, error_obj.location.dot(Field::pBuildInfo).dot(Field::srcAccelerationStructureData), LogObjectList(commandBuffer),
            pBuildInfo->srcAccelerationStructureData);
    }
    if (pBuildInfo->srcAccelerationStructureData && pBuildInfo->scratchData) {
        const auto src_buffer_states = GetBuffersByAddress(pBuildInfo->srcAccelerationStructureData);
        const auto scratch_buffer_states = GetBuffersByAddress(pBuildInfo->scratchData);
        for (const auto& scratch_buffer_state : scratch_buffer_states) {
            vvl::range<VkDeviceAddress> scratch_address_range = scratch_buffer_state->DeviceAddressRange();

            if (!scratch_address_range.empty()) {
                for (const auto& buffer_state : src_buffer_states) {
                    const vvl::range<VkDeviceAddress> buffer_address_range = buffer_state->DeviceAddressRange();
                    if (buffer_address_range.intersects(scratch_address_range)) {
                        const LogObjectList objlist(commandBuffer, buffer_state->Handle(), scratch_buffer_state->Handle());
                        skip |=
                            LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10547", objlist,
                                     error_obj.location.dot(Field::pBuildInfo).dot(Field::srcAccelerationStructureData),
                                     "%s address range %s intersects scratchData address range %s",
                                     FormatHandle(buffer_state->Handle()).c_str(), string_range_hex(buffer_address_range).c_str(),
                                     string_range_hex(scratch_address_range).c_str());
                    }
                }
            }
        }
    }

    {
        BufferAddressValidation<2> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10552",
               [](const vvl::Buffer& buffer_state) {
                   return (buffer_state.usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) == 0;
               },
               []() { return "The following buffers are missing VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR"; },
               kUsageErrorMsgBuffer},
              {"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10543",
               [&build_size_info](const vvl::Buffer& buffer_state) {
                   return buffer_state.requirements.size < build_size_info.accelerationStructureSize;
               },
               [&build_size_info]() {
                   return "The accelerationStructureSize (" + std::to_string(build_size_info.accelerationStructureSize) +
                          ") does not fit in any buffer";
               },
               kEmptyErrorMsgBuffer}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, error_obj.location.dot(Field::pBuildInfo).dot(Field::dstAccelerationStructureData), LogObjectList(commandBuffer),
            pBuildInfo->dstAccelerationStructureData, build_size_info.accelerationStructureSize);
    }

    if (pBuildInfo->dstAccelerationStructureData && pBuildInfo->scratchData) {
        const auto dst_buffer_states = GetBuffersByAddress(pBuildInfo->dstAccelerationStructureData);
        if (!dst_buffer_states.empty()) {
            const auto scratch_buffer_states = GetBuffersByAddress(pBuildInfo->scratchData);
            for (const auto& scratch_buffer_state : scratch_buffer_states) {
                vvl::range<VkDeviceAddress> scratch_address_range = scratch_buffer_state->DeviceAddressRange();
                if (!scratch_address_range.empty()) {
                    for (const auto& buffer_state : dst_buffer_states) {
                        const vvl::range<VkDeviceAddress> buffer_address_range = buffer_state->DeviceAddressRange();
                        if (buffer_address_range.intersects(scratch_address_range)) {
                            const LogObjectList objlist(commandBuffer, buffer_state->Handle(), scratch_buffer_state->Handle());
                            skip |= LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10548", objlist,
                                             error_obj.location.dot(Field::pBuildInfo).dot(Field::dstAccelerationStructureData),
                                             "%s address range %s intersects scratchData address range %s",
                                             FormatHandle(buffer_state->Handle()).c_str(),
                                             string_range_hex(buffer_address_range).c_str(),
                                             string_range_hex(scratch_address_range).c_str());
                        }
                    }
                }
            }
        }
    }

    // Check for src vs dst overlap, but only if they are different addresses (in-place update is allowed)
    if (pBuildInfo->srcAccelerationStructureData && pBuildInfo->dstAccelerationStructureData &&
        pBuildInfo->srcAccelerationStructureData != pBuildInfo->dstAccelerationStructureData) {
        const auto src_buffer_states = GetBuffersByAddress(pBuildInfo->srcAccelerationStructureData);
        const auto dst_buffer_states = GetBuffersByAddress(pBuildInfo->dstAccelerationStructureData);
        for (const auto& src_buffer_state : src_buffer_states) {
            const vvl::range<VkDeviceAddress> src_address_range = src_buffer_state->DeviceAddressRange();
            if (src_address_range.empty()) {
                continue;
            }
            for (const auto& dst_buffer_state : dst_buffer_states) {
                const vvl::range<VkDeviceAddress> dst_address_range = dst_buffer_state->DeviceAddressRange();
                if (src_address_range.intersects(dst_address_range)) {
                    const LogObjectList objlist(commandBuffer, src_buffer_state->Handle(), dst_buffer_state->Handle());
                    skip |=
                        LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10549", objlist,
                                    error_obj.location.dot(Field::pBuildInfo).dot(Field::srcAccelerationStructureData),
                                    "(%s) address range %s intersects "
                                    "dstAccelerationStructureData (%s) address range %s",
                                    FormatHandle(src_buffer_state->Handle()).c_str(), string_range_hex(src_address_range).c_str(),
                                    FormatHandle(dst_buffer_state->Handle()).c_str(), string_range_hex(dst_address_range).c_str());
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pBuildInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.partitionedAccelerationStructure) {
        skip |= LogError("VUID-vkGetPartitionedAccelerationStructuresBuildSizesNV-partitionedAccelerationStructure-10534", device,
                         error_obj.location, "partitionedAccelerationStructure feature was not enabled.");
    }
    if ((pInfo->partitionCount + pInfo->maxInstanceInGlobalPartitionCount) >
        phys_dev_ext_props.partitioned_acceleration_structure_props.maxPartitionCount) {
        skip |= LogError("VUID-VkPartitionedAccelerationStructureInstancesInputNV-partitionCount-10535", device,
                         error_obj.location.dot(Field::pInfo).dot(Field::partitionCount),
                         "(%" PRIu32 ") and maxInstanceInGlobalPartitionCount (%" PRIu32
                         ") sum must be less than or equal to "
                         "maxPartitionCount (%" PRIu32 ") ",
                         pInfo->partitionCount, pInfo->maxInstanceInGlobalPartitionCount,
                         phys_dev_ext_props.partitioned_acceleration_structure_props.maxPartitionCount);
    }
    return skip;
}

bool CoreChecks::ValidateBuildPartitionedAccelerationStructureInfoNV(
    const VkBuildPartitionedAccelerationStructureInfoNV& build_info, const Location& build_info_loc,
    VkDeviceSize build_scratch_size, VkDeviceSize build_acceleration_structure_size) const {
    bool skip = false;

    if (!build_info.scratchData) {
        if (build_scratch_size != 0) {
            skip |= LogError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-scratchData-10558", device,
                             build_info_loc.dot(Field::scratchData), "(0x%" PRIx64 ") must not be NULL", build_info.scratchData);
        }
    } else {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-VkBuildPartitionedAccelerationStructureInfoNV-scratchData-10559",
               [&build_scratch_size](const vvl::Buffer& buffer_state) {
                   return buffer_state.requirements.size < build_scratch_size;
               },
               [&build_scratch_size]() {
                   return "The buildScratchSize (" + std::to_string(build_scratch_size) + ") does not fit in any buffer";
               },
               kEmptyErrorMsgBuffer}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(*this, build_info_loc.dot(Field::scratchData), LogObjectList(device),
                                                               build_info.scratchData, build_scratch_size);
    }
    if (build_info.dstAccelerationStructureData != 0) {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-VkBuildPartitionedAccelerationStructureInfoNV-dstAccelerationStructureData-10562",
               [&build_acceleration_structure_size](const vvl::Buffer& buffer_state) {
                   return buffer_state.requirements.size < build_acceleration_structure_size;
               },
               [&build_acceleration_structure_size]() {
                   return "The accelerationStructureSize (" + std::to_string(build_acceleration_structure_size) +
                          ") does not fit in any buffer";
               },
               kEmptyErrorMsgBuffer}}}};
        skip |= buffer_address_validator.ValidateDeviceAddress(*this, build_info_loc.dot(Field::dstAccelerationStructureData),
                                                               LogObjectList(device), build_info.dstAccelerationStructureData,
                                                               build_acceleration_structure_size);
    }

    if (!IsPointerAligned(build_info.srcInfosCount, 4)) {
        skip |= LogError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-srcInfosCount-10563", device,
                         build_info_loc.dot(Field::srcInfosCount), "(0x%" PRIx64 ") must be aligned to 256 bytes",
                         build_info.srcInfosCount);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    const Location command_infos_loc = error_obj.location.dot(Field::pCommandInfos);
    const LogObjectList objlist(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateClusterAccelerationStructureCommandsInfoNV(*pCommandInfos, objlist, command_infos_loc);
    if (!enabled_features.clusterAccelerationStructure) {
        skip |= LogError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-clusterAccelerationStructure-10443", objlist,
                         error_obj.location, "clusterAccelerationStructures feature was not enabled.");
    }

    const auto& last_bound_state = cb_state->GetLastBoundRayTracing();
    const auto* pipeline_state = last_bound_state.pipeline_state;
    if (pipeline_state && !vku::FindStructInPNextChain<VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV>(
                              pipeline_state->RayTracingCreateInfo().pNext)) {
        skip |= LogError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pNext-10444", objlist, error_obj.location,
                         "The pNext chain of the bound ray tracing pipeline must include a "
                         "VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV structure.\n%s",
                         PrintPNextChain(Struct::VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV,
                                         pipeline_state->RayTracingCreateInfo().pNext)
                             .c_str());
    }

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructure_size = vku::InitStructHelper();
    DispatchGetClusterAccelerationStructureBuildSizesNV(device, &(pCommandInfos->input), &accelerationStructure_size);

    {
        BufferAddressValidation<2> scratch_buffer_validator = {{{
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-scratchData-12300",
             [&accelerationStructure_size](const vvl::Buffer& buffer_state) {
                 return buffer_state.create_info.size < accelerationStructure_size.buildScratchSize;
             },
             [&accelerationStructure_size]() {
                 return "The buildScratchSize (" + std::to_string(accelerationStructure_size.buildScratchSize) +
                        ") does not fit in any buffer";
             },
             kEmptyErrorMsgBuffer},
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12304",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT"; }, kUsageErrorMsgBuffer},
        }}};
        skip |= scratch_buffer_validator.ValidateDeviceAddress(*this, command_infos_loc.dot(Field::scratchData), objlist,
                                                               pCommandInfos->scratchData);
    }

    if (!IsPointerAligned(pCommandInfos->scratchData, phys_dev_ext_props.cluster_acceleration_props.clusterScratchByteAlignment)) {
        skip |= LogError(
            "VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-scratchData-12301", commandBuffer,
            command_infos_loc.dot(Field::scratchData),
            "(0x%" PRIx64
            ") must be aligned to VkPhysicalDeviceClusterAccelerationPropertiesNV::clusterScratchByteAlignment (%" PRIu32 ")",
            pCommandInfos->scratchData, phys_dev_ext_props.cluster_acceleration_props.clusterScratchByteAlignment);
    }

    {
        BufferAddressValidation<1> src_infos_array_validator = {{{
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12305",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) &
                         VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
             },
             []() {
                 return "The following buffers are missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
             },
             kUsageErrorMsgBuffer},
        }}};

        skip |= src_infos_array_validator.ValidateDeviceAddress(
            *this, command_infos_loc.dot(Field::srcInfosArray).dot(Field::deviceAddress), objlist,
            pCommandInfos->srcInfosArray.deviceAddress);
    }

    {
        BufferAddressValidation<1> src_infos_count_validator = {{{
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12306",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) &
                         VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
             },
             []() {
                 return "The following buffers are missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
             },
             kUsageErrorMsgBuffer},
        }}};

        skip |= src_infos_count_validator.ValidateDeviceAddress(*this, command_infos_loc.dot(Field::srcInfosCount), objlist,
                                                                pCommandInfos->srcInfosCount);
    }

    {
        BufferAddressValidation<1> dst_addresses_array_validator = {{{
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12307",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) == 0;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR"; },
             kUsageErrorMsgBuffer},
        }}};

        skip |= dst_addresses_array_validator.ValidateDeviceAddress(
            *this, command_infos_loc.dot(Field::dstAddressesArray).dot(Field::deviceAddress), objlist,
            pCommandInfos->dstAddressesArray.deviceAddress);
    }

    {
        BufferAddressValidation<1> dst_implicit_data_validator = {{{
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12308",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) == 0;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR"; },
             kUsageErrorMsgBuffer},
        }}};

        skip |= dst_implicit_data_validator.ValidateDeviceAddress(*this, command_infos_loc.dot(Field::dstImplicitData), objlist,
                                                                  pCommandInfos->dstImplicitData);
    }

    if (pCommandInfos->scratchData && pCommandInfos->dstImplicitData) {
        const vvl::range<VkDeviceAddress> scratch_range = {
            pCommandInfos->scratchData, pCommandInfos->scratchData + accelerationStructure_size.buildScratchSize};
        const vvl::range<VkDeviceAddress> dst_implicit_range = {
            pCommandInfos->dstImplicitData, pCommandInfos->dstImplicitData + accelerationStructure_size.accelerationStructureSize};
        if (scratch_range.intersects(dst_implicit_range)) {
            skip |= LogError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-dstImplicitData-12303", objlist,
                             command_infos_loc.dot(Field::dstImplicitData),
                             "address range %s intersects with scratchData address range %s",
                             string_range_hex(dst_implicit_range).c_str(), string_range_hex(scratch_range).c_str());
        }
    }

    if (pCommandInfos->scratchData && pCommandInfos->dstAddressesArray.deviceAddress) {
        const vvl::range<VkDeviceAddress> scratch_range = {
            pCommandInfos->scratchData, pCommandInfos->scratchData + accelerationStructure_size.buildScratchSize};
        const VkDeviceSize dst_addresses_size =
            pCommandInfos->dstAddressesArray.stride * pCommandInfos->input.maxAccelerationStructureCount;
        const vvl::range<VkDeviceAddress> dst_addresses_range = {
            pCommandInfos->dstAddressesArray.deviceAddress, pCommandInfos->dstAddressesArray.deviceAddress + dst_addresses_size};
        if (scratch_range.intersects(dst_addresses_range)) {
            skip |= LogError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-dstAddressesArray-12302", objlist,
                             command_infos_loc.dot(Field::dstAddressesArray),
                             "address range %s intersects with scratchData address range %s",
                             string_range_hex(dst_addresses_range).c_str(), string_range_hex(scratch_range).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                            const VkClusterAccelerationStructureInputInfoNV* pinfo,
                                                                            VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.clusterAccelerationStructure) {
        skip |= LogError("VUID-vkGetClusterAccelerationStructureBuildSizesNV-clusterAccelerationStructure-10438", device,
                         error_obj.location, "clusterAccelerationStructures feature was not enabled.");
    }

    if (IsValueIn(pinfo->opType, {VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV})) {
        const VkClusterAccelerationStructureTriangleClusterInputNV* triangle_input =
            reinterpret_cast<const VkClusterAccelerationStructureTriangleClusterInputNV*>(pinfo->opInput.pTriangleClusters);

        skip |= ValidateClusterAccelerationStructureTriangleClusterInputNV(
            *triangle_input, error_obj.location.dot(Field::input).dot(Field::opInput).dot(Field::pTriangleClusters));
    }

    return skip;
}
bool CoreChecks::ValidateClusterAccelerationStructureTriangleClusterInputNV(
    const VkClusterAccelerationStructureTriangleClusterInputNV& input, const Location& input_loc) const {
    bool skip = false;
    const VkFormatProperties3 vertex_properties = GetPDFormatProperties(input.vertexFormat);
    if (!(vertex_properties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
        skip |=
            LogError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-vertexFormat-10439", device,
                     input_loc.dot(Field::vertexFormat),
                     "(%s) doesn't support VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR.\n"
                     "(supported bufferFeatures: %s)",
                     string_VkFormat(input.vertexFormat), string_VkFormatFeatureFlags2(vertex_properties.bufferFeatures).c_str());
    }

    if (input.maxClusterTriangleCount > phys_dev_ext_props.cluster_acceleration_props.maxTrianglesPerCluster) {
        skip |= LogError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-maxClusterTriangleCount-10440", device,
                         input_loc.dot(Field::maxClusterTriangleCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::maxTrianglesPerCluster (%" PRIu32 ")",
                         input.maxClusterTriangleCount, phys_dev_ext_props.cluster_acceleration_props.maxTrianglesPerCluster);
    }

    if (input.maxClusterVertexCount > phys_dev_ext_props.cluster_acceleration_props.maxVerticesPerCluster) {
        skip |= LogError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-maxClusterVertexCount-10441", device,
                         input_loc.dot(Field::maxClusterVertexCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::maxVerticesPerCluster (%" PRIu32 ")",
                         input.maxClusterVertexCount, phys_dev_ext_props.cluster_acceleration_props.maxVerticesPerCluster);
    }

    if (input.minPositionTruncateBitCount > 32) {
        skip |= LogError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-minPositionTruncateBitCount-10442", device,
                         input_loc.dot(Field::minPositionTruncateBitCount), "(%" PRIu32 ") must be less than or equal to 32",
                         input.minPositionTruncateBitCount);
    }
    return skip;
}

bool CoreChecks::ValidateClusterAccelerationStructureCommandsInfoNV(
    const VkClusterAccelerationStructureCommandsInfoNV& command_infos, const LogObjectList& objlist,
    const Location& command_infos_loc) const {
    bool skip = false;
    bool invalid_triangle_input = false;
    if (IsValueIn(command_infos.input.opType, {VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV,
                                               VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV,
                                               VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV,
                                               VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV})) {
        const VkClusterAccelerationStructureTriangleClusterInputNV* triangle_input =
            reinterpret_cast<const VkClusterAccelerationStructureTriangleClusterInputNV*>(
                command_infos.input.opInput.pTriangleClusters);
        skip |= ValidateClusterAccelerationStructureTriangleClusterInputNV(
            *triangle_input, command_infos_loc.dot(Field::input).dot(Field::opInput).dot(Field::pTriangleClusters));

        if (triangle_input->maxClusterTriangleCount > phys_dev_ext_props.cluster_acceleration_props.maxTrianglesPerCluster ||
            triangle_input->maxClusterVertexCount > phys_dev_ext_props.cluster_acceleration_props.maxVerticesPerCluster ||
            triangle_input->minPositionTruncateBitCount > 32) {
            invalid_triangle_input = true;
        }
    }
    // aligned based on the cluster acceleration structure type and its alignment properties as described in
    // VkPhysicalDeviceClusterAccelerationStructurePropertiesNV
    uint32_t alignment_type = 1;
    const char* vuid = kVUIDUndefined;
    switch (command_infos.input.opType) {
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV:
            alignment_type = phys_dev_ext_props.cluster_acceleration_props.clusterTemplateByteAlignment;
            vuid = "VUID-VkClusterAccelerationStructureCommandsInfoNV-input-12318";
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV:
            alignment_type = phys_dev_ext_props.cluster_acceleration_props.clusterByteAlignment;
            vuid = "VUID-VkClusterAccelerationStructureCommandsInfoNV-input-12317";
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV:
            alignment_type = phys_dev_ext_props.cluster_acceleration_props.clusterByteAlignment;
            vuid = "VUID-VkClusterAccelerationStructureCommandsInfoNV-input-12319";
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MAX_ENUM_NV:
            break;
    }
    VkAccelerationStructureBuildSizesInfoKHR accelerationStructure_size = vku::InitStructHelper();
    DispatchGetClusterAccelerationStructureBuildSizesNV(device, &(command_infos.input), &accelerationStructure_size);
    if (command_infos.input.opMode == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV) {
        if (command_infos.dstImplicitData == 0) {
            skip |= LogError(
                "VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-12309", objlist,
                command_infos_loc.dot(Field::dstImplicitData),
                "(0x%" PRIx64
                ") must be a valid address if input::opMode is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV",
                command_infos.dstImplicitData);
        } else if (!IsPointerAligned(command_infos.dstImplicitData, alignment_type)) {
            skip |= LogError(vuid, objlist, command_infos_loc.dot(Field::dstImplicitData),
                             "(0x%" PRIx64 ") must be aligned to (%" PRIu32
                             ") depending on the input::opMode (%s) and input::opType (%s)",
                             command_infos.dstImplicitData, alignment_type,
                             string_VkClusterAccelerationStructureOpModeNV(command_infos.input.opMode),
                             string_VkClusterAccelerationStructureOpTypeNV(command_infos.input.opType));
        } else {
            if (!invalid_triangle_input &&
                command_infos.input.opType != VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV) {
                BufferAddressValidation<1> dst_implicit_size_validator = {{{
                    {"VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-12310",
                     [&accelerationStructure_size](const vvl::Buffer& buffer_state) {
                         return buffer_state.create_info.size < accelerationStructure_size.accelerationStructureSize;
                     },
                     [&accelerationStructure_size]() {
                         return "The accelerationStructureSize (" +
                                std::to_string(accelerationStructure_size.accelerationStructureSize) +
                                ") does not fit in any buffer";
                     },
                     kEmptyErrorMsgBuffer},
                }}};

                skip |= dst_implicit_size_validator.ValidateDeviceAddress(*this, command_infos_loc.dot(Field::dstImplicitData),
                                                                          objlist, command_infos.dstImplicitData,
                                                                          accelerationStructure_size.accelerationStructureSize);
            }
        }
    }

    if (command_infos.input.opMode == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_COMPUTE_SIZES_NV) {
        if (command_infos.dstSizesArray.deviceAddress == 0) {
            skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-12312", objlist,
                             command_infos_loc.dot(Field::dstSizesArray).dot(Field::deviceAddress),
                             "is zero, but input::opMode is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_COMPUTE_SIZES_NV");
        }
        skip |= ValidateDeviceAddress(command_infos_loc.dot(Field::dstSizesArray).dot(Field::deviceAddress), objlist,
                                      command_infos.dstSizesArray.deviceAddress);
    }

    if (command_infos.input.opMode == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_EXPLICIT_DESTINATIONS_NV) {
        if (command_infos.dstAddressesArray.deviceAddress == 0) {
            skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-12313", objlist,
                             command_infos_loc.dot(Field::dstAddressesArray).dot(Field::deviceAddress),
                             "is zero, but input::opMode is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_EXPLICIT_DESTINATIONS_NV");
        }
        skip |= ValidateDeviceAddress(command_infos_loc.dot(Field::dstAddressesArray).dot(Field::deviceAddress), objlist,
                                      command_infos.dstAddressesArray.deviceAddress);
    }

    if (command_infos.dstAddressesArray.deviceAddress != 0 && command_infos.dstAddressesArray.stride < 8) {
        skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-dstAddressesArray-10474", objlist,
                         command_infos_loc.dot(Field::dstAddressesArray).dot(Field::stride),
                         "(%" PRIu64 ") must be greater than or equal to 8", command_infos.dstAddressesArray.stride);
    }

    if (command_infos.dstSizesArray.deviceAddress != 0 && command_infos.dstSizesArray.stride < 4) {
        skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-dstSizesArray-10475", objlist,
                         command_infos_loc.dot(Field::dstSizesArray).dot(Field::stride),
                         "(%" PRIu64 ") must be greater than or equal to 4", command_infos.dstSizesArray.stride);
    }
    uint32_t stride_min = 0;
    switch (command_infos.input.opType) {
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV:
            stride_min = sizeof(VkClusterAccelerationStructureMoveObjectsInfoNV);
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV:
            stride_min = sizeof(VkClusterAccelerationStructureBuildClustersBottomLevelInfoNV);
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV:
            stride_min = sizeof(VkClusterAccelerationStructureBuildTriangleClusterInfoNV);
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV:
            stride_min = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV:
            stride_min = sizeof(VkClusterAccelerationStructureInstantiateClusterInfoNV);
            break;

        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MAX_ENUM_NV:
            break;
    }

    if (command_infos.srcInfosArray.stride < stride_min && command_infos.srcInfosArray.stride != 0) {
        skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-srcInfosArray-10476", objlist,
                         command_infos_loc.dot(Field::srcInfosArray).dot(Field::stride),
                         "(%" PRIu64 ") must be greater than size of %s (%" PRIu32 ")", command_infos.srcInfosArray.stride,
                         string_VkClusterAccelerationStructureOpTypeNV(command_infos.input.opType), stride_min);
    }

    if (!IsPointerAligned(command_infos.scratchData, phys_dev_ext_props.cluster_acceleration_props.clusterScratchByteAlignment)) {
        skip |= LogError(
            "VUID-VkClusterAccelerationStructureCommandsInfoNV-scratchData-12320", objlist,
            command_infos_loc.dot(Field::scratchData),
            "(0x%" PRIx64
            ") must be aligned to VkPhysicalDeviceClusterAccelerationPropertiesNV::clusterScratchByteAlignment (%" PRIu32 ")",
            command_infos.scratchData, phys_dev_ext_props.cluster_acceleration_props.clusterScratchByteAlignment);
    }

    if (accelerationStructure_size.buildScratchSize != 0) {
        if (command_infos.scratchData == 0) {
            skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-buildScratchSize-12321", objlist,
                             command_infos_loc.dot(Field::scratchData),
                             "(0x%" PRIx64 ") must be a valid VkDeviceAddress when buildScratchSize (%" PRIu64 ") is not 0",
                             command_infos.scratchData, static_cast<uint64_t>(accelerationStructure_size.buildScratchSize));
        } else {
            skip |= ValidateDeviceAddress(command_infos_loc.dot(Field::scratchData), objlist, command_infos.scratchData);
        }
    }

    if (!IsPointerAligned(command_infos.srcInfosCount, 4)) {
        skip |= LogError("VUID-VkClusterAccelerationStructureCommandsInfoNV-srcInfosCount-12322", objlist,
                         command_infos_loc.dot(Field::srcInfosCount), "(0x%" PRIx64 ") must be 4-byte aligned",
                         command_infos.srcInfosCount);
    }

    return skip;
}
