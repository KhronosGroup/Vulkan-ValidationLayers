/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
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
#include <cmath>
#include <cstdint>
#include "error_message/logging.h"
#include "stateless/stateless_validation.h"
#include "utils/math_utils.h"

namespace stateless {
bool Device::manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                       VkQueryResultFlags flags, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (queryCount > 1 && stride == 0) {
        skip |= LogError("VUID-vkGetQueryPoolResults-queryCount-09438", queryPool, error_obj.location.dot(Field::queryCount),
                         "is %" PRIu32 " but stride is zero.", queryCount);
    }

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkGetQueryPoolResults-flags-09443", queryPool, error_obj.location.dot(Field::flags),
                         "(%s) include both STATUS_BIT and AVAILABILITY_BIT.", string_VkQueryResultFlags(flags).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdCopyQueryPoolResultsToMemoryKHR(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                      uint32_t firstQuery, uint32_t queryCount,
                                                                      const VkStridedDeviceAddressRangeKHR* pDstRange,
                                                                      VkAddressCommandFlagsKHR dstFlags,
                                                                      VkQueryResultFlags queryResultFlags,
                                                                      const Context& context) const {
    bool skip = false;

    const auto& error_obj = context.error_obj;
    const LogObjectList objlist(commandBuffer, queryPool);
    skip |= context.ValidateDeviceAddressFlags(error_obj.location.dot(Field::dstFlags), dstFlags);

    if (queryCount > 1 && pDstRange->stride == 0) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-queryCount-09438", objlist,
                         error_obj.location.dot(Field::queryCount), "is %" PRIu32 " but stride is zero.", queryCount);
    }

    if (queryResultFlags & VK_QUERY_RESULT_64_BIT) {
        if (!IsPointerAligned(pDstRange->address, 8) || !IsIntegerMultipleOf(pDstRange->stride, 8)) {
            skip |= LogError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-13078", objlist,
                             error_obj.location.dot(Field::pDstRange).dot(Field::address),
                             "(0x%" PRIx64 ") must be aligned to 8 and pDstRange.stride (%" PRIu64 ") must be a multiple of 8.",
                             pDstRange->address, pDstRange->stride);
        }
    } else {
        if (!IsPointerAligned(pDstRange->address, 4) || !IsIntegerMultipleOf(pDstRange->stride, 4)) {
            skip |= LogError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-13077", objlist,
                             error_obj.location.dot(Field::pDstRange).dot(Field::address),
                             "(0x%" PRIx64 ") must be aligned to 4 and pDstRange.stride (%" PRIu64 ") must be a multiple of 4.",
                             pDstRange->address, pDstRange->stride);
        }
    }

    if ((queryResultFlags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (queryResultFlags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-09443", objlist,
                         error_obj.location.dot(Field::queryResultFlags),
                         "(%s) includes both VK_QUERY_RESULT_WITH_STATUS_BIT_KHR and VK_QUERY_RESULT_WITH_AVAILABILITY_BIT.",
                         string_VkQueryResultFlags(queryResultFlags).c_str());
    }

    if (dstFlags & VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-dstFlags-13085", objlist,
                         error_obj.location.dot(Field::dstFlags), "(%s) must not contain VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR.",
                         string_VkAddressCommandFlagsKHR(dstFlags).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                  const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    const LogObjectList objlist(device, queryPool);

    if (!enabled_features.hostQueryReset) {
        skip |= LogError("VUID-vkResetQueryPool-None-02665", device, error_obj.location, "hostQueryReset feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                                           const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    const LogObjectList objlist(commandBuffer, queryPool);

    if (flags & VK_QUERY_RESULT_64_BIT) {
        if (queryCount > 1 && !IsIntegerMultipleOf(stride, 8)) {
            skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryCount-12255", objlist, error_obj.location.dot(Field::stride),
                             "(%" PRIu64 ") is not a multiple of 8. (queryCount is %" PRIu32 ")", stride, queryCount);
        }
        if (!IsIntegerMultipleOf(dstOffset, 8)) {
            skip |= LogError("VUID-vkCmdCopyQueryPoolResults-flags-00823", objlist, error_obj.location.dot(Field::dstOffset),
                             "(%" PRIu64 ") is not a multiple of 8.", dstOffset);
        }
    } else {
        if (queryCount > 1 && !IsIntegerMultipleOf(stride, 4)) {
            skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryCount-12254", objlist, error_obj.location.dot(Field::stride),
                             "(%" PRIu64 ") is not a multiple of 4. (queryCount is %" PRIu32 ")", stride, queryCount);
        }
        if (!IsIntegerMultipleOf(dstOffset, 4)) {
            skip |= LogError("VUID-vkCmdCopyQueryPoolResults-flags-00822", objlist, error_obj.location.dot(Field::dstOffset),
                             "(%" PRIu64 ") is not a multiple of 4.", dstOffset);
        }
    }

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-flags-09443", objlist, error_obj.location.dot(Field::flags),
                         "(%s) include both STATUS_BIT and AVAILABILITY_BIT.", string_VkQueryResultFlags(flags).c_str());
    }

    if (queryCount > 1 && stride == 0) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryCount-09438", objlist, error_obj.location.dot(Field::queryCount),
                         "is %" PRIu32 " but stride is zero.", queryCount);
    }
    return skip;
}

}  // namespace stateless