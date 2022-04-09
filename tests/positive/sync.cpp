/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

//
// POSITIVE VALIDATION TESTS
//
// These tests do not expect to encounter ANY validation errors pass only if this is true

TEST_F(VkPositiveLayerTest, ThreadSafetyDisplayObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPropertiesKHR in thread-safety.");

    bool mp_extensions =
        InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME) && InstanceExtensionSupported(VK_KHR_DISPLAY_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR SURFACE and DISPLAY extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkGetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceDisplayPropertiesKHR"));
    auto vkGetDisplayModePropertiesKHR =
        reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(vk::GetInstanceProcAddr(instance(), "vkGetDisplayModePropertiesKHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceDisplayPropertiesKHR != nullptr);
    ASSERT_TRUE(vkGetDisplayModePropertiesKHR != nullptr);

    m_errorMonitor->ExpectSuccess();
    uint32_t prop_count = 0;
    vkGetPhysicalDeviceDisplayPropertiesKHR(gpu(), &prop_count, nullptr);
    if (prop_count != 0) {
        VkDisplayPropertiesKHR display_props = {};
        // Create a VkDisplayKHR object
        vkGetPhysicalDeviceDisplayPropertiesKHR(gpu(), &prop_count, &display_props);
        // Now use this new object in an API call that thread safety will track
        prop_count = 0;
        vkGetDisplayModePropertiesKHR(gpu(), display_props.display, &prop_count, nullptr);
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ThreadSafetyDisplayPlaneObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPlanePropertiesKHR in thread-safety.");

    bool mp_extensions =
        InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME) && InstanceExtensionSupported(VK_KHR_DISPLAY_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR SURFACE and DISPLAY extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkGetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
    auto vkGetDisplayModePropertiesKHR =
        reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(vk::GetInstanceProcAddr(instance(), "vkGetDisplayModePropertiesKHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceDisplayPlanePropertiesKHR != nullptr);
    ASSERT_TRUE(vkGetDisplayModePropertiesKHR != nullptr);

    m_errorMonitor->ExpectSuccess();
    uint32_t prop_count = 0;
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &prop_count, nullptr);
    if (prop_count != 0) {
        // only grab first plane property
        prop_count = 1;
        VkDisplayPlanePropertiesKHR display_plane_props = {};
        // Create a VkDisplayKHR object
        vkGetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &prop_count, &display_plane_props);
        // Now use this new object in an API call
        prop_count = 0;
        vkGetDisplayModePropertiesKHR(gpu(), display_plane_props.currentDisplay, &prop_count, nullptr);
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, Sync2OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        return;
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    uint32_t no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (no_gfx == UINT32_MAX) {
        printf("%s Required queue families not present (non-graphics capable required).\n", kSkipPrefix);
        return;
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx)[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
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
    image_barrier.dstQueueFamilyIndex = no_gfx;

    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, nullptr, &image_barrier);

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx;
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

TEST_F(VkPositiveLayerTest, Sync2OwnershipTranfersBuffer) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        return;
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    uint32_t no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (no_gfx == UINT32_MAX) {
        printf("%s Required queue families not present (non-graphics capable required).\n", kSkipPrefix);
        return;
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx)[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
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
    buffer_barrier.dstQueueFamilyIndex = no_gfx;
    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, &buffer_barrier, nullptr);

    // Transfer it to gfx
    buffer_barrier.srcQueueFamilyIndex = no_gfx;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR;

    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, &buffer_barrier, nullptr);
}

TEST_F(VkPositiveLayerTest, LayoutFromPresentWithoutAccessMemoryRead) {
    // Transition an image away from PRESENT_SRC_KHR without ACCESS_MEMORY_READ
    // in srcAccessMask.

    // The required behavior here was a bit unclear in earlier versions of the
    // spec, but there is no memory dependency required here, so this should
    // work without warnings.

    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
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

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, QueueSubmitSemaphoresAndLayoutTracking) {
    TEST_DESCRIPTION("Submit multiple command buffers with chained semaphore signals and layout transitions");

    m_errorMonitor->ExpectSuccess();

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
    VkSemaphore semaphore1, semaphore2;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore1);
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore2);
    VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    VkSubmitInfo submit_info[3];
    submit_info[0] = LvlInitStruct<VkSubmitInfo>();
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = &cmd_bufs[0];
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore1;
    submit_info[0].waitSemaphoreCount = 0;
    submit_info[0].pWaitDstStageMask = nullptr;
    submit_info[0].pWaitDstStageMask = flags;
    submit_info[1] = LvlInitStruct<VkSubmitInfo>();
    submit_info[1].commandBufferCount = 1;
    submit_info[1].pCommandBuffers = &cmd_bufs[1];
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore1;
    submit_info[1].signalSemaphoreCount = 1;
    submit_info[1].pSignalSemaphores = &semaphore2;
    submit_info[1].pWaitDstStageMask = flags;
    submit_info[2] = LvlInitStruct<VkSubmitInfo>();
    submit_info[2].commandBufferCount = 2;
    submit_info[2].pCommandBuffers = &cmd_bufs[2];
    submit_info[2].waitSemaphoreCount = 1;
    submit_info[2].pWaitSemaphores = &semaphore2;
    submit_info[2].signalSemaphoreCount = 0;
    submit_info[2].pSignalSemaphores = nullptr;
    submit_info[2].pWaitDstStageMask = flags;
    vk::QueueSubmit(m_device->m_queue, 3, submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroySemaphore(m_device->device(), semaphore1, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore2, nullptr);
    m_errorMonitor->VerifyNotFound();
}

// This is a positive test. We used to expect error in this case but spec now allows it
TEST_F(VkPositiveLayerTest, ResetUnsignaledFence) {
    m_errorMonitor->ExpectSuccess();
    vk_testing::Fence testFence;
    VkFenceCreateInfo fenceInfo = LvlInitStruct<VkFenceCreateInfo>();

    ASSERT_NO_FATAL_FAILURE(Init());
    testFence.init(*m_device, fenceInfo);
    VkFence fences[1] = {testFence.handle()};
    VkResult result = vk::ResetFences(m_device->device(), 1, fences);
    ASSERT_VK_SUCCESS(result);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, FenceCreateSignaledWaitHandling) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkResult err;

    // A fence created signaled
    VkFenceCreateInfo fci1 = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    VkFence f1;
    err = vk::CreateFence(m_device->device(), &fci1, nullptr, &f1);
    ASSERT_VK_SUCCESS(err);

    // A fence created not
    VkFenceCreateInfo fci2 = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence f2;
    err = vk::CreateFence(m_device->device(), &fci2, nullptr, &f2);
    ASSERT_VK_SUCCESS(err);

    // Submit the unsignaled fence
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 0, nullptr, 0, nullptr};
    err = vk::QueueSubmit(m_device->m_queue, 1, &si, f2);

    // Wait on both fences, with signaled first.
    VkFence fences[] = {f1, f2};
    vk::WaitForFences(m_device->device(), 2, fences, VK_TRUE, UINT64_MAX);

    // Should have both retired!
    vk::DestroyFence(m_device->device(), f1, nullptr);
    vk::DestroyFence(m_device->device(), f2, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoFencesThreeFrames) {
    TEST_DESCRIPTION(
        "Two command buffers with two separate fences are each run through a Submit & WaitForFences cycle 3 times. This previously "
        "revealed a bug so running this positive test to prevent a regression.");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    static const uint32_t NUM_OBJECTS = 2;
    static const uint32_t NUM_FRAMES = 3;
    VkCommandBuffer cmd_buffers[NUM_OBJECTS] = {};
    VkFence fences[NUM_OBJECTS] = {};

    VkCommandPool cmd_pool;
    VkCommandPoolCreateInfo cmd_pool_ci = LvlInitStruct<VkCommandPoolCreateInfo>();
    cmd_pool_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult err = vk::CreateCommandPool(m_device->device(), &cmd_pool_ci, nullptr, &cmd_pool);
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferAllocateInfo cmd_buf_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buf_info.commandPool = cmd_pool;
    cmd_buf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_info.commandBufferCount = 1;

    VkFenceCreateInfo fence_ci = LvlInitStruct<VkFenceCreateInfo>();
    fence_ci.flags = 0;

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
            err = vk::WaitForFences(m_device->device(), 1, &fences[obj], VK_TRUE, UINT64_MAX);
            ASSERT_VK_SUCCESS(err);
            err = vk::ResetFences(m_device->device(), 1, &fences[obj]);
            ASSERT_VK_SUCCESS(err);
        }
    }
    m_errorMonitor->VerifyNotFound();
    vk::DestroyCommandPool(m_device->device(), cmd_pool, nullptr);
    for (uint32_t i = 0; i < NUM_OBJECTS; ++i) {
        vk::DestroyFence(m_device->device(), fences[i], nullptr);
    }
}
// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceQWI) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues followed by a QueueWaitIdle.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info.pSignalSemaphores = &semaphore;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceQWIFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by a QueueWaitIdle.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info.pSignalSemaphores = &semaphore;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceTwoWFF) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by two consecutive WaitForFences calls on the same fence.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info.pSignalSemaphores = &semaphore;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, TwoQueuesEnsureCorrectRetirementWithWorkStolen) {
    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Test requires two queues, skipping\n", kSkipPrefix);
        return;
    }

    VkResult err;

    m_errorMonitor->ExpectSuccess();

    VkQueue q0 = m_device->m_queue;
    VkQueue q1 = nullptr;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &q1);
    ASSERT_NE(q1, nullptr);

    // An (empty) command buffer. We must have work in the first submission --
    // the layer treats unfenced work differently from fenced work.
    VkCommandPoolCreateInfo cpci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, 0, 0};
    VkCommandPool pool;
    err = vk::CreateCommandPool(m_device->device(), &cpci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);
    VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool,
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
    VkSemaphore s;
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &s);
    ASSERT_VK_SUCCESS(err);

    // First submission, to q0
    VkSubmitInfo s0 = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cb, 1, &s};

    err = vk::QueueSubmit(q0, 1, &s0, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Second submission, to q1, waiting on s
    VkFlags waitmask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // doesn't really matter what this value is.
    VkSubmitInfo s1 = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &s, &waitmask, 0, nullptr, 0, nullptr};

    err = vk::QueueSubmit(q1, 1, &s1, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Wait for q0 idle
    err = vk::QueueWaitIdle(q0);
    ASSERT_VK_SUCCESS(err);

    // Command buffer should have been completed (it was on q0); reset the pool.
    vk::FreeCommandBuffers(m_device->device(), pool, 1, &cb);

    m_errorMonitor->VerifyNotFound();

    // Force device completely idle and clean up resources
    vk::DeviceWaitIdle(m_device->device());
    vk::DestroyCommandPool(m_device->device(), pool, nullptr);
    vk::DestroySemaphore(m_device->device(), s, nullptr);
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence, "
        "followed by a WaitForFences call.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info.pSignalSemaphores = &semaphore;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsSeparateQueuesWithTimelineSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, ordered by a timeline semaphore,"
        " the second having a fence, followed by a WaitForFences call.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        return;
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreTypeCreateInfo semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 0;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info.pSignalSemaphores = &semaphore;
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
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence));
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsOneQueueWithSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, sharing a signal/wait semaphore, the second "
        "having a fence, followed by a WaitForFences call.");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info.pSignalSemaphores = &semaphore;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsOneQueueNullQueueSubmitWithFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, no fences, followed by a third QueueSubmit "
        "with NO SubmitInfos but with a fence, followed by a WaitForFences call.");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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

    vk::QueueSubmit(m_device->m_queue, 0, nullptr, fence);

    VkResult err = vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    ASSERT_VK_SUCCESS(err);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoQueueSubmitsOneQueueOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, the second having a fence, followed by a "
        "WaitForFences call.");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoSubmitInfosWithSemaphoreOneQueueSubmitsOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers each in a separate SubmitInfo sent in a single QueueSubmit call followed by a WaitForFences call.");
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
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
        submit_info[0].pSignalSemaphores = &semaphore;
        submit_info[0].waitSemaphoreCount = 0;
        submit_info[0].pWaitSemaphores = nullptr;
        submit_info[0].pWaitDstStageMask = 0;

        submit_info[1] = LvlInitStruct<VkSubmitInfo>();
        submit_info[1].commandBufferCount = 1;
        submit_info[1].pCommandBuffers = &command_buffer[1];
        submit_info[1].waitSemaphoreCount = 1;
        submit_info[1].pWaitSemaphores = &semaphore;
        submit_info[1].pWaitDstStageMask = flags;
        submit_info[1].signalSemaphoreCount = 0;
        submit_info[1].pSignalSemaphores = nullptr;
        vk::QueueSubmit(m_device->m_queue, 2, &submit_info[0], fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, LongSemaphoreChain) {
    m_errorMonitor->ExpectSuccess();

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
    VkFence fence;
    err = vk::CreateFence(m_device->device(), &fci, nullptr, &fence);
    ASSERT_VK_SUCCESS(err);
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &semaphores.back(), &flags, 0, nullptr, 0, nullptr};
    err = vk::QueueSubmit(m_device->m_queue, 1, &si, fence);
    ASSERT_VK_SUCCESS(err);

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    for (auto semaphore : semaphores) vk::DestroySemaphore(m_device->device(), semaphore, nullptr);

    vk::DestroyFence(m_device->device(), fence, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ExternalSemaphore) {
#ifdef _WIN32
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    // Check for external semaphore instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for external semaphore device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR, nullptr,
                                                    handle_type};
    VkExternalSemaphorePropertiesKHR esp = {VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR, nullptr};
    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
        reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR>(vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR"));
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External semaphore does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    VkResult err;
    m_errorMonitor->ExpectSuccess();

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfoKHR esci = {VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR, nullptr, handle_type};
    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &esci, 0};

    VkSemaphore export_semaphore;
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &export_semaphore);
    ASSERT_VK_SUCCESS(err);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    VkSemaphore import_semaphore;
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &import_semaphore);
    ASSERT_VK_SUCCESS(err);

#ifdef _WIN32
    // Export semaphore payload to an opaque handle
    HANDLE handle = nullptr;
    VkSemaphoreGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_semaphore,
                                            handle_type};
    auto vkGetSemaphoreWin32HandleKHR = reinterpret_cast<PFN_vkGetSemaphoreWin32HandleKHR>(
        vk::GetDeviceProcAddr(m_device->device(), "vkGetSemaphoreWin32HandleKHR"));
    err = vkGetSemaphoreWin32HandleKHR(m_device->device(), &ghi, &handle);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above
    VkImportSemaphoreWin32HandleInfoKHR ihi = {
        VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR, nullptr, import_semaphore, 0, handle_type, handle, nullptr};
    auto vkImportSemaphoreWin32HandleKHR = reinterpret_cast<PFN_vkImportSemaphoreWin32HandleKHR>(
        vk::GetDeviceProcAddr(m_device->device(), "vkImportSemaphoreWin32HandleKHR"));
    err = vkImportSemaphoreWin32HandleKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#else
    // Export semaphore payload to an opaque handle
    int fd = 0;
    VkSemaphoreGetFdInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR, nullptr, export_semaphore, handle_type};
    auto vkGetSemaphoreFdKHR =
        reinterpret_cast<PFN_vkGetSemaphoreFdKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkGetSemaphoreFdKHR"));
    err = vkGetSemaphoreFdKHR(m_device->device(), &ghi, &fd);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above
    VkImportSemaphoreFdInfoKHR ihi = {
        VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR, nullptr, import_semaphore, 0, handle_type, fd};
    auto vkImportSemaphoreFdKHR =
        reinterpret_cast<PFN_vkImportSemaphoreFdKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkImportSemaphoreFdKHR"));
    err = vkImportSemaphoreFdKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#endif

    // Signal the exported semaphore and wait on the imported semaphore
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo si[] = {
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, &flags, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &import_semaphore, &flags, 0, nullptr, 0, nullptr},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, &flags, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &import_semaphore, &flags, 0, nullptr, 0, nullptr},
    };
    err = vk::QueueSubmit(m_device->m_queue, 4, si, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    if (m_device->phy().features().sparseBinding) {
        // Signal the imported semaphore and wait on the exported semaphore
        VkBindSparseInfo bi[] = {
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &import_semaphore},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &export_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &import_semaphore},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &export_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
        };
        err = vk::QueueBindSparse(m_device->m_queue, 4, bi, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
    }

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vk::DestroySemaphore(m_device->device(), export_semaphore, nullptr);
    vk::DestroySemaphore(m_device->device(), import_semaphore, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ExternalFence) {
#ifdef _WIN32
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    // Check for external fence instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External fence extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for external fence device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    } else {
        printf("%s External fence extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfoKHR efi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR, nullptr, handle_type};
    VkExternalFencePropertiesKHR efp = {VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES_KHR, nullptr};
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR"));
    vkGetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External fence does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    VkResult err;
    m_errorMonitor->ExpectSuccess();

    // Create a fence to export payload from
    VkFence export_fence;
    {
        VkExportFenceCreateInfoKHR efci = {VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO_KHR, nullptr, handle_type};
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, &efci, 0};
        err = vk::CreateFence(m_device->device(), &fci, nullptr, &export_fence);
        ASSERT_VK_SUCCESS(err);
    }

    // Create a fence to import payload into
    VkFence import_fence;
    {
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        err = vk::CreateFence(m_device->device(), &fci, nullptr, &import_fence);
        ASSERT_VK_SUCCESS(err);
    }

#ifdef _WIN32
    // Export fence payload to an opaque handle
    HANDLE handle = nullptr;
    {
        VkFenceGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceWin32HandleKHR =
            reinterpret_cast<PFN_vkGetFenceWin32HandleKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkGetFenceWin32HandleKHR"));
        err = vkGetFenceWin32HandleKHR(m_device->device(), &ghi, &handle);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceWin32HandleInfoKHR ifi = {
            VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR, nullptr, import_fence, 0, handle_type, handle, nullptr};
        auto vkImportFenceWin32HandleKHR = reinterpret_cast<PFN_vkImportFenceWin32HandleKHR>(
            vk::GetDeviceProcAddr(m_device->device(), "vkImportFenceWin32HandleKHR"));
        err = vkImportFenceWin32HandleKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#else
    // Export fence payload to an opaque handle
    int fd = 0;
    {
        VkFenceGetFdInfoKHR gfi = {VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceFdKHR = reinterpret_cast<PFN_vkGetFenceFdKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkGetFenceFdKHR"));
        err = vkGetFenceFdKHR(m_device->device(), &gfi, &fd);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceFdInfoKHR ifi = {VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR, nullptr, import_fence, 0, handle_type, fd};
        auto vkImportFenceFdKHR =
            reinterpret_cast<PFN_vkImportFenceFdKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkImportFenceFdKHR"));
        err = vkImportFenceFdKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#endif

    // Signal the exported fence and wait on the imported fence
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, export_fence);
    vk::WaitForFences(m_device->device(), 1, &import_fence, VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &import_fence);
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, export_fence);
    vk::WaitForFences(m_device->device(), 1, &import_fence, VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &import_fence);

    // Signal the imported fence and wait on the exported fence
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    vk::WaitForFences(m_device->device(), 1, &export_fence, VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &export_fence);
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    vk::WaitForFences(m_device->device(), 1, &export_fence, VK_TRUE, 1000000000);
    vk::ResetFences(m_device->device(), 1, &export_fence);

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vk::DestroyFence(m_device->device(), export_fence, nullptr);
    vk::DestroyFence(m_device->device(), import_fence, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ThreadNullFenceCollision) {
    test_platform_thread thread;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "THREADING ERROR");

    ASSERT_NO_FATAL_FAILURE(Init());

    struct thread_data_struct data;
    data.device = m_device->device();
    bool bailout = false;
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Call vk::DestroyFence of VK_NULL_HANDLE repeatedly using multiple threads.
    // There should be no validation error from collision of that non-object.
    test_platform_thread_create(&thread, ReleaseNullFence, (void *)&data);
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(m_device->device(), VK_NULL_HANDLE, nullptr);
    }
    test_platform_thread_join(thread, nullptr);

    m_errorMonitor->SetBailout(nullptr);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, WaitEventThenSet) {
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");

    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer, &begin_info);

        vk::CmdWaitEvents(command_buffer, 1, &event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0,
                          nullptr, 0, nullptr);
        vk::CmdResetEvent(command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
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
    { vk::SetEvent(m_device->device(), event); }

    vk::QueueWaitIdle(queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, DoubleLayoutTransition) {
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

        m_errorMonitor->ExpectSuccess();
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, image_barriers);
        m_errorMonitor->VerifyNotFound();
    }

    {
        VkImageMemoryBarrier image_barriers[] = {
            image.image_memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, image_sub_range),
            image.image_memory_barrier(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, image_sub_range)};

        m_errorMonitor->ExpectSuccess();
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 2, image_barriers);
        m_errorMonitor->VerifyNotFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, QueueSubmitTimelineSemaphore2Queue) {
    TEST_DESCRIPTION("Signal a timeline semaphore on 2 queues.");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        return;
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
    }
    auto fpWaitSemaphores =
        reinterpret_cast<PFN_vkWaitSemaphoresKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkWaitSemaphoresKHR"));
    auto fpSignalSemaphore =
        reinterpret_cast<PFN_vkSignalSemaphoreKHR>(vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR"));

    vk_testing::Queue *q0 = m_device->graphics_queues()[0];
    vk_testing::Queue *q1 = nullptr;

    if (m_device->graphics_queues().size() > 1) {
        q1 = m_device->graphics_queues()[1];
    }
    if (q1 == nullptr) {
        for (auto *q: m_device->compute_queues()) {
            if (q != q0) {
                q1 = q;
                break;
            }
        }
    }
    if (q1 == nullptr) {
        for (auto *q: m_device->dma_queues()) {
            if (q != q0) {
                q1 = q;
                break;
            }
        }
    }
    if (q1 == nullptr) {
        printf("%s Test requires 2 queues, skipping test\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();
    auto buffer_a = layer_data::make_unique<VkBufferObj>();
    auto buffer_b = layer_data::make_unique<VkBufferObj>();
    auto buffer_c = layer_data::make_unique<VkBufferObj>();
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
    m_errorMonitor->VerifyNotFound();

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

    // NOTE: this test calls VerifyNotFound after every call, because (now fixed) VVL false positives
    // would cause the test to hang at some point, with no output.
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(q0->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    submit_wait_value = kQ1Begin;
    submit_signal_value = kQ1End;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cb1.handle();
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(q1->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    // signal semaphore to allow q0 to proceed
    auto signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    signal_info.semaphore = semaphore.handle();
    signal_info.value = kQ0Begin;
    m_errorMonitor->ExpectSuccess();
    fpSignalSemaphore(m_device->device(), &signal_info);
    m_errorMonitor->VerifyNotFound();

    // buffer_a is only used by the q0 commands
    uint64_t wait_info_value = kQ0End;
    auto wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &semaphore.handle();
    wait_info.pValues = &wait_info_value;
    m_errorMonitor->ExpectSuccess();
    fpWaitSemaphores(m_device->device(), &wait_info, 1000000000);
    buffer_a.reset();
    m_errorMonitor->VerifyNotFound();

    // signal semaphore to 3 to allow q1 to proceed
    signal_info.value = kQ1Begin;
    m_errorMonitor->ExpectSuccess();
    fpSignalSemaphore(m_device->device(), &signal_info);
    m_errorMonitor->VerifyNotFound();

    // buffer_b is used by both q0 and q1, buffer_c is used by q1
    wait_info_value = kQ1End;
    m_errorMonitor->ExpectSuccess();
    fpWaitSemaphores(m_device->device(), &wait_info, 1000000000);
    buffer_b.reset();
    buffer_c.reset();
    m_errorMonitor->VerifyNotFound();

    vk::DeviceWaitIdle(m_device->device());
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ResetQueryPoolFromDifferentCBWithFenceAfter) {
    TEST_DESCRIPTION("Reset query pool from a different command buffer and wait on fence after both are submitted");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        printf("%s Device graphic queue has timestampValidBits of 0, skipping.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

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
        vk::WaitForFences(m_device->device(), 1, &fence_handle, true, std::numeric_limits<uint64_t>::max());
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, command_buffer);

    m_errorMonitor->VerifyNotFound();
}
