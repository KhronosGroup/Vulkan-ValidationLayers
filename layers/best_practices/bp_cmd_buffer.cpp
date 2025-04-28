/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include "best_practices/bp_state.h"
#include "state_tracker/device_state.h"

void BestPractices::Created(vvl::CommandBuffer& cb_state) {
    cb_state.SetSubState(container_type, std::make_unique<bp_state::CommandBufferSubState>(cb_state));
}

bool BestPractices::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;

    if (pCreateInfo->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
        skip |= LogPerformanceWarning("BestPractices-vkCreateCommandPool-command-buffer-reset", device,
                                      error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
                                      "has VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT set. Consider resetting entire "
                                      "pool instead.");
    }

    return skip;
}

bool BestPractices::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                          VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const {
    bool skip = false;

    auto cp_state = Get<vvl::CommandPool>(pAllocateInfo->commandPool);
    ASSERT_AND_RETURN_SKIP(cp_state);

    const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[cp_state->queueFamilyIndex].queueFlags;
    const VkQueueFlags sec_cmd_buf_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    if (pAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY && (queue_flags & sec_cmd_buf_queue_flags) == 0) {
        skip |= LogWarning("BestPractices-vkAllocateCommandBuffers-unusable-secondary", device, error_obj.location,
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
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto& sub_state = bp_state::SubState(*cb_state);
    // reset
    sub_state.num_submits = 0;
    sub_state.uses_vertex_buffer = false;
    sub_state.small_indexed_draw_call_count = 0;
}

bool BestPractices::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;

    if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
        skip |= LogPerformanceWarning("BestPractices-vkBeginCommandBuffer-simultaneous-use", device,
                                      error_obj.location.dot(Field::pBeginInfo).dot(Field::flags),
                                      "(%s) has VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT.",
                                      string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str());
    }

    const bool is_one_time_submit = (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) != 0;
    if (VendorCheckEnabled(kBPVendorArm)) {
        if (!is_one_time_submit) {
            skip |=
                LogPerformanceWarning("BestPractices-Arm-vkBeginCommandBuffer-one-time-submit", device,
                                      error_obj.location.dot(Field::pBeginInfo).dot(Field::flags),
                                      "(%s) doesn't have VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set. %s For best performance "
                                      "on Mali GPUs, consider setting ONE_TIME_SUBMIT by default.",
                                      string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str(), VendorSpecificTag(kBPVendorArm));
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
        const auto& sub_state = bp_state::SubState(*cb_state);
        if (sub_state.num_submits == 1 && !is_one_time_submit) {
            skip |= LogPerformanceWarning("BestPractices-NVIDIA-vkBeginCommandBuffer-one-time-submit", device,
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

    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::pipelineStage),
                                    static_cast<VkPipelineStageFlags>(pipelineStage));

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

    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::pipelineStage), pipelineStage);

    return skip;
}

bool BestPractices::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                       VkQueryResultFlags flags, const ErrorObject& error_obj) const {
    bool skip = false;

    const auto query_pool_state = Get<vvl::QueryPool>(queryPool);
    ASSERT_AND_RETURN_SKIP(query_pool_state);

    for (uint32_t i = firstQuery; i < firstQuery + queryCount; ++i) {
        // Some query type can't have a begin call on it (see VUID-vkCmdBeginQuery-queryType-02804)
        const bool can_have_begin =
            !IsValueIn(query_pool_state->create_info.queryType,
                       {VK_QUERY_TYPE_TIMESTAMP, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                        VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,
                        VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
                        VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV});
        if (can_have_begin && query_pool_state->GetQueryState(i, 0u) == QUERYSTATE_RESET) {
            const LogObjectList objlist(queryPool);
            skip |= LogWarning("BestPractices-QueryPool-Unavailable", objlist, error_obj.location,
                               "QueryPool %s and query %" PRIu32 ": vkCmdBeginQuery() was never called.",
                               FormatHandle(query_pool_state->Handle()).c_str(), i);
            break;
        }
    }

    return skip;
}

void BestPractices::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                       const RecordObject& record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto& sub_state = bp_state::SubState(*cb_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestState(sub_state, depthCompareOp, sub_state.nv.depth_test_enable);
    }
}

void BestPractices::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                          const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void BestPractices::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                        const RecordObject& record_obj) {
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
        auto& sub_state = bp_state::SubState(*cb_state);
        RecordSetDepthTestState(sub_state, sub_state.nv.depth_compare_op, depthTestEnable != VK_FALSE);
    }
}

void BestPractices::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                           const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

namespace {
struct EventValidator {
    const Logger& log;
    vvl::unordered_map<VkEvent, bool> signaling_state;

    EventValidator(const Logger& log_) : log(log_) {}

    bool ValidateSecondaryCbSignalingState(const bp_state::CommandBufferSubState& primary_cb,
                                           const bp_state::CommandBufferSubState& secondary_cb, const Location& secondary_cb_loc) {
        bool skip = false;
        for (const auto& [event, signaling_info] : secondary_cb.event_signaling_state) {
            if (signaling_info.first_state_change_is_signal) {
                bool signaled = false;
                if (auto* p_signaled = vvl::Find(signaling_state, event)) {
                    // check local tracking map
                    signaled = *p_signaled;
                } else if (auto* primary_signal_info = vvl::Find(primary_cb.event_signaling_state, event)) {
                    // check parent command buffer
                    signaled = primary_signal_info->signaled;
                }
                if (signaled) {
                    // the most recent state update was signal (signaled == true) and the secondary
                    // command buffer starts with a signal too (first_state_change_is_signal).
                    const LogObjectList objlist(primary_cb.VkHandle(), secondary_cb.VkHandle(), event);
                    skip |= log.LogWarning(
                        "BestPractices-Event-SignalSignaledEvent", objlist, secondary_cb_loc,
                        "%s sets event %s which was already set (in the primary command buffer %s or in the executed secondary "
                        "command buffers). If this is not the desired behavior, the event must be reset before it is set again.",
                        log.FormatHandle(secondary_cb.VkHandle()).c_str(), log.FormatHandle(event).c_str(),
                        log.FormatHandle(primary_cb.VkHandle()).c_str());
                }
            }
            signaling_state[event] = signaling_info.signaled;
        }
        return skip;
    }
};
}  // namespace

bool BestPractices::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const {
    bool skip = false;
    EventValidator event_validator(*this);
    const auto primary = GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto& primary_sub_state = bp_state::SubState(*primary);
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        const auto secondary_cb = GetRead<vvl::CommandBuffer>(pCommandBuffers[i]);
        if (secondary_cb == nullptr) {
            continue;
        }
        const auto& secondary_sub_state = bp_state::SubState(*secondary_cb);
        const Location& cb_loc = error_obj.location.dot(Field::pCommandBuffers, i);
        const auto& secondary = secondary_sub_state.render_pass_state;
        for (auto& clear : secondary.earlyClearAttachments) {
            if (ClearAttachmentsIsFullClear(primary_sub_state, uint32_t(clear.rects.size()), clear.rects.data())) {
                skip |= ValidateClearAttachment(primary_sub_state, clear.framebufferAttachment, clear.colorAttachment,
                                                clear.aspects, cb_loc);
            }
        }

        if (!(secondary_cb->begin_info_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (primary->begin_info_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // Warn that non-simultaneous secondary cmd buffer renders primary non-simultaneous
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogWarning("BestPractices-vkCmdExecuteCommands-CommandBufferSimultaneousUse", objlist, cb_loc,
                                   "(%s) does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set and will cause primary "
                                   "(%s) to be treated as if it does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set, even though it does.",
                                   FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(commandBuffer).c_str());
            }
        }
        skip |= event_validator.ValidateSecondaryCbSignalingState(primary_sub_state, secondary_sub_state, cb_loc);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (commandBufferCount > 0) {
            skip |= LogPerformanceWarning("BestPractices-AMD-VkCommandBuffer-AvoidSecondaryCmdBuffers", commandBuffer,
                                          error_obj.location, "%s Use of secondary command buffers is not recommended.",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }
    return skip;
}

void BestPractices::PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                     const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    auto primary = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!primary) {
        return;
    }
    auto& primary_sub_state = bp_state::SubState(*primary);

    for (uint32_t i = 0; i < commandBufferCount; i++) {
        auto secondary = GetWrite<vvl::CommandBuffer>(pCommandBuffers[i]);
        if (!secondary) {
            continue;
        }
        auto& secondary_sub_state = bp_state::SubState(*secondary);

        for (auto& early_clear : secondary_sub_state.render_pass_state.earlyClearAttachments) {
            if (ClearAttachmentsIsFullClear(primary_sub_state, uint32_t(early_clear.rects.size()), early_clear.rects.data())) {
                RecordAttachmentClearAttachments(primary_sub_state, early_clear.framebufferAttachment, early_clear.colorAttachment,
                                                 early_clear.aspects, uint32_t(early_clear.rects.size()), early_clear.rects.data());
            } else {
                RecordAttachmentAccess(primary_sub_state, early_clear.framebufferAttachment, early_clear.aspects);
            }
        }

        for (auto& touch : secondary_sub_state.render_pass_state.touchesAttachments) {
            RecordAttachmentAccess(primary_sub_state, touch.framebufferAttachment, touch.aspects);
        }

        primary_sub_state.render_pass_state.numDrawCallsDepthEqualCompare +=
            secondary_sub_state.render_pass_state.numDrawCallsDepthEqualCompare;
        primary_sub_state.render_pass_state.numDrawCallsDepthOnly += secondary_sub_state.render_pass_state.numDrawCallsDepthOnly;

        for (const auto& [event, secondary_info] : secondary_sub_state.event_signaling_state) {
            if (auto* primary_info = vvl::Find(primary_sub_state.event_signaling_state, event)) {
                primary_info->signaled = secondary_info.signaled;
            } else {
                primary_sub_state.event_signaling_state.emplace(event, secondary_info);
            }
        }
    }
}
