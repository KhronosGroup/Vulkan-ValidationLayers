/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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
#include "state_tracker/ray_tracing_state.h"

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
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if ((cb_state->GetQueueFlags() & VK_QUEUE_GRAPHICS_BIT) != 0) {
            skip |= LogPerformanceWarning("BestPractices-NVIDIA-AccelerationStructure-NotAsync", commandBuffer, loc,
                                          "%s Prefer building acceleration structures on an asynchronous "
                                          "compute queue, instead of using the universal graphics queue.",
                                          VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}
