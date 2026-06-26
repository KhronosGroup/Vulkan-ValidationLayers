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

#include <algorithm>
#include <assert.h>
#include <string>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#include "containers/limits.h"
#include "containers/range.h"
#include "core_validation.h"
#include "cc_vuid_maps.h"
#include "core_checks/cc_state_tracker.h"
#include "cc_buffer_address.h"
#include "error_message/error_location.h"
#include "error_message/logging.h"
#include "utils/ray_tracing_utils.h"
#include "utils/math_utils.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/pipeline_state.h"
#include "error_message/error_strings.h"
#include "containers/container_utils.h"

bool CoreChecks::PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkAccelerationStructureKHR* pAccelerationStructure,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    auto buffer_state = Get<vvl::Buffer>(pCreateInfo->buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);

    if (!(buffer_state->usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614", buffer_state->Handle(),
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::buffer), "was created with %s.",
                         string_VkBufferUsageFlags2(buffer_state->usage).c_str());
    }
    if (buffer_state->GetFlags() & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-buffer-03615", buffer_state->Handle(),
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::buffer), "was created with %s.",
                         string_VkBufferCreateFlags(buffer_state->GetFlags()).c_str());
    }
    if (pCreateInfo->offset + pCreateInfo->size > buffer_state->GetSize()) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-offset-03616", buffer_state->Handle(),
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::offset),
                         "(%" PRIu64 ") + size (%" PRIu64 ") must be less than the size of buffer (%" PRIu64 ").",
                         pCreateInfo->offset, pCreateInfo->size, buffer_state->GetSize());
    }

    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkCreateAccelerationStructureKHR-device-03489", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateAccelerationStructure2KHR(VkDevice device,
                                                                const VkAccelerationStructureCreateInfo2KHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkAccelerationStructureKHR* pAccelerationStructure,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    const bool has_capture_replay =
        pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    const char* capture_replay_vuid = has_capture_replay ? "VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-11607"
                                                         : "VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-11606";
    std::string capture_replay_msg = std::string("The following buffers were created with") +
                                     std::string(has_capture_replay ? "out" : "") +
                                     std::string(" VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT");

    BufferAddressValidation<2> buffer_address_validator = {{{
        {"VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11604",
         [](const vvl::Buffer& buffer_state) { return (buffer_state.GetFlags() & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) != 0; },
         []() { return "The following buffers were created with VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT"; }, kFlagErrorMsgBuffer},
        {capture_replay_vuid,
         [has_capture_replay](const vvl::Buffer& buffer_state) {
             return (has_capture_replay && (buffer_state.GetFlags() & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) == 0) ||
                    (!has_capture_replay && ((buffer_state.GetFlags() & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) != 0));
         },
         [&capture_replay_msg]() { return capture_replay_msg; }, kFlagErrorMsgBuffer},
    }}};
    skip |= buffer_address_validator.ValidateDeviceAddress(*this, create_info_loc.dot(Field::addressRange).dot(Field::address),
                                                           device, pCreateInfo->addressRange.address);

    skip |= ValidateDeviceAddressRange(pCreateInfo->addressRange.address, pCreateInfo->addressRange.size, false,
                                       create_info_loc.dot(Field::addressRange), LogObjectList(device),
                                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                                       "VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11603");

    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkCreateAccelerationStructure2KHR-device-03489", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }

    skip |= ValidateDeviceAddressCommands(device, pCreateInfo->addressRange.address, pCreateInfo->addressRange.size,
                                          pCreateInfo->addressFlags, create_info_loc.dot(Field::addressRange));

    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetAccelerationStructureBuildSizesKHR-device-03618", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }
    return skip;
}

bool CoreChecks::ValidateAccelerationStructuresMemoryAlisasing(const LogObjectList& objlist, uint32_t infoCount,
                                                               const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                               uint32_t info_i, const ErrorObject& error_obj) const {
    using vvl::range;

    bool skip = false;
    const VkAccelerationStructureBuildGeometryInfoKHR& info = pInfos[info_i];
    const Location info_i_loc = error_obj.location.dot(Field::pInfos, info_i);
    const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
    const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);

    const bool info_in_mode_update = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;

    if (info_in_mode_update && info.srcAccelerationStructure != info.dstAccelerationStructure && src_as_state && dst_as_state) {
        const char* vuid = error_obj.location.function == Func::vkCmdBuildAccelerationStructuresKHR
                               ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03668"
                           : error_obj.location.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                               ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03668"
                               : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03668";
        skip |= ValidateAccelStructsMemoryDoNotOverlap(error_obj.location, objlist, *src_as_state,
                                                       info_i_loc.dot(Field::srcAccelerationStructure), *dst_as_state,
                                                       info_i_loc.dot(Field::dstAccelerationStructure), vuid);
    }

    // Loop on other acceleration structure builds info.
    // Given that comparisons are commutative, only need to consider elements after info_i
    assert(infoCount > info_i);
    for (auto [other_info_j, other_info] : vvl::enumerate(pInfos + info_i + 1, infoCount - (info_i + 1))) {
        // Validate that scratch buffer's memory does not overlap destination acceleration structure's memory, or source
        // acceleration structure's memory if build mode is update, or other scratch buffers' memory.
        // Here validation is pessimistic: if one buffer associated to pInfos[other_info_j].scratchData.deviceAddress has an
        // overlap, an error will be logged.

        const Location other_info_j_loc = error_obj.location.dot(Field::pInfos, other_info_j);

        const auto other_dst_as_state = Get<vvl::AccelerationStructureKHR>(other_info.dstAccelerationStructure);
        const auto other_src_as_state = Get<vvl::AccelerationStructureKHR>(other_info.srcAccelerationStructure);

        // Validate destination acceleration structure's memory is not overlapped by another source acceleration structure's
        // memory that is going to be updated by this cmd
        if (dst_as_state && other_src_as_state) {
            if (other_info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                const char* vuid = error_obj.location.function == Func::vkCmdBuildAccelerationStructuresKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701"
                                   : error_obj.location.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03701"
                                       : "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03701";

                skip |= ValidateAccelStructsMemoryDoNotOverlap(error_obj.location, objlist, *dst_as_state,
                                                               info_i_loc.dot(Field::dstAccelerationStructure), *other_src_as_state,
                                                               other_info_j_loc.dot(Field::srcAccelerationStructure), vuid);
            }
        }

        // Validate that there is no destination acceleration structures' memory overlaps
        if (dst_as_state && other_dst_as_state) {
            const char* vuid = error_obj.location.function == Func::vkCmdBuildAccelerationStructuresKHR
                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702"
                               : error_obj.location.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03702"
                                   : "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03702";

            skip |= ValidateAccelStructsMemoryDoNotOverlap(error_obj.location, objlist, *dst_as_state,
                                                           info_i_loc.dot(Field::dstAccelerationStructure), *other_dst_as_state,
                                                           other_info_j_loc.dot(Field::dstAccelerationStructure), vuid);
        }
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructuresDeviceScratchBufferMemoryAliasing(
    VkCommandBuffer cmd_buffer, uint32_t info_count, const VkAccelerationStructureBuildGeometryInfoKHR* p_infos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* pp_range_infos, const Location& loc) const {
    assert(loc.function != Func::vkBuildAccelerationStructuresKHR);

    bool skip = false;

    enum class AddressRangeOrigin : uint8_t { Undefined, SrcAccelStruct, DstAccelStruct, Scratch };
    struct AddressRange {
        vvl::range<VkDeviceAddress> range = {};
        uint32_t info_i = 0;
        AddressRangeOrigin origin = AddressRangeOrigin::Undefined;
    };
    // Gather all address ranges  from source acceleration structtures, destination acceleration structures
    // and scratch buffers in a single sorted vector, to more rapidly lookup overlaps
    std::vector<AddressRange> address_ranges;
    address_ranges.reserve(3 * info_count);

    const auto insert_address = [](std::vector<AddressRange>& address_ranges, const AddressRange address_range) {
        std::optional<AddressRange> overlapped_range = std::nullopt;
        auto insert_pos =
            std::lower_bound(address_ranges.begin(), address_ranges.end(), address_range,
                             [&overlapped_range](const AddressRange iter, const AddressRange new_elt) -> bool {
                                 const vvl::range<VkDeviceAddress> intersection = iter.range & new_elt.range;
                                 if (intersection.non_empty()) {
                                     const bool both_src_as_ranges = iter.origin == AddressRangeOrigin::SrcAccelStruct &&
                                                                     new_elt.origin == AddressRangeOrigin::SrcAccelStruct;
                                     const bool as_update =
                                         iter.info_i == new_elt.info_i && ((iter.origin == AddressRangeOrigin::SrcAccelStruct &&
                                                                            new_elt.origin == AddressRangeOrigin::DstAccelStruct) ||
                                                                           (new_elt.origin == AddressRangeOrigin::SrcAccelStruct &&
                                                                            iter.origin == AddressRangeOrigin::DstAccelStruct));

                                     if (!both_src_as_ranges && !as_update) {
                                         overlapped_range = iter;
                                     }
                                 }

                                 return iter.range < new_elt.range;
                             });

        address_ranges.insert(insert_pos, address_range);
        return overlapped_range;
    };

    for (const auto [info_i, info] : vvl::enumerate(p_infos, info_count)) {
        const Location info_i_loc = loc.dot(Field::pInfos, info_i);

        const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);

        if (const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
            src_as_state && info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
            if (const vvl::range<VkDeviceAddress> src_as_range = src_as_state->GetVvlEffectiveDeviceAddressRange();
                !src_as_range.empty()) {
                if (dst_as_state && dst_as_state->VkHandle() != src_as_state->VkHandle()) {
                    const vvl::range<VkDeviceAddress> dst_as_range = dst_as_state->GetVvlEffectiveDeviceAddressRange();

                    if (const vvl::range<VkDeviceAddress> dst_as_src_as_intersection = dst_as_range & src_as_range;
                        dst_as_src_as_intersection.non_empty()) {
                        skip |=
                            LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03668",
                                     LogObjectList(cmd_buffer, src_as_state->VkHandle(), dst_as_state->VkHandle()),
                                     info_i_loc.dot(Field::dstAccelerationStructure),
                                     "overlaps with srcAccelerationStructure, which is in update mode, on device address range %s.",
                                     string_range_hex(dst_as_src_as_intersection).c_str());
                    }
                }

                const AddressRange src_as_address_range = {src_as_range, info_i, AddressRangeOrigin::SrcAccelStruct};
                const std::optional<AddressRange> overlapped_address_range = insert_address(address_ranges, src_as_address_range);
                if (overlapped_address_range.has_value()) {
                    switch (overlapped_address_range->origin) {
                        case AddressRangeOrigin::SrcAccelStruct: {
                            // Valid overlap, source acceleration structures being read only
                            break;
                        }
                        case AddressRangeOrigin::DstAccelStruct: {
                            if (const auto other_dst_as_state = Get<vvl::AccelerationStructureKHR>(
                                    p_infos[overlapped_address_range->info_i].dstAccelerationStructure)) {
                                const vvl::range<VkDeviceAddress> src_as_other_dst_as_intersection =
                                    src_as_address_range.range & overlapped_address_range->range;

                                skip |= LogError(
                                    "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701",
                                    LogObjectList(cmd_buffer, src_as_state->VkHandle(), other_dst_as_state->VkHandle()),
                                    info_i_loc.dot(Field::srcAccelerationStructure),
                                    "is in update mode, but overlaps with dstAccelerationStructure of pInfos[%" PRIu32
                                    "] on device address range %s.",
                                    overlapped_address_range->info_i, string_range_hex(src_as_other_dst_as_intersection).c_str());
                            }
                            break;
                        }
                        case AddressRangeOrigin::Scratch: {
                            const vvl::range<VkDeviceAddress> src_as_other_scratch_intersection =
                                src_as_address_range.range & overlapped_address_range->range;
                            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705",
                                             LogObjectList(cmd_buffer, src_as_state->VkHandle()),
                                             info_i_loc.dot(Field::srcAccelerationStructure),
                                             "overlaps with scratch buffer of pInfos[%" PRIu32 "] on device address range %s.",
                                             overlapped_address_range->info_i,
                                             string_range_hex(src_as_other_scratch_intersection).c_str());
                            break;
                        }
                        case AddressRangeOrigin::Undefined:
                            assert(false);
                            break;
                    }
                }
            }
        }

        if (dst_as_state) {
            if (const vvl::range<VkDeviceAddress> dst_as_range = dst_as_state->GetVvlEffectiveDeviceAddressRange();
                !dst_as_range.empty()) {
                const AddressRange dst_as_address_range = {dst_as_range, info_i, AddressRangeOrigin::DstAccelStruct};
                const std::optional<AddressRange> overlapped_address_range = insert_address(address_ranges, dst_as_address_range);
                if (overlapped_address_range.has_value()) {
                    switch (overlapped_address_range->origin) {
                        case AddressRangeOrigin::SrcAccelStruct: {
                            if (const auto other_src_as_state = Get<vvl::AccelerationStructureKHR>(
                                    p_infos[overlapped_address_range->info_i].srcAccelerationStructure)) {
                                const vvl::range<VkDeviceAddress> dst_as_other_src_as_intersection =
                                    dst_as_address_range.range & overlapped_address_range->range;

                                skip |= LogError(
                                    "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701",
                                    LogObjectList(cmd_buffer, dst_as_state->VkHandle(), other_src_as_state->VkHandle()),
                                    info_i_loc.dot(Field::dstAccelerationStructure),
                                    "overlaps with srcAccelerationStructure of pInfos[%" PRIu32
                                    "], which is in update mode, on device address range %s.",
                                    overlapped_address_range->info_i, string_range_hex(dst_as_other_src_as_intersection).c_str());
                            }
                            break;
                        }
                        case AddressRangeOrigin::DstAccelStruct: {
                            if (const auto other_dst_as_state = Get<vvl::AccelerationStructureKHR>(
                                    p_infos[overlapped_address_range->info_i].dstAccelerationStructure)) {
                                const vvl::range<VkDeviceAddress> dst_as_other_dst_as_intersection =
                                    dst_as_address_range.range & overlapped_address_range->range;

                                skip |= LogError(
                                    "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702",
                                    LogObjectList(cmd_buffer, dst_as_state->VkHandle(), other_dst_as_state->VkHandle()),
                                    info_i_loc.dot(Field::dstAccelerationStructure),
                                    "overlaps with dstAccelerationStructure of pInfos[%" PRIu32 "], on device address range %s.",
                                    overlapped_address_range->info_i, string_range_hex(dst_as_other_dst_as_intersection).c_str());
                            }
                            break;
                        }
                        case AddressRangeOrigin::Scratch: {
                            const vvl::range<VkDeviceAddress> src_as_other_scratch_intersection =
                                dst_as_address_range.range & overlapped_address_range->range;
                            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703",
                                             LogObjectList(cmd_buffer, dst_as_state->VkHandle()),
                                             info_i_loc.dot(Field::dstAccelerationStructure),
                                             "overlaps with scratch buffer of pInfos[%" PRIu32 "] on device address range %s.",
                                             overlapped_address_range->info_i,
                                             string_range_hex(src_as_other_scratch_intersection).c_str());
                            break;
                        }
                        case AddressRangeOrigin::Undefined:
                            assert(false);
                            break;
                    }
                }
            }
        }

        // Do not attempt looking for memory overlaps here if any buffer associated to scratch address is sparse
        const auto scratch_buffers = GetBuffersByAddress(info.scratchData.deviceAddress);
        const bool any_scratch_sparse = std::any_of(scratch_buffers.begin(), scratch_buffers.end(),
                                                    [](const vvl::Buffer* buffer) { return buffer && buffer->sparse; });
        if (!any_scratch_sparse) {
            const VkDeviceSize assumed_scratch_size =
                rt::ComputeScratchSize(rt::BuildType::Device, device, info, pp_range_infos[info_i]);

            const vvl::range<VkDeviceAddress> scratch_range = {info.scratchData.deviceAddress,
                                                               info.scratchData.deviceAddress + assumed_scratch_size};

            const AddressRange scratch_address_range = {scratch_range, info_i, AddressRangeOrigin::Scratch};
            const std::optional<AddressRange> overlapped_address_range = insert_address(address_ranges, scratch_address_range);
            if (overlapped_address_range.has_value()) {
                switch (overlapped_address_range->origin) {
                    case AddressRangeOrigin::SrcAccelStruct: {
                        if (const auto other_src_as_state = Get<vvl::AccelerationStructureKHR>(
                                p_infos[overlapped_address_range->info_i].srcAccelerationStructure)) {
                            const vvl::range<VkDeviceAddress> scratch_other_src_as_intersection =
                                scratch_address_range.range & overlapped_address_range->range;

                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705",
                                LogObjectList(cmd_buffer, other_src_as_state->VkHandle()), info_i_loc.dot(Field::scratchData),
                                "overlaps with srcAccelerationStructure of pInfos[%" PRIu32
                                "], which is in update mode, on device address range %s.",
                                overlapped_address_range->info_i, string_range_hex(scratch_other_src_as_intersection).c_str());
                        }
                        break;
                    }
                    case AddressRangeOrigin::DstAccelStruct: {
                        if (const auto other_dst_as_state = Get<vvl::AccelerationStructureKHR>(
                                p_infos[overlapped_address_range->info_i].dstAccelerationStructure)) {
                            const vvl::range<VkDeviceAddress> dst_as_other_dst_as_intersection =
                                scratch_address_range.range & overlapped_address_range->range;

                            skip |= LogError(
                                "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703",
                                LogObjectList(cmd_buffer, other_dst_as_state->VkHandle()), info_i_loc.dot(Field::scratchData),
                                "overlaps with dstAccelerationStructure of pInfos[%" PRIu32 "], on device address range %s.",
                                overlapped_address_range->info_i, string_range_hex(dst_as_other_dst_as_intersection).c_str());
                        }
                        break;
                    }
                    case AddressRangeOrigin::Scratch: {
                        const vvl::range<VkDeviceAddress> src_as_other_scratch_intersection =
                            scratch_address_range.range & overlapped_address_range->range;
                        skip |=
                            LogError("VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704", LogObjectList(cmd_buffer),
                                     info_i_loc.dot(Field::scratchData),
                                     "overlaps with scratch buffer of pInfos[%" PRIu32 "] on device address range %s.",
                                     overlapped_address_range->info_i, string_range_hex(src_as_other_scratch_intersection).c_str());
                        break;
                    }
                    case AddressRangeOrigin::Undefined:
                        assert(false);
                        break;
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                         const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkGetAccelerationStructureDeviceAddressKHR-accelerationStructure-08935", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetAccelerationStructureDeviceAddressKHR-device-03504", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }

    if (const auto as = Get<vvl::AccelerationStructureKHR>(pInfo->accelerationStructure)) {
        const Location info_loc = error_obj.location.dot(Field::pInfo);
        const auto as_buffer = as->GetFirstValidBuffer(*device_state);
        if (!as_buffer) {
            return skip;
        }
        skip |=
            ValidateMemoryIsBoundToBuffer(device, *as_buffer.state, info_loc.dot(Field::accelerationStructure).dot(Field::buffer),
                                          "VUID-vkGetAccelerationStructureDeviceAddressKHR-pInfo-09541");

        if (!(as_buffer.state->usage & VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT)) {
            skip |= LogError("VUID-vkGetAccelerationStructureDeviceAddressKHR-pInfo-09542", LogObjectList(device),
                             info_loc.dot(Field::accelerationStructure).dot(Field::buffer), "was created with usage flag(s) %s.",
                             string_VkBufferUsageFlags2(as_buffer.state->usage).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationVertex(VkFormat vertex_format, VkDeviceOrHostAddressConstKHR vertex_data,
                                            VkDeviceSize vertex_stride, const LogObjectList& objlist, const Location& loc) const {
    bool skip = false;

    uint32_t format_alignment = 0;
    const bool is_packed = vkuFormatIsPacked(vertex_format);
    if (is_packed) {
        format_alignment = vkuFormatTexelBlockSize(vertex_format);
    } else {
        uint32_t min_component_bits_size = vvl::kU32Max;
        const VKU_FORMAT_INFO format_info = vkuGetFormatInfo(vertex_format);
        for (uint32_t component_i = 0; component_i < format_info.component_count; ++component_i) {
            min_component_bits_size = std::min(format_info.components[component_i].size, min_component_bits_size);
        }
        format_alignment = min_component_bits_size / 8;
    }

    if (!IsPointerAligned(vertex_data.deviceAddress, format_alignment)) {
        const char* vuid = loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                               ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03711"
                               : "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03711";
        skip |= LogError(vuid, objlist, loc.dot(Field::vertexData).dot(Field::deviceAddress),
                         "(0x%" PRIx64 ") is not aligned to the %s (%" PRIu32 ") of its corresponding vertexFormat (%s).",
                         vertex_data.deviceAddress, is_packed ? "texel block size" : "minimum component byte size",
                         format_alignment, string_VkFormat(vertex_format));
    }
    if (!IsIntegerMultipleOf(vertex_stride, format_alignment)) {
        const char* vuid = loc.structure == Struct::VkAccelerationStructureGeometrySpheresDataNV
                               ? "VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10431"
                           : loc.structure == Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV
                               ? "VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10421"
                               : "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03735";
        skip |= LogError(vuid, objlist, loc.dot(Field::vertexStride),
                         "(%" PRIu64 ") is not a multiple to the %s (%" PRIu32 ") of its corresponding vertexFormat (%s).",
                         vertex_stride, is_packed ? "texel block size" : "minimum component byte size", format_alignment,
                         string_VkFormat(vertex_format));
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructureBuildGeometryInfoDevice(
    VkCommandBuffer cmd_buffer, uint32_t info_i, const VkAccelerationStructureBuildGeometryInfoKHR& info,
    const VkAccelerationStructureBuildRangeInfoKHR* geometry_build_ranges, const Location& info_loc) const {
    bool skip = false;

    const auto pick_vuid = [&info_loc](const char* direct_build_vu, const char* indirect_build_vu) -> const char* {
        return info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR ? direct_build_vu : indirect_build_vu;
    };

    const bool is_indirect = info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR;

    const LogObjectList cb_objlist(cmd_buffer);
    auto buffer_check = [this, &pick_vuid, &cb_objlist](const VkDeviceOrHostAddressConstKHR address, const Location& loc) -> bool {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673",
                         "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-geometry-03673"),
               [](const vvl::Buffer& buffer_state) {
                   return (buffer_state.usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
               },
               []() {
                   return "The following buffers are missing "
                          "VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
               },
               kUsageErrorMsgBuffer}}}};

        return buffer_address_validator.ValidateDeviceAddress(*this, loc.dot(Field::deviceAddress), cb_objlist,
                                                              address.deviceAddress);
    };

    for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
        const Location p_geom_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
        const Location p_geom_geom_loc = p_geom_loc.dot(Field::geometry);
        const VkAccelerationStructureGeometryKHR& geom = rt::GetGeometry(info, geom_i);
        const uint32_t geometry_build_range_primitive_count =
            geometry_build_ranges ? geometry_build_ranges[geom_i].primitiveCount : 0;

        if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
            const Location p_geom_geom_triangles_loc = p_geom_geom_loc.dot(Field::triangles);
            const auto& triangles = geom.geometry.triangles;

            if (geometry_build_range_primitive_count > 0) {
                if (triangles.vertexData.deviceAddress == 0) {
                    skip |=
                        LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03804",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03804"),
                                 cmd_buffer, p_geom_geom_triangles_loc.dot(Field::vertexData).dot(Field::deviceAddress), "is zero");
                }
                skip |= buffer_check(triangles.vertexData, p_geom_geom_triangles_loc.dot(Field::vertexData));
            }

            if (triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                if (geometry_build_range_primitive_count > 0) {
                    if (triangles.indexData.deviceAddress == 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03806",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03806"),
                                         cmd_buffer, p_geom_geom_triangles_loc.dot(Field::indexData).dot(Field::deviceAddress),
                                         "is zero");
                    }

                    skip |= buffer_check(triangles.indexData, p_geom_geom_triangles_loc.dot(Field::indexData));
                }

                if (info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR &&
                    info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                    const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
                    if (src_as_state && src_as_state->GetBuildInfo().has_value()) {
                        if (geom_i < src_as_state->GetBuildRangeInfos().size()) {
                            if (const uint32_t recorded_primitive_count = src_as_state->GetBuildRangeInfos()[geom_i].primitiveCount;
                                recorded_primitive_count != geometry_build_range_primitive_count) {
                                const Location pp_build_range_info_loc(info_loc.function, Field::ppBuildRangeInfos, info_i);
                                const LogObjectList objlist(cmd_buffer, info.srcAccelerationStructure);
                                skip |= LogError(
                                    "VUID-vkCmdBuildAccelerationStructuresKHR-primitiveCount-03769", objlist, p_geom_loc,
                                    " has corresponding VkAccelerationStructureBuildRangeInfoKHR %s, but this build range has its "
                                    "primitiveCount member set to (%" PRIu32 ") when it was last specified as (%" PRIu32 ").",
                                    pp_build_range_info_loc.brackets(geom_i).Fields().c_str(), geometry_build_range_primitive_count,
                                    recorded_primitive_count);
                            }
                        }
                    }
                }
            }

            const VkFormatProperties3 format_properties = GetPDFormatProperties(triangles.vertexFormat);
            if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-03797", cmd_buffer,
                                 p_geom_geom_triangles_loc.dot(Field::vertexFormat),
                                 "is %s which doesn't support VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR.\n"
                                 "(supported bufferFeatures: %s)",
                                 string_VkFormat(triangles.vertexFormat),
                                 string_VkFormatFeatureFlags2(format_properties.bufferFeatures).c_str());
            } else {
                // Only try to get format info if vertex format is valid
                skip |= ValidateAccelerationVertex(triangles.vertexFormat, triangles.vertexData, triangles.vertexStride, cb_objlist,
                                                   p_geom_geom_triangles_loc);
            }

            if (triangles.transformData.deviceAddress != 0 && geometry_build_range_primitive_count > 0) {
                skip |= buffer_check(triangles.transformData, p_geom_geom_triangles_loc.dot(Field::transformData));
            }

            if (const auto* micromap_ext =
                    vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapEXT>(triangles.pNext)) {
                if (micromap_ext->indexType == VK_INDEX_TYPE_NONE_KHR) {
                    if (micromap_ext->indexBuffer.deviceAddress != 0) {
                        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-10904", device,
                                         p_geom_geom_triangles_loc
                                             .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapEXT, Field::indexBuffer)
                                             .dot(Field::deviceAddress),
                                         "is 0x%" PRIx64 " (non-zero) but indexType is VK_INDEX_TYPE_NONE_KHR.",
                                         micromap_ext->indexBuffer.deviceAddress);
                    }
                } else {
                    skip |= ValidateDeviceAddress(
                        p_geom_geom_triangles_loc
                            .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapEXT, Field::indexBuffer)
                            .dot(Field::deviceAddress),
                        cb_objlist, micromap_ext->indexBuffer.deviceAddress);
                }
            }

            if (const auto* motion_triangles =
                    vku::FindStructInPNextChain<VkAccelerationStructureGeometryMotionTrianglesDataNV>(triangles.pNext)) {
                const Location& motion_triangles_loc = p_geom_geom_triangles_loc.pNext(
                    Struct::VkAccelerationStructureGeometryMotionTrianglesDataNV, Field::vertexData);
                if (motion_triangles->vertexData.deviceAddress == 0) {
                    // There is no indirect build version supported
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11845", cmd_buffer,
                                     motion_triangles_loc.dot(Field::deviceAddress), "is zero");
                }
                skip |= ValidateDeviceAddress(motion_triangles_loc.dot(Field::deviceAddress), cb_objlist,
                                              motion_triangles->vertexData.deviceAddress);
            }

            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
                if (src_as_state && src_as_state->GetBuildInfo().has_value() &&
                    info.geometryCount == src_as_state->GetBuildInfo()->geometryCount) {
                    const auto& src_build_info = src_as_state->GetBuildInfo();
                    const bool src_was_deserialized = src_as_state->WasDeserialized();
                    const bool src_has_micromap_update_bit =
                        src_build_info->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_BIT_KHR;
                    const bool has_micromap_data_update_bit =
                        src_build_info->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_DATA_UPDATE_BIT_EXT;

                    const VkAccelerationStructureGeometryKHR& src_geom = rt::GetGeometry(*src_build_info->ptr(), geom_i);
                    const auto* src_triangles_micromap =
                        vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapKHR>(
                            src_geom.geometry.triangles.pNext);
                    const auto* dst_triangle_micromap =
                        vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapKHR>(
                            geom.geometry.triangles.pNext);
                    if (dst_triangle_micromap && !src_triangles_micromap) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11627",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-micromap-11627"),
                                         LogObjectList(cmd_buffer, info.srcAccelerationStructure),
                                         p_geom_geom_loc.dot(Field::triangles).dot(Field::pNext),
                                         "contains VkAccelerationStructureTrianglesOpacityMicromapKHR, but the "
                                         "VkAccelerationStructureTrianglesOpacityMicromapKHR "
                                         "is not present in the corresponding built geometry of srcAccelerationStructure (%s)",
                                         FormatHandle(info.srcAccelerationStructure).c_str());
                    } else if (dst_triangle_micromap && src_triangles_micromap) {
                        if ((dst_triangle_micromap->micromap != VK_NULL_HANDLE) &&
                            (src_triangles_micromap->micromap == VK_NULL_HANDLE)) {
                            skip |=
                                LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11626",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-micromap-11626"),
                                         LogObjectList(cmd_buffer, info.srcAccelerationStructure),
                                         p_geom_geom_loc.dot(Field::triangles)
                                             .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::micromap),
                                         "is not NULL (%s), but the micromap member in the corresponding built geometry of "
                                         "srcAccelerationStructure "
                                         "(%s) is NULL.",
                                         FormatHandle(dst_triangle_micromap->micromap).c_str(),
                                         FormatHandle(info.srcAccelerationStructure).c_str());
                        } else if ((dst_triangle_micromap->micromap == VK_NULL_HANDLE) &&
                                   (src_triangles_micromap->micromap != VK_NULL_HANDLE)) {
                            skip |=
                                LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11625",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-micromap-11625"),
                                         LogObjectList(cmd_buffer, info.srcAccelerationStructure, src_triangles_micromap->micromap),
                                         p_geom_geom_loc.dot(Field::triangles)
                                             .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::micromap),
                                         "is NULL, but the micromap member in the corresponding built geometry of "
                                         "srcAccelerationStructure is %s.",
                                         FormatHandle(src_triangles_micromap->micromap).c_str());
                        }

                        if (src_triangles_micromap->micromap != VK_NULL_HANDLE) {
                            if (!src_was_deserialized && !src_has_micromap_update_bit && !has_micromap_data_update_bit) {
                                if (!dst_triangle_micromap || dst_triangle_micromap->micromap != src_triangles_micromap->micromap) {
                                    skip |= LogError(
                                        pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11629",
                                                  "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-micromap-11629"),
                                        LogObjectList(cmd_buffer, info.srcAccelerationStructure, src_triangles_micromap->micromap),
                                        p_geom_geom_loc.dot(Field::triangles)
                                            .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::micromap),
                                        "is %s, but srcAccelerationStructure (%s) was originally built without "
                                        "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_BIT_KHR or "
                                        "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_DATA_UPDATE_BIT_EXT, "
                                        "and the original build used micromap %s.",
                                        dst_triangle_micromap ? FormatHandle(dst_triangle_micromap->micromap).c_str()
                                                              : "VK_NULL_HANDLE",
                                        FormatHandle(info.srcAccelerationStructure).c_str(),
                                        FormatHandle(src_triangles_micromap->micromap).c_str());
                                }
                            }
                        }
                    }
                    if (src_triangles_micromap && src_triangles_micromap->micromap != VK_NULL_HANDLE) {
                        if (src_was_deserialized && !src_has_micromap_update_bit) {
                            if (!dst_triangle_micromap || dst_triangle_micromap->micromap == VK_NULL_HANDLE) {
                                skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11628",
                                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-micromap-11628"),
                                                 LogObjectList(cmd_buffer, info.srcAccelerationStructure),
                                                 p_geom_geom_loc.dot(Field::triangles).dot(Field::pNext),
                                                 "srcAccelerationStructure (%s) was deserialized without "
                                                 "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_BIT_KHR, "
                                                 "but the geometry does not contain a "
                                                 "VkAccelerationStructureTrianglesOpacityMicromapKHR "
                                                 "with a micromap deserialized from the original build.",
                                                 FormatHandle(info.srcAccelerationStructure).c_str());
                            }
                        }
                    }
                }
            }

            if (const auto* micromap_khr =
                    vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapKHR>(triangles.pNext)) {
                skip |= PreCallValidateAccelerationStructureTrianglesOpacityMicromapKHR(*micromap_khr, cb_objlist,
                                                                                        p_geom_geom_triangles_loc);

                if (micromap_khr->indexType != VK_INDEX_TYPE_NONE_KHR) {
                    const uint64_t index_buffer_size =
                        uint64_t(micromap_khr->indexStride) * uint64_t(geometry_build_range_primitive_count);
                    const vvl::range<VkDeviceAddress> index_buffer_range(micromap_khr->indexBuffer,
                                                                         micromap_khr->indexBuffer + index_buffer_size);

                    BufferAddressValidation<1> index_buffer_range_validator = {{{
                        {"VUID-vkCmdBuildAccelerationStructuresKHR-indexBuffer-11577",
                         [index_buffer_range](const vvl::Buffer& buffer_state) {
                             return !buffer_state.DeviceAddressRange().includes(index_buffer_range);
                         },
                         [index_buffer_size]() {
                             return "The opacity micromap indexBuffer range of size " + std::to_string(index_buffer_size) +
                                    " does not fit in any buffer.";
                         },
                         kEmptyErrorMsgBuffer},
                    }}};

                    skip |= index_buffer_range_validator.ValidateDeviceAddress(
                        *this,
                        p_geom_geom_triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR,
                                                        Field::indexBuffer),
                        cb_objlist, micromap_khr->indexBuffer, index_buffer_size);
                }

                if (auto micromap = Get<vvl::AccelerationStructureKHR>(micromap_khr->micromap)) {
                    if (!is_indirect) {
                        if (const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
                            const LogObjectList objlist(device, cmd_buffer, info.dstAccelerationStructure);
                            skip |= ValidateAccelStructsMemoryDoNotOverlap(
                                info_loc, objlist, *dst_as_state, info_loc.dot(Field::dstAccelerationStructure), *micromap,
                                p_geom_loc.dot(Field::triangles).dot(Field::pNext),
                                "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-11549");

                            const vvl::range<VkDeviceAddress> dst_as_range = dst_as_state->GetVvlEffectiveDeviceAddressRange();
                            const vvl::range<VkDeviceAddress> micromap_as_range = micromap->GetVvlEffectiveDeviceAddressRange();
                            if (dst_as_range.intersects(micromap_as_range)) {
                                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-11548",
                                                 LogObjectList(cmd_buffer, micromap->Handle(), info.dstAccelerationStructure),
                                                 p_geom_geom_triangles_loc.pNext(
                                                     Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::micromap),
                                                 "(%s) range (%s) overlaps with dstAccelerationStructure (%s) range (%s).",
                                                 FormatHandle(micromap->Handle()).c_str(),
                                                 vvl::string_range_hex(micromap_as_range).c_str(),
                                                 FormatHandle(info.dstAccelerationStructure).c_str(),
                                                 vvl::string_range_hex(dst_as_range).c_str());
                            }
                        }
                    }

                    if (micromap->GetBuildInfo().has_value() &&
                        micromap->GetBuildInfo()->type != VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11624",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-micromap-11624"),
                                         cmd_buffer,
                                         p_geom_geom_triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR,
                                                                         Field::micromap),
                                         "(%s) was built with type %s, but must have been built with "
                                         "VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR.",
                                         FormatHandle(micromap_khr->micromap).c_str(),
                                         string_VkAccelerationStructureTypeKHR(micromap->GetBuildInfo()->type));
                    } else if (micromap->GetBuildInfo().has_value() && micromap_khr->indexType == VK_INDEX_TYPE_NONE_KHR &&
                               !is_indirect && geometry_build_range_primitive_count > 0) {
                        const uint64_t micromap_total_triangles =
                            rt::MicromapUsageTotalTriangleCount(*micromap->GetBuildInfo()->ptr());
                        if ((uint64_t(geometry_build_range_primitive_count - 1) + uint64_t(micromap_khr->baseTriangle)) >
                            micromap_total_triangles) {
                            skip |=
                                LogError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-geometry-11576", cb_objlist,
                                         p_geom_geom_triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR,
                                                                         Field::baseTriangle),
                                         "indexType is VK_INDEX_TYPE_NONE_KHR, numTriangles is %" PRIu32
                                         ", and index plus baseTriangle for the last triangle (%" PRIu64
                                         ") is greater than the total triangles in micromap (%" PRIu64 ").",
                                         geometry_build_range_primitive_count,
                                         uint64_t(geometry_build_range_primitive_count - 1) + uint64_t(micromap_khr->baseTriangle),
                                         micromap_total_triangles);
                        }
                    }
                }
            }
        } else if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
            if (geometry_build_range_primitive_count > 0) {
                const Location instances_loc = p_geom_geom_loc.dot(Field::instances);
                const Location instances_data_loc = instances_loc.dot(Field::data);
                const auto& instances = geom.geometry.instances;
                if (instances.data.deviceAddress == 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03813",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03813"),
                                     cmd_buffer, instances_data_loc.dot(Field::deviceAddress), "is zero");
                }

                skip |= buffer_check(instances.data, instances_data_loc);
            }
        } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
            if (geometry_build_range_primitive_count > 0) {
                const Location aabbs_loc = p_geom_geom_loc.dot(Field::aabbs);
                const Location aabbs_data_loc = aabbs_loc.dot(Field::data);
                const auto& aabbs = geom.geometry.aabbs;

                if (aabbs.data.deviceAddress == 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03811",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03811"),
                                     cmd_buffer, aabbs_data_loc.dot(Field::deviceAddress), "is zero");
                }

                skip |= buffer_check(aabbs.data, aabbs_data_loc);
            }
        } else if (geom.geometryType == VK_GEOMETRY_TYPE_SPHERES_NV) {
            const Location p_geom_geom_spheres_loc = p_geom_geom_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV);
            auto sphere_struct = reinterpret_cast<VkAccelerationStructureGeometrySpheresDataNV const*>(geom.pNext);
            ASSERT_AND_RETURN_SKIP(sphere_struct);

            if (geometry_build_range_primitive_count > 0) {
                if (sphere_struct->indexType == VK_INDEX_TYPE_NONE_KHR) {
                    if (sphere_struct->indexData.deviceAddress != 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11846",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11846"),
                                         cmd_buffer, p_geom_geom_spheres_loc.dot(Field::indexData).dot(Field::deviceAddress),
                                         "(0x%" PRIx64 ") is not 0 when indexType is VK_INDEX_TYPE_NONE_KHR.",
                                         sphere_struct->indexData.deviceAddress);
                    }
                } else {
                    if (sphere_struct->indexData.deviceAddress == 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11847",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11847"),
                                         cmd_buffer, p_geom_geom_spheres_loc.dot(Field::indexData).dot(Field::deviceAddress),
                                         "is zero");
                    }
                    skip |= buffer_check(sphere_struct->indexData, p_geom_geom_spheres_loc.dot(Field::indexData));
                }
                if (sphere_struct->vertexData.deviceAddress == 0) {
                    skip |=
                        LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11848",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11848"),
                                 cmd_buffer, p_geom_geom_spheres_loc.dot(Field::vertexData).dot(Field::deviceAddress), "is zero");
                }
                skip |= buffer_check(sphere_struct->vertexData, p_geom_geom_spheres_loc.dot(Field::vertexData));

                if (sphere_struct->radiusData.deviceAddress == 0) {
                    skip |=
                        LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11849",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11849"),
                                 cmd_buffer, p_geom_geom_spheres_loc.dot(Field::radiusData).dot(Field::deviceAddress), "is zero");
                }
                skip |= buffer_check(sphere_struct->radiusData, p_geom_geom_spheres_loc.dot(Field::radiusData));
            }

            const VkFormatProperties3KHR vertex_properties = GetPDFormatProperties(sphere_struct->vertexFormat);
            if (!(vertex_properties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
                skip |= LogError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexFormat-10434", cmd_buffer,
                                 p_geom_geom_spheres_loc.dot(Field::vertexFormat),
                                 "is %s which doesn't support VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR.\n"
                                 "(supported bufferFeatures: %s)",
                                 string_VkFormat(sphere_struct->vertexFormat),
                                 string_VkFormatFeatureFlags2(vertex_properties.bufferFeatures).c_str());
            } else {
                // Only try to get format info if vertex format is valid
                skip |= ValidateAccelerationVertex(sphere_struct->vertexFormat, sphere_struct->vertexData,
                                                   sphere_struct->vertexStride, cb_objlist, p_geom_geom_spheres_loc);
            }

            const VkFormatProperties3KHR radius_properties = GetPDFormatProperties(sphere_struct->radiusFormat);
            if (!(radius_properties.bufferFeatures & VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV)) {
                skip |= LogError("VUID-VkAccelerationStructureGeometrySpheresDataNV-radiusFormat-10435", cmd_buffer,
                                 p_geom_geom_spheres_loc.dot(Field::radiusFormat),
                                 "is %s which doesn't support VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV.\n"
                                 "(supported bufferFeatures: %s)",
                                 string_VkFormat(sphere_struct->radiusFormat),
                                 string_VkFormatFeatureFlags2(radius_properties.bufferFeatures).c_str());
            }
        } else if (geom.geometryType == VK_GEOMETRY_TYPE_LINEAR_SWEPT_SPHERES_NV) {
            const Location p_geom_geom_linear_spheres_loc =
                p_geom_geom_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV);
            auto sphere_linear_struct =
                reinterpret_cast<VkAccelerationStructureGeometryLinearSweptSpheresDataNV const*>(geom.pNext);
            ASSERT_AND_RETURN_SKIP(sphere_linear_struct);

            if (geometry_build_range_primitive_count > 0) {
                if (sphere_linear_struct->indexType == VK_INDEX_TYPE_NONE_KHR) {
                    if (sphere_linear_struct->indexData.deviceAddress != 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11850",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11850"),
                                         cmd_buffer, p_geom_geom_linear_spheres_loc.dot(Field::indexData).dot(Field::deviceAddress),
                                         "(0x%" PRIx64 ") is not 0 when indexType is VK_INDEX_TYPE_NONE_KHR.",
                                         sphere_linear_struct->indexData.deviceAddress);
                    }
                } else {
                    if (sphere_linear_struct->indexData.deviceAddress == 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11851",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11851"),
                                         cmd_buffer, p_geom_geom_linear_spheres_loc.dot(Field::indexData).dot(Field::deviceAddress),
                                         "is zero");
                    }
                    skip |= buffer_check(sphere_linear_struct->indexData, p_geom_geom_linear_spheres_loc.dot(Field::indexData));
                }
                if (sphere_linear_struct->vertexData.deviceAddress == 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11852",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11852"),
                                     cmd_buffer, p_geom_geom_linear_spheres_loc.dot(Field::vertexData).dot(Field::deviceAddress),
                                     "is zero");
                }
                skip |= buffer_check(sphere_linear_struct->vertexData, p_geom_geom_linear_spheres_loc.dot(Field::vertexData));

                if (sphere_linear_struct->radiusData.deviceAddress == 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-11853",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-11853"),
                                     cmd_buffer, p_geom_geom_linear_spheres_loc.dot(Field::radiusData).dot(Field::deviceAddress),
                                     "is zero");
                }
                skip |= buffer_check(sphere_linear_struct->radiusData, p_geom_geom_linear_spheres_loc.dot(Field::radiusData));
            }
            const VkFormatProperties3KHR vertex_properties = GetPDFormatProperties(sphere_linear_struct->vertexFormat);
            if (!(vertex_properties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexFormat-10423", cmd_buffer,
                                 p_geom_geom_linear_spheres_loc.dot(Field::vertexFormat),
                                 "is %s which doesn't support VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR.\n"
                                 "(supported bufferFeatures: %s)",
                                 string_VkFormat(sphere_linear_struct->vertexFormat),
                                 string_VkFormatFeatureFlags2(vertex_properties.bufferFeatures).c_str());
            } else {
                // Only try to get format info if vertex format is valid
                skip |= ValidateAccelerationVertex(sphere_linear_struct->vertexFormat, sphere_linear_struct->vertexData,
                                                   sphere_linear_struct->vertexStride, cb_objlist, p_geom_geom_linear_spheres_loc);
            }

            const VkFormatProperties3KHR radius_properties = GetPDFormatProperties(sphere_linear_struct->radiusFormat);
            if (!(radius_properties.bufferFeatures & VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV)) {
                skip |= LogError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-radiusFormat-10424", cmd_buffer,
                                 p_geom_geom_linear_spheres_loc.dot(Field::radiusFormat),
                                 "is %s which doesn't support VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV.\n"
                                 "(supported bufferFeatures: %s)",
                                 string_VkFormat(sphere_linear_struct->radiusFormat),
                                 string_VkFormatFeatureFlags2(radius_properties.bufferFeatures).c_str());
            }

            if (sphere_linear_struct->indexingMode == VK_RAY_TRACING_LSS_INDEXING_MODE_SUCCESSIVE_NV) {
                if (!sphere_linear_struct->indexData.deviceAddress && !sphere_linear_struct->indexData.hostAddress) {
                    skip |= LogError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-indexingMode-10427", cmd_buffer,
                                     p_geom_geom_linear_spheres_loc.dot(Field::indexData),
                                     "shouldn't be NULL if indexing mode is VK_RAY_TRACING_LSS_INDEXING_MODE_SUCCESSIVE_NV.");
                }
            }
        }

        if (!is_indirect) {
            if (const auto* micromap_geometry_data =
                    vku::FindStructInPNextChain<VkAccelerationStructureGeometryMicromapDataKHR>(geom.pNext)) {
                BufferAddressValidation<1> data_address = {{{
                    {"VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11551",
                     [](const vvl::Buffer& buffer_state) {
                         return (buffer_state.usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
                     },
                     []() {
                         return "The following buffers are missing "
                                "VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
                     },
                     kUsageErrorMsgBuffer},
                }}};

                skip |= data_address.ValidateDeviceAddress(*this, p_geom_loc.dot(Field::pNext).dot(Field::data), cb_objlist,
                                                           micromap_geometry_data->data);
                BufferAddressValidation<1> triangle_array_address = {{{
                    {"VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11551",
                     [](const vvl::Buffer& buffer_state) {
                         return (buffer_state.usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) == 0;
                     },
                     []() {
                         return "The following buffers are missing "
                                "VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR";
                     },
                     kUsageErrorMsgBuffer},
                }}};

                skip |= triangle_array_address.ValidateDeviceAddress(*this, p_geom_loc.dot(Field::pNext).dot(Field::triangleArray),
                                                                     cb_objlist, micromap_geometry_data->triangleArray);

                const bool has_usage_counts = (micromap_geometry_data->pUsageCounts || micromap_geometry_data->ppUsageCounts) &&
                                              micromap_geometry_data->usageCountsCount > 0;
                if (has_usage_counts) {
                    uint64_t total_triangle_count = 0;
                    for (uint32_t usage_i = 0; usage_i < micromap_geometry_data->usageCountsCount; ++usage_i) {
                        total_triangle_count += rt::GetMicroMapUsage(*micromap_geometry_data, usage_i).count;
                    }

                    if (total_triangle_count > 0) {
                        const uint64_t triangle_array_size = micromap_geometry_data->triangleArrayStride * total_triangle_count;
                        const vvl::range<VkDeviceAddress> triangle_array_range(
                            micromap_geometry_data->triangleArray, micromap_geometry_data->triangleArray + triangle_array_size);

                        BufferAddressValidation<1> triangle_array_range_validator = {{{
                            {"VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11554",
                             [triangle_array_range](const vvl::Buffer& buffer_state) {
                                 return !buffer_state.DeviceAddressRange().includes(triangle_array_range);
                             },
                             [micromap_geometry_data, triangle_array_size, total_triangle_count]() {
                                 std::stringstream triangles_arrays_addr_str;
                                 triangles_arrays_addr_str << std::hex << "0x" << micromap_geometry_data->triangleArray;
                                 return "The triangleArray range [triangleArray (" + triangles_arrays_addr_str.str() +
                                        "), triangleArray + (triangleArrayStride (" +
                                        std::to_string(micromap_geometry_data->triangleArrayStride) + ") * numTriangles (" +
                                        std::to_string(total_triangle_count) + ")) = " + std::to_string(triangle_array_size) +
                                        "] does not fit in any buffer.";
                             },
                             kEmptyErrorMsgBuffer},
                        }}};

                        skip |= triangle_array_range_validator.ValidateDeviceAddress(
                            *this, p_geom_loc.dot(Field::pNext).dot(Field::triangleArray), cb_objlist,
                            micromap_geometry_data->triangleArray, triangle_array_size);
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructureBuildScratch(VkCommandBuffer cmd_buffer,
                                                           const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                                           const VkAccelerationStructureBuildRangeInfoKHR* geometry_build_ranges,
                                                           const Location& info_loc) const {
    bool skip = false;

    const VkDeviceSize scratch_size = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                          ? rt::ComputeScratchSize(rt::BuildType::Device, device, info, geometry_build_ranges)
                                          : 1;
    if (scratch_size == 0) {
        return skip;
    }

    const auto pick_vuid = [&info_loc](const char* direct_build_vu, const char* indirect_build_vu) -> const char* {
        return info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR ? direct_build_vu : indirect_build_vu;
    };

    if (info.scratchData.deviceAddress == 0) {
        const char* scratch_address_range_vuid = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
                                                     ? pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12261",
                                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-12261")
                                                     : pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12260",
                                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-12260");
        skip |= LogError(scratch_address_range_vuid, device, info_loc.dot(Field::scratchData).dot(Field::deviceAddress), "is zero");
    } else {
        // Hardcoded value of 1 for indirect calls because scratch size cannot be computed on the CPU in this case
        // (need to access build ranges). Easier to hardcode than to add the logic to not perform scratch buffer size
        // validation for indirect calls.

        const vvl::range<VkDeviceSize> scratch_address_range(info.scratchData.deviceAddress,
                                                             info.scratchData.deviceAddress + scratch_size);
        const char* scratch_address_range_vuid = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
                                                     ? pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12258",
                                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-12258")
                                                     : pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12259",
                                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-12259");

        const char* scratch_buffer_has_storage_flag_vuid =
            info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
                ? pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12261",
                            "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-12261")
                : pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-12260",
                            "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-12260");

        BufferAddressValidation<2> buffer_address_validator = {
            {{{scratch_buffer_has_storage_flag_vuid,
               [](const vvl::Buffer& buffer_state) { return (buffer_state.usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) == 0; },
               []() { return "The following buffers are missing VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT"; }, kUsageErrorMsgBuffer},

              {scratch_address_range_vuid,
               [scratch_address_range](const vvl::Buffer& buffer_state) {
                   const vvl::range<VkDeviceSize> buffer_address_range = buffer_state.DeviceAddressRange();
                   return !buffer_address_range.includes(scratch_address_range);
               },
               [scratch_size]() { return "The scratch size (" + std::to_string(scratch_size) + ") does not fit in any buffer"; },
               kEmptyErrorMsgBuffer}}}};

        skip |=
            buffer_address_validator.ValidateDeviceAddress(*this, info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                                                           LogObjectList(cmd_buffer), info.scratchData.deviceAddress, scratch_size);
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructureBuildGeometryInfoUpdate(const vvl::AccelerationStructureKHR& src_as_state,
                                                                      const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                                                      const Location& info_loc,
                                                                      const VulkanTypedHandle& handle) const {
    bool skip = false;

    if (!src_as_state.GetBuildInfo().has_value()) {
        return skip;
    }
    if (!(src_as_state.GetBuildInfo()->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR)) {
        const LogObjectList objlist(handle, info.srcAccelerationStructure);
        skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::IsBuilt_03667), objlist, info_loc.dot(Field::mode),
                         "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but srcAccelerationStructure has been previously "
                         "constructed with flags %s.",
                         string_VkBuildAccelerationStructureFlagsKHR(src_as_state.GetBuildInfo()->flags).c_str());
    }

    if (info.flags != src_as_state.GetBuildInfo()->flags) {
        const LogObjectList objlist(handle, info.srcAccelerationStructure);
        skip |=
            LogError(GetBuildASVUID(info_loc, vvl::BuildASError::SameFlags_03759), objlist, info_loc.dot(Field::mode),
                     "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but %s (%s) must have the same value as "
                     "specified when srcAccelerationStructure was last built (%s).",
                     info_loc.dot(Field::flags).Fields().c_str(), string_VkBuildAccelerationStructureFlagsKHR(info.flags).c_str(),
                     string_VkBuildAccelerationStructureFlagsKHR(src_as_state.GetBuildInfo()->flags).c_str());
    }

    if (info.type != src_as_state.GetBuildInfo()->type) {
        const LogObjectList objlist(handle, info.srcAccelerationStructure);
        skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::SameType_03760), objlist, info_loc.dot(Field::mode),
                         "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but type (%s) must have the same value as "
                         "specified when srcAccelerationStructure was last built (%s).",
                         string_VkAccelerationStructureTypeKHR(info.type),
                         string_VkAccelerationStructureTypeKHR(src_as_state.GetBuildInfo()->type));
    }

    if (info.geometryCount != src_as_state.GetBuildInfo()->geometryCount) {
        const LogObjectList objlist(handle, info.srcAccelerationStructure);
        skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::SameCount_03758), objlist, info_loc.dot(Field::mode),
                         "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,"
                         " but geometryCount (%" PRIu32
                         ") must have the same value as specified when "
                         "srcAccelerationStructure was last built (%" PRIu32 ").",
                         info.geometryCount, src_as_state.GetBuildInfo()->geometryCount);
    } else if (info.pGeometries || info.ppGeometries) {
        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& updated_geometry = rt::GetGeometry(info, geom_i);
            const VkAccelerationStructureGeometryKHR& last_geometry =
                rt::GetGeometry(*src_as_state.GetBuildInfo().value().ptr(), geom_i);

            const Location geometry_ptr_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);

            if (updated_geometry.geometryType != last_geometry.geometryType) {
                const LogObjectList objlist(handle, info.srcAccelerationStructure);
                skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::SameType_03761), objlist,
                                 geometry_ptr_loc.dot(Field::geometryType), "is %s but was last specified as %s.",
                                 string_VkGeometryTypeKHR(updated_geometry.geometryType),
                                 string_VkGeometryTypeKHR(last_geometry.geometryType));
            }

            if (updated_geometry.flags != last_geometry.flags) {
                const LogObjectList objlist(handle, info.srcAccelerationStructure);
                skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::SameFlags_03762), objlist,
                                 geometry_ptr_loc.dot(Field::flags), "is %s but was last specified as %s.",
                                 string_VkGeometryFlagsKHR(updated_geometry.flags).c_str(),
                                 string_VkGeometryFlagsKHR(last_geometry.flags).c_str());
            }

            if (updated_geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                if (updated_geometry.geometry.triangles.vertexFormat != last_geometry.geometry.triangles.vertexFormat) {
                    const LogObjectList objlist(handle, info.srcAccelerationStructure);
                    skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::TriangleVertexFormat_03763), objlist,
                                     geometry_ptr_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::vertexFormat),
                                     "is %s but was last specified as %s.",
                                     string_VkFormat(updated_geometry.geometry.triangles.vertexFormat),
                                     string_VkFormat(last_geometry.geometry.triangles.vertexFormat));
                }

                if (updated_geometry.geometry.triangles.maxVertex != last_geometry.geometry.triangles.maxVertex) {
                    const LogObjectList objlist(handle, info.srcAccelerationStructure);
                    skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::TriangleMaxVertex_03764), objlist,
                                     geometry_ptr_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::maxVertex),
                                     "is %" PRIu32 " but was last specified as %" PRIu32 ".",
                                     updated_geometry.geometry.triangles.maxVertex, last_geometry.geometry.triangles.maxVertex);
                }

                if (updated_geometry.geometry.triangles.indexType != last_geometry.geometry.triangles.indexType) {
                    const LogObjectList objlist(handle, info.srcAccelerationStructure);
                    skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::TriangleIndexType_03765), objlist,
                                     geometry_ptr_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::indexType),
                                     "is %s but was last specified as %s.",
                                     string_VkIndexType(updated_geometry.geometry.triangles.indexType),
                                     string_VkIndexType(last_geometry.geometry.triangles.indexType));
                }

                if (last_geometry.geometry.triangles.transformData.deviceAddress == 0 &&
                    updated_geometry.geometry.triangles.transformData.deviceAddress != 0) {
                    const LogObjectList objlist(handle, info.srcAccelerationStructure);
                    skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::TriangleTransformData_03766), objlist,
                                     geometry_ptr_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::transformData),
                                     "is 0x%" PRIx64 " but was last specified as NULL.",
                                     updated_geometry.geometry.triangles.transformData.deviceAddress);
                }

                if (last_geometry.geometry.triangles.transformData.deviceAddress != 0 &&
                    updated_geometry.geometry.triangles.transformData.deviceAddress == 0) {
                    const LogObjectList objlist(handle, info.srcAccelerationStructure);
                    skip |= LogError(GetBuildASVUID(info_loc, vvl::BuildASError::TriangleTransformData_03767), objlist,
                                     geometry_ptr_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::transformData),
                                     "is NULL but was last specified as 0x%" PRIx64 ".",
                                     last_geometry.geometry.triangles.transformData.deviceAddress);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructureBuildDst(const vvl::AccelerationStructureKHR& dst_as_state,
                                                       const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                                       const Location& info_loc, const VulkanTypedHandle& handle) const {
    bool skip = false;

    const VkAccelerationStructureTypeKHR dst_as_type = dst_as_state.GetType();
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
        if (dst_as_type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
            dst_as_type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
            skip |= LogError(
                GetBuildASVUID(info_loc, vvl::BuildASError::DstBottom_03700), handle, info_loc.dot(Field::type),
                "is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, but its dstAccelerationStructure was created with %s.",
                string_VkAccelerationStructureTypeKHR(dst_as_type));
        }
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
        if (dst_as_type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
            dst_as_type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
            skip |=
                LogError(GetBuildASVUID(info_loc, vvl::BuildASError::DstTop_03699), handle, info_loc.dot(Field::type),
                         "is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but its dstAccelerationStructure was created with %s.",
                         string_VkAccelerationStructureTypeKHR(dst_as_type));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const ErrorObject& error_obj) const {
    using vvl::range;
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!pInfos || !ppBuildRangeInfos) {
        return skip;
    }

    if (!cb_state->unprotected) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-09547", commandBuffer, error_obj.location,
                         "command can't be used in protected command buffers.");
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        if (const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure)) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && src_as_state->UsesCreateInfo1()) {
                const auto src_as_buffer = src_as_state->GetFirstValidBuffer(*device_state);
                if (!src_as_buffer) {
                    const LogObjectList objlist(commandBuffer, info.srcAccelerationStructure);
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03708", objlist, info_loc.dot(Field::mode),
                                     "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR but the buffer associated with "
                                     "srcAccelerationStructure is not valid.");
                } else {
                    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_as_buffer.state,
                                                          info_loc.dot(Field::srcAccelerationStructure),
                                                          "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03708");
                }

                skip |= ValidateAccelerationStructureBuildGeometryInfoUpdate(*src_as_state, info, info_loc, error_obj.handle);
            }
        }

        if (const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            const auto dst_as_buffer = dst_as_state->GetFirstValidBuffer(*device_state);
            if (!dst_as_buffer || dst_as_buffer.state->Destroyed()) {
                const LogObjectList objlist(commandBuffer, info.srcAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure), "is backed by an invalid or destroyed buffer.");
            } else {
                skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_as_buffer.state,
                                                      info_loc.dot(Field::srcAccelerationStructure),
                                                      "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
            }

            skip |= ValidateAccelerationStructureBuildDst(*dst_as_state, info, info_loc, error_obj.handle);

            if (!(info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
                  (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR))) {
                const VkDeviceSize as_minimum_size =
                    rt::ComputeAccelerationStructureSize(rt::BuildType::Device, device, info, ppBuildRangeInfos[info_i]);
                if (dst_as_state->GetSize() < as_minimum_size) {
                    const LogObjectList objlist(commandBuffer, info.dstAccelerationStructure);
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-10126", objlist,
                                     info_loc.dot(Field::dstAccelerationStructure),
                                     " was created with size (%" PRIu64
                                     "), but an acceleration structure build with corresponding ppBuildRangeInfos[%" PRIu32
                                     "] requires a minimum size of (%" PRIu64 ").",
                                     dst_as_state->GetSize(), info_i, as_minimum_size);
                }
            }
            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR) {
                auto dst_as_type = dst_as_state->GetType();
                if (dst_as_type != VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR &&
                    dst_as_type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    const LogObjectList objlist(commandBuffer, info.dstAccelerationStructure);
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-11547", objlist,
                                     info_loc.dot(Field::type),
                                     "is VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR, but its dstAccelerationStructure was "
                                     "created with %s.",
                                     string_VkAccelerationStructureTypeKHR(dst_as_state->GetType()));
                }
            }
        }

        skip |=
            ValidateAccelerationStructureBuildGeometryInfoDevice(commandBuffer, info_i, info, ppBuildRangeInfos[info_i], info_loc);
        skip |= ValidateAccelerationStructureBuildScratch(commandBuffer, info, ppBuildRangeInfos[info_i], info_loc);
    }

    skip |= ValidateAccelerationStructuresDeviceScratchBufferMemoryAliasing(commandBuffer, infoCount, pInfos, ppBuildRangeInfos,
                                                                            error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateAccelerationStructureTrianglesOpacityMicromapKHR(
    const VkAccelerationStructureTrianglesOpacityMicromapKHR& pInfo, const LogObjectList& objlist,
    const Location& triangles_loc) const {
    bool skip = false;

    if (!IsValueIn(pInfo.indexType, {VK_INDEX_TYPE_UINT8, VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, VK_INDEX_TYPE_NONE_KHR})) {
        skip |= LogError(
            "VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexType-11570", objlist,
            triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::indexType),
            "is %s, but must be VK_INDEX_TYPE_UINT8, VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR.",
            string_VkIndexType(pInfo.indexType));
    }

    if (pInfo.indexType == VK_INDEX_TYPE_NONE_KHR) {
        if (pInfo.indexBuffer != 0) {
            skip |= LogError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexBuffer-11571", objlist,
                             triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::indexBuffer),
                             "is 0x%" PRIx64 ", but must be 0 when indexType is VK_INDEX_TYPE_NONE_KHR.", pInfo.indexBuffer);
        }
        if (pInfo.micromap == VK_NULL_HANDLE) {
            skip |= LogError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-micromap-11579", objlist,
                             triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::micromap),
                             "is VK_NULL_HANDLE, but must not be VK_NULL_HANDLE when indexType is VK_INDEX_TYPE_NONE_KHR.");
        }
    }

    if (pInfo.indexType != VK_INDEX_TYPE_NONE_KHR) {
        const uint32_t index_type_size = IndexTypeByteSize(pInfo.indexType);
        if (!IsIntegerMultipleOf(pInfo.indexStride, index_type_size)) {
            skip |= LogError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexStride-11573", objlist,
                             triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::indexStride),
                             "is %" PRIu64 ", but must be a multiple of the size of indexType (%s).", pInfo.indexStride,
                             string_VkIndexType(pInfo.indexType));
        }
        if (pInfo.indexBuffer == 0) {
            skip |= LogError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexBuffer-11572", objlist,
                             triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::indexBuffer),
                             "is 0, but must be a valid VkDeviceAddress when indexType (%s) is not VK_INDEX_TYPE_NONE_KHR.",
                             string_VkIndexType(pInfo.indexType));
        } else {
            skip |= ValidateDeviceAddress(
                triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::indexBuffer), objlist,
                pInfo.indexBuffer);
        }
        if (pInfo.indexStride > vvl::kU32Max) {
            skip |= LogError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexStride-11574", objlist,
                             triangles_loc.pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::indexStride),
                             "is %" PRIu64 ", but must be less than or equal to 2^32 - 1 (%" PRIu32
                             ") when indexType is not VK_INDEX_TYPE_NONE_KHR.",
                             pInfo.indexStride, vvl::kU32Max);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkBuildAccelerationStructuresKHR-deferredOperation-03678");

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);
        auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
        auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);

        if (src_as_state) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                skip |= ValidateAccelStructBufferMemoryIsHostVisible(*src_as_state, info_loc.dot(Field::srcAccelerationStructure),
                                                                     "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
                skip |=
                    ValidateAccelStructBufferMemoryIsNotMultiInstance(*src_as_state, info_loc.dot(Field::srcAccelerationStructure),
                                                                      "VUID-vkBuildAccelerationStructuresKHR-pInfos-03776");
                skip |= ValidateAccelerationStructureBuildGeometryInfoUpdate(*src_as_state, info, info_loc, error_obj.handle);

                if (src_as_state->UsesCreateInfo1()) {
                    const auto src_as_buffer = src_as_state->GetFirstValidBuffer(*device_state);
                    if (!src_as_buffer) {
                        const LogObjectList objlist(device, info.srcAccelerationStructure);
                        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03708", objlist, info_loc.dot(Field::mode),
                                         "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR but the buffer associated with "
                                         "srcAccelerationStructure is not valid.");
                    } else {
                        skip |= ValidateMemoryIsBoundToBuffer(device, *src_as_buffer.state,
                                                              info_loc.dot(Field::srcAccelerationStructure),
                                                              "VUID-vkBuildAccelerationStructuresKHR-pInfos-03708");
                    }
                }
            }
        }

        if (dst_as_state) {
            const auto dst_as_buffer = dst_as_state->GetFirstValidBuffer(*device_state);
            if (!dst_as_buffer || dst_as_buffer.state->Destroyed()) {
                const LogObjectList objlist(device, info.dstAccelerationStructure);
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03707", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure), "is backed by an invalid or destroyed buffer.");
            } else {
                skip |= ValidateMemoryIsBoundToBuffer(device, *dst_as_buffer.state, info_loc.dot(Field::dstAccelerationStructure),
                                                      "VUID-vkBuildAccelerationStructuresKHR-pInfos-03707");
            }

            if (dst_as_state->UsesCreateInfo2()) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11703", info.dstAccelerationStructure,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "was created with vkCreateAccelerationStructure2KHR, but needs to be created with "
                                 "vkCreateAccelerationStructureKHR.");
            }
            skip |= ValidateAccelStructBufferMemoryIsHostVisible(*dst_as_state, info_loc.dot(Field::dstAccelerationStructure),
                                                                 "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");

            skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*dst_as_state, info_loc.dot(Field::dstAccelerationStructure),
                                                                      "VUID-vkBuildAccelerationStructuresKHR-pInfos-03775");

            skip |= ValidateAccelerationStructureBuildDst(*dst_as_state, info, info_loc, error_obj.handle);

            if (!(info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
                  (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR))) {
                const VkDeviceSize as_minimum_size =
                    rt::ComputeAccelerationStructureSize(rt::BuildType::Host, device, info, ppBuildRangeInfos[info_i]);
                if (dst_as_state->GetSize() < as_minimum_size) {
                    const LogObjectList objlist(info.dstAccelerationStructure);
                    skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-10126", objlist,
                                     info_loc.dot(Field::dstAccelerationStructure),
                                     " was created with size (%" PRIu64
                                     "), but an acceleration structure build with corresponding ppBuildRangeInfos[%" PRIu32
                                     "] requires a minimum size of (%" PRIu64 ").",
                                     dst_as_state->GetSize(), info_i, as_minimum_size);
                }
            }
        }

        skip |= ValidateAccelerationStructuresMemoryAlisasing(LogObjectList(), infoCount, pInfos, info_i, error_obj);
        const VkDeviceSize scratch_i_size = rt::ComputeScratchSize(rt::BuildType::Host, device, info, ppBuildRangeInfos[info_i]);
        auto scratch_i_host_addr = reinterpret_cast<uint64_t>(info.scratchData.hostAddress);
        const vvl::range<uint64_t> scratch_addr_range(scratch_i_host_addr, scratch_i_host_addr + scratch_i_size);

        assert(infoCount > info_i);
        for (auto [other_info_j, other_info] : vvl::enumerate(pInfos + info_i + 1, infoCount - (info_i + 1))) {
            const Location other_info_j_loc = error_obj.location.dot(Field::pInfos, other_info_j + info_i + 1);
            const VkDeviceSize other_scratch_size =
                rt::ComputeScratchSize(rt::BuildType::Host, device, other_info, ppBuildRangeInfos[other_info_j]);
            auto other_scratch_host_addr = reinterpret_cast<uint64_t>(other_info.scratchData.hostAddress);
            const vvl::range<uint64_t> other_scratch_addr_range(other_scratch_host_addr,
                                                                other_scratch_host_addr + other_scratch_size);

            if (scratch_addr_range.intersects(other_scratch_addr_range)) {
                std::string info_i_scratch_str = info_loc.dot(Field::scratchData).Fields();
                std::string other_info_j_scratch_str = other_info_j_loc.dot(Field::scratchData).Fields();
                skip |= LogError(
                    "VUID-vkBuildAccelerationStructuresKHR-scratchData-03704", device, info_loc.dot(Field::scratchData),
                    "overlaps with %s on host address range %s.\n"
                    "%s.hostAddress is %p and assumed scratch size is %" PRIu64
                    ".\n"
                    "%s.hostAddress is %p and assumed scratch size is %" PRIu64 ".",
                    other_info_j_scratch_str.c_str(), string_range_hex(scratch_addr_range & other_scratch_addr_range).c_str(),
                    info_i_scratch_str.c_str(), info.scratchData.hostAddress, scratch_i_size, other_info_j_scratch_str.c_str(),
                    other_info.scratchData.hostAddress, other_scratch_size);
            }
        }

        if (src_as_state && src_as_state->UsesCreateInfo2()) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                skip |=
                    LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11706", device, info_loc.dot(Field::mode),
                             "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but srcAccelerationStructure was created with "
                             "vkCreateAccelerationStructure2KHR (needs to be created with vkCreateAccelerationStructureKHR)");
            }
        }

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& geom = rt::GetGeometry(info, geom_i);
            const Location geometry_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);

            if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                if (const auto* micromap_khr = vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapKHR>(
                        geom.geometry.triangles.pNext)) {
                    if (micromap_khr->micromap != VK_NULL_HANDLE) {
                        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11705", device,
                                         geometry_loc.dot(Field::geometry)
                                             .dot(Field::triangles)
                                             .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapKHR, Field::micromap),
                                         "is %s, but needs to be VK_NULL_HANDLE (since host builds are not supported)",
                                         FormatHandle(micromap_khr->micromap).c_str());
                    }
                }
            }

            if (geom.geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                continue;
            }

            if (src_as_state && src_as_state->UsesCreateInfo2()) {
                skip |=
                    LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11704", device, geometry_loc.dot(Field::geometryType),
                             "is VK_GEOMETRY_TYPE_INSTANCES_KHR, but srcAccelerationStructure was created with "
                             "vkCreateAccelerationStructure2KHR (needs to be created with vkCreateAccelerationStructureKHR)");
            }

            const VkAccelerationStructureBuildRangeInfoKHR& build_range = ppBuildRangeInfos[info_i][geom_i];
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && src_as_state &&
                src_as_state->GetBuildInfo().has_value()) {
                if (geom_i < src_as_state->GetBuildRangeInfos().size()) {
                    if (const uint32_t recorded_primitive_count = src_as_state->GetBuildRangeInfos()[geom_i].primitiveCount;
                        recorded_primitive_count != build_range.primitiveCount) {
                        const LogObjectList objlist(info.srcAccelerationStructure);
                        skip |=
                            LogError("VUID-vkCmdBuildAccelerationStructuresKHR-primitiveCount-03769", objlist, geometry_loc,
                                     " has corresponding VkAccelerationStructureBuildRangeInfoKHR %s, but this build range has its "
                                     "primitiveCount member set to (%" PRIu32 ") when it was last specified as (%" PRIu32 ").",
                                     error_obj.location.dot(Field::ppBuildRangeInfos, info_i).brackets(geom_i).Fields().c_str(),
                                     build_range.primitiveCount, recorded_primitive_count);
                    }
                }
            }

            for (uint32_t instance_i = 0; instance_i < build_range.primitiveCount; ++instance_i) {
                if (!geom.geometry.instances.data.hostAddress) {
                    continue;
                }
                const VkAccelerationStructureInstanceKHR* instance = nullptr;
                if (geom.geometry.instances.arrayOfPointers) {
                    auto instance_pointers_array = reinterpret_cast<VkAccelerationStructureInstanceKHR const* const*>(
                        geom.geometry.instances.data.hostAddress);
                    instance = instance_pointers_array[instance_i];
                } else {
                    auto instances_array =
                        reinterpret_cast<VkAccelerationStructureInstanceKHR const*>(geom.geometry.instances.data.hostAddress);
                    instance = instances_array + instance_i;
                }

                // Can only get here if geom.geometry.instances.arrayOfPointers is true
                if (!instance) {
                    skip |= LogError(
                        "VUID-vkBuildAccelerationStructuresKHR-pInfos-03779", device,
                        geometry_loc.dot(Field::geometry)
                            .dot(Field::instances)
                            .dot(Field::data)
                            .dot(Field::hostAddress, instance_i),
                        "(0x%p) does not reference a valid VkAccelerationStructureKHR object. %s is %s.", instance,
                        geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::arrayOfPointers).Fields().c_str(),
                        string_VkBool32(geom.geometry.instances.arrayOfPointers).c_str());

                    // Following checks rely on instance not being null
                    continue;
                }

                const VkAccelerationStructureKHR accel_struct =
                    CastFromUint64<VkAccelerationStructureKHR>(instance->accelerationStructureReference);
                auto accel_struct_state = Get<vvl::AccelerationStructureKHR>(accel_struct);

                if (!accel_struct_state) {
                    skip |= LogError(
                        "VUID-vkBuildAccelerationStructuresKHR-pInfos-03779", device,
                        geometry_loc.dot(Field::geometry)
                            .dot(Field::instances)
                            .dot(Field::data)
                            .dot(Field::hostAddress, instance_i)
                            .dot(Field::accelerationStructureReference),
                        "(%" PRIu64 ") does not reference a valid VkAccelerationStructureKHR object. %s is %s.",
                        instance->accelerationStructureReference,
                        geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::arrayOfPointers).Fields().c_str(),
                        string_VkBool32(geom.geometry.instances.arrayOfPointers).c_str());
                } else {
                    if ((instance->flags & VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_BIT_KHR) &&
                        accel_struct_state->GetBuildInfo().has_value() &&
                        ((accel_struct_state->GetBuildInfo()->flags &
                          VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISABLE_OPACITY_MICROMAPS_BIT_KHR) == 0)) {
                        skip |= LogError(
                            "VUID-VkAccelerationStructureInstanceKHR-flags-11581", LogObjectList(device, accel_struct),
                            geometry_loc.dot(Field::geometry)
                                .dot(Field::instances)
                                .dot(Field::data)
                                .dot(Field::hostAddress, instance_i)
                                .dot(Field::flags),
                            "includes VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_BIT_KHR, but accelerationStructureReference "
                            "(%s) "
                            "refers to an acceleration structure built with flags %s, which do not include "
                            "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISABLE_OPACITY_MICROMAPS_BIT_KHR.",
                            FormatHandle(accel_struct).c_str(),
                            string_VkBuildAccelerationStructureFlagsKHR(accel_struct_state->GetBuildInfo()->flags).c_str());
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                          const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                          const uint32_t* pIndirectStrides,
                                                                          const uint32_t* const* ppMaxPrimitiveCounts,
                                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (!cb_state->unprotected) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-09547", commandBuffer, error_obj.location,
                         "called in a protected command buffer.");
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        if (auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure)) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                const auto src_as_buffer = src_as_state->GetFirstValidBuffer(*device_state);
                if (!src_as_buffer) {
                    const LogObjectList objlist(commandBuffer, info.srcAccelerationStructure);
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03708", objlist,
                                     info_loc.dot(Field::mode),
                                     "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR but the buffer associated with "
                                     "srcAccelerationStructure is not valid.");
                } else {
                    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_as_buffer.state,
                                                          info_loc.dot(Field::srcAccelerationStructure),
                                                          "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03708");
                }
                skip |= ValidateAccelerationStructureBuildGeometryInfoUpdate(*src_as_state, info, info_loc, error_obj.handle);
            }
        }

        if (auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            const auto dst_as_buffer = dst_as_state->GetFirstValidBuffer(*device_state);
            if (!dst_as_buffer || dst_as_buffer.state->Destroyed()) {
                const LogObjectList objlist(commandBuffer, info.srcAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03707", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure), "is backed by an invalid or destroyed buffer.");
            } else {
                skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_as_buffer.state,
                                                      info_loc.dot(Field::srcAccelerationStructure),
                                                      "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03707");
            }
            skip |= ValidateAccelerationStructureBuildDst(*dst_as_state, info, info_loc, error_obj.handle);
        }

        skip |= ValidateAccelerationStructuresMemoryAlisasing(commandBuffer, infoCount, pInfos, info_i, error_obj);

        skip |= ValidateAccelerationStructureBuildGeometryInfoDevice(commandBuffer, info_i, info, nullptr, info_loc);
        skip |= ValidateAccelerationStructureBuildScratch(commandBuffer, info, nullptr, info_loc);
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    if (auto as_state = Get<vvl::AccelerationStructureKHR>(accelerationStructure)) {
        skip |= ValidateObjectNotInUse(as_state.get(), error_obj.location,
                                       "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    }
    return skip;
}

bool CoreChecks::PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                         const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                         VkQueryType queryType, size_t dataSize, void* pData,
                                                                         size_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        const Location as_loc = error_obj.location.dot(Field::pAccelerationStructures, i);
        auto as_state = Get<vvl::AccelerationStructureKHR>(pAccelerationStructures[i]);
        ASSERT_AND_CONTINUE(as_state);

        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*as_state, as_loc,
                                                             "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*as_state, as_loc,
                                                                  "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03784");

        if (!as_state->GetBuildInfo().has_value()) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-11591",
                             pAccelerationStructures[i], as_loc,
                             "has not been constructed prior to the execution of this command on the device.");
        } else if (as_state->GetBuildInfo().has_value()) {
            if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
                if (!(as_state->GetBuildInfo()->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                    skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431",
                                     pAccelerationStructures[i], as_loc, "has flags %s.",
                                     string_VkBuildAccelerationStructureFlagsKHR(as_state->GetBuildInfo()->flags).c_str());
                }
            }
        }

        if (as_state->UsesCreateInfo2()) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-11592",
                             pAccelerationStructures[i], as_loc,
                             "was created with vkCreateAccelerationStructure2KHR, but needs to be created with "
                             "vkCreateAccelerationStructureKHR");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    auto query_pool_state = Get<vvl::QueryPool>(queryPool);
    ASSERT_AND_RETURN_SKIP(query_pool_state);
    const auto& query_pool_ci = query_pool_state->create_info;
    if (query_pool_ci.queryType != queryType) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryPool-02493", commandBuffer,
                         error_obj.location.dot(Field::queryType),
                         "was created with %s which is different from the type queryPool was created with (%s).",
                         string_VkQueryType(queryType), string_VkQueryType(query_pool_ci.queryType));
    }
    if (firstQuery + accelerationStructureCount > query_pool_state->create_info.queryCount) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-query-04880", commandBuffer,
                         error_obj.location.dot(Field::firstQuery),
                         "(%" PRIu32 ") + accelerationStructureCount (%" PRIu32 "), or %" PRIu32
                         ", is greater than the number of queries in queryPool (%" PRIu32 ").",
                         firstQuery, accelerationStructureCount, firstQuery + accelerationStructureCount,
                         query_pool_state->create_info.queryCount);
    }

    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        const Location as_loc = error_obj.location.dot(Field::pAccelerationStructures, i);
        auto as_state = Get<vvl::AccelerationStructureKHR>(pAccelerationStructures[i]);
        ASSERT_AND_CONTINUE(as_state);

        const auto as_buffer = as_state->GetFirstValidBuffer(*device_state);
        if (!as_buffer) {
            return skip;
        }
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *as_buffer.state, as_loc.dot(Field::buffer),
                                              "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-buffer-03736");

        if (!as_state->GetBuildInfo().has_value()) {
            LogObjectList objlist(commandBuffer, pAccelerationStructures[i]);
            skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-11684", objlist, as_loc,
                             "has not been built.");
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR && as_state->GetBuildInfo().has_value()) {
            if (!(as_state->GetBuildInfo()->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                LogObjectList objlist(commandBuffer, pAccelerationStructures[i]);
                skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431", objlist, as_loc,
                                 "was built with %s, but queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR.",
                                 string_VkBuildAccelerationStructureFlagsKHR(as_state->GetBuildInfo()->flags).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR& as_info,
                                                          const VulkanTypedHandle& handle, const Location& info_loc) const {
    bool skip = false;

    auto src_as_state = Get<vvl::AccelerationStructureKHR>(as_info.src);
    if (src_as_state) {
        const vvl::BufferAndOffset src_as_buffer = src_as_state->GetFirstValidBuffer(*device_state);
        if (!src_as_buffer || src_as_buffer.state->Destroyed()) {
            skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718", as_info.src, info_loc.dot(Field::src),
                             "is backed by %s buffer.", (!src_as_buffer ? "an invalid" : "a destroyed"));
        } else {
            skip |= ValidateMemoryIsBoundToBuffer(LogObjectList(as_info.src, src_as_buffer.state->VkHandle()), *src_as_buffer.state,
                                                  info_loc.dot(Field::src), "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
        }

        if (as_info.mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR && src_as_state->GetBuildInfo().has_value()) {
            if (!(src_as_state->GetBuildInfo()->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                const LogObjectList objlist(handle, as_info.src);
                skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-src-03411", objlist, info_loc.dot(Field::src),
                                 "(%s) must have been built with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR"
                                 "if mode is VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR.",
                                 FormatHandle(as_info.src).c_str());
            }
        }
    }
    auto dst_as_state = Get<vvl::AccelerationStructureKHR>(as_info.dst);
    if (dst_as_state) {
        const vvl::BufferAndOffset dst_as_buffer = dst_as_state->GetFirstValidBuffer(*device_state);
        if (!dst_as_buffer || dst_as_buffer.state->Destroyed()) {
            skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719", as_info.dst, info_loc.dot(Field::dst),
                             "is backed by %s buffer.", (!dst_as_buffer ? "an invalid" : "a destroyed"));
        } else {
            skip |= ValidateMemoryIsBoundToBuffer(LogObjectList(as_info.dst, dst_as_buffer.state->VkHandle()), *dst_as_buffer.state,
                                                  info_loc.dot(Field::dst), "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
        }
    }

    if (src_as_state && dst_as_state) {
        skip |= ValidateAccelStructsMemoryDoNotOverlap(info_loc.function, LogObjectList(), *src_as_state, info_loc.dot(Field::src),
                                                       *dst_as_state, info_loc.dot(Field::dst),
                                                       "VUID-VkCopyAccelerationStructureInfoKHR-dst-07791");
    }

    return skip;
}

// (These VUs are all a mess)
// https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7112#note_604131
// For VU 03745, it is legal to create acceleration structure with a VkBuffer (vkCreateAcclerationStructureKHR path)
// that is not bound to device memory. It must be bound to device memory before its first use (ex. build, copy, etc).
// However, now that vkCreateAccelrationStructure2KHR is around, 03745 does not make sense as you don't bind VkDeviceAddress to
// memory. 11717 is meant to test for both definitions without special casing based on the creation method.
bool CoreChecks::ValidateCopyMemoryToAccelerationStructureInfoKHR(const vvl::AccelerationStructureKHR& dst_as_state,
                                                                  const Location& dst_as_loc) const {
    bool skip = false;

    const vvl::BufferAndOffset dst_as_buffer = dst_as_state.GetFirstValidBuffer(*device_state);
    if (!dst_as_buffer || dst_as_buffer.state->Destroyed()) {
        skip |= LogError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-11717", dst_as_state.VkHandle(), dst_as_loc,
                         "is backed by %s buffer.", (!dst_as_buffer ? "an invalid" : "a destroyed"));
    } else {
        skip |= ValidateMemoryIsBoundToBuffer(LogObjectList(dst_as_state.VkHandle(), dst_as_buffer.state->VkHandle()),
                                              *dst_as_buffer.state, dst_as_loc,
                                              "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-11717");
    }

    return skip;
}

// See ValidateCopyMemoryToAccelerationStructureInfoKHR() for reason why we have this function
bool CoreChecks::ValidateCopyAccelerationStructureToMemoryInfoKHR(const vvl::AccelerationStructureKHR& src_as_state,
                                                                  const Location& src_as_loc) const {
    bool skip = false;

    const vvl::BufferAndOffset src_as_buffer = src_as_state.GetFirstValidBuffer(*device_state);
    if (!src_as_buffer || src_as_buffer.state->Destroyed()) {
        skip |= LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-11714", src_as_state.VkHandle(), src_as_loc,
                         "is backed by %s buffer.", (!src_as_buffer ? "an invalid" : "a destroyed"));
    } else {
        skip |= ValidateMemoryIsBoundToBuffer(LogObjectList(src_as_state.VkHandle(), src_as_buffer.state->VkHandle()),
                                              *src_as_buffer.state, src_as_loc,
                                              "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-11714");
    }

    return skip;
}

bool CoreChecks::ValidateDeserializedAccelerationStructureReferencedMicromaps(const vvl::AccelerationStructureKHR& src_as_state,
                                                                              const LogObjectList& objlist, const Location& src_loc,
                                                                              const char* vuid) const {
    bool skip = false;

    if (!src_as_state.WasDeserialized()) {
        return skip;
    }

    const auto build_info = src_as_state.GetBuildInfo();
    if (!build_info.has_value()) {
        return skip;
    }

    for (uint32_t geom_i = 0; geom_i < build_info->geometryCount; ++geom_i) {
        const VkAccelerationStructureGeometryKHR& geom = rt::GetGeometry(*build_info->ptr(), geom_i);
        if (geom.geometryType != VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
            continue;
        }

        const auto* triangle_micromap =
            vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapKHR>(geom.geometry.triangles.pNext);
        if (!triangle_micromap || triangle_micromap->micromap == VK_NULL_HANDLE) {
            continue;  // Seems having a null handle is valid?
        }

        auto micromap_state = Get<vvl::AccelerationStructureKHR>(triangle_micromap->micromap);
        if (!micromap_state || !micromap_state->WasDeserialized()) {
            skip |=
                LogError(vuid, objlist, src_loc,
                         "(%s) was constructed through deserialization and references micromap %s in recorded geometry[%" PRIu32
                         "], but that micromap was not deserialized from the corresponding original serialized micromap data.",
                         FormatHandle(src_as_state.Handle()).c_str(), FormatHandle(triangle_micromap->micromap).c_str(), geom_i);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);
    if (auto src_as_state = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        skip |= ValidateDeserializedAccelerationStructureReferencedMicromaps(
            *src_as_state, LogObjectList(commandBuffer, pInfo->src), info_loc.dot(Field::src),
            "VUID-vkCmdCopyAccelerationStructureKHR-src-11634");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, operation, error_obj.location.dot(Field::operation),
                                      "VUID-vkDestroyDeferredOperationKHR-operation-03436");
    return skip;
}

bool CoreChecks::PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyAccelerationStructureKHR-deferredOperation-03678");

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);

    if (auto src_as_state = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const Location src_as_loc = info_loc.dot(Field::src);
        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*src_as_state, src_as_loc,
                                                             "VUID-vkCopyAccelerationStructureKHR-buffer-03727");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*src_as_state, src_as_loc,
                                                                  "VUID-vkCopyAccelerationStructureKHR-buffer-03780");

        if (src_as_state->UsesCreateInfo2()) {
            skip |= LogError("VUID-vkCopyAccelerationStructureKHR-src-11588", pInfo->src, src_as_loc,
                             "was created with vkCreateAccelerationStructure2KHR, but needs to be created with "
                             "vkCreateAccelerationStructureKHR");
        }

        skip |= ValidateDeserializedAccelerationStructureReferencedMicromaps(*src_as_state, LogObjectList(pInfo->src), src_as_loc,
                                                                             "VUID-vkCopyAccelerationStructureKHR-src-11587");
    }

    if (auto dst_as_state = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        const Location dst_as_loc = info_loc.dot(Field::dst);
        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*dst_as_state, dst_as_loc,
                                                             "VUID-vkCopyAccelerationStructureKHR-buffer-03728");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*dst_as_state, dst_as_loc,
                                                                  "VUID-vkCopyAccelerationStructureKHR-buffer-03781");

        if (dst_as_state->UsesCreateInfo2()) {
            skip |= LogError("VUID-vkCopyAccelerationStructureKHR-dst-11589", pInfo->dst, dst_as_loc,
                             "was created with vkCreateAccelerationStructure2KHR, but needs to be created with "
                             "vkCreateAccelerationStructureKHR");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyAccelerationStructureToMemoryKHR-deferredOperation-03678");

    if (const auto src_as = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const Location src_as_loc = info_loc.dot(Field::src);
        skip |= ValidateCopyAccelerationStructureToMemoryInfoKHR(*src_as, src_as_loc);
        if (src_as->UsesCreateInfo1()) {
            if (auto src_as_buffer = src_as->GetFirstValidBuffer(*device_state)) {
                skip |= ValidateAccelStructBufferMemoryIsHostVisible(*src_as, src_as_loc,
                                                                     "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03731");

                skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(
                    *src_as, src_as_loc, "VUID-vkCopyAccelerationStructureToMemoryKHR-buffer-03783");
            }
        } else {
            skip |= LogError("VUID-vkCopyAccelerationStructureToMemoryKHR-src-11678", pInfo->src, src_as_loc,
                             "was created with vkCreateAccelerationStructure2KHR, but needs to be created with "
                             "vkCreateAccelerationStructureKHR");
        }

        skip |= ValidateDeserializedAccelerationStructureReferencedMicromaps(
            *src_as, LogObjectList(pInfo->src), src_as_loc, "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-11582");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    const Location info_loc = error_obj.location.dot(Field::pInfo);

    if (auto src_as_state = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const Location src_as_loc = info_loc.dot(Field::src);
        skip |= ValidateCopyAccelerationStructureToMemoryInfoKHR(*src_as_state, src_as_loc);

        if (src_as_state->GetType() == VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR) {
            if (!IsPointerAligned(pInfo->dst.deviceAddress, 128)) {
                const LogObjectList objlist(commandBuffer, pInfo->src);
                skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-11699", objlist,
                                 info_loc.dot(Field::dst).dot(Field::deviceAddress),
                                 "(0x%" PRIx64 ") must be aligned to 128 bytes.", pInfo->dst.deviceAddress);
            }

            if (pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
                const VkDeviceAddress src_address = src_as_state->GetAccelerationStructureAddress();
                if (!IsPointerAligned(src_address, 128)) {
                    const LogObjectList objlist(commandBuffer, pInfo->src);
                    skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-11700", objlist, src_as_loc,
                                     "was originally built with type VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR, "
                                     "so its device address "
                                     "(0x%" PRIx64 ") must be aligned to 128 bytes.",
                                     src_address);
                }
            }
        }
    }

    const VkDeviceAddress dst_address = pInfo->dst.deviceAddress;
    if (dst_address == 0) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03739", commandBuffer,
                         info_loc.dot(Field::dst).dot(Field::deviceAddress), "is zero");
    } else {
        auto buffers = GetBuffersByAddress(dst_address);
        if (buffers.empty()) {
            skip |= LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-11715", commandBuffer,
                             info_loc.dot(Field::dst).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") does not refer to any known buffer.", dst_address);
        } else {
            // More info at https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7112#note_604035
            for (const auto* buffer_state : buffers) {
                skip |=
                    ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, info_loc.dot(Field::dst).dot(Field::deviceAddress),
                                                  "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-11715");
            }
        }
    }

    // TODO this likely is already covered by 11715 and just bad spec VUs that need to get fix
    skip |= ValidateDeviceAddress(info_loc.dot(Field::dst).dot(Field::deviceAddress), LogObjectList(commandBuffer), dst_address);

    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    const Location info_loc = error_obj.location.dot(Field::pInfo);

    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMemoryToAccelerationStructureKHR-deferredOperation-03678");

    if (auto dst_as_state = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        const Location dst_as_loc = info_loc.dot(Field::dst);
        skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(*dst_as_state, dst_as_loc);
        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*dst_as_state, dst_as_loc,
                                                             "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*dst_as_state, dst_as_loc,
                                                                  "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03782");
        if (dst_as_state->UsesCreateInfo2()) {
            skip |= LogError("VUID-vkCopyMemoryToAccelerationStructureKHR-dst-11677", device, dst_as_loc,
                             "was created with vkCreateAccelerationStructure2KHR, but needs to be created with "
                             "vkCreateAccelerationStructureKHR");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    const Location info_loc = error_obj.location.dot(Field::pInfo);

    const VkDeviceAddress src_address = pInfo->src.deviceAddress;
    if (src_address == 0) {
        skip |= LogError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03742", commandBuffer,
                         info_loc.dot(Field::src).dot(Field::deviceAddress), "is zero");
    } else {
        // handle if deviceAddress is null as well
        auto buffers = GetBuffersByAddress(src_address);
        if (buffers.empty()) {
            skip |= LogError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-11716", commandBuffer,
                             info_loc.dot(Field::src).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") does not refer to any known buffer.", src_address);
        } else {
            // More info at https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7112#note_604035
            for (const auto* buffer_state : buffers) {
                skip |=
                    ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, info_loc.dot(Field::src).dot(Field::deviceAddress),
                                                  "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-11716");
            }
        }
    }

    // TODO this likely is already covered by 11715 and just bad spec VUs that need to get fix
    skip |= ValidateDeviceAddress(info_loc.dot(Field::src).dot(Field::deviceAddress), LogObjectList(commandBuffer), src_address);

    return skip;
}

// Calculates the total number of shader groups taking libraries into account.
static uint32_t CalcTotalShaderGroupCount(const CoreChecks& validator, const vvl::Pipeline& pipeline) {
    uint32_t total = 0;
    const vku::safe_VkRayTracingPipelineCreateInfoCommon& create_info = pipeline.RayTracingCreateInfo();
    total = create_info.groupCount;

    if (create_info.pLibraryInfo) {
        for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
            auto library_pipeline_state = validator.Get<vvl::Pipeline>(create_info.pLibraryInfo->pLibraries[i]);
            if (!library_pipeline_state) continue;
            total += CalcTotalShaderGroupCount(validator, *library_pipeline_state.get());
        }
    }
    return total;
}

bool CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    auto pipeline_ptr = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_ptr);
    if (pipeline_ptr->pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        skip |=
            LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-04619", pipeline, error_obj.location.dot(Field::pipeline),
                     "is a %s pipeline.", string_VkPipelineBindPoint(pipeline_ptr->pipeline_type));
        return skip;
    }

    const vvl::Pipeline& pipeline_state = *pipeline_ptr;
    if (pipeline_state.create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        if (!enabled_features.pipelineLibraryGroupHandles) {
            skip |= LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-07828", pipeline,
                             error_obj.location.dot(Field::pipeline),
                             "was created with %s, but the pipelineLibraryGroupHandles feature was not enabled.",
                             string_VkPipelineCreateFlags2(pipeline_state.create_flags).c_str());
        }
    }
    if (dataSize < (phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleSize * groupCount)) {
        skip |=
            LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-dataSize-02420", device, error_obj.location.dot(Field::dataSize),
                     "(%zu) must be at least "
                     "shaderGroupHandleSize (%" PRIu32 ") * groupCount (%" PRIu32 ").",
                     dataSize, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleSize, groupCount);
    }

    const uint32_t total_group_count = CalcTotalShaderGroupCount(*this, pipeline_state);

    if (firstGroup >= total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-firstGroup-04050", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32 ") must be less than the number of shader groups in pipeline (%" PRIu32 ").", firstGroup,
                         total_group_count);
    }
    if ((firstGroup + groupCount) > total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-firstGroup-02419", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32 ") + groupCount (%" PRIu32
                         ") must be less than or equal to the number "
                         "of shader groups in pipeline (%" PRIu32 ").",
                         firstGroup, groupCount, total_group_count);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                uint32_t firstGroup, uint32_t groupCount,
                                                                                size_t dataSize, void* pData,
                                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    if (dataSize < (phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleCaptureReplaySize * groupCount)) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484", device,
                         error_obj.location.dot(Field::dataSize),
                         "(%zu) must be at least "
                         "shaderGroupHandleCaptureReplaySize (%" PRIu32 ") * groupCount (%" PRIu32 ").",
                         dataSize, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleCaptureReplaySize, groupCount);
    }
    auto pipeline_state = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_state);
    if (pipeline_state->pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-04620", pipeline,
                         error_obj.location.dot(Field::pipeline), "is a %s pipeline.",
                         string_VkPipelineBindPoint(pipeline_state->pipeline_type));
        return skip;
    }

    if (pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        if (!enabled_features.pipelineLibraryGroupHandles) {
            skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-07829", pipeline,
                             error_obj.location.dot(Field::pipeline),
                             "was created with %s, but the pipelineLibraryGroupHandles feature was not enabled.",
                             string_VkPipelineCreateFlags2(pipeline_state->create_flags).c_str());
        }
    }

    const uint32_t total_group_count = CalcTotalShaderGroupCount(*this, *pipeline_state);

    if (firstGroup >= total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-04051", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32
                         ") must be less than the number of shader "
                         "groups in pipeline (%" PRIu32 ").",
                         firstGroup, total_group_count);
    }
    if ((firstGroup + groupCount) > total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32 ") + groupCount (%" PRIu32
                         ") must be less than or equal to the number of shader groups in pipeline (%" PRIu32 ").",
                         firstGroup, groupCount, total_group_count);
    }
    if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-03607", pipeline,
                         error_obj.location.dot(Field::pipeline), "was created with %s.",
                         string_VkPipelineCreateFlags2(pipeline_state->create_flags).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

static vku::safe_VkRayTracingShaderGroupCreateInfoKHR* GetRayTracingShaderGroup(const CoreChecks& validator,
                                                                                const vvl::Pipeline& pipeline, uint32_t group_i) {
    const vku::safe_VkRayTracingPipelineCreateInfoCommon& create_info = pipeline.RayTracingCreateInfo();
    // Target group is in currently explored pipeline
    if (group_i < create_info.groupCount) {
        return &create_info.pGroups[group_i];
    }

    // Target group is in a linked pipeline library, recursively explore them
    if (create_info.pLibraryInfo) {
        for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
            auto library_pipeline_state = validator.Get<vvl::Pipeline>(create_info.pLibraryInfo->pLibraries[i]);
            if (!library_pipeline_state) {
                continue;
            }
            return GetRayTracingShaderGroup(validator, *library_pipeline_state.get(), group_i - create_info.groupCount);
        }
    }

    return nullptr;
}

bool CoreChecks::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                     VkShaderGroupShaderKHR groupShader,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    auto pipeline_state = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_state);

    if (pipeline_state->pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        skip |= LogError("VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-04622", pipeline,
                         error_obj.location.dot(Field::pipeline), "is a %s pipeline.",
                         string_VkPipelineBindPoint(pipeline_state->pipeline_type));
    } else {
        const auto& create_info = pipeline_state->RayTracingCreateInfo();
        const uint32_t total_group_count = CalcTotalShaderGroupCount(*this, *pipeline_state);
        if (group >= total_group_count) {
            skip |= LogError(
                "VUID-vkGetRayTracingShaderGroupStackSizeKHR-group-03608", pipeline, error_obj.location.dot(Field::group),
                "(%" PRIu32 ") must be less than the number of shader groups in pipeline (create info had a groupCount of %" PRIu32
                " and %" PRIu32 " got added from pLibraryInfo).",
                group, create_info.groupCount, total_group_count - create_info.groupCount);
        } else {
            const auto* group_info = GetRayTracingShaderGroup(*this, *pipeline_state, group);
            ASSERT_AND_RETURN_SKIP(group_info);
            bool unused_group = false;
            switch (groupShader) {
                case VK_SHADER_GROUP_SHADER_GENERAL_KHR:
                    if (group_info->generalShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;
                case VK_SHADER_GROUP_SHADER_CLOSEST_HIT_KHR:
                    if (group_info->closestHitShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;
                case VK_SHADER_GROUP_SHADER_ANY_HIT_KHR:
                    if (group_info->anyHitShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;
                case VK_SHADER_GROUP_SHADER_INTERSECTION_KHR:
                    if (group_info->intersectionShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;

                default:
                    break;
            }
            if (unused_group) {
                const LogObjectList objlist(device, pipeline);
                skip |= LogError("VUID-vkGetRayTracingShaderGroupStackSizeKHR-groupShader-03609", objlist,
                                 error_obj.location.dot(Field::groupShader),
                                 "is %s but the corresponding shader in shader group %" PRIu32 " is VK_SHADER_UNUSED_KHR.",
                                 string_VkShaderGroupShaderKHR(groupShader), group);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateRaytracingShaderBindingTable(const vvl::CommandBuffer& cb_state, const Location& table_loc,
                                                      const char* vuid_binding_table_flag,
                                                      const VkStridedDeviceAddressRegionKHR& binding_table) const {
    bool skip = false;

    if (binding_table.deviceAddress == 0 || binding_table.size == 0) {
        return skip;
    }

    const VkDeviceSize requested_size = binding_table.size - 1;
    const vvl::range<VkDeviceSize> requested_range(binding_table.deviceAddress, binding_table.deviceAddress + requested_size);

    BufferAddressValidation<3> buffer_address_validator = {{{
        {vuid_binding_table_flag,
         [](const vvl::Buffer& buffer_state) {
             return (static_cast<uint32_t>(buffer_state.usage) & VK_BUFFER_USAGE_2_SHADER_BINDING_TABLE_BIT_KHR) == 0;
         },
         []() { return "The following buffers are missing VK_BUFFER_USAGE_2_SHADER_BINDING_TABLE_BIT_KHR"; }, kUsageErrorMsgBuffer},

        {"VUID-VkStridedDeviceAddressRegionKHR-size-04631",
         [&requested_range](const vvl::Buffer& buffer_state) {
             const auto buffer_address_range = buffer_state.DeviceAddressRange();
             return !buffer_address_range.includes(requested_range);
         },
         [&table_loc, &binding_table]() {
             return "The " + table_loc.Fields() + "->size (" + std::to_string(binding_table.size) +
                    ") - 1 does not fit in any buffer";
         },
         kEmptyErrorMsgBuffer},

        {"VUID-VkStridedDeviceAddressRegionKHR-size-04632",
         [&binding_table](const vvl::Buffer& buffer_state) { return binding_table.stride > buffer_state.GetSize(); },
         [table_loc, &binding_table]() {
             return "The " + table_loc.Fields() + "->stride (" + std::to_string(binding_table.stride) +
                    ") does not fit in any buffer";
         },
         kEmptyErrorMsgBuffer},
    }}};

    skip |= buffer_address_validator.ValidateDeviceAddress(*this, table_loc.dot(Field::deviceAddress),
                                                           cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                                                           binding_table.deviceAddress, requested_size);

    return skip;
}

bool CoreChecks::ValidateDeferredOperation(VkDevice device, VkDeferredOperationKHR deferred_operation, const Location& loc,
                                           const char* vuid) const {
    // validate in core check because need to make sure it is a valid VkDeferredOperationKHR object
    bool skip = false;
    if (deferred_operation != VK_NULL_HANDLE) {
        VkResult result = DispatchGetDeferredOperationResultKHR(device, deferred_operation);
        if (result == VK_NOT_READY) {
            skip |= LogError(vuid, deferred_operation, loc.dot(Field::deferredOperation), "%s is not completed.",
                             FormatHandle(deferred_operation).c_str());
        }
    }
    return skip;
}
