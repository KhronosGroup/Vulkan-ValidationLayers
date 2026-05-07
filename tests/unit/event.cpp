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

TEST_F(NegativeEvent, EventStageMask) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    // Source stage mask does not match CmdSetEvent's stage mask
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeEvent, EventStageMask2) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    // Source stage mask does not match CmdSetEvent's stage mask
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-01158");
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeEvent, EventStageMaskSubmit) {
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer command_buffer1(*m_device, m_command_pool);
    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    vkt::Event event(*m_device);

    command_buffer1.Begin();
    command_buffer1.SetEvent(event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    command_buffer1.End();
    m_default_queue->Submit(command_buffer1);

    command_buffer2.Begin();
    // Source stage mask does not match CmdSetEvent's stage mask
    vk::CmdWaitEvents(command_buffer2, 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    command_buffer2.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-parameter");
    m_default_queue->Submit(command_buffer2);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeEvent, DetectInterQueueEventUsage) {
    TEST_DESCRIPTION("Sets event on one queue and tries to wait on a different queue (CmdSetEvent/CmdWaitEvents)");
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }
    const vkt::Event event(*m_device);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdSetEvent(cb1, event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    cb1.End();

    vkt::CommandPool pool2(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb2(*m_device, pool2);
    cb2.Begin();
    vk::CmdWaitEvents(cb2, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, nullptr,
                      0, nullptr, 0, nullptr);
    cb2.End();

    m_default_queue->Submit(cb1);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();

    m_device->Wait();
}

TEST_F(NegativeEvent, DetectInterQueueEventUsage2) {
    TEST_DESCRIPTION("Sets event on one queue and tries to wait on a different queue (CmdSetEvent2/CmdWaitEvents2)");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
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

TEST_F(NegativeEvent, ResetEventThenSet) {
    TEST_DESCRIPTION("Reset an event then set it after the reset has been submitted.");
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdResetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    vk::SetEvent(device(), event);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

// This test should only be used for manual inspection
// Because a command buffer with vkCmdWaitEvents is submitted with an
// event that is never signaled, the test results in a VK_ERROR_DEVICE_LOST
TEST_F(NegativeEvent, DISABLED_WaitEventThenSet) {
#if defined(VVL_ENABLE_TSAN)
    // NOTE: This test in particular has failed sporadically on CI when TSAN is enabled.
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    vk::CmdResetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    vk::SetEvent(device(), event);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeEvent, EventOwnershipTransferUseAllStagesNoFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    // Enable extension but do not enable maintenance8 feature
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-dependencyFlags-10394");
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeEvent, EventOwnershipTransferUseAllStages) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags =
        VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR | VK_DEPENDENCY_VIEW_LOCAL_BIT;

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
