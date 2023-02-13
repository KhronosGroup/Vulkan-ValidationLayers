/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include <algorithm>
#include <assert.h>
#include <sstream>
#include <string>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"

bool CoreChecks::ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, const DEVICE_MEMORY_STATE *mem_info,
                                                                VkDeviceSize mem_offset, const char *api_name) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV), mem_info, mem_offset,
                                     api_name);
}

bool CoreChecks::PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkAccelerationStructureNV *pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo != nullptr && pCreateInfo->info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pCreateInfo->info.geometryCount; i++) {
            skip |= ValidateGeometryNV(pCreateInfo->info.pGeometries[i], "vkCreateAccelerationStructureNV():");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkAccelerationStructureKHR *pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo) {
        auto buffer_state = Get<BUFFER_STATE>(pCreateInfo->buffer);
        if (buffer_state) {
            if (!(buffer_state->createInfo.usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
                skip |=
                    LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614",
                             "VkAccelerationStructureCreateInfoKHR(): buffer must have been created with a usage value containing "
                             "VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR.");
            }
            if (buffer_state->createInfo.flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) {
                skip |= LogError(device, "VUID-VkAccelerationStructureCreateInfoKHR-buffer-03615",
                                 "VkAccelerationStructureCreateInfoKHR(): buffer must not have been created with "
                                 "VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT.");
            }
            if (pCreateInfo->offset + pCreateInfo->size > buffer_state->createInfo.size) {
                skip |= LogError(
                    device, "VUID-VkAccelerationStructureCreateInfoKHR-offset-03616",
                    "VkAccelerationStructureCreateInfoKHR(): The sum of offset and size must be less than the size of buffer.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateBindAccelerationStructureMemory(VkDevice device,
                                                         const VkBindAccelerationStructureMemoryInfoNV &info) const {
    bool skip = false;

    auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(info.accelerationStructure);
    if (!as_state) {
        return skip;
    }
    if (as_state->HasFullRangeBound()) {
        skip |=
            LogError(info.accelerationStructure, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620",
                     "vkBindAccelerationStructureMemoryNV(): accelerationStructure must not already be backed by a memory object.");
    }

    // Validate bound memory range information
    auto mem_info = Get<DEVICE_MEMORY_STATE>(info.memory);
    if (mem_info) {
        skip |= ValidateInsertAccelerationStructureMemoryRange(info.accelerationStructure, mem_info.get(), info.memoryOffset,
                                                               "vkBindAccelerationStructureMemoryNV()");
        skip |= ValidateMemoryTypes(mem_info.get(), as_state->memory_requirements.memoryTypeBits,
                                    "vkBindAccelerationStructureMemoryNV()",
                                    "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
    }

    // Validate memory requirements alignment
    if (SafeModulo(info.memoryOffset, as_state->memory_requirements.alignment) != 0) {
        skip |= LogError(info.accelerationStructure, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623",
                         "vkBindAccelerationStructureMemoryNV(): memoryOffset  0x%" PRIxLEAST64
                         " must be an integer multiple of the alignment 0x%" PRIxLEAST64
                         " member of the VkMemoryRequirements structure returned from "
                         "a call to vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure and type of "
                         "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV",
                         info.memoryOffset, as_state->memory_requirements.alignment);
    }

    if (mem_info) {
        // Validate memory requirements size
        if (as_state->memory_requirements.size > (mem_info->alloc_info.allocationSize - info.memoryOffset)) {
            skip |= LogError(info.accelerationStructure, "VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624",
                             "vkBindAccelerationStructureMemoryNV(): The size 0x%" PRIxLEAST64
                             " member of the VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure and type of "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV must be less than or equal to the size "
                             "of memory minus memoryOffset 0x%" PRIxLEAST64 ".",
                             as_state->memory_requirements.size, mem_info->alloc_info.allocationSize - info.memoryOffset);
        }
    }

    return skip;
}
bool CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV *pBindInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        skip |= ValidateBindAccelerationStructureMemory(device, pBindInfos[i]);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                 size_t dataSize, void *pData) const {
    bool skip = false;

    auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(accelerationStructure);
    if (as_state != nullptr) {
        skip = ValidateMemoryIsBoundToAccelerationStructure(device, *as_state, "vkGetAccelerationStructureHandleNV",
                                                            "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    skip |= ValidateCmd(*cb_state, CMD_BUILDACCELERATIONSTRUCTURESKHR);
    if (pInfos != nullptr) {
        for (uint32_t info_index = 0; info_index < infoCount; ++info_index) {
            auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[info_index].srcAccelerationStructure);
            auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[info_index].dstAccelerationStructure);

            if (dst_as_state != nullptr) {
                skip |=
                    ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_as_state->buffer_state, "vkCmdBuildAccelerationStructuresKHR",
                                                  "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
            }

            if (pInfos[info_index].mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                if (pInfos[info_index].srcAccelerationStructure == VK_NULL_HANDLE) {
                    const LogObjectList objlist(device, commandBuffer);
                    skip |= LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630",
                                     "vkCmdBuildAccelerationStructuresKHR(): pInfos[%" PRIu32
                                     "].mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR but pInfos[%" PRIu32
                                     "].srcAccelerationStructure=VK_NULL_HANDLE.",
                                     info_index, info_index);
                } else if (src_as_state == nullptr || !src_as_state->built ||
                           !(src_as_state->build_info_khr.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR)) {
                    const LogObjectList objlist(device, commandBuffer);
                    skip |= LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03667",
                                     "vkCmdBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is "
                                     "VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its srcAccelerationStructure member must "
                                     "have been built before with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR set in "
                                     "VkAccelerationStructureBuildGeometryInfoKHR::flags.");
                }
                if (src_as_state != nullptr) {
                    if (!src_as_state->buffer_state) {
                        const LogObjectList objlist(device, commandBuffer, src_as_state->Handle());
                        skip |= LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03708",
                                         "vkCmdBuildAccelerationStructuresKHR(): pInfos[%" PRIu32
                                         "].mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR but the buffer associated with "
                                         "pInfos[%" PRIu32 "].srcAccelerationStructure is not valid.",
                                         info_index, info_index);
                    } else {
                        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_as_state->buffer_state,
                                                              "vkCmdBuildAccelerationStructuresKHR",
                                                              "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03708");
                    }
                    if (pInfos[info_index].geometryCount != src_as_state->build_info_khr.geometryCount) {
                        const LogObjectList objlist(device, commandBuffer);
                        skip |= LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03758",
                                         "vkCmdBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is "
                                         "VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,"
                                         " its geometryCount member must have the same value which was specified when "
                                         "srcAccelerationStructure was last built.");
                    }
                    if (pInfos[info_index].flags != src_as_state->build_info_khr.flags) {
                        const LogObjectList objlist(device, commandBuffer);
                        skip |= LogError(
                            objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03759",
                            "vkCmdBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is"
                            " VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its flags member must have the same value which"
                            " was specified when srcAccelerationStructure was last built.");
                    }
                    if (pInfos[info_index].type != src_as_state->build_info_khr.type) {
                        const LogObjectList objlist(device, commandBuffer);
                        skip |= LogError(
                            objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03760",
                            "vkCmdBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is"
                            " VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its type member must have the same value which"
                            " was specified when srcAccelerationStructure was last built.");
                    }
                }
            }
            if (pInfos[info_index].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                if (!dst_as_state ||
                    (dst_as_state && dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
                     dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR)) {
                    const LogObjectList objlist(device, commandBuffer);
                    skip |=
                        LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03700",
                                 "vkCmdBuildAccelerationStructuresKHR(): For each element of pInfos, if its type member is "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, its dstAccelerationStructure member must have "
                                 "been created with a value of VkAccelerationStructureCreateInfoKHR::type equal to either "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
                }
            }
            if (pInfos[info_index].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
                if (!dst_as_state ||
                    (dst_as_state && dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                     dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR)) {
                    const LogObjectList objlist(device, commandBuffer);
                    skip |=
                        LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03699",
                                 "vkCmdBuildAccelerationStructuresKHR(): For each element of pInfos, if its type member is "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, its dstAccelerationStructure member must have been "
                                 "created with a value of VkAccelerationStructureCreateInfoKHR::type equal to either "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
                }
            }

            skip |= ValidateAccelerationBuffers(info_index, pInfos[info_index], "vkCmdBuildAccelerationStructuresKHR");
        }
    }
    return skip;
}

bool CoreChecks::ValidateAccelerationBuffers(uint32_t info_index, const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                             const char *func_name) const {
    bool skip = false;
    const auto geometry_count = info.geometryCount;
    const auto *p_geometries = info.pGeometries;
    const auto *const *const pp_geometries = info.ppGeometries;

    auto buffer_check = [this, info_index, func_name](uint32_t gi, const VkDeviceOrHostAddressConstKHR address,
                                                      const char *field) -> bool {
        const auto buffer_states = GetBuffersByAddress(address.deviceAddress);
        const bool no_valid_buffer_found =
            !buffer_states.empty() &&
            std::none_of(
                buffer_states.begin(), buffer_states.end(), [](const ValidationStateTracker::BUFFER_STATE_PTR &buffer_state) {
                    return buffer_state->createInfo.usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
                });
        if (no_valid_buffer_found) {
            LogObjectList objlist(device);
            for (const auto &buffer_state : buffer_states) {
                objlist.add(buffer_state->Handle());
            }
            return LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673",
                            "%s(): No buffer associated with pInfos[%" PRIu32 "].pGeometries[%" PRIu32
                            "].%s was created with VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR.",
                            func_name, info_index, gi, field);
        }

        return false;
    };

    // Parameter validation has already checked VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788
    // !(pGeometries && ppGeometries)
    std::function<const VkAccelerationStructureGeometryKHR &(uint32_t)> geom_accessor;
    if (p_geometries) {
        geom_accessor = [p_geometries](uint32_t i) -> const VkAccelerationStructureGeometryKHR & { return p_geometries[i]; };
    } else if (pp_geometries) {
        geom_accessor = [pp_geometries](uint32_t i) -> const VkAccelerationStructureGeometryKHR & {
            // pp_geometries[i] is assumed to be a valid pointer
            return *pp_geometries[i];
        };
    }

    if (geom_accessor) {
        for (uint32_t geom_index = 0; geom_index < geometry_count; ++geom_index) {
            const auto &geom_data = geom_accessor(geom_index);
            switch (geom_data.geometryType) {
                case VK_GEOMETRY_TYPE_TRIANGLES_KHR:  // == VK_GEOMETRY_TYPE_TRIANGLES_NV
                    skip |= buffer_check(geom_index, geom_data.geometry.triangles.vertexData, "geometry.triangles.vertexData");
                    skip |= buffer_check(geom_index, geom_data.geometry.triangles.indexData, "geometry.triangles.indexData");
                    skip |=
                        buffer_check(geom_index, geom_data.geometry.triangles.transformData, "geometry.triangles.transformData");
                    break;
                case VK_GEOMETRY_TYPE_INSTANCES_KHR:
                    skip |= buffer_check(geom_index, geom_data.geometry.instances.data, "geometry.instances.data");
                    break;
                case VK_GEOMETRY_TYPE_AABBS_KHR:  // == VK_GEOMETRY_TYPE_AABBS_NV
                    skip |= buffer_check(geom_index, geom_data.geometry.aabbs.data, "geometry.aabbs.data");
                    break;
                default:
                    // no-op
                    break;
            }
        }
    }

    const auto buffer_states = GetBuffersByAddress(info.scratchData.deviceAddress);
    if (buffer_states.empty()) {
        skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802",
                         "vkCmdBuildAccelerationStructuresKHR(): The buffer associated with pInfos[%" PRIu32
                         "].scratchData.deviceAddress %" PRIx64 " is not a valid device address.",
                         info_index, info.scratchData.deviceAddress);
    } else {
        const bool no_valid_buffer_found = std::none_of(
            buffer_states.begin(), buffer_states.end(), [](const ValidationStateTracker::BUFFER_STATE_PTR &buffer_state) {
                return buffer_state->createInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            });
        if (no_valid_buffer_found) {
            LogObjectList objlist(device);
            for (const auto &buffer_state : buffer_states) {
                objlist.add(buffer_state->Handle());
            }
            skip |= LogError(objlist, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674",
                             "vkBuildAccelerationStructuresKHR(): No buffer associated with pInfos[%" PRIu32
                             "].scratchData.deviceAddress was created with VK_BUFFER_USAGE_STORAGE_BUFFER_BIT bit.",
                             info_index);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < infoCount; ++i) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[i].srcAccelerationStructure);
        auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[i].dstAccelerationStructure);
        if (dst_as_state) {
            skip |= ValidateHostVisibleMemoryIsBoundToBuffer(*dst_as_state->buffer_state, "vkBuildAccelerationStructuresKHR",
                                                             "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
        }
        if (pInfos[i].mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
            if (src_as_state == nullptr || !src_as_state->built ||
                !(src_as_state->build_info_khr.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR)) {
                skip |= LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03667",
                                 "vkBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is "
                                 "VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its srcAccelerationStructure member must have "
                                 "been built before with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR set in "
                                 "VkAccelerationStructureBuildGeometryInfoKHR::flags.");
            }
            if (src_as_state) {
                skip |= ValidateHostVisibleMemoryIsBoundToBuffer(*src_as_state->buffer_state, "vkBuildAccelerationStructuresKHR",
                                                                 "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
                if (pInfos[i].geometryCount != src_as_state->build_info_khr.geometryCount) {
                    skip |= LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03758",
                                     "vkBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is "
                                     "VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,"
                                     " its geometryCount member must have the same value which was specified when "
                                     "srcAccelerationStructure was last built.");
                }
                if (pInfos[i].flags != src_as_state->build_info_khr.flags) {
                    skip |=
                        LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03759",
                                 "vkBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is"
                                 " VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its flags member must have the same value which"
                                 " was specified when srcAccelerationStructure was last built.");
                }
                if (pInfos[i].type != src_as_state->build_info_khr.type) {
                    skip |=
                        LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03760",
                                 "vkBuildAccelerationStructuresKHR(): For each element of pInfos, if its mode member is"
                                 " VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its type member must have the same value which"
                                 " was specified when srcAccelerationStructure was last built.");
                }
            }
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
            if (!dst_as_state ||
                (dst_as_state && dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
                 dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR)) {
                skip |= LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03700",
                                 "vkBuildAccelerationStructuresKHR(): For each element of pInfos, if its type member is "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, its dstAccelerationStructure member must have "
                                 "been created with a value of VkAccelerationStructureCreateInfoKHR::type equal to either "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
            }
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
            if (!dst_as_state ||
                (dst_as_state && dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                 dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR)) {
                skip |= LogError(device, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03699",
                                 "vkBuildAccelerationStructuresKHR(): For each element of pInfos, if its type member is "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, its dstAccelerationStructure member must have been "
                                 "created with a value of VkAccelerationStructureCreateInfoKHR::type equal to either "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
            }
        }
    }
    return skip;
}
bool CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, CMD_BUILDACCELERATIONSTRUCTURENV);

    if (pInfo != nullptr && pInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
            skip |= ValidateGeometryNV(pInfo->pGeometries[i], "vkCmdBuildAccelerationStructureNV():");
        }
    }

    if (pInfo != nullptr && pInfo->geometryCount > phys_dev_ext_props.ray_tracing_props_nv.maxGeometryCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241",
                         "vkCmdBuildAccelerationStructureNV(): geometryCount [%d] must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.",
                         pInfo->geometryCount);
    }

    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE>(dst);
    auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE>(src);
    auto scratch_buffer_state = Get<BUFFER_STATE>(scratch);

    if (dst_as_state != nullptr && pInfo != nullptr) {
        if (dst_as_state->create_infoNV.info.type != pInfo->type) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                             "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::type"
                             "[%s] must be identical to build info VkAccelerationStructureInfoNV::type [%s].",
                             string_VkAccelerationStructureTypeNV(dst_as_state->create_infoNV.info.type),
                             string_VkAccelerationStructureTypeNV(pInfo->type));
        }
        if (dst_as_state->create_infoNV.info.flags != pInfo->flags) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                             "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::flags"
                             "[0x%X] must be identical to build info VkAccelerationStructureInfoNV::flags [0x%X].",
                             dst_as_state->create_infoNV.info.flags, pInfo->flags);
        }
        if (dst_as_state->create_infoNV.info.instanceCount < pInfo->instanceCount) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                             "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::instanceCount "
                             "[%d] must be greater than or equal to build info VkAccelerationStructureInfoNV::instanceCount [%d].",
                             dst_as_state->create_infoNV.info.instanceCount, pInfo->instanceCount);
        }
        if (dst_as_state->create_infoNV.info.geometryCount < pInfo->geometryCount) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                             "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::geometryCount"
                             "[%d] must be greater than or equal to build info VkAccelerationStructureInfoNV::geometryCount [%d].",
                             dst_as_state->create_infoNV.info.geometryCount, pInfo->geometryCount);
        } else {
            for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
                const VkGeometryDataNV &create_geometry_data = dst_as_state->create_infoNV.info.pGeometries[i].geometry;
                const VkGeometryDataNV &build_geometry_data = pInfo->pGeometries[i].geometry;
                if (create_geometry_data.triangles.vertexCount < build_geometry_data.triangles.vertexCount) {
                    skip |= LogError(
                        commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                        "vkCmdBuildAccelerationStructureNV(): create info pGeometries[%d].geometry.triangles.vertexCount [%d]"
                        "must be greater than or equal to build info pGeometries[%d].geometry.triangles.vertexCount [%d].",
                        i, create_geometry_data.triangles.vertexCount, i, build_geometry_data.triangles.vertexCount);
                    break;
                }
                if (create_geometry_data.triangles.indexCount < build_geometry_data.triangles.indexCount) {
                    skip |= LogError(
                        commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                        "vkCmdBuildAccelerationStructureNV(): create info pGeometries[%d].geometry.triangles.indexCount [%d]"
                        "must be greater than or equal to build info pGeometries[%d].geometry.triangles.indexCount [%d].",
                        i, create_geometry_data.triangles.indexCount, i, build_geometry_data.triangles.indexCount);
                    break;
                }
                if (create_geometry_data.aabbs.numAABBs < build_geometry_data.aabbs.numAABBs) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                                     "vkCmdBuildAccelerationStructureNV(): create info pGeometries[%d].geometry.aabbs.numAABBs [%d]"
                                     "must be greater than or equal to build info pGeometries[%d].geometry.aabbs.numAABBs [%d].",
                                     i, create_geometry_data.aabbs.numAABBs, i, build_geometry_data.aabbs.numAABBs);
                    break;
                }
            }
        }
    }

    if (dst_as_state != nullptr) {
        skip |= ValidateMemoryIsBoundToAccelerationStructure(commandBuffer, *dst_as_state, "vkCmdBuildAccelerationStructureNV()",
                                                             "VUID-vkCmdBuildAccelerationStructureNV-dst-07787");
    }

    if (update == VK_TRUE) {
        if (src == VK_NULL_HANDLE) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-update-02489",
                             "vkCmdBuildAccelerationStructureNV(): If update is VK_TRUE, src must not be VK_NULL_HANDLE.");
        } else {
            if (src_as_state == nullptr || !src_as_state->built ||
                !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-update-02490",
                                 "vkCmdBuildAccelerationStructureNV(): If update is VK_TRUE, src must have been built before "
                                 "with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV set in "
                                 "VkAccelerationStructureInfoNV::flags.");
            }
        }
        if (dst_as_state != nullptr && !dst_as_state->update_scratch_memory_requirements_checked) {
            skip |=
                LogWarning(dst, kVUID_Core_CmdBuildAccelNV_NoUpdateMemReqQuery,
                           "vkCmdBuildAccelerationStructureNV(): Updating %s but vkGetAccelerationStructureMemoryRequirementsNV() "
                           "has not been called for update scratch memory.",
                           report_data->FormatHandle(dst_as_state->acceleration_structure()).c_str());
            // Use requirements fetched at create time
        }
        if (scratch_buffer_state != nullptr && dst_as_state != nullptr &&
            dst_as_state->update_scratch_memory_requirements.size > (scratch_buffer_state->createInfo.size - scratchOffset)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-update-02492",
                             "vkCmdBuildAccelerationStructureNV(): If update is VK_TRUE, The size member of the "
                             "VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV must be less than "
                             "or equal to the size of scratch minus scratchOffset");
        }
    } else {
        if (dst_as_state != nullptr && !dst_as_state->build_scratch_memory_requirements_checked) {
            skip |= LogWarning(dst, kVUID_Core_CmdBuildAccelNV_NoScratchMemReqQuery,
                               "vkCmdBuildAccelerationStructureNV(): Assigning scratch buffer to %s but "
                               "vkGetAccelerationStructureMemoryRequirementsNV() has not been called for scratch memory.",
                               report_data->FormatHandle(dst_as_state->acceleration_structure()).c_str());
            // Use requirements fetched at create time
        }
        if (scratch_buffer_state != nullptr && dst_as_state != nullptr &&
            dst_as_state->build_scratch_memory_requirements.size > (scratch_buffer_state->createInfo.size - scratchOffset)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBuildAccelerationStructureNV-update-02491",
                             "vkCmdBuildAccelerationStructureNV(): If update is VK_FALSE, The size member of the "
                             "VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV must be less than "
                             "or equal to the size of scratch minus scratchOffset");
        }
    }
    if (instanceData != VK_NULL_HANDLE) {
        auto buffer_state = Get<BUFFER_STATE>(instanceData);
        if (buffer_state) {
            skip |= ValidateBufferUsageFlags(commandBuffer, *buffer_state, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, true,
                                             "VUID-VkAccelerationStructureInfoNV-instanceData-02782",
                                             "vkCmdBuildAccelerationStructureNV()", "VK_BUFFER_USAGE_RAY_TRACING_BIT_NV");
        }
    }
    if (scratch_buffer_state) {
        skip |= ValidateBufferUsageFlags(commandBuffer, *scratch_buffer_state, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, true,
                                         "VUID-VkAccelerationStructureInfoNV-scratch-02781", "vkCmdBuildAccelerationStructureNV()",
                                         "VK_BUFFER_USAGE_RAY_TRACING_BIT_NV");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeNV mode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, CMD_COPYACCELERATIONSTRUCTURENV);
    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE>(dst);
    auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE>(src);

    if (dst_as_state != nullptr) {
        skip |= ValidateMemoryIsBoundToAccelerationStructure(commandBuffer, *dst_as_state, "vkCmdCopyAccelerationStructureNV()",
                                                             "VUID-vkCmdCopyAccelerationStructureNV-dst-07792");
    }

    if (mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV) {
        if (src_as_state != nullptr &&
            (!src_as_state->built || !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV))) {
            skip |= LogError(commandBuffer, "VUID-vkCmdCopyAccelerationStructureNV-src-03411",
                             "vkCmdCopyAccelerationStructureNV(): src must have been built with "
                             "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV if mode is "
                             "VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV.");
        }
    }
    if (!(mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV || mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdCopyAccelerationStructureNV-mode-03410",
                         "vkCmdCopyAccelerationStructureNV():mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR"
                         "or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks *pAllocator) const {
    auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(accelerationStructure);
    bool skip = false;
    if (as_state) {
        skip |= ValidateObjectNotInUse(as_state.get(), "vkDestroyAccelerationStructureNV",
                                       "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                const VkAllocationCallbacks *pAllocator) const {
    auto as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(accelerationStructure);
    bool skip = false;
    if (as_state) {
        skip |= ValidateObjectNotInUse(as_state.get(), "vkDestroyAccelerationStructureKHR",
                                       "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                          uint32_t accelerationStructureCount,
                                                                          const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                          VkQueryType queryType, VkQueryPool queryPool,
                                                                          uint32_t firstQuery) {
    if (disabled[query_validation]) return;
    // Enqueue the submit time validation check here, before the submit time state update in StateTracker::PostCall...
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    CMD_TYPE cmd_type = CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESKHR;
    cb_state->queryUpdates.emplace_back([accelerationStructureCount, firstQuery, queryPool, cmd_type](
                                            CMD_BUFFER_STATE &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                            uint32_t perfPass, QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        for (uint32_t i = 0; i < accelerationStructureCount; i++) {
            QueryObject query = {{queryPool, firstQuery + i}, perfPass};
            skip |= VerifyQueryIsReset(cb_state_arg, query, cmd_type, firstPerfQueryPool, perfPass, localQueryToStateMap);
        }
        return skip;
    });
}

bool CoreChecks::PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                         const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                         VkQueryType queryType, size_t dataSize, void *pData,
                                                                         size_t stride) const {
    bool skip = false;
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        auto as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pAccelerationStructures[i]);
        const auto &as_info = as_state->build_info_khr;
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            if (!(as_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                skip |= LogError(device, "VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431",
                                 "vkWriteAccelerationStructuresPropertiesKHR: All acceleration structures (%s) in "
                                 "pAccelerationStructures must have been built with"
                                 "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR if queryType is "
                                 "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR.",
                                 report_data->FormatHandle(as_state->acceleration_structure()).c_str());
            }
        }
        if (as_state) {
            skip |= ValidateHostVisibleMemoryIsBoundToBuffer(*as_state->buffer_state, "vkWriteAccelerationStructuresPropertiesKHR",
                                                             "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(*cb_state, CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESKHR);
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    if (query_pool_ci.queryType != queryType) {
        skip |= LogError(
            device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryPool-02493",
            "vkCmdWriteAccelerationStructuresPropertiesKHR: queryPool must have been created with a queryType matching queryType.");
    }
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            auto as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pAccelerationStructures[i]);
            if (!(as_state->build_info_khr.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                skip |= LogError(
                    device, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431",
                    "vkCmdWriteAccelerationStructuresPropertiesKHR: All acceleration structures in pAccelerationStructures "
                    "must have been built with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR if queryType is "
                    "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR.");
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t accelerationStructureCount,
                                                                           const VkAccelerationStructureNV *pAccelerationStructures,
                                                                           VkQueryType queryType, VkQueryPool queryPool,
                                                                           uint32_t firstQuery) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(*cb_state, CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESNV);
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    if (query_pool_ci.queryType != queryType) {
        skip |= LogError(
            device, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryPool-03755",
            "vkCmdWriteAccelerationStructuresPropertiesNV: queryPool must have been created with a queryType matching queryType.");
    }
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
            auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(pAccelerationStructures[i]);
            if (!(as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                skip |=
                    LogError(device, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-pAccelerationStructures-06215",
                             "vkCmdWriteAccelerationStructuresPropertiesNV: All acceleration structures in pAccelerationStructures "
                             "must have been built with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR if queryType is "
                             "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV.");
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                          const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                          const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                          const uint32_t *pIndirectStrides,
                                                                          const uint32_t *const *ppMaxPrimitiveCounts) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_BUILDACCELERATIONSTRUCTURESINDIRECTKHR);
    skip |= ValidateCmdRayQueryState(*cb_state, CMD_BUILDACCELERATIONSTRUCTURESINDIRECTKHR, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    for (uint32_t i = 0; i < infoCount; ++i) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[i].srcAccelerationStructure);
        auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[i].dstAccelerationStructure);
        skip |=
            ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_as_state->buffer_state, "vkCmdBuildAccelerationStructuresIndirectKHR",
                                          "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03707");
        if (pInfos[i].mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
            skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_as_state->buffer_state,
                                                  "vkCmdBuildAccelerationStructuresIndirectKHR",
                                                  "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03708");
            if (src_as_state == nullptr || !src_as_state->built ||
                !(src_as_state->build_info_khr.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR)) {
                skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03667",
                                 "vkCmdBuildAccelerationStructuresIndirectKHR(): For each element of pInfos, if its mode member is "
                                 "VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its srcAccelerationStructure member must have "
                                 "been built before with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR set in "
                                 "VkAccelerationStructureBuildGeometryInfoKHR::flags.");
            }
            if (pInfos[i].geometryCount != src_as_state->build_info_khr.geometryCount) {
                skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03758",
                                 "vkCmdBuildAccelerationStructuresIndirectKHR(): For each element of pInfos, if its mode member is "
                                 "VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,"
                                 " its geometryCount member must have the same value which was specified when "
                                 "srcAccelerationStructure was last built.");
            }
            if (pInfos[i].flags != src_as_state->build_info_khr.flags) {
                skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03759",
                                 "vkCmdBuildAccelerationStructuresIndirectKHR(): For each element of pInfos, if its mode member is"
                                 " VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its flags member must have the same value which"
                                 " was specified when srcAccelerationStructure was last built.");
            }
            if (pInfos[i].type != src_as_state->build_info_khr.type) {
                skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03760",
                                 "vkCmdBuildAccelerationStructuresIndirectKHR(): For each element of pInfos, if its mode member is"
                                 " VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, its type member must have the same value which"
                                 " was specified when srcAccelerationStructure was last built.");
            }
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
            if (!dst_as_state ||
                (dst_as_state && dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
                 dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR)) {
                skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03700",
                                 "vkCmdBuildAccelerationStructuresIndirectKHR(): For each element of pInfos, if its type member is "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, its dstAccelerationStructure member must have "
                                 "been created with a value of VkAccelerationStructureCreateInfoKHR::type equal to either "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
            }
        }
        if (pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
            if (!dst_as_state ||
                (dst_as_state && dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                 dst_as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR)) {
                skip |= LogError(device, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03699",
                                 "vkCmdBuildAccelerationStructuresIndirectKHR():For each element of pInfos, if its type member is "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, its dstAccelerationStructure member must have been "
                                 "created with a value of VkAccelerationStructureCreateInfoKHR::type equal to either "
                                 "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                          const char *api_name) const {
    bool skip = false;
    if (pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
        if (!(src_as_state->build_info_khr.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
            skip |= LogError(device, "VUID-VkCopyAccelerationStructureInfoKHR-src-03411",
                             "(%s): src must have been built with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR"
                             "if mode is VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR.",
                             api_name);
        }
    }
    auto src_accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
    if (src_accel_state) {
        auto buffer_state = Get<BUFFER_STATE>(src_accel_state->create_infoKHR.buffer);
        skip |=
            ValidateMemoryIsBoundToBuffer(device, *buffer_state, api_name, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    }
    auto dst_accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
    if (dst_accel_state) {
        auto buffer_state = Get<BUFFER_STATE>(dst_accel_state->create_infoKHR.buffer);
        skip |=
            ValidateMemoryIsBoundToBuffer(device, *buffer_state, api_name, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    skip |= ValidateCmd(*cb_state, CMD_COPYACCELERATIONSTRUCTUREKHR);
    if (pInfo) {
        skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCmdCopyAccelerationStructureKHR");
        auto src_accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
        if (src_accel_state) {
            skip |=
                ValidateMemoryIsBoundToBuffer(commandBuffer, *src_accel_state->buffer_state, "vkCmdCopyAccelerationStructureKHR",
                                              "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
        }
        auto dst_accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
        if (dst_accel_state) {
            skip |=
                ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_accel_state->buffer_state, "vkCmdCopyAccelerationStructureKHR",
                                              "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;
    if (pInfo) {
        skip |= ValidateCopyAccelerationStructureInfoKHR(pInfo, "vkCopyAccelerationStructureKHR");
        auto src_accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
        if (src_accel_state) {
            skip |= ValidateHostVisibleMemoryIsBoundToBuffer(*src_accel_state->buffer_state, "vkCopyAccelerationStructureKHR",
                                                             "VUID-vkCopyAccelerationStructureKHR-buffer-03727");
        }
        auto dst_accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
        if (dst_accel_state) {
            skip |= ValidateHostVisibleMemoryIsBoundToBuffer(*dst_accel_state->buffer_state, "vkCopyAccelerationStructureKHR",
                                                             "VUID-vkCopyAccelerationStructureKHR-buffer-03728");
        }
    }
    return skip;
}
bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_COPYACCELERATIONSTRUCTURETOMEMORYKHR);

    auto accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
    if (accel_state) {
        auto buffer_state = Get<BUFFER_STATE>(accel_state->create_infoKHR.buffer);
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, "vkCmdCopyAccelerationStructureToMemoryKHR",
                                              "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-None-03559");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    bool skip = false;

    auto accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
    if (accel_state) {
        skip |= ValidateHostVisibleMemoryIsBoundToBuffer(*accel_state->buffer_state, "vkCopyMemoryToAccelerationStructureKHR",
                                                         "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_COPYMEMORYTOACCELERATIONSTRUCTUREKHR);

    auto accel_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
    if (accel_state) {
        skip |=
            ValidateMemoryIsBoundToBuffer(commandBuffer, *accel_state->buffer_state, "vkCmdCopyAccelerationStructureToMemoryKHR",
                                          "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    }
    return skip;
}

bool CoreChecks::ValidateCmdRayQueryState(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type,
                                          const VkPipelineBindPoint bind_point) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipe = last_bound.pipeline_state;

    bool ray_query_shader = false;
    if (nullptr != pipe) {
        if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            ray_query_shader = true;
        } else {
            // TODO - Loop through shader for RayQueryKHR for draw/dispatch commands
        }
    }

    if (cb_state.unprotected == false && ray_query_shader) {
        skip |= LogError(cb_state.commandBuffer(), vuid.ray_query_protected_cb,
                         "%s(): can't use in protected command buffers for RayQuery operations.", CommandTypeString(cmd_type));
    }

    return skip;
}

uint32_t CoreChecks::CalcTotalShaderGroupCount(const PIPELINE_STATE &pipeline) const {
    uint32_t total = 0;
    if (pipeline.GetCreateInfoSType() == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR) {
        const auto &create_info = pipeline.GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
        total = create_info.groupCount;

        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                auto library_pipeline_state = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[i]);
                total += CalcTotalShaderGroupCount(*library_pipeline_state.get());
            }
        }
    } else if (pipeline.GetCreateInfoSType() == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV) {
        const auto &create_info = pipeline.GetCreateInfo<VkRayTracingPipelineCreateInfoNV>();
        total = create_info.groupCount;

        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                auto library_pipeline_state = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[i]);
                total += CalcTotalShaderGroupCount(*library_pipeline_state.get());
            }
        }
    }

    return total;
}

bool CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void *pData) const {
    bool skip = false;
    auto pPipeline = Get<PIPELINE_STATE>(pipeline);
    if (!pPipeline) {
        return skip;
    }
    const PIPELINE_STATE &pipeline_state = *pPipeline;
    if (pipeline_state.GetPipelineCreateFlags() & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        if (!enabled_features.pipeline_library_group_handles_features.pipelineLibraryGroupHandles) {
            const char *vuid = IsExtEnabled(device_extensions.vk_ext_pipeline_library_group_handles)
                                   ? "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-07828"
                                   : "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-03482";
            skip |= LogError(device, vuid,
                             "vkGetRayTracingShaderGroupHandlesKHR: If the pipelineLibraryGroupHandles feature is not enabled, "
                             "pipeline must have not been created with "
                             "VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.");
        }
    }
    if (dataSize < (phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleSize * groupCount)) {
        skip |= LogError(device, "VUID-vkGetRayTracingShaderGroupHandlesKHR-dataSize-02420",
                         "vkGetRayTracingShaderGroupHandlesKHR: dataSize (%zu) must be at least "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleSize * groupCount.",
                         dataSize);
    }

    uint32_t total_group_count = CalcTotalShaderGroupCount(pipeline_state);

    if (firstGroup >= total_group_count) {
        skip |=
            LogError(device, "VUID-vkGetRayTracingShaderGroupHandlesKHR-firstGroup-04050",
                     "vkGetRayTracingShaderGroupHandlesKHR: firstGroup must be less than the number of shader groups in pipeline.");
    }
    if ((firstGroup + groupCount) > total_group_count) {
        skip |= LogError(
            device, "VUID-vkGetRayTracingShaderGroupHandlesKHR-firstGroup-02419",
            "vkGetRayTracingShaderGroupHandlesKHR: The sum of firstGroup and groupCount must be less than or equal the number "
            "of shader groups in pipeline.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                uint32_t firstGroup, uint32_t groupCount,
                                                                                size_t dataSize, void *pData) const {
    bool skip = false;
    if (dataSize < (phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleCaptureReplaySize * groupCount)) {
        skip |= LogError(device, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484",
                         "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR: dataSize (%zu) must be at least "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleCaptureReplaySize * groupCount.",
                         dataSize);
    }
    auto pipeline_state = Get<PIPELINE_STATE>(pipeline);
    if (!pipeline_state) {
        return skip;
    }
    const auto &create_info = pipeline_state->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
    if (create_info.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        if (!enabled_features.pipeline_library_group_handles_features.pipelineLibraryGroupHandles) {
            const char *vuid = IsExtEnabled(device_extensions.vk_ext_pipeline_library_group_handles)
                                   ? "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-07829"
                                   : "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-07830";
            skip |= LogError(
                device, vuid,
                "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR: If the pipelineLibraryGroupHandles feature is not enabled, "
                "pipeline must have not been created with "
                "VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.");
        }
    }
    if (firstGroup >= create_info.groupCount) {
        skip |= LogError(device, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-04051",
                         "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR: firstGroup must be less than the number of shader "
                         "groups in pipeline.");
    }
    if ((firstGroup + groupCount) > create_info.groupCount) {
        skip |= LogError(device, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483",
                         "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR: The sum of firstGroup and groupCount must be less "
                         "than or equal to the number of shader groups in pipeline.");
    }
    if (!(create_info.flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
        skip |= LogError(device, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-03607",
                         "pipeline must have been created with a flags that included "
                         "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer,
                                                                     uint32_t pipelineStackSize) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETRAYTRACINGPIPELINESTACKSIZEKHR, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                     VkShaderGroupShaderKHR groupShader) const {
    bool skip = false;
    auto pipeline_state = Get<PIPELINE_STATE>(pipeline);
    if (pipeline_state) {
        if (pipeline_state->GetPipelineType() != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            skip |=
                LogError(device, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-04622",
                         "vkGetRayTracingShaderGroupStackSizeKHR: Pipeline must be a ray-tracing pipeline, but is a %s pipeline.",
                         string_VkPipelineBindPoint(pipeline_state->GetPipelineType()));
        } else if (group >= pipeline_state->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>().groupCount) {
            skip |=
                LogError(device, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-group-03608",
                         "vkGetRayTracingShaderGroupStackSizeKHR: The value of group must be less than the number of shader groups "
                         "in pipeline.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, const char *func_name) const {
    bool skip = false;

    auto vb_state = Get<BUFFER_STATE>(triangles.vertexData);
    if (vb_state != nullptr && vb_state->createInfo.size <= triangles.vertexOffset) {
        skip |= LogError(device, "VUID-VkGeometryTrianglesNV-vertexOffset-02428", "%s", func_name);
    }

    auto ib_state = Get<BUFFER_STATE>(triangles.indexData);
    if (ib_state != nullptr && ib_state->createInfo.size <= triangles.indexOffset) {
        skip |= LogError(device, "VUID-VkGeometryTrianglesNV-indexOffset-02431", "%s", func_name);
    }

    auto td_state = Get<BUFFER_STATE>(triangles.transformData);
    if (td_state != nullptr && td_state->createInfo.size <= triangles.transformOffset) {
        skip |= LogError(device, "VUID-VkGeometryTrianglesNV-transformOffset-02437", "%s", func_name);
    }

    return skip;
}

bool CoreChecks::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, const char *func_name) const {
    bool skip = false;

    auto aabb_state = Get<BUFFER_STATE>(aabbs.aabbData);
    if (aabb_state != nullptr && aabb_state->createInfo.size > 0 && aabb_state->createInfo.size <= aabbs.offset) {
        skip |= LogError(device, "VUID-VkGeometryAABBNV-offset-02439", "%s", func_name);
    }

    return skip;
}

bool CoreChecks::ValidateGeometryNV(const VkGeometryNV &geometry, const char *func_name) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip = ValidateGeometryTrianglesNV(geometry.geometry.triangles, func_name);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip = ValidateGeometryAABBNV(geometry.geometry.aabbs, func_name);
    }
    return skip;
}

bool CoreChecks::ValidateRaytracingShaderBindingTable(VkCommandBuffer commandBuffer, const char *rt_func_name,
                                                      const char *vuid_single_device_memory, const char *vuid_binding_table_flag,
                                                      const VkStridedDeviceAddressRegionKHR &binding_table,
                                                      const char *binding_table_name) const {
    bool skip = false;

    if (binding_table.deviceAddress == 0) {
        return skip;
    }

    const auto buffer_states = GetBuffersByAddress(binding_table.deviceAddress);
    if (buffer_states.empty()) {
        skip |= LogError(device, "VUID-VkStridedDeviceAddressRegionKHR-size-04631",
                         "%s: no buffer is associated with %s->deviceAddress (0x%" PRIx64 ").", rt_func_name, binding_table_name,
                         binding_table.deviceAddress);
    } else {
        // Try to find a buffer satisfying all VUIDs
        const bool no_valid_buffer_found = std::none_of(
            buffer_states.begin(), buffer_states.end(),
            [&binding_table](const ValidationStateTracker::BUFFER_STATE_PTR &buffer_state) {
                assert(buffer_state);

                if (!buffer_state) {
                    return false;
                }

                if (!(static_cast<uint32_t>(buffer_state->createInfo.usage) & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                    return false;
                }
                if (!buffer_state->sparse) {
                    if (const auto mem_state = buffer_state->MemState(); !mem_state || mem_state->Destroyed()) {
                        return false;
                    }
                }
                if (binding_table.size != 0) {
                    const auto device_address_range = buffer_state->DeviceAddressRange();
                    const sparse_container::range<VkDeviceSize> requested_range(
                        binding_table.deviceAddress, binding_table.deviceAddress + binding_table.size - 1);
                    if (!device_address_range.includes(requested_range)) {
                        return false;
                    }

                    if (binding_table.size != 0 && binding_table.stride > buffer_state->createInfo.size) {
                        return false;
                    }
                }

                return true;
            });

        // If no valid buffer was found, for each violated VUID,
        // output the list of buffers (associated to binding_table.deviceAddress) that violates it,
        // alongside relevant info.
        if (no_valid_buffer_found) {
            struct InvalidBuffers {
                LogObjectList buffers;
                std::string error_msg;
            };

            InvalidBuffers vuid_binding_table_flag_invalid_buffers{{commandBuffer}};
            InvalidBuffers vuid_address_range_invalid_buffers{{commandBuffer}};
            InvalidBuffers vuid_stride_invalid_buffers{{commandBuffer}};
            const std::string address_string = std::to_string(binding_table.deviceAddress);
            const sparse_container::range<VkDeviceSize> requested_range(binding_table.deviceAddress,
                                                                        binding_table.deviceAddress + binding_table.size - 1);
            std::string requested_range_string =
                '[' + std::to_string(requested_range.begin) + ", " + std::to_string(requested_range.end) + ')';
            std::stringstream error_msg_prefix_ss;
            error_msg_prefix_ss << rt_func_name << ": No buffer associated to" << binding_table_name << "->deviceAddress (0x"
                                << address_string
                                << ") was found such that valid usage passes. "
                                   "At least one buffer associated to this device address must be valid. The following buffers ";
            const auto error_msg_prefix = error_msg_prefix_ss.str();
            for (const auto &buffer_state : buffer_states) {
                assert(buffer_state);

                if (!buffer_state) {
                    continue;
                }

                skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, rt_func_name, vuid_single_device_memory);

                if (!(static_cast<uint32_t>(buffer_state->createInfo.usage) & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                    vuid_binding_table_flag_invalid_buffers.buffers.add(buffer_state->Handle());

                    std::string &error_msg = vuid_binding_table_flag_invalid_buffers.error_msg;
                    if (error_msg.empty()) {
                        error_msg += error_msg_prefix +
                                     "have not been created with the VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR usage flag:\n";
                    }
                    // +1 because commandBuffer takes index 0 in the log object list
                    const auto obj_index = vuid_binding_table_flag_invalid_buffers.buffers.object_list.size() - 1 + 1;
                    error_msg += "Object " + std::to_string(obj_index) + ": buffer has usage " +
                                 string_VkBufferUsageFlags(buffer_state->createInfo.usage) + '\n';
                }

                if (binding_table.size != 0) {
                    const auto buffer_address_range = buffer_state->DeviceAddressRange();
                    if (!buffer_address_range.includes(requested_range)) {
                        vuid_address_range_invalid_buffers.buffers.add(buffer_state->Handle());

                        std::string &error_msg = vuid_address_range_invalid_buffers.error_msg;
                        if (error_msg.empty()) {
                            error_msg += error_msg_prefix + "do not include " + binding_table_name +
                                         " buffer device address range " + requested_range_string + ":\n";
                        }
                        const auto obj_index = vuid_address_range_invalid_buffers.buffers.object_list.size() - 1 + 1;
                        const std::string buffer_address_range_string = '[' + std::to_string(buffer_address_range.begin) + ", " +
                                                                        std::to_string(buffer_address_range.end) + ')';
                        error_msg += "Object " + std::to_string(obj_index) + ": buffer device address range is " +
                                     buffer_address_range_string + '\n';
                    }

                    if (binding_table.size != 0 && binding_table.stride > buffer_state->createInfo.size) {
                        vuid_stride_invalid_buffers.buffers.add(buffer_state->Handle());

                        std::string &error_msg = vuid_stride_invalid_buffers.error_msg;
                        if (error_msg.empty()) {
                            error_msg += error_msg_prefix + "have a size inferior to " + binding_table_name + "->stride (" +
                                         std::to_string(binding_table.stride) + "):\n";
                        }
                        const auto obj_index = vuid_stride_invalid_buffers.buffers.object_list.size() - 1 + 1;
                        error_msg += "Object " + std::to_string(obj_index) + ": buffer size is " +
                                     std::to_string(buffer_state->createInfo.size) + '\n';
                    }
                }
            }

            if (!vuid_binding_table_flag_invalid_buffers.error_msg.empty()) {
                skip |= LogError(vuid_binding_table_flag_invalid_buffers.buffers, vuid_binding_table_flag, "%s",
                                 vuid_binding_table_flag_invalid_buffers.error_msg.c_str());
            }

            if (!vuid_address_range_invalid_buffers.error_msg.empty()) {
                skip |= LogError(vuid_address_range_invalid_buffers.buffers, "VUID-VkStridedDeviceAddressRegionKHR-size-04631",
                                 "%s", vuid_address_range_invalid_buffers.error_msg.c_str());
            }

            if (!vuid_stride_invalid_buffers.error_msg.empty()) {
                skip |= LogError(vuid_stride_invalid_buffers.buffers, "VUID-VkStridedDeviceAddressRegionKHR-size-04632", "%s",
                                 vuid_stride_invalid_buffers.error_msg.c_str());
            }
        }
    }

    return skip;
}
