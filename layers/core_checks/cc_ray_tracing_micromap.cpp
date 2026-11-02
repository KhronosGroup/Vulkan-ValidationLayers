/* Copyright (c) 2026 The Khronos Group Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "cc_buffer_address.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/query_state.h"

bool CoreChecks::PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    auto buffer_state = Get<vvl::Buffer>(pCreateInfo->buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const Location buffer_loc = create_info_loc.dot(Field::buffer);
    if (!(buffer_state->usage & VK_BUFFER_USAGE_2_MICROMAP_STORAGE_BIT_EXT)) {
        skip |= LogError("VUID-VkMicromapCreateInfoEXT-buffer-07435", buffer_state->Handle(), buffer_loc, "was created with %s.",
                         string_VkBufferUsageFlags2(buffer_state->usage).c_str());
    }
    if (buffer_state->GetFlags() & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) {
        skip |= LogError("VUID-VkMicromapCreateInfoEXT-buffer-07436", buffer_state->Handle(), buffer_loc, "was created with %s.",
                         string_VkBufferCreateFlags(buffer_state->GetFlags()).c_str());
    }
    if (pCreateInfo->offset + pCreateInfo->size > buffer_state->GetSize()) {
        skip |= LogError("VUID-VkMicromapCreateInfoEXT-offset-07437", buffer_state->Handle(), buffer_loc.dot(Field::offset),
                         "(%" PRIu64 ") + size (%" PRIu64 ") must not be greater than the size of the buffer (%" PRIu64 ").",
                         pCreateInfo->offset, pCreateInfo->size, buffer_state->GetSize());
    }

    if (pCreateInfo->createFlags & VK_MICROMAP_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT) {
        if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
            !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
            skip |= LogError("VUID-VkMicromapCreateInfoEXT-bufferDeviceAddressMultiDevice-11617", device, error_obj.location,
                             "device was created with multiple physical devices (%" PRIu32
                             "), but the bufferDeviceAddressMultiDevice feature was not enabled.",
                             device_state->physical_device_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                         const VkMicromapBuildInfoEXT* pBuildInfo,
                                                         VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetMicromapBuildSizesEXT-device-07440", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                const VkCopyMicromapInfoEXT* pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMicromapEXT-deferredOperation-03678");
    return skip;
}

bool CoreChecks::PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMicromapToMemoryEXT-deferredOperation-03678");
    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMemoryToMicromapEXT-deferredOperation-03678");
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                     const VkMicromapBuildInfoEXT* pInfos, const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    const LogObjectList objlist(commandBuffer);
    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        BufferAddressValidation<1> input_address_validator = {{{
            {"VUID-vkCmdBuildMicromapsEXT-data-07510",
             [](const vvl::Buffer& buffer_state) {
                 return (buffer_state.usage & VK_BUFFER_USAGE_2_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT) == 0;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT"; },
             ErrorMsgBuffer::Usage},
        }}};
        skip |= input_address_validator.ValidateDeviceAddress(*this, info_loc.dot(Field::data).dot(Field::deviceAddress), objlist,
                                                              info.data.deviceAddress);
        skip |= input_address_validator.ValidateDeviceAddress(*this, info_loc.dot(Field::triangleArray).dot(Field::deviceAddress),
                                                              objlist, info.triangleArray.deviceAddress);

        BufferAddressValidation<1> scratch_address_validator = {{{
            {"VUID-vkCmdBuildMicromapsEXT-pInfos-07511",
             [](const vvl::Buffer& buffer_state) { return (buffer_state.usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0; },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT"; }, ErrorMsgBuffer::Usage},
        }}};
        skip |= scratch_address_validator.ValidateDeviceAddress(*this, info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                                                                objlist, info.scratchData.deviceAddress);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                               const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                               VkQueryPool queryPool, uint32_t firstQuery,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    auto query_pool_state = Get<vvl::QueryPool>(queryPool);
    ASSERT_AND_RETURN_SKIP(query_pool_state);
    const auto& query_pool_ci = query_pool_state->create_info;
    if (query_pool_ci.queryType != queryType) {
        const LogObjectList objlist(commandBuffer, queryPool);
        skip |= LogError("VUID-vkCmdWriteMicromapsPropertiesEXT-queryPool-07525", objlist, error_obj.location.dot(Field::queryType),
                         "is %s, but queryPool was created with %s.", string_VkQueryType(queryType),
                         string_VkQueryType(query_pool_ci.queryType));
    }
    if (firstQuery + micromapCount > query_pool_ci.queryCount) {
        const LogObjectList objlist(commandBuffer, queryPool);
        skip |= LogError("VUID-vkCmdWriteMicromapsPropertiesEXT-query-07528", objlist, error_obj.location.dot(Field::firstQuery),
                         "(%" PRIu32 ") + micromapCount (%" PRIu32 ") is greater than the number of queries in queryPool (%" PRIu32
                         ").",
                         firstQuery, micromapCount, query_pool_ci.queryCount);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                           const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                           const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                           const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                           const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}
