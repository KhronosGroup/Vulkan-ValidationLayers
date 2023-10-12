/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

bool BestPractices::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                   const VkAccelerationStructureInfoNV* pInfo,
                                                                   VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                                   VkBool32 update, VkAccelerationStructureNV dst,
                                                                   VkAccelerationStructureNV src, VkBuffer scratch,
                                                                   VkDeviceSize scratchOffset, const ErrorObject& error_obj) const {
    return ValidateBuildAccelerationStructure(commandBuffer, error_obj.location);
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides, const uint32_t* const* ppMaxPrimitiveCounts,
    const ErrorObject& error_obj) const {
    return ValidateBuildAccelerationStructure(commandBuffer, error_obj.location);
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const ErrorObject& error_obj) const {
    return ValidateBuildAccelerationStructure(commandBuffer, error_obj.location);
}

bool BestPractices::ValidateBuildAccelerationStructure(VkCommandBuffer commandBuffer, const Location& loc) const {
    bool skip = false;
    auto cb_node = GetRead<bp_state::CommandBuffer>(commandBuffer);
    assert(cb_node);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if ((cb_node->GetQueueFlags() & VK_QUEUE_GRAPHICS_BIT) != 0) {
            skip |= LogPerformanceWarning(kVUID_BestPractices_AccelerationStructure_NotAsync, commandBuffer, loc,
                                          "%s Performance warning: Prefer building acceleration structures on an asynchronous "
                                          "compute queue, instead of using the universal graphics queue.",
                                          VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                     const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        auto as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(pBindInfos[i].accelerationStructure);
        if (!as_state->memory_requirements_checked) {
            // There's not an explicit requirement in the spec to call vkGetImageMemoryRequirements() prior to calling
            // BindAccelerationStructureMemoryNV but it's implied in that memory being bound must conform with
            // VkAccelerationStructureMemoryRequirementsInfoNV from vkGetAccelerationStructureMemoryRequirementsNV
            skip |= LogWarning(
                kVUID_BestPractices_BindAccelNV_NoMemReqQuery, device, error_obj.location,
                "Binding memory to %s but vkGetAccelerationStructureMemoryRequirementsNV() has not been called on that structure.",
                FormatHandle(pBindInfos[i].accelerationStructure).c_str());
        }
    }

    return skip;
}
