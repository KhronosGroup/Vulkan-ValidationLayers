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

#include "containers/range.h"
#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <iostream>
#include <sstream>
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"

namespace gpudump {

void CommandBufferSubState::DumpCopyMemoryIndirectCommon(std::ostringstream& ss, uint32_t copy_count,
                                                         VkStridedDeviceAddressRangeKHR copy_address_range) const {
    ss << "copyCount = " << copy_count << ", starting address = 0x" << std::hex << copy_address_range.address
       << ", size = " << std::dec << copy_address_range.size << ", stride = " << copy_address_range.stride << "\n";
    auto buffer_states = dev_data.GetBuffersByAddress(copy_address_range.address);
    for (auto& buffer_state : buffer_states) {
        ss << "    - " << buffer_state->Describe(dev_data) << "\n";
    }
    if (buffer_states.empty()) {
        ss << "    - No VkBuffer found at 0x" << std::hex << copy_address_range.address << "\n";
    }
}

void CommandBufferSubState::DumpCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) const {
    std::ostringstream ss;
    ss << "[Dump Copy Memory Indirect]\n";

    DumpCopyMemoryIndirectCommon(ss, info.copyCount, info.copyAddressRange);

    for (uint32_t i = 0; i < info.copyCount; i++) {
        VkDeviceAddress start_address = info.copyAddressRange.address + (i * info.copyAddressRange.stride);
        vvl::range<VkDeviceAddress> range{start_address, start_address + info.copyAddressRange.size};
        ss << " - Copy [" << i << "] VkCopyMemoryIndirectCommandKHR found at " << vvl::string_range_hex(range) << "\n";
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        // Don't provide a LogObjectList, embed it into the messsage instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(kInformationBit, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

void CommandBufferSubState::DumpCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info,
                                                          const Location& loc) const {
    std::ostringstream ss;
    ss << "[Dump Copy Memory Indirect]\n";

    DumpCopyMemoryIndirectCommon(ss, info.copyCount, info.copyAddressRange);

    for (uint32_t i = 0; i < info.copyCount; i++) {
        VkDeviceAddress start_address = info.copyAddressRange.address + (i * info.copyAddressRange.stride);
        vvl::range<VkDeviceAddress> range{start_address, start_address + info.copyAddressRange.size};
        ss << " - Copy [" << i << "] VkCopyMemoryToImageIndirectCommandKHR found at " << vvl::string_range_hex(range) << "\n";
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        // Don't provide a LogObjectList, embed it into the messsage instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(kInformationBit, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
