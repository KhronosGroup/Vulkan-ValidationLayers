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
#include "state_tracker/buffer_state.h"
#include "state_tracker/device_memory_state.h"

namespace gpudump {

GpuDump::GpuDump(vvl::DispatchDevice* dev, gpudump::Instance* instance_vo)
    : vvl::DeviceProxy(dev, instance_vo, LayerObjectTypeGpuDump) {}

GpuDump::~GpuDump() {}

std::vector<uint8_t> GpuDump::CopyDataFromMemory(VkDeviceAddress memory_addresss, VkDeviceSize copy_size) {
    std::vector<uint8_t> result;
    vvl::span<vvl::Buffer* const> buffer_list = device_state->GetBuffersByAddress(memory_addresss);
    if (buffer_list.empty()) {
        return result;
    }

    auto buffer_state = *buffer_list.begin();
    const vvl::DeviceMemory& memory_state = *buffer_state->MemoryState();

    VkDeviceSize offset = memory_addresss - buffer_state->DeviceAddressRange().begin;
    if (memory_state.mappable) {
        uint8_t* data_ptr = (uint8_t*)memory_state.p_driver_data;
        ;

        if (!memory_state.p_driver_data) {
            DispatchMapMemory(device, memory_state.VkHandle(), offset, copy_size, 0, (void**)&data_ptr);

            // Skip checking if coherent memory or not
            VkMappedMemoryRange memory_range = vku::InitStructHelper();
            memory_range.memory = memory_state.VkHandle();
            memory_range.offset = offset;
            memory_range.size = copy_size;
            DispatchInvalidateMappedMemoryRanges(device, 1, &memory_range);
        }

        data_ptr += offset;
        result.resize(static_cast<uint32_t>(copy_size));
        memcpy(result.data(), data_ptr, static_cast<uint32_t>(copy_size));

        if (!memory_state.p_driver_data) {
            DispatchUnmapMemory(device, memory_state.VkHandle());
        }
    } else {
        // TODO - Handle non-host visible memory
    }

    return result;
}

}  // namespace gpudump
