/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

bool BestPractices::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;

    if (pCreateInfo->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
        skip |= LogPerformanceWarning(kVUID_BestPractices_CreateCommandPool_CommandBufferReset, device,
                                      error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
                                      "VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT is set. Consider resetting entire "
                                      "pool instead.");
    }

    return skip;
}

bool BestPractices::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                          VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const {
    bool skip = false;

    auto cp_state = Get<vvl::CommandPool>(pAllocateInfo->commandPool);
    if (!cp_state) return false;

    const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[cp_state->queueFamilyIndex].queueFlags;
    const VkQueueFlags sec_cmd_buf_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    if (pAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY && (queue_flags & sec_cmd_buf_queue_flags) == 0) {
        skip |= LogWarning(kVUID_BestPractices_AllocateCommandBuffers_UnusableSecondary, device, error_obj.location,
                           "Allocating secondary level command buffer from command pool "
                           "created against queue family #%u (queue flags: %s), but vkCmdExecuteCommands() is only "
                           "supported on queue families supporting VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, or "
                           "VK_QUEUE_TRANSFER_BIT. The allocated command buffer will not be submittable.",
                           cp_state->queueFamilyIndex, string_VkQueueFlags(queue_flags).c_str());
    }

    return skip;
}

void BestPractices::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                    const RecordObject& record_obj) {
    StateTracker::PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);

    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    cb_state->num_submits = 0;
}

bool BestPractices::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;

    if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
        skip |= LogPerformanceWarning(kVUID_BestPractices_BeginCommandBuffer_SimultaneousUse, device,
                                      error_obj.location.dot(Field::pBeginInfo).dot(Field::flags),
                                      "(%s) has VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT.",
                                      string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str());
    }

    const bool is_one_time_submit = (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) != 0;
    if (VendorCheckEnabled(kBPVendorArm)) {
        if (!is_one_time_submit) {
            skip |=
                LogPerformanceWarning(kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit, device,
                                      error_obj.location.dot(Field::pBeginInfo).dot(Field::flags),
                                      "(%s) doesn't have VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set. %s For best performance "
                                      "on Mali GPUs, consider setting ONE_TIME_SUBMIT by default.",
                                      string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str(), VendorSpecificTag(kBPVendorArm));
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto cb_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        if (cb_state->num_submits == 1 && !is_one_time_submit) {
            skip |= LogPerformanceWarning(kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit, device,
                                          error_obj.location.dot(Field::pBeginInfo).dot(Field::flags),
                                          "(%s) doesn't have VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set "
                                          "and the command buffer has only been submitted once. %s "
                                          "For best performance on NVIDIA GPUs, use ONE_TIME_SUBMIT.",
                                          string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str(),
                                          VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                     VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags(error_obj.location.dot(Field::pipelineStage), static_cast<VkPipelineStageFlags>(pipelineStage));

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                         VkQueryPool queryPool, uint32_t query,
                                                         const ErrorObject& error_obj) const {
    return PreCallValidateCmdWriteTimestamp2(commandBuffer, pipelineStage, queryPool, query, error_obj);
}

bool BestPractices::PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                                      VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags(error_obj.location.dot(Field::pipelineStage), pipelineStage);

    return skip;
}

bool BestPractices::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                       VkQueryResultFlags flags, const ErrorObject& error_obj) const {
    bool skip = false;

    const auto& query_pool_state = *Get<vvl::QueryPool>(queryPool);

    for (uint32_t i = firstQuery; i < firstQuery + queryCount; ++i) {
        if (query_pool_state.GetQueryState(i, 0u) == QUERYSTATE_RESET) {
            const LogObjectList objlist(queryPool);
            skip |= LogWarning(kVUID_BestPractices_QueryPool_Unavailable, objlist, error_obj.location,
                               "QueryPool %s and query %" PRIu32 ": vkCmdBeginQuery() was never called.",
                               FormatHandle(query_pool_state.pool()).c_str(), i);
            break;
        }
    }

    return skip;
}

void BestPractices::PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                      const RecordObject& record_obj) {
    StateTracker::PreCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);

    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cb_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(*cb_state, depthCompareOp, cb_state->nv.depth_test_enable);
    }
}

void BestPractices::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                         const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void BestPractices::PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                       const RecordObject& record_obj) {
    StateTracker::PreCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);

    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cb_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(*cb_state, cb_state->nv.depth_compare_op, depthTestEnable != VK_FALSE);
    }
}

void BestPractices::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                          const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

bool BestPractices::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const {
    bool skip = false;
    const auto primary = GetRead<bp_state::CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        const auto secondary_cb = GetRead<bp_state::CommandBuffer>(pCommandBuffers[i]);
        if (secondary_cb == nullptr) {
            continue;
        }
        const Location& cb_loc = error_obj.location.dot(Field::pCommandBuffers, i);
        const auto& secondary = secondary_cb->render_pass_state;
        for (auto& clear : secondary.earlyClearAttachments) {
            if (ClearAttachmentsIsFullClear(*primary, uint32_t(clear.rects.size()), clear.rects.data())) {
                skip |=
                    ValidateClearAttachment(*primary, clear.framebufferAttachment, clear.colorAttachment, clear.aspects, cb_loc);
            }
        }

        if (!(secondary_cb->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (primary->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // Warn that non-simultaneous secondary cmd buffer renders primary non-simultaneous
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogWarning(kVUID_BestPractices_DrawState_InvalidCommandBufferSimultaneousUse, objlist, cb_loc,
                                   "(%s) does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set and will cause primary "
                                   "(%s) to be treated as if it does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set, even though it does.",
                                   FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(commandBuffer).c_str());
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (commandBufferCount > 0) {
            skip |=
                LogPerformanceWarning(kVUID_BestPractices_CmdBuffer_AvoidSecondaryCmdBuffers, device, error_obj.location,
                                      "%s Use of secondary command buffers is not recommended.", VendorSpecificTag(kBPVendorAMD));
        }
    }
    return skip;
}

void BestPractices::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    ValidationStateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, record_obj);

    auto primary = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (!primary) {
        return;
    }

    for (uint32_t i = 0; i < commandBufferCount; i++) {
        auto secondary = GetWrite<bp_state::CommandBuffer>(pCommandBuffers[i]);
        if (!secondary) {
            continue;
        }

        for (auto& early_clear : secondary->render_pass_state.earlyClearAttachments) {
            if (ClearAttachmentsIsFullClear(*primary, uint32_t(early_clear.rects.size()), early_clear.rects.data())) {
                RecordAttachmentClearAttachments(*primary, early_clear.framebufferAttachment, early_clear.colorAttachment,
                                                 early_clear.aspects, uint32_t(early_clear.rects.size()), early_clear.rects.data());
            } else {
                RecordAttachmentAccess(*primary, early_clear.framebufferAttachment, early_clear.aspects);
            }
        }

        for (auto& touch : secondary->render_pass_state.touchesAttachments) {
            RecordAttachmentAccess(*primary, touch.framebufferAttachment, touch.aspects);
        }

        primary->render_pass_state.numDrawCallsDepthEqualCompare += secondary->render_pass_state.numDrawCallsDepthEqualCompare;
        primary->render_pass_state.numDrawCallsDepthOnly += secondary->render_pass_state.numDrawCallsDepthOnly;
    }
}
