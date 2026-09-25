/* Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "gpu_dump.h"
#include <cstdint>
#include "chassis/layer_object_id.h"
#include "generated/dispatch_functions.h"
#include "generated/vk_extension_helper.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/device_memory_state.h"
#include "state_tracker/ray_tracing_state.h"

namespace gpudump {

GpuDump::GpuDump(vvl::DispatchDevice* dev, gpudump::Instance* instance_vo)
    : vvl::DeviceProxy(dev, instance_vo, LayerObjectTypeGpuDump) {}

GpuDump::~GpuDump() {}

std::vector<uint8_t> GpuDump::CopyDataFromMemory(VkDeviceAddress memory_address, VkDeviceSize copy_size) {
    std::vector<uint8_t> result;
    if (copy_size == 0) {
        return result;
    }
    vvl::span<vvl::Buffer* const> buffer_list = device_state->GetBuffersByAddress(memory_address);
    if (buffer_list.empty()) {
        return result;
    }

    const vvl::Buffer& buffer_state = **buffer_list.begin();
    // Sparse buffers don't have a single memory binding
    const vvl::MemoryBinding* binding = buffer_state.Binding();
    if (!binding || !binding->memory_state || binding->memory_state->Destroyed()) {
        return result;
    }
    const vvl::DeviceMemory& memory_state = *binding->memory_state;

    // Prevent copying OOB of a buffer
    if ((memory_address + copy_size) > buffer_state.DeviceAddressRange().end) {
        return result;
    }

    if (!memory_state.mappable) {
        // TODO - Handle non-host visible memory
        // When we add, we need to guard against if trying to read non-aligned data
        //   (which should have a warning already)
        return result;
    }

    // The buffer might not be bound at the start of the VkDeviceMemory
    const VkDeviceSize memory_offset = binding->memory_offset + (memory_address - buffer_state.DeviceAddressRange().begin);

    if (memory_state.p_driver_data) {
        // The application has the memory mapped already, only read if its mapping covers the data
        const vvl::MemRange& mapped_range = memory_state.mapped_range;
        const VkDeviceSize mapped_end = (mapped_range.size == VK_WHOLE_SIZE) ? memory_state.allocate_info.allocationSize
                                                                             : mapped_range.offset + mapped_range.size;
        if (memory_offset < mapped_range.offset || (memory_offset + copy_size) > mapped_end) {
            return result;
        }
        const uint8_t* data_ptr = static_cast<const uint8_t*>(memory_state.p_driver_data) + (memory_offset - mapped_range.offset);
        result.resize(static_cast<size_t>(copy_size));
        memcpy(result.data(), data_ptr, static_cast<size_t>(copy_size));
        return result;
    }

    // Map from an offset aligned to nonCoherentAtomSize so it can also be used for vkInvalidateMappedMemoryRanges.
    // Just use WHOLE_SIZE to avoid issues with partial mappings
    // Example:
    //  The |memory_address| is 0x1001 and |copy_size| is 4, the driver will return back something
    //  aligned to a value like 64, so if the buffer is only 64 bytes, you will now be accessing data over it
    VkDeviceSize map_offset = memory_offset;
    if (memory_state.cache_non_coherent) {
        const VkDeviceSize atom_size = phys_dev_props.limits.nonCoherentAtomSize;
        map_offset = (memory_offset / atom_size) * atom_size;
    }

    void* mapped_data = nullptr;
    if (DispatchMapMemory(device, memory_state.VkHandle(), map_offset, VK_WHOLE_SIZE, 0, &mapped_data) != VK_SUCCESS) {
        return result;
    }

    if (memory_state.cache_non_coherent) {
        VkMappedMemoryRange memory_range = vku::InitStructHelper();
        memory_range.memory = memory_state.VkHandle();
        memory_range.offset = map_offset;
        memory_range.size = VK_WHOLE_SIZE;
        DispatchInvalidateMappedMemoryRanges(device, 1, &memory_range);
    }

    const uint8_t* data_ptr = static_cast<const uint8_t*>(mapped_data) + (memory_offset - map_offset);
    result.resize(static_cast<size_t>(copy_size));
    memcpy(result.data(), data_ptr, static_cast<size_t>(copy_size));

    DispatchUnmapMemory(device, memory_state.VkHandle());

    return result;
}

bool GpuDump::ListBuffers(std::ostringstream& ss, VkDeviceAddress address, uint32_t indents, bool new_line_start) {
    if (new_line_start) {
        ss << "\n";
    }

    auto buffer_states = GetBuffersByAddress(address);
    for (uint32_t i = 0; i < indents; i++) {
        ss << "    ";
    }

    for (uint32_t i = 0; i < buffer_states.size(); i++) {
        if (i != 0) {
            ss << '\n';
            for (uint32_t j = 0; j < indents; j++) {
                ss << "    ";
            }
        }
        auto& buffer_state = buffer_states[i];
        ss << "- " << buffer_state->Describe(*this);
    }

    if (buffer_states.empty()) {
        ss << "- [WARNING] No VkBuffer found at 0x" << std::hex << address;
    }

    if (!new_line_start) {
        ss << "\n";
    }

    return buffer_states.empty();
}

bool GpuDump::ListAccelerationStructures(std::ostringstream& ss, VkDeviceAddress address, uint32_t indents, bool new_line_start) {
    if (new_line_start) {
        ss << "\n";
    }

    auto as_states = GetAccelerationStructuresByAddress(address);

    for (uint32_t i = 0; i < indents; i++) {
        ss << "    ";
    }

    for (uint32_t i = 0; i < as_states.size(); i++) {
        if (i != 0) {
            ss << '\n';
            for (uint32_t j = 0; j < indents; j++) {
                ss << "    ";
            }
        }
        auto& as_state = as_states[i];
        ss << "- " << as_state->Describe(*this);
    }

    if (as_states.empty()) {
        ss << "- [WARNING] No VkAccelerationStructureKHR found at 0x" << std::hex << address;
    }

    if (!new_line_start) {
        ss << "\n";
    }

    return as_states.empty();
}

}  // namespace gpudump
