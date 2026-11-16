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
#pragma once

#include <vulkan/vulkan_core.h>
#include "utils/math_utils.h"

namespace vvl {

// How many bytes the token reads from the indirect buffer for each sequence (VK_EXT_device_generated_commands)
inline uint32_t IndirectCommandsTokenDataSize(const VkIndirectCommandsLayoutTokenEXT& token) {
    switch (token.type) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
            // Shader objects have an index for each stage
            if (token.data.pExecutionSet->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {
                return sizeof(uint32_t) * CountSetBits(token.data.pExecutionSet->shaderStages);
            }
            return sizeof(uint32_t);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_EXT:
            return token.data.pPushConstant->updateRange.size;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_SEQUENCE_INDEX_EXT:
            return 0;  // value comes from the sequence index, nothing is read from the indirect buffer
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT:
            return sizeof(VkBindIndexBufferIndirectCommandEXT);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
            return sizeof(VkBindVertexBufferIndirectCommandEXT);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT:
            return sizeof(VkDrawIndexedIndirectCommand);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT:
            return sizeof(VkDrawIndirectCommand);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT:
            return sizeof(VkDrawIndirectCountIndirectCommandEXT);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT:
            return sizeof(VkDispatchIndirectCommand);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT:
            return sizeof(VkDrawMeshTasksIndirectCommandNV);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT:
            return sizeof(VkDrawMeshTasksIndirectCommandEXT);
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT:
            return sizeof(VkTraceRaysIndirectCommand2KHR);
        default:
            break;
    }
    return 0;
}

}  // namespace vvl
