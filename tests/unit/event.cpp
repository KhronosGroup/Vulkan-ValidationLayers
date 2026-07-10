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

#include "layer_validation_tests.h"
#include "render_pass_helper.h"

class NegativeEvent : public SyncObjectTest {};

TEST_F(NegativeEvent, WaitEventsDifferentQueueFamilies) {
    TEST_DESCRIPTION("Using CmdWaitEvents with invalid barrier queue families");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }

    vkt::Event event(*m_device);
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkBufferMemoryBarrier BufferMemoryBarrier = vku::InitStructHelper();
    BufferMemoryBarrier.srcAccessMask = 0;
    BufferMemoryBarrier.dstAccessMask = 0;
    BufferMemoryBarrier.buffer = buffer;
    BufferMemoryBarrier.offset = 0;
    BufferMemoryBarrier.size = 256;
    BufferMemoryBarrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    BufferMemoryBarrier.dstQueueFamilyIndex = no_gfx.value();

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    VkImageMemoryBarrier ImageMemoryBarrier = vku::InitStructHelper();
    ImageMemoryBarrier.srcAccessMask = 0;
    ImageMemoryBarrier.dstAccessMask = 0;
    ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImageMemoryBarrier.image = image;
    ImageMemoryBarrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ImageMemoryBarrier.dstQueueFamilyIndex = no_gfx.value();
    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.layerCount = 1;
    ImageMemoryBarrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                      nullptr, 1, &BufferMemoryBarrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                      nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, CmdWaitEvents2DependencyFlags) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkEvent event_handle = event;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-dependencyFlags-10394");
    vk::CmdWaitEvents2KHR(m_command_buffer, 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Event event(*m_device);
    VkEvent event_handle = event;

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-dependencyFlags-03844");
    vk::CmdWaitEvents2KHR(m_command_buffer, 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskResetEvent) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.WaitEvent(event);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskHostResetEvent) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);
    event.Reset();

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event);
    m_command_buffer.End();
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->Submit(m_command_buffer);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, StageMaskResetEventSecondary) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, EventStageMaskSubmit) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    command_buffer.End();
    m_default_queue->Submit(command_buffer);

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent(event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    command_buffer2.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->Submit(command_buffer2);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeEvent, StageMaskWaitBeforeSet) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event);

    // Test that during submit validation this Set can't resolve previous wait
    m_command_buffer.SetEvent(event);
    m_command_buffer.End();

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->SubmitAndWait(m_command_buffer);
    monitor_.VerifyFound();
}

TEST_F(NegativeEvent, StageMaskSecondaryWaitBeforeSet) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event);
    secondary.SetEvent(event);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->SubmitAndWait(m_command_buffer);
    monitor_.VerifyFound();
}

TEST_F(NegativeEvent, SecondaryWaitIncludesHostStage) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary.End();

    m_command_buffer.Begin();

    // Record-time validation is only possible if we know there was reset,
    // otherwise we don't know if CmdSetEvent is ignored because it is a duplicate
    m_command_buffer.ResetEvent(event);

    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskTwoEventsTwoSubmits) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);
    const VkEvent events[2] = {event, event2};

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.SetEvent(event2, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    vk::CmdWaitEvents(command_buffer2, 2, events, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    command_buffer2.End();

    m_default_queue->Submit(command_buffer);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->SubmitAndWait(command_buffer2);
    monitor_.VerifyFound();
}

TEST_F(NegativeEvent, EventStageMaskHost) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_command_buffer.End();

    // SetEvent was not called on the host
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, StageMaskHost2) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_command_buffer.End();

    event.Set();

    // CmdWaitEvents srcStageMask does not contain HOST stage
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, StageMaskHostAndExtraDeviceStage) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);
    event.Set();

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, StageMaskPrimarySetSecondaryWait) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary.End();

    m_command_buffer.Begin();

    // Record-time validation is only possible if we know there was reset,
    // otherwise we don't know if CmdSetEvent is ignored because it is a duplicate
    m_command_buffer.ResetEvent(event);

    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskPrimarySetSecondaryWaitSubmit) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary.End();

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.ExecuteCommands(secondary);
    command_buffer2.End();

    m_default_queue->Submit(command_buffer);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_default_queue->SubmitAndWait(command_buffer2);
    monitor_.VerifyFound();
}

TEST_F(NegativeEvent, StageMaskPrimarySetSecondaryResetAndWait) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);
    const VkEvent events[2] = {event, event2};

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    // Reset the first event so its signal's stage mask is not taken into account during Wait call
    secondary.ResetEvent(event);

    vk::CmdWaitEvents(secondary, 2, events, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.ResetEvent(event2);
    m_command_buffer.SetEvent(event2, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskSecondarySetSecondaryWait) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.End();

    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary2.Begin();
    secondary2.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary2.End();

    m_command_buffer.Begin();
    // Make sure WaitEvents is validated at record-time.
    // Without Reset the validation can't prove CmdSetEvent is not ignored
    m_command_buffer.ResetEvent(event);

    m_command_buffer.ExecuteCommands(secondary);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary2);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskSecondarySetSecondaryWait2) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.ResetEvent(event);  // Make sure this is validated at record-time
    secondary.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.End();

    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary2.Begin();
    secondary2.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary2.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary2);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskTwoSecondariesSameCommand) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.End();

    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary2.Begin();
    secondary2.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    secondary2.End();

    const VkCommandBuffer secondaries[2] = {secondary, secondary2};

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    vk::CmdExecuteCommands(m_command_buffer, 2, secondaries);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, StageMaskPrimaryResetSecondarySetAndWait) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, RepeatedSetEvent) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, InterQueueEventUsage) {
    TEST_DESCRIPTION("Sets event on one queue and tries to wait on a different queue (CmdSetEvent/CmdWaitEvents)");
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }
    const vkt::Event event(*m_device);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    cb1.SetEvent(event);
    cb1.End();

    vkt::CommandPool pool2(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb2(*m_device, pool2);
    cb2.Begin();
    cb2.WaitEvent(event);
    cb2.End();

    m_default_queue->Submit(cb1);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeEvent, InterQueueEvent2Usage) {
    TEST_DESCRIPTION("Sets event on one queue and tries to wait on a different queue (CmdSetEvent2/CmdWaitEvents2)");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_NONE;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdSetEvent2(cb1, event, &dependency_info);
    cb1.End();

    vkt::CommandPool pool2(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb2(*m_device, pool2);
    cb2.Begin();
    vk::CmdWaitEvents2(cb2, 1, &event.handle(), &dependency_info);
    cb2.End();

    m_default_queue->Submit(cb1);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeEvent, InterQueueEventUsageSecondary) {
    TEST_DESCRIPTION("Set/Wait event from different queues and use secondary command buffer");
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }
    const vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.End();

    vkt::CommandPool pool(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event);
    secondary.End();

    vkt::CommandBuffer cb(*m_device, pool);
    cb.Begin();
    cb.ExecuteCommands(secondary);
    cb.End();

    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeEvent, InterQueueEvent2UsageSecondary) {
    TEST_DESCRIPTION("Set2/Wait2 event from different queues and use secondary command buffer");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }
    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_NONE;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event, barrier);
    m_command_buffer.End();

    vkt::CommandBuffer secondary(*m_device, m_second_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent2(event, barrier);
    secondary.End();

    vkt::CommandBuffer cb(*m_device, m_second_command_pool);
    cb.Begin();
    cb.ExecuteCommands(secondary);
    cb.End();

    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeEvent, WaitOnNoEvent) {
    RETURN_IF_SKIP(Init());
    VkEvent bad_event = CastToHandle<VkEvent, uintptr_t>(0xbaadbeef);
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-pEvents-parameter");
    vk::CmdWaitEvents(m_command_buffer, 1, &bad_event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr,
                      0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, InvalidDeviceOnlyEvent) {
    TEST_DESCRIPTION("Attempt to use device only event with host commands.");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkEventCreateInfo event_ci = vku::InitStructHelper();
    event_ci.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
    vkt::Event ev(*m_device, event_ci);

    m_errorMonitor->SetDesiredError("VUID-vkResetEvent-event-03823");
    vk::ResetEvent(*m_device, ev);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-03941");
    vk::SetEvent(*m_device, ev);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, SetEvent2DependencyFlags) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dependencyFlags-03825");
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, SetEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-srcStageMask-09391");  // src
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dstStageMask-09392");  // dst
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, SetEvent2HostStageKHR) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-srcStageMask-09391");  // src
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dstStageMask-09392");  // dst
    vk::CmdSetEvent2KHR(m_command_buffer, event, &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, WaitEventRenderPassHostBit) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-07308");
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr,
                      0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, ResetEventThenHostSet) {
    TEST_DESCRIPTION("Submit event reset then set it on the host");
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    event.Set();
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeEvent, SetEventThenHostSet) {
    TEST_DESCRIPTION("Submit event set then set it on the host");
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    event.Set();
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeEvent, WaitEventThenHostSet) {
    TEST_DESCRIPTION("Submit event wait then set it on the host");
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    event.Set();
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeEvent, OwnershipTransferUseAllStagesNoFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    // Enable extension but do not enable maintenance8 feature
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-maintenance8-10205");
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, CmdWaitEvents2KHRUsedButSynchronizaion2Disabled) {
    TEST_DESCRIPTION("Using CmdWaitEvents2KHR when synchronization2 is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkEvent event_handle = event;
    VkDependencyInfo dependency_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-synchronization2-03836");
    vk::CmdWaitEvents2KHR(m_command_buffer, 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-synchronization2-03836");
        vk::CmdWaitEvents2(m_command_buffer, 1, &event_handle, &dependency_info);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.End();
}

TEST_F(NegativeEvent, AsymmetricSetEvent2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barriers[2];
    barriers[0] = vku::InitStructHelper();
    barriers[0].srcAccessMask = 0;
    barriers[0].dstAccessMask = 0;
    barriers[0].srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barriers[0].dstStageMask = 0;
    barriers[1] = vku::InitStructHelper();
    barriers[1].srcAccessMask = 0;
    barriers[1].dstAccessMask = 0;
    barriers[1].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    barriers[1].dstStageMask = 0;

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.buffer = buffer;
    buffer_barrier.offset = 0u;
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_ASYMMETRIC_EVENT_BIT_KHR;
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = barriers;
    dependency_info.bufferMemoryBarrierCount = 1u;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;

    const vkt::Event event(*m_device);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dependencyFlags-10785");
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    m_errorMonitor->VerifyFound();

    dependency_info.bufferMemoryBarrierCount = 0u;
    dependency_info.memoryBarrierCount = 2u;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dependencyFlags-10786");
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    m_errorMonitor->VerifyFound();

    dependency_info.memoryBarrierCount = 1u;
    barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dependencyFlags-10787");
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeEvent, AsymmetricWaitEvent2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    m_command_buffer.Begin();

    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);

    dependency_info.dependencyFlags = VK_DEPENDENCY_ASYMMETRIC_EVENT_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);

    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-10789");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}


TEST_F(NegativeEvent, MismatchedDependencyInfo) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    m_command_buffer.Begin();

    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);

    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);

    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-10788");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, MismatchedDependencyInfo2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    const vkt::Event event(*m_device);

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent2(event, barrier);
    command_buffer.End();

    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent2(event, barrier);
    command_buffer2.End();

    const VkCommandBuffer command_buffers[2] = {command_buffer, command_buffer2};
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers;

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-10788");
    vk::QueueSubmit(*m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, InvalidAssymetricSrcStagemask) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::geometryShader);
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

    barrier.srcStageMask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);

    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-10790");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, StageMaskHost) {
    TEST_DESCRIPTION("Test invalid usage of VK_PIPELINE_STAGE_HOST_BIT.");
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-01149");
    vk::CmdSetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent-stageMask-01153");
    vk::CmdResetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device);
    // Signal the semaphore so we can wait on it.
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-00078");
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore, VK_PIPELINE_STAGE_HOST_BIT));
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeEvent, SetWait2Mismatch) {
    TEST_DESCRIPTION("CmdSetEvent -> CmdWaitEvents2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);  // ensure that event is in known (unsignaled) state
    m_command_buffer.SetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03837");
    m_command_buffer.WaitEvent2(event, barrier);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, SetWait2MismatchSecondary) {
    TEST_DESCRIPTION("CmdSetEvent -> CmdWaitEvents2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent2(event, barrier);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);  // ensure that event is in known (unsignaled) state
    m_command_buffer.SetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03837");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, SetWait2MismatchSecondarySubmit) {
    TEST_DESCRIPTION("CmdSetEvent -> CmdWaitEvents2 in secondary");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent2(event, barrier);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03837");
    m_default_queue->Submit(m_command_buffer);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, SetWait2MismatchSubmit) {
    TEST_DESCRIPTION("CmdSetEvent -> CmdWaitEvents2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event);
    command_buffer.End();
    m_default_queue->Submit(command_buffer);

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent2(event, barrier);
    command_buffer2.End();
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03837");
    m_default_queue->Submit(command_buffer2);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, SetWaitThenWait2MismatchSubmit) {
    TEST_DESCRIPTION("CmdSetEvent -> CmdWaitEvents -> CmdWaitEvents2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent(event);
    command_buffer.WaitEvent(event);
    command_buffer.End();
    m_default_queue->Submit(command_buffer);

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent2(event, barrier);
    command_buffer2.End();
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03837");
    m_default_queue->Submit(command_buffer2);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, Set2WaitMismatch) {
    TEST_DESCRIPTION("CmdSetEvent2 -> CmdWaitEvents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent2(event);  // ensure the event is in known (unsignaled) state
    m_command_buffer.SetEvent2(event, barrier);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    m_command_buffer.WaitEvent(event);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, Set2WaitMismatchSecondary) {
    TEST_DESCRIPTION("CmdSetEvent2 -> CmdWaitEvents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent2(event);  // put event in known (unsignaled) state
    m_command_buffer.SetEvent2(event, barrier);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, Set2WaitMismatchIgnoredSignalSecondary) {
    TEST_DESCRIPTION("CmdSetEvent2 in primary -> ignored CmdSetEvent in secondary -> CmdWaitEvents in secondary");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event);  // ignored signal
    secondary.WaitEvent(event);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent2(event);
    m_command_buffer.SetEvent2(event, barrier);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, Set2WaitMismatchSubmit) {
    TEST_DESCRIPTION("CmdSetEvent2 -> CmdWaitEvents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent2(event, barrier);
    command_buffer.End();
    m_default_queue->Submit(command_buffer);

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent(event);
    command_buffer2.End();
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    m_default_queue->Submit(command_buffer2);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, Set2WaitMismatchIgnoredSignal) {
    TEST_DESCRIPTION("CmdSetEvent2 -> ignored CmdSetEvent -> CmdWaitEvents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.ResetEvent(event);
    command_buffer.SetEvent2(event, barrier);
    command_buffer.SetEvent(event);  // ignored signal
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    command_buffer.WaitEvent(event);
    monitor_.VerifyFound();
    command_buffer.End();
}

TEST_F(NegativeEvent, Set2WaitMismatchIgnoredSignalSubmit) {
    TEST_DESCRIPTION("CmdSetEvent2 -> ignored CmdSetEvent -> CmdWaitEvents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent2(event, barrier);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.SetEvent(event);  // ignored signal
    command_buffer2.WaitEvent(event);
    command_buffer2.End();

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    m_default_queue->Submit({command_buffer, command_buffer2});
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, Set2Wait2ThenWaitMismatchSubmit) {
    TEST_DESCRIPTION("CmdSetEvent2 -> CmdWaitEvents2 -> CmdWaitEvents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.SetEvent2(event, barrier);
    command_buffer.WaitEvent2(event, barrier);
    command_buffer.End();
    m_default_queue->Submit(command_buffer);

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent(event);
    command_buffer2.End();
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-pEvents-03847");
    m_default_queue->Submit(command_buffer2);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, ResetAfterWait) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    monitor_.SetDesiredError("VUID-vkCmdResetEvent-event-03834");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, ResetAfterWait2) {
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
    monitor_.SetDesiredError("VUID-vkCmdResetEvent-event-03835");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, Reset2AfterWait) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    monitor_.SetDesiredError("VUID-vkCmdResetEvent2-event-03831");
    m_command_buffer.ResetEvent2(event, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, Reset2AfterWait2) {
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
    monitor_.SetDesiredError("VUID-vkCmdResetEvent2-event-03832");
    m_command_buffer.ResetEvent2(event, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, ResetAfterWaitBarrierExecutionDependency) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

    VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
    barrier2.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkMemoryBarrier2 barriers[2] = {barrier, barrier2};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 2;
    dep_info.pMemoryBarriers = barriers;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.Barrier(dep_info);

    // Barriers are applied independently, so STAGE_COMPUTE from the first barrier can't
    // create execution dependency with STAGE_COLOR_ATTACHMENT_OUTPUT from the second barrier
    monitor_.SetDesiredError("VUID-vkCmdResetEvent-event-03834");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, ResetAfterWaitBarrierExecutionDependency2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;

    VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
    barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkMemoryBarrier2 barriers[2] = {barrier, barrier2};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 2;
    dep_info.pMemoryBarriers = barriers;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.Barrier(dep_info);

    // Barriers are applied independently, they do not form execution dependency between STAGE_COMPUTE
    // and STAGE_COLOR_ATTACHMENT_OUTPUT as it would be in the case of two sequential barriers
    monitor_.SetDesiredError("VUID-vkCmdResetEvent-event-03834");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, ResetAfterSecondaryWaitRace) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.SetDesiredError("VUID-vkCmdResetEvent-event-03834");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, ResetAfterWaitMultipleEvents) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event0(*m_device);
    vkt::Event event1(*m_device);
    const VkEvent events[2] = {event0, event1};

    VkMemoryBarrier2 barrier0 = vku::InitStructHelper();
    barrier0.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier0.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

    VkMemoryBarrier2 barrier1 = vku::InitStructHelper();
    barrier1.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier1.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;

    VkDependencyInfo dep_infos[2];
    dep_infos[0] = vku::InitStructHelper();
    dep_infos[0].memoryBarrierCount = 1;
    dep_infos[0].pMemoryBarriers = &barrier0;
    dep_infos[1] = vku::InitStructHelper();
    dep_infos[1].memoryBarrierCount = 1;
    dep_infos[1].pMemoryBarriers = &barrier1;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event0, barrier0);
    m_command_buffer.SetEvent2(event1, barrier1);
    vk::CmdWaitEvents2(m_command_buffer, 2, events, dep_infos);

    // Reset must use COMPUTE_STAGE to sync with event0 wait.
    // This test checks barriers are applied independently, so barrier1 does not
    // have effect on event0 wait (does not introduce COPY_STAGE)
    monitor_.SetDesiredError("VUID-vkCmdResetEvent2-event-03832");
    m_command_buffer.ResetEvent2(event0, VK_PIPELINE_STAGE_2_COPY_BIT);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitWithoutSet) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    m_command_buffer.Begin();
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03841");
    m_command_buffer.ResetEvent(event);
    m_command_buffer.WaitEvent2(event, barrier);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitWithoutSetSecondary) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent2(event, barrier);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03841");
    m_command_buffer.ExecuteCommands(secondary);
    monitor_.VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitWithoutSetSubmit) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.End();

    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03841");
    m_default_queue->Submit(m_command_buffer);
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, HostSetBarrierScope) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.End();

    // Barrier source stage mask must be HOST stages
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03839");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, HostSetBarrierScope2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_HOST_BIT;

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.End();

    // Barrier source stage mask must be HOST stages
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03839");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, HostSetBarrierScopeSeconary) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent2(event, barrier);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();

    // Barrier source stage mask must be HOST stages
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03839");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, HostSetAndIgnoresDeviceSet) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event, barrier);
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.End();

    // CmdSetEvent2 is ignored because the event is already signaled by the host.
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03839");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, WaitWithOnlyHostDependency) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03840");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, WaitWithOnlyHostDependency2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent2(event);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03840");
    m_command_buffer.WaitEvent2(event, barrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitWithOnlyHostDependency3) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    vkt::CommandBuffer reset_cb(*m_device, m_command_pool);
    reset_cb.Begin();
    reset_cb.ResetEvent2(event);
    reset_cb.End();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;

    vkt::CommandBuffer wait_cb(*m_device, m_command_pool);
    wait_cb.Begin();
    wait_cb.WaitEvent2(event, barrier);
    wait_cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03840");
    m_default_queue->Submit({reset_cb, wait_cb});
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, WaitWithOnlyHostDependencySecondary) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    event.Set();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.WaitEvent2(event, barrier);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent2(event);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-pEvents-03840");
    m_command_buffer.ExecuteCommands(secondary);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitEventsLayoutTransitionInsideRenderPass) {
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddColorAttachment(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddSubpassSelfDependency();
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp, 1, &image_view.handle());

    vkt::Event event(*m_device);

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp, fb);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents-oldLayout-01181");
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 1, &image_barrier);
    monitor_.VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, WaitEventsLayoutTransitionInsideRenderPass2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();

    vkt::Event event(*m_device);

    VkRenderingAttachmentInfo attachment = vku::InitStructHelper();
    attachment.imageView = image_view;
    attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {64, 64};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &attachment;

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    monitor_.SetDesiredError("VUID-vkCmdWaitEvents2-oldLayout-01181");
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dep_info);
    monitor_.VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, HostResetPendingWait) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    monitor_.SetDesiredError("VUID-vkResetEvent-event-03821");
    event.Reset();
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, HostResetPendingWait2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    m_command_buffer.Begin();
    m_command_buffer.SetEvent2(event, barrier);
    m_command_buffer.WaitEvent2(event, barrier);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    monitor_.SetDesiredError("VUID-vkResetEvent-event-03822");
    event.Reset();
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, HostResetMultiplePendingWaits) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    vkt::Fence fence(*m_device);

    event.Set();

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.Begin();
    command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.WaitEvent(event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    command_buffer2.End();

    m_default_queue->Submit(command_buffer, fence);
    m_default_queue->Submit(command_buffer2);

    fence.Wait(kWaitTimeout);

    monitor_.SetDesiredError("VUID-vkResetEvent-event-03821");
    event.Reset();
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, HostResetPendingWaitAndReset) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event);
    m_command_buffer.WaitEvent(event);
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 0, nullptr);
    m_command_buffer.ResetEvent(event);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    monitor_.SetDesiredError("VUID-vkResetEvent-event-03821");
    event.Reset();
    monitor_.VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeEvent, HostResetPendingWaitSecondary) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.SetEvent(event);
    secondary.WaitEvent(event);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    monitor_.SetDesiredError("VUID-vkResetEvent-event-03821");
    event.Reset();
    monitor_.VerifyFound();
    m_default_queue->Wait();
}
