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
#include "error_message/error_strings.h"
#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"

namespace gpudump {

// return for warning if no buffer found
static bool ListBuffers(std::ostringstream& ss, const GpuDump& dev_data, VkDeviceAddress address, uint32_t indents) {
    auto buffer_states = dev_data.GetBuffersByAddress(address);
    for (uint32_t i = 0; i < indents; i++) {
        ss << "    ";
    }
    for (auto& buffer_state : buffer_states) {
        ss << "- " << buffer_state->Describe(dev_data) << "\n";
    }
    if (buffer_states.empty()) {
        ss << "- No VkBuffer found at 0x" << std::hex << address << "\n";
        return true;
    }
    return false;
}

bool CommandBufferSubState::DumpCopyMemoryIndirectCommon(std::ostringstream& ss, uint32_t copy_count,
                                                         VkStridedDeviceAddressRangeKHR copy_address_range) const {
    ss << "copyCount = " << copy_count << ", starting address = 0x" << std::hex << copy_address_range.address
       << ", size = " << std::dec << copy_address_range.size << ", stride = " << copy_address_range.stride << "\n";
    return ListBuffers(ss, dev_data, copy_address_range.address, 1);
}

void CommandBufferSubState::DumpCopyMemoryIndirect(const VkCopyMemoryIndirectInfoKHR& info, const Location& loc) const {
    std::ostringstream ss;
    ss << "[Dump Copy Memory Indirect]\n";

    bool warn_no_buffer = DumpCopyMemoryIndirectCommon(ss, info.copyCount, info.copyAddressRange);

    std::vector<uint8_t> indirect_data = dev_data.CopyDataFromMemory(info.copyAddressRange.address, info.copyAddressRange.size);
    const bool know_data = !indirect_data.empty();

    for (uint32_t i = 0; i < info.copyCount; i++) {
        VkDeviceSize stride_i = info.copyAddressRange.stride * i;
        VkDeviceAddress start_address = info.copyAddressRange.address + stride_i;
        vvl::range<VkDeviceAddress> range{start_address, start_address + info.copyAddressRange.size};
        ss << " - Copy [" << std::dec << i << "] (offset " << stride_i << ") VkCopyMemoryIndirectCommandKHR found at "
           << vvl::string_range_hex(range) << '\n';
        if (know_data) {
            auto command = *(VkCopyMemoryIndirectCommandKHR*)(indirect_data.data() + stride_i);
            ss << "    - size: " << std::dec << command.size << '\n';
            ss << "    - srcAddress: 0x" << std::hex << command.srcAddress << '\n';
            warn_no_buffer |= ListBuffers(ss, dev_data, command.srcAddress, 2);
            ss << "    - dstAddress: 0x" << std::hex << command.dstAddress << '\n';
            warn_no_buffer |= ListBuffers(ss, dev_data, command.dstAddress, 2);
        }
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        const VkFlags log_level = warn_no_buffer ? kWarningBit : kInformationBit;
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(log_level, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

void CommandBufferSubState::DumpCopyMemoryToImageIndirect(const VkCopyMemoryToImageIndirectInfoKHR& info,
                                                          const Location& loc) const {
    std::ostringstream ss;
    ss << "[Dump Copy Memory Indirect]\n";

    bool warn_no_buffer = DumpCopyMemoryIndirectCommon(ss, info.copyCount, info.copyAddressRange);

    std::vector<uint8_t> indirect_data = dev_data.CopyDataFromMemory(info.copyAddressRange.address, info.copyAddressRange.size);
    const bool know_data = !indirect_data.empty();

    for (uint32_t i = 0; i < info.copyCount; i++) {
        VkDeviceSize stride_i = info.copyAddressRange.stride * i;
        VkDeviceAddress start_address = info.copyAddressRange.address + stride_i;
        vvl::range<VkDeviceAddress> range{start_address, start_address + info.copyAddressRange.size};
        ss << " - Copy [" << std::dec << i << "] (offset " << stride_i << ") VkCopyMemoryToImageIndirectCommandKHR found at "
           << vvl::string_range_hex(range) << "\n";

        if (know_data) {
            auto command = *(VkCopyMemoryToImageIndirectCommandKHR*)(indirect_data.data() + stride_i);
            ss << "    - srcAddress: 0x" << std::hex << command.srcAddress << '\n';
            warn_no_buffer |= ListBuffers(ss, dev_data, command.srcAddress, 2);
            ss << "    - bufferRowLength: " << std::dec << command.bufferRowLength << '\n';
            ss << "    - bufferImageHeight: " << command.bufferImageHeight << '\n';
            ss << "    - imageSubresource: " << string_VkImageSubresourceLayers(command.imageSubresource) << '\n';
            ss << "    - imageOffset: " << string_VkOffset3D(command.imageOffset) << '\n';
            ss << "    - imageExtent: " << string_VkExtent3D(command.imageExtent) << '\n';
        }
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        const VkFlags log_level = warn_no_buffer ? kWarningBit : kInformationBit;
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(log_level, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
