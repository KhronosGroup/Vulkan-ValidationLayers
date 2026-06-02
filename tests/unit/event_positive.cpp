/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2026 Google, Inc.
 * Modifications Copyright (C) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveEvent : public SyncObjectTest {};

TEST_F(PositiveEvent, EventStageMask) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, EventStageMaskTwoSubmits) {
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer commandBuffer1(*m_device, m_command_pool);
    vkt::CommandBuffer commandBuffer2(*m_device, m_command_pool);
    vkt::Event event(*m_device);

    commandBuffer1.Begin();
    vk::CmdSetEvent(commandBuffer1, event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.End();
    m_default_queue->Submit(commandBuffer1);

    commandBuffer2.Begin();
    vk::CmdWaitEvents(commandBuffer2, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.End();
    m_default_queue->Submit(commandBuffer2);

    m_default_queue->Wait();
}

TEST_F(PositiveEvent, TwoCommandBuffers) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    command_buffer2.End();

    const VkCommandBuffer command_buffers[2] = {command_buffer, command_buffer2};

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers;
    vk::QueueSubmit(*m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveEvent, TwoBatches) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.ResetEvent(event);
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    // This signal goes after the signal from another command buffer and is ignored
    command_buffer2.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);

    command_buffer2.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    command_buffer2.End();

    // Need to use semaphore to guarantee ordering of CmdSetEvents
    vkt::Semaphore semaphore(*m_device);

    VkSubmitInfo submit_infos[2];
    submit_infos[0] = vku::InitStructHelper();
    submit_infos[0].commandBufferCount = 1;
    submit_infos[0].pCommandBuffers = &command_buffer.handle();
    submit_infos[0].signalSemaphoreCount = 1;
    submit_infos[0].pSignalSemaphores = &semaphore.handle();

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    submit_infos[1] = vku::InitStructHelper();
    submit_infos[1].waitSemaphoreCount = 1;
    submit_infos[1].pWaitSemaphores = &semaphore.handle();
    submit_infos[1].pWaitDstStageMask = &wait_stage;
    submit_infos[1].commandBufferCount = 1;
    submit_infos[1].pCommandBuffers = &command_buffer2.handle();

    vk::QueueSubmit(*m_default_queue, 2, submit_infos, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveEvent, TwoEventsTwoSubmits) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);
    const VkEvent events[2] = {event, event2};

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.SetEvent(event2, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    vk::CmdWaitEvents(command_buffer2, 2, events, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    command_buffer2.End();

    m_default_queue->Submit(command_buffer);
    m_default_queue->SubmitAndWait(command_buffer2);
}

TEST_F(PositiveEvent, StageMaskTwoEventsTwoSubmits2) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);
    const VkEvent events[2] = {event, event2};

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.SetEvent(event2, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    command_buffer2.End();

    vkt::CommandBuffer command_buffer3(*m_device, m_command_pool);
    command_buffer3.Begin();
    vk::CmdWaitEvents(command_buffer3, 2, events, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    command_buffer3.End();

    m_default_queue->SubmitAndWait(command_buffer);
    m_default_queue->SubmitAndWait(command_buffer2);
    m_default_queue->SubmitAndWait(command_buffer3);
}

TEST_F(PositiveEvent, EventStageMaskHostSubmit) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_command_buffer.End();

    event.Set();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, PrimarySetSecondaryWaitCantDetectMismatch) {
    TEST_DESCRIPTION("CmdSetEvent is not possible to validate during record time without preceding CmdResetEvent");
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary.End();

    m_command_buffer.Begin();
    // We don't know if signal is ignored or not (there could be earlier signal which takes priority),
    // that's why we can't check for signal/wait source stage mismatch
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, SecondarySetAndWaitMismatch) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    // For the secondary command buffer we can't validate signal/wait stage mismatch
    // at record-time. The signal can be ignored if it is preceded by another one
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, SecondarySetAndWaitSubmit) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, SecondarySetPrimaryWait) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.End();

    // If due to regression ExecuteCommands has no effect then submit validation will report an error
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, PrimarySetSecondaryWait) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();

    // The src stage mask validation of WaitEvents happens during CmdExecuteCommands.
    // The following submit tests that src stage mask validation is not happening during submit time
    // (if validation leaks into submit processing it will likely report false positive)
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, PrimaryResetSecondarySetAndWait) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, SecondaryWaitTwoEvents) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);

    const VkEvent events[2] = {event, event2};

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    vk::CmdWaitEvents(secondary, 2, events, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    secondary.End();

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event2, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    command_buffer2.ExecuteCommands(secondary);
    command_buffer2.End();

    m_default_queue->Submit(command_buffer);
    m_default_queue->SubmitAndWait(command_buffer2);
}

TEST_F(PositiveEvent, BasicSetAndWaitEvent) {
    TEST_DESCRIPTION("Sets event and then wait for it using CmdSetEvent/CmdWaitEvents");
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveEvent, BasicSetAndWaitEvent2) {
    TEST_DESCRIPTION("Sets event and then wait for it using CmdSetEvent2/CmdWaitEvents2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_NONE;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    // Record time validation
    m_command_buffer.Begin();
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);
    m_command_buffer.End();

    // Also submit to the queue to test submit time validation
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveEvent, WaitEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;  // Ok to use if outside the renderpass
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdWaitEvents2KHR(m_command_buffer, 1, &event.handle(), &dependency_info);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, SetEvent2Flags) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkMemoryBarrier2 memory_barrier = vku::InitStructHelper();

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_ASYMMETRIC_EVENT_BIT_KHR;
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &memory_barrier;

    vkt::Event event(*m_device);
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
}

TEST_F(PositiveEvent, AsymmetricWaitEvent2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_ASYMMETRIC_EVENT_BIT_KHR;
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    m_command_buffer.Begin();

    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);

    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveEvent, AsymmetricEventNoMemorySubmit) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_ASYMMETRIC_EVENT_BIT_KHR;
    vkt::Event event(*m_device);

    monitor_.SetAllowedFailureMsg("VUID-vkCmdSetEvent2-dependencyFlags-10786");
    m_command_buffer.Begin();
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);
    m_command_buffer.End();

    // Check that missing memory barrier does not confuse submit validation
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, ResetAfterWait) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWait2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event, barrier);
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWait3) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event, barrier);
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWaitBarrierExecutionDependency) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);

    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWaitBarrierExecutionDependency2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.Barrier(barrier);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWaitBarrierExecutionDependency3) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 wait_barrier = vku::InitStructHelper();
    wait_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    wait_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

    VkMemoryBarrier2 transfer_barrier = vku::InitStructHelper();
    transfer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    // Use TRANSFER meta stage (potentially expanded by the validation) to check it correctly
    // chains with TRANSFER in the next barrier
    transfer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

    VkMemoryBarrier2 color_barrier = vku::InitStructHelper();
    color_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    color_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event, wait_barrier);
    m_command_buffer.WaitEvent2(event, wait_barrier);
    m_command_buffer.Barrier(transfer_barrier);
    m_command_buffer.Barrier(color_barrier);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWaitEmptyBarrier2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkDependencyInfo empty_dep_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    // The empty dependency should not affect execution dependency between Wait and Reset via COMPUTE_STAGE
    m_command_buffer.Barrier(empty_dep_info);

    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWaitSecondaryBarrier) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    vk::CmdPipelineBarrier(secondary, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, ResetAfterWaitSecondaryReset) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.ResetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.End();
}
