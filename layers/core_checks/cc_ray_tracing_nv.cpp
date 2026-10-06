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
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10550",
               [](const vvl::Buffer& buffer_state) { return (buffer_state.usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0; },
               []() { return "The following buffers are missing VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT"; }, ErrorMsgBuffer::Usage}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, error_obj.location.dot(Field::pBuildInfo).dot(Field::scratchData), LogObjectList(commandBuffer),
            pBuildInfo->scratchData, build_size_info.buildScratchSize,
            "VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10541");
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
               ErrorMsgBuffer::Usage}}}};

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
               ErrorMsgBuffer::Usage}}}};

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
               ErrorMsgBuffer::Usage}}}};

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
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10552",
               [](const vvl::Buffer& buffer_state) {
                   return (buffer_state.usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) == 0;
               },
               []() { return "The following buffers are missing VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR"; },
               ErrorMsgBuffer::Usage}}}};

        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, error_obj.location.dot(Field::pBuildInfo).dot(Field::dstAccelerationStructureData), LogObjectList(commandBuffer),
            pBuildInfo->dstAccelerationStructureData, build_size_info.accelerationStructureSize,
            "VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10543");
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
                    skip |= LogError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10549", objlist,
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
                         ") must sum to less than or equal to "
                         "maxPartitionCount (%" PRIu32 ").",
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
        BufferAddressValidation<0> buffer_address_validator;
        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, build_info_loc.dot(Field::scratchData), LogObjectList(device), build_info.scratchData, build_scratch_size,
            "VUID-VkBuildPartitionedAccelerationStructureInfoNV-scratchData-10559");
    }
    if (build_info.dstAccelerationStructureData != 0) {
        BufferAddressValidation<0> buffer_address_validator;
        skip |= buffer_address_validator.ValidateDeviceAddress(
            *this, build_info_loc.dot(Field::dstAccelerationStructureData), LogObjectList(device),
            build_info.dstAccelerationStructureData, build_acceleration_structure_size,
            "VUID-VkBuildPartitionedAccelerationStructureInfoNV-dstAccelerationStructureData-10562");
    }

    if (!IsPointerAligned(build_info.srcInfosCount, 4)) {
        skip |= LogError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-srcInfosCount-10563", device,
                         build_info_loc.dot(Field::srcInfosCount), "(0x%" PRIx64 ") must be aligned to 4 bytes",
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
                 return buffer_state.GetSize() < accelerationStructure_size.buildScratchSize;
             },
             [&accelerationStructure_size]() {
                 return "The buildScratchSize (" + std::to_string(accelerationStructure_size.buildScratchSize) +
                        ") does not fit in any buffer";
             },
             ErrorMsgBuffer::Empty},
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12304",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT"; }, ErrorMsgBuffer::Usage},
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
             ErrorMsgBuffer::Usage},
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
             ErrorMsgBuffer::Usage},
        }}};

        skip |= src_infos_count_validator.ValidateDeviceAddress(*this, command_infos_loc.dot(Field::srcInfosCount), objlist,
                                                                pCommandInfos->srcInfosCount);
    }

    {
        BufferAddressValidation<1> dst_addresses_array_validator = {{{
            {"VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-12381",
             [](const vvl::Buffer& buffer_state) {
                 return (static_cast<uint32_t>(buffer_state.usage) & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT"; }, ErrorMsgBuffer::Usage},
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
             ErrorMsgBuffer::Usage},
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
                BufferAddressValidation<0> dst_implicit_size_validator;
                skip |= dst_implicit_size_validator.ValidateDeviceAddress(
                    *this, command_infos_loc.dot(Field::dstImplicitData), objlist, command_infos.dstImplicitData,
                    accelerationStructure_size.accelerationStructureSize,
                    "VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-12310");
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
                         "(%" PRIu64 ") must be greater than or equal to the size of %s (%" PRIu32 ")",
                         command_infos.srcInfosArray.stride,
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
