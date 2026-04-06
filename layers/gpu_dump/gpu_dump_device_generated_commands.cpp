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
#include "state_tracker/device_generated_commands_state.h"
#include "state_tracker/last_bound_state.h"

namespace gpudump {

void CommandBufferSubState::DumpDeviceGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                                        const Location& loc) const {
    std::ostringstream ss;
    ss << "[Dump Device Generated Commands]\n";

    const LastBound& last_bound = base.lastBound[ConvertToVvlBindPoint(bind_point)];
    const bool has_pipeline = last_bound.pipeline_state != nullptr;

    ss << "maxDrawCount = " << info.maxDrawCount << ", maxSequenceCount = " << info.maxSequenceCount << "\n";
    if (info.sequenceCountAddress != 0) {
        vvl::range<VkDeviceAddress> sequence_range{info.sequenceCountAddress, info.sequenceCountAddress + sizeof(uint32_t)};
        ss << "Sequence count found at " << vvl::string_range_hex(sequence_range) << "\n";
        auto buffer_states = dev_data.GetBuffersByAddress(info.sequenceCountAddress);
        for (auto& buffer_state : buffer_states) {
            ss << "    - " << buffer_state->Describe(dev_data) << "\n";
        }
        if (buffer_states.empty()) {
            ss << "    - No VkBuffer found at 0x" << std::hex << info.sequenceCountAddress << "\n";
        }
    }

    if (info.preprocessAddress != 0) {
        vvl::range<VkDeviceAddress> preprocess_range{info.preprocessAddress, info.preprocessAddress + info.preprocessSize};
        ss << "Preprocess at " << vvl::string_range_hex(preprocess_range) << "\n";
        auto buffer_states = dev_data.GetBuffersByAddress(info.preprocessAddress);
        for (auto& buffer_state : buffer_states) {
            ss << "    - " << buffer_state->Describe(dev_data) << "\n";
        }
        if (buffer_states.empty()) {
            ss << "    - No VkBuffer found at 0x" << std::hex << info.preprocessAddress << "\n";
        }
    }

    {
        vvl::range<VkDeviceAddress> indirect_range{info.indirectAddress, info.indirectAddress + info.indirectAddressSize};
        ss << "Indirect buffer at " << vvl::string_range_hex(indirect_range) << "\n";
        auto buffer_states = dev_data.GetBuffersByAddress(info.indirectAddress);
        for (auto& buffer_state : buffer_states) {
            ss << "    - " << buffer_state->Describe(dev_data) << "\n";
        }
        if (buffer_states.empty()) {
            ss << "    - No VkBuffer found at 0x" << std::hex << info.indirectAddress << "\n";
        }
    }

    const auto indirect_commands_layout = dev_data.Get<vvl::IndirectCommandsLayout>(info.indirectCommandsLayout);
    ss << "Tokens:\n";
    for (uint32_t i = 0; i < indirect_commands_layout->create_info.tokenCount; i++) {
        const VkIndirectCommandsLayoutTokenEXT& token = indirect_commands_layout->create_info.pTokens[i];
        ss << " - Token [" << i << "] ";
        switch (token.type) {
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
                ss << "[EXECUTION_SET] " << (has_pipeline ? "pipeline" : "shader object") << ", stage "
                   << string_VkShaderStageFlags(token.data.pExecutionSet->shaderStages);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
                ss << "[PUSH_CONSTANT] " << string_VkPushConstantRange(token.data.pPushConstant->updateRange);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT:
                ss << "[SEQUENCE_INDEX] " << string_VkPushConstantRange(token.data.pPushConstant->updateRange);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT:
                ss << "[INDEX_BUFFER] mode = " << string_VkIndirectCommandsInputModeFlagBitsEXT(token.data.pIndexBuffer->mode);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
                ss << "[VERTEX_BUFFER] vertexBindingUnit = " << token.data.pVertexBuffer->vertexBindingUnit;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT:
                ss << "[DRAW_INDEXED] VkDrawIndexedIndirectCommand";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT:
                ss << "[DRAW] VkDrawIndirectCommand";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT:
                ss << "[DRAW_INDEXED_COUNT] VkDrawIndirectCountIndirectCommandEXT";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT:
                ss << "[DRAW_COUNT] VkDrawIndirectCountIndirectCommandEXT";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT:
                ss << "[DISPATCH] VkDispatchIndirectCommand";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_EXT:
                ss << "[PUSH_DATA] " << string_VkPushConstantRange(token.data.pPushConstant->updateRange);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_SEQUENCE_INDEX_EXT:
                ss << "[PUSH_DATA_SEQUENCE_INDEX] " << string_VkPushConstantRange(token.data.pPushConstant->updateRange);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT:
                ss << "[DRAW_MESH_TASKS_NV] VkDrawMeshTasksIndirectCommandNV";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT:
                ss << "[DRAW_MESH_TASKS_COUNT_NV] VkDrawIndirectCountIndirectCommandEXT";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT:
                ss << "[DRAW_MESH_TASKS] VkDrawMeshTasksIndirectCommandEXT";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT:
                ss << "[DRAW_MESH_TASKS_COUNT] VkDrawIndirectCountIndirectCommandEXT";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT:
                ss << "[TRACE_RAYS2] VkTraceRaysIndirectCommand2KHR";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_MAX_ENUM_EXT:
                break;
        }
        ss << " (offset " << std::dec << token.offset << ")\n";
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(kInformationBit, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
