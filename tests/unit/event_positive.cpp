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
    vk::CmdWaitEvents(commandBuffer2, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.End();
    m_default_queue->Submit(commandBuffer2);

    m_default_queue->Wait();
}

TEST_F(PositiveEvent, EventStageMaskHost) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
                               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveEvent, EventStageMaskHostSubmit) {
    RETURN_IF_SKIP(Init());
    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.WaitEvent(event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_command_buffer.End();

    event.Set();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveEvent, BasicSetAndWaitEvent) {
    TEST_DESCRIPTION("Sets event and then wait for it using CmdSetEvent/CmdWaitEvents");
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    // Record time validation
    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.End();

    // Also submit to the queue to test submit time validation
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
