/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Michael Lentine <mlentine@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chia-I Wu <olv@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Jeremy Hayes <jeremy@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Mike Schuchardt <mikes@lunarg.com>
 * Author: Mike Weiblen <mikew@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 */

// Allow use of STL min and max functions in Windows
#define NOMINMAX

#include "chassis.h"
#include "core_validation.h"

static inline void UpdateResourceTrackingOnDraw(GLOBAL_CB_NODE *pCB) { pCB->draw_data.push_back(pCB->current_draw_data); }

// Generic function to handle validation for all CmdDraw* type functions
bool CoreChecks::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type,
                                     const char *caller, VkQueueFlags queue_flags, const char *queue_flag_code,
                                     const char *renderpass_msg_code, const char *pipebound_msg_code,
                                     const char *dynamic_state_msg_code) {
    bool skip = false;
    GLOBAL_CB_NODE *cb_state = GetCBNode(cmd_buffer);
    if (cb_state) {
        skip |= ValidateCmdQueueFlags(cb_state, caller, queue_flags, queue_flag_code);
        skip |= ValidateCmd(cb_state, cmd_type, caller);
        skip |=
            ValidateCmdBufDrawState(cb_state, cmd_type, indexed, bind_point, caller, pipebound_msg_code, dynamic_state_msg_code);
        skip |= (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) ? OutsideRenderPass(cb_state, caller, renderpass_msg_code)
                                                                : InsideRenderPass(cb_state, caller, renderpass_msg_code);
    }
    return skip;
}

// Generic function to handle state update for all CmdDraw* and CmdDispatch* type functions
void CoreChecks::UpdateStateCmdDrawDispatchType(GLOBAL_CB_NODE *cb_state, VkPipelineBindPoint bind_point) {
    UpdateDrawState(cb_state, bind_point);
}

// Generic function to handle state update for all CmdDraw* type functions
void CoreChecks::UpdateStateCmdDrawType(GLOBAL_CB_NODE *cb_state, VkPipelineBindPoint bind_point) {
    UpdateStateCmdDrawDispatchType(cb_state, bind_point);
    UpdateResourceTrackingOnDraw(cb_state);
    cb_state->hasDrawCmd = true;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance) {
    return ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAW, "vkCmdDraw()",
                               VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDraw-commandBuffer-cmdpool", "VUID-vkCmdDraw-renderpass",
                               "VUID-vkCmdDraw-None-00442", "VUID-vkCmdDraw-None-00443");
}

void CoreChecks::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                      uint32_t firstVertex, uint32_t firstInstance) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void CoreChecks::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                       uint32_t firstVertex, uint32_t firstInstance) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXED, "vkCmdDrawIndexed()",
                                    VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexed-commandBuffer-cmdpool",
                                    "VUID-vkCmdDrawIndexed-renderpass", "VUID-vkCmdDrawIndexed-None-00461",
                                    "VUID-vkCmdDrawIndexed-None-00462");
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    if (!skip && (cb_state->status & CBSTATUS_INDEX_BUFFER_BOUND)) {
        unsigned int index_size = 0;
        const auto &index_buffer_binding = cb_state->index_buffer_binding;
        if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT16) {
            index_size = 2;
        } else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT32) {
            index_size = 4;
        }
        VkDeviceSize end_offset = (index_size * ((VkDeviceSize)firstIndex + indexCount)) + index_buffer_binding.offset;
        if (end_offset > index_buffer_binding.size) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(index_buffer_binding.buffer), "VUID-vkCmdDrawIndexed-indexSize-00463",
                            "vkCmdDrawIndexed() index size (%d) * (firstIndex (%d) + indexCount (%d)) "
                            "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                            " bytes, "
                            "which is greater than the index buffer size (%" PRIuLEAST64 ").",
                            index_size, firstIndex, indexCount, index_buffer_binding.offset, end_offset, index_buffer_binding.size);
        }
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                             uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void CoreChecks::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                              uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                                uint32_t stride) {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECT, "vkCmdDrawIndirect()",
                                    VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirect-commandBuffer-cmdpool",
                                    "VUID-vkCmdDrawIndirect-renderpass", "VUID-vkCmdDrawIndirect-None-00485",
                                    "VUID-vkCmdDrawIndirect-None-00486");
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndirect()", "VUID-vkCmdDrawIndirect-buffer-00474");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, "VUID-vkCmdDrawIndirect-buffer-01660",
                                     "vkCmdDrawIndirect()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void CoreChecks::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t count, uint32_t stride) {
    bool skip = ValidateCmdDrawType(
        commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECT, "vkCmdDrawIndexedIndirect()",
        VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-cmdpool", "VUID-vkCmdDrawIndexedIndirect-renderpass",
        "VUID-vkCmdDrawIndexedIndirect-None-00537", "VUID-vkCmdDrawIndexedIndirect-None-00538");
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndexedIndirect()", "VUID-vkCmdDrawIndexedIndirect-buffer-00526");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirect-buffer-01665", "vkCmdDrawIndexedIndirect()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t count, uint32_t stride) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void CoreChecks::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t count, uint32_t stride) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    bool skip = false;
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    if (cb_state) {
        skip |= ValidateComputeWorkGroupInvocations(cb_state, x, y, z);
    }
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCH, "vkCmdDispatch()",
                                VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatch-commandBuffer-cmdpool", "VUID-vkCmdDispatch-renderpass",
                                "VUID-vkCmdDispatch-None-00391", kVUIDUndefined);
    return skip;
}

void CoreChecks::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void CoreChecks::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    bool skip =
        ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHINDIRECT, "vkCmdDispatchIndirect()",
                            VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatchIndirect-commandBuffer-cmdpool",
                            "VUID-vkCmdDispatchIndirect-renderpass", "VUID-vkCmdDispatchIndirect-None-00404", kVUIDUndefined);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDispatchIndirect()", "VUID-vkCmdDispatchIndirect-buffer-00401");
    skip |=
        ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, "VUID-vkCmdDispatchIndirect-buffer-00405",
                                 "vkCmdDispatchIndirect()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void CoreChecks::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void CoreChecks::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) {
    bool skip = false;
    if (offset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndirectCountKHR-offset-03108",
                        "vkCmdDrawIndirectCountKHR() parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                        offset);
    }

    if (countBufferOffset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndirectCountKHR-countBufferOffset-03109",
                        "vkCmdDrawIndirectCountKHR() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        countBufferOffset);
    }

    if ((stride & 3) || stride < sizeof(VkDrawIndirectCommand)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndirectCountKHR-stride-03110",
                        "vkCmdDrawIndirectCountKHR() parameter, uint32_t stride (0x%" PRIxLEAST32
                        "), is not a multiple of 4 or smaller than sizeof (VkDrawIndirectCommand).",
                        stride);
    }

    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTCOUNTKHR,
                                "vkCmdDrawIndirectCountKHR()", VK_QUEUE_GRAPHICS_BIT,
                                "VUID-vkCmdDrawIndirectCountKHR-commandBuffer-cmdpool", "VUID-vkCmdDrawIndirectCountKHR-renderpass",
                                "VUID-vkCmdDrawIndirectCountKHR-None-03119", "VUID-vkCmdDrawIndirectCountKHR-None-03120");
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |=
        ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndirectCountKHR()", "VUID-vkCmdDrawIndirectCountKHR-buffer-03104");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawIndirectCountKHR()",
                                          "VUID-vkCmdDrawIndirectCountKHR-countBuffer-03106");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndirectCountKHR-buffer-03105", "vkCmdDrawIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndirectCountKHR-countBuffer-03107", "vkCmdDrawIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
    AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    bool skip = false;
    if (offset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndexedIndirectCountKHR-offset-03140",
                        "vkCmdDrawIndexedIndirectCountKHR() parameter, VkDeviceSize offset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        offset);
    }

    if (countBufferOffset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndexedIndirectCountKHR-countBufferOffset-03141",
                        "vkCmdDrawIndexedIndirectCountKHR() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        countBufferOffset);
    }

    if ((stride & 3) || stride < sizeof(VkDrawIndexedIndirectCommand)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndexedIndirectCountKHR-stride-03142",
                        "vkCmdDrawIndexedIndirectCountKHR() parameter, uint32_t stride (0x%" PRIxLEAST32
                        "), is not a multiple of 4 or smaller than sizeof (VkDrawIndexedIndirectCommand).",
                        stride);
    }

    skip |= ValidateCmdDrawType(
        commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECTCOUNTKHR, "vkCmdDrawIndexedIndirectCountKHR()",
        VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirectCountKHR-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndexedIndirectCountKHR-renderpass", "VUID-vkCmdDrawIndexedIndirectCountKHR-None-03151",
        "VUID-vkCmdDrawIndexedIndirectCountKHR-None-03152");
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndexedIndirectCountKHR()",
                                          "VUID-vkCmdDrawIndexedIndirectCountKHR-buffer-03136");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawIndexedIndirectCountKHR()",
                                          "VUID-vkCmdDrawIndexedIndirectCountKHR-countBuffer-03138");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirectCountKHR-buffer-03137", "vkCmdDrawIndexedIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirectCountKHR-countBuffer-03139",
                                     "vkCmdDrawIndexedIndirectCountKHR()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
    AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSNV,
                                    "vkCmdDrawMeshTasksNV()", VK_QUEUE_GRAPHICS_BIT,
                                    "VUID-vkCmdDrawMeshTasksNV-commandBuffer-cmdpool", "VUID-vkCmdDrawMeshTasksNV-renderpass",
                                    "VUID-vkCmdDrawMeshTasksNV-None-02125", "VUID-vkCmdDrawMeshTasksNV-None-02126");
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTNV,
                                    "vkCmdDrawMeshTasksIndirectNV()", VK_QUEUE_GRAPHICS_BIT,
                                    "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-cmdpool",
                                    "VUID-vkCmdDrawMeshTasksIndirectNV-renderpass", "VUID-vkCmdDrawMeshTasksIndirectNV-None-02154",
                                    "VUID-vkCmdDrawMeshTasksIndirectNV-None-02155");
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawMeshTasksIndirectNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02143");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02144", "vkCmdDrawMeshTasksIndirectNV()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        AddCommandBufferBindingBuffer(cb_state, buffer_state);
    }
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    bool skip = ValidateCmdDrawType(
        commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTCOUNTNV,
        "vkCmdDrawMeshTasksIndirectCountNV()", VK_QUEUE_GRAPHICS_BIT,
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-cmdpool", "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderpass",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02189", "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02190");
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawMeshTasksIndirectCountNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02176");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawMeshTasksIndirectCountNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02178");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02177", "vkCmdDrawIndexedIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02179",
                                     "vkCmdDrawIndexedIndirectCountKHR()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {
    GLOBAL_CB_NODE *cb_state = GetCBNode(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (buffer_state) {
        AddCommandBufferBindingBuffer(cb_state, buffer_state);
    }
    if (count_buffer_state) {
        AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
    }
}
