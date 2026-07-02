/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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

#include "state_tracker/ray_tracing_state.h"

#include "state_tracker/state_tracker.h"

namespace vvl {

AccelerationStructureKHR::AccelerationStructureKHR(vvl::DeviceState& device_state,
                                                   const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                   VkAccelerationStructureKHR handle)
    : StateObject(handle, kVulkanObjectTypeAccelerationStructureKHR),
      create_info(std::in_place_type<CreateInfo1>, pCreateInfo, device_state.Get<vvl::Buffer>(pCreateInfo->buffer)) {
    auto& ci_1 = std::get<CreateInfo1>(create_info);
    if (ci_1.buffer_state) {
        VkDeviceAddress buffer_address = 0;
        if (ci_1.buffer_state->deviceAddress != 0) {
            buffer_address = ci_1.buffer_state->deviceAddress;
        } else if (ci_1.buffer_state->Binding()) {
            buffer_address = device_state.GetBufferDeviceAddressHelper(ci_1.buffer_state->VkHandle());
        }
        device_address_range = {buffer_address + pCreateInfo->offset, pCreateInfo->size};
    }
}

AccelerationStructureKHR::AccelerationStructureKHR(vvl::DeviceState& device_state,
                                                   const VkAccelerationStructureCreateInfo2KHR* pCreateInfo,
                                                   VkAccelerationStructureKHR handle)
    : StateObject(handle, kVulkanObjectTypeAccelerationStructureKHR),
      create_info(std::in_place_type<vku::safe_VkAccelerationStructureCreateInfo2KHR>, pCreateInfo),
      device_address_range(pCreateInfo->addressRange) {}

AccelerationStructureKHR::~AccelerationStructureKHR() {
    if (!Destroyed()) {
        Destroy();
    }
}

void AccelerationStructureKHR::LinkChildNodes() {
    if (auto* ci_1 = std::get_if<CreateInfo1>(&create_info)) {
        if (ci_1->buffer_state) {
            ci_1->buffer_state->AddParent(this);
        }
    }
}

void AccelerationStructureKHR::Destroy() {
    for (auto& item : sub_states_) {
        item.second->Destroy();
    }
    if (auto* ci_1 = std::get_if<CreateInfo1>(&create_info)) {
        if (ci_1->buffer_state) {
            ci_1->buffer_state->RemoveParent(this);
            ci_1->buffer_state = nullptr;
        }
    }
    StateObject::Destroy();
}

void AccelerationStructureKHR::NotifyInvalidate(const StateObject::NodeList& invalid_nodes, bool unlink) {
    for (auto& item : sub_states_) {
        item.second->NotifyInvalidate(invalid_nodes, unlink);
    }
    StateObject::NotifyInvalidate(invalid_nodes, unlink);
}

void AccelerationStructureKHR::Build(const VkAccelerationStructureBuildGeometryInfoKHR* pInfo, const bool is_host,
                                     const VkAccelerationStructureBuildRangeInfoKHR* build_range_info) {
    if (!GetBuildInfo().has_value()) {
        build_geometry_info = vku::safe_VkAccelerationStructureBuildGeometryInfoKHR();
    }
    build_geometry_info->initialize(pInfo, is_host, build_range_info);
};

void AccelerationStructureKHR::UpdateBuildRangeInfos(const VkAccelerationStructureBuildRangeInfoKHR* p_build_range_infos,
                                                     uint32_t geometry_count) {
    // range info is null for indirect builds (vkCmdBuildAccelerationStructuresIndirectKHR) or VK_GEOMETRY_TYPE_MICROMAP_KHR
    if (!p_build_range_infos) {
        build_range_infos.clear();
        return;
    }

    build_range_infos.resize(geometry_count);
    for (const auto [i, build_range] : vvl::enumerate(p_build_range_infos, geometry_count)) {
        build_range_infos[i] = build_range;
    }
}

bool AccelerationStructureKHR::UsesCreateInfo1() const { return std::get_if<CreateInfo1>(&create_info) != nullptr; }

bool AccelerationStructureKHR::UsesCreateInfo2() const {
    return std::get_if<vku::safe_VkAccelerationStructureCreateInfo2KHR>(&create_info) != nullptr;
}

BufferAndOffset AccelerationStructureKHR::GetFirstValidBuffer(const vvl::DeviceState& device_state) const {
    if (const auto* ci_1 = std::get_if<CreateInfo1>(&create_info)) {
        return {ci_1->buffer_state.get(), ci_1->ci.offset};
    }

    const auto& ci_2 = std::get<vku::safe_VkAccelerationStructureCreateInfo2KHR>(create_info);
    const auto buffer_states = device_state.GetBuffersByAddress(ci_2.addressRange.address);
    const vvl::range<VkDeviceAddress> as_addr_range{ci_2.addressRange.address, ci_2.addressRange.address + ci_2.addressRange.size};
    for (const auto& as_buffer : buffer_states) {
        if ((as_buffer->usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) &&
            as_buffer->DeviceAddressRange().includes(as_addr_range)) {
            return {as_buffer, ci_2.addressRange.address - as_buffer->deviceAddress};
        }
    }
    return {nullptr, 0};
}

VkAccelerationStructureCreateFlagsKHR AccelerationStructureKHR::GetCreateFlags() const {
    if (const auto* ci_1 = std::get_if<CreateInfo1>(&create_info)) {
        return ci_1->ci.createFlags;
    }
    return std::get<vku::safe_VkAccelerationStructureCreateInfo2KHR>(create_info).createFlags;
}

VkDeviceSize AccelerationStructureKHR::GetSize() const {
    if (const auto* ci_1 = std::get_if<CreateInfo1>(&create_info)) {
        return ci_1->ci.size;
    }
    return device_address_range.size;
}

VkAccelerationStructureTypeKHR AccelerationStructureKHR::GetType() const {
    if (const auto* ci_1 = std::get_if<CreateInfo1>(&create_info)) {
        return ci_1->ci.type;
    }
    return std::get<vku::safe_VkAccelerationStructureCreateInfo2KHR>(create_info).type;
}

VkDeviceAddressRangeKHR AccelerationStructureKHR::GetEffectiveDeviceAddressRange() const { return device_address_range; }

vvl::range<VkDeviceAddress> AccelerationStructureKHR::GetVvlEffectiveDeviceAddressRange() const {
    const VkDeviceAddressRangeKHR khr_device_address_range = GetEffectiveDeviceAddressRange();
    return {khr_device_address_range.address, khr_device_address_range.address + khr_device_address_range.size};
}

std::string AccelerationStructureKHR::Describe(const Logger& dev_data) const {
    std::stringstream ss;

    ss << dev_data.FormatHandle(Handle());
    if (UsesCreateInfo1()) {
        const auto* ci_1 = std::get_if<CreateInfo1>(&create_info);
        ss << ", createFlags: " << string_VkAccelerationStructureCreateFlagsKHR(ci_1->ci.createFlags)
           << ", buffer: " << dev_data.FormatHandle(*ci_1->buffer_state) << ", offset: " << ci_1->ci.offset
           << ", size: " << ci_1->ci.size << ", type: " << string_VkAccelerationStructureTypeKHR(ci_1->ci.type)
           << ", deviceAddress: 0x" << std::hex << ci_1->ci.deviceAddress;
    } else if (UsesCreateInfo2()) {
        const auto& ci_2 = std::get<vku::safe_VkAccelerationStructureCreateInfo2KHR>(create_info);
        ss << ", createFlags: " << string_VkAccelerationStructureCreateFlagsKHR(ci_2.createFlags) << ", addressRange.address: 0x"
           << std::hex << ci_2.addressRange.address << ", addressRange.size: " << std::dec << ci_2.addressRange.size
           << ", addressFlags: " << string_VkAddressCommandFlagsKHR(ci_2.addressFlags)
           << ", type: " << string_VkAccelerationStructureTypeKHR(ci_2.type);
    }

    return ss.str();
}

}  // namespace vvl
