/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

#include "utils/ray_tracing_utils.h"

bool StatelessValidation::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles,
                                                      VkAccelerationStructureNV object_handle, const Location &loc) const {
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
        if (vertex_component_size > 0 && SafeModulo(triangles.vertexOffset, vertex_component_size) != 0) {
            skip |= LogError("VUID-VkGeometryTrianglesNV-vertexOffset-02429", object_handle, loc, "is invalid.");
        }
    }

    if (triangles.indexType != VK_INDEX_TYPE_UINT32 && triangles.indexType != VK_INDEX_TYPE_UINT16 &&
        triangles.indexType != VK_INDEX_TYPE_NONE_NV) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-indexType-02433", object_handle, loc, "is invalid.");
    } else {
        const uint32_t index_element_size = GetIndexAlignment(triangles.indexType);
        if (index_element_size > 0 && SafeModulo(triangles.indexOffset, index_element_size) != 0) {
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

    if (SafeModulo(triangles.transformOffset, 16) != 0) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-transformOffset-02438", object_handle, loc, "is invalid.");
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, VkAccelerationStructureNV object_handle,
                                                 const Location &loc) const {
    bool skip = false;

    if (SafeModulo(aabbs.offset, 8) != 0) {
        skip |= LogError("VUID-VkGeometryAABBNV-offset-02440", object_handle, loc, "is invalid.");
    }
    if (SafeModulo(aabbs.stride, 8) != 0) {
        skip |= LogError("VUID-VkGeometryAABBNV-stride-02441", object_handle, loc, "is invalid.");
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle,
                                             const Location &loc) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip |= ValidateGeometryTrianglesNV(geometry.geometry.triangles, object_handle, loc);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip |= ValidateGeometryAABBNV(geometry.geometry.aabbs, object_handle, loc);
    }
    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info,
                                                              VkAccelerationStructureNV object_handle, const Location &loc) const {
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
            const VkGeometryNV &geometry = info.pGeometries[i];

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
            const VkGeometryNV &geometry = info.pGeometries[i];
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
    skip |= ValidateFlags(loc.dot(Field::flags), vvl::FlagBitmask::VkBuildAccelerationStructureFlagBitsKHR,
                          AllVkBuildAccelerationStructureFlagBitsKHR, info.flags, kOptionalFlags,
                          "VUID-VkAccelerationStructureInfoNV-flags-parameter");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureNV(
    VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureNV *pAccelerationStructure, const ErrorObject &error_obj) const {
    bool skip = false;
    if (pCreateInfo) {
        if ((pCreateInfo->compactedSize != 0) &&
            ((pCreateInfo->info.geometryCount != 0) || (pCreateInfo->info.instanceCount != 0))) {
            skip |= LogError("VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421", device, error_obj.location,
                             "pCreateInfo->compactedSize nonzero (%" PRIu64 ") with info.geometryCount (%" PRIu32
                             ") or info.instanceCount (%" PRIu32 ") nonzero.",
                             pCreateInfo->compactedSize, pCreateInfo->info.geometryCount, pCreateInfo->info.instanceCount);
        }

        skip |= ValidateAccelerationStructureInfoNV(pCreateInfo->info, VkAccelerationStructureNV(0), error_obj.location);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructureNV(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset,
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset,
    const ErrorObject &error_obj) const {
    bool skip = false;

    if (pInfo != nullptr) {
        skip |= ValidateAccelerationStructureInfoNV(*pInfo, dst, error_obj.location);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureKHR(
    VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureKHR *pAccelerationStructure, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkCreateAccelerationStructureKHR-accelerationStructure-03611", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    if (pCreateInfo) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
        if (pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR &&
            (!acceleration_structure_features || acceleration_structure_features->accelerationStructureCaptureReplay == VK_FALSE)) {
            skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613", device,
                             create_info_loc.dot(Field::createFlags),
                             "includes "
                             "VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR, "
                             "but accelerationStructureCaptureReplay feature is not enabled.");
        }
        if (pCreateInfo->deviceAddress &&
            !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR)) {
            skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612", device,
                             create_info_loc.dot(Field::createFlags), "is %s, but deviceAddress (%" PRIu64 ") is not zero.",
                             string_VkAccelerationStructureCreateFlagsKHR(pCreateInfo->createFlags).c_str(),
                             pCreateInfo->deviceAddress);
        }
        if (pCreateInfo->deviceAddress &&
            (!acceleration_structure_features || acceleration_structure_features->accelerationStructureCaptureReplay == VK_FALSE)) {
            skip |= LogError(
                "VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488", device, create_info_loc.dot(Field::deviceAddress),
                "is %" PRIu64 " but accelerationStructureCaptureReplay feature was not enabled.", pCreateInfo->deviceAddress);
        }
        if (SafeModulo(pCreateInfo->offset, 256) != 0) {
            skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-offset-03734", device, create_info_loc.dot(Field::offset),
                             "%" PRIu64 " must be a multiple of 256 bytes", pCreateInfo->offset);
        }

        const auto *descriptor_buffer_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(device_createinfo_pnext);
        if ((pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
            (!descriptor_buffer_features ||
             (descriptor_buffer_features && descriptor_buffer_features->descriptorBufferCaptureReplay == VK_FALSE))) {
            skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-createFlags-08108", device,
                             create_info_loc.dot(Field::createFlags),
                             "includes VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT, but the "
                             "descriptorBufferCaptureReplay feature was not enabled.");
        }

        const auto *opaque_capture_descriptor_buffer =
            vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        if (opaque_capture_descriptor_buffer &&
            !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |=
                LogError("VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109", device, create_info_loc.dot(Field::createFlags),
                         "includes VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT, but  "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateDestroyAccelerationStructureKHR(VkDevice device,
                                                                                VkAccelerationStructureKHR accelerationStructure,
                                                                                const VkAllocationCallbacks *pAllocator,
                                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-08934", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                                                                                 VkAccelerationStructureNV accelerationStructure,
                                                                                 size_t dataSize, void *pData,
                                                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    if (dataSize < 8) {
        skip |= LogError("VUID-vkGetAccelerationStructureHandleNV-dataSize-02240", accelerationStructure,
                         error_obj.location.dot(Field::dataSize), "must be greater than or equal to 8.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject &error_obj) const {
    bool skip = false;
    if (queryType != VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryType-06216", device, error_obj.location,
                         "queryType must be "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
            ValidatePipelineShaderStageCreateInfo(pCreateInfos[i].pStages[stage_index],
                                                  create_info_loc.dot(Field::pStages, stage_index));
        }
        auto feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651", device,
                             create_info_loc.dot(Field::stageCount),
                             "(%" PRIu32 ") must equal VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount (%" PRIu32
                             ").",
                             pCreateInfos[i].stageCount, feedback_struct->pipelineStageCreationFeedbackCount);
        }

        const auto *pipeline_create_flags = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfoKHR>(pCreateInfos[i].pNext);
        if (!pipeline_create_flags) {
            skip |= ValidateFlags(create_info_loc.dot(Field::flags), vvl::FlagBitmask::VkPipelineCreateFlagBits,
                                  AllVkPipelineCreateFlagBits, pCreateInfos[i].flags, kOptionalFlags,
                                  "VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        }

        const auto *vulkan_13_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
        const auto *pipeline_cache_contol_features =
            vku::FindStructInPNextChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if ((!vulkan_13_features || vulkan_13_features->pipelineCreationCacheControl == VK_FALSE) &&
            (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE)) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905", device,
                                 create_info_loc.dot(Field::flags),
                                 "is %s but the pipelineCreationCacheControl feature is not enabled.",
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-02904", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV) &&
            (pCreateInfos[i].flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT)) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-02957", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(
                        "VUID-VkRayTracingPipelineCreateInfoNV-flags-07986", device, create_info_loc.dot(Field::flags),
                        "is %s, %s is %" PRId32 ", but %s is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str(),
                        create_info_loc.dot(Field::basePipelineIndex).Fields().c_str(), pCreateInfos[i].basePipelineIndex,
                        create_info_loc.dot(Field::basePipelineHandle).Fields().c_str(),
                        FormatHandle(pCreateInfos[i].basePipelineHandle).c_str());
                }
                if (pCreateInfos[i].basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |=
                        LogError("VUID-vkCreateRayTracingPipelinesNV-flags-03415", device, create_info_loc.dot(Field::flags),
                                 "is %s, but %s is %" PRId32 ".", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str(),
                                 create_info_loc.dot(Field::basePipelineIndex).Fields().c_str(), pCreateInfos[i].basePipelineIndex);
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07985", device,
                                     create_info_loc.dot(Field::basePipelineHandle), "is VK_NULL_HANDLE, but %s is %s.",
                                     create_info_loc.dot(Field::flags).Fields().c_str(),
                                     string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
                }
            } else {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |=
                        LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07986", device, create_info_loc.dot(Field::flags),
                                 "is %s, but %s is %" PRId32 ".", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str(),
                                 create_info_loc.dot(Field::basePipelineIndex).Fields().c_str(), pCreateInfos[i].basePipelineIndex);
                }
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03456", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03458", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03459", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03460", device, create_info_loc.dot(Field::flags),
                             "is %s", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03461", device, create_info_loc.dot(Field::flags),
                             "is %s", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03462", device, create_info_loc.dot(Field::flags),
                             "is %s", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03463", device, create_info_loc.dot(Field::flags),
                             "is %s", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03588", device, create_info_loc.dot(Field::flags),
                             "is %s", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
            skip |= LogError("VUID-vkCreateRayTracingPipelinesNV-flags-03816", device, create_info_loc.dot(Field::flags), "is %s",
                             string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-flags-04948", device, create_info_loc.dot(Field::flags),
                             "is %s", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *raytracing_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipeline == VK_FALSE) {
        skip |= LogError("VUID-vkCreateRayTracingPipelinesKHR-rayTracingPipeline-03586", device, error_obj.location,
                         "The rayTracingPipeline feature was not enabled.");
    }
    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
            const Location stage_loc = create_info_loc.dot(Field::pStages, stage_index);
            ValidatePipelineShaderStageCreateInfo(pCreateInfos[i].pStages[stage_index], stage_loc);

            const auto stage = pCreateInfos[i].pStages[stage_index].stage;
            if ((stage & kShaderStageAllRayTracing) == 0) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-stage-06899", device, stage_loc.dot(Field::stage),
                                 "is %s.", string_VkShaderStageFlagBits(stage));
            }
        }

        if (!raytracing_features || (raytracing_features && raytracing_features->rayTraversalPrimitiveCulling == VK_FALSE)) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03596", device,
                                 create_info_loc.dot(Field::flags), "is %s.",
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03597", device,
                                 create_info_loc.dot(Field::flags), "is %s.",
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        auto feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(
                "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineStageCreationFeedbackCount-06652", device,
                create_info_loc.pNext(Struct::VkPipelineCreationFeedbackCreateInfo, Field::pipelineStageCreationFeedbackCount),
                "(%" PRIu32 ") is not equal to %s (%" PRIu32 ").", feedback_struct->pipelineStageCreationFeedbackCount,
                create_info_loc.Fields().c_str(), pCreateInfos[i].stageCount);
        }

        const auto *pipeline_create_flags = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfoKHR>(pCreateInfos[i].pNext);
        if (!pipeline_create_flags) {
            skip |= ValidateFlags(create_info_loc.dot(Field::flags), vvl::FlagBitmask::VkPipelineCreateFlagBits,
                                  AllVkPipelineCreateFlagBits, pCreateInfos[i].flags, kOptionalFlags,
                                  "VUID-VkRayTracingPipelineCreateInfoKHR-None-09497");
        }

        const auto *vulkan_13_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
        const auto *pipeline_cache_contol_features =
            vku::FindStructInPNextChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if ((!vulkan_13_features || vulkan_13_features->pipelineCreationCacheControl == VK_FALSE) &&
            (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE)) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905", device,
                                 create_info_loc.dot(Field::flags),
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            if (pCreateInfos[i].pLibraryInterface == NULL) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465", device, create_info_loc.dot(Field::flags),
                                 "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
            skip |= LogError("VUID-vkCreateRayTracingPipelinesKHR-flags-03816", device, create_info_loc.dot(Field::flags), "is %s.",
                             string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }

        for (uint32_t group_index = 0; group_index < pCreateInfos[i].groupCount; ++group_index) {
            const Location group_loc = create_info_loc.dot(Field::pGroups, group_index);
            if ((pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) ||
                (pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR)) {
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].anyHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470", device,
                                     create_info_loc.dot(Field::flags), "is %s, but %s is VK_SHADER_UNUSED_KHR.",
                                     string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str(),
                                     group_loc.dot(Field::anyHitShader).Fields().c_str());
                }
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].closestHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471", device,
                                     create_info_loc.dot(Field::flags), "is %s, but %s is VK_SHADER_UNUSED_KHR.",
                                     string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str(),
                                     group_loc.dot(Field::closestHitShader).Fields().c_str());
                }
            }
            if (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_TRUE &&
                pCreateInfos[i].pGroups[group_index].pShaderGroupCaptureReplayHandle) {
                if (!(pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |= LogError(
                        "VUID-VkRayTracingPipelineCreateInfoKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03599", device,
                        create_info_loc.dot(Field::flags), "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
                }
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(
                        "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07986", device, create_info_loc.dot(Field::basePipelineIndex),
                        "is %" PRId32 " and basePipelineHandle is not VK_NULL_HANDLE.", pCreateInfos[i].basePipelineIndex);
                }
                if (pCreateInfos[i].basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |= LogError("VUID-vkCreateRayTracingPipelinesKHR-flags-03415", device,
                                     create_info_loc.dot(Field::basePipelineIndex), "is %" PRId32 ".",
                                     pCreateInfos[i].basePipelineIndex);
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (pCreateInfos[i].basePipelineIndex < 0 ||
                    static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |=
                        LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-07985", device, create_info_loc.dot(Field::flags),
                                 "is %s but basePipelineIndex has invalid index value %" PRId32 ".",
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str(), pCreateInfos[i].basePipelineIndex);
                }
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR &&
            (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE)) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03598", device, create_info_loc.dot(Field::flags),
                             "is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
        }

        const bool library_enabled = IsExtEnabled(device_extensions.vk_khr_pipeline_library);
        if (!library_enabled) {
            if (pCreateInfos[i].pLibraryInfo) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595", device,
                                 create_info_loc.dot(Field::pLibraryInfo), "is not NULL.");
            }
            if (pCreateInfos[i].pLibraryInterface) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595", device,
                                 create_info_loc.dot(Field::pLibraryInterface), "is not NULL.");
            }
        }

        if (pCreateInfos[i].pLibraryInfo) {
            if ((pCreateInfos[i].pLibraryInfo->libraryCount > 0) && (pCreateInfos[i].pLibraryInterface == nullptr)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590", device,
                                 create_info_loc.dot(Field::pLibraryInfo).dot(Field::libraryCount),
                                 "is %" PRIu32 ", but pLibraryInterface is NULL.", pCreateInfos[i].pLibraryInfo->libraryCount);
            }
        }

        if (pCreateInfos[i].pLibraryInfo == nullptr) {
            if (pCreateInfos[i].stageCount == 0) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999", device,
                                 create_info_loc.dot(Field::pLibraryInfo), "is NULL and stageCount is zero.");
            }
            if (((pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) && (pCreateInfos[i].groupCount == 0)) {
                skip |=
                    LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700", device, create_info_loc.dot(Field::pLibraryInfo),
                             "is NULL and flags is %s.", string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        } else if (pCreateInfos[i].pLibraryInfo->libraryCount == 0) {
            if (pCreateInfos[i].stageCount == 0) {
                skip |=
                    LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999", device,
                             create_info_loc.dot(Field::pLibraryInfo).dot(Field::libraryCount), "is zero and stageCount is zero.");
            }
            if (((pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) && (pCreateInfos[i].groupCount == 0)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700", device,
                                 create_info_loc.dot(Field::pLibraryInfo).dot(Field::libraryCount), "is zero and flags is %s.",
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        if (pCreateInfos[i].pLibraryInterface) {
            if (pCreateInfos[i].pLibraryInterface->maxPipelineRayHitAttributeSize >
                phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize) {
                skip |= LogError(
                    "VUID-VkRayTracingPipelineInterfaceCreateInfoKHR-maxPipelineRayHitAttributeSize-03605", device,
                    create_info_loc.dot(Field::pLibraryInterface).dot(Field::maxPipelineRayHitAttributeSize),
                    "(%" PRIu32 ") is larger than VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayHitAttributeSize (%" PRIu32
                    ").",
                    pCreateInfos[i].pLibraryInterface->maxPipelineRayHitAttributeSize,
                    phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize);
            }
        }

        if (deferredOperation != VK_NULL_HANDLE) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT) {
                skip |= LogError("VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-03587", device,
                                 create_info_loc.dot(Field::flags), "is %s, but deferredOperation is not VK_NULL_HANDLE.",
                                 string_VkPipelineCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        if (pCreateInfos[i].pDynamicState) {
            for (uint32_t j = 0; j < pCreateInfos[i].pDynamicState->dynamicStateCount; ++j) {
                if (pCreateInfos[i].pDynamicState->pDynamicStates[j] != VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602", device,
                                     create_info_loc.dot(Field::pDynamicState).dot(Field::pDynamicStates, j), "is %s.",
                                     string_VkDynamicState(pCreateInfos[i].pDynamicState->pDynamicStates[j]));
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |= LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412", device, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyAccelerationStructureModeKHR(pInfo->mode));
    }
    const auto *acc_struct_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError("VUID-vkCopyAccelerationStructureToMemoryKHR-accelerationStructureHostCommands-03584", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }
    skip |= ValidateRequiredPointer(info_loc.dot(Field::dst).dot(Field::hostAddress), pInfo->dst.hostAddress,
                                    "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03732");
    if (SafeModulo((VkDeviceSize)pInfo->dst.hostAddress, 16) != 0) {
        skip |= LogError("VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03751", device,
                         info_loc.dot(Field::dst).dot(Field::hostAddress), "(0x%" PRIx64 ") must be aligned to 16 bytes.",
                         (VkDeviceAddress)pInfo->dst.hostAddress);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-accelerationStructure-08926", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |=  // to update VUID to VkCmdCopyAccelerationStructureToMemoryInfoKHR after spec update
            LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412", commandBuffer, info_loc.dot(Field::mode),
                     "is %s.", string_VkCopyAccelerationStructureModeKHR(pInfo->mode));
    }
    if (SafeModulo(pInfo->dst.deviceAddress, 256) != 0) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03740", commandBuffer,
                         info_loc.dot(Field::dst).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256 bytes.",
                         pInfo->dst.deviceAddress);
    }
    return skip;
}

bool StatelessValidation::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR &as_info,
                                                                   const VulkanTypedHandle &handle,
                                                                   const Location &info_loc) const {
    bool skip = false;
    if (!(as_info.mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR ||
          as_info.mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        const LogObjectList objlist(handle);
        skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-mode-03410", objlist, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyAccelerationStructureModeKHR(as_info.mode));
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureKHR(VkDevice device,
                                                                             VkDeferredOperationKHR deferredOperation,
                                                                             const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, error_obj.location.dot(Field::pInfo));
    const auto *acc_struct_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError("VUID-vkCopyAccelerationStructureKHR-accelerationStructureHostCommands-03582", device, error_obj.location,
                         "feature was not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                                const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureKHR-accelerationStructure-08925", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, error_obj.location.dot(Field::pInfo));
    return skip;
}

bool StatelessValidation::ValidateCopyMemoryToAccelerationStructureInfoKHR(
    const VkCopyMemoryToAccelerationStructureInfoKHR &as_info, const VulkanTypedHandle &handle, const Location &loc) const {
    bool skip = false;
    if (as_info.mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR) {
        const LogObjectList objlist(handle);
        skip |= LogError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-mode-03413", objlist, loc.dot(Field::mode), "is %s.",
                         string_VkCopyAccelerationStructureModeKHR(as_info.mode));
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);
    const auto *acc_struct_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError("VUID-vkCopyMemoryToAccelerationStructureKHR-accelerationStructureHostCommands-03583", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }
    skip |= ValidateRequiredPointer(info_loc.dot(Field::src).dot(Field::hostAddress), pInfo->src.hostAddress,
                                    "VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03729");

    if (SafeModulo((VkDeviceAddress)pInfo->src.hostAddress, 16) != 0) {
        skip |= LogError("VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03750", device,
                         info_loc.dot(Field::src).dot(Field::hostAddress), "(0x%" PRIx64 ") must be aligned to 16 bytes.",
                         (VkDeviceAddress)pInfo->src.hostAddress);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-accelerationStructure-08927", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);
    if (SafeModulo(pInfo->src.deviceAddress, 256) != 0) {
        skip |= LogError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03743", commandBuffer,
                         info_loc.dot(Field::src).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256 bytes.",
                         pInfo->src.deviceAddress);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-accelerationStructure-08924", commandBuffer,
                         error_obj.location, "accelerationStructure feature was not enabled.");
    }

    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR)) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742", commandBuffer,
                         error_obj.location.dot(Field::queryType), "(%s) is invalid.", string_VkQueryType(queryType));
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void *pData, size_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *acc_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_structure_features || acc_structure_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructureHostCommands-03585", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }
    if (dataSize < accelerationStructureCount * stride) {
        skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-dataSize-03452", device,
                         error_obj.location.dot(Field::dataSize),
                         "(%zu) is less than "
                         "accelerationStructureCount (%" PRIu32 ") x stride (%zu).",
                         dataSize, accelerationStructureCount, stride);
    }
    const Location query_type_loc = error_obj.location.dot(Field::queryType);
    const Location data_size_loc = error_obj.location.dot(Field::dataSize);
    if (dataSize < sizeof(VkDeviceSize)) {
        switch (queryType) {
            case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
                skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03449", device, query_type_loc,
                                 "is %s, but %s is %zu.", string_VkQueryType(queryType), data_size_loc.Fields().c_str(), dataSize);
                break;
            case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
                skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03451", device, query_type_loc,
                                 "is %s, but %s is %zu.", string_VkQueryType(queryType), data_size_loc.Fields().c_str(), dataSize);
                break;
            case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
                skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06734", device, query_type_loc,
                                 "is %s, but %s is %zu.", string_VkQueryType(queryType), data_size_loc.Fields().c_str(), dataSize);
                break;
            case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
                skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06732", device, query_type_loc,
                                 "is %s, but %s is %zu.", string_VkQueryType(queryType), data_size_loc.Fields().c_str(), dataSize);
                break;
            default:
                break;
        }
    }

    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR)) {
        skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742", device,
                         error_obj.location.dot(Field::queryType), "is %s.", string_VkQueryType(queryType));
    }

    if (SafeModulo(stride, sizeof(VkDeviceSize)) != 0) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448", device,
                             error_obj.location.dot(Field::queryType),
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, but then stride (%zu) must be a multiple "
                             "of the size of VkDeviceSize.",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450", device,
                             error_obj.location.dot(Field::queryType),
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, but then stride (%zu) must be a "
                             "multiple of the size of VkDeviceSize.",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731", device,
                             error_obj.location.dot(Field::queryType),
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but then stride (%zu) must be a multiple of the "
                             "size of VkDeviceSize.",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733", device,
                             error_obj.location.dot(Field::queryType),
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but then stride "
                             "(%zu) must be a multiple of the size of VkDeviceSize.",
                             stride);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData,
    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *raytracing_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE) {
        skip |= LogError(
            "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03606", device,
            error_obj.location, "rayTracingPipelineShaderGroupHandleCaptureReplay feature was not enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
    VkAccelerationStructureCompatibilityKHR *pCompatibility, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *accel_struct_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!(accel_struct_features && accel_struct_features->accelerationStructure)) {
        skip |= LogError("VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-accelerationStructure-08928", device,
                         error_obj.location, "accelerationStructure feature was not enabled.");
    }
    return skip;
}

bool StatelessValidation::ValidateTotalPrimitivesCount(uint64_t total_triangles_count, uint64_t total_aabbs_count,
                                                       const VulkanTypedHandle &handle, const Location &loc) const {
    bool skip = false;

    if (total_triangles_count > phys_dev_ext_props.acc_structure_props.maxPrimitiveCount) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795", handle, loc,
                         "total number of triangles in all geometries (%" PRIu64
                         ") is larger than maxPrimitiveCount "
                         "(%" PRIu64 ")",
                         total_triangles_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
    }

    if (total_aabbs_count > phys_dev_ext_props.acc_structure_props.maxPrimitiveCount) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794", handle, loc,
                         "total number of AABBs in all geometries (%" PRIu64
                         ") is larger than maxPrimitiveCount "
                         "(%" PRIu64 ")",
                         total_aabbs_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
    }

    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureBuildGeometryInfoKHR(const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                                                            const VulkanTypedHandle &handle,
                                                                            const Location &info_loc) const {
    bool skip = false;

    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654", handle, info_loc.dot(Field::type),
                         "must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-03796", handle, info_loc.dot(Field::flags),
                         "has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR bit set,"
                         "then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR bit set.");
    }
    if (info.pGeometries && info.ppGeometries) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788", handle, info_loc,
                         "both pGeometries and ppGeometries are not NULL.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && info.geometryCount != 1) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03790", handle, info_loc.dot(Field::type),
                         "is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but geometryCount is %" PRIu32 ".", info.geometryCount);
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
        info.geometryCount > phys_dev_ext_props.acc_structure_props.maxGeometryCount) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03793", handle, info_loc.dot(Field::type),
                         "is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR but geometryCount (%" PRIu32
                         ") is greater than maxGeometryCount (%" PRIu64 ").",
                         info.geometryCount, phys_dev_ext_props.acc_structure_props.maxGeometryCount);
    }

    if (info.geometryCount > 0 && !info.pGeometries && !info.ppGeometries) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788", handle,
                         info_loc.dot(Field::geometryCount), "is (%" PRIu32 ") but both pGeometries and ppGeometries are NULL.",
                         info.geometryCount);
        return skip;
    }

    for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
        const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(info, geom_i);

        const Location geometry_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);

        skip |= ValidateRangedEnum(geometry_loc.dot(Field::geometryType), vvl::Enum::VkGeometryTypeKHR, geom.geometryType,
                                   "VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
        if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
            constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT};

            skip |=
                ValidateStructType(geometry_loc.dot(Field::geometry).dot(Field::triangles),
                                   "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                                   &(geom.geometry.triangles), VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                                   false, kVUIDUndefined, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-sType-sType");
            skip |= ValidateStructPnext(geometry_loc.dot(Field::geometry).dot(Field::triangles), geom.geometry.triangles.pNext,
                                        allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
                                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-pNext-pNext", kVUIDUndefined);
            skip |= ValidateRangedEnum(geometry_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::vertexFormat),
                                       vvl::Enum::VkFormat, geom.geometry.triangles.vertexFormat,
                                       "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-parameter");
            skip |=
                ValidateStructType(geometry_loc.dot(Field::geometry).dot(Field::triangles),
                                   "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR", &geom.geometry.triangles,
                                   VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, true,
                                   "VUID-VkAccelerationStructureGeometryKHR-triangles-parameter", kVUIDUndefined);
            skip |= ValidateRangedEnum(geometry_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::indexType),
                                       vvl::Enum::VkIndexType, geom.geometry.triangles.indexType,
                                       "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-parameter");

            if (geom.geometry.triangles.vertexStride > vvl::kU32Max) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819", handle,
                                 geometry_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::vertexStride),
                                 "(%" PRIu64 ") must be less than or equal to 2^32-1.", geom.geometry.triangles.vertexStride);
            }
            if (geom.geometry.triangles.indexType != VK_INDEX_TYPE_UINT16 &&
                geom.geometry.triangles.indexType != VK_INDEX_TYPE_UINT32 &&
                geom.geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798", handle,
                                 geometry_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::indexType), "is %s.",
                                 string_VkIndexType(geom.geometry.triangles.indexType));
            }
        }
        if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
            skip |=
                ValidateStructType(geometry_loc.dot(Field::geometry).dot(Field::instances),
                                   "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR", &geom.geometry.instances,
                                   VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, true,
                                   "VUID-VkAccelerationStructureGeometryKHR-instances-parameter", kVUIDUndefined);
            skip |=
                ValidateStructType(geometry_loc.dot(Field::geometry).dot(Field::instances),
                                   "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                                   &(geom.geometry.instances), VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                                   false, kVUIDUndefined, "VUID-VkAccelerationStructureGeometryInstancesDataKHR-sType-sType");
            skip |= ValidateStructPnext(geometry_loc.dot(Field::geometry).dot(Field::instances), geom.geometry.instances.pNext, 0,
                                        nullptr, GeneratedVulkanHeaderVersion,
                                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-pNext-pNext", kVUIDUndefined);

            skip |= ValidateBool32(geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::arrayOfPointers),
                                   geom.geometry.instances.arrayOfPointers);
        }
        if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
            skip |= ValidateStructType(geometry_loc.dot(Field::geometry).dot(Field::aabbs),
                                       "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR", &geom.geometry.aabbs,
                                       VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, true,
                                       "VUID-VkAccelerationStructureGeometryKHR-aabbs-parameter", kVUIDUndefined);
            skip |= ValidateStructType(geometry_loc.dot(Field::geometry).dot(Field::aabbs),
                                       "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR", &(geom.geometry.aabbs),
                                       VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, false, kVUIDUndefined,
                                       "VUID-VkAccelerationStructureGeometryAabbsDataKHR-sType-sType");
            skip |= ValidateStructPnext(geometry_loc.dot(Field::geometry).dot(Field::aabbs), geom.geometry.aabbs.pNext, 0, nullptr,
                                        GeneratedVulkanHeaderVersion,
                                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-pNext-pNext", kVUIDUndefined);
            if (geom.geometry.aabbs.stride % 8) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545", handle,
                                 geometry_loc.dot(Field::geometry).dot(Field::aabbs).dot(Field::stride),
                                 "(%" PRIu64 ") is not a multiple of 8.", geom.geometry.aabbs.stride);
            }
            if (geom.geometry.aabbs.stride > vvl::kU32Max) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820", handle,
                                 geometry_loc.dot(Field::geometry).dot(Field::aabbs).dot(Field::stride),
                                 "(%" PRIu64 ") must be less than or equal to 2^32-1.", geom.geometry.aabbs.stride);
            }
        }
        if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && geom.geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
            skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789", handle,
                             geometry_loc.dot(Field::geometryType), "is %s but %s is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR.",
                             string_VkGeometryTypeKHR(geom.geometryType), info_loc.dot(Field::type).Fields().c_str());
        }
        if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
            if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791", handle,
                                 geometry_loc.dot(Field::geometryType),
                                 "is %s but %s is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR.",
                                 string_VkGeometryTypeKHR(geom.geometryType), info_loc.dot(Field::type).Fields().c_str());
            }
            if (geom.geometryType != rt::GetGeometry(info, 0).geometryType) {
                skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792", handle,
                                 geometry_loc.dot(Field::geometryType), "(%s) is different than pGeometries[0].geometryType (%s)",
                                 string_VkGeometryTypeKHR(geom.geometryType),
                                 string_VkGeometryTypeKHR(rt::GetGeometry(info, 0).geometryType));
            }
        }
    }

    return skip;
}

static void ComputeTotalPrimitiveCountWithBuildRanges(uint32_t info_count,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_infos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR *const *build_ranges,
                                                      uint64_t *out_total_triangles_count, uint64_t *out_total_aabbs_count) {
    *out_total_triangles_count = 0;
    *out_total_aabbs_count = 0;

    for (const auto [info_i, info] : vvl::enumerate(build_geometry_infos, info_count)) {
        if (!info->pGeometries && !info->ppGeometries) {
            *out_total_triangles_count = 0;
            *out_total_aabbs_count = 0;
            return;
        }

        for (uint32_t geom_i = 0; geom_i < info->geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(*info, geom_i);
            switch (geom.geometryType) {
                case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
                    *out_total_triangles_count += build_ranges[info_i][geom_i].primitiveCount;
                    break;
                case VK_GEOMETRY_TYPE_AABBS_KHR:
                    *out_total_aabbs_count += build_ranges[info_i][geom_i].primitiveCount;
                    break;
                default:
                    break;
            }
        }
    }

    return;
}

static void ComputeTotalPrimitiveCountWithMaxPrimitivesCount(
    uint32_t info_count, const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_infos,
    const uint32_t *const *max_primitives, uint64_t *out_total_triangles_count, uint64_t *out_total_aabbs_count) {
    *out_total_triangles_count = 0;
    *out_total_aabbs_count = 0;

    for (const auto [info_i, info] : vvl::enumerate(build_geometry_infos, info_count)) {
        if (!info->pGeometries && !info->ppGeometries) {
            *out_total_triangles_count = 0;
            *out_total_aabbs_count = 0;
            return;
        }

        for (uint32_t geom_i = 0; geom_i < info->geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(*info, geom_i);
            switch (geom.geometryType) {
                case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
                    *out_total_triangles_count += max_primitives[info_i][geom_i];
                    break;
                case VK_GEOMETRY_TYPE_AABBS_KHR:
                    *out_total_aabbs_count += max_primitives[info_i][geom_i];
                    break;
                default:
                    break;
            }
        }
    }

    return;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features || acceleration_structure_features->accelerationStructure == VK_FALSE) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-accelerationStructure-08923", commandBuffer, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithBuildRanges(infoCount, pInfos, ppBuildRangeInfos, &total_triangles_count, &total_aabbs_count);
    ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(*info, error_obj.handle, info_loc);

        if (SafeModulo(info->scratchData.deviceAddress,
                       phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment) != 0) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03710", commandBuffer,
                             info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                             "(%" PRIu64 ") must be a multiple of minAccelerationStructureScratchOffsetAlignment (%" PRIu32 ").",
                             info->scratchData.deviceAddress,
                             phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment);
        }
        skip |= ValidateRangedEnum(info_loc.dot(Field::mode), vvl::Enum::VkBuildAccelerationStructureModeKHR, info->mode,
                                   "VUID-vkCmdBuildAccelerationStructuresKHR-mode-04628");
        if (info->mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && info->srcAccelerationStructure == VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630", commandBuffer, info_loc.dot(Field::mode),
                             "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but srcAccelerationStructure is VK_NULL_HANDLE.");
        }

        for (const auto [other_info_j, other_info] : vvl::enumerate(pInfos, infoCount)) {
            if (info_i == other_info_j) continue;
            if (info->dstAccelerationStructure == other_info->dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info->dstAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03698", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info->dstAccelerationStructure).c_str());
                break;
            }
            if (info->srcAccelerationStructure == other_info->dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info->srcAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03403", objlist,
                                 info_loc.dot(Field::srcAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info->srcAccelerationStructure).c_str());
                break;
            }
        }

            for (uint32_t geom_i = 0; geom_i < info->geometryCount; ++geom_i) {
                const VkAccelerationStructureGeometryKHR &as_geometry = rt::GetGeometry(*info, geom_i);
                const Location p_geom_loc = info_loc.dot(info->pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
                const Location p_geom_geom_loc = p_geom_loc.dot(Field::geometry);
                switch (as_geometry.geometryType) {
                    case VK_GEOMETRY_TYPE_TRIANGLES_KHR: {
                        const VkDeviceSize index_buffer_alignment = GetIndexAlignment(as_geometry.geometry.triangles.indexType);
                        if (SafeModulo(as_geometry.geometry.triangles.indexData.deviceAddress, index_buffer_alignment) != 0) {
                            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03712", commandBuffer,
                                             p_geom_geom_loc.dot(Field::triangles).dot(Field::indexData).dot(Field::deviceAddress),
                                             "(0x%" PRIx64
                                             ") is not aligned to the size in bytes of its corresponding index type (%s).",
                                             as_geometry.geometry.triangles.indexData.deviceAddress,
                                             string_VkIndexType(as_geometry.geometry.triangles.indexType));
                        }

                        if (SafeModulo(as_geometry.geometry.triangles.transformData.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03810", commandBuffer,
                                p_geom_geom_loc.dot(Field::triangles).dot(Field::transformData).dot(Field::deviceAddress),
                                "(%" PRIu64 ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_TRIANGLES_KHR.",
                                as_geometry.geometry.triangles.transformData.deviceAddress);
                        }

                        break;
                    }
                    case VK_GEOMETRY_TYPE_AABBS_KHR: {
                        if (SafeModulo(as_geometry.geometry.aabbs.data.deviceAddress, 8) != 0) {
                            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03714", commandBuffer,
                                             p_geom_geom_loc.dot(Field::aabbs).dot(Field::data).dot(Field::deviceAddress),
                                             "(0x%" PRIx64
                                             ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_AABBS_KHR.",
                                             as_geometry.geometry.aabbs.data.deviceAddress);
                        }
                        break;
                    }
                    case VK_GEOMETRY_TYPE_INSTANCES_KHR: {
                        if (as_geometry.geometry.instances.arrayOfPointers == VK_TRUE) {
                            if (SafeModulo(as_geometry.geometry.instances.data.deviceAddress, 8) != 0) {
                                skip |=
                                    LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03716", commandBuffer,
                                             p_geom_geom_loc.dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                             "(%" PRIu64
                                             ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                             "geometry.instances.arrayOfPointers is "
                                             "VK_TRUE.",
                                             as_geometry.geometry.instances.data.deviceAddress);
                            }
                        } else {
                            if (SafeModulo(as_geometry.geometry.instances.data.deviceAddress, 16) != 0) {
                                skip |= LogError(
                                    "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03715", commandBuffer,
                                    p_geom_geom_loc.dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                    "(%" PRIu64
                                    ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                    "geometry.instances.arrayOfPointers is VK_FALSE.",
                                    as_geometry.geometry.instances.data.deviceAddress);
                            }
                        }
                        const Location p_build_range_loc = error_obj.location.dot(Field::ppBuildRangeInfos, info_i);
                        for (const auto [build_range_i, build_range] :
                             vvl::enumerate(ppBuildRangeInfos[geom_i], info->geometryCount)) {
                            if (build_range->primitiveCount > phys_dev_ext_props.acc_structure_props.maxInstanceCount) {
                                skip |= LogError(
                                    "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03801", commandBuffer, p_build_range_loc,
                                    "[%" PRIu32 "].primitiveCount (%" PRIu32
                                    ") is superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxInstanceCount "
                                    "(%" PRIu64 ").",
                                    build_range_i, build_range->primitiveCount,
                                    phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
        }
        skip |= ValidateArray(info_loc.dot(Field::geometryCount), error_obj.location.dot(Field::ppBuildRangeInfos, info_i),
                              info->geometryCount, &ppBuildRangeInfos[info_i], false, true, kVUIDUndefined,
                              "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-03676");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts,
    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *ray_tracing_acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!ray_tracing_acceleration_structure_features ||
        ray_tracing_acceleration_structure_features->accelerationStructureIndirectBuild == VK_FALSE) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-accelerationStructureIndirectBuild-03650", commandBuffer,
                         error_obj.location, "the accelerationStructureIndirectBuild feature was not enabled.");
    }

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithMaxPrimitivesCount(infoCount, pInfos, ppMaxPrimitiveCounts, &total_triangles_count,
                                                     &total_aabbs_count);
    ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos[info_i], error_obj.handle, info_loc);

        if (SafeModulo(info->scratchData.deviceAddress,
                       phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment) != 0) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03710", commandBuffer,
                             info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                             "(%" PRIu64 ") must be a multiple of minAccelerationStructureScratchOffsetAlignment (%" PRIu32 ").",
                             info->scratchData.deviceAddress,
                             phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment);
        }
        skip |= ValidateRangedEnum(info_loc.dot(Field::mode), vvl::Enum::VkBuildAccelerationStructureModeKHR, info->mode,
                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-mode-04628");

        if (info->mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && info->srcAccelerationStructure == VK_NULL_HANDLE) {
            skip |=
                LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-04630", commandBuffer, info_loc.dot(Field::mode),
                         "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but srcAccelerationStructure is VK_NULL_HANDLE.");
        }

        for (uint32_t info_k = 0; info_k < infoCount; ++info_k) {
            if (info_i == info_k) continue;
            if (info->dstAccelerationStructure == pInfos[info_k].dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info->dstAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03698", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", info_k,
                                 FormatHandle(info->dstAccelerationStructure).c_str());
                break;
            }
            if (info->srcAccelerationStructure == pInfos[info_k].dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info->srcAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03403", objlist,
                                 info_loc.dot(Field::srcAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", info_k,
                                 FormatHandle(info->srcAccelerationStructure).c_str());
                break;
            }
        }
        for (uint32_t j = 0; j < info->geometryCount; ++j) {
            if (info->pGeometries) {
                const VkAccelerationStructureGeometryKHR &as_geometry = info->pGeometries[j];
                const Location geometry_loc = error_obj.location.dot(Field::pGeometries, j);
                if (as_geometry.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (as_geometry.geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(as_geometry.geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716", commandBuffer,
                                geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                "(%" PRIu64
                                ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                "geometry.instances.arrayOfPointers is "
                                "VK_TRUE.",
                                as_geometry.geometry.instances.data.deviceAddress);
                        }
                    } else {
                        if (SafeModulo(as_geometry.geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715", commandBuffer,
                                geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                "(%" PRIu64
                                ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                "geometry.instances.arrayOfPointers is VK_FALSE.",
                                as_geometry.geometry.instances.data.deviceAddress);
                        }
                    }
                }
                if (as_geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(as_geometry.geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714", commandBuffer,
                            geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                            "(%" PRIu64 ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_AABBS_KHR.",
                            as_geometry.geometry.instances.data.deviceAddress);
                    }
                }
                if (as_geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    const VkDeviceSize index_buffer_alignment = GetIndexAlignment(as_geometry.geometry.triangles.indexType);
                    if (SafeModulo(as_geometry.geometry.triangles.indexData.deviceAddress, index_buffer_alignment) != 0) {
                        skip |= LogError(
                            "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03712", commandBuffer,
                            geometry_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::indexData).dot(Field::deviceAddress),
                            "(0x%" PRIx64 ") is not aligned to the size in bytes of its corresponding index type (%s).",
                            as_geometry.geometry.triangles.indexData.deviceAddress,
                            string_VkIndexType(as_geometry.geometry.triangles.indexType));
                    }

                    if (SafeModulo(as_geometry.geometry.triangles.indexData.deviceAddress, 16) != 0) {
                        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810", commandBuffer,
                                         geometry_loc.dot(Field::geometry)
                                             .dot(Field::triangles)
                                             .dot(Field::transformData)
                                             .dot(Field::deviceAddress),
                                         "(%" PRIu64
                                         ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_TRIANGLES_KHR.",
                                         as_geometry.geometry.triangles.transformData.deviceAddress);
                    }
                }
            } else if (info->ppGeometries) {
                const VkAccelerationStructureGeometryKHR *as_geometry = info->ppGeometries[j];
                const Location geometry_loc = error_obj.location.dot(Field::ppGeometries, j);
                if (as_geometry->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (as_geometry->geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(as_geometry->geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716", commandBuffer,
                                geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                "(%" PRIu64
                                ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                "geometry.instances.arrayOfPointers is "
                                "VK_TRUE.",
                                info->pGeometries[j].geometry.instances.data.deviceAddress);
                        }
                    } else {
                        if (SafeModulo(as_geometry->geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715", commandBuffer,
                                geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                "(%" PRIu64
                                ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                "geometry.instances.arrayOfPointers is VK_FALSE.",
                                info->pGeometries[j].geometry.instances.data.deviceAddress);
                        }
                    }
                }
                if (as_geometry->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(as_geometry->geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714", commandBuffer,
                            geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                            "(%" PRIu64 ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_AABBS_KHR.",
                            info->pGeometries[j].geometry.instances.data.deviceAddress);
                    }
                }
                if (as_geometry->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(as_geometry->geometry.triangles.indexData.deviceAddress, 16) != 0) {
                        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810", commandBuffer,
                                         geometry_loc.dot(Field::geometry)
                                             .dot(Field::triangles)
                                             .dot(Field::transformData)
                                             .dot(Field::deviceAddress),
                                         "(%" PRIu64
                                         ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_TRIANGLES_KHR.",
                                         info->pGeometries[j].geometry.triangles.transformData.deviceAddress);
                    }
                }
            }
        }

        if (SafeModulo(pIndirectDeviceAddresses[info_i], 4) != 0) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pIndirectDeviceAddresses-03648", commandBuffer,
                             error_obj.location.dot(Field::pIndirectStrides, info_i), "is 0x%" PRIx64 ".",
                             pIndirectDeviceAddresses[info_i]);
        }

        if (SafeModulo(pIndirectStrides[info_i], 4) != 0) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pIndirectStrides-03787", commandBuffer,
                             error_obj.location.dot(Field::pIndirectStrides, info_i), "is %" PRIu32 ".", pIndirectStrides[info_i]);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *ray_tracing_acceleration_structure_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!ray_tracing_acceleration_structure_features ||
        ray_tracing_acceleration_structure_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-accelerationStructureHostCommands-03581", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithBuildRanges(infoCount, pInfos, ppBuildRangeInfos, &total_triangles_count, &total_aabbs_count);
    ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(*info, error_obj.handle, error_obj.location);

        skip |= ValidateRangedEnum(info_loc.dot(Field::mode), vvl::Enum::VkBuildAccelerationStructureModeKHR, info->mode,
                                   "VUID-vkBuildAccelerationStructuresKHR-mode-04628");

        skip |= ValidateArray(info_loc.dot(Field::geometryCount), error_obj.location.dot(Field::ppBuildRangeInfos, info_i),
                              info->geometryCount, &ppBuildRangeInfos[info_i], false, true, kVUIDUndefined,
                              "VUID-vkBuildAccelerationStructuresKHR-ppBuildRangeInfos-03676");

        if (info->mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR) {
            if (info->scratchData.hostAddress == nullptr) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03725", device,
                                 info_loc.dot(Field::scratchData).dot(Field::hostAddress), "is NULL.");
            }
        } else if (info->mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
            if (info->scratchData.hostAddress == nullptr) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03726", device,
                                 info_loc.dot(Field::scratchData).dot(Field::hostAddress), "is NULL.");
            }

            if (info->srcAccelerationStructure == VK_NULL_HANDLE) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-04630", device, info_loc.dot(Field::mode),
                                 "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but %s is VK_NULL_HANDLE.",
                                 info_loc.dot(Field::srcAccelerationStructure).Fields().c_str());
            }
        }

        for (uint32_t geom_i = 0; geom_i < info->geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(*info, geom_i);
            switch (geom.geometryType) {
                case VK_GEOMETRY_TYPE_TRIANGLES_KHR: {
                    if (geom.geometry.triangles.vertexData.hostAddress == nullptr) {
                        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03771", device,
                                         info_loc.dot(Field::triangles).dot(Field::vertexData).dot(Field::hostAddress), "is NULL.");
                    }
                    if (geom.geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                        if (geom.geometry.triangles.indexData.hostAddress == nullptr) {
                            skip |=
                                LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03772", device,
                                         info_loc.dot(Field::triangles).dot(Field::indexData).dot(Field::hostAddress), "is NULL.");
                        }
                    }
                    break;
                }
                case VK_GEOMETRY_TYPE_AABBS_KHR: {
                    if (geom.geometry.aabbs.data.hostAddress == nullptr) {
                        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03774", device,
                                         info_loc.dot(Field::aabbs).dot(Field::data).dot(Field::hostAddress), "is NULL.");
                    }
                    break;
                }
                case VK_GEOMETRY_TYPE_INSTANCES_KHR: {
                    if (geom.geometry.instances.data.hostAddress == nullptr) {
                        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03778", device,
                                         info_loc.dot(Field::instances).dot(Field::data).dot(Field::hostAddress), "is NULL.");
                    }
                    break;
                }

                default:
                    break;
            }
        }

        for (uint32_t info_k = 0; info_k < infoCount; ++info_k) {
            if (info_i == info_k) continue;
            if (info->dstAccelerationStructure == pInfos[info_k].dstAccelerationStructure) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03698", device,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", info_k,
                                 FormatHandle(info->dstAccelerationStructure).c_str());
                break;
            }
            if (info->srcAccelerationStructure == pInfos[info_k].dstAccelerationStructure) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03403", device,
                                 info_loc.dot(Field::srcAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", info_k,
                                 FormatHandle(info->srcAccelerationStructure).c_str());
                break;
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
    const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithMaxPrimitivesCount(1, pBuildInfo, &pMaxPrimitiveCounts, &total_triangles_count,
                                                     &total_aabbs_count);
    ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(*pBuildInfo, error_obj.handle,
                                                              error_obj.location.dot(Field::pBuildInfo, 0));
    const auto *accel_struct_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!(accel_struct_features && accel_struct_features->accelerationStructure)) {
        skip |= LogError("VUID-vkGetAccelerationStructureBuildSizesKHR-accelerationStructure-08933", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    if (pBuildInfo) {
        if (pBuildInfo->geometryCount != 0 && !pMaxPrimitiveCounts) {
            skip |= LogError("VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03619", device,
                             error_obj.location.dot(Field::pBuildInfo).dot(Field::geometryCount),
                             "is %" PRIu32 ", but pMaxPrimitiveCounts is NULL.", pBuildInfo->geometryCount);
        }

        if (pMaxPrimitiveCounts && (pBuildInfo->pGeometries || pBuildInfo->ppGeometries)) {
            for (uint32_t geom_i = 0; geom_i < pBuildInfo->geometryCount; ++geom_i) {
                const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(*pBuildInfo, geom_i);
                switch (geom.geometryType) {
                    case VK_GEOMETRY_TYPE_INSTANCES_KHR: {
                        if (pMaxPrimitiveCounts[geom_i] > phys_dev_ext_props.acc_structure_props.maxInstanceCount) {
                            const Field p_geom = pBuildInfo->pGeometries ? Field::pGeometries : Field::ppGeometries;
                            skip |= LogError(
                                "VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03785", device,
                                error_obj.location.dot(Field::pBuildInfo).dot(p_geom, geom_i).dot(Field::geometryType),
                                "is %s, but pMaxPrimitiveCount[%" PRIu32 "] (%" PRIu32
                                ") is larger than VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxInstanceCount (%" PRIu64
                                ").",
                                string_VkGeometryTypeKHR(geom.geometryType), geom_i, pMaxPrimitiveCounts[geom_i],
                                phys_dev_ext_props.acc_structure_props.maxInstanceCount);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    return skip;
}
