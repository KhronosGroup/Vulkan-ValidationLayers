/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
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

#include "vertex_index_buffer_state.h"

#include "error_message/error_strings.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/state_tracker.h"

#include <sstream>

namespace vvl {

VkBuffer VertexBufferBinding::Handle(const vvl::DeviceState& device) const {
    if (address != 0) {
        const auto buffer_states = device.GetBuffersByAddress(address);
        for (const auto buffer_state : buffer_states) {
            // Only return a buffer that makes sense
            if (buffer_state && ((buffer_state->usage & VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT) != 0)) {
                return buffer_state->VkHandle();
            }
        }
    }
    return buffer;  // acts as "null" if |address| isn't set
}

VkBuffer IndexBufferBinding::Handle(const vvl::DeviceState& device) const {
    if (address != 0) {
        const auto buffer_states = device.GetBuffersByAddress(address);
        for (const auto buffer_state : buffer_states) {
            // Only return a buffer that makes sense
            if (buffer_state && ((buffer_state->usage & VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT) != 0)) {
                return buffer_state->VkHandle();
            }
        }
    }
    return buffer;  // acts as "null" if |address| isn't set
}

std::string VertexBufferBinding::String(const vvl::DeviceState& device) const {
    std::stringstream ss;
    ss << "Vertex buffer binding info:\n";
    if (buffer) {
        ss << "- Buffer: " << device.FormatHandle(buffer) << '\n';
        ss << "- Binding offset: " << offset << '\n';
    } else {
        ss << "- Address: 0x" << std::hex << address << std::dec << '\n';
        std::string buffers_list_str = string_BuffersFromAddress(device, address);
        if (!buffers_list_str.empty()) {
            ss << "Pertains to the following buffers:\n" << buffers_list_str << '\n';
        }
    }
    ss << "- Effective size: " << effective_size << " bytes\n";
    ss << "- Stride: " << stride << " bytes\n";

    return ss.str();
}

std::string IndexBufferBinding::String(const vvl::DeviceState& device) const {
    std::stringstream ss;
    ss << "Index buffer binding info:\n";
    ss << "- Index type: " << string_VkIndexType(index_type) << '\n';
    if (buffer) {
        ss << "- Buffer: " << device.FormatHandle(buffer) << '\n';
        ss << "- Binding offset: " << offset << '\n';
    } else {
        ss << "- Address: 0x" << std::hex << address << std::dec << '\n';
        std::string buffers_list_str = string_BuffersFromAddress(device, address);
        if (!buffers_list_str.empty()) {
            ss << "Pertains to the following buffers:\n" << buffers_list_str << '\n';
        }
    }
    ss << "- Binding size: " << size << " bytes\n";

    return ss.str();
}

}  // namespace vvl
