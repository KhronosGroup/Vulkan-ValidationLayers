/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

#ifndef VK_USE_PLATFORM_WIN32_KHR
#include <poll.h>
#endif

TEST_F(PositiveSyncObject, ThreadSafetyDisplayObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPropertiesKHR in thread-safety.");

    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPropertiesKHR(gpu(), &prop_count, nullptr);
    if (prop_count == 0) {
        GTEST_SKIP() << "No VkDisplayKHR properties to query";
    }

    std::vector<VkDisplayPropertiesKHR> display_props{prop_count};
    // Create a VkDisplayKHR object
    vk::GetPhysicalDeviceDisplayPropertiesKHR(gpu(), &prop_count, display_props.data());
    ASSERT_NE(prop_count, 0U);

    // Now use this new object in an API call that thread safety will track
    prop_count = 0;
    vk::GetDisplayModePropertiesKHR(gpu(), display_props[0].display, &prop_count, nullptr);
}

TEST_F(PositiveSyncObject, ThreadSafetyDisplayPlaneObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPlanePropertiesKHR in thread-safety.");

    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &prop_count, nullptr);
    if (prop_count != 0) {
        // only grab first plane property
        prop_count = 1;
        VkDisplayPlanePropertiesKHR display_plane_props = {};
        // Create a VkDisplayKHR object
        vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &prop_count, &display_plane_props);
        // Now use this new object in an API call
        prop_count = 0;
        vk::GetDisplayModePropertiesKHR(gpu(), display_plane_props.currentDisplay, &prop_count, nullptr);
    }
}

TEST_F(PositiveSyncObject, Sync2OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics capable required)";
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx.value())[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj no_gfx_cb(m_device, &no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, no_gfx_queue);

    // Create an "exclusive" image owned by the graphics queue.
    VkImageObj image(m_device);
    VkFlags image_use = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, image_use, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    auto image_subres = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    auto image_barrier = image.image_memory_barrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                                    image.Layout(), image.Layout(), image_subres);
    image_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.dstQueueFamilyIndex = no_gfx.value();

    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, nullptr, &image_barrier);

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx.value();
    image_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR;
    image_barrier.oldLayout = image.Layout();
    // Make sure the new layout is different from the old
    if (image_barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, nullptr, &image_barrier);
}

TEST_F(PositiveSyncObject, Sync2OwnershipTranfersBuffer) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics capable required)";
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx.value())[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj no_gfx_cb(m_device, &no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, no_gfx_queue);

    // Create a buffer
    const VkDeviceSize buffer_size = 256;
    uint8_t data[buffer_size] = {0xFF};
    VkConstantBufferObj buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized());
    auto buffer_barrier =
        buffer.buffer_memory_barrier(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
                                     VK_ACCESS_2_NONE_KHR, VK_ACCESS_2_NONE_KHR, 0, VK_WHOLE_SIZE);

    // Let gfx own it.
    buffer_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ValidOwnershipTransferOp(m_errorMonitor, m_commandBuffer, &buffer_barrier, nullptr);

    // Transfer it to non-gfx
    buffer_barrier.dstQueueFamilyIndex = no_gfx.value();
    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, &buffer_barrier, nullptr);

    // Transfer it to gfx
    buffer_barrier.srcQueueFamilyIndex = no_gfx.value();
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR;

    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, &buffer_barrier, nullptr);
}

TEST_F(PositiveSyncObject, LayoutFromPresentWithoutAccessMemoryRead) {
    // Transition an image away from PRESENT_SRC_KHR without ACCESS_MEMORY_READ
    // in srcAccessMask.

    // The required behavior here was a bit unclear in earlier versions of the
    // spec, but there is no memory dependency required here, so this should
    // work without warnings.

    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageMemoryBarrier barrier = LvlInitStruct<VkImageMemoryBarrier>();
    VkImageSubresourceRange range;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.image = image.handle();
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    barrier.subresourceRange = range;
    VkCommandBufferObj cmdbuf(m_device, m_commandPool);
    cmdbuf.begin();
    cmdbuf.PipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    cmdbuf.PipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &barrier);
}

TEST_F(PositiveSyncObject, QueueSubmitSemaphoresAndLayoutTracking) {
    TEST_DESCRIPTION("Submit multiple command buffers with chained semaphore signals and layout transitions");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkCommandBuffer cmd_bufs[4];
    VkCommandBufferAllocateInfo alloc_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 4;
    alloc_info.commandPool = m_commandPool->handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &alloc_info, cmd_bufs);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
               (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkCommandBufferBeginInfo cb_binfo = LvlInitStruct<VkCommandBufferBeginInfo>();
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    // Use 4 command buffers, each with an image layout transition, ColorAO->General->ColorAO->TransferSrc->TransferDst
    vk::BeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(cmd_bufs[0], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[0]);
    vk::BeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk::CmdPipelineBarrier(cmd_bufs[1], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[1]);
    vk::BeginCommandBuffer(cmd_bufs[2], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk::CmdPipelineBarrier(cmd_bufs[2], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[2]);
    vk::BeginCommandBuffer(cmd_bufs[3], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk::CmdPipelineBarrier(cmd_bufs[3], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[3]);

    // Submit 4 command buffers in 3 submits, with submits 2 and 3 waiting for semaphores from submits 1 and 2
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore1(*m_device, semaphore_create_info);
    vk_testing::Semaphore semaphore2(*m_device, semaphore_create_info);
    VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    VkSubmitInfo submit_info[3];
    submit_info[0] = LvlInitStruct<VkSubmitInfo>();
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = &cmd_bufs[0];
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore1.handle();
    submit_info[0].waitSemaphoreCount = 0;
    submit_info[0].pWaitDstStageMask = nullptr;
    submit_info[0].pWaitDstStageMask = flags;
    submit_info[1] = LvlInitStruct<VkSubmitInfo>();
    submit_info[1].commandBufferCount = 1;
    submit_info[1].pCommandBuffers = &cmd_bufs[1];
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore1.handle();
    submit_info[1].signalSemaphoreCount = 1;
    submit_info[1].pSignalSemaphores = &semaphore2.handle();
    submit_info[1].pWaitDstStageMask = flags;
    submit_info[2] = LvlInitStruct<VkSubmitInfo>();
    submit_info[2].commandBufferCount = 2;
    submit_info[2].pCommandBuffers = &cmd_bufs[2];
    submit_info[2].waitSemaphoreCount = 1;
    submit_info[2].pWaitSemaphores = &semaphore2.handle();
    submit_info[2].signalSemaphoreCount = 0;
    submit_info[2].pSignalSemaphores = nullptr;
    submit_info[2].pWaitDstStageMask = flags;
    vk::QueueSubmit(m_device->m_queue, 3, submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(PositiveSyncObject, ResetUnsignaledFence) {
    vk_testing::Fence testFence;
    VkFenceCreateInfo fenceInfo = LvlInitStruct<VkFenceCreateInfo>();

    ASSERT_NO_FATAL_FAILURE(Init());
    testFence.init(*m_device, fenceInfo);
    VkFence fences[1] = {testFence.handle()};
    VkResult result = vk::ResetFences(m_device->device(), 1, fences);
    ASSERT_VK_SUCCESS(result);
}

TEST_F(PositiveSyncObject, FenceCreateSignaledWaitHandling) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // A fence created signaled
    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    vk_testing::Fence f1(*m_device, fci);

    // A fence created not
    fci.flags = 0;
    vk_testing::Fence f2(*m_device, fci);

    // Submit the unsignaled fence
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 0, nullptr, 0, nullptr};
    vk::QueueSubmit(m_device->m_queue, 1, &si, f2.handle());

    // Wait on both fences, with signaled first.
    VkFence fences[] = {f1.handle(), f2.handle()};
    vk::WaitForFences(m_device->device(), 2, fences, VK_TRUE, kWaitTimeout);
    // Should have both retired! (get destroyed now)
}

TEST_F(PositiveSyncObject, TwoFencesThreeFrames) {
    TEST_DESCRIPTION(
        "Two command buffers with two separate fences are each run through a Submit & WaitForFences cycle 3 times. This previously "
        "revealed a bug so running this positive test to prevent a regression.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    static const uint32_t NUM_OBJECTS = 2;
    static const uint32_t NUM_FRAMES = 3;
    VkCommandBuffer cmd_buffers[NUM_OBJECTS] = {};
    VkFence fences[NUM_OBJECTS] = {};

    VkCommandPoolCreateInfo cmd_pool_ci = LvlInitStruct<VkCommandPoolCreateInfo>();
    cmd_pool_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool cmd_pool(*m_device, cmd_pool_ci);

    VkCommandBufferAllocateInfo cmd_buf_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buf_info.commandPool = cmd_pool.handle();
    cmd_buf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_info.commandBufferCount = 1;

    VkFenceCreateInfo fence_ci = LvlInitStruct<VkFenceCreateInfo>();
    fence_ci.flags = 0;

    VkResult err;
    for (uint32_t i = 0; i < NUM_OBJECTS; ++i) {
        err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buf_info, &cmd_buffers[i]);
        ASSERT_VK_SUCCESS(err);
        err = vk::CreateFence(m_device->device(), &fence_ci, nullptr, &fences[i]);
        ASSERT_VK_SUCCESS(err);
    }

    for (uint32_t frame = 0; frame < NUM_FRAMES; ++frame) {
        for (uint32_t obj = 0; obj < NUM_OBJECTS; ++obj) {
            // Create empty cmd buffer
            VkCommandBufferBeginInfo cmdBufBeginDesc = LvlInitStruct<VkCommandBufferBeginInfo>();

            err = vk::BeginCommandBuffer(cmd_buffers[obj], &cmdBufBeginDesc);
            ASSERT_VK_SUCCESS(err);
            err = vk::EndCommandBuffer(cmd_buffers[obj]);
            ASSERT_VK_SUCCESS(err);

            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &cmd_buffers[obj];
            // Submit cmd buffer and wait for fence
            err = vk::QueueSubmit(queue, 1, &submit_info, fences[obj]);
            ASSERT_VK_SUCCESS(err);
            err = vk::WaitForFences(m_device->device(), 1, &fences[obj], VK_TRUE, kWaitTimeout);
            ASSERT_VK_SUCCESS(err);
            err = vk::ResetFences(m_device->device(), 1, &fences[obj]);
            ASSERT_VK_SUCCESS(err);
        }
    }
    for (uint32_t i = 0; i < NUM_OBJECTS; ++i) {
        vk::DestroyFence(m_device->device(), fences[i], nullptr);
    }
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceQWI) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues followed by a QueueWaitIdle.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore.handle();
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceQWIFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by a QueueWaitIdle.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }

    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore.handle();
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceTwoWFF) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by two consecutive WaitForFences calls on the same fence.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }

    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore.handle();
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());
    }

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueuesEnsureCorrectRetirementWithWorkStolen) {
    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Test requires two queues";
    }

    VkResult err;

    VkQueue q0 = m_device->m_queue;
    VkQueue q1 = nullptr;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &q1);
    ASSERT_NE(q1, nullptr);

    // An (empty) command buffer. We must have work in the first submission --
    // the layer treats unfenced work differently from fenced work.
    VkCommandPoolCreateInfo cpci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, 0, 0};
    vk_testing::CommandPool command_pool(*m_device, cpci);

    VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, command_pool.handle(),
                                        VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    VkCommandBuffer cb;
    err = vk::AllocateCommandBuffers(m_device->device(), &cbai, &cb);
    ASSERT_VK_SUCCESS(err);
    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    err = vk::BeginCommandBuffer(cb, &cbbi);
    ASSERT_VK_SUCCESS(err);
    err = vk::EndCommandBuffer(cb);
    ASSERT_VK_SUCCESS(err);

    // A semaphore
    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    vk_testing::Semaphore s(*m_device, sci);

    // First submission, to q0
    VkSubmitInfo s0 = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cb, 1, &s.handle()};

    err = vk::QueueSubmit(q0, 1, &s0, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Second submission, to q1, waiting on s
    VkFlags waitmask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // doesn't really matter what this value is.
    VkSubmitInfo s1 = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &s.handle(), &waitmask, 0, nullptr, 0, nullptr};

    err = vk::QueueSubmit(q1, 1, &s1, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Wait for q0 idle
    err = vk::QueueWaitIdle(q0);
    ASSERT_VK_SUCCESS(err);

    // Command buffer should have been completed (it was on q0); reset the pool.
    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 1, &cb);

    // Force device completely idle and clean up resources
    vk::DeviceWaitIdle(m_device->device());
    ;
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence, "
        "followed by a WaitForFences call.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }

    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore.handle();
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());
    }

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithTimelineSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, ordered by a timeline semaphore,"
        " the second having a fence, followed by a WaitForFences call.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }

    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 0;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        uint64_t signal_value = 1;
        VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
        timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &signal_value;
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();
        ASSERT_VK_SUCCESS(vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));
    }
    {
        uint64_t wait_value = 1;
        VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
        timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pWaitSemaphoreValues = &wait_value;
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore.handle();
        submit_info.pWaitDstStageMask = flags;
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle()));
    }

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsOneQueueWithSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, sharing a signal/wait semaphore, the second "
        "having a fence, followed by a WaitForFences call.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore.handle();
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());
    }

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsOneQueueNullQueueSubmitWithFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, no fences, followed by a third QueueSubmit "
        "with NO SubmitInfos but with a fence, followed by a WaitForFences call.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = VK_NULL_HANDLE;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = VK_NULL_HANDLE;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueSubmit(m_device->m_queue, 0, NULL, fence.handle());

    VkResult err = vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
    ASSERT_VK_SUCCESS(err);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsOneQueueOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, the second having a fence, followed by a "
        "WaitForFences call.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = VK_NULL_HANDLE;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = VK_NULL_HANDLE;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());
    }

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, TwoSubmitInfosWithSemaphoreOneQueueSubmitsOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers each in a separate SubmitInfo sent in a single QueueSubmit call followed by a WaitForFences call.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info[2];
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

        submit_info[0] = LvlInitStruct<VkSubmitInfo>();
        submit_info[0].commandBufferCount = 1;
        submit_info[0].pCommandBuffers = &command_buffer[0];
        submit_info[0].signalSemaphoreCount = 1;
        submit_info[0].pSignalSemaphores = &semaphore.handle();
        submit_info[0].waitSemaphoreCount = 0;
        submit_info[0].pWaitSemaphores = NULL;
        submit_info[0].pWaitDstStageMask = 0;

        submit_info[1] = LvlInitStruct<VkSubmitInfo>();
        submit_info[1].commandBufferCount = 1;
        submit_info[1].pCommandBuffers = &command_buffer[1];
        submit_info[1].waitSemaphoreCount = 1;
        submit_info[1].pWaitSemaphores = &semaphore.handle();
        submit_info[1].pWaitDstStageMask = flags;
        submit_info[1].signalSemaphoreCount = 0;
        submit_info[1].pSignalSemaphores = NULL;
        vk::QueueSubmit(m_device->m_queue, 2, &submit_info[0], fence.handle());
    }

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, &command_buffer[0]);
}

TEST_F(PositiveSyncObject, LongSemaphoreChain) {
    ASSERT_NO_FATAL_FAILURE(Init());
    VkResult err;

    std::vector<VkSemaphore> semaphores;

    const int chainLength = 32768;
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    for (int i = 0; i < chainLength; i++) {
        VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
        VkSemaphore semaphore;
        err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &semaphore);
        ASSERT_VK_SUCCESS(err);

        semaphores.push_back(semaphore);

        VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           nullptr,
                           semaphores.size() > 1 ? 1u : 0u,
                           semaphores.size() > 1 ? &semaphores[semaphores.size() - 2] : nullptr,
                           &flags,
                           0,
                           nullptr,
                           1,
                           &semaphores[semaphores.size() - 1]};
        err = vk::QueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
    }

    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    vk_testing::Fence fence(*m_device, fci);
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &semaphores.back(), &flags, 0, nullptr, 0, nullptr};
    err = vk::QueueSubmit(m_device->m_queue, 1, &si, fence.handle());
    ASSERT_VK_SUCCESS(err);

    vk::WaitForFences(m_device->device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    for (auto semaphore : semaphores) vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

TEST_F(PositiveSyncObject, ExternalSemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR, nullptr,
                                                    handle_type};
    VkExternalSemaphorePropertiesKHR esp = {VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR, nullptr};
    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }

    VkResult err;

    // Create a semaphore to export payload from
    auto esci = LvlInitStruct<VkExportSemaphoreCreateInfoKHR>();
    esci.handleTypes = handle_type;
    auto sci = LvlInitStruct<VkSemaphoreCreateInfo>(&esci);

    vk_testing::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    vk_testing::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    err = export_semaphore.export_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);
    err = import_semaphore.import_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);

    // Signal the exported semaphore and wait on the imported semaphore
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkSubmitInfo> si(4, LvlInitStruct<VkSubmitInfo>());
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;
    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[2] = si[0];
    si[3] = si[1];

    err = vk::QueueSubmit(m_device->m_queue, si.size(), si.data(), VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    if (m_device->phy().features().sparseBinding) {
        // Signal the imported semaphore and wait on the exported semaphore
        std::vector<VkBindSparseInfo> bi(4, LvlInitStruct<VkBindSparseInfo>());
        bi[0].signalSemaphoreCount = 1;
        bi[0].pSignalSemaphores = &export_semaphore.handle();
        bi[1].waitSemaphoreCount = 1;
        bi[1].pWaitSemaphores = &import_semaphore.handle();
        bi[2] = bi[0];
        bi[3] = bi[1];
        err = vk::QueueBindSparse(m_device->m_queue, bi.size(), bi.data(), VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
    }

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(PositiveSyncObject, ExternalTimelineSemaphore) {
    TEST_DESCRIPTION(
        "Export and import a timeline semaphore. "
        "Should be roughly equivalant to the CTS *cross_instance*timeline_semaphore* tests");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));

    // Check for external semaphore import and export capability
    auto tci = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    tci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto esi = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfoKHR>(&tci);
    esi.handleType = handle_type;

    auto esp = LvlInitStruct<VkExternalSemaphorePropertiesKHR>();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }
    VkResult err;

    // Create a semaphore to export payload from
    auto esci = LvlInitStruct<VkExportSemaphoreCreateInfoKHR>();
    esci.handleTypes = handle_type;
    auto stci = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>(&esci);
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    auto sci = LvlInitStruct<VkSemaphoreCreateInfo>(&stci);

    vk_testing::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    stci.pNext = nullptr;
    vk_testing::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    err = export_semaphore.export_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);
    err = import_semaphore.import_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);

    uint64_t wait_value = 1;
    uint64_t signal_value = 12345;

    // Signal the exported semaphore and wait on the imported semaphore
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkTimelineSemaphoreSubmitInfo> ti(2, LvlInitStruct<VkTimelineSemaphoreSubmitInfo>());
    std::vector<VkSubmitInfo> si(2, LvlInitStruct<VkSubmitInfo>());

    si[0].pWaitDstStageMask = &flags;
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();
    si[0].pNext = &ti[0];
    ti[0].signalSemaphoreValueCount = 1;
    ti[0].pSignalSemaphoreValues = &signal_value;
    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;
    si[1].pNext = &ti[1];
    ti[1].waitSemaphoreValueCount = 1;
    ti[1].pWaitSemaphoreValues = &wait_value;

    err = vk::QueueSubmit(m_device->m_queue, si.size(), si.data(), VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    uint64_t import_value{0}, export_value{0};

    err = vk::GetSemaphoreCounterValueKHR(m_device->handle(), export_semaphore.handle(), &export_value);
    ASSERT_VK_SUCCESS(err);
    ASSERT_EQ(export_value, signal_value);

    err = vk::GetSemaphoreCounterValueKHR(m_device->handle(), import_semaphore.handle(), &import_value);
    ASSERT_VK_SUCCESS(err);
    ASSERT_EQ(import_value, signal_value);
}

TEST_F(PositiveSyncObject, ExternalFence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external fence import and export capability
    auto efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    auto efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    VkResult err;

    // Create a fence to export payload from
    auto efci = LvlInitStruct<VkExportFenceCreateInfoKHR>();
    efci.handleTypes = handle_type;
    auto fci = LvlInitStruct<VkFenceCreateInfo>(&efci);
    vk_testing::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vk_testing::Fence import_fence(*m_device, fci);

    // Export fence payload to an opaque handle
    ExternalHandle ext_fence{};
    err = export_fence.export_handle(ext_fence, handle_type);
    ASSERT_VK_SUCCESS(err);
    err = import_fence.import_handle(ext_fence, handle_type);
    ASSERT_VK_SUCCESS(err);

    // Signal the exported fence and wait on the imported fence
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, export_fence.handle());
    vk::WaitForFences(m_device->device(), 1, &import_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &import_fence.handle());
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, export_fence.handle());
    vk::WaitForFences(m_device->device(), 1, &import_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &import_fence.handle());

    // Signal the imported fence and wait on the exported fence
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence.handle());
    vk::WaitForFences(m_device->device(), 1, &export_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &export_fence.handle());
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence.handle());
    vk::WaitForFences(m_device->device(), 1, &export_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &export_fence.handle());

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(PositiveSyncObject, ExternalFenceSyncFdLoop) {
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external fence import and export capability
    auto efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    auto efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
        return;
    }

    VkResult err;

    // Create a fence to export payload from
    auto efci = LvlInitStruct<VkExportFenceCreateInfoKHR>();
    efci.handleTypes = handle_type;
    auto fci = LvlInitStruct<VkFenceCreateInfo>(&efci);
    vk_testing::Fence export_fence(*m_device, fci);

    fci.pNext = nullptr;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    std::array<vk_testing::Fence, 2> fences;
    fences[0].init(*m_device, fci);
    fences[1].init(*m_device, fci);

    for (uint32_t i = 0; i < 1000; i++) {
        auto submitter = i & 1;
        auto waiter = (~i) & 1;
        fences[submitter].reset();
        err = vk::QueueSubmit(m_device->m_queue, 0, nullptr, fences[submitter].handle());
        ASSERT_VK_SUCCESS(err);

        err = fences[waiter].wait(kWaitTimeout);
        ASSERT_VK_SUCCESS(err);

        vk::QueueSubmit(m_device->m_queue, 0, nullptr, export_fence.handle());
        int fd_handle = -1;
        err = export_fence.export_handle(fd_handle, handle_type);
        ASSERT_VK_SUCCESS(err);
#ifndef VK_USE_PLATFORM_WIN32_KHR
        close(fd_handle);
#endif
    }

    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(PositiveSyncObject, ExternalFenceSubmitCmdBuffer) {
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Check for external fence export capability
    auto efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    auto efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support exporting, skipping test.";
        return;
    }

    VkResult err;

    // Create a fence to export payload from
    auto efci = LvlInitStruct<VkExportFenceCreateInfoKHR>();
    efci.handleTypes = handle_type;
    auto fci = LvlInitStruct<VkFenceCreateInfo>(&efci);
    vk_testing::Fence export_fence(*m_device, fci);

    for (uint32_t i = 0; i < 1000; i++) {
        m_commandBuffer->begin();
        m_commandBuffer->end();

        auto submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, export_fence.handle());
        ASSERT_VK_SUCCESS(err);

        int fd_handle = -1;
        err = export_fence.export_handle(fd_handle, handle_type);
        ASSERT_VK_SUCCESS(err);

#ifndef VK_USE_PLATFORM_WIN32_KHR
        // Wait until command buffer is finished using the exported handle.
        if (fd_handle != -1) {
            struct pollfd fds;
            fds.fd = fd_handle;
            fds.events = POLLIN;
            int timeout_ms = static_cast<int>(kWaitTimeout / 1000000);
            while (true) {
                int ret = poll(&fds, 1, timeout_ms);
                if (ret > 0) {
                    ASSERT_FALSE(fds.revents & (POLLERR | POLLNVAL));
                    break;
                }
                ASSERT_FALSE(ret == 0);                                         // Timeout.
                ASSERT_TRUE(ret == -1 && (errno == EINTR || errno == EAGAIN));  // Retry...
            }
            close(fd_handle);
        }
#else
        // On Windows this test works with MockICD driver and it's fine not to close fd_handle,
        // because it's a dummy value. In case we get access to a real POSIX environment on
        // Windows and VK_KHR_external_fence_fd will be provided through regular graphics drivers,
        // then we need to do a proper POSIX clean-up sequence as shown above.
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
#endif

        m_commandBuffer->reset();
    }

    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(PositiveSyncObject, ThreadNullFenceCollision) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "THREADING ERROR");

    ASSERT_NO_FATAL_FAILURE(Init());

    ThreadTestData data;
    data.device = m_device->device();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Call vk::DestroyFence of VK_NULL_HANDLE repeatedly using multiple threads.
    // There should be no validation error from collision of that non-object.
    std::thread thread(ReleaseNullFence, &data);
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(m_device->device(), VK_NULL_HANDLE, NULL);
    }
    thread.join();

    m_errorMonitor->SetBailout(NULL);
}

TEST_F(PositiveSyncObject, WaitEventThenSet) {
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk_testing::Event event(*m_device, event_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer, &begin_info);

        vk::CmdWaitEvents(command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                          nullptr, 0, nullptr, 0, nullptr);
        vk::CmdResetEvent(command_buffer, event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        vk::EndCommandBuffer(command_buffer);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    { vk::SetEvent(m_device->device(), event.handle()); }

    vk::QueueWaitIdle(queue);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 1, &command_buffer);
}

TEST_F(PositiveSyncObject, DoubleLayoutTransition) {
    TEST_DESCRIPTION("Attempt vkCmdPipelineBarrier with 2 layout transitions of the same image.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageSubresource image_sub = VkImageObj::subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
    VkImageSubresourceRange image_sub_range = VkImageObj::subresource_range(image_sub);

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier image_barriers[] = {image.image_memory_barrier(
            0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_sub_range)};

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, image_barriers);
    }

    {
        VkImageMemoryBarrier image_barriers[] = {
            image.image_memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, image_sub_range),
            image.image_memory_barrier(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, image_sub_range)};

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 2, image_barriers);
    }

    m_commandBuffer->end();
}

TEST_F(PositiveSyncObject, QueueSubmitTimelineSemaphore2Queue) {
    TEST_DESCRIPTION("Signal a timeline semaphore on 2 queues.");
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    vk_testing::Queue *q0 = m_device->graphics_queues()[0];
    vk_testing::Queue *q1 = nullptr;

    if (m_device->graphics_queues().size() > 1) {
        q1 = m_device->graphics_queues()[1];
    }
    if (q1 == nullptr) {
        for (auto *q : m_device->compute_queues()) {
            if (q != q0) {
                q1 = q;
                break;
            }
        }
    }
    if (q1 == nullptr) {
        for (auto *q : m_device->dma_queues()) {
            if (q != q0) {
                q1 = q;
                break;
            }
        }
    }
    if (q1 == nullptr) {
        GTEST_SKIP() << "Test requires 2 queues";
    }

    auto buffer_a = std::make_unique<VkBufferObj>();
    auto buffer_b = std::make_unique<VkBufferObj>();
    auto buffer_c = std::make_unique<VkBufferObj>();
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    buffer_a->init_as_src_and_dst(*m_device, 256, mem_prop);
    buffer_b->init_as_src_and_dst(*m_device, 256, mem_prop);
    buffer_c->init_as_src_and_dst(*m_device, 256, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkCommandPoolObj pool0(m_device, q0->get_family_index());
    VkCommandBufferObj cb0(m_device, &pool0);
    cb0.begin();
    vk::CmdCopyBuffer(cb0.handle(), buffer_a->handle(), buffer_b->handle(), 1, &region);
    cb0.end();

    VkCommandPoolObj pool1(m_device, q1->get_family_index());
    VkCommandBufferObj cb1(m_device, &pool1);
    cb1.begin();
    vk::CmdCopyBuffer(cb1.handle(), buffer_c->handle(), buffer_b->handle(), 1, &region);
    cb1.end();

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    semaphore_create_info.pNext = &semaphore_type_create_info;

    vk_testing::Semaphore semaphore;
    semaphore.init(*m_device, semaphore_create_info);

    // timeline values, Begins will be signaled by host, Ends by the queues
    constexpr uint64_t kQ0Begin = 1;
    constexpr uint64_t kQ0End = 2;
    constexpr uint64_t kQ1Begin = 3;
    constexpr uint64_t kQ1End = 4;

    uint64_t submit_wait_value = kQ0Begin;
    uint64_t submit_signal_value = kQ0End;

    auto timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &submit_wait_value;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &submit_signal_value;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    auto submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cb0.handle();

    // would cause the test to hang at some point, with no output.
    vk::QueueSubmit(q0->handle(), 1, &submit_info, VK_NULL_HANDLE);

    submit_wait_value = kQ1Begin;
    submit_signal_value = kQ1End;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cb1.handle();
    vk::QueueSubmit(q1->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // signal semaphore to allow q0 to proceed
    auto signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    signal_info.semaphore = semaphore.handle();
    signal_info.value = kQ0Begin;
    vk::SignalSemaphoreKHR(m_device->device(), &signal_info);

    // buffer_a is only used by the q0 commands
    uint64_t wait_info_value = kQ0End;
    auto wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &semaphore.handle();
    wait_info.pValues = &wait_info_value;
    vk::WaitSemaphoresKHR(m_device->device(), &wait_info, 1000000000);
    buffer_a.reset();

    // signal semaphore to 3 to allow q1 to proceed
    signal_info.value = kQ1Begin;
    vk::SignalSemaphoreKHR(m_device->device(), &signal_info);

    // buffer_b is used by both q0 and q1, buffer_c is used by q1
    wait_info_value = kQ1End;
    vk::WaitSemaphoresKHR(m_device->device(), &wait_info, 1000000000);
    buffer_b.reset();
    buffer_c.reset();

    vk::DeviceWaitIdle(m_device->device());
}

TEST_F(PositiveSyncObject, ResetQueryPoolFromDifferentCBWithFenceAfter) {
    TEST_DESCRIPTION("Reset query pool from a different command buffer and wait on fence after both are submitted");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkFenceCreateInfo fence_info = LvlInitStruct<VkFenceCreateInfo>();
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vk_testing::Fence ts_fence;
    ts_fence.init(*m_device, fence_info);
    VkFence fence_handle = ts_fence.handle();

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        vk::BeginCommandBuffer(command_buffer[0], &begin_info);
        vk::CmdResetQueryPool(command_buffer[0], query_pool.handle(), 0, 1);
        vk::EndCommandBuffer(command_buffer[0]);

        vk::BeginCommandBuffer(command_buffer[1], &begin_info);
        vk::CmdWriteTimestamp(command_buffer[1], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool.handle(), 0);
        vk::EndCommandBuffer(command_buffer[1]);
    }

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;

    // Begin by resetting the query pool.
    {
        submit_info.pCommandBuffers = &command_buffer[0];
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    // Write a timestamp, and add a fence to be signalled.
    {
        submit_info.pCommandBuffers = &command_buffer[1];
        vk::ResetFences(m_device->device(), 1, &fence_handle);
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence_handle);
    }

    // Reset query pool again.
    {
        submit_info.pCommandBuffers = &command_buffer[0];
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    // Finally, write a second timestamp, but before that, wait for the fence.
    {
        submit_info.pCommandBuffers = &command_buffer[1];
        vk::WaitForFences(m_device->device(), 1, &fence_handle, true, kWaitTimeout);
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, command_buffer);
}

struct FenceSemRaceData {
    VkDevice device{VK_NULL_HANDLE};
    VkSemaphore sem{VK_NULL_HANDLE};
    std::atomic<bool> *bailout{nullptr};
    uint64_t wait_value{0};
    uint64_t timeout{kWaitTimeout};
    uint32_t iterations{100000};
};

void WaitTimelineSem(FenceSemRaceData *data) {
    uint64_t wait_value = data->wait_value;
    auto wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &data->sem;
    wait_info.pValues = &wait_value;

    for (uint32_t i = 0; i < data->iterations; i++, wait_value++) {
        vk::WaitSemaphoresKHR(data->device, &wait_info, data->timeout);
        if (*data->bailout) {
            break;
        }
    }
}

TEST_F(PositiveSyncObject, FenceSemThreadRace) {
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (IsDriver(VK_DRIVER_ID_GOOGLE_SWIFTSHADER)) {
        GTEST_SKIP() << "This test hangs on SwiftShader.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));

    auto fence_ci = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_ci);
    auto fence_handle = fence.handle();

    auto timeline_ci = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    timeline_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timeline_ci.initialValue = 0;

    auto sem_ci = LvlInitStruct<VkSemaphoreCreateInfo>(&timeline_ci);
    vk_testing::Semaphore sem(*m_device, sem_ci);
    auto sem_handle = sem.handle();

    uint64_t signal_value = 1;
    auto timeline_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfo>();
    timeline_info.signalSemaphoreValueCount = 1;
    timeline_info.pSignalSemaphoreValues = &signal_value;

    auto submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_info);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &sem_handle;

    std::atomic<bool> bailout{false};
    FenceSemRaceData data;
    data.device = m_device->device();
    data.sem = sem.handle();
    data.wait_value = signal_value;
    data.bailout = &bailout;
    std::thread thread(WaitTimelineSem, &data);

    m_errorMonitor->SetBailout(&bailout);

    for (uint32_t i = 0; i < data.iterations; i++, signal_value++) {
        auto err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence_handle);
        ASSERT_VK_SUCCESS(err);

        err = fence.wait(data.timeout);
        ASSERT_VK_SUCCESS(err);

        err = vk::ResetFences(m_device->device(), 1, &fence_handle);
        ASSERT_VK_SUCCESS(err);
    }
    m_errorMonitor->SetBailout(nullptr);

    thread.join();
}

TEST_F(PositiveSyncObject, SubmitFenceButWaitIdle) {
    TEST_DESCRIPTION("Submit a CB and Fence but synchronize with vkQueueWaitIdle() (Issue 2756)");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping CmdCopySwapchainImage test";
    }
    uint32_t image_index, image_count;
    vk::GetSwapchainImagesKHR(m_device->handle(), m_swapchain, &image_count, nullptr);
    std::vector<VkImage> swapchainImages(image_count, VK_NULL_HANDLE);
    vk::GetSwapchainImagesKHR(m_device->handle(), m_swapchain, &image_count, swapchainImages.data());

    auto fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    vk_testing::Semaphore sem(*m_device);

    auto pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    std::optional<vk_testing::CommandPool> command_pool(vvl::in_place, *m_device, pool_create_info);

    // create a raw command buffer because we'll just the destroy the pool.
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    auto alloc_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    alloc_info.commandPool = command_pool->handle();
    alloc_info.commandBufferCount = 1;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    auto err = vk::AllocateCommandBuffers(m_device->handle(), &alloc_info, &command_buffer);
    ASSERT_VK_SUCCESS(err);

    err = vk::AcquireNextImageKHR(m_device->handle(), m_swapchain, kWaitTimeout, sem.handle(), VK_NULL_HANDLE, &image_index);
    ASSERT_VK_SUCCESS(err);

    auto begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    err = vk::BeginCommandBuffer(command_buffer, &begin_info);
    ASSERT_VK_SUCCESS(err);

    vk::CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;
    vk::CmdSetViewport(command_buffer, 0, 1, &viewport);
    err = vk::EndCommandBuffer(command_buffer);
    ASSERT_VK_SUCCESS(err);

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence.handle());

    vk::QueueWaitIdle(m_device->m_queue);

    command_pool.reset();
}

struct SemBufferRaceData {
    SemBufferRaceData(VkDeviceObj &dev_) : dev(dev_) {
        auto timeline_ci = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
        timeline_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timeline_ci.initialValue = 0;

        auto sem_ci = LvlInitStruct<VkSemaphoreCreateInfo>(&timeline_ci);
        sem.init(dev, sem_ci);
    }

    VkDeviceObj &dev;
    vk_testing::Semaphore sem;
    uint64_t start_wait_value{0};
    uint64_t timeout_ns{kWaitTimeout};
    uint32_t iterations{10000};
    std::atomic<bool> bailout{false};

    std::unique_ptr<VkBufferObj> thread_buffer;

    virtual VkResult Wait(uint64_t sem_value) = 0;

    virtual VkResult Signal(uint64_t sem_value) {
        auto signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
        signal_info.semaphore = sem.handle();
        signal_info.value = sem_value;
        return vk::SignalSemaphoreKHR(dev.handle(), &signal_info);
    }

    void ThreadFunc() {
        auto wait_value = start_wait_value;

        while (!bailout) {
            auto err = Wait(wait_value);
            if (err != VK_SUCCESS) {
                break;
            }
            auto buffer = std::move(thread_buffer);
            if (!buffer) {
                break;
            }
            buffer.reset();

            err = Signal(wait_value + 1);
            ASSERT_VK_SUCCESS(err);
            wait_value += 3;
        }
    }

    void Run(VkCommandPoolObj &command_pool, ErrorMonitor &error_mon) {
        uint64_t gpu_wait_value, gpu_signal_value;
        auto timeline_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfo>();
        timeline_info.waitSemaphoreValueCount = 1;
        timeline_info.pWaitSemaphoreValues = &gpu_wait_value;
        timeline_info.signalSemaphoreValueCount = 1;
        timeline_info.pSignalSemaphoreValues = &gpu_signal_value;

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        auto submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_info);
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &sem.handle();
        submit_info.pWaitDstStageMask = &wait_stage;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &sem.handle();
        submit_info.commandBufferCount = 1;

        VkResult err;

        start_wait_value = 2;
        error_mon.SetBailout(&bailout);
        std::thread thread(&SemBufferRaceData::ThreadFunc, this);
        for (uint32_t i = 0; i < iterations; i++) {
            VkCommandBufferObj cb(&dev, &command_pool);

            VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            auto buffer = std::make_unique<vk_testing::Buffer>();
            buffer->init_as_dst(dev, (VkDeviceSize)20, reqs);

            // main thread sets up buffer
            // main thread signals 1
            // gpu queue waits for 1
            // gpu queue signals 2
            // sub thread waits for 2
            // sub thread frees buffer
            // sub thread signals 3
            // main thread waits for 3
            uint64_t host_signal_value = (i * 3) + 1;
            gpu_wait_value = host_signal_value;
            gpu_signal_value = (i * 3) + 2;
            uint64_t host_wait_value = (i * 3) + 3;

            cb.begin();
            cb.FillBuffer(buffer->handle(), 0, 12, 0x11111111);
            cb.end();
            thread_buffer = std::move(buffer);

            submit_info.pCommandBuffers = &cb.handle();
            err = vk::QueueSubmit(dev.m_queue, 1, &submit_info, VK_NULL_HANDLE);
            ASSERT_VK_SUCCESS(err);

            err = Signal(host_signal_value);
            ASSERT_VK_SUCCESS(err);

            err = Wait(host_wait_value);
            ASSERT_VK_SUCCESS(err);
        }
        bailout = true;
        // make sure worker thread wakes up.
        err = Signal((iterations + 1) * 3);
        ASSERT_VK_SUCCESS(err);
        thread.join();
        error_mon.SetBailout(nullptr);
        vk::QueueWaitIdle(dev.m_queue);
    }
};

struct WaitTimelineSemThreadData : public SemBufferRaceData {
    WaitTimelineSemThreadData(VkDeviceObj &dev_) : SemBufferRaceData(dev_) {}

    VkResult Wait(uint64_t sem_value) {
        auto wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &sem.handle();
        wait_info.pValues = &sem_value;

        return vk::WaitSemaphoresKHR(dev.handle(), &wait_info, timeout_ns);
    }
};

TEST_F(PositiveSyncObject, WaitTimelineSemThreadRace) {
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    WaitTimelineSemThreadData data(*m_device);

    data.Run(*m_commandPool, *m_errorMonitor);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveSyncObject, WaitTimelineSemaphoreWithWin32HandleRetrieved) {
    TEST_DESCRIPTION("Use vkWaitSemaphores with exported semaphore to wait for the queue");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        // Older AMD driver does not like timeline + export properties combo
        GTEST_SKIP() << "Please update AMD drivers at least to Adrenalin 23.5.2 to run this test. Then remove this check.";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));
    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (!SemaphoreExportImportSupported(gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    // Create exportable timeline semaphore
    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 0;

    auto export_info = LvlInitStruct<VkExportSemaphoreCreateInfo>(&semaphore_type_create_info);
    export_info.handleTypes = handle_type;

    const auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&export_info);
    vk_testing::Semaphore semaphore(*m_device, create_info);

    // This caused original issue: exported semaphore failed to retire queue operations.
    HANDLE win32_handle = NULL;
    ASSERT_VK_SUCCESS(semaphore.export_handle(win32_handle, handle_type));

    // Put semaphore to work
    const uint64_t signal_value = 1;
    auto timeline_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfo>();
    timeline_submit_info.signalSemaphoreValueCount = 1;
    timeline_submit_info.pSignalSemaphoreValues = &signal_value;

    auto submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_submit_info);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    // This wait (with exported semaphore) should properly retire all queue operations
    auto wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &semaphore.handle();
    wait_info.pValues = &signal_value;
    ASSERT_VK_SUCCESS(vk::WaitSemaphores(*m_device, &wait_info, uint64_t(1e10)));
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
