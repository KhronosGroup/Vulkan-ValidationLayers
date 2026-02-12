/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2025-2026 Advanced Micro Devices, Inc. All rights reserved.
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
#include "containers/container_utils.h"
#include "error_message/error_location.h"
#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

#include "utils/vk_api_utils.h"
#include "utils/math_utils.h"

namespace stateless {

bool Device::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV& triangles, VkAccelerationStructureNV object_handle,
                                         const Location& loc) const {
    bool skip = false;

    if (triangles.vertexFormat != VK_FORMAT_R32G32B32_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16B16_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16B16_SNORM && triangles.vertexFormat != VK_FORMAT_R32G32_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16_SNORM) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-vertexFormat-02430", object_handle, loc, "is invalid.");
    } else {
        uint32_t vertex_component_size = 0;
        if (triangles.vertexFormat == VK_FORMAT_R32G32B32_SFLOAT || triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT) {
            vertex_component_size = 4;
        } else if (triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16B16_SNORM ||
                   triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16_SNORM) {
            vertex_component_size = 2;
        }
        if (vertex_component_size > 0 && !IsIntegerMultipleOf(triangles.vertexOffset, vertex_component_size)) {
            skip |= LogError("VUID-VkGeometryTrianglesNV-vertexOffset-02429", object_handle, loc, "is invalid.");
        }
    }

    if (triangles.indexType != VK_INDEX_TYPE_UINT32 && triangles.indexType != VK_INDEX_TYPE_UINT16 &&
        triangles.indexType != VK_INDEX_TYPE_NONE_NV) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-indexType-02433", object_handle, loc, "is invalid.");
    } else {
        const uint32_t index_type_size = IndexTypeByteSize(triangles.indexType);
        if (index_type_size > 0 && !IsIntegerMultipleOf(triangles.indexOffset, index_type_size)) {
            skip |= LogError("VUID-VkGeometryTrianglesNV-indexOffset-02432", object_handle, loc, "is invalid.");
        }

        if (triangles.indexType == VK_INDEX_TYPE_NONE_NV) {
            if (triangles.indexCount != 0) {
                skip |= LogError("VUID-VkGeometryTrianglesNV-indexCount-02436", object_handle, loc, "is invalid.");
            }
            if (triangles.indexData != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkGeometryTrianglesNV-indexData-02434", object_handle, loc, "is invalid.");
            }
        }
    }

    if (!IsIntegerMultipleOf(triangles.transformOffset, 16)) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-transformOffset-02438", object_handle, loc, "is invalid.");
    }

    return skip;
}

bool Device::ValidateGeometryAABBNV(const VkGeometryAABBNV& aabbs, VkAccelerationStructureNV object_handle,
                                    const Location& loc) const {
    bool skip = false;

    if (!IsIntegerMultipleOf(aabbs.offset, 8)) {
        skip |= LogError("VUID-VkGeometryAABBNV-offset-02440", object_handle, loc, "is invalid.");
    }
    if (!IsIntegerMultipleOf(aabbs.stride, 8)) {
        skip |= LogError("VUID-VkGeometryAABBNV-stride-02441", object_handle, loc, "is invalid.");
    }

    return skip;
}

bool Device::ValidateGeometryNV(const VkGeometryNV& geometry, VkAccelerationStructureNV object_handle, const Location& loc) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip |= ValidateGeometryTrianglesNV(geometry.geometry.triangles, object_handle, loc);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip |= ValidateGeometryAABBNV(geometry.geometry.aabbs, object_handle, loc);
    }
    return skip;
}

bool Device::ValidateAccelerationStructureInfoNV(const Context& context, const VkAccelerationStructureInfoNV& info,
                                                 VkAccelerationStructureNV object_handle, const Location& loc) const {
    bool skip = false;

    bool is_cmd = loc.function == Func::vkCmdBuildAccelerationStructureNV;
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV && info.geometryCount != 0) {
        skip |= LogError("VUID-VkAccelerationStructureInfoNV-type-02425", object_handle, loc,
                         "If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV then "
                         "geometryCount must be 0.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.instanceCount != 0) {
        skip |= LogError("VUID-VkAccelerationStructureInfoNV-type-02426", object_handle, loc,
                         "If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV then "
                         "instanceCount must be 0.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
        skip |= LogError("VUID-VkAccelerationStructureInfoNV-type-04623", object_handle, loc,
                         "type is invalid VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV) {
        skip |= LogError("VUID-VkAccelerationStructureInfoNV-flags-02592", object_handle, loc,
                         "If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV"
                         "bit set, then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV bit set.");
    }
    if (info.geometryCount > phys_dev_ext_props.ray_tracing_props_nv.maxGeometryCount) {
        skip |= LogError(is_cmd ? "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241"
                                : "VUID-VkAccelerationStructureInfoNV-geometryCount-02422",
                         object_handle, loc,
                         "geometryCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.");
    }
    if (info.instanceCount > phys_dev_ext_props.ray_tracing_props_nv.maxInstanceCount) {
        skip |= LogError("VUID-VkAccelerationStructureInfoNV-instanceCount-02423", object_handle, loc,
                         "instanceCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxInstanceCount.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 0) {
        uint64_t total_triangle_count = 0;
        for (uint32_t i = 0; i < info.geometryCount; i++) {
            const VkGeometryNV& geometry = info.pGeometries[i];

            skip |= ValidateGeometryNV(geometry, object_handle, loc);

            if (geometry.geometryType != VK_GEOMETRY_TYPE_TRIANGLES_NV) {
                continue;
            }
            total_triangle_count += geometry.geometry.triangles.indexCount / 3;
        }
        if (total_triangle_count > phys_dev_ext_props.ray_tracing_props_nv.maxTriangleCount) {
            skip |= LogError("VUID-VkAccelerationStructureInfoNV-maxTriangleCount-02424", object_handle, loc,
                             "The total number of triangles in all geometries must be less than "
                             "or equal to VkPhysicalDeviceRayTracingPropertiesNV::maxTriangleCount.");
        }
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 1) {
        const VkGeometryTypeNV first_geometry_type = info.pGeometries[0].geometryType;
        for (uint32_t i = 1; i < info.geometryCount; i++) {
            const VkGeometryNV& geometry = info.pGeometries[i];
            if (geometry.geometryType != first_geometry_type) {
                skip |= LogError("VUID-VkAccelerationStructureInfoNV-type-02786", object_handle, loc,
                                 "info.pGeometries[%" PRIu32
                                 "].geometryType does not match "
                                 "info.pGeometries[0].geometryType.",
                                 i);
            }
        }
    }
    for (uint32_t geometry_index = 0; geometry_index < info.geometryCount; ++geometry_index) {
        if (!(info.pGeometries[geometry_index].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV ||
              info.pGeometries[geometry_index].geometryType == VK_GEOMETRY_TYPE_AABBS_NV)) {
            skip |= LogError("VUID-VkGeometryNV-geometryType-03503", object_handle, loc,
                             "geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV"
                             "or VK_GEOMETRY_TYPE_AABBS_NV.");
        }
    }
    skip |= context.ValidateFlags(loc.dot(Field::flags), vvl::FlagBitmask::VkBuildAccelerationStructureFlagBitsKHR,
                                  AllVkBuildAccelerationStructureFlagBitsKHR, info.flags, kOptionalFlags,
                                  "VUID-VkAccelerationStructureInfoNV-flags-parameter");
    return skip;
}

bool Device::manual_PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                                 const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkAccelerationStructureNV* pAccelerationStructure,
                                                                 const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if ((pCreateInfo->compactedSize != 0) && ((pCreateInfo->info.geometryCount != 0) || (pCreateInfo->info.instanceCount != 0))) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421", device, error_obj.location,
                         "pCreateInfo->compactedSize nonzero (%" PRIu64 ") with info.geometryCount (%" PRIu32
                         ") or info.instanceCount (%" PRIu32 ") nonzero.",
                         pCreateInfo->compactedSize, pCreateInfo->info.geometryCount, pCreateInfo->info.instanceCount);
    }

    skip |= ValidateAccelerationStructureInfoNV(context, pCreateInfo->info, VkAccelerationStructureNV(0), error_obj.location);
    return skip;
}

bool Device::manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                   const VkAccelerationStructureInfoNV* pInfo,
                                                                   VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                                   VkBool32 update, VkAccelerationStructureNV dst,
                                                                   VkAccelerationStructureNV src, VkBuffer scratch,
                                                                   VkDeviceSize scratchOffset, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (pInfo != nullptr) {
        skip |= ValidateAccelerationStructureInfoNV(context, *pInfo, dst, error_obj.location);
    }

    return skip;
}

bool Device::manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                                                                    VkAccelerationStructureNV accelerationStructure,
                                                                    size_t dataSize, void* pData, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    if (dataSize < 8) {
        skip |= LogError("VUID-vkGetAccelerationStructureHandleNV-dataSize-02240", accelerationStructure,
                         error_obj.location.dot(Field::dataSize), "must be greater than or equal to 8.");
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    if (queryType != VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryType-06216", device, error_obj.location,
                         "queryType must be "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV.");
    }
    return skip;
}

bool Device::ValidateCreateRayTracingPipelinesFlagsNV(const VkPipelineCreateFlags2 flags, const Location& flags_loc) const {
    bool skip = false;
    if (flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-02904", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-11008", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV) && (flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT)) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-02957", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03456", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03458", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03459", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03460", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03461", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03462", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03463", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03588", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
        skip |= LogError("VUID-vkCreateRayTracingPipelinesNV-flags-03816", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-04948", device, flags_loc, "is %s",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & (VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT)) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-12341", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    return skip;
}

bool Device::manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                               uint32_t createInfoCount,
                                                               const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                               const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkRayTracingPipelineCreateInfoNV& create_info = pCreateInfos[i];

        for (uint32_t stage_index = 0; stage_index < create_info.stageCount; ++stage_index) {
            skip |= ValidatePipelineShaderStageCreateInfoCommon(context, create_info.pStages[stage_index],
                                                                create_info_loc.dot(Field::pStages, stage_index));
        }
        auto feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != create_info.stageCount)) {
            skip |=
                LogError("VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651", device,
                         create_info_loc.dot(Field::stageCount),
                         "(%" PRIu32 ") must equal VkPipelineCreationFeedback::pipelineStageCreationFeedbackCount (%" PRIu32 ").",
                         create_info.stageCount, feedback_struct->pipelineStageCreationFeedbackCount);
        }

        const auto* create_flags_2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
        const VkPipelineCreateFlags2 flags =
            create_flags_2 ? create_flags_2->flags : static_cast<VkPipelineCreateFlags2>(create_info.flags);
        const Location flags_loc = create_flags_2 ? create_info_loc.pNext(Struct::VkPipelineCreateFlags2CreateInfo, Field::flags)
                                                  : create_info_loc.dot(Field::flags);
        if (!create_flags_2) {
            skip |= context.ValidateFlags(flags_loc, vvl::FlagBitmask::VkPipelineCreateFlagBits, AllVkPipelineCreateFlagBits,
                                          create_info.flags, kOptionalFlags, "VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        } else {
            skip |= ValidateCreatePipelinesFlags2(create_info.flags, flags, flags_loc);
        }
        skip |= ValidateCreatePipelinesFlagsCommon(flags, flags_loc);

        if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (create_info.basePipelineIndex != -1) {
                if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07986", device, flags_loc,
                                     "is %s, %s is %" PRId32 ", but %s is %s.", string_VkPipelineCreateFlags2(flags).c_str(),
                                     create_info_loc.dot(Field::basePipelineIndex).Fields().c_str(), create_info.basePipelineIndex,
                                     create_info_loc.dot(Field::basePipelineHandle).Fields().c_str(),
                                     FormatHandle(create_info.basePipelineHandle).c_str());
                }
                if (create_info.basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |= LogError("VUID-vkCreateRayTracingPipelinesNV-flags-03415", device, flags_loc,
                                     "is %s, but %s is %" PRId32 ".", string_VkPipelineCreateFlags2(flags).c_str(),
                                     create_info_loc.dot(Field::basePipelineIndex).Fields().c_str(), create_info.basePipelineIndex);
                }
            }
            if (create_info.basePipelineHandle == VK_NULL_HANDLE) {
                if (static_cast<uint32_t>(create_info.basePipelineIndex) >= createInfoCount) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07985", device,
                                     create_info_loc.dot(Field::basePipelineHandle), "is VK_NULL_HANDLE, but %s is %s.",
                                     flags_loc.Fields().c_str(), string_VkPipelineCreateFlags2(flags).c_str());
                }
            } else {
                if (create_info.basePipelineIndex != -1) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07986", device, flags_loc,
                                     "is %s, but %s is %" PRId32 ".", string_VkPipelineCreateFlags2(flags).c_str(),
                                     create_info_loc.dot(Field::basePipelineIndex).Fields().c_str(), create_info.basePipelineIndex);
                }
            }
        }

        skip |=
            ValidatePipelineBinaryInfo(create_info.pNext, create_info.flags, pipelineCache, create_info.layout, create_info_loc);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    skip |= context.ValidateStructType(error_obj.location.dot(Field::pBuildInfo).dot(Field::input), &(pBuildInfo->input),
                                       VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV, true,
                                       "VUID-VkBuildPartitionedAccelerationStructureInfoNV-input-parameter",
                                       "VUID-VkBuildPartitionedAccelerationStructureInfoNV-sType-sType");

    return skip;
}

bool Device::manual_PreCallValidateCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    const Location input_loc = info_loc.dot(Field::input);
    const Location op_input_loc = input_loc.dot(Field::opInput);
    const auto& input = pInfo->input;

    skip |= context.ValidateStructType(input_loc, &pInfo->input, VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV,
                                       true, "VUID-VkClusterAccelerationStructureCommandsInfoNV-input-parameter",
                                       "VUID-VkClusterAccelerationStructureInputInfoNV-sType-sType");

    if (input.opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV) {
        if (input.opInput.pClustersBottomLevel) {
            skip |= ValidateClusterAccelerationStructureClustersBottomLevelInputNV(context, *input.opInput.pClustersBottomLevel,
                                                                                   op_input_loc.dot(Field::pClustersBottomLevel));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pClustersBottomLevel-parameter", device,
                             op_input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV, but "
                             "opInput.pClustersBottomLevel is null");
        }
    }

    if (IsValueIn(input.opType, {VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV,
                                 VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV,
                                 VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV,
                                 VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV})) {
        if (input.opInput.pTriangleClusters) {
            skip |= ValidateClusterAccelerationStructureTriangleClusterInputNV(context, *input.opInput.pTriangleClusters,
                                                                               op_input_loc.dot(Field::pTriangleClusters));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pTriangleClusters-parameter", device,
                             op_input_loc.dot(Field::opType), "is %s, but opInput.pTriangleClusters is null.",
                             string_VkClusterAccelerationStructureOpTypeNV(input.opType));
        }
    }

    if (input.opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV) {
        if (input.opInput.pMoveObjects) {
            skip |= ValidateClusterAccelerationStructureMoveObjectsInputNV(context, *input.opInput.pMoveObjects,
                                                                           op_input_loc.dot(Field::pMoveObjects));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pMoveObjects-parameter", device,
                             op_input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV, but opInput.pMoveObjects is null");
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateGetClusterAccelerationStructureBuildSizesNV(
    VkDevice device, const VkClusterAccelerationStructureInputInfoNV* pInfo, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
    const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    const Location input_loc = error_obj.location.dot(Field::pInfo);
    const Location op_input_loc = input_loc.dot(Field::opInput);

    if (pInfo->opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV) {
        if (pInfo->opInput.pClustersBottomLevel) {
            skip |= ValidateClusterAccelerationStructureClustersBottomLevelInputNV(context, *pInfo->opInput.pClustersBottomLevel,
                                                                                   op_input_loc.dot(Field::pClustersBottomLevel));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pClustersBottomLevel-parameter", device,
                             op_input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV, but "
                             "opInput.pClustersBottomLevel is null");
        }
    }

    if (IsValueIn(pInfo->opType, {VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV})) {
        if (pInfo->opInput.pTriangleClusters) {
            skip |= ValidateClusterAccelerationStructureTriangleClusterInputNV(context, *pInfo->opInput.pTriangleClusters,
                                                                               op_input_loc.dot(Field::pTriangleClusters));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pTriangleClusters-parameter", device,
                             op_input_loc.dot(Field::opType), "is %s, but opInput.pTriangleClusters is null.",
                             string_VkClusterAccelerationStructureOpTypeNV(pInfo->opType));
        }
    }

    if (pInfo->opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV) {
        if (pInfo->opInput.pMoveObjects) {
            skip |= ValidateClusterAccelerationStructureMoveObjectsInputNV(context, *pInfo->opInput.pMoveObjects,
                                                                           op_input_loc.dot(Field::pMoveObjects));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pMoveObjects-parameter", device,
                             op_input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV, but opInput.pMoveObjects is null");
        }
    }

    return skip;
}

}  // namespace stateless