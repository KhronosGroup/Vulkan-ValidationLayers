/* Copyright (c) 2026 LunarG, Inc.
 * Copyright (C) 2026 Google Inc.
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
#include "utils/math_utils.h"

namespace stateless {

// (VK_KHR_device_address_commands)
bool Device::ValidateDrawIndirect2Info(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR& info, const Location& info_loc,
                                       const Context& context) const {
    bool skip = false;

    skip |= context.ValidateDeviceAddressFlags(info_loc.dot(Field::addressFlags), info.addressFlags);

    if (!IsPointerAligned(info.addressRange.address, 4u)) {
        skip |= LogError("VUID-VkDrawIndirect2InfoKHR-addressRange-13109", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::address), "(0x%" PRIu64 ") must be aligned to 4 bytes.",
                         info.addressRange.address);
    }

    if (!enabled_features.multiDrawIndirect && (info.drawCount > 1)) {
        skip |= LogError("VUID-VkDrawIndirect2InfoKHR-drawCount-02718", commandBuffer, info_loc.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", info.drawCount);
    } else if (info.drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-VkDrawIndirect2InfoKHR-drawCount-02719", commandBuffer, info_loc.dot(Field::drawCount),
                         "%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", info.drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdDrawIndirect2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR *pInfo,
                                                       const Context &context) const {
    bool skip = false;

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    skip |= ValidateDrawIndirect2Info(commandBuffer, *pInfo, info_loc, context);

    const VkStridedDeviceAddressRangeKHR& address_range = pInfo->addressRange;
    if (pInfo->drawCount > 0) {
        const VkDeviceSize required_size = (pInfo->drawCount - 1) * address_range.stride + sizeof(VkDrawIndirectCommand);
        if (address_range.size < required_size) {
            skip |=
                LogError("VUID-vkCmdDrawIndirect2KHR-pInfo-13110", commandBuffer, info_loc.dot(Field::drawCount),
                         "is %" PRIu32 " (non-zero), but addressRange.size (%" PRIu64 ") is less than the required size of %" PRIu64
                         ".\nRequired size: (drawCount - 1) * stride + sizeof(VkDrawIndirectCommand) -> (%" PRIu32
                         " - 1) * %" PRIu64 " + %zu",
                         pInfo->drawCount, address_range.size, required_size, pInfo->drawCount, address_range.stride,
                         sizeof(VkDrawIndirectCommand));
        }
    }
    if (address_range.stride == 0) {
        if (pInfo->drawCount > 1) {
            skip |= LogError("VUID-vkCmdDrawIndirect2KHR-pInfo-13111", commandBuffer,
                             info_loc.dot(Field::addressRange).dot(Field::drawCount),
                             "is %" PRIu32 ", but addressRange.stride is 0.", pInfo->drawCount);
        }
    } else if (address_range.stride < sizeof(VkDrawIndirectCommand)) {
        skip |=
            LogError("VUID-vkCmdDrawIndirect2KHR-pInfo-13112", commandBuffer, info_loc.dot(Field::addressRange).dot(Field::stride),
                     "(%" PRIu64 ") must be at least sizeof(VkDrawIndirectCommand) (%zu).", address_range.stride,
                     sizeof(VkDrawIndirectCommand));
    } else if (!IsIntegerMultipleOf(address_range.stride, 4u)) {
        skip |=
            LogError("VUID-vkCmdDrawIndirect2KHR-pInfo-13113", commandBuffer, info_loc.dot(Field::addressRange).dot(Field::stride),
                     "(%" PRIu64 ") must be a multiple of 4.", address_range.stride);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdDrawIndexedIndirect2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR *pInfo,
                                                              const Context &context) const {
    bool skip = false;

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    skip |= ValidateDrawIndirect2Info(commandBuffer, *pInfo, info_loc, context);

    const VkStridedDeviceAddressRangeKHR& address_range = pInfo->addressRange;
    if (pInfo->drawCount > 0) {
        const VkDeviceSize required_size = (pInfo->drawCount - 1) * address_range.stride + sizeof(VkDrawIndexedIndirectCommand);
        if (address_range.size < required_size) {
            skip |=
                LogError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13110", commandBuffer, info_loc.dot(Field::drawCount),
                         "is %" PRIu32 " (non-zero), but addressRange.size (%" PRIu64 ") is less than the required size of %" PRIu64
                         ".\nRequired size: (drawCount - 1) * stride + sizeof(VkDrawIndexedIndirectCommand) -> (%" PRIu32
                         " - 1) * %" PRIu64 " + %zu",
                         pInfo->drawCount, address_range.size, required_size, pInfo->drawCount, address_range.stride,
                         sizeof(VkDrawIndexedIndirectCommand));
        }
    }
    if (address_range.stride == 0) {
        if (pInfo->drawCount > 1) {
            skip |= LogError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13111", commandBuffer,
                             info_loc.dot(Field::addressRange).dot(Field::drawCount),
                             "is %" PRIu32 ", but addressRange.stride is 0.", pInfo->drawCount);
        }
    } else if (address_range.stride < sizeof(VkDrawIndexedIndirectCommand)) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13112", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::stride),
                         "(%" PRIu64 ") must be at least sizeof(VkDrawIndexedIndirectCommand) (%zu).", address_range.stride,
                         sizeof(VkDrawIndexedIndirectCommand));
    } else if (!IsIntegerMultipleOf(address_range.stride, 4u)) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13113", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::stride), "(%" PRIu64 ") must be a multiple of 4.",
                         address_range.stride);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdDrawMeshTasksIndirect2EXT(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR *pInfo,
                                                                const Context &context) const {
    bool skip = false;

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    skip |= ValidateDrawIndirect2Info(commandBuffer, *pInfo, info_loc, context);

    return skip;
}

// (VK_KHR_device_address_commands)
bool Device::ValidateDrawIndirectCount2Info(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR& info,
                                            const Location& info_loc, const Context& context) const {
    bool skip = false;

    skip |= context.ValidateDeviceAddressFlags(info_loc.dot(Field::addressFlags), info.addressFlags);
    skip |= context.ValidateDeviceAddressFlags(info_loc.dot(Field::countAddressFlags), info.countAddressFlags);

    if (!IsPointerAligned(info.addressRange.address, 4u)) {
        skip |= LogError("VUID-VkDrawIndirectCount2InfoKHR-addressRange-13109", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::address), "(0x%" PRIx64 ") must be aligned to 4 bytes.",
                         info.addressRange.address);
    }
    if (!IsPointerAligned(info.countAddressRange.address, 4u)) {
        skip |= LogError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13115", commandBuffer,
                         info_loc.dot(Field::countAddressRange).dot(Field::address), "(0x%" PRIx64 ") must be aligned to 4 bytes.",
                         info.countAddressRange.address);
    }
    if (info.countAddressRange.size < 4) {
        skip |= LogError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13117", commandBuffer,
                         info_loc.dot(Field::countAddressRange).dot(Field::size), "(%" PRIu64 ") is less than 4.",
                         info.countAddressRange.size);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdDrawIndirectCount2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR *pInfo,
                                                            const Context &context) const {
    bool skip = false;

    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount2KHR-drawIndirectCount-13119", commandBuffer, context.error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    skip |= ValidateDrawIndirectCount2Info(commandBuffer, *pInfo, info_loc, context);

    const VkStridedDeviceAddressRangeKHR& address_range = pInfo->addressRange;
    if (pInfo->maxDrawCount > 0) {
        const VkDeviceSize required_size = (pInfo->maxDrawCount - 1) * address_range.stride + sizeof(VkDrawIndirectCommand);
        if (address_range.size < required_size) {
            skip |=
                LogError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13110", commandBuffer, info_loc.dot(Field::maxDrawCount),
                         "is %" PRIu32 " (non-zero), but addressRange.size (%" PRIu64 ") is less than the required size of %" PRIu64
                         ".\nRequired size: (maxDrawCount - 1) * stride + sizeof(VkDrawIndirectCommand) -> (%" PRIu32
                         " - 1) * %" PRIu64 " + %zu",
                         pInfo->maxDrawCount, address_range.size, required_size, pInfo->maxDrawCount, address_range.stride,
                         sizeof(VkDrawIndirectCommand));
        }
    }
    if (address_range.stride == 0) {
        if (pInfo->maxDrawCount > 1) {
            skip |= LogError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13111", commandBuffer,
                             info_loc.dot(Field::addressRange).dot(Field::drawCount),
                             "is %" PRIu32 ", but addressRange.stride is 0.", pInfo->maxDrawCount);
        }
    } else if (address_range.stride < sizeof(VkDrawIndirectCommand)) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13112", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::stride),
                         "(%" PRIu64 ") must be at least sizeof(VkDrawIndirectCommand) (%zu).", address_range.stride,
                         sizeof(VkDrawIndirectCommand));
    } else if (!IsIntegerMultipleOf(address_range.stride, 4u)) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13113", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::stride), "(%" PRIu64 ") must be a multiple of 4.",
                         address_range.stride);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdDrawIndexedIndirectCount2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkDrawIndirectCount2InfoKHR *pInfo,
                                                                   const Context &context) const {
    bool skip = false;

    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |=
            LogError("VUID-vkCmdDrawIndexedIndirectCount2KHR-drawIndirectCount-13120", commandBuffer, context.error_obj.location,
                     "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                     "call this command.");
    }

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    skip |= ValidateDrawIndirectCount2Info(commandBuffer, *pInfo, info_loc, context);

    const VkStridedDeviceAddressRangeKHR& address_range = pInfo->addressRange;
    if (pInfo->maxDrawCount > 0) {
        const VkDeviceSize required_size = (pInfo->maxDrawCount - 1) * address_range.stride + sizeof(VkDrawIndexedIndirectCommand);
        if (address_range.size < required_size) {
            skip |=
                LogError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13110", commandBuffer, info_loc.dot(Field::maxDrawCount),
                         "is %" PRIu32 " (non-zero), but addressRange.size (%" PRIu64 ") is less than the required size of %" PRIu64
                         ".\nRequired size: (maxDrawCount - 1) * stride + sizeof(VkDrawIndexedIndirectCommand) -> (%" PRIu32
                         " - 1) * %" PRIu64 " + %zu",
                         pInfo->maxDrawCount, address_range.size, required_size, pInfo->maxDrawCount, address_range.stride,
                         sizeof(VkDrawIndexedIndirectCommand));
        }
    }

    if (address_range.stride == 0) {
        if (pInfo->maxDrawCount > 1) {
            skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13111", commandBuffer,
                             info_loc.dot(Field::addressRange).dot(Field::maxDrawCount),
                             "is %" PRIu32 ", but addressRange.stride is 0.", pInfo->maxDrawCount);
        }
    } else if (address_range.stride < sizeof(VkDrawIndexedIndirectCommand)) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13112", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::stride),
                         "(%" PRIu64 ") must be at least sizeof(VkDrawIndexedIndirectCommand) (%zu).", pInfo->addressRange.stride,
                         sizeof(VkDrawIndexedIndirectCommand));
    } else if (!IsIntegerMultipleOf(address_range.stride, 4u)) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13113", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::stride), "(%" PRIu64 ") must be a multiple of 4.",
                         pInfo->addressRange.stride);
    }

    return skip;
}
bool Device::manual_PreCallValidateCmdDrawMeshTasksIndirectCount2EXT(VkCommandBuffer commandBuffer,
                                                                     const VkDrawIndirectCount2InfoKHR *pInfo,
                                                                     const Context &context) const {
    bool skip = false;

    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |=
            LogError("VUID-vkCmdDrawMeshTasksIndirectCount2EXT-drawIndirectCount-13069", commandBuffer, context.error_obj.location,
                     "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                     "call this command.");
    }

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    skip |= ValidateDrawIndirectCount2Info(commandBuffer, *pInfo, info_loc, context);

    return skip;
}

bool Device::manual_PreCallValidateCmdDispatchIndirect2KHR(VkCommandBuffer commandBuffer, const VkDispatchIndirect2InfoKHR *pInfo,
                                                           const Context &context) const {
    bool skip = false;

    const Location info_loc = context.error_obj.location.dot(Field::pInfo);
    if (pInfo->addressRange.size < sizeof(VkDispatchIndirectCommand)) {
        skip |= LogError("VUID-vkCmdDispatchIndirect2KHR-pInfo-13050", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::size),
                         "(%" PRIu64 ") is less than sizeof(VkDispatchIndirectCommand) (%zu).", pInfo->addressRange.size,
                         sizeof(VkDispatchIndirectCommand));
    }

    if (!IsPointerAligned(pInfo->addressRange.address, 4u)) {
        skip |= LogError("VUID-VkDispatchIndirect2InfoKHR-addressRange-13109", commandBuffer,
                         info_loc.dot(Field::addressRange).dot(Field::address), "(0x%" PRIu64 ") must be aligned to 4 bytes.",
                         pInfo->addressRange.address);
    }

    skip |= context.ValidateDeviceAddressFlags(info_loc.dot(Field::addressFlags), pInfo->addressFlags);

    return skip;
}

}  // namespace stateless