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
#include "utils/device_generated_commands_utils.h"

namespace gpudump {

void CommandBufferSubState::DumpDeviceGeneratedCommands(const VkGeneratedCommandsInfoEXT& info, VkPipelineBindPoint bind_point,
                                                        const Location& loc) const {
    const auto indirect_commands_layout = dev_data.Get<vvl::IndirectCommandsLayout>(info.indirectCommandsLayout);
    if (!indirect_commands_layout) {
        return;  // Invalid handle, will be caught by object lifetime validation
    }
    const auto indirect_execution_set = dev_data.Get<vvl::IndirectExecutionSet>(info.indirectExecutionSet);
    if (info.indirectExecutionSet != VK_NULL_HANDLE && !indirect_execution_set) {
        return;  // Invalid handle, will be caught by object lifetime validation
    }

    std::ostringstream ss;
    ss << "[Dump Device Generated Commands] (" << dev_data.FormatHandle(base.Handle()) << ", "
       << dev_data.FormatHandle(indirect_commands_layout->Handle());
    if (indirect_execution_set) {
        ss << ", " << dev_data.FormatHandle(indirect_execution_set->Handle());
    }
    ss << ")\n";

    bool found_warning = false;

    ss << "- shaderStages = " << string_VkShaderStageFlags(info.shaderStages) << " (" << string_VkPipelineBindPoint(bind_point)
       << ")\n";
    ss << "- maxSequenceCount = " << info.maxSequenceCount << ", maxDrawCount = " << info.maxDrawCount << "\n";
    if (info.sequenceCountAddress != 0) {
        vvl::range<VkDeviceAddress> sequence_range{info.sequenceCountAddress, info.sequenceCountAddress + sizeof(uint32_t)};
        ss << "- sequenceCountAddress at " << vvl::string_range_hex(sequence_range) << "\n";
        found_warning |= dev_data.ListBuffers(ss, info.sequenceCountAddress, 1);
    } else {
        ss << "- sequenceCountAddress = 0 (maxSequenceCount is used as the sequence count)\n";
    }

    {
        vvl::range<VkDeviceAddress> indirect_range{info.indirectAddress, info.indirectAddress + info.indirectAddressSize};
        ss << "- indirectAddress at " << vvl::string_range_hex(indirect_range)
           << " (indirectAddressSize = " << info.indirectAddressSize << ")\n";
        found_warning |= dev_data.ListBuffers(ss, info.indirectAddress, 1);
    }

    if (info.preprocessAddress != 0) {
        vvl::range<VkDeviceAddress> preprocess_range{info.preprocessAddress, info.preprocessAddress + info.preprocessSize};
        ss << "- preprocessAddress at " << vvl::string_range_hex(preprocess_range) << " (preprocessSize = " << info.preprocessSize
           << ")\n";
        found_warning |= dev_data.ListBuffers(ss, info.preprocessAddress, 1);
    }

    if (indirect_execution_set) {
        ss << "- " << dev_data.FormatHandle(indirect_execution_set->Handle()) << " contains ";
        if (indirect_execution_set->is_pipeline) {
            ss << "pipelines (maxPipelineCount = " << indirect_execution_set->max_pipeline_count << ")\n";
        } else {
            ss << "shader objects (maxShaderCount = " << indirect_execution_set->max_shader_count << ")\n";
        }
    }

    const VkIndirectCommandsLayoutCreateInfoEXT& layout_ci = indirect_commands_layout->create_info;
    ss << "- " << dev_data.FormatHandle(indirect_commands_layout->Handle()) << " indirectStride = " << layout_ci.indirectStride;
    if (layout_ci.flags != 0) {
        ss << ", flags = " << string_VkIndirectCommandsLayoutUsageFlagsEXT(layout_ci.flags);
    }
    ss << "\n";
    for (uint32_t i = 0; i < layout_ci.tokenCount; i++) {
        const VkIndirectCommandsLayoutTokenEXT& token = layout_ci.pTokens[i];
        ss << "  - Token [" << i << "] ";
        switch (token.type) {
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
                ss << "[EXECUTION_SET] "
                   << (token.data.pExecutionSet->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT ? "pipeline"
                                                                                                           : "shader object")
                   << ", stage " << string_VkShaderStageFlags(token.data.pExecutionSet->shaderStages);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
                ss << "[PUSH_CONSTANT] " << string_VkPushConstantRange(token.data.pPushConstant->updateRange);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT:
                ss << "[SEQUENCE_INDEX] " << string_VkPushConstantRange(token.data.pPushConstant->updateRange);
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT:
                ss << "[INDEX_BUFFER] mode = " << string_VkIndirectCommandsInputModeFlagBitsEXT(token.data.pIndexBuffer->mode)
                   << ", VkBindIndexBufferIndirectCommandEXT";
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
                ss << "[VERTEX_BUFFER] vertexBindingUnit = " << token.data.pVertexBuffer->vertexBindingUnit
                   << ", VkBindVertexBufferIndirectCommandEXT";
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

        const uint32_t data_size = vvl::IndirectCommandsTokenDataSize(token);
        if (data_size == 0) {
            // offset is ignored for these tokens
            ss << " (nothing read from the indirect buffer, the sequence index is used)\n";
        } else {
            ss << " (offset " << token.offset << ", size " << data_size << ")\n";
        }
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        const VkFlags log_level = found_warning ? kWarningBit : kInformationBit;
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(log_level, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
