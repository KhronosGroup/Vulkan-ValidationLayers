/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

bool StatelessValidation::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles,
                                                      VkAccelerationStructureNV object_handle, const char *func_name) const {
    bool skip = false;

    if (triangles.vertexFormat != VK_FORMAT_R32G32B32_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16B16_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16B16_SNORM && triangles.vertexFormat != VK_FORMAT_R32G32_SFLOAT &&
        triangles.vertexFormat != VK_FORMAT_R16G16_SFLOAT && triangles.vertexFormat != VK_FORMAT_R16G16_SNORM) {
        skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-vertexFormat-02430", "%s", func_name);
    } else {
        uint32_t vertex_component_size = 0;
        if (triangles.vertexFormat == VK_FORMAT_R32G32B32_SFLOAT || triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT) {
            vertex_component_size = 4;
        } else if (triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16B16_SNORM ||
                   triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT || triangles.vertexFormat == VK_FORMAT_R16G16_SNORM) {
            vertex_component_size = 2;
        }
        if (vertex_component_size > 0 && SafeModulo(triangles.vertexOffset, vertex_component_size) != 0) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-vertexOffset-02429", "%s", func_name);
        }
    }

    if (triangles.indexType != VK_INDEX_TYPE_UINT32 && triangles.indexType != VK_INDEX_TYPE_UINT16 &&
        triangles.indexType != VK_INDEX_TYPE_NONE_NV) {
        skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexType-02433", "%s", func_name);
    } else {
        const uint32_t index_element_size = GetIndexAlignment(triangles.indexType);
        if (index_element_size > 0 && SafeModulo(triangles.indexOffset, index_element_size) != 0) {
            skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexOffset-02432", "%s", func_name);
        }

        if (triangles.indexType == VK_INDEX_TYPE_NONE_NV) {
            if (triangles.indexCount != 0) {
                skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexCount-02436", "%s", func_name);
            }
            if (triangles.indexData != VK_NULL_HANDLE) {
                skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-indexData-02434", "%s", func_name);
            }
        }
    }

    if (SafeModulo(triangles.transformOffset, 16) != 0) {
        skip |= LogError(object_handle, "VUID-VkGeometryTrianglesNV-transformOffset-02438", "%s", func_name);
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, VkAccelerationStructureNV object_handle,
                                                 const char *func_name) const {
    bool skip = false;

    if (SafeModulo(aabbs.offset, 8) != 0) {
        skip |= LogError(object_handle, "VUID-VkGeometryAABBNV-offset-02440", "%s", func_name);
    }
    if (SafeModulo(aabbs.stride, 8) != 0) {
        skip |= LogError(object_handle, "VUID-VkGeometryAABBNV-stride-02441", "%s", func_name);
    }

    return skip;
}

bool StatelessValidation::ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle,
                                             const char *func_name) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip = ValidateGeometryTrianglesNV(geometry.geometry.triangles, object_handle, func_name);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip = ValidateGeometryAABBNV(geometry.geometry.aabbs, object_handle, func_name);
    }
    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV &info,
                                                              VkAccelerationStructureNV object_handle, const char *func_name,
                                                              bool is_cmd) const {
    bool skip = false;
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV && info.geometryCount != 0) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-type-02425",
                         "VkAccelerationStructureInfoNV: If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV then "
                         "geometryCount must be 0.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.instanceCount != 0) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-type-02426",
                         "VkAccelerationStructureInfoNV: If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV then "
                         "instanceCount must be 0.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-type-04623",
                         "VkAccelerationStructureInfoNV: type is invalid VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-flags-02592",
                         "VkAccelerationStructureInfoNV: If flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV"
                         "bit set, then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV bit set.");
    }
    if (info.geometryCount > phys_dev_ext_props.ray_tracing_props_nv.maxGeometryCount) {
        skip |= LogError(object_handle,
                         is_cmd ? "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241"
                                : "VUID-VkAccelerationStructureInfoNV-geometryCount-02422",
                         "VkAccelerationStructureInfoNV: geometryCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.");
    }
    if (info.instanceCount > phys_dev_ext_props.ray_tracing_props_nv.maxInstanceCount) {
        skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-instanceCount-02423",
                         "VkAccelerationStructureInfoNV: instanceCount must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxInstanceCount.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 0) {
        uint64_t total_triangle_count = 0;
        for (uint32_t i = 0; i < info.geometryCount; i++) {
            const VkGeometryNV &geometry = info.pGeometries[i];

            skip |= ValidateGeometryNV(geometry, object_handle, func_name);

            if (geometry.geometryType != VK_GEOMETRY_TYPE_TRIANGLES_NV) {
                continue;
            }
            total_triangle_count += geometry.geometry.triangles.indexCount / 3;
        }
        if (total_triangle_count > phys_dev_ext_props.ray_tracing_props_nv.maxTriangleCount) {
            skip |= LogError(object_handle, "VUID-VkAccelerationStructureInfoNV-maxTriangleCount-02424",
                             "VkAccelerationStructureInfoNV: The total number of triangles in all geometries must be less than "
                             "or equal to VkPhysicalDeviceRayTracingPropertiesNV::maxTriangleCount.");
        }
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV && info.geometryCount > 1) {
        const VkGeometryTypeNV first_geometry_type = info.pGeometries[0].geometryType;
        for (uint32_t i = 1; i < info.geometryCount; i++) {
            const VkGeometryNV &geometry = info.pGeometries[i];
            if (geometry.geometryType != first_geometry_type) {
                skip |= LogError(device, "VUID-VkAccelerationStructureInfoNV-type-02786",
                                 "VkAccelerationStructureInfoNV: info.pGeometries[%" PRIu32
                                 "].geometryType does not match "
                                 "info.pGeometries[0].geometryType.",
                                 i);
            }
        }
    }
    for (uint32_t geometry_index = 0; geometry_index < info.geometryCount; ++geometry_index) {
        if (!(info.pGeometries[geometry_index].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV ||
              info.pGeometries[geometry_index].geometryType == VK_GEOMETRY_TYPE_AABBS_NV)) {
            skip |= LogError(device, "VUID-VkGeometryNV-geometryType-03503",
                             "VkGeometryNV: geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV"
                             "or VK_GEOMETRY_TYPE_AABBS_NV.");
        }
    }
    skip |=
        ValidateFlags(func_name, "info.flags", "VkBuildAccelerationStructureFlagBitsNV", AllVkBuildAccelerationStructureFlagBitsNV,
                      info.flags, kOptionalFlags, "VUID-VkAccelerationStructureInfoNV-flags-parameter");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureNV(
    VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureNV *pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo) {
        if ((pCreateInfo->compactedSize != 0) &&
            ((pCreateInfo->info.geometryCount != 0) || (pCreateInfo->info.instanceCount != 0))) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421",
                             "vkCreateAccelerationStructureNV(): pCreateInfo->compactedSize nonzero (%" PRIu64
                             ") with info.geometryCount (%" PRIu32 ") or info.instanceCount (%" PRIu32 ") nonzero.",
                             pCreateInfo->compactedSize, pCreateInfo->info.geometryCount, pCreateInfo->info.instanceCount);
        }

        skip |= ValidateAccelerationStructureInfoNV(pCreateInfo->info, VkAccelerationStructureNV(0),
                                                    "vkCreateAccelerationStructureNV()", false);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                                const VkAccelerationStructureInfoNV *pInfo,
                                                                                VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                                                VkBool32 update, VkAccelerationStructureNV dst,
                                                                                VkAccelerationStructureNV src, VkBuffer scratch,
                                                                                VkDeviceSize scratchOffset) const {
    bool skip = false;

    if (pInfo != nullptr) {
        skip |= ValidateAccelerationStructureInfoNV(*pInfo, dst, "vkCmdBuildAccelerationStructureNV()", true);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateAccelerationStructureKHR(
    VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkAccelerationStructureKHR *pAccelerationStructure) const {
    bool skip = false;
    const auto *acceleration_structure_features =
        LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acceleration_structure_features ||
        (acceleration_structure_features && acceleration_structure_features->accelerationStructure == VK_FALSE)) {
        skip |= LogError(device, "VUID-vkCreateAccelerationStructureKHR-accelerationStructure-03611",
                         "vkCreateAccelerationStructureKHR(): The accelerationStructure feature must be enabled");
    }
    if (pCreateInfo) {
        if (pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR &&
            (!acceleration_structure_features ||
             (acceleration_structure_features &&
              acceleration_structure_features->accelerationStructureCaptureReplay == VK_FALSE))) {
            skip |=
                LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613",
                         "vkCreateAccelerationStructureKHR(): If createFlags includes "
                         "VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR, "
                         "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureCaptureReplay must be VK_TRUE");
        }
        if (pCreateInfo->deviceAddress &&
            !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR)) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612",
                             "vkCreateAccelerationStructureKHR(): If deviceAddress is not zero, createFlags must include "
                             "VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR");
        }
        if (pCreateInfo->deviceAddress && (!acceleration_structure_features ||
                                           (acceleration_structure_features &&
                                            acceleration_structure_features->accelerationStructureCaptureReplay == VK_FALSE))) {
            skip |= LogError(
                device, "VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488",
                "VkAccelerationStructureCreateInfoKHR(): VkAccelerationStructureCreateInfoKHR::deviceAddress is not zero, but "
                "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureCaptureReplay is not enabled.");
        }
        if (SafeModulo(pCreateInfo->offset, 256) != 0) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-offset-03734",
                             "vkCreateAccelerationStructureKHR(): offset %" PRIu64 " must be a multiple of 256 bytes",
                             pCreateInfo->offset);
        }

        const auto *descriptor_buffer_features =
            LvlFindInChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(device_createinfo_pnext);
        if ((pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
            (!descriptor_buffer_features ||
             (descriptor_buffer_features && descriptor_buffer_features->descriptorBufferCaptureReplay == VK_FALSE))) {
            skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-08108",
                             "vkCreateAccelerationStructureKHR(): the descriptorBufferCaptureReplay device feature is disabled: "
                             "Acceleration structures cannot be created with "
                             "the VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT.");
        }

        const auto *opaque_capture_descriptor_buffer =
            LvlFindInChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        if (opaque_capture_descriptor_buffer &&
            !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |=
                LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109",
                         "vkCreateAccelerationStructureKHR(): VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain, but "
                         "VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT is not set.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                                                                                 VkAccelerationStructureNV accelerationStructure,
                                                                                 size_t dataSize, void *pData) const {
    bool skip = false;
    if (dataSize < 8) {
        skip = LogError(accelerationStructure, "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240",
                        "vkGetAccelerationStructureHandleNV(): dataSize must be greater than or equal to 8.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    bool skip = false;
    if (queryType != VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
        skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryType-06216",
                         "vkCmdWriteAccelerationStructuresPropertiesNV: queryType must be "
                         "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                            uint32_t createInfoCount,
                                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                            const VkAllocationCallbacks *pAllocator,
                                                                            VkPipeline *pPipelines) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
            std::stringstream msg;
            msg << "pCreateInfos[%" << i << "].pStages[%" << stage_index << "]";
            ValidatePipelineShaderStageCreateInfo("vkCreateRayTracingPipelinesNV", msg.str().c_str(),
                                                  &pCreateInfos[i].pStages[stage_index]);
        }
        auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651",
                             "vkCreateRayTracingPipelinesNV(): in pCreateInfo[%" PRIu32
                             "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                             "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoNV::stageCount(=%" PRIu32 ").",
                             i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }

        const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
        const auto *pipeline_cache_contol_features =
            LvlFindInChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if ((!vulkan_13_features || vulkan_13_features->pipelineCreationCacheControl == VK_FALSE) &&
            (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE)) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905",
                                 "vkCreateRayTracingPipelinesNV(): If the pipelineCreationCacheControl feature is not enabled,"
                                 "flags must not include VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT or"
                                 "VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT.");
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02904",
                         "vkCreateRayTracingPipelinesNV(): flags must not include VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV.");
        }
        if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV) &&
            (pCreateInfos[i].flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT)) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02957",
                         "vkCreateRayTracingPipelinesNV(): flags must not include both VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV and"
                         "VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT at the same time.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-07986",
                                     "vkCreateRayTracingPipelinesNV parameter, pCreateInfos->basePipelineHandle, must be "
                                     "VK_NULL_HANDLE if pCreateInfos->flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag "
                                     "and pCreateInfos->basePipelineIndex is not -1.");
                }
                if (pCreateInfos[i].basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |=
                        LogError(device, "VUID-vkCreateRayTracingPipelinesNV-flags-03415",
                                 "vkCreateRayTracingPipelinesNV: If the flags member of any element of pCreateInfos contains the"
                                 "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag, and the basePipelineIndex member of that same element"
                                 "is not -1, basePipelineIndex must be less than the index into pCreateInfos that corresponds to "
                                 "that element.");
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |=
                        LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-07985",
                                 "vkCreateRayTracingPipelinesNV if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                 "basePipelineHandle is VK_NULL_HANDLE, basePipelineIndex must be a valid index into the calling"
                                 "commands pCreateInfos parameter.");
                }
            } else {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-07986",
                                     "vkCreateRayTracingPipelinesNV if flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT and"
                                     "basePipelineHandle is not VK_NULL_HANDLE, basePipelineIndex must be -1.");
                }
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03456",
                             "vkCreateRayTracingPipelinesNV: flags must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03458",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03459",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03460",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03461",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
            skip |= LogError(
                device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03462",
                "vkCreateRayTracingPipelinesNV: flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
            skip |= LogError(
                device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03463",
                "vkCreateRayTracingPipelinesNV: flags must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR .");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03588",
                             "vkCreateRayTracingPipelinesNV: flags must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
            skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesNV-flags-03816",
                             "vkCreateRayTracingPipelinesNV: flags must not contain the VK_PIPELINE_CREATE_DISPATCH_BASE flag.");
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-flags-04948",
                             "vkCreateRayTracingPipelinesNV: flags must not contain the "
                             "VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV flag.");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipeline == VK_FALSE) {
        skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-rayTracingPipeline-03586",
                         "vkCreateRayTracingPipelinesKHR(): The rayTracingPipeline feature must be enabled.");
    }
    for (uint32_t i = 0; i < createInfoCount; i++) {
        for (uint32_t stage_index = 0; stage_index < pCreateInfos[i].stageCount; ++stage_index) {
            std::stringstream stage_ss;
            stage_ss << "pCreateInfos[" << i << "].pStages[" << stage_index << "]";
            const std::string stage_str = stage_ss.str();
            ValidatePipelineShaderStageCreateInfo("vkCreateRayTracingPipelinesKHR", stage_str.c_str(),
                                                  &pCreateInfos[i].pStages[stage_index]);

            const auto stage = pCreateInfos[i].pStages[stage_index].stage;
            constexpr std::array allowed_stages = {VK_SHADER_STAGE_RAYGEN_BIT_KHR,       VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                                   VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,  VK_SHADER_STAGE_MISS_BIT_KHR,
                                                   VK_SHADER_STAGE_INTERSECTION_BIT_KHR, VK_SHADER_STAGE_CALLABLE_BIT_KHR};
            if (std::find(allowed_stages.begin(), allowed_stages.end(), stage) == allowed_stages.end()) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-06899",
                                 "vkCreateRayTracingPipelinesKHR(): %s.stage is %s.", stage_str.c_str(),
                                 string_VkShaderStageFlagBits(stage));
            }
        }

        if (!raytracing_features || (raytracing_features && raytracing_features->rayTraversalPrimitiveCulling == VK_FALSE)) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03596",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                                 string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
            }
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03597",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                                 string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0) &&
            (feedback_struct->pipelineStageCreationFeedbackCount != pCreateInfos[i].stageCount)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineStageCreationFeedbackCount-06652",
                             "vkCreateRayTracingPipelinesKHR(): in pCreateInfo[%" PRIu32
                             "], When chained to VkRayTracingPipelineCreateInfoKHR, "
                             "VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                             "(=%" PRIu32 ") must equal VkRayTracingPipelineCreateInfoKHR::stageCount(=%" PRIu32 ").",
                             i, feedback_struct->pipelineStageCreationFeedbackCount, pCreateInfos[i].stageCount);
        }

        const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
        const auto *pipeline_cache_contol_features =
            LvlFindInChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(device_createinfo_pnext);
        if ((!vulkan_13_features || vulkan_13_features->pipelineCreationCacheControl == VK_FALSE) &&
            (!pipeline_cache_contol_features || pipeline_cache_contol_features->pipelineCreationCacheControl == VK_FALSE)) {
            if (pCreateInfos[i].flags & (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
                                         VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                                 string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904",
                             "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                             string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            if (pCreateInfos[i].pLibraryInterface == NULL) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                                 string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }
        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
            skip |= LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03816",
                             "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                             string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
        }

        for (uint32_t group_index = 0; group_index < pCreateInfos[i].groupCount; ++group_index) {
            if ((pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) ||
                (pCreateInfos[i].pGroups[group_index].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR)) {
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].anyHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32
                                     "].flags is %s, but pCreateInfos[%" PRIu32 "].pGroups[%" PRIu32
                                     "].anyHitShader is VK_SHADER_UNUSED_KHR.",
                                     i, string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str(), i, group_index);
                }
                if ((pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) &&
                    (pCreateInfos[i].pGroups[group_index].closestHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32
                                     "].flags is %s, but pCreateInfos[%" PRIu32 "].pGroups[%" PRIu32
                                     "].closestHitShader is VK_SHADER_UNUSED_KHR.",
                                     i, string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str(), i, group_index);
                }
            }
            if (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_TRUE &&
                pCreateInfos[i].pGroups[group_index].pShaderGroupCaptureReplayHandle) {
                if (!(pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03599",
                        "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                        string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
                }
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineIndex != -1) {
                if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07986",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].basePipelineIndex is %" PRId32
                                     " and pCreateInfos[%" PRIu32 "].basePipelineHandle is not VK_NULL_HANDLE.",
                                     i, pCreateInfos[i].basePipelineIndex, i);
                }
                if (pCreateInfos[i].basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |=
                        LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].basePipelineIndex is %" PRId32 ".", i,
                                 pCreateInfos[i].basePipelineIndex);
                }
            }
            if (pCreateInfos[i].basePipelineHandle == VK_NULL_HANDLE) {
                if (pCreateInfos[i].basePipelineIndex < 0 ||
                    static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07985",
                        "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s but pCreateInfos[%" PRIu32
                        "].basePipelineIndex has invalid index value %" PRId32 ".",
                        i, string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str(), i, pCreateInfos[i].basePipelineIndex);
                }
            }
        }

        if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR &&
            (raytracing_features && raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE)) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03598",
                             "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].flags is %s.", i,
                             string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
        }

        const bool library_enabled = IsExtEnabled(device_extensions.vk_khr_pipeline_library);
        if (!library_enabled) {
            if (pCreateInfos[i].pLibraryInfo) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].pLibraryInfo is not NULL.", i);
            }
            if (pCreateInfos[i].pLibraryInterface) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].pLibraryInterface is not NULL.", i);
            }
        }

        if (pCreateInfos[i].pLibraryInfo) {
            if ((pCreateInfos[i].pLibraryInfo->libraryCount > 0) && (pCreateInfos[i].pLibraryInterface == nullptr)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32
                                 "].pLibraryInfo is not NULL, its libraryCount is %" PRIu32 ", but pCreateInfos[%" PRIu32
                                 "].pLibraryInterface is NULL.",
                                 i, pCreateInfos[i].pLibraryInfo->libraryCount, i);
            }
        }

        if (pCreateInfos[i].pLibraryInfo == nullptr) {
            if (pCreateInfos[i].stageCount == 0) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].pLibraryInfo is NULL.", i);
            }
            if (((pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) && (pCreateInfos[i].groupCount == 0)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].pLibraryInfo is NULL.", i);
            }
        } else if (pCreateInfos[i].pLibraryInfo->libraryCount == 0) {
            if (pCreateInfos[i].stageCount == 0) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].pLibraryInfo->libraryCount is 0.", i);
            }
            if (((pCreateInfos[i].flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) && (pCreateInfos[i].groupCount == 0)) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32 "].pLibraryInfo->libraryCount is 0.", i);
            }
        }

        if (pCreateInfos[i].pLibraryInterface) {
            if (pCreateInfos[i].pLibraryInterface->maxPipelineRayHitAttributeSize >
                phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize) {
                skip |= LogError(device, "VUID-VkRayTracingPipelineInterfaceCreateInfoKHR-maxPipelineRayHitAttributeSize-03605",
                                 "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32
                                 "].pLibraryInterface->maxPipelineRayHitAttributeSize (%" PRIu32
                                 ") > VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayHitAttributeSize (%" PRIu32 ").",
                                 i, pCreateInfos[i].pLibraryInterface->maxPipelineRayHitAttributeSize,
                                 phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize);
            }
        }

        if (deferredOperation != VK_NULL_HANDLE) {
            if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT) {
                skip |= LogError(
                    device, "VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-03587",
                    "vkCreateRayTracingPipelinesKHR(): Parameter defferedOperation is not VK_NULL_HANDLE, but pCreateInfos[%" PRIu32
                    "].flags is %s.",
                    i, string_VkPipelineCacheCreateFlags(pCreateInfos[i].flags).c_str());
            }
        }

        if (pCreateInfos[i].pDynamicState) {
            for (uint32_t j = 0; j < pCreateInfos[i].pDynamicState->dynamicStateCount; ++j) {
                if (pCreateInfos[i].pDynamicState->pDynamicStates[j] != VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates[%" PRIu32 "] is %s.",
                                     i, j, string_VkDynamicState(pCreateInfos[i].pDynamicState->pDynamicStates[j]));
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |= LogError(device, "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412",
                         "vkCopyAccelerationStructureToMemoryKHR: mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR.");
    }
    const auto *acc_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyAccelerationStructureToMemoryKHR-accelerationStructureHostCommands-03584",
            "vkCopyAccelerationStructureToMemoryKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    skip |= ValidateRequiredPointer("vkCopyAccelerationStructureToMemoryKHR", "pInfo->dst.hostAddress", pInfo->dst.hostAddress,
                                    "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03732");
    if (SafeModulo((VkDeviceSize)pInfo->dst.hostAddress, 16) != 0) {
        skip |= LogError(device, "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03751",
                         "vkCopyAccelerationStructureToMemoryKHR(): pInfo->dst.hostAddress must be aligned to 16 bytes.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |=  // to update VUID to VkCmdCopyAccelerationStructureToMemoryInfoKHR after spec update
            LogError(commandBuffer, "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412",
                     "vkCmdCopyAccelerationStructureToMemoryKHR: mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR.");
    }
    if (SafeModulo(pInfo->dst.deviceAddress, 256) != 0) {
        skip |= LogError(device, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03740",
                         "vkCmdCopyAccelerationStructureToMemoryKHR(): pInfo->dst.deviceAddress (0x%" PRIx64
                         ") must be aligned to 256 bytes.",
                         pInfo->dst.deviceAddress);
    }
    return skip;
}

bool StatelessValidation::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                   const char *api_name) const {
    bool skip = false;
    if (!(pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR ||
          pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        skip |= LogError(device, "VUID-VkCopyAccelerationStructureInfoKHR-mode-03410",
                         "(%s): mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR"
                         "or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR.",
                         api_name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyAccelerationStructureKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCopyAccelerationStructureKHR()");
    const auto *acc_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyAccelerationStructureKHR-accelerationStructureHostCommands-03582",
            "vkCopyAccelerationStructureKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCmdCopyAccelerationStructureKHR()");
    return skip;
}

bool StatelessValidation::ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                           const char *api_name, bool is_cmd) const {
    bool skip = false;
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR) {
        skip |= LogError(device, "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-mode-03413",
                         "(%s): mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR.", api_name);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(pInfo, "vkCopyMemoryToAccelerationStructureKHR()", true);
    const auto *acc_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_struct_features || acc_struct_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCopyMemoryToAccelerationStructureKHR-accelerationStructureHostCommands-03583",
            "vkCopyMemoryToAccelerationStructureKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    skip |= ValidateRequiredPointer("vkCopyMemoryToAccelerationStructureKHR", "pInfo->src.hostAddress", pInfo->src.hostAddress,
                                    "VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03729");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(pInfo, "vkCmdCopyMemoryToAccelerationStructureKHR()", false);
    if (SafeModulo(pInfo->src.deviceAddress, 256) != 0) {
        skip |= LogError(device, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03743",
                         "vkCmdCopyMemoryToAccelerationStructureKHR(): pInfo->src.deviceAddress (0x%" PRIx64
                         ") must be aligned to 256 bytes.",
                         pInfo->src.deviceAddress);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    bool skip = false;
    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR)) {
        if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1)) {
            skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-03432",
                             "vkCmdWriteAccelerationStructuresPropertiesKHR(): queryType (%s) is invalid.",
                             string_VkQueryType(queryType));
        } else if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
                     queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR)) {
            skip |= LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742",
                             "vkCmdWriteAccelerationStructuresPropertiesKHR(): queryType (%s) must be is invalid.",
                             string_VkQueryType(queryType));
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void *pData, size_t stride) const {
    bool skip = false;
    const auto *acc_structure_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!acc_structure_features || acc_structure_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructureHostCommands-03585",
            "vkCmdWriteAccelerationStructuresPropertiesKHR(): The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled.");
    }
    if (dataSize < accelerationStructureCount * stride) {
        skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-dataSize-03452",
                         "vkWriteAccelerationStructuresPropertiesKHR(): dataSize (%zu) must be greater than or equal to "
                         "accelerationStructureCount (%" PRIu32 ") *stride(%zu).",
                         dataSize, accelerationStructureCount, stride);
    }

    if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
          queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR)) {
        if (!IsExtEnabled(device_extensions.vk_khr_ray_tracing_maintenance1)) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03432",
                             "vkWriteAccelerationStructuresPropertiesKHR(): queryType (%s) must be "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR.",
                             string_VkQueryType(queryType));
        } else if (!(queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
                     queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR)) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742",
                             "vkWriteAccelerationStructuresPropertiesKHR(): queryType (%s) must be "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR or "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR.",
                             string_VkQueryType(queryType));
        }
    }

    if (SafeModulo(stride, sizeof(VkDeviceSize)) != 0) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448",
                             "vkWriteAccelerationStructuresPropertiesKHR(): If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, "
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize.",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450",
                             "vkWriteAccelerationStructuresPropertiesKHR(): If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, "
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize.",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731",
                             "vkWriteAccelerationStructuresPropertiesKHR(): If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, "
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize.",
                             stride);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR) {
            skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733",
                             "vkWriteAccelerationStructuresPropertiesKHR(): If queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, "
                             "then stride (%zu) must be a multiple of the size of VkDeviceSize.",
                             stride);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03606",
            "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR:VkPhysicalDeviceRayTracingPipelineFeaturesKHR::"
            "rayTracingPipelineShaderGroupHandleCaptureReplay must be enabled to call this function.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                                uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip = false;
    // RayGen
    if (pRaygenShaderBindingTable->size != pRaygenShaderBindingTable->stride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-size-04023",
                         "vkCmdTraceRaysKHR: The size member of pRayGenShaderBindingTable must be equal to its stride member");
    }
    if (SafeModulo(pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03682",
                         "vkCmdTraceRaysKHR: pRaygenShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // Callable
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-03694",
                         "vkCmdTraceRaysKHR: The stride member of pCallableShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04041",
                         "vkCmdTraceRaysKHR: The stride member of pCallableShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03693",
                         "vkCmdTraceRaysKHR: pCallableShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-03690",
                         "vkCmdTraceRaysKHR: The stride member of pHitShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04035",
                         "vkCmdTraceRaysKHR: TThe stride member of pHitShaderBindingTable must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride");
    }
    if (SafeModulo(pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03689",
                         "vkCmdTraceRaysKHR: pHitShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // missShader
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-03686",
                         "vkCmdTraceRaysKHR: The stride member of pMissShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment");
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-stride-04029",
                         "vkCmdTraceRaysKHR: The stride member of pMissShaderBindingTable must be"
                         "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03685",
                         "vkCmdTraceRaysKHR: pMissShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    if (width * depth * height > phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-width-03641",
                         "vkCmdTraceRaysKHR: width {times} height {times} depth must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayDispatchInvocationCount");
    }
    if (width > device_limits.maxComputeWorkGroupCount[0] * device_limits.maxComputeWorkGroupSize[0]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysKHR-width-03638",
                     "vkCmdTraceRaysKHR: width must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] "
                     "{times} VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0]");
    }

    if (height > device_limits.maxComputeWorkGroupCount[1] * device_limits.maxComputeWorkGroupSize[1]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysKHR-height-03639",
                     "vkCmdTraceRaysKHR: height must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] "
                     "{times} VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1]");
    }

    if (depth > device_limits.maxComputeWorkGroupCount[2] * device_limits.maxComputeWorkGroupSize[2]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysKHR-depth-03640",
                     "vkCmdTraceRaysKHR: depth must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] "
                     "{times} VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2]");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysIndirectKHR(
    VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features || raytracing_features->rayTracingPipelineTraceRaysIndirect == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCmdTraceRaysIndirectKHR-rayTracingPipelineTraceRaysIndirect-03637",
            "vkCmdTraceRaysIndirectKHR: the VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineTraceRaysIndirect "
            "feature must be enabled.");
    }
    // RayGen
    if (pRaygenShaderBindingTable->size != pRaygenShaderBindingTable->stride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-size-04023",
                         "vkCmdTraceRaysKHR: The size member of pRayGenShaderBindingTable must be equal to its stride member");
    }
    if (SafeModulo(pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03682",
                         "vkCmdTraceRaysIndirectKHR: pRaygenShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // Callabe
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-03694",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pCallableShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pCallableShaderBindingTable must be less than or equal "
                         "to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03693",
                         "vkCmdTraceRaysIndirectKHR: pCallableShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pHitShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pHitShaderBindingTable must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03689",
                         "vkCmdTraceRaysIndirectKHR: pHitShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }
    // missShader
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-03686",
                         "vkCmdTraceRaysIndirectKHR:The stride member of pMissShaderBindingTable must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment.");
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029",
                         "vkCmdTraceRaysIndirectKHR: The stride member of pMissShaderBindingTable must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride.");
    }
    if (SafeModulo(pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03685",
                         "vkCmdTraceRaysIndirectKHR: pMissShaderBindingTable->deviceAddress must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment.");
    }

    if (SafeModulo(indirectDeviceAddress, 4) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03634",
                         "vkCmdTraceRaysIndirectKHR: indirectDeviceAddress must be a multiple of 4.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                                         VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    const auto *raytracing_features = LvlFindInChain<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>(device_createinfo_pnext);
    if (!raytracing_features) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637",
                         "vkCmdTraceRaysIndirect2KHR(): no VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR structure was found "
                         "in device create info pNext chain.");
    } else if (!raytracing_features->rayTracingPipelineTraceRaysIndirect2) {
        skip |= LogError(
            device, "VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637",
            "vkCmdTraceRaysIndirect2KHR(): VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingPipelineTraceRaysIndirect2 "
            "found in device create info pNext chain is VK_FALSE");
    }

    if (SafeModulo(indirectDeviceAddress, 4) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634",
                         "vkCmdTraceRaysIndirect2KHR: indirectDeviceAddress (0x%" PRIx64 ") must be a multiple of 4.",
                         indirectDeviceAddress);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdTraceRaysNV(
    VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset,
    VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride,
    VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
    uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip = false;
    if (SafeModulo(callableShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462",
                         "vkCmdTraceRaysNV: callableShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(callableShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465",
                         "vkCmdTraceRaysNV: callableShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (callableShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468",
                         "vkCmdTraceRaysNV: callableShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride. ");
    }

    // hitShader
    if (SafeModulo(hitShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460",
                         "vkCmdTraceRaysNV: hitShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(hitShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464",
                         "vkCmdTraceRaysNV: hitShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (hitShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467",
                         "vkCmdTraceRaysNV: hitShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(missShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458",
                         "vkCmdTraceRaysNV: missShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(missShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463",
                         "vkCmdTraceRaysNV: missShaderBindingStride must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (missShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466",
                         "vkCmdTraceRaysNV: missShaderBindingStride must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(raygenShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456",
                         "vkCmdTraceRaysNV: raygenShaderBindingOffset must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (width > device_limits.maxComputeWorkGroupCount[0]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysNV-width-02469",
                     "vkCmdTraceRaysNV: width must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[o].");
    }
    if (height > device_limits.maxComputeWorkGroupCount[1]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysNV-height-02470",
                     "vkCmdTraceRaysNV: height must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1].");
    }
    if (depth > device_limits.maxComputeWorkGroupCount[2]) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysNV-depth-02471",
                     "vkCmdTraceRaysNV: depth must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2].");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
    VkAccelerationStructureCompatibilityKHR *pCompatibility) const {
    bool skip = false;
    const auto *accel_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!(accel_struct_features && accel_struct_features->accelerationStructure)) {
        skip |= LogError(device, "VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-accelerationStructure-08928",
                         "vkGetDeviceAccelerationStructureCompatibilityKHR: The accelerationStructure feature must be enabled");
    }
    return skip;
}

bool StatelessValidation::ValidateAccelerationStructureBuildGeometryInfoKHR(
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, uint32_t infoCount, uint64_t total_primitive_count,
    const char *api_name) const {
    bool skip = false;
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654",
                             "%s(): pInfos[%" PRIu32 "].type must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.", api_name, i);
        }
        if (pInfos[i].flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR &&
            pInfos[i].flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-03796",
                             "%s(): If pInfos[%" PRIu32
                             "].flags has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR bit set,"
                             "then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR bit set.",
                             api_name, i);
        }
        if (pInfos[i].pGeometries && pInfos[i].ppGeometries) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788",
                             "%s(): Only one of pInfos[%" PRIu32 "].pGeometries or pInfos[%" PRIu32
                             "].ppGeometries can be a valid pointer, the other must be NULL",
                             api_name, i, i);
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && pInfos[i].geometryCount != 1) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03790",
                             "%s(): If pInfos[%" PRIu32
                             "].type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, geometryCount must be 1",
                             api_name, i);
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
            pInfos[i].geometryCount > phys_dev_ext_props.acc_structure_props.maxGeometryCount) {
            skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03793",
                             "%s(): If pInfos[%" PRIu32
                             "].type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR then geometryCount must be"
                             " less than or equal to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxGeometryCount",
                             api_name, i);
        }

        const VkAccelerationStructureGeometryKHR *pGeometry = pInfos[i].pGeometries    ? pInfos[i].pGeometries
                                                              : pInfos[i].ppGeometries ? pInfos[i].ppGeometries[0]
                                                                                       : nullptr;
        if (pGeometry) {
            if (total_primitive_count > phys_dev_ext_props.acc_structure_props.maxPrimitiveCount) {
                switch (pGeometry->geometryType) {
                    case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
                        skip |=
                            LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795",
                                     "%s(): pInfos[%" PRIu32 "] total number of triangles in all geometries (%" PRIu64
                                     ") is superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount "
                                     "(%" PRIu64 ")",
                                     api_name, i, total_primitive_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
                        break;
                    case VK_GEOMETRY_TYPE_AABBS_KHR:
                        skip |=
                            LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794",
                                     "%s(): pInfos[%" PRIu32 "] total number of AABBs in all geometries (%" PRIu64
                                     ") is superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount "
                                     "(%" PRIu64 ")",
                                     api_name, i, total_primitive_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
                        break;
                    default:
                        break;
                }
            }
        }

        if (pInfos[i].pGeometries) {
            for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
                skip |= ValidateRangedEnum(
                    api_name, ParameterName("pInfos[%i].pGeometries[%i].geometryType", ParameterName::IndexVector{i, j}),
                    "VkGeometryTypeKHR", pInfos[i].pGeometries[j].geometryType,
                    "VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    constexpr std::array allowed_structs = {
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT};

                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                        &(pInfos[i].pGeometries[j].geometry.triangles),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].pGeometries[j].geometry.triangles.pNext, allowed_structs.size(), allowed_structs.data(),
                        GeneratedVulkanHeaderVersion, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-pNext-pNext",
                        kVUIDUndefined);
                    skip |= ValidateRangedEnum(api_name,
                                               ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles.vertexFormat",
                                                             ParameterName::IndexVector{i, j}),
                                               "VkFormat", pInfos[i].pGeometries[j].geometry.triangles.vertexFormat,
                                               "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-parameter");
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.triangles",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                                               &pInfos[i].pGeometries[j].geometry.triangles,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-triangles-parameter", kVUIDUndefined);
                    skip |= ValidateRangedEnum(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.triangles.indexType", ParameterName::IndexVector{i, j}),
                        "VkIndexType", pInfos[i].pGeometries[j].geometry.triangles.indexType,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-parameter");

                    if (pInfos[i].pGeometries[j].geometry.triangles.vertexStride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819",
                                         "%s(): pInfos[%" PRIu32 "].pGeometries[%" PRIu32
                                         "].geometry.triangles.vertexStride must be less than or equal to 2^32-1",
                                         api_name, i, j);
                    }
                    if (pInfos[i].pGeometries[j].geometry.triangles.indexType != VK_INDEX_TYPE_UINT16 &&
                        pInfos[i].pGeometries[j].geometry.triangles.indexType != VK_INDEX_TYPE_UINT32 &&
                        pInfos[i].pGeometries[j].geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798",
                                         "%s(): pInfos[%" PRIu32 "].pGeometries[%" PRIu32
                                         "].geometry.triangles.indexType must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or "
                                         "VK_INDEX_TYPE_NONE_KHR",
                                         api_name, i, j);
                    }
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.instances",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                                               &pInfos[i].pGeometries[j].geometry.instances,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-instances-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].pGeometries[%i].geometry.instances", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                        &(pInfos[i].pGeometries[j].geometry.instances),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.instances.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].pGeometries[j].geometry.instances.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-pNext-pNext", kVUIDUndefined);

                    skip |= ValidateBool32(api_name,
                                           ParameterName("pInfos[%i].pGeometries[%i].geometry.instances.arrayOfPointers",
                                                         ParameterName::IndexVector{i, j}),
                                           pInfos[i].pGeometries[j].geometry.instances.arrayOfPointers);
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.aabbs",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                                               &pInfos[i].pGeometries[j].geometry.aabbs,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-aabbs-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].pGeometries[%i].geometry.aabbs", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                        &(pInfos[i].pGeometries[j].geometry.aabbs),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].pGeometries[%i].geometry.aabbs.pNext", ParameterName::IndexVector{i, j}), NULL,
                        pInfos[i].pGeometries[j].geometry.aabbs.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-pNext-pNext", kVUIDUndefined);
                    if (pInfos[i].pGeometries[j].geometry.aabbs.stride % 8) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545",
                                         "%s(): pInfos[%" PRIu32 "].pGeometries[%" PRIu32 "].geometry.aabbs.stride (%" PRIu64
                                         ") is not a multiple of 8.",
                                         api_name, i, j, pInfos[i].pGeometries[j].geometry.aabbs.stride);
                    }
                    if (pInfos[i].pGeometries[j].geometry.aabbs.stride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820",
                                         "%s(): pInfos[%" PRIu32 "].pGeometries[%" PRIu32 "].geometry.aabbs.stride (%" PRIu64
                                         ") must be less than or equal to 2^32-1.",
                                         api_name, i, j, pInfos[i].pGeometries[j].geometry.aabbs.stride);
                    }
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    pInfos[i].pGeometries[j].geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789",
                                     "%s(): pInfos[%" PRIu32
                                     "].type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but pGeometries[%" PRIu32
                                     "].geometryType is not VK_GEOMETRY_TYPE_INSTANCES_KHR.",
                                     api_name, i, j);
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                    if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791",
                                         "%s(): If pInfos[%" PRIu32
                                         "].type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR then pGeometries[%" PRIu32
                                         "].geometryType must not be VK_GEOMETRY_TYPE_INSTANCES_KHR.",
                                         api_name, i, j);
                    }
                    if (pInfos[i].pGeometries[j].geometryType != pInfos[i].pGeometries[0].geometryType) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792",
                                         "%s(): pInfos[%" PRIu32
                                         "].type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR but pGeometries[%" PRIu32
                                         "].geometryType (%s) is different than pGeometries[0].geometryType (%s)",
                                         api_name, i, j, string_VkGeometryTypeKHR(pInfos[i].pGeometries[j].geometryType),
                                         string_VkGeometryTypeKHR(pInfos[i].pGeometries[0].geometryType));
                    }
                }
            }
        }
        if (pInfos[i].ppGeometries != NULL) {
            for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
                skip |= ValidateRangedEnum(
                    api_name, ParameterName("pInfos[%i].ppGeometries[%i]->geometryType", ParameterName::IndexVector{i, j}),
                    "VkGeometryTypeKHR", pInfos[i].ppGeometries[j]->geometryType,
                    "VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.triangles",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                                               &pInfos[i].ppGeometries[j]->geometry.triangles,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-triangles-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR",
                        &(pInfos[i].ppGeometries[j]->geometry.triangles),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].ppGeometries[j]->geometry.triangles.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-pNext-pNext", kVUIDUndefined);
                    skip |= ValidateRangedEnum(api_name,
                                               ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles.vertexFormat",
                                                             ParameterName::IndexVector{i, j}),
                                               "VkFormat", pInfos[i].ppGeometries[j]->geometry.triangles.vertexFormat,
                                               "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-parameter");
                    skip |= ValidateRangedEnum(api_name,
                                               ParameterName("pInfos[%i].ppGeometries[%i]->geometry.triangles.indexType",
                                                             ParameterName::IndexVector{i, j}),
                                               "VkIndexType", pInfos[i].ppGeometries[j]->geometry.triangles.indexType,
                                               "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-parameter");
                    if (pInfos[i].ppGeometries[j]->geometry.triangles.vertexStride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819",
                                         "%s():vertexStride must be less than or equal to 2^32-1", api_name);
                    }
                    if (pInfos[i].ppGeometries[j]->geometry.triangles.indexType != VK_INDEX_TYPE_UINT16 &&
                        pInfos[i].ppGeometries[j]->geometry.triangles.indexType != VK_INDEX_TYPE_UINT32 &&
                        pInfos[i].ppGeometries[j]->geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798",
                                         "%s(): pInfos[%" PRIu32 "].ppGeometries[%" PRIu32
                                         "]->geometry.triangles.indexType must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or "
                                         "VK_INDEX_TYPE_NONE_KHR",
                                         api_name, i, j);
                    }
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.instances",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                                               &pInfos[i].ppGeometries[j]->geometry.instances,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-instances-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.instances", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR",
                        &(pInfos[i].ppGeometries[j]->geometry.instances),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.instances.pNext", ParameterName::IndexVector{i, j}),
                        NULL, pInfos[i].ppGeometries[j]->geometry.instances.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryInstancesDataKHR-pNext-pNext", kVUIDUndefined);
                    skip |= ValidateBool32(api_name,
                                           ParameterName("pInfos[%i].ppGeometries[%i]->geometry.instances.arrayOfPointers",
                                                         ParameterName::IndexVector{i, j}),
                                           pInfos[i].ppGeometries[j]->geometry.instances.arrayOfPointers);
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    skip |= ValidateStructType(api_name, "pInfos[i].pGeometries[j].geometry.aabbs",
                                               "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                                               &pInfos[i].ppGeometries[j]->geometry.aabbs,
                                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, true,
                                               "VUID-VkAccelerationStructureGeometryKHR-aabbs-parameter", kVUIDUndefined);
                    skip |= ValidateStructType(
                        api_name, ParameterName("pInfos[%i].ppGeometries[%i]->geometry.aabbs", ParameterName::IndexVector{i, j}),
                        "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR",
                        &(pInfos[i].ppGeometries[j]->geometry.aabbs),
                        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR, false, kVUIDUndefined,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-sType-sType");
                    skip |= ValidateStructPnext(
                        api_name,
                        ParameterName("pInfos[%i].ppGeometries[%i]->geometry.aabbs.pNext", ParameterName::IndexVector{i, j}), NULL,
                        pInfos[i].ppGeometries[j]->geometry.aabbs.pNext, 0, NULL, GeneratedVulkanHeaderVersion,
                        "VUID-VkAccelerationStructureGeometryAabbsDataKHR-pNext-pNext", kVUIDUndefined);
                    if (pInfos[i].ppGeometries[j]->geometry.aabbs.stride > vvl::kU32Max) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820",
                                         "%s(): pInfos[%" PRIu32 "].ppGeometries[%" PRIu32 "]->geometry.aabbs.stride (%" PRIu64
                                         ") must be less than or equal to 2^32-1",
                                         api_name, i, j, pInfos[i].ppGeometries[j]->geometry.aabbs.stride);
                    }
                    if (pInfos[i].ppGeometries[j]->geometry.aabbs.stride % 8) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545",
                                         "%s(): pInfos[%" PRIu32 "].ppGeometries[%" PRIu32 "]->geometry.aabbs.stride (%" PRIu64
                                         ") is not a multiple of 8.",
                                         api_name, i, j, pInfos[i].ppGeometries[j]->geometry.aabbs.stride);
                    }
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    pInfos[i].ppGeometries[j]->geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789",
                                     "%s(): If pInfos[%" PRIu32
                                     "].type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR then ppGeometries[%" PRIu32
                                     "]->geometryType must not be VK_GEOMETRY_TYPE_INSTANCES_KHR.",
                                     api_name, i, j);
                }
                if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                    if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791",
                                         "%s(): If pInfos[%" PRIu32
                                         "].type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR then ppGeometries[%" PRIu32
                                         "]->geometryType must not be VK_GEOMETRY_TYPE_INSTANCES_KHR.",
                                         api_name, i, j);
                    }
                    if (pInfos[i].ppGeometries[j]->geometryType != pInfos[i].ppGeometries[0]->geometryType) {
                        skip |= LogError(device, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792",
                                         "%s(): pInfos[%" PRIu32
                                         "].type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR but ppGeometries[%" PRIu32
                                         "]->geometryType (%s) is different than ppGeometries[0]->geometryType (%s)",
                                         api_name, i, j, string_VkGeometryTypeKHR(pInfos[i].ppGeometries[j]->geometryType),
                                         string_VkGeometryTypeKHR(pInfos[i].ppGeometries[0]->geometryType));
                    }
                }
            }
        }
    }
    return skip;
}
bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const {
    bool skip = false;
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos, infoCount, 0, "vkCmdBuildAccelerationStructuresKHR");
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (SafeModulo(pInfos[i].scratchData.deviceAddress,
                       phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment) != 0) {
            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03710",
                             "vkCmdBuildAccelerationStructuresKHR:For each element of pInfos, its "
                             "scratchData.deviceAddress member must be a multiple of "
                             "VkPhysicalDeviceAccelerationStructurePropertiesKHR::minAccelerationStructureScratchOffsetAlignment.");
        }
        for (uint32_t k = 0; k < infoCount; ++k) {
            if (i == k) continue;
            bool found = false;
            if (pInfos[i].dstAccelerationStructure == pInfos[k].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03698",
                             "vkCmdBuildAccelerationStructuresKHR:The dstAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, k);
                found = true;
            }
            if (pInfos[i].srcAccelerationStructure == pInfos[k].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03403",
                             "vkCmdBuildAccelerationStructuresKHR:The srcAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, k);
                found = true;
            }
            if (found) break;
        }
        for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
            if (pInfos[i].pGeometries) {
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].pGeometries[j].geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03716",
                                             "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                             "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                             "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |=
                                LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03715",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                         "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                         "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                } else if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                } else if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.triangles.transformData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            } else if (pInfos[i].ppGeometries) {
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].ppGeometries[j]->geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03716",
                                             "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                             "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                             "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |=
                                LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03715",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                                         "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                         "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                } else if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                } else if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.triangles.transformData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides,
    const uint32_t *const *ppMaxPrimitiveCounts) const {
    bool skip = false;
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos, infoCount, 0, "vkCmdBuildAccelerationStructuresIndirectKHR");
    const auto *ray_tracing_acceleration_structure_features =
        LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!ray_tracing_acceleration_structure_features ||
        ray_tracing_acceleration_structure_features->accelerationStructureIndirectBuild == VK_FALSE) {
        skip |= LogError(
            device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-accelerationStructureIndirectBuild-03650",
            "vkCmdBuildAccelerationStructuresIndirectKHR: The "
            "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureIndirectBuild feature must be enabled.");
    }
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (SafeModulo(pInfos[i].scratchData.deviceAddress,
                       phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment) != 0) {
            skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03710",
                             "vkCmdBuildAccelerationStructuresIndirectKHR:For each element of pInfos, its "
                             "scratchData.deviceAddress member must be a multiple of "
                             "VkPhysicalDeviceAccelerationStructurePropertiesKHR::minAccelerationStructureScratchOffsetAlignment.");
        }
        for (uint32_t k = 0; k < infoCount; ++k) {
            if (i == k) continue;
            if (pInfos[i].srcAccelerationStructure == pInfos[k].dstAccelerationStructure) {
                skip |= LogError(
                    device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03403",
                    "vkCmdBuildAccelerationStructuresIndirectKHR:The srcAccelerationStructure member of any element (%" PRIu32
                    ") "
                    "of pInfos must not be the same acceleration structure as the dstAccelerationStructure member of "
                    "any other element [%" PRIu32 ") of pInfos.",
                    i, k);
                break;
            }
        }
        for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
            if (pInfos[i].pGeometries) {
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].pGeometries[j].geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                }
                if (pInfos[i].pGeometries[j].geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].pGeometries[j].geometry.triangles.indexData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            } else if (pInfos[i].ppGeometries) {
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pInfos[i].ppGeometries[j]->geometry.instances.arrayOfPointers == VK_TRUE) {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is "
                                "VK_TRUE, geometry.data->deviceAddress must be aligned to 8 bytes.");
                        }
                    } else {
                        if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 16) != 0) {
                            skip |= LogError(
                                device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715",
                                "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                                "geometryType of VK_GEOMETRY_TYPE_INSTANCES_KHR, if geometry.arrayOfPointers is VK_FALSE, "
                                "geometry.data->deviceAddress must be aligned to 16 bytes.");
                        }
                    }
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.instances.data.deviceAddress, 8) != 0) {
                        skip |= LogError(
                            device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714",
                            "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries with a "
                            "geometryType of VK_GEOMETRY_TYPE_AABBS_KHR, geometry.data->deviceAddress must be aligned to 8 bytes.");
                    }
                }
                if (pInfos[i].ppGeometries[j]->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                    if (SafeModulo(pInfos[i].ppGeometries[j]->geometry.triangles.indexData.deviceAddress, 16) != 0) {
                        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810",
                                         "vkCmdBuildAccelerationStructuresIndirectKHR:For any element of pInfos[i].pGeometries "
                                         "with a geometryType of VK_GEOMETRY_TYPE_TRIANGLES_KHR, "
                                         "geometry.transformData->deviceAddress must be aligned to 16 bytes.");
                    }
                }
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const {
    bool skip = false;
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pInfos, infoCount, 0, "vkBuildAccelerationStructuresKHR");
    const auto *ray_tracing_acceleration_structure_features =
        LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!ray_tracing_acceleration_structure_features ||
        ray_tracing_acceleration_structure_features->accelerationStructureHostCommands == VK_FALSE) {
        skip |=
            LogError(device, "VUID-vkBuildAccelerationStructuresKHR-accelerationStructureHostCommands-03581",
                     "vkBuildAccelerationStructuresKHR: The "
                     "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands feature must be enabled");
    }
    for (uint32_t i = 0; i < infoCount; ++i) {
        for (uint32_t j = 0; j < infoCount; ++j) {
            if (i == j) continue;
            bool found = false;
            if (pInfos[i].dstAccelerationStructure == pInfos[j].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03698",
                             "vkBuildAccelerationStructuresKHR(): The dstAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, j);
                found = true;
            }
            if (pInfos[i].srcAccelerationStructure == pInfos[j].dstAccelerationStructure) {
                skip |=
                    LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03403",
                             "vkBuildAccelerationStructuresKHR(): The srcAccelerationStructure member of any element (%" PRIu32
                             ") of pInfos must "
                             "not be "
                             "the same acceleration structure as the dstAccelerationStructure member of any other element (%" PRIu32
                             ") of pInfos.",
                             i, j);
                found = true;
            }
            if (found) break;
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
    const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo) const {
    bool skip = false;
    uint64_t total_primitive_count = 0;
    if (pBuildInfo && pMaxPrimitiveCounts) {
        for (uint32_t i = 0; i < pBuildInfo->geometryCount; ++i) {
            total_primitive_count += pMaxPrimitiveCounts[i];
        }
    }
    skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(pBuildInfo, 1, total_primitive_count,
                                                              "vkGetAccelerationStructureBuildSizesKHR");
    const auto *accel_struct_features = LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(device_createinfo_pnext);
    if (!(accel_struct_features && accel_struct_features->accelerationStructure)) {
        skip |= LogError(device, "VUID-vkGetAccelerationStructureBuildSizesKHR-accelerationStructure-08933",
                         "vkGetAccelerationStructureBuildSizesKHR: The accelerationStructure feature must be enabled");
    }
    if (pBuildInfo) {
        if (pBuildInfo->geometryCount != 0 && !pMaxPrimitiveCounts) {
            skip |= LogError(device, "VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03619",
                             "vkGetAccelerationStructureBuildSizesKHR: If pBuildInfo->geometryCount is not 0, pMaxPrimitiveCounts "
                             "must be a valid pointer to an array of pBuildInfo->geometryCount uint32_t values");
        }
    }
    return skip;
}
