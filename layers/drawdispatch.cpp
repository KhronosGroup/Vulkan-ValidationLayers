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

static inline void UpdateResourceTrackingOnDraw(CMD_BUFFER_STATE *pCB) {
    pCB->cb_vertex_buffer_binding_info.push_back(pCB->current_vertex_buffer_binding_info);
}

// Generic function to handle validation for all CmdDraw* type functions
bool CoreChecks::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type,
                                     const char *caller, VkQueueFlags queue_flags, const char *queue_flag_code,
                                     const char *renderpass_msg_code, const char *pipebound_msg_code,
                                     const char *dynamic_state_msg_code) const {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(cmd_buffer);
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
void ValidationStateTracker::UpdateStateCmdDrawDispatchType(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point) {
    UpdateDrawState(cb_state, bind_point);
    cb_state->hasDispatchCmd = true;
}

// Generic function to handle state update for all CmdDraw* type functions
void ValidationStateTracker::UpdateStateCmdDrawType(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point) {
    UpdateStateCmdDrawDispatchType(cb_state, bind_point);
    UpdateResourceTrackingOnDraw(cb_state);
    cb_state->hasDrawCmd = true;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance) {
    return ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAW, "vkCmdDraw()",
                               VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDraw-commandBuffer-cmdpool", "VUID-vkCmdDraw-renderpass",
                               "VUID-vkCmdDraw-None-02700", "VUID-vkCmdDraw-commandBuffer-02701");
}

void CoreChecks::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                      uint32_t firstVertex, uint32_t firstInstance) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                   uint32_t firstVertex, uint32_t firstInstance) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXED, "vkCmdDrawIndexed()",
                                    VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexed-commandBuffer-cmdpool",
                                    "VUID-vkCmdDrawIndexed-renderpass", "VUID-vkCmdDrawIndexed-None-02700",
                                    "VUID-vkCmdDrawIndexed-commandBuffer-02701");
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!skip && (cb_state->status & CBSTATUS_INDEX_BUFFER_BOUND)) {
        unsigned int index_size = 0;
        const auto &index_buffer_binding = cb_state->index_buffer_binding;
        if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT16) {
            index_size = 2;
        } else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT32) {
            index_size = 4;
        } else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT8_EXT) {
            index_size = 1;
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

void ValidationStateTracker::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                                uint32_t stride) {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECT, "vkCmdDrawIndirect()",
                                    VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirect-commandBuffer-cmdpool",
                                    "VUID-vkCmdDrawIndirect-renderpass", "VUID-vkCmdDrawIndirect-None-02700",
                                    "VUID-vkCmdDrawIndirect-commandBuffer-02701");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndirect()", "VUID-vkCmdDrawIndirect-buffer-02708");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, "VUID-vkCmdDrawIndirect-buffer-02709",
                                     "vkCmdDrawIndirect()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (count > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand));
        skip |=
            ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                            "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), count, offset, buffer_state);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t count, uint32_t stride) {
    bool skip = ValidateCmdDrawType(
        commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECT, "vkCmdDrawIndexedIndirect()",
        VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-cmdpool", "VUID-vkCmdDrawIndexedIndirect-renderpass",
        "VUID-vkCmdDrawIndexedIndirect-None-02700", "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02701");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndexedIndirect()", "VUID-vkCmdDrawIndexedIndirect-buffer-02708");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirect-buffer-02709", "vkCmdDrawIndexedIndirect()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (count > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), count, offset,
                                                buffer_state);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

void CoreChecks::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t count, uint32_t stride) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t count, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    bool skip = false;
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCH, "vkCmdDispatch()",
                                VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatch-commandBuffer-cmdpool", "VUID-vkCmdDispatch-renderpass",
                                "VUID-vkCmdDispatch-None-02700", kVUIDUndefined);
    return skip;
}

void CoreChecks::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void ValidationStateTracker::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    bool skip =
        ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHINDIRECT, "vkCmdDispatchIndirect()",
                            VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatchIndirect-commandBuffer-cmdpool",
                            "VUID-vkCmdDispatchIndirect-renderpass", "VUID-vkCmdDispatchIndirect-None-02700", kVUIDUndefined);
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDispatchIndirect()", "VUID-vkCmdDispatchIndirect-buffer-02708");
    skip |=
        ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, "VUID-vkCmdDispatchIndirect-buffer-02709",
                                 "vkCmdDispatchIndirect()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void CoreChecks::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void ValidationStateTracker::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                               VkDeviceSize offset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndirectCountKHR-offset-02710",
                        "vkCmdDrawIndirectCountKHR() parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                        offset);
    }

    if (countBufferOffset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndirectCountKHR-countBufferOffset-02716",
                        "vkCmdDrawIndirectCountKHR() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        countBufferOffset);
    }
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirectCountKHR-stride-03110", stride,
                                            "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand));
    if (maxDrawCount > 1) {
        const BUFFER_STATE *buffer_state = GetBufferState(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirectCountKHR-maxDrawCount-03111", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), maxDrawCount, offset,
                                                buffer_state);
    }

    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTCOUNTKHR,
                                "vkCmdDrawIndirectCountKHR()", VK_QUEUE_GRAPHICS_BIT,
                                "VUID-vkCmdDrawIndirectCountKHR-commandBuffer-cmdpool", "VUID-vkCmdDrawIndirectCountKHR-renderpass",
                                "VUID-vkCmdDrawIndirectCountKHR-None-02700", "VUID-vkCmdDrawIndirectCountKHR-commandBuffer-02701");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |=
        ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndirectCountKHR()", "VUID-vkCmdDrawIndirectCountKHR-buffer-02708");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawIndirectCountKHR()",
                                          "VUID-vkCmdDrawIndirectCountKHR-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndirectCountKHR-buffer-02709", "vkCmdDrawIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndirectCountKHR-countBuffer-02715", "vkCmdDrawIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void ValidationStateTracker::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndexedIndirectCountKHR-offset-02710",
                        "vkCmdDrawIndexedIndirectCountKHR() parameter, VkDeviceSize offset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        offset);
    }

    if (countBufferOffset & 3) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdDrawIndexedIndirectCountKHR-countBufferOffset-02716",
                        "vkCmdDrawIndexedIndirectCountKHR() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                        "), is not a multiple of 4.",
                        countBufferOffset);
    }

    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCountKHR-stride-03142", stride,
                                            "VkDrawIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
    if (maxDrawCount > 1) {
        const BUFFER_STATE *buffer_state = GetBufferState(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCountKHR-maxDrawCount-03143", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), maxDrawCount, offset,
                                                buffer_state);
    }

    skip |= ValidateCmdDrawType(
        commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECTCOUNTKHR, "vkCmdDrawIndexedIndirectCountKHR()",
        VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirectCountKHR-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndexedIndirectCountKHR-renderpass", "VUID-vkCmdDrawIndexedIndirectCountKHR-None-02700",
        "VUID-vkCmdDrawIndexedIndirectCountKHR-commandBuffer-02701");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawIndexedIndirectCountKHR()",
                                          "VUID-vkCmdDrawIndexedIndirectCountKHR-buffer-02708");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawIndexedIndirectCountKHR()",
                                          "VUID-vkCmdDrawIndexedIndirectCountKHR-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirectCountKHR-buffer-02709", "vkCmdDrawIndexedIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirectCountKHR-countBuffer-02715",
                                     "vkCmdDrawIndexedIndirectCountKHR()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

void ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                         VkDeviceSize offset, VkBuffer countBuffer,
                                                                         VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                         uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
    AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
}

void CoreChecks::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                             VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                             VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                             VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                             VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                             VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                             uint32_t width, uint32_t height, uint32_t depth) {
    GpuAllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
}

void CoreChecks::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
    cb_state->hasTraceRaysCmd = true;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSNV,
                                    "vkCmdDrawMeshTasksNV()", VK_QUEUE_GRAPHICS_BIT,
                                    "VUID-vkCmdDrawMeshTasksNV-commandBuffer-cmdpool", "VUID-vkCmdDrawMeshTasksNV-renderpass",
                                    "VUID-vkCmdDrawMeshTasksNV-None-02700", "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02701");
    return skip;
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                             uint32_t firstTask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTNV,
                                    "vkCmdDrawMeshTasksIndirectNV()", VK_QUEUE_GRAPHICS_BIT,
                                    "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-cmdpool",
                                    "VUID-vkCmdDrawMeshTasksIndirectNV-renderpass", "VUID-vkCmdDrawMeshTasksIndirectNV-None-02700",
                                    "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02701");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawMeshTasksIndirectNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02709", "vkCmdDrawMeshTasksIndirectNV()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                drawCount, offset, buffer_state);
    }
    return skip;
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02700", "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02701");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdDrawMeshTasksIndirectCountNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02708");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawMeshTasksIndirectCountNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02709", "vkCmdDrawIndexedIndirectCountKHR()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02715",
                                     "vkCmdDrawIndexedIndirectCountKHR()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                            "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV));
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, buffer_state);
    }
    return skip;
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                          VkDeviceSize offset, VkBuffer countBuffer,
                                                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                          uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
