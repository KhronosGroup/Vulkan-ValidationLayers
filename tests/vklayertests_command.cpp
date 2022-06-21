/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, InvalidCommandPoolConsistency) {
    TEST_DESCRIPTION("Allocate command buffers from one command pool and attempt to delete them from another.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeCommandBuffers-pCommandBuffers-parent");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkCommandPool command_pool_one;
    VkCommandPool command_pool_two;

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool_one);

    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool_two);

    VkCommandBuffer cb;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool_one;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &cb);

    vk::FreeCommandBuffers(m_device->device(), command_pool_two, 1, &cb);

    m_errorMonitor->VerifyFound();

    vk::DestroyCommandPool(m_device->device(), command_pool_one, NULL);
    vk::DestroyCommandPool(m_device->device(), command_pool_two, NULL);
}

TEST_F(VkLayerTest, InvalidSecondaryCommandBufferBarrier) {
    TEST_DESCRIPTION("Add an invalid image barrier in a secondary command buffer");
    ASSERT_NO_FATAL_FAILURE(Init());

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_ACCESS_SHADER_WRITE_BIT,
                               VK_ACCESS_SHADER_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 1, &dep};
    VkRenderPass rp;

    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    // Second image that img_barrier will incorrectly use
    VkImageObj image2(m_device);
    image2.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                                           nullptr,
                                           rp,
                                           0,
                                           VK_NULL_HANDLE,  // Set to NULL FB handle intentionally to flesh out any errors
                                           VK_FALSE,
                                           0,
                                           0};
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                                     &cbii};
    vk::BeginCommandBuffer(secondary.handle(), &cbbi);
    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image2.handle();  // Image mis-matches with FB image
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(secondary.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    secondary.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-image-04073");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, DynamicDepthBiasNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Depth Bias dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic depth bias
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic depth bias state not set for this command buffer");
    VKTriangleTest(BsoFailDepthBias);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicLineWidthNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Line Width dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic line width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic line width state not set for this command buffer");
    VKTriangleTest(BsoFailLineWidth);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicLineStippleNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Line Stipple dynamic state is required but not correctly bound.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto line_rasterization_features = LvlInitStruct<VkPhysicalDeviceLineRasterizationFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&line_rasterization_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!line_rasterization_features.stippledBresenhamLines || !line_rasterization_features.bresenhamLines) {
        printf("%sStipple Bresenham lines not supported; skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic line stipple state not set for this command buffer");
    VKTriangleTest(BsoFailLineStipple);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicViewportNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Viewport dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic viewport state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Dynamic viewport(s) 0 are used by pipeline state object, but were not provided");
    VKTriangleTest(BsoFailViewport);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicScissorNotBound) {
    TEST_DESCRIPTION("Run a simple draw calls to validate failure when Scissor dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic scissor state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Dynamic scissor(s) 0 are used by pipeline state object, but were not provided");
    VKTriangleTest(BsoFailScissor);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicBlendConstantsNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Blend Constants dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic blend constant state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic blend constants state not set for this command buffer");
    VKTriangleTest(BsoFailBlend);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicDepthBoundsNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Depth Bounds dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if (!m_device->phy().features().depthBounds) {
        printf("%s Device does not support depthBounds test; skipped.\n", kSkipPrefix);
        return;
    }
    // Dynamic depth bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic depth bounds state not set for this command buffer");
    VKTriangleTest(BsoFailDepthBounds);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicStencilReadNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Stencil Read dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic stencil read mask
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic stencil read mask state not set for this command buffer");
    VKTriangleTest(BsoFailStencilReadMask);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicStencilWriteNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Stencil Write dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic stencil write mask
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic stencil write mask state not set for this command buffer");
    VKTriangleTest(BsoFailStencilWriteMask);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicStencilRefNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Stencil Ref dynamic state is required but not correctly bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Dynamic stencil reference
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic stencil reference state not set for this command buffer");
    VKTriangleTest(BsoFailStencilReference);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IndexBufferNotBound) {
    TEST_DESCRIPTION("Run an indexed draw call without an index buffer bound.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Index buffer object not bound to this command buffer when Indexed ");
    VKTriangleTest(BsoFailIndexBuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IndexBufferBadSize) {
    TEST_DESCRIPTION("Run indexed draw call with bad index buffer size.");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdDrawIndexed(): index size ");
    VKTriangleTest(BsoFailIndexBufferBadSize);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IndexBufferBadOffset) {
    TEST_DESCRIPTION("Run indexed draw call with bad index buffer offset.");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdDrawIndexed(): index size ");
    VKTriangleTest(BsoFailIndexBufferBadOffset);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IndexBufferBadBindSize) {
    TEST_DESCRIPTION("Run bind index buffer with a size greater than the index buffer.");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdDrawIndexed(): index size ");
    VKTriangleTest(BsoFailIndexBufferBadMapSize);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IndexBufferBadBindOffset) {
    TEST_DESCRIPTION("Run bind index buffer with an offset greater than the size of the index buffer.");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdDrawIndexed(): index size ");
    VKTriangleTest(BsoFailIndexBufferBadMapOffset);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingClearAttachment) {
    TEST_DESCRIPTION("Points to a wrong colorAttachment index in a VkClearAttachment structure passed to vkCmdClearAttachments");
    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-02501");

    VKTriangleTest(BsoFailCmdClearAttachments);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SecondaryCommandbufferAsPrimary) {
    TEST_DESCRIPTION("Create a secondary command buffer and pass it to QueueSubmit.");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-00075");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    secondary.ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    secondary.end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &secondary.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Sync2SecondaryCommandbufferAsPrimary) {
    TEST_DESCRIPTION("Create a secondary command buffer and pass it to QueueSubmit2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    } else {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
    auto fpQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferSubmitInfo-commandBuffer-03890");

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    secondary.ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    secondary.end();

    auto cb_info = lvl_init_struct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = secondary.handle();

    auto submit_info = lvl_init_struct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CommandBufferTwoSubmits) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "was begun w/ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set, but has been submitted");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // We luck out b/c by default the framework creates CB w/ the
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set
    m_commandBuffer->begin();
    m_commandBuffer->ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    m_commandBuffer->end();

    // Bypass framework since it does the waits automatically
    VkResult err = VK_SUCCESS;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);
    vk::QueueWaitIdle(m_device->m_queue);

    // Cause validation error by re-submitting cmd buffer that should only be
    // submitted once
    err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Sync2CommandBufferTwoSubmits) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    } else {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
    auto fpQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "was begun w/ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set, but has been submitted");
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // We luck out b/c by default the framework creates CB w/ the
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set
    m_commandBuffer->begin();
    m_commandBuffer->ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    m_commandBuffer->end();

    // Bypass framework since it does the waits automatically
    VkResult err = VK_SUCCESS;
    auto cb_info = lvl_init_struct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = m_commandBuffer->handle();

    auto submit_info = lvl_init_struct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    err = fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);
    vk::QueueWaitIdle(m_device->m_queue);

    // Cause validation error by re-submitting cmd buffer that should only be
    // submitted once
    err = fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidPushConstants) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineLayout pipeline_layout;
    VkPushConstantRange pc_range = {};
    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &pc_range;

    //
    // Check for invalid push constant ranges in pipeline layouts.
    //
    struct PipelineLayoutTestCase {
        VkPushConstantRange const range;
        char const *msg;
    };

    const uint32_t too_big = m_device->props.limits.maxPushConstantsSize + 0x4;
    const std::array<PipelineLayoutTestCase, 10> range_tests = {{
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, 0}, "VUID-VkPushConstantRange-size-00296"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, 1}, "VUID-VkPushConstantRange-size-00297"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 4, 1}, "VUID-VkPushConstantRange-size-00297"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 4, 0}, "VUID-VkPushConstantRange-size-00296"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 1, 4}, "VUID-VkPushConstantRange-offset-00295"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, too_big}, "VUID-VkPushConstantRange-size-00298"},
        {{VK_SHADER_STAGE_VERTEX_BIT, too_big, too_big}, "VUID-VkPushConstantRange-offset-00294"},
        {{VK_SHADER_STAGE_VERTEX_BIT, too_big, 4}, "VUID-VkPushConstantRange-offset-00294"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0xFFFFFFF0, 0x00000020}, "VUID-VkPushConstantRange-offset-00294"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0x00000020, 0xFFFFFFF0}, "VUID-VkPushConstantRange-size-00298"},
    }};

    // Check for invalid offset and size
    for (const auto &iter : range_tests) {
        pc_range = iter.range;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.msg);
        vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    // Check for invalid stage flag
    pc_range.offset = 0;
    pc_range.size = 16;
    pc_range.stageFlags = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-stageFlags-requiredbitmask");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // Check for duplicate stage flags in a list of push constant ranges.
    // A shader can only have one push constant block and that block is mapped
    // to the push constant range that has that shader's stage flag set.
    // The shader's stage flag can only appear once in all the ranges, so the
    // implementation can find the one and only range to map it to.
    const uint32_t ranges_per_test = 5;
    struct DuplicateStageFlagsTestCase {
        VkPushConstantRange const ranges[ranges_per_test];
        std::vector<char const *> const msg;
    };
    // Overlapping ranges are OK, but a stage flag can appear only once.
    const std::array<DuplicateStageFlagsTestCase, 3> duplicate_stageFlags_tests = {
        {
            {{{VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4}},
             {
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 0 and 1.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 0 and 2.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 0 and 3.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 0 and 4.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 1 and 2.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 1 and 3.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 1 and 4.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 2 and 3.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 2 and 4.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 3 and 4.",
             }},
            {{{VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4},
              {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4}},
             {
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 0 and 3.",
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 1 and 4.",
             }},
            {{{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
              {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4}},
             {
                 "vkCreatePipelineLayout() Duplicate stage flags found in ranges 2 and 3.",
             }},
        },
    };

    for (const auto &iter : duplicate_stageFlags_tests) {
        pipeline_layout_ci.pPushConstantRanges = iter.ranges;
        pipeline_layout_ci.pushConstantRangeCount = ranges_per_test;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.msg.begin(), iter.msg.end());
        vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    //
    // CmdPushConstants tests
    //

    // Setup a pipeline layout with ranges: [0,32) [16,80)
    const std::vector<VkPushConstantRange> pc_range2 = {{VK_SHADER_STAGE_VERTEX_BIT, 16, 64},
                                                        {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 32}};
    const VkPipelineLayoutObj pipeline_layout_obj(m_device, {}, pc_range2);

    const uint8_t dummy_values[100] = {};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Check for invalid stage flag
    // Note that VU 00996 isn't reached due to parameter validation
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-stageFlags-requiredbitmask");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), 0, 0, 16, dummy_values);
    m_errorMonitor->VerifyFound();

    // Positive tests for the overlapping ranges
    m_errorMonitor->ExpectSuccess();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16,
                         dummy_values);
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->ExpectSuccess();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_VERTEX_BIT, 32, 48, dummy_values);
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->ExpectSuccess();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 16, 16, dummy_values);
    m_errorMonitor->VerifyNotFound();

    // Wrong cmd stages for extant range
    // No range for all cmd stages -- "VUID-vkCmdPushConstants-offset-01795" VUID-vkCmdPushConstants-offset-01795
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-01795");
    // Missing cmd stages for found overlapping range -- "VUID-vkCmdPushConstants-offset-01796" VUID-vkCmdPushConstants-offset-01796
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-01796");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_GEOMETRY_BIT, 0, 16,
                         dummy_values);
    m_errorMonitor->VerifyFound();

    // Wrong no extant range
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-01795");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 80, 4,
                         dummy_values);
    m_errorMonitor->VerifyFound();

    // Wrong overlapping extent
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-01795");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 20, dummy_values);
    m_errorMonitor->VerifyFound();

    // Wrong stage flags for valid overlapping range
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-01796");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_VERTEX_BIT, 16, 16, dummy_values);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, NoBeginCommandBuffer) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-commandBuffer-00059");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkCommandBufferObj commandBuffer(m_device, m_commandPool);
    // Call EndCommandBuffer() w/o calling BeginCommandBuffer()
    vk::EndCommandBuffer(commandBuffer.handle());

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SecondaryCommandBufferRerecordedExplicitReset) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "was destroyed or rerecorded");

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.begin();
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());

    // rerecording of secondary
    secondary.reset();  // explicit reset here.
    secondary.begin();
    secondary.end();

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SecondaryCommandBufferRerecordedNoReset) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "was destroyed or rerecorded");

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.begin();
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());

    // rerecording of secondary
    secondary.begin();  // implicit reset in begin
    secondary.end();

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CascadedInvalidation) {
    ASSERT_NO_FATAL_FAILURE(Init());

    VkEventCreateInfo eci = LvlInitStruct<VkEventCreateInfo>();
    eci.flags = 0;
    VkEvent event;
    vk::CreateEvent(m_device->device(), &eci, nullptr, &event);

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    vk::CmdSetEvent(secondary.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->end();

    // destroying the event should invalidate both primary and secondary CB
    vk::DestroyEvent(m_device->device(), event, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkEvent");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CommandBufferResetErrors) {
    // Cause error due to Begin while recording CB
    // Then cause 2 errors for attempting to reset CB w/o having
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT set for the pool from
    // which CBs were allocated. Note that this bit is off by default.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBeginCommandBuffer-commandBuffer-00049");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Calls AllocateCommandBuffers
    VkCommandBufferObj commandBuffer(m_device, m_commandPool);

    // Force the failure by setting the Renderpass and Framebuffer fields with (fake) data
    VkCommandBufferInheritanceInfo cmd_buf_hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    VkCommandBufferBeginInfo cmd_buf_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

    // Begin CB to transition to recording state
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    // Can't re-begin. This should trigger error
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetCommandBuffer-commandBuffer-00046");
    VkCommandBufferResetFlags flags = 0;  // Don't care about flags for this test
    // Reset attempt will trigger error due to incorrect CommandPool state
    vk::ResetCommandBuffer(commandBuffer.handle(), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBeginCommandBuffer-commandBuffer-00050");
    // Transition CB to RECORDED state
    vk::EndCommandBuffer(commandBuffer.handle());
    // Now attempting to Begin will implicitly reset, which triggers error
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CommandBufferPrimaryFlags) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // Calls AllocateCommandBuffers
    VkCommandBufferObj commandBuffer(m_device, m_commandPool);

    VkCommandBufferBeginInfo cmd_buf_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBeginCommandBuffer-commandBuffer-02840");
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearColorAttachmentsOutsideRenderPass) {
    // Call CmdClearAttachmentss outside of an active RenderPass

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdClearAttachments: This call must be issued inside an active render pass");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Start no RenderPass
    m_commandBuffer->begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearColorAttachmentsZeroLayercount) {
    TEST_DESCRIPTION("Call CmdClearAttachments with a pRect having a layerCount of zero.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-layerCount-01934");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}};
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearColorAttachmentsZeroExtent) {
    TEST_DESCRIPTION("Call CmdClearAttachments with a pRect having a rect2D extent of zero.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    clear_rect.rect.extent = {0, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-rect-02682");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    clear_rect.rect.extent = {1, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-rect-02683");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearAttachmentsInvalidAspectMasks) {
    TEST_DESCRIPTION("Check VkClearAttachment invalid aspect masks.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.clearValue.color.float32[0] = 0;
    attachment.clearValue.color.float32[1] = 0;
    attachment.clearValue.color.float32[2] = 0;
    attachment.clearValue.color.float32[3] = 0;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-00020");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-00020");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    attachment.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-02246");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-02246");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearAttachmentsImplicitCheck) {
    TEST_DESCRIPTION("Check VkClearAttachment implicit VUs.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment color_attachment;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    color_attachment.aspectMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-requiredbitmask");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    color_attachment.aspectMask = 0xffffffff;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-parameter");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearColorAttachmentsDepthStencil) {
    TEST_DESCRIPTION("Call CmdClearAttachments with invalid depth/stencil aspect masks.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Creates a color attachment
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.clearValue.color.float32[0] = 0;
    attachment.clearValue.color.float32[1] = 0;
    attachment.clearValue.color.float32[2] = 0;
    attachment.clearValue.color.float32[3] = 0;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_errorMonitor->ExpectSuccess();
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-02502");
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-02503");
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExecuteCommandsPrimaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a primary command buffer (should only be secondary)");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // An empty primary command buffer
    VkCommandBufferObj cb(m_device, m_commandPool);
    cb.begin();
    cb.end();

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    VkCommandBuffer handle = cb.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00088");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &handle);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("All elements of pCommandBuffers must not be in the pending state");

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ExecuteCommandsToSecondaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands to a Secondary command buffer");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkCommandBufferObj main_cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferObj secondary_cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.begin();
    secondary_cb.end();

    main_cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-bufferlevel");
    vk::CmdExecuteCommands(main_cb.handle(), 1, &secondary_cb.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidVertexAttributeAlignment) {
    TEST_DESCRIPTION("Check for proper aligment of attribAddress which depends on a bound pipeline and on a bound vertex buffer");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPipelineLayoutObj pipeline_layout(m_device);

    struct VboEntry {
        uint16_t input0[2];
        uint32_t input1;
        float input2[4];
    };

    const unsigned vbo_entry_count = 3;
    const VboEntry vbo_data[vbo_entry_count] = {};

    VkConstantBufferObj vbo(m_device, static_cast<int>(sizeof(VboEntry) * vbo_entry_count),
                            reinterpret_cast<const void *>(vbo_data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkVertexInputBindingDescription input_binding;
    input_binding.binding = 0;
    input_binding.stride = sizeof(VboEntry);
    input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription input_attribs[3];

    input_attribs[0].binding = 0;
    // Location switch between attrib[0] and attrib[1] is intentional
    input_attribs[0].location = 1;
    input_attribs[0].format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    input_attribs[0].offset = offsetof(VboEntry, input1);

    input_attribs[1].binding = 0;
    input_attribs[1].location = 0;
    input_attribs[1].format = VK_FORMAT_R16G16_UNORM;
    input_attribs[1].offset = offsetof(VboEntry, input0);

    input_attribs[2].binding = 0;
    input_attribs[2].location = 2;
    input_attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    input_attribs[2].offset = offsetof(VboEntry, input2);

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        layout(location = 1) in vec4 input1;
        layout(location = 2) in vec4 input2;
        void main(){
           gl_Position = input1 + input2;
           gl_Position.xy += input0;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe1(m_device);
    pipe1.AddDefaultColorAttachment();
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddVertexInputBindings(&input_binding, 1);
    pipe1.AddVertexInputAttribs(&input_attribs[0], 3);
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);
    pipe1.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    input_binding.stride = 6;

    VkPipelineObj pipe2(m_device);
    pipe2.AddDefaultColorAttachment();
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddVertexInputBindings(&input_binding, 1);
    pipe2.AddVertexInputAttribs(&input_attribs[0], 3);
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);
    pipe2.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Test with invalid buffer offset
    VkDeviceSize offset = 1;
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 1
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 2
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Test with invalid buffer stride
    offset = 0;
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 0
    // Attribute[1] is aligned properly even with a wrong stride
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 2
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, NonSimultaneousSecondaryMarksPrimary) {
    ASSERT_NO_FATAL_FAILURE(Init());
    const char *simultaneous_use_message = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBufferSimultaneousUse";

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.begin();
    secondary.end();

    VkCommandBufferBeginInfo cbbi = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        nullptr,
    };

    m_commandBuffer->begin(&cbbi);
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, simultaneous_use_message);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SimultaneousUseSecondaryTwoExecutes) {
    ASSERT_NO_FATAL_FAILURE(Init());

    const char *simultaneous_use_message = "VUID-vkCmdExecuteCommands-pCommandBuffers-00092";

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inh = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, &inh};

    secondary.begin(&cbbi);
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, simultaneous_use_message);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SimultaneousUseSecondarySingleExecute) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // variation on previous test executing the same CB twice in the same
    // CmdExecuteCommands call

    const char *simultaneous_use_message = "VUID-vkCmdExecuteCommands-pCommandBuffers-00093";

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inh = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, &inh};

    secondary.begin(&cbbi);
    secondary.end();

    m_commandBuffer->begin();
    VkCommandBuffer cbs[] = {secondary.handle(), secondary.handle()};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, simultaneous_use_message);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 2, cbs);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SimultaneousUseOneShot) {
    TEST_DESCRIPTION("Submit the same command buffer twice in one submit looking for simultaneous use and one time submit errors");
    const char *simultaneous_use_message = "is already in use and is not marked for simultaneous use";
    const char *one_shot_message = "VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set, but has been submitted";
    ASSERT_NO_FATAL_FAILURE(Init());

    VkCommandBuffer cmd_bufs[2];
    VkCommandBufferAllocateInfo alloc_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 2;
    alloc_info.commandPool = m_commandPool->handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &alloc_info, cmd_bufs);

    VkCommandBufferBeginInfo cb_binfo = LvlInitStruct<VkCommandBufferBeginInfo>();
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    vk::BeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(cmd_bufs[0], 0, 1, &viewport);
    vk::EndCommandBuffer(cmd_bufs[0]);
    VkCommandBuffer duplicates[2] = {cmd_bufs[0], cmd_bufs[0]};

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = duplicates;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, simultaneous_use_message);
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    // Set one time use and now look for one time submit
    duplicates[0] = duplicates[1] = cmd_bufs[1];
    cb_binfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    vk::CmdSetViewport(cmd_bufs[1], 0, 1, &viewport);
    vk::EndCommandBuffer(cmd_bufs[1]);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00071");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, one_shot_message);
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, DrawTimeImageViewTypeMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "requires an image view of type VK_IMAGE_VIEW_TYPE_3D");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkTextureObj texture(m_device, nullptr);
    VkSamplerObj sampler(m_device);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkResult err = pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_commandBuffer->BindDescriptorSet(descriptorSet);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    // error produced here.
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DrawTimeImageMultisampleMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when a multisampled images are consumed via singlesample images types in the shader, or "
        "vice versa.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "requires bound image to have multiple samples");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2DMS s;
        layout(location=0) out vec4 color;
        void main() {
           color = texelFetch(s, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkTextureObj texture(m_device, nullptr);  // THIS LINE CAUSES CRASH ON MALI
    VkSamplerObj sampler(m_device);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkResult err = pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_commandBuffer->BindDescriptorSet(descriptorSet);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    // error produced here.
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DrawTimeImageComponentTypeMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the component type of an imageview disagrees with the type in the shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "SINT component type, but bound descriptor");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform isampler2D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texelFetch(s, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkTextureObj texture(m_device, nullptr);  // UNORM texture by default, incompatible with isampler2D
    VkSamplerObj sampler(m_device);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkResult err = pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_commandBuffer->BindDescriptorSet(descriptorSet);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    // error produced here.
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageLayerCountMismatch) {
    TEST_DESCRIPTION(
        "Try to copy between images with the source subresource having a different layerCount than the destination subresource");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool maintenance1 = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        maintenance1 = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkFormat image_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_format, &format_props);
    if ((format_props.optimalTilingFeatures & (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) == 0) {
        printf("%s Transfer for format is not supported.\n", kSkipPrefix);
        return;
    }

    // Create two images to copy between
    VkImageObj src_image_obj(m_device);
    VkImageObj dst_image_obj(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = image_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;

    src_image_obj.init(&image_create_info);
    ASSERT_TRUE(src_image_obj.initialized());

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    dst_image_obj.init(&image_create_info);
    ASSERT_TRUE(dst_image_obj.initialized());

    m_commandBuffer->begin();
    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset.x = 0;
    copyRegion.srcOffset.y = 0;
    copyRegion.srcOffset.z = 0;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    // Introduce failure by forcing the dst layerCount to differ from src
    copyRegion.dstSubresource.layerCount = 3;
    copyRegion.dstOffset.x = 0;
    copyRegion.dstOffset.y = 0;
    copyRegion.dstOffset.z = 0;
    copyRegion.extent.width = 1;
    copyRegion.extent.height = 1;
    copyRegion.extent.depth = 1;

    const char *vuid = (maintenance1 == true) ? "VUID-VkImageCopy-extent-00140" : "VUID-VkImageCopy-layerCount-00138";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_commandBuffer->CopyImage(src_image_obj.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image_obj.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copyRegion);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CompressedImageMipCopyTests) {
    TEST_DESCRIPTION("Image/Buffer copies for higher mip levels");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool copy_commands2 = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
        copy_commands2 = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdCopyBufferToImage2KHR vkCmdCopyBufferToImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdCopyBufferToImage2Function =
            (PFN_vkCmdCopyBufferToImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyBufferToImage2KHR");
    }

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    VkFormat compressed_format = VK_FORMAT_UNDEFINED;
    if (device_features.textureCompressionBC) {
        compressed_format = VK_FORMAT_BC3_SRGB_BLOCK;
    } else if (device_features.textureCompressionETC2) {
        compressed_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    } else if (device_features.textureCompressionASTC_LDR) {
        compressed_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    } else {
        printf("%s No compressed formats supported - CompressedImageMipCopyTests skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = compressed_format;
    ci.extent = {32, 32, 1};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image(m_device);
    image.init(&ci);
    ASSERT_TRUE(image.initialized());

    VkImageObj odd_image(m_device);
    ci.extent = {31, 32, 1};  // Mips are [31,32] [15,16] [7,8] [3,4], [1,2] [1,1]
    odd_image.init(&ci);
    ASSERT_TRUE(odd_image.initialized());

    // Allocate buffers
    VkMemoryPropertyFlags reqs = 0;
    VkBufferObj buffer_1024, buffer_64, buffer_16, buffer_8;
    buffer_1024.init_as_src_and_dst(*m_device, 1024, reqs);
    buffer_64.init_as_src_and_dst(*m_device, 64, reqs);
    buffer_16.init_as_src_and_dst(*m_device, 16, reqs);
    buffer_8.init_as_src_and_dst(*m_device, 8, reqs);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    // start recording
    m_commandBuffer->begin();

    VkMemoryBarrier mem_barriers[3];
    mem_barriers[0] = LvlInitStruct<VkMemoryBarrier>();
    mem_barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[1] = LvlInitStruct<VkMemoryBarrier>();
    mem_barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2] = LvlInitStruct<VkMemoryBarrier>();
    mem_barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // Mip level copies that work - 5 levels
    m_errorMonitor->ExpectSuccess();

    // Mip 0 should fit in 1k buffer - 1k texels @ 1b each
    region.imageExtent = {32, 32, 1};
    region.imageSubresource.mipLevel = 0;
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_1024.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[2], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_1024.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 2 should fit in 64b buffer - 64 texels @ 1b each
    region.imageExtent = {8, 8, 1};
    region.imageSubresource.mipLevel = 2;
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_64.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 3 should fit in 16b buffer - 16 texels @ 1b each
    region.imageExtent = {4, 4, 1};
    region.imageSubresource.mipLevel = 3;
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 4&5 should fit in 16b buffer with no complaint - 4 & 1 texels @ 1b each
    region.imageExtent = {2, 2, 1};
    region.imageSubresource.mipLevel = 4;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    region.imageExtent = {1, 1, 1};
    region.imageSubresource.mipLevel = 5;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyNotFound();

    // Buffer must accommodate a full compressed block, regardless of texel count
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-pRegions-00183");
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_8.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-pRegions-00171");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_8.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Copy width < compressed block size, but not the full mip width
    region.imageExtent = {1, 2, 1};
    region.imageSubresource.mipLevel = 4;
    // width not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageExtent-00207");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-imageExtent-00207");  // width not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-01793");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Copy height < compressed block size but not the full mip height
    region.imageExtent = {2, 1, 1};
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageExtent-00208");  // height not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-imageExtent-00208");  // height not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-01793");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Offsets must be multiple of compressed block size
    region.imageOffset = {1, 1, 0};
    region.imageExtent = {1, 1, 1};
    // imageOffset not a multiple of block size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageOffset-00205");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-00205");  // imageOffset not a multiple of block size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-01793");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyBufferToImage2Function) {
        const VkBufferImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR,
                                               NULL,
                                               region.bufferOffset,
                                               region.bufferRowLength,
                                               region.bufferImageHeight,
                                               region.imageSubresource,
                                               region.imageOffset,
                                               region.imageExtent};
        const VkCopyBufferToImageInfo2KHR copy_buffer_to_image_info2 = {VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR,
                                                                        NULL,
                                                                        buffer_16.handle(),
                                                                        image.handle(),
                                                                        VK_IMAGE_LAYOUT_GENERAL,
                                                                        1,
                                                                        &region2};
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkCopyBufferToImageInfo2-imageOffset-00205");  // imageOffset not a multiple of block size
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkCopyBufferToImageInfo2-imageOffset-01793");  // image transfer granularity
        vkCmdCopyBufferToImage2Function(m_commandBuffer->handle(), &copy_buffer_to_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Offset + extent width = mip width - should succeed
    region.imageOffset = {4, 4, 0};
    region.imageExtent = {3, 4, 1};
    region.imageSubresource.mipLevel = 2;
    m_errorMonitor->ExpectSuccess();

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1,
                             &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyNotFound();

    // Offset + extent width < mip width and not a multiple of block width - should fail
    region.imageExtent = {3, 3, 1};
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageExtent-00208");  // offset+extent not a multiple of block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-imageExtent-00208");  // offset+extent not a multiple of block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-01793");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageBufferCopyTests) {
    TEST_DESCRIPTION("Image to buffer and buffer to image tests");

    // Enable KHR multiplane req'd extensions for multi-planar copy tests
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, nullptr));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Bail if any dimension of transfer granularity is 0.
    auto index = m_device->graphics_queue_node_index_;
    auto queue_family_properties = m_device->phy().queue_properties();
    if ((queue_family_properties[index].minImageTransferGranularity.depth == 0) ||
        (queue_family_properties[index].minImageTransferGranularity.width == 0) ||
        (queue_family_properties[index].minImageTransferGranularity.height == 0)) {
        printf("%s Subresource copies are disallowed when xfer granularity (x|y|z) is 0. Skipped.\n", kSkipPrefix);
        return;
    }

    // All VkImageObj must be defined here as if defined inside below scopes will cause image memory to be deleted when out of scope
    // and invalidates the entire command buffer. This prevents from having to reset the commmand buffer every scope rgba
    VkImageObj image_64k(m_device);  // 128^2 texels, 64k
    VkImageObj image_16k(m_device);  // 64^2 texels, 16k
    // depth stencil
    VkImageObj image_16k_depth(m_device);  // 64^2 texels, depth, 16k
    VkImageObj ds_image_4D_1S(m_device);   // 256^2 texels, 512kb (256k depth, 64k stencil, 192k pack)
    VkImageObj ds_image_3D_1S(m_device);   // 256^2 texels, 256kb (192k depth, 64k stencil)
    VkImageObj ds_image_2D(m_device);      // 256^2 texels, 128k (128k depth)
    VkImageObj ds_image_1S(m_device);      // 256^2 texels, 64k (64k stencil)
    // compression
    VkImageObj image_16k_4x4comp(m_device);   // 128^2 texels as 32^2 compressed (4x4) blocks, 16k
    VkImageObj image_NPOT_4x4comp(m_device);  // 130^2 texels as 33^2 compressed (4x4) blocks
    // multi-planar
    VkImageObj image_multi_planar(m_device);  // 128^2 texels in plane_0 and 64^2 texels in plane_1

    // Verify R8G8B8A8_UINT format is supported for transfer
    bool missing_rgba_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UINT, &props);
    missing_rgba_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_rgba_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_rgba_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (!missing_rgba_support) {
        image_64k.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UINT,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                       VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image_64k.initialized());

        image_16k.Init(64, 64, 1, VK_FORMAT_R8G8B8A8_UINT,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                       VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image_16k.initialized());
    }

    // Verify all needed Depth/Stencil formats are supported
    bool missing_ds_support = false;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D32_SFLOAT_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D24_UNORM_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D16_UNORM, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (!missing_ds_support) {
        image_16k_depth.Init(64, 64, 1, VK_FORMAT_D24_UNORM_S8_UINT,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image_16k_depth.initialized());

        ds_image_4D_1S.Init(
            256, 256, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ds_image_4D_1S.initialized());

        ds_image_3D_1S.Init(
            256, 256, 1, VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ds_image_3D_1S.initialized());

        ds_image_2D.Init(
            256, 256, 1, VK_FORMAT_D16_UNORM,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ds_image_2D.initialized());

        ds_image_1S.Init(
            256, 256, 1, VK_FORMAT_S8_UINT,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ds_image_1S.initialized());
    }

    // Allocate buffers
    VkBufferObj buffer_256k, buffer_128k, buffer_64k, buffer_16k;
    VkMemoryPropertyFlags reqs = 0;
    buffer_256k.init_as_src_and_dst(*m_device, 262144, reqs);  // 256k
    buffer_128k.init_as_src_and_dst(*m_device, 131072, reqs);  // 128k
    buffer_64k.init_as_src_and_dst(*m_device, 65536, reqs);    // 64k
    buffer_16k.init_as_src_and_dst(*m_device, 16384, reqs);    // 16k

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 0;

    VkMemoryBarrier mem_barriers[3];
    mem_barriers[0] = LvlInitStruct<VkMemoryBarrier>();
    mem_barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[1] = LvlInitStruct<VkMemoryBarrier>();
    mem_barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2] = LvlInitStruct<VkMemoryBarrier>();
    mem_barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    if (missing_rgba_support) {
        printf("%s R8G8B8A8_UINT transfer unsupported - skipping RGBA tests.\n", kSkipPrefix);

        // start recording for future tests
        m_commandBuffer->begin();
    } else {
        // attempt copies before putting command buffer in recording state
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-recording");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_64k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-recording");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        // start recording
        m_commandBuffer->begin();

        // successful copies
        m_errorMonitor->ExpectSuccess();
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[2], 0, nullptr, 0, nullptr);
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        region.imageOffset.x = 16;  // 16k copy, offset requires larger image

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        region.imageExtent.height = 78;  // > 16k copy requires larger buffer & image

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[1], 0, nullptr, 0, nullptr);
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_64k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        region.imageOffset.x = 0;
        region.imageExtent.height = 64;
        region.bufferOffset = 256;  // 16k copy with buffer offset, requires larger buffer

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                               &mem_barriers[1], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyNotFound();

        // image/buffer too small (extent too large) on copy to image
        region.imageExtent = {65, 64, 1};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-pRegions-00171");  // buffer too small
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-06218");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-pRegions-06217");  // image too small
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_64k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        // image/buffer too small (offset) on copy to image
        region.imageExtent = {64, 64, 1};
        region.imageOffset = {0, 4, 0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-pRegions-00171");  // buffer too small
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-06218");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-06219");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-pRegions-06217");  // image too small
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_64k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        // image/buffer too small on copy to buffer
        region.imageExtent = {64, 64, 1};
        region.imageOffset = {0, 0, 0};
        region.bufferOffset = 4;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // buffer too small
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        region.imageExtent = {64, 65, 1};
        region.bufferOffset = 0;
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-pRegions-06222");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyImageToBuffer-pRegions-06220");  // image too small
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        // buffer size OK but rowlength causes loose packing
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-pRegions-00183");
        region.imageExtent = {64, 64, 1};
        region.bufferRowLength = 68;
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06659");
        region.imageExtent.width = 0;
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        // aspect bits
        region.imageExtent = {64, 64, 1};
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        if (!missing_ds_support) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-VkBufferImageCopy-aspectMask-00212");  // more than 1 aspect bit set
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_depth.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdCopyImageToBuffer-aspectMask-00211");  // different mis-matched aspect
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_depth.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyImageToBuffer-aspectMask-00211");  // mis-matched aspect
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // Out-of-range mip levels should fail
        region.imageSubresource.mipLevel = image_16k.create_info().mipLevels + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageSubresource-01703");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-pRegions-06221");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-pRegions-06222");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageOffset-00200");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-06220");  // unavoidable "region exceeds image bounds" for non-existent mip
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageSubresource-01701");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-06218");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-06219");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageOffset-00200");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyBufferToImage-pRegions-06217");  // unavoidable "region exceeds image bounds" for non-existent mip
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        region.imageSubresource.mipLevel = 0;

        // Out-of-range array layers should fail
        region.imageSubresource.baseArrayLayer = image_16k.create_info().arrayLayers;
        region.imageSubresource.layerCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageSubresource-01704");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageSubresource-01702");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        region.imageSubresource.baseArrayLayer = 0;

        // Layout mismatch should fail
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_16k.handle(), 1, &region);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_16k.handle(),
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        m_errorMonitor->VerifyFound();
    }

    // Test Depth/Stencil copies
    if (missing_ds_support) {
        printf("%s Depth / Stencil formats unsupported - skipping D/S tests.\n", kSkipPrefix);
    } else {
        VkBufferImageCopy ds_region = {};
        ds_region.bufferOffset = 0;
        ds_region.bufferRowLength = 0;
        ds_region.bufferImageHeight = 0;
        ds_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        ds_region.imageSubresource.mipLevel = 0;
        ds_region.imageSubresource.baseArrayLayer = 0;
        ds_region.imageSubresource.layerCount = 1;
        ds_region.imageOffset = {0, 0, 0};
        ds_region.imageExtent = {256, 256, 1};

        // Depth copies that should succeed
        m_errorMonitor->ExpectSuccess();  // Extract 4b depth per texel, pack into 256k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_256k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->ExpectSuccess();  // Extract 3b depth per texel, pack (loose) into 256k buffer
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_256k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->ExpectSuccess();  // Copy 2b depth per texel, into 128k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_128k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyNotFound();

        // Depth copies that should fail
        ds_region.bufferOffset = 4;
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 4b depth per texel, pack into 256k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_256k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 3b depth per texel, pack (loose) into 128k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_128k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Copy 2b depth per texel, into 128k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_128k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();

        ds_region.bufferOffset = 5;
        ds_region.imageExtent = {64, 64, 1};  // need smaller so offset works
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-04053");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_128k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();
        ds_region.imageExtent = {256, 256, 1};

        // Stencil copies that should succeed
        ds_region.bufferOffset = 0;
        ds_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        m_errorMonitor->ExpectSuccess();  // Extract 1b stencil per texel, pack into 64k buffer
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->ExpectSuccess();  // Extract 1b stencil per texel, pack into 64k buffer
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->ExpectSuccess();  // Copy 1b depth per texel, into 64k buffer
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyNotFound();

        // Stencil copies that should fail
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 1b stencil per texel, pack into 64k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_16k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 1b stencil per texel, pack into 64k buffer
        ds_region.bufferRowLength = 260;
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();

        ds_region.bufferRowLength = 0;
        ds_region.bufferOffset = 4;
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Copy 1b depth per texel, into 64k buffer
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();
    }

    // Test compressed formats, if supported
    // Support here requires both feature bit for compression and picked format supports transfer feature bits
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (!(device_features.textureCompressionBC || device_features.textureCompressionETC2 ||
          device_features.textureCompressionASTC_LDR)) {
        printf("%s No compressed formats supported - block compression tests skipped.\n", kSkipPrefix);
    } else {
        // Verify transfer support for each compression format used blow
        bool missing_bc_support = false;
        bool missing_etc_support = false;
        bool missing_astc_support = false;
        bool missing_compression_support = false;

        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_BC3_SRGB_BLOCK, &props);
        missing_bc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
        missing_bc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
        missing_bc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, &props);
        missing_etc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
        missing_etc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
        missing_etc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_ASTC_4x4_UNORM_BLOCK, &props);
        missing_astc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
        missing_astc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
        missing_astc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

        if (device_features.textureCompressionBC && (!missing_bc_support)) {
            image_16k_4x4comp.Init(128, 128, 1, VK_FORMAT_BC3_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL,
                                   0);
            image_NPOT_4x4comp.Init(130, 130, 1, VK_FORMAT_BC3_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL,
                                    0);
        } else if (device_features.textureCompressionETC2 && (!missing_etc_support)) {
            image_16k_4x4comp.Init(128, 128, 1, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                   VK_IMAGE_TILING_OPTIMAL, 0);
            image_NPOT_4x4comp.Init(130, 130, 1, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                    VK_IMAGE_TILING_OPTIMAL, 0);
        } else if (device_features.textureCompressionASTC_LDR && (!missing_astc_support)) {
            image_16k_4x4comp.Init(128, 128, 1, VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                   VK_IMAGE_TILING_OPTIMAL, 0);
            image_NPOT_4x4comp.Init(130, 130, 1, VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                    VK_IMAGE_TILING_OPTIMAL, 0);
        } else {
            missing_compression_support = true;
        }

        if (missing_compression_support) {
            printf("%s No compressed formats transfers bits are supported - block compression tests skipped.\n", kSkipPrefix);
        } else {
            ASSERT_TRUE(image_16k_4x4comp.initialized());
            std::string vuid;
            // Just fits
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                                   &mem_barriers[0], 0, nullptr, 0, nullptr);
            m_errorMonitor->ExpectSuccess();
            region.imageExtent = {128, 128, 1};
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyNotFound();

            // with offset, too big for buffer
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-pRegions-00183");
            region.bufferOffset = 16;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
            region.bufferOffset = 0;

            // extents that are not a multiple of compressed block size
            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageExtent-00207");  // extent width not a multiple of block size
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
            region.imageExtent.width = 66;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
            region.imageExtent.width = 128;

            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageExtent-00208");  // extent height not a multiple of block size
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
            region.imageExtent.height = 2;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
            region.imageExtent.height = 128;

            // TODO: All available compressed formats are 2D, with block depth of 1. Unable to provoke VU_01277.

            // non-multiple extents are allowed if at the far edge of a non-block-multiple image - these should pass
            m_errorMonitor->ExpectSuccess();
            region.imageExtent.width = 66;
            region.imageOffset.x = 64;
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                                   &mem_barriers[0], 0, nullptr, 0, nullptr);
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            region.imageExtent.width = 16;
            region.imageOffset.x = 0;
            region.imageExtent.height = 2;
            region.imageOffset.y = 128;
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                                   &mem_barriers[0], 0, nullptr, 0, nullptr);
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyNotFound();
            region.imageOffset = {0, 0, 0};

            // buffer offset must be a multiple of texel block size (16)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-bufferOffset-00206");
            vuid =
                mp_extensions ? "VUID-vkCmdCopyImageToBuffer-bufferOffset-01558" : "VUID-vkCmdCopyImageToBuffer-bufferOffset-00193";
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
            region.imageExtent = {64, 64, 1};
            region.bufferOffset = 24;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();

            // rowlength not a multiple of block width (4)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-bufferRowLength-00203");
            region.bufferOffset = 0;
            region.bufferRowLength = 130;
            region.bufferImageHeight = 0;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_64k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();

            // imageheight not a multiple of block height (4)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-00204");
            region.bufferRowLength = 0;
            region.bufferImageHeight = 130;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_64k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
        }
    }

    // Test multi-planar formats, if supported
    if (!mp_extensions) {
        printf("%s multi-planar extensions not supported; skipped.\n", kSkipPrefix);
    } else {
        // Try to use G8_B8R8_2PLANE_420_UNORM because need 2-plane format for some tests and likely supported due to copy support
        // being required with samplerYcbcrConversion feature
        bool missing_mp_support = false;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &props);
        missing_mp_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
        missing_mp_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
        missing_mp_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

        if (missing_mp_support) {
            printf("%s VK_FORMAT_G8_B8R8_2PLANE_420_UNORM transfer not supported; skipped.\n", kSkipPrefix);
        } else {
            VkBufferImageCopy mp_region = {};
            mp_region.bufferOffset = 0;
            mp_region.bufferRowLength = 0;
            mp_region.bufferImageHeight = 0;
            mp_region.imageSubresource.mipLevel = 0;
            mp_region.imageSubresource.baseArrayLayer = 0;
            mp_region.imageSubresource.layerCount = 1;
            mp_region.imageOffset = {0, 0, 0};
            mp_region.imageExtent = {128, 128, 1};

            // YUV420 means 1/2 width and height so plane_0 is 128x128 and plane_1 is 64x64 here
            image_multi_planar.Init(128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                    VK_IMAGE_TILING_OPTIMAL, 0);
            ASSERT_TRUE(image_multi_planar.initialized());

            // Copies into a mutli-planar image aspect properly
            m_errorMonitor->ExpectSuccess();
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                                   &mem_barriers[2], 0, nullptr, 0, nullptr);
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyNotFound();

            // uses plane_2 without being 3 planar format
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-aspectMask-01560");
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyFound();

            // uses single-plane aspect mask
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-aspectMask-01560");
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyFound();

            // buffer offset must be a multiple of texel block size for VK_FORMAT_R8G8_UNORM (2)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-bufferOffset-01559");
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
            mp_region.bufferOffset = 5;
            mp_region.imageExtent = {8, 8, 1};
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, MiscImageLayerTests) {
    TEST_DESCRIPTION("Image-related tests that don't belong elsewhere");

    ASSERT_NO_FATAL_FAILURE(Init());

    // TODO: Ideally we should check if a format is supported, before using it.
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);  // 64bpp
    ASSERT_TRUE(image.initialized());
    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src(*m_device, 128 * 128 * 8, reqs);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 128;
    region.bufferImageHeight = 128;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // layerCount can't be 0 - Expect MISMATCHED_IMAGE_ASPECT
    region.imageSubresource.layerCount = 1;
    region.imageExtent.height = 4;
    region.imageExtent.width = 4;
    region.imageExtent.depth = 1;

    VkImageObj image2(m_device);
    image2.Init(128, 128, 1, VK_FORMAT_R8G8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);  // 16bpp
    ASSERT_TRUE(image2.initialized());
    VkBufferObj buffer2;
    VkMemoryPropertyFlags reqs2 = 0;
    buffer2.init_as_src(*m_device, 128 * 128 * 2, reqs2);
    m_commandBuffer->begin();

    // Image must have offset.z of 0 and extent.depth of 1
    // Introduce failure by setting imageExtent.depth to 0
    region.imageExtent.depth = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-srcImage-00201");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageExtent.depth = 1;

    // Image must have offset.z of 0 and extent.depth of 1
    // Introduce failure by setting imageOffset.z to 4
    // Note: Also (unavoidably) triggers 'region exceeds image' #1228
    region.imageOffset.z = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-srcImage-00201");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-00200");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-pRegions-06217");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageOffset.z = 0;
    // BufferOffset must be a multiple of the calling command's VkImage parameter's texel size
    // Introduce failure by setting bufferOffset to 1 and 1/2 texels
    region.bufferOffset = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-bufferOffset-00193");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
    region.bufferOffset = 0;
    region.imageExtent.height = 128;
    region.imageExtent.width = 128;
    // Introduce failure by setting bufferRowLength > 0 but less than width
    region.bufferRowLength = 64;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-bufferRowLength-00195");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
    region.bufferRowLength = 128;
    // Introduce failure by setting bufferRowHeight > 0 but less than height
    region.bufferImageHeight = 64;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-bufferImageHeight-00196");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.bufferImageHeight = 128;
    VkImageObj intImage1(m_device);
    intImage1.Init(128, 128, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    intImage1.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkImageObj intImage2(m_device);
    intImage2.Init(128, 128, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    intImage2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {128, 0, 0};
    blitRegion.srcOffsets[1] = {128, 128, 1};
    blitRegion.dstOffsets[0] = {0, 128, 0};
    blitRegion.dstOffsets[1] = {128, 128, 1};

    // Look for NULL-blit warning
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "vkCmdBlitImage: pRegions[0].srcOffsets specify a zero-volume area.");
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "vkCmdBlitImage: pRegions[0].dstOffsets specify a zero-volume area.");
    vk::CmdBlitImage(m_commandBuffer->handle(), intImage1.handle(), intImage1.Layout(), intImage2.handle(), intImage2.Layout(), 1,
                     &blitRegion, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CopyImageTypeExtentMismatch) {
    // Image copy tests where format type and extents don't match
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool copy_commands2 = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
        copy_commands2 = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdCopyImage2KHR vkCmdCopyImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdCopyImage2Function = (PFN_vkCmdCopyImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyImage2KHR");
    }

    // Tests are designed to run without Maintenance1 which was promoted in 1.1
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 1, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Create 1D image
    VkImageObj image_1D(m_device);
    image_1D.init(&ci);
    ASSERT_TRUE(image_1D.initialized());

    // 2D image
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    VkImageObj image_2D(m_device);
    image_2D.init(&ci);
    ASSERT_TRUE(image_2D.initialized());

    // 3D image
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {32, 32, 8};
    VkImageObj image_3D(m_device);
    image_3D.init(&ci);
    ASSERT_TRUE(image_3D.initialized());

    // 2D image array
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    ci.arrayLayers = 8;
    VkImageObj image_2D_array(m_device);
    image_2D_array.init(&ci);
    ASSERT_TRUE(image_2D_array.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Sanity check
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->CopyImage(image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // Equivalent sanity check using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                         NULL,
                                         copy_region.srcSubresource,
                                         copy_region.srcOffset,
                                         copy_region.dstSubresource,
                                         copy_region.dstOffset,
                                         copy_region.extent};
        const VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      image_1D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      image_2D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      1,
                                                      &region2};
        m_errorMonitor->ExpectSuccess();
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyNotFound();
    }

    // 1D texture w/ offset.y > 0. Source = VU 09c00124, dest = 09c00130
    copy_region.srcOffset.y = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");  // also y-dim overrun
    m_commandBuffer->CopyImage(image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                         NULL,
                                         copy_region.srcSubresource,
                                         copy_region.srcOffset,
                                         copy_region.dstSubresource,
                                         copy_region.dstOffset,
                                         copy_region.extent};
        const VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      image_1D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      image_2D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      1,
                                                      &region2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImage-00146");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcOffset-00145");  // also y-dim overrun
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    copy_region.srcOffset.y = 0;
    copy_region.dstOffset.y = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-00152");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");  // also y-dim overrun
    m_commandBuffer->CopyImage(image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                         NULL,
                                         copy_region.srcSubresource,
                                         copy_region.srcOffset,
                                         copy_region.dstSubresource,
                                         copy_region.dstOffset,
                                         copy_region.extent};
        const VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      image_2D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      image_1D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      1,
                                                      &region2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImage-00152");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstOffset-00151");  // also y-dim overrun
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    copy_region.dstOffset.y = 0;

    // 1D texture w/ extent.height > 1. Source = VU 09c00124, dest = 09c00130
    copy_region.extent.height = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");  // also y-dim overrun
    m_commandBuffer->CopyImage(image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                         NULL,
                                         copy_region.srcSubresource,
                                         copy_region.srcOffset,
                                         copy_region.dstSubresource,
                                         copy_region.dstOffset,
                                         copy_region.extent};
        const VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      image_1D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      image_2D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      1,
                                                      &region2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImage-00146");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcOffset-00145");  // also y-dim overrun
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-00152");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");  // also y-dim overrun
    m_commandBuffer->CopyImage(image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                         NULL,
                                         copy_region.srcSubresource,
                                         copy_region.srcOffset,
                                         copy_region.dstSubresource,
                                         copy_region.dstOffset,
                                         copy_region.extent};
        const VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      image_2D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      image_1D.image(),
                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                      1,
                                                      &region2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImage-00152");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstOffset-00151");  // also y-dim overrun
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    copy_region.extent.height = 1;

    // 1D texture w/ offset.z > 0. Source = VU 09c00df2, dest = 09c00df4
    copy_region.srcOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01785");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun
    m_commandBuffer->CopyImage(image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset.z = 0;
    copy_region.dstOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01786");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun
    m_commandBuffer->CopyImage(image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset.z = 0;

    // 1D texture w/ extent.depth > 1. Source = VU 09c00df2, dest = 09c00df4
    copy_region.extent.depth = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01785");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun (src)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcImage-01789");  // 2D needs to be 1 pre-Vulkan 1.1
    m_commandBuffer->CopyImage(image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01786");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun (src)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcImage-01789");  // 2D needs to be 1 pre-Vulkan 1.1
    m_commandBuffer->CopyImage(image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.depth = 1;

    // 2D texture w/ offset.z > 0. Source = VU 09c00df6, dest = 09c00df8
    copy_region.extent = {16, 16, 1};
    copy_region.srcOffset.z = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01787");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun (src)
    m_commandBuffer->CopyImage(image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset.z = 0;
    copy_region.dstOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01788");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun (dst)
    m_commandBuffer->CopyImage(image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset.z = 0;

    // 3D texture accessing an array layer other than 0. VU 09c0011a
    copy_region.extent = {4, 4, 1};
    copy_region.srcSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00139");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcSubresource-01698");  // also 'too many layers'
    m_commandBuffer->CopyImage(image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcSubresource.baseArrayLayer = 0;

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageTypeExtentMismatchMaintenance1) {
    // Image copy tests where format type and extents don't match and the Maintenance1 extension is enabled
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    } else {
        printf("%s Maintenance1 extension cannot be enabled, test skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties format_props;
    // TODO: Remove this check if or when devsim handles extensions.
    // The chosen format has mandatory support the transfer src and dst format features when Maitenance1 is enabled. However, our
    // use of devsim and the mock ICD violate this guarantee.
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_format, &format_props);
    if (!(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        printf("%s Maintenance1 extension is not supported.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.format = image_format;
    ci.extent = {32, 1, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Create 1D image
    VkImageObj image_1D(m_device);
    image_1D.init(&ci);
    ASSERT_TRUE(image_1D.initialized());

    // 2D image
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    VkImageObj image_2D(m_device);
    image_2D.init(&ci);
    ASSERT_TRUE(image_2D.initialized());

    // 3D image
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {32, 32, 8};
    VkImageObj image_3D(m_device);
    image_3D.init(&ci);
    ASSERT_TRUE(image_3D.initialized());

    // 2D image array
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    ci.arrayLayers = 8;
    VkImageObj image_2D_array(m_device);
    image_2D_array.init(&ci);
    ASSERT_TRUE(image_2D_array.initialized());

    // second 2D image array
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    ci.arrayLayers = 8;
    VkImageObj image_2D_array_2(m_device);
    image_2D_array_2.init(&ci);
    ASSERT_TRUE(image_2D_array_2.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Copy from layer not present
    copy_region.srcSubresource.baseArrayLayer = 4;
    copy_region.srcSubresource.layerCount = 6;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcSubresource-01698");
    m_commandBuffer->CopyImage(image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;

    // Copy to layer not present
    copy_region.dstSubresource.baseArrayLayer = 1;
    copy_region.dstSubresource.layerCount = 8;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstSubresource-01699");
    m_commandBuffer->CopyImage(image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.layerCount = 1;

    // both 2D and extent.depth not 1
    // Need two 2D array images to prevent other errors
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01790");
    m_commandBuffer->CopyImage(image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                               1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    // 2D src / 3D dst and depth not equal to src layerCount
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01791");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-00140");
    m_commandBuffer->CopyImage(image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    // 3D src / 2D dst and depth not equal to dst layerCount
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01792");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-00140");
    m_commandBuffer->CopyImage(image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageCompressedBlockAlignment) {
    // Image copy tests on compressed images with block alignment errors
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());

    // Select a compressed format and verify support
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    VkFormat compressed_format = VK_FORMAT_UNDEFINED;
    if (device_features.textureCompressionBC) {
        compressed_format = VK_FORMAT_BC3_SRGB_BLOCK;
    } else if (device_features.textureCompressionETC2) {
        compressed_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    } else if (device_features.textureCompressionASTC_LDR) {
        compressed_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    }

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = compressed_format;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), ci.format, ci.imageType, ci.tiling,
                                                                 ci.usage, ci.flags, &img_prop)) {
        printf("%s No compressed formats supported - CopyImageCompressedBlockAlignment skipped.\n", kSkipPrefix);
        return;
    }

    // Create images
    VkImageObj image_1(m_device);
    image_1.init(&ci);
    ASSERT_TRUE(image_1.initialized());

    ci.extent = {62, 62, 1};  // slightly smaller and not divisible by block size
    VkImageObj image_2(m_device);
    image_2.init(&ci);
    ASSERT_TRUE(image_2.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {48, 48, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Sanity check
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyNotFound();

    std::string vuid;
    bool ycbcr = (DeviceExtensionEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ||
                  (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    // Src, Dest offsets must be multiples of compressed block sizes {4, 4, 1}
    // Image transfer granularity gets set to compressed block size, so an ITG error is also (unavoidably) triggered.
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01727" : "VUID-vkCmdCopyImage-srcImage-01727";
    copy_region.srcOffset = {2, 4, 0};  // source width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {12, 1, 0};  // source height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {0, 0, 0};

    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01731" : "VUID-vkCmdCopyImage-dstImage-01731";
    copy_region.dstOffset = {1, 0, 0};  // dest width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dstOffset image transfer granularity
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {4, 1, 0};  // dest height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dstOffset image transfer granularity
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {0, 0, 0};

    // Copy extent must be multiples of compressed block sizes {4, 4, 1} if not full width/height
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01728" : "VUID-vkCmdCopyImage-srcImage-01728";
    copy_region.extent = {62, 60, 1};  // source width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01729" : "VUID-vkCmdCopyImage-srcImage-01729";
    copy_region.extent = {60, 62, 1};  // source height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    m_commandBuffer->CopyImage(image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01732" : "VUID-vkCmdCopyImage-dstImage-01732";
    copy_region.extent = {62, 60, 1};  // dest width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    m_commandBuffer->CopyImage(image_2.image(), VK_IMAGE_LAYOUT_GENERAL, image_1.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
    copy_region.extent = {60, 62, 1};  // dest height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    m_commandBuffer->CopyImage(image_2.image(), VK_IMAGE_LAYOUT_GENERAL, image_1.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Note: "VUID-vkCmdCopyImage-srcImage-01730", "VUID-vkCmdCopyImage-dstImage-01734", "VUID-vkCmdCopyImage-srcImage-01730",
    // "VUID-vkCmdCopyImage-dstImage-01734"
    //       There are currently no supported compressed formats with a block depth other than 1,
    //       so impossible to create a 'not a multiple' condition for depth.
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageSinglePlane422Alignment) {
    // Image copy tests on single-plane _422 formats with block alignment errors

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Select a _422 format and verify support
    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8B8G8R8_422_UNORM_KHR;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify formats
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    if (!supported) {
        printf("%s Single-plane _422 image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    // Create images
    ci.extent = {64, 64, 1};
    VkImageObj image_422(m_device);
    image_422.init(&ci);
    ASSERT_TRUE(image_422.initialized());

    ci.extent = {64, 64, 1};
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_ucmp(m_device);
    image_ucmp.init(&ci);
    ASSERT_TRUE(image_ucmp.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {48, 48, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Src offsets must be multiples of compressed block sizes
    copy_region.srcOffset = {3, 4, 0};  // source offset x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01727");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-01783");
    m_commandBuffer->CopyImage(image_422.image(), VK_IMAGE_LAYOUT_GENERAL, image_ucmp.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {0, 0, 0};

    // Dst offsets must be multiples of compressed block sizes
    copy_region.dstOffset = {1, 0, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01731");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-01784");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    m_commandBuffer->CopyImage(image_ucmp.image(), VK_IMAGE_LAYOUT_GENERAL, image_422.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {0, 0, 0};

    // Copy extent must be multiples of compressed block sizes if not full width/height
    copy_region.extent = {31, 60, 1};  // 422 source, extent.x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01728");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-01783");
    m_commandBuffer->CopyImage(image_422.image(), VK_IMAGE_LAYOUT_GENERAL, image_ucmp.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // 422 dest
    m_commandBuffer->CopyImage(image_ucmp.image(), VK_IMAGE_LAYOUT_GENERAL, image_422.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();
    copy_region.dstOffset = {0, 0, 0};

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageMultiplaneAspectBits) {
    // Image copy tests on multiplane images with aspect errors

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Select multi-plane formats and verify support
    VkFormat mp3_format = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR;
    VkFormat mp2_format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR;

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = mp2_format;
    ci.extent = {256, 256, 1};
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify formats
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    ci.format = VK_FORMAT_D24_UNORM_S8_UINT;
    supported = supported && ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    ci.format = mp3_format;
    supported = supported && ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    if (!supported) {
        printf("%s Multiplane image formats or optimally tiled depth-stencil buffers not supported.  Skipping test.\n",
               kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    // Create images
    VkImageObj mp3_image(m_device);
    mp3_image.init(&ci);
    ASSERT_TRUE(mp3_image.initialized());

    ci.format = mp2_format;
    VkImageObj mp2_image(m_device);
    mp2_image.init(&ci);
    ASSERT_TRUE(mp2_image.initialized());

    ci.format = VK_FORMAT_D24_UNORM_S8_UINT;
    VkImageObj sp_image(m_device);
    sp_image.init(&ci);
    ASSERT_TRUE(sp_image.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {128, 128, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01552");
    m_commandBuffer->CopyImage(mp2_image.image(), VK_IMAGE_LAYOUT_GENERAL, mp3_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01553");
    m_commandBuffer->CopyImage(mp3_image.image(), VK_IMAGE_LAYOUT_GENERAL, mp2_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01554");
    m_commandBuffer->CopyImage(mp3_image.image(), VK_IMAGE_LAYOUT_GENERAL, mp2_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01555");
    m_commandBuffer->CopyImage(mp2_image.image(), VK_IMAGE_LAYOUT_GENERAL, mp3_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01556");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-None-01549");  // since also non-compatiable
    m_commandBuffer->CopyImage(mp2_image.image(), VK_IMAGE_LAYOUT_GENERAL, sp_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01557");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-None-01549");  // since also non-compatiable
    m_commandBuffer->CopyImage(sp_image.image(), VK_IMAGE_LAYOUT_GENERAL, mp3_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageSrcSizeExceeded) {
    // Image copy with source region specified greater than src image size
    ASSERT_NO_FATAL_FAILURE(Init());

    // Create images with full mip chain
    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj src_image(m_device);
    src_image.init(&ci);
    ASSERT_TRUE(src_image.initialized());

    // Dest image with one more mip level
    ci.extent = {64, 64, 16};
    ci.mipLevels = 7;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj dst_image(m_device);
    dst_image.init(&ci);
    ASSERT_TRUE(dst_image.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 32, 8};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // Source exceeded in x-dim
    copy_region.srcOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00144");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in x-dim in negative direction (since offset is a signed in)
    copy_region.extent.width = 4;
    copy_region.srcOffset.x = -8;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00144");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.width = 32;

    // Source exceeded in y-dim
    copy_region.srcOffset.x = 0;
    copy_region.extent.height = 48;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Source exceeded in z-dim
    copy_region.extent = {4, 4, 4};
    copy_region.srcSubresource.mipLevel = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00147");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageDstSizeExceeded) {
    // Image copy with dest region specified greater than dest image size
    ASSERT_NO_FATAL_FAILURE(Init());

    // Create images with full mip chain
    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj dst_image(m_device);
    dst_image.init(&ci);
    ASSERT_TRUE(dst_image.initialized());

    // Src image with one more mip level
    ci.extent = {64, 64, 16};
    ci.mipLevels = 7;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkImageObj src_image(m_device);
    src_image.init(&ci);
    ASSERT_TRUE(src_image.initialized());

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 32, 8};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // Dest exceeded in x-dim
    copy_region.dstOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in x-dim in negative direction (since offset is a signed in)
    copy_region.extent.width = 4;
    copy_region.dstOffset.x = -8;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.width = 32;

    copy_region.dstOffset.x = 0;
    copy_region.extent.height = 48;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in z-dim
    copy_region.extent = {4, 4, 4};
    copy_region.dstSubresource.mipLevel = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00153");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageZeroSize) {
    TEST_DESCRIPTION("Image Copy with empty regions");
    ASSERT_NO_FATAL_FAILURE(Init());

    // Create images with full mip chain
    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj src_image(m_device);
    src_image.init(&ci);
    ASSERT_TRUE(src_image.initialized());

    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj dst_image(m_device);
    dst_image.init(&ci);
    ASSERT_TRUE(dst_image.initialized());

    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src_and_dst(*m_device, 16384, reqs);  // large enough for image

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    copy_region.extent = {4, 4, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-06670");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.extent = {0, 0, 4};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-06668");  // width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-06669");  // height
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    VkImageSubresourceLayers image_subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = image_subresource;
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.bufferOffset = 0;

    buffer_image_copy.imageExtent = {4, 0, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06660");
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06660");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    // depth is now zero
    buffer_image_copy.imageExtent = {4, 1, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageMultiPlaneSizeExceeded) {
    TEST_DESCRIPTION("Image Copy for multi-planar format that exceed size of plane for both src and dst");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions == true) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions == true) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Try to use VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM because need multi-plane format for some tests and likely supported due to
    // copy support being required with samplerYcbcrConversion feature
    VkFormatProperties props = {0, 0, 0};
    bool missing_format_support = false;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, &props);
    missing_format_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_format_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_format_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (missing_format_support == true) {
        printf("%s VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM transfer not supported; skipped.\n", kSkipPrefix);
        return;
    }

    // 128^2 texels in plane_0 and 64^2 texels in plane_1
    VkImageObj src_image(m_device);
    VkImageObj dst_image(m_device);
    src_image.Init(128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(src_image.initialized());
    dst_image.Init(128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(dst_image.initialized());

    VkImageCopy copy_region = {};
    copy_region.extent = {64, 64, 1};  // Size of plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    VkImageCopy original_region = copy_region;

    m_commandBuffer->begin();

    // Should be able to do a 64x64 copy from plane 1 -> Plane 1
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // Should be able to do a 64x64 copy from plane 0 -> Plane 0
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    VkMemoryBarrier mem_barrier = LvlInitStruct<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // Should be able to do a 64x64 copy from plane 0 -> Plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->ExpectSuccess();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // Should be able to do a 64x64 copy from plane 0 -> Plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->ExpectSuccess();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // Should be able to do a 128x64 copy from plane 0 -> Plane 0
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->ExpectSuccess();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyNotFound();

    // 128x64 copy from plane 0 -> Plane 1
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // 128x64 copy from plane 1 -> Plane 0
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00144");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // src exceeded in y-dim from offset
    copy_region = original_region;
    copy_region.srcOffset.y = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // dst exceeded in y-dim from offset
    copy_region = original_region;
    copy_region.dstOffset.y = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");
    m_commandBuffer->CopyImage(src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageFormatSizeMismatch) {
    if (!EnableDeviceProfileLayer()) {
        GTEST_SKIP() << "Failed to enable device profile layer.";
    }

    // Enable KHR multiplane req'd extensions
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto mp_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(mp_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to device profile layer.";
    }

    // Set transfer for all potential used formats
    VkFormatProperties format_props;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R8_UNORM, &format_props);
    format_props.optimalTilingFeatures |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R8_UNORM, format_props);

    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R8_UINT, &format_props);
    format_props.optimalTilingFeatures |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R8_UINT, format_props);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = 0;

    image_create_info.format = VK_FORMAT_R8_UNORM;
    VkImageObj image_8b_unorm(m_device);
    image_8b_unorm.init(&image_create_info);

    image_create_info.format = VK_FORMAT_R8_UINT;
    VkImageObj image_8b_uint(m_device);
    image_8b_uint.init(&image_create_info);

    // First try to test two single plane mismatch
    {
        fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_props);
        format_props.optimalTilingFeatures |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R8G8B8A8_UNORM, format_props);

        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        VkImageObj image_32b_unorm(m_device);
        image_32b_unorm.init(&image_create_info);

        m_commandBuffer->begin();
        VkImageCopy copyRegion;
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset.x = 0;
        copyRegion.srcOffset.y = 0;
        copyRegion.srcOffset.z = 0;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset.x = 0;
        copyRegion.dstOffset.y = 0;
        copyRegion.dstOffset.z = 0;
        copyRegion.extent.width = 1;
        copyRegion.extent.height = 1;
        copyRegion.extent.depth = 1;

        // Sanity check between two 8bit formats
        m_commandBuffer->CopyImage(image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_uint.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        const char *vuid = (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME))
                               ? "VUID-vkCmdCopyImage-srcImage-01548"
                               : "VUID-vkCmdCopyImage-srcImage-00135";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        m_commandBuffer->CopyImage(image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_32b_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        // Swap src and dst
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        m_commandBuffer->CopyImage(image_32b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }

    // DstImage is a mismatched plane of a multi-planar format
    if (!mp_features.samplerYcbcrConversion) {
        printf("%s No multi-planar support; section of tests skipped.\n", kSkipPrefix);
    } else {
        fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &format_props);
        format_props.optimalTilingFeatures |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, format_props);

        image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        VkImageObj image_8b_16b_420_unorm(m_device);
        image_8b_16b_420_unorm.init(&image_create_info);

        m_commandBuffer->begin();
        VkImageCopy copyRegion;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset.x = 0;
        copyRegion.srcOffset.y = 0;
        copyRegion.srcOffset.z = 0;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset.x = 0;
        copyRegion.dstOffset.y = 0;
        copyRegion.dstOffset.z = 0;
        copyRegion.extent.width = 1;
        copyRegion.extent.height = 1;
        copyRegion.extent.depth = 1;

        // First test single-plane -> multi-plan
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;

        // Plane 0 is VK_FORMAT_R8_UNORM so this should succeed
        m_commandBuffer->CopyImage(image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_16b_420_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        image_8b_16b_420_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);

        // Make sure no false postiives if Compatible format
        m_commandBuffer->CopyImage(image_8b_uint.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_16b_420_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        // Plane 1 is VK_FORMAT_R8G8_UNORM so this should fail
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-None-01549");
        m_commandBuffer->CopyImage(image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_16b_420_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        // Same tests but swap src and dst
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        image_8b_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                          VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_8b_16b_420_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);

        m_commandBuffer->CopyImage(image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        image_8b_16b_420_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);

        m_commandBuffer->CopyImage(image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_uint.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-None-01549");
        m_commandBuffer->CopyImage(image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                                   VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(VkLayerTest, CopyImageDepthStencilFormatMismatch) {
    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s Couldn't depth stencil image format.\n", kSkipPrefix);
        return;
    }

    VkFormatProperties properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D32_SFLOAT, &properties);
    if (properties.optimalTilingFeatures == 0) {
        printf("%s Image format not supported; skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj srcImage(m_device);
    srcImage.Init(32, 32, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(srcImage.initialized());
    VkImageObj dstImage(m_device);
    dstImage.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(dstImage.initialized());

    // Create two images of different types and try to copy between them

    m_commandBuffer->begin();
    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset.x = 0;
    copyRegion.srcOffset.y = 0;
    copyRegion.srcOffset.z = 0;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset.x = 0;
    copyRegion.dstOffset.y = 0;
    copyRegion.dstOffset.z = 0;
    copyRegion.extent.width = 1;
    copyRegion.extent.height = 1;
    copyRegion.extent.depth = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00135");
    m_commandBuffer->CopyImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copyRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CopyImageSampleCountMismatch) {
    TEST_DESCRIPTION("Image copies with sample count mis-matches");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0,
                                               &image_format_properties);

    if ((0 == (VK_SAMPLE_COUNT_2_BIT & image_format_properties.sampleCounts)) ||
        (0 == (VK_SAMPLE_COUNT_4_BIT & image_format_properties.sampleCounts))) {
        printf("%s Image multi-sample support not found; skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image1(m_device);
    image1.init(&ci);
    ASSERT_TRUE(image1.initialized());

    ci.samples = VK_SAMPLE_COUNT_2_BIT;
    VkImageObj image2(m_device);
    image2.init(&ci);
    ASSERT_TRUE(image2.initialized());

    ci.samples = VK_SAMPLE_COUNT_4_BIT;
    VkImageObj image4(m_device);
    image4.init(&ci);
    ASSERT_TRUE(image4.initialized());

    m_commandBuffer->begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {128, 128, 1};

    // Copy a single sample image to/from a multi-sample image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_commandBuffer->handle(), image1.handle(), VK_IMAGE_LAYOUT_GENERAL, image4.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copyRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_commandBuffer->handle(), image2.handle(), VK_IMAGE_LAYOUT_GENERAL, image1.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Copy between multi-sample images with different sample counts
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_commandBuffer->handle(), image2.handle(), VK_IMAGE_LAYOUT_GENERAL, image4.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copyRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_commandBuffer->handle(), image4.handle(), VK_IMAGE_LAYOUT_GENERAL, image2.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copyRegion);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyImageAspectMismatch) {
    TEST_DESCRIPTION("Image copies with aspect mask errors");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Required extensions are not avaiable.\n", kSkipPrefix);
        return;
    }

    auto ds_format = FindSupportedDepthStencilFormat(gpu());
    if (!ds_format) {
        printf("%s Couldn't find depth stencil format.\n", kSkipPrefix);
        return;
    }

    // Add Transfer support for all used formats
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32_SFLOAT, &formatProps);
    formatProps.optimalTilingFeatures |= (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32_SFLOAT, formatProps);
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D32_SFLOAT, &formatProps);
    formatProps.optimalTilingFeatures |= (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32_SFLOAT, formatProps);
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), ds_format, &formatProps);
    formatProps.optimalTilingFeatures |= (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), ds_format, formatProps);

    VkImageObj color_image(m_device), ds_image(m_device), depth_image(m_device);
    color_image.Init(128, 128, 1, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    depth_image.Init(128, 128, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                     VK_IMAGE_TILING_OPTIMAL, 0);
    ds_image.Init(128, 128, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                  VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(color_image.initialized());
    ASSERT_TRUE(depth_image.initialized());
    ASSERT_TRUE(ds_image.initialized());

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {64, 0, 0};
    copyRegion.extent = {64, 128, 1};

    // Submitting command before command buffer is in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "You must call vkBeginCommandBuffer");  // "VUID-vkCmdCopyImage-commandBuffer-recording");
    vk::CmdCopyImage(m_commandBuffer->handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Src and dest aspect masks don't match
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    const char *vuid = mp_extensions ? "VUID-vkCmdCopyImage-srcImage-01551" : "VUID-VkImageCopy-aspectMask-00137";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), ds_image.handle(), VK_IMAGE_LAYOUT_GENERAL, ds_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Illegal combinations of aspect bits
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;  // color must be alone
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00167");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00142");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();
    // same test for dstSubresource
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;  // color must be alone
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00167");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00143");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Metadata aspect is illegal
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00168");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();
    // same test for dstSubresource
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00168");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Aspect Memory Plane mask is illegal
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-02247");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    const char *compatible_vuid = mp_extensions ? "VUID-vkCmdCopyImage-srcImage-01548" : "VUID-vkCmdCopyImage-srcImage-00135";

    // Aspect mask doesn't match source image format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00142");
    // Again redundant but unavoidable
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, compatible_vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Aspect mask doesn't match dest image format
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00143");
    // Again redundant but unavoidable
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, compatible_vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Check no performance warnings regarding layout are thrown when copying from and to the same image
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->ExpectSuccess(kPerformanceWarningBit);
    vk::CmdCopyImage(m_commandBuffer->handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ResolveImageLowSampleCount) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-00257");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create two images of sample count 1 and try to Resolve between them

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
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = 0;

    VkImageObj srcImage(m_device);
    srcImage.init(&image_create_info);
    ASSERT_TRUE(srcImage.initialized());

    VkImageObj dstImage(m_device);
    dstImage.init(&image_create_info);
    ASSERT_TRUE(dstImage.initialized());

    m_commandBuffer->begin();
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;
    m_commandBuffer->ResolveImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ResolveImageHighSampleCount) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00259");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create two images of sample count 4 and try to Resolve between them

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj srcImage(m_device);
    srcImage.init(&image_create_info);
    ASSERT_TRUE(srcImage.initialized());

    VkImageObj dstImage(m_device);
    dstImage.init(&image_create_info);
    ASSERT_TRUE(dstImage.initialized());

    m_commandBuffer->begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;
    m_commandBuffer->ResolveImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ResolveImageFormatMismatch) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-01386");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    srcImage.init(&image_create_info);

    // Set format to something other than source image
    image_create_info.format = VK_FORMAT_R32_SFLOAT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dstImage.init(&image_create_info);

    m_commandBuffer->begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;
    m_commandBuffer->ResolveImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ResolveImageTypeMismatch) {
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-CoreValidation-DrawState-MismatchedImageType");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    srcImage.init(&image_create_info);

    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dstImage.init(&image_create_info);

    m_commandBuffer->begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;
    m_commandBuffer->ResolveImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ResolveImageLayoutMismatch) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;
    srcImage.init(&image_create_info);
    ASSERT_TRUE(srcImage.initialized());

    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dstImage.init(&image_create_info);
    ASSERT_TRUE(dstImage.initialized());

    m_commandBuffer->begin();
    // source image must have valid contents before resolve
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.layerCount = 1;
    subresource.levelCount = 1;
    srcImage.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_commandBuffer->ClearColorImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &subresource);
    srcImage.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dstImage.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;
    // source image layout mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImageLayout-00260");
    m_commandBuffer->ResolveImage(srcImage.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage.image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    // dst image layout mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImageLayout-00262");
    m_commandBuffer->ResolveImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(), VK_IMAGE_LAYOUT_GENERAL,
                                  1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ResolveInvalidSubresource) {
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool copy_commands2 = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
        copy_commands2 = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdResolveImage2KHR vkCmdResolveImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdResolveImage2Function = (PFN_vkCmdResolveImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdResolveImage2KHR");
    }

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;
    srcImage.init(&image_create_info);
    ASSERT_TRUE(srcImage.initialized());

    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dstImage.init(&image_create_info);
    ASSERT_TRUE(dstImage.initialized());

    m_commandBuffer->begin();
    // source image must have valid contents before resolve
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.layerCount = 1;
    subresource.levelCount = 1;
    srcImage.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_commandBuffer->ClearColorImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &subresource);
    srcImage.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dstImage.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;
    // invalid source mip level
    resolveRegion.srcSubresource.mipLevel = image_create_info.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcSubresource-01709");
    m_commandBuffer->ResolveImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdResolveImage2Function) {
        const VkImageResolve2KHR resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR,
                                                   NULL,
                                                   resolveRegion.srcSubresource,
                                                   resolveRegion.srcOffset,
                                                   resolveRegion.dstSubresource,
                                                   resolveRegion.dstOffset,
                                                   resolveRegion.extent};
        const VkResolveImageInfo2KHR resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR,
                                                            NULL,
                                                            srcImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                            dstImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                            1,
                                                            &resolveRegion2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkResolveImageInfo2-srcSubresource-01709");
        vkCmdResolveImage2Function(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.srcSubresource.mipLevel = 0;
    // invalid dest mip level
    resolveRegion.dstSubresource.mipLevel = image_create_info.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstSubresource-01710");
    m_commandBuffer->ResolveImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdResolveImage2Function) {
        const VkImageResolve2KHR resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR,
                                                   NULL,
                                                   resolveRegion.srcSubresource,
                                                   resolveRegion.srcOffset,
                                                   resolveRegion.dstSubresource,
                                                   resolveRegion.dstOffset,
                                                   resolveRegion.extent};
        const VkResolveImageInfo2KHR resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR,
                                                            NULL,
                                                            srcImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                            dstImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                            1,
                                                            &resolveRegion2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkResolveImageInfo2-dstSubresource-01710");
        vkCmdResolveImage2Function(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.dstSubresource.mipLevel = 0;
    // invalid source array layer range
    resolveRegion.srcSubresource.baseArrayLayer = image_create_info.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcSubresource-01711");
    m_commandBuffer->ResolveImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdResolveImage2Function) {
        const VkImageResolve2KHR resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR,
                                                   NULL,
                                                   resolveRegion.srcSubresource,
                                                   resolveRegion.srcOffset,
                                                   resolveRegion.dstSubresource,
                                                   resolveRegion.dstOffset,
                                                   resolveRegion.extent};
        const VkResolveImageInfo2KHR resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR,
                                                            NULL,
                                                            srcImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                            dstImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                            1,
                                                            &resolveRegion2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkResolveImageInfo2-srcSubresource-01711");
        vkCmdResolveImage2Function(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.srcSubresource.baseArrayLayer = 0;
    // invalid dest array layer range
    resolveRegion.dstSubresource.baseArrayLayer = image_create_info.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstSubresource-01712");
    m_commandBuffer->ResolveImage(srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdResolveImage2Function) {
        const VkImageResolve2KHR resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR,
                                                   NULL,
                                                   resolveRegion.srcSubresource,
                                                   resolveRegion.srcOffset,
                                                   resolveRegion.dstSubresource,
                                                   resolveRegion.dstOffset,
                                                   resolveRegion.extent};
        const VkResolveImageInfo2KHR resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR,
                                                            NULL,
                                                            srcImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                            dstImage.image(),
                                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                            1,
                                                            &resolveRegion2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkResolveImageInfo2-dstSubresource-01712");
        vkCmdResolveImage2Function(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.dstSubresource.baseArrayLayer = 0;

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ResolveImageImageType) {
    ASSERT_NO_FATAL_FAILURE(Init());
    // Create images of different types and try to resolve between them
    VkImageObj srcImage2D(m_device);
    VkImageObj dstImage1D(m_device);
    VkImageObj dstImage3D(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;  // more than 1 to not trip other validation
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    srcImage2D.init(&image_create_info);
    ASSERT_TRUE(srcImage2D.initialized());

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    dstImage1D.init(&image_create_info);
    ASSERT_TRUE(dstImage1D.initialized());

    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.height = 16;
    image_create_info.extent.depth = 16;
    image_create_info.arrayLayers = 1;
    dstImage3D.init(&image_create_info);
    ASSERT_TRUE(dstImage3D.initialized());

    m_commandBuffer->begin();

    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;

    // non-zero value baseArrayLayer
    resolveRegion.srcSubresource.baseArrayLayer = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-04446");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage3D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcSubresource.baseArrayLayer = 0;

    // Set height with 1D dstImage
    resolveRegion.extent.height = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00276");
    // Also exceed height of both images
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00270");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00275");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage1D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.extent.height = 1;

    // Set depth with 1D dstImage and 2D srcImage
    resolveRegion.extent.depth = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00278");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-00273");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage1D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.extent.depth = 1;

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ResolveImageSizeExceeded) {
    TEST_DESCRIPTION("Resolve Image with subresource region greater than size of src/dst image");
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->ExpectSuccess();
    VkImageObj srcImage2D(m_device);
    VkImageObj dstImage2D(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    srcImage2D.init(&image_create_info);
    ASSERT_TRUE(srcImage2D.initialized());

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dstImage2D.init(&image_create_info);
    ASSERT_TRUE(dstImage2D.initialized());

    m_commandBuffer->begin();

    VkImageResolve resolveRegion = {};
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 32;
    resolveRegion.extent.height = 32;
    resolveRegion.extent.depth = 1;

    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyNotFound();

    // srcImage exceeded in x-dim
    resolveRegion.srcOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00269");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.x = 0;

    // dstImage exceeded in x-dim
    resolveRegion.dstOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00274");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.x = 0;

    // both image exceeded in y-dim
    resolveRegion.srcOffset.y = 32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00270");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.y = 0;

    resolveRegion.dstOffset.y = 32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00275");
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.y = 0;

    // srcImage exceeded in z-dim
    resolveRegion.srcOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00272");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-00273");  // because it's a 2d image
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.z = 0;

    // dstImage exceeded in z-dim
    resolveRegion.dstOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00277");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00278");  // because it's a 2d image
    m_commandBuffer->ResolveImage(srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.z = 0;

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ClearImageErrors) {
    TEST_DESCRIPTION("Call ClearColorImage w/ a depth|stencil image and ClearDepthStencilImage with a color image.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    // Color image
    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);
    const VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t img_width = 32;
    const int32_t img_height = 32;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = color_format;
    image_create_info.extent.width = img_width;
    image_create_info.extent.height = img_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk_testing::Image color_image_no_transfer;
    color_image_no_transfer.init(*m_device, image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vk_testing::Image color_image;
    color_image.init(*m_device, image_create_info);

    const VkImageSubresourceRange color_range = vk_testing::Image::subresource_range(image_create_info, VK_IMAGE_ASPECT_COLOR_BIT);

    // Depth/Stencil image
    VkClearDepthStencilValue clear_value = {0};
    VkImageCreateInfo ds_image_create_info = vk_testing::Image::create_info();
    ds_image_create_info.imageType = VK_IMAGE_TYPE_2D;
    ds_image_create_info.format = VK_FORMAT_D16_UNORM;
    ds_image_create_info.extent.width = 64;
    ds_image_create_info.extent.height = 64;
    ds_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    ds_image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    vk_testing::Image ds_image;
    ds_image.init(*m_device, ds_image_create_info);

    const VkImageSubresourceRange ds_range = vk_testing::Image::subresource_range(ds_image_create_info, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-image-00007");

    vk::CmdClearColorImage(m_commandBuffer->handle(), ds_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &color_range);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-image-00002");

    vk::CmdClearColorImage(m_commandBuffer->handle(), color_image_no_transfer.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1,
                           &color_range);

    m_errorMonitor->VerifyFound();

    // Call CmdClearDepthStencilImage with color image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-00014");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-02826");

    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  &clear_value, 1, &ds_range);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CommandQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics and try to issue a graphics-only command");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queueFamilyIndex = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (queueFamilyIndex == UINT32_MAX) {
        printf("%s Non-graphics queue family not found; skipped.\n", kSkipPrefix);
        return;
    } else {
        // Create command pool on a non-graphics queue
        VkCommandPoolObj command_pool(m_device, queueFamilyIndex);

        // Setup command buffer on pool
        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();

        // Issue a graphics only command
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-commandBuffer-cmdpool");
        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        command_buffer.SetViewport(0, 1, &viewport);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DepthStencilImageCopyNoGraphicsQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics and try to issue a depth/stencil image copy to "
        "buffer");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queueFamilyIndex = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (queueFamilyIndex == UINT32_MAX) {
        printf("%s Non-graphics queue family not found; skipped.\n", kSkipPrefix);
        return;
    } else {
        // Create Depth image
        const VkFormat ds_format = FindSupportedDepthOnlyFormat(gpu());
        if (ds_format == VK_FORMAT_UNDEFINED) {
            printf("%s No only Depth format found. Skipped.\n", kSkipPrefix);
            return;
        }

        VkImageObj ds_image(m_device);
        ds_image.Init(64, 64, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ds_image.initialized());

        // Allocate buffers
        VkBufferObj buffer;
        VkMemoryPropertyFlags reqs = 0;
        buffer.init_as_src_and_dst(*m_device, 262144, reqs);  // 256k to have more then enough to copy

        VkBufferImageCopy region = {};
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {64, 64, 1};
        region.bufferOffset = 0;

        // Create command pool on a non-graphics queue
        VkCommandPoolObj command_pool(m_device, queueFamilyIndex);

        // Setup command buffer on pool
        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-04477");
        vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), ds_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 1, &region);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ImageCopyTransferQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics/compute and try to issue an invalid image copy to "
        "buffer");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Should be left with a tranfser queue
    uint32_t queueFamilyIndex = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    if (queueFamilyIndex == UINT32_MAX) {
        printf("%s Non-graphics/compute queue family not found; skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    // Allocate buffers
    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src_and_dst(*m_device, 262144, reqs);  // 256k to have more then enough to copy

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {16, 16, 1};
    region.bufferOffset = 5;

    // Create command pool on a non-graphics queue
    VkCommandPoolObj command_pool(m_device, queueFamilyIndex);

    // Setup command buffer on pool
    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-bufferOffset-00193");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-04052");
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExecuteDiffertQueueFlagsSecondaryCB) {
    TEST_DESCRIPTION("Allocate a command buffer from two different queues and try to use a secondary command buffer");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (m_device->queue_props.size() < 2) {
        GTEST_SKIP() << "Need 2 different queues for testing skipping.";
    }

    // First two queue families
    uint32_t queue_index_a = 0;
    uint32_t queue_index_b = 1;

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.flags = 0;

    pool_create_info.queueFamilyIndex = queue_index_a;
    vk_testing::CommandPool command_pool_a(*m_device, pool_create_info);
    ASSERT_TRUE(command_pool_a.initialized());

    pool_create_info.queueFamilyIndex = queue_index_b;
    vk_testing::CommandPool command_pool_b(*m_device, pool_create_info);
    ASSERT_TRUE(command_pool_b.initialized());

    auto command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.commandPool = command_pool_a.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk_testing::CommandBuffer command_buffer_primary(*m_device, command_buffer_allocate_info);
    ASSERT_TRUE(command_buffer_primary.initialized());

    command_buffer_allocate_info.commandPool = command_pool_b.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    vk_testing::CommandBuffer command_buffer_secondary(*m_device, command_buffer_allocate_info);

    auto cmdbuff_ii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmdbuff_ii.renderPass = m_renderPass;
    cmdbuff_ii.subpass = 0;
    cmdbuff_ii.framebuffer = m_framebuffer;

    auto begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    begin_info.pInheritanceInfo = &cmdbuff_ii;

    // secondary
    command_buffer_secondary.begin(&begin_info);
    command_buffer_secondary.end();

    // Try using different pool's command buffer as secondary
    command_buffer_primary.begin(&begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00094");
    vk::CmdExecuteCommands(command_buffer_primary.handle(), 1, &command_buffer_secondary.handle());
    m_errorMonitor->VerifyFound();
    command_buffer_primary.end();
}

TEST_F(VkLayerTest, ExecuteUnrecordedSecondaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a CB in the initial state");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    // never record secondary

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00089");
    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ExecuteSecondaryCBWithLayoutMismatch) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a CB with incorrect initial layout.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

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
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = 0;

    VkImageSubresource image_sub = VkImageObj::subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
    VkImageSubresourceRange image_sub_range = VkImageObj::subresource_range(image_sub);

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier image_barrier =
        image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, image_sub_range);

    auto pipeline = [&image_barrier](const VkCommandBufferObj &cb, VkImageLayout old_layout, VkImageLayout new_layout) {
        image_barrier.oldLayout = old_layout;
        image_barrier.newLayout = new_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
    };

    // Validate that mismatched use of image layout in secondary command buffer is caught at record time
    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    pipeline(secondary, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    secondary.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001");
    m_commandBuffer->begin();
    pipeline(*m_commandBuffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->reset();
    secondary.reset();

    // Validate that UNDEFINED doesn't false positive on us
    secondary.begin();
    pipeline(secondary, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    secondary.end();
    m_commandBuffer->begin();
    pipeline(*m_commandBuffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_errorMonitor->ExpectSuccess();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SetDynViewportParamTests) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetViewport without multiViewport feature");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    const VkViewport vp = {0.0, 0.0, 64.0, 64.0, 0.0, 1.0};
    const VkViewport viewports[] = {vp, vp};

    m_commandBuffer->begin();

    // array tests
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01224");
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 1, viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-01225");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 2, viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01224");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-01225");
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 2, viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-pViewports-parameter");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, nullptr);
    m_errorMonitor->VerifyFound();

    // core viewport tests
    using std::vector;
    struct TestCase {
        VkViewport vp;
        std::string veid;
    };

    // not necessarily boundary values (unspecified cast rounding), but guaranteed to be over limit
    const auto one_past_max_w = NearestGreater(static_cast<float>(m_device->props.limits.maxViewportDimensions[0]));
    const auto one_past_max_h = NearestGreater(static_cast<float>(m_device->props.limits.maxViewportDimensions[1]));

    const auto min_bound = m_device->props.limits.viewportBoundsRange[0];
    const auto max_bound = m_device->props.limits.viewportBoundsRange[1];
    const auto one_before_min_bounds = NearestSmaller(min_bound);
    const auto one_past_max_bounds = NearestGreater(max_bound);

    const auto below_zero = NearestSmaller(0.0f);
    const auto past_one = NearestGreater(1.0f);

    vector<TestCase> test_cases = {
        {{0.0, 0.0, 0.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-width-01770"},
        {{0.0, 0.0, one_past_max_w, 64.0, 0.0, 1.0}, "VUID-VkViewport-width-01771"},
        {{0.0, 0.0, NAN, 64.0, 0.0, 1.0}, "VUID-VkViewport-width-01770"},
        {{0.0, 0.0, 64.0, one_past_max_h, 0.0, 1.0}, "VUID-VkViewport-height-01773"},
        {{one_before_min_bounds, 0.0, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01774"},
        {{one_past_max_bounds, 0.0, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01232"},
        {{NAN, 0.0, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01774"},
        {{0.0, one_before_min_bounds, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-y-01775"},
        {{0.0, NAN, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-y-01775"},
        {{max_bound, 0.0, 1.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01232"},
        {{0.0, max_bound, 64.0, 1.0, 0.0, 1.0}, "VUID-VkViewport-y-01233"},
        {{0.0, 0.0, 64.0, 64.0, below_zero, 1.0}, "VUID-VkViewport-minDepth-01234"},
        {{0.0, 0.0, 64.0, 64.0, past_one, 1.0}, "VUID-VkViewport-minDepth-01234"},
        {{0.0, 0.0, 64.0, 64.0, NAN, 1.0}, "VUID-VkViewport-minDepth-01234"},
        {{0.0, 0.0, 64.0, 64.0, 0.0, below_zero}, "VUID-VkViewport-maxDepth-01235"},
        {{0.0, 0.0, 64.0, 64.0, 0.0, past_one}, "VUID-VkViewport-maxDepth-01235"},
        {{0.0, 0.0, 64.0, 64.0, 0.0, NAN}, "VUID-VkViewport-maxDepth-01235"},
    };

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        test_cases.push_back({{0.0, 0.0, 64.0, 0.0, 0.0, 1.0}, "VUID-VkViewport-height-01772"});
        test_cases.push_back({{0.0, 0.0, 64.0, NAN, 0.0, 1.0}, "VUID-VkViewport-height-01772"});
    } else {
        test_cases.push_back({{0.0, 0.0, 64.0, NAN, 0.0, 1.0}, "VUID-VkViewport-height-01773"});
    }

    for (const auto &test_case : test_cases) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.veid);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &test_case.vp);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, SetDynViewportParamMaintenance1Tests) {
    TEST_DESCRIPTION("Verify errors are detected on misuse of SetViewport with a negative viewport extension enabled.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    } else {
        printf("%s VK_KHR_maintenance1 extension not supported -- skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    NegHeightViewportTests(m_device, m_commandBuffer, m_errorMonitor);
}

TEST_F(VkLayerTest, SetDynViewportParamMultiviewportTests) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetViewport with multiViewport feature enabled");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().multiViewport) {
        printf("%s VkPhysicalDeviceFeatures::multiViewport is not supported -- skipping test.\n", kSkipPrefix);
        return;
    }

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    const auto max_viewports = m_device->props.limits.maxViewports;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-pViewports-parameter");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, max_viewports, nullptr);
    m_errorMonitor->VerifyFound();

    const uint32_t too_big_max_viewports = 65536 + 1;  // let's say this is too much to allocate
    if (max_viewports >= too_big_max_viewports) {
        printf("%s VkPhysicalDeviceLimits::maxViewports is too large to practically test against -- skipping part of test.\n",
               kSkipPrefix);
    } else {
        const VkViewport vp = {0.0, 0.0, 64.0, 64.0, 0.0, 1.0};
        const std::vector<VkViewport> viewports(max_viewports + 1, vp);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01223");
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, max_viewports + 1, viewports.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01223");
        vk::CmdSetViewport(m_commandBuffer->handle(), max_viewports, 1, viewports.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01223");
        vk::CmdSetViewport(m_commandBuffer->handle(), 1, max_viewports, viewports.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
        vk::CmdSetViewport(m_commandBuffer->handle(), 1, 0, viewports.data());
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, BadRenderPassScopeSecondaryCmdBuffer) {
    TEST_DESCRIPTION(
        "Test secondary buffers executed in wrong render pass scope wrt VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj sec_cmdbuff_inside_rp(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferObj sec_cmdbuff_outside_rp(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        m_renderPass,
        0,  // subpass
        m_framebuffer,
    };
    const VkCommandBufferBeginInfo cmdbuff_bi_tmpl = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                      nullptr,  // pNext
                                                      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};

    VkCommandBufferBeginInfo cmdbuff_inside_rp_bi = cmdbuff_bi_tmpl;
    cmdbuff_inside_rp_bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    sec_cmdbuff_inside_rp.begin(&cmdbuff_inside_rp_bi);
    sec_cmdbuff_inside_rp.end();

    VkCommandBufferBeginInfo cmdbuff_outside_rp_bi = cmdbuff_bi_tmpl;
    cmdbuff_outside_rp_bi.flags &= ~VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    sec_cmdbuff_outside_rp.begin(&cmdbuff_outside_rp_bi);
    sec_cmdbuff_outside_rp.end();

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00100");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &sec_cmdbuff_inside_rp.handle());
    m_errorMonitor->VerifyFound();

    const VkRenderPassBeginInfo rp_bi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                      nullptr,  // pNext
                                      m_renderPass,
                                      m_framebuffer,
                                      {{0, 0}, {32, 32}},
                                      static_cast<uint32_t>(m_renderPassClearValues.size()),
                                      m_renderPassClearValues.data()};
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00096");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &sec_cmdbuff_outside_rp.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SecondaryCommandBufferClearColorAttachmentsRenderArea) {
    TEST_DESCRIPTION(
        "Create a secondary command buffer with CmdClearAttachments call that has a rect outside of renderPass renderArea");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_VK_SUCCESS(vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer));
    VkCommandBufferBeginInfo command_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    command_buffer_inheritance_info.renderPass = m_renderPass;
    command_buffer_inheritance_info.framebuffer = m_framebuffer;

    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    vk::BeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    // x extent of 257 exceeds render area of 256
    VkClearRect clear_rect = {{{0, 0}, {257, 32}}, 0, 1};
    vk::CmdClearAttachments(secondary_command_buffer, 1, &color_attachment, 1, &clear_rect);
    vk::EndCommandBuffer(secondary_command_buffer);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00016");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_command_buffer);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, PushDescriptorSetCmdPushBadArgs) {
    TEST_DESCRIPTION("Attempt to push a push descriptor set with incorrect arguments.");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    // Create ordinary and push descriptor set layout
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj ds_layout(m_device, {binding});
    ASSERT_TRUE(ds_layout.initialized());
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    ASSERT_TRUE(push_ds_layout.initialized());

    // Now use the descriptor set layouts to create a pipeline layout
    const VkPipelineLayoutObj pipeline_layout(m_device, {&push_ds_layout, &ds_layout});
    ASSERT_TRUE(pipeline_layout.initialized());

    // Create a descriptor to push
    const uint32_t buffer_data[4] = {4, 5, 6, 7};
    VkConstantBufferObj buffer_obj(m_device, sizeof(buffer_data), &buffer_data);
    ASSERT_TRUE(buffer_obj.initialized());

    // Create a "write" struct, noting that the buffer_info cannot be a temporary arg (the return from write_descriptor_set
    // references its data), and the DescriptorSet() can be temporary, because the value is ignored
    VkDescriptorBufferInfo buffer_info = {buffer_obj.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_write = vk_testing::Device::write_descriptor_set(
        vk_testing::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    // Find address of extension call and make the call
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    ASSERT_TRUE(vkCmdPushDescriptorSetKHR != nullptr);

    // Section 1: Queue family matching/capabilities.
    // Create command pool on a non-graphics queue
    const uint32_t no_gfx_qfi = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    const uint32_t transfer_only_qfi =
        m_device->QueueFamilyMatching(VK_QUEUE_TRANSFER_BIT, (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT));
    if ((UINT32_MAX == transfer_only_qfi) && (UINT32_MAX == no_gfx_qfi)) {
        printf("%s No compute or transfer only queue family, skipping bindpoint and queue tests.\n", kSkipPrefix);
    } else {
        const uint32_t err_qfi = (UINT32_MAX == no_gfx_qfi) ? transfer_only_qfi : no_gfx_qfi;

        VkCommandPoolObj command_pool(m_device, err_qfi);
        ASSERT_TRUE(command_pool.initialized());
        VkCommandBufferObj command_buffer(m_device, &command_pool);
        ASSERT_TRUE(command_buffer.initialized());
        command_buffer.begin();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00330");
        if (err_qfi == transfer_only_qfi) {
            // This as this queue neither supports the gfx or compute bindpoints, we'll get two errors
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-cmdpool");
        }
        vkCmdPushDescriptorSetKHR(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_write);
        m_errorMonitor->VerifyFound();
        command_buffer.end();

        // If we succeed in testing only one condition above, we need to test the other below.
        if ((UINT32_MAX != transfer_only_qfi) && (err_qfi != transfer_only_qfi)) {
            // Need to test the neither compute/gfx supported case separately.
            VkCommandPoolObj tran_command_pool(m_device, transfer_only_qfi);
            ASSERT_TRUE(tran_command_pool.initialized());
            VkCommandBufferObj tran_command_buffer(m_device, &tran_command_pool);
            ASSERT_TRUE(tran_command_buffer.initialized());
            tran_command_buffer.begin();

            // We can't avoid getting *both* errors in this case
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00330");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-cmdpool");
            vkCmdPushDescriptorSetKHR(tran_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                      &descriptor_write);
            m_errorMonitor->VerifyFound();
            tran_command_buffer.end();
        }
    }

    // Push to the non-push binding
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushDescriptorSetKHR-set-00365");
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &descriptor_write);
    m_errorMonitor->VerifyFound();

    // Specify set out of bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushDescriptorSetKHR-set-00364");
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 2, 1,
                              &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // This is a test for VUID-vkCmdPushDescriptorSetKHR-commandBuffer-recording
    // TODO: Add VALIDATION_ERROR_ code support to core_validation::ValidateCmd
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "You must call vkBeginCommandBuffer() before this call to vkCmdPushDescriptorSetKHR");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00330");
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_write);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PushDescriptorSetCmdBufferOffsetUnaligned) {
    TEST_DESCRIPTION("Attempt to push a push descriptor set buffer with unaligned offset.");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto const push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0.
        printf("%s maxPushDescriptors is zero, skipping test\n", kSkipPrefix);
        return;
    }

    auto const min_alignment = m_device->props.limits.minUniformBufferOffsetAlignment;
    if (min_alignment == 0) {
        printf("%s minUniformBufferOffsetAlignment is zero, skipping test\n", kSkipPrefix);
        return;
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    ASSERT_TRUE(push_ds_layout.initialized());

    const VkPipelineLayoutObj pipeline_layout(m_device, {&push_ds_layout});
    ASSERT_TRUE(pipeline_layout.initialized());

    const uint32_t buffer_data[4] = {4, 5, 6, 7};
    VkConstantBufferObj buffer_obj(m_device, sizeof(buffer_data), &buffer_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ASSERT_TRUE(buffer_obj.initialized());

    // Use an invalid alignment.
    VkDescriptorBufferInfo buffer_info = {buffer_obj.handle(), min_alignment - 1, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vk_testing::Device::write_descriptor_set(
        vk_testing::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    ASSERT_TRUE(vkCmdPushDescriptorSetKHR != nullptr);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00327");
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_write);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SetDynScissorParamTests) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetScissor without multiViewport feature");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    const VkRect2D scissor = {{0, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    m_commandBuffer->begin();

    // array tests
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00593");
    vk::CmdSetScissor(m_commandBuffer->handle(), 1, 1, scissors);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-arraylength");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-00594");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 2, scissors);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00593");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-00594");
    vk::CmdSetScissor(m_commandBuffer->handle(), 1, 2, scissors);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-pScissors-parameter");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, nullptr);
    m_errorMonitor->VerifyFound();

    struct TestCase {
        VkRect2D scissor;
        std::string vuid;
    };

    std::vector<TestCase> test_cases = {{{{-1, 0}, {16, 16}}, "VUID-vkCmdSetScissor-x-00595"},
                                        {{{0, -1}, {16, 16}}, "VUID-vkCmdSetScissor-x-00595"},
                                        {{{1, 0}, {INT32_MAX, 16}}, "VUID-vkCmdSetScissor-offset-00596"},
                                        {{{INT32_MAX, 0}, {1, 16}}, "VUID-vkCmdSetScissor-offset-00596"},
                                        {{{0, 0}, {uint32_t{INT32_MAX} + 1, 16}}, "VUID-vkCmdSetScissor-offset-00596"},
                                        {{{0, 1}, {16, INT32_MAX}}, "VUID-vkCmdSetScissor-offset-00597"},
                                        {{{0, INT32_MAX}, {16, 1}}, "VUID-vkCmdSetScissor-offset-00597"},
                                        {{{0, 0}, {16, uint32_t{INT32_MAX} + 1}}, "VUID-vkCmdSetScissor-offset-00597"}};

    for (const auto &test_case : test_cases) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.vuid);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &test_case.scissor);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SetDynScissorParamMultiviewportTests) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetScissor with multiViewport feature enabled");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().multiViewport) {
        printf("%s VkPhysicalDeviceFeatures::multiViewport is not supported -- skipping test.\n", kSkipPrefix);
        return;
    }

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-arraylength");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    const auto max_scissors = m_device->props.limits.maxViewports;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-pScissors-parameter");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, max_scissors, nullptr);
    m_errorMonitor->VerifyFound();

    const uint32_t too_big_max_scissors = 65536 + 1;  // let's say this is too much to allocate
    if (max_scissors >= too_big_max_scissors) {
        printf("%s VkPhysicalDeviceLimits::maxViewports is too large to practically test against -- skipping part of test.\n",
               kSkipPrefix);
    } else {
        const VkRect2D scissor = {{0, 0}, {16, 16}};
        const std::vector<VkRect2D> scissors(max_scissors + 1, scissor);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00592");
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, max_scissors + 1, scissors.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00592");
        vk::CmdSetScissor(m_commandBuffer->handle(), max_scissors, 1, scissors.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00592");
        vk::CmdSetScissor(m_commandBuffer->handle(), 1, max_scissors, scissors.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-arraylength");
        vk::CmdSetScissor(m_commandBuffer->handle(), 1, 0, scissors.data());
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MultiDrawTests) {
    TEST_DESCRIPTION("Test validation of multi_draw extension");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto multi_draw_features = LvlInitStruct<VkPhysicalDeviceMultiDrawFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multi_draw_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!multi_draw_features.multiDraw) {
        printf("%s Test requires (unsupported) multiDraw, skipping\n", kSkipPrefix);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_MULTI_DRAW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    } else {
        printf("%s VK_EXT_multi_draw extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    auto multi_draw_properties = LvlInitStruct<VkPhysicalDeviceMultiDrawPropertiesEXT>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&multi_draw_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &properties2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiEXT");
    auto vkCmdDrawMultiIndexedEXT =
        (PFN_vkCmdDrawMultiIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiIndexedEXT");
    assert(vkCmdDrawMultiEXT != nullptr && vkCmdDrawMultiIndexedEXT != nullptr);

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-02700");
    vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-02700");
    vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // New VUIDs added with multi_draw (also see GPU-AV)
    VkBufferObj buffer;
    buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    multi_draw_indices[2].indexCount = 513;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-firstIndex-04938");
    m_commandBuffer->BindIndexBuffer(&buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
    multi_draw_indices[2].indexCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-stride-04936");
    vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT) + 1);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-stride-04941");
    vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT) + 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-drawCount-04935");
    vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, nullptr, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940");
    vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, nullptr, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    if (multi_draw_properties.maxMultiDrawCount < UINT32_MAX) {
        uint32_t draw_count = multi_draw_properties.maxMultiDrawCount + 1;
        std::vector<VkMultiDrawInfoEXT> max_multi_draws(draw_count);
        std::vector<VkMultiDrawIndexedInfoEXT> max_multi_indexed_draws(draw_count);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-drawCount-04934");
        vkCmdDrawMultiEXT(m_commandBuffer->handle(), draw_count, max_multi_draws.data(), 1, 0, sizeof(VkMultiDrawInfoEXT));
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939");
        vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), draw_count, max_multi_indexed_draws.data(), 1, 0,
                                 sizeof(VkMultiDrawIndexedInfoEXT), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MultiDrawFeatures) {
    TEST_DESCRIPTION("Test validation of multi draw feature enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_MULTI_DRAW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    } else {
        printf("%s VK_EXT_multi_draw extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto multi_draw_props = LvlInitStruct<VkPhysicalDeviceMultiDrawPropertiesEXT>();
    auto pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&multi_draw_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);
    if (multi_draw_props.maxMultiDrawCount == 0) {
        // If using MockICD and devsim the value might be zero'ed and cause false errors
        return;
    }

    auto vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiEXT");
    auto vkCmdDrawMultiIndexedEXT =
        (PFN_vkCmdDrawMultiIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiIndexedEXT");
    assert(vkCmdDrawMultiEXT != nullptr && vkCmdDrawMultiIndexedEXT != nullptr);

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-04933");
    vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    VkBufferObj buffer;
    buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    m_commandBuffer->BindIndexBuffer(&buffer, 0, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-04937");
    vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IndirectDrawTests) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirect and vkCmdDrawIndexedIndirect");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    VkBufferObj draw_buffer;
    draw_buffer.init(*m_device, buffer_create_info);

    VkBufferObj draw_buffer_correct;
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    draw_buffer_correct.init(*m_device, buffer_create_info);

    // VUID-vkCmdDrawIndirect-buffer-02709
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-buffer-02709");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    // VUID-vkCmdDrawIndirect-drawCount-02718
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-drawCount-00488");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer_correct.handle(), 0, 2, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    // VUID-vkCmdDrawIndexedIndirect-commandBuffer-02701
    // VUID-vkCmdDrawIndexedIndirect-drawCount-00540
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02701");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), draw_buffer_correct.handle(), 0, 2, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DrawIndirectByteCountEXT) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirectByteCountEXT");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    } else {
        printf("%s VK_EXT_transform_feedback extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    auto pd_properties = LvlInitStruct<VkPhysicalDeviceProperties2>(&tf_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_properties);

    PFN_vkCmdDrawIndirectByteCountEXT fpvkCmdDrawIndirectByteCountEXT =
        (PFN_vkCmdDrawIndirectByteCountEXT)vk::GetDeviceProcAddr(device(), "vkCmdDrawIndirectByteCountEXT");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    buffer_create_info.size = 1024;
    VkBufferObj counter_buffer;
    counter_buffer.init(*m_device, buffer_create_info);

    // Greater stride than maxTransformFeedbackBufferDataStride
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289");
    fpvkCmdDrawIndirectByteCountEXT(m_commandBuffer->handle(), 1, 0, counter_buffer.handle(), 0, 0, 0xCADECADE);
    m_errorMonitor->VerifyFound();

    // some mock ICD json files are missing a valid stride value
    if (tf_properties.maxTransformFeedbackBufferDataStride > 0) {
        // non-4 multiple stride
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568");
        fpvkCmdDrawIndirectByteCountEXT(m_commandBuffer->handle(), 1, 0, counter_buffer.handle(), 0, 1, 4);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    if (!tf_properties.maxTransformFeedbackBufferDataStride) {
        printf("%s , maxTransformFeedbackBufferDataStride is zero, skipping subtests\n", kSkipPrefix);
        return;
    }

    std::vector<const char *> device_extension_names;
    device_extension_names.push_back(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    VkDeviceObj test_device(0, gpu(), device_extension_names);
    VkCommandPoolObj commandPool(&test_device, 0);
    VkCommandBufferObj commandBuffer(&test_device, &commandPool);
    VkBufferObj counter_buffer2;
    counter_buffer2.init(test_device, buffer_create_info);
    VkPipelineLayoutObj pipelineLayout(&test_device);
    VkRenderPass renderpass;
    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    VkSubpassDescription subpass = {};
    rp_info.pSubpasses = &subpass;
    rp_info.subpassCount = 1;
    vk::CreateRenderPass(test_device.handle(), &rp_info, nullptr, &renderpass);
    VkPipelineObj pipeline(&test_device);
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    pipeline.AddShader(&vs);
    pipeline.CreateVKPipeline(pipelineLayout.handle(), renderpass);
    m_renderPassBeginInfo.renderPass = renderpass;
    VkFramebuffer fb;
    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, renderpass, 0, nullptr, 256, 256, 1};
    vk::CreateFramebuffer(test_device.handle(), &fbci, nullptr, &fb);
    m_renderPassBeginInfo.framebuffer = fb;
    m_renderPassBeginInfo.renderPass = renderpass;
    commandBuffer.begin();
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(commandBuffer.handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(commandBuffer.handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
    commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
    if (!tf_properties.transformFeedbackDraw) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287");
    fpvkCmdDrawIndirectByteCountEXT(commandBuffer.handle(), 1, 0, counter_buffer2.handle(), 0, 0, 1);
    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(test_device.handle(), renderpass, nullptr);
    vk::DestroyFramebuffer(test_device.handle(), fb, nullptr);
}

TEST_F(VkLayerTest, DrawIndirectCountKHR) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirectCountKHR");

    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdDrawIndirectCountKHR =
        (PFN_vkCmdDrawIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndirectCountKHR");

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vk_testing::Buffer draw_buffer;
    draw_buffer.init_no_mem(*m_device, buffer_create_info);
    ASSERT_TRUE(draw_buffer.initialized());

    VkDeviceSize count_buffer_size = 128;
    VkBufferCreateInfo count_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    count_buffer_create_info.size = count_buffer_size;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj count_buffer;
    count_buffer.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-buffer-02708");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                              sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    draw_buffer.bind_memory(*m_device, 0, 0);

    vk_testing::Buffer count_buffer_unbound;
    count_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(count_buffer_unbound.initialized());

    VkBufferObj count_buffer_wrong;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    count_buffer_wrong.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02714");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                              sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " VUID-vkCmdDrawIndirectCount-countBuffer-02715");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_wrong.handle(), 0, 1,
                              sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-offset-02710");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 1, count_buffer.handle(), 0, 1,
                              sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBufferOffset-02716");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 1, 1,
                              sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBufferOffset-04129");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), count_buffer_size, 1,
                              sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-stride-03110");
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DrawIndexedIndirectCountKHR) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndexedIndirectCountKHR");

    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdDrawIndexedIndirectCountKHR =
        (PFN_vkCmdDrawIndexedIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndexedIndirectCountKHR");

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj draw_buffer;
    draw_buffer.init(*m_device, buffer_create_info);

    VkDeviceSize count_buffer_size = 128;
    VkBufferCreateInfo count_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    count_buffer_create_info.size = count_buffer_size;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj count_buffer;
    count_buffer.init(*m_device, count_buffer_create_info);

    VkBufferCreateInfo index_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    index_buffer_create_info.size = sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBufferObj index_buffer;
    index_buffer.init(*m_device, index_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02701");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    vk_testing::Buffer draw_buffer_unbound;
    draw_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(draw_buffer_unbound.initialized());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer_unbound.handle(), 0, count_buffer.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk_testing::Buffer count_buffer_unbound;
    count_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(count_buffer_unbound.initialized());

    VkBufferObj count_buffer_wrong;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    count_buffer_wrong.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_wrong.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-offset-02710");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 1, count_buffer.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 1, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-04129");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), count_buffer_size,
                                     1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142");
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DrawIndirectCountFeature) {
    TEST_DESCRIPTION("Test covered valid usage for the 1.2 drawIndirectCount feature");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkBufferObj indirect_buffer;
    indirect_buffer.init(*m_device, sizeof(VkDrawIndirectCommand), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj indexed_indirect_buffer;
    indexed_indirect_buffer.init(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj count_buffer;
    count_buffer.init(*m_device, sizeof(uint32_t), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj index_buffer;
    index_buffer.init(*m_device, sizeof(uint32_t), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Make calls to valid commands but without the drawIndirectCount feature set
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-04445");
    vk::CmdDrawIndirectCount(m_commandBuffer->handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-04445");
    vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ExclusiveScissorNV) {
    TEST_DESCRIPTION("Test VK_NV_scissor_exclusive with multiViewport disabled.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables exclusive scissor but disables multiViewport
    auto exclusive_scissor_features = LvlInitStruct<VkPhysicalDeviceExclusiveScissorFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&exclusive_scissor_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    features2.features.multiViewport = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (m_device->phy().properties().limits.maxViewports) {
        printf("%s Device doesn't support the necessary number of viewports, skipping test.\n", kSkipPrefix);
        return;
    }

    // Based on PSOViewportStateTests
    {
        VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
        VkViewport viewports[] = {viewport, viewport};
        VkRect2D scissor = {{0, 0}, {64, 64}};
        VkRect2D scissors[100] = {scissor, scissor};

        using std::vector;
        struct TestCase {
            uint32_t viewport_count;
            VkViewport *viewports;
            uint32_t scissor_count;
            VkRect2D *scissors;
            uint32_t exclusive_scissor_count;
            VkRect2D *exclusive_scissors;

            vector<std::string> vuids;
        };

        vector<TestCase> test_cases = {
            {1,
             viewports,
             1,
             scissors,
             2,
             scissors,
             {"VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
              "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029"}},
            {1,
             viewports,
             1,
             scissors,
             100,
             scissors,
             {"VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
              "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02028",
              "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029"}},
            {1, viewports, 1, scissors, 1, nullptr, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04056"}},
        };

        for (const auto &test_case : test_cases) {
            VkPipelineViewportExclusiveScissorStateCreateInfoNV exc =
                LvlInitStruct<VkPipelineViewportExclusiveScissorStateCreateInfoNV>();

            const auto break_vp = [&test_case, &exc](CreatePipelineHelper &helper) {
                helper.vp_state_ci_.viewportCount = test_case.viewport_count;
                helper.vp_state_ci_.pViewports = test_case.viewports;
                helper.vp_state_ci_.scissorCount = test_case.scissor_count;
                helper.vp_state_ci_.pScissors = test_case.scissors;
                helper.vp_state_ci_.pNext = &exc;

                exc.exclusiveScissorCount = test_case.exclusive_scissor_count;
                exc.pExclusiveScissors = test_case.exclusive_scissors;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
        }
    }

    // Based on SetDynScissorParamTests
    {
        auto vkCmdSetExclusiveScissorNV =
            (PFN_vkCmdSetExclusiveScissorNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetExclusiveScissorNV");

        const VkRect2D scissor = {{0, 0}, {16, 16}};
        const VkRect2D scissors[] = {scissor, scissor};

        m_commandBuffer->begin();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 1, 1, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "vkCmdSetExclusiveScissorNV: parameter exclusiveScissorCount must be greater than 0");
        vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 0, nullptr);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036");
        vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 2, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "vkCmdSetExclusiveScissorNV: parameter exclusiveScissorCount must be greater than 0");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 1, 0, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036");
        vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 1, 2, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "vkCmdSetExclusiveScissorNV: required parameter pExclusiveScissors specified as NULL");
        vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 1, nullptr);
        m_errorMonitor->VerifyFound();

        struct TestCase {
            VkRect2D scissor;
            std::string vuid;
        };

        std::vector<TestCase> test_cases = {
            {{{-1, 0}, {16, 16}}, "VUID-vkCmdSetExclusiveScissorNV-x-02037"},
            {{{0, -1}, {16, 16}}, "VUID-vkCmdSetExclusiveScissorNV-x-02037"},
            {{{1, 0}, {INT32_MAX, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{INT32_MAX, 0}, {1, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{0, 0}, {uint32_t{INT32_MAX} + 1, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{0, 1}, {16, INT32_MAX}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"},
            {{{0, INT32_MAX}, {16, 1}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"},
            {{{0, 0}, {16, uint32_t{INT32_MAX} + 1}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"}};

        for (const auto &test_case : test_cases) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.vuid);
            vkCmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 1, &test_case.scissor);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->end();
    }
}

TEST_F(VkLayerTest, MeshShaderNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_NV_MESH_SHADER_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mesh_shader_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    features2.features.multiDrawIndirect = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char vertShaderText[] = R"glsl(
        #version 450
        vec2 vertices[3];
        void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_PointSize = 1.0f;
        }
    )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
              gl_MeshVerticesNV[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
              gl_PrimitiveIndicesNV[0] = 0;
              gl_PrimitiveIndicesNV[1] = 1;
              gl_PrimitiveIndicesNV[2] = 2;
              gl_PrimitiveCountNV = 1;
        }
    )glsl";

    VkShaderObj vs(this, vertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Test pipeline creation
    {
        // can't mix mesh with vertex
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo(), ms.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02095"}));

        // vertex or mesh must be present
        const auto break_vp2 = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {fs.GetStageCreateInfo()}; };
        CreatePipelineHelper::OneshotTest(*this, break_vp2, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-stage-02096"}));

        // vertexinput and inputassembly must be valid when vertex stage is present
        const auto break_vp3 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pVertexInputState = nullptr;
            helper.gp_ci_.pInputAssemblyState = nullptr;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp3, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02097",
                                                               "VUID-VkGraphicsPipelineCreateInfo-pStages-02098"}));
    }

    PFN_vkCmdDrawMeshTasksIndirectNV vkCmdDrawMeshTasksIndirectNV =
        (PFN_vkCmdDrawMeshTasksIndirectNV)vk::GetInstanceProcAddr(instance(), "vkCmdDrawMeshTasksIndirectNV");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBuffer buffer;
    VkResult result = vk::CreateBuffer(m_device->device(), &buffer_create_info, nullptr, &buffer);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718");
    vkCmdDrawMeshTasksIndirectNV(m_commandBuffer->handle(), buffer, 0, 2, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    vk::DestroyBuffer(m_device->device(), buffer, 0);
}

TEST_F(VkLayerTest, MeshShaderDisabledNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader VUs with NV_mesh_shader disabled.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mesh_shader_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        printf("%s Mesh shader feature not supported\n", kSkipPrefix);
        return;
    }

    mesh_shader_features.meshShader = VK_FALSE;
    mesh_shader_features.taskShader = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-04095");
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-04096");
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-04095");
    vk::CmdResetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-04096");
    vk::CmdResetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-04095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstStageMask-04095");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
                      VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-04096");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstStageMask-04096");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
                      VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-04095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstStageMask-04095");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-04096");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstStageMask-04096");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();

    // Signal the semaphore so the next test can wait on it.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.pWaitDstStageMask = &stage_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-04095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-04096");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    static const char task_src[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 32) in;

        taskNV out Task {
          uint baseID;
        } OUT;

        void main() {
            OUT.baseID = 1;
        }
    )glsl";

    static const char mesh_src[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;

        taskNV in Task {
          uint baseID;
        } IN;

        void main() {
        }
    )glsl";

    VkShaderObj task_shader(this, task_src, VK_SHADER_STAGE_TASK_BIT_NV);
    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_NV);

    // mesh and task shaders not supported
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                                                           "VUID-VkPipelineShaderStageCreateInfo-stage-02092"}));

    vk::DestroyEvent(m_device->device(), event, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

TEST_F(VkLayerTest, ViewportWScalingNV) {
    TEST_DESCRIPTION("Verify VK_NV_clip_space_w_scaling");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    if (!device_features.multiViewport) {
        printf("%s VkPhysicalDeviceFeatures::multiViewport is not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdSetViewportWScalingNV =
        reinterpret_cast<PFN_vkCmdSetViewportWScalingNV>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportWScalingNV"));

    const char vs_src[] = R"glsl(
        #version 450
        const vec2 positions[] = { vec2(-1.0f,  1.0f),
                                   vec2( 1.0f,  1.0f),
                                   vec2(-1.0f, -1.0f),
                                   vec2( 1.0f, -1.0f) };
        out gl_PerVertex {
            vec4 gl_Position;
        };

        void main() {
            gl_Position = vec4(positions[gl_VertexIndex % 4], 0.0f, 1.0f);
        }
    )glsl";

    const char fs_src[] = R"glsl(
        #version 450
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        }
    )glsl";

    const std::vector<VkViewport> vp = {
        {0.0f, 0.0f, 64.0f, 64.0f}, {0.0f, 0.0f, 64.0f, 64.0f}, {0.0f, 0.0f, 64.0f, 64.0f}, {0.0f, 0.0f, 64.0f, 64.0f}};
    const std::vector<VkRect2D> sc = {{{0, 0}, {32, 32}}, {{32, 0}, {32, 32}}, {{0, 32}, {32, 32}}, {{32, 32}, {32, 32}}};
    const std::vector<VkViewportWScalingNV> scale = {{-0.2f, -0.2f}, {0.2f, -0.2f}, {-0.2f, 0.2f}, {0.2f, 0.2f}};

    const uint32_t vp_count = static_cast<uint32_t>(vp.size());

    VkPipelineViewportWScalingStateCreateInfoNV vpsi = LvlInitStruct<VkPipelineViewportWScalingStateCreateInfoNV>();
    vpsi.viewportWScalingEnable = VK_TRUE;
    vpsi.viewportCount = vp_count;
    vpsi.pViewportWScalings = scale.data();

    VkPipelineViewportStateCreateInfo vpci = LvlInitStruct<VkPipelineViewportStateCreateInfo>(&vpsi);
    vpci.viewportCount = vp_count;
    vpci.pViewports = vp.data();
    vpci.scissorCount = vp_count;
    vpci.pScissors = sc.data();

    const auto set_vpci = [&vpci](CreatePipelineHelper &helper) { helper.vp_state_ci_ = vpci; };

    // Make sure no errors show up when creating the pipeline with w-scaling enabled
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit, vector<std::string>(), true);

    // Create pipeline with w-scaling enabled but without a valid scaling array
    vpsi.pViewportWScalings = nullptr;
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit,
                                      vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01715"}));

    vpsi.pViewportWScalings = scale.data();

    // Create pipeline with w-scaling enabled but without matching viewport counts
    vpsi.viewportCount = 1;
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineViewportStateCreateInfo-viewportWScalingEnable-01726"}));

    const VkPipelineLayoutObj pl(m_device);

    VkShaderObj vs(this, vs_src, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.SetViewport(vp);
    pipe.SetScissor(sc);
    pipe.CreateVKPipeline(pl.handle(), renderPass());

    VkPipelineObj pipeDynWScale(m_device);
    pipeDynWScale.AddDefaultColorAttachment();
    pipeDynWScale.AddShader(&vs);
    pipeDynWScale.AddShader(&fs);
    pipeDynWScale.SetViewport(vp);
    pipeDynWScale.SetScissor(sc);
    pipeDynWScale.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
    pipeDynWScale.CreateVKPipeline(pl.handle(), renderPass());

    m_commandBuffer->begin();

    // Bind pipeline without dynamic w scaling enabled
    m_errorMonitor->ExpectSuccess();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_errorMonitor->VerifyNotFound();

    // Bind pipeline that has dynamic w-scaling enabled
    m_errorMonitor->ExpectSuccess();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeDynWScale.handle());
    m_errorMonitor->VerifyNotFound();

    const auto max_vps = m_device->props.limits.maxViewports;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWScalingNV-firstViewport-01324");
    vkCmdSetViewportWScalingNV(m_commandBuffer->handle(), 1, max_vps, scale.data());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vkCmdSetViewportWScalingNV(m_commandBuffer->handle(), 0, vp_count, scale.data());
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CreateSamplerYcbcrConversionEnable) {
    TEST_DESCRIPTION("Checks samplerYcbcrConversion is enabled before calling vkCreateSamplerYcbcrConversion");

    // Enable Sampler YCbCr Conversion req'd extensions
    // Only need revision 1 of vkGetPhysicalDeviceProperties2 and this allows more device capable for testing
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires Sampler YCbCr Conversion extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    // Explictly not enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    ycbcr_features.samplerYcbcrConversion = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ycbcr_features));

    PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionFunction =
        (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
    if (vkCreateSamplerYcbcrConversionFunction == nullptr) {
        printf("%s did not find vkCreateSamplerYcbcrConversionKHR function pointer;  Skipping.\n", kSkipPrefix);
        return;
    }

    // Create Ycbcr conversion
    VkSamplerYcbcrConversion conversions;
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
                                                            NULL,
                                                            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
                                                            VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_FILTER_NEAREST,
                                                            false};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateSamplerYcbcrConversion-None-01648");
    vkCreateSamplerYcbcrConversionFunction(m_device->handle(), &ycbcr_create_info, nullptr, &conversions);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TransformFeedbackFeatureEnabled) {
    TEST_DESCRIPTION("VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback must be enabled");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME)) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);

    {
        PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
            (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
        ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

        auto tf_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
        auto pd_features = LvlInitStruct<VkPhysicalDeviceFeatures2>(&tf_features);
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &pd_features);

        if (!tf_features.transformFeedback) {
            printf("%s transformFeedback not supported; skipped.\n", kSkipPrefix);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        auto vkCmdBindTransformFeedbackBuffersEXT = (PFN_vkCmdBindTransformFeedbackBuffersEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdBindTransformFeedbackBuffersEXT");
        ASSERT_TRUE(vkCmdBindTransformFeedbackBuffersEXT != nullptr);

        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 4;
        VkBufferObj buffer;
        buffer.init(*m_device, info);
        VkDeviceSize offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-transformFeedback-02355");
        vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        auto vkCmdBeginTransformFeedbackEXT =
            (PFN_vkCmdBeginTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginTransformFeedbackEXT");
        ASSERT_TRUE(vkCmdBeginTransformFeedbackEXT != nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-transformFeedback-02366");
        vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        auto vkCmdEndTransformFeedbackEXT =
            (PFN_vkCmdEndTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndTransformFeedbackEXT");
        ASSERT_TRUE(vkCmdEndTransformFeedbackEXT != nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-transformFeedback-02374");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndTransformFeedbackEXT-None-02375");
        vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, TransformFeedbackCmdBindTransformFeedbackBuffersEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdBindTransformFeedbackBuffersEXT");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "Test temporarily disabled on S10 device";
    }

    {
        auto tf_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
        auto pd_features = GetPhysicalDeviceFeatures2(tf_features);

        if (!tf_features.transformFeedback) {
            GTEST_SKIP() << "transformFeedback not supported; skipped.";
        }

        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features));
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdBindTransformFeedbackBuffersEXT =
        (PFN_vkCmdBindTransformFeedbackBuffersEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBindTransformFeedbackBuffersEXT");
    ASSERT_TRUE(vkCmdBindTransformFeedbackBuffersEXT != nullptr);

    {
        auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        GetPhysicalDeviceProperties2(tf_properties);

        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 8;
        VkBufferObj const buffer_obj(*m_device, info);

        // Request a firstBinding that is too large.
        {
            auto const firstBinding = tf_properties.maxTransformFeedbackBuffers;
            VkDeviceSize const offsets[1]{};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), firstBinding, 1, &buffer_obj.handle(), offsets,
                                                 nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request too many bindings.
        if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
            auto const bindingCount = tf_properties.maxTransformFeedbackBuffers + 1;
            std::vector<VkBuffer> buffers(bindingCount, buffer_obj.handle());

            std::vector<VkDeviceSize> offsets(bindingCount);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, bindingCount, buffers.data(), offsets.data(),
                                                 nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request a size that is larger than the maximum size.
        if (tf_properties.maxTransformFeedbackBufferSize < std::numeric_limits<VkDeviceSize>::max()) {
            VkDeviceSize const offsets[1]{};
            VkDeviceSize const sizes[1]{tf_properties.maxTransformFeedbackBufferSize + 1};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 8;
        VkBufferObj const buffer_obj(*m_device, info);

        // Request an offset that is too large.
        {
            VkDeviceSize const offsets[1]{info.size + 4};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02358");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request an offset that is not a multiple of 4.
        {
            VkDeviceSize const offsets[1]{1};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request a size that is larger than the buffer's size.
        {
            VkDeviceSize const offsets[1]{};
            VkDeviceSize const sizes[1]{info.size + 1};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSizes-02362");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }

        // Request an offset and size whose sum is larger than the buffer's size.
        {
            VkDeviceSize const offsets[1]{4};
            VkDeviceSize const sizes[1]{info.size - 3};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02363");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }

        // Bind while transform feedback is active.
        if (!IsDriver(VK_DRIVER_ID_MESA_RADV)) {
            auto vkCmdBeginTransformFeedbackEXT =
                (PFN_vkCmdBeginTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginTransformFeedbackEXT");
            ASSERT_TRUE(vkCmdBeginTransformFeedbackEXT != nullptr);
            vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

            VkDeviceSize const offsets[1]{};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-None-02365");
            vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();

            auto vkCmdEndTransformFeedbackEXT =
                (PFN_vkCmdEndTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndTransformFeedbackEXT");
            ASSERT_TRUE(vkCmdEndTransformFeedbackEXT != nullptr);
            vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        }
    }

    // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT.
    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        // info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 4;
        m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-usage-parameter");
        VkBufferObj const buffer_obj(*m_device, info);

        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02360");
        vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // Don't bind memory.
    {
        VkBuffer buffer{};
        {
            auto vkCreateBuffer = (PFN_vkCreateBuffer)vk::GetDeviceProcAddr(m_device->device(), "vkCreateBuffer");
            ASSERT_TRUE(vkCreateBuffer != nullptr);

            auto info = LvlInitStruct<VkBufferCreateInfo>();
            info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
            info.size = 4;
            vkCreateBuffer(m_device->device(), &info, nullptr, &buffer);
        }

        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02364");
        vkCmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer, offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, TransformFeedbackCmdBeginTransformFeedbackEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdBeginTransformFeedbackEXT");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "Test temporarily disabled on S10 device";
    }

    {
        auto tf_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
        auto pd_features = GetPhysicalDeviceFeatures2(tf_features);

        if (!tf_features.transformFeedback) {
            GTEST_SKIP() << "transformFeedback not supported; skipped.";
        }

        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features));
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdBeginTransformFeedbackEXT =
        (PFN_vkCmdBeginTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginTransformFeedbackEXT");
    ASSERT_TRUE(vkCmdBeginTransformFeedbackEXT != nullptr);

    {
        auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        GetPhysicalDeviceProperties2(tf_properties);

        // Request a firstCounterBuffer that is too large.
        {
            auto const firstCounterBuffer = tf_properties.maxTransformFeedbackBuffers;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369");
            vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), firstCounterBuffer, 1, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request too many buffers.
        if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
            auto const counterBufferCount = tf_properties.maxTransformFeedbackBuffers + 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369");
            vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, counterBufferCount, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }
    }

    // Request an out-of-bounds location.
    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
        info.size = 4;
        VkBufferObj const buffer_obj(*m_device, info);

        VkDeviceSize const offsets[1]{1};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-02370");
        vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets);
        m_errorMonitor->VerifyFound();
    }

    // Request specific offsets without specifying buffers.
    {
        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffer-02371");
        vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, offsets);
        m_errorMonitor->VerifyFound();
    }

    // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT.
    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        // info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
        info.size = 4;
        m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-usage-parameter");
        VkBufferObj const buffer_obj(*m_device, info);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372");
        vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    // Begin while transform feedback is active.
    if (!IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-None-02367");
        vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();

        auto vkCmdEndTransformFeedbackEXT =
            (PFN_vkCmdEndTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndTransformFeedbackEXT");
        ASSERT_TRUE(vkCmdEndTransformFeedbackEXT != nullptr);

        vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    }
}

TEST_F(VkLayerTest, TransformFeedbackCmdEndTransformFeedbackEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdEndTransformFeedbackEXT");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "Test temporarily disabled on S10 device";
    }

    {
        auto tf_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
        auto pd_features = GetPhysicalDeviceFeatures2(tf_features);

        if (!tf_features.transformFeedback) {
            GTEST_SKIP() << "transformFeedback not supported; skipped.";
        }

        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features));
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdEndTransformFeedbackEXT =
        (PFN_vkCmdEndTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndTransformFeedbackEXT");
    ASSERT_TRUE(vkCmdEndTransformFeedbackEXT != nullptr);

    if (!IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        {
            // Activate transform feedback.
            auto vkCmdBeginTransformFeedbackEXT =
                (PFN_vkCmdBeginTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginTransformFeedbackEXT");
            ASSERT_TRUE(vkCmdBeginTransformFeedbackEXT != nullptr);
            vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

            {
                auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
                GetPhysicalDeviceProperties2(tf_properties);

                // Request a firstCounterBuffer that is too large.
                {
                    auto const firstCounterBuffer = tf_properties.maxTransformFeedbackBuffers;

                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376");
                    m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377");
                    vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), firstCounterBuffer, 1, nullptr, nullptr);
                    m_errorMonitor->VerifyFound();
                }

                // Request too many buffers.
                if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
                    auto const counterBufferCount = tf_properties.maxTransformFeedbackBuffers + 1;

                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377");
                    vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, counterBufferCount, nullptr, nullptr);
                    m_errorMonitor->VerifyFound();
                }
            }

            // Request an out-of-bounds location.
            {
                auto info = LvlInitStruct<VkBufferCreateInfo>();
                info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
                info.size = 4;
                VkBufferObj const buffer_obj(*m_device, info);

                VkDeviceSize const offsets[1]{1};

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-02378");
                vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets);
                m_errorMonitor->VerifyFound();
            }

            // Request specific offsets without specifying buffers.
            {
                VkDeviceSize const offsets[1]{};

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffer-02379");
                vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, offsets);
                m_errorMonitor->VerifyFound();
            }

            // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT.
            {
                auto info = LvlInitStruct<VkBufferCreateInfo>();
                // info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
                info.size = 4;
                m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-usage-parameter");
                VkBufferObj const buffer_obj(*m_device, info);

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380");
                vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), nullptr);
                m_errorMonitor->VerifyFound();
            }
        }

        // End while transform feedback is inactive.
        {
            vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-None-02375");
            vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, InvalidUnprotectedCommands) {
    TEST_DESCRIPTION("Test making commands in unprotected command buffers that can't be used");

    // protect memory added in VK 1.1
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto protected_memory_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&protected_memory_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        printf("%s protectedMemory feature not supported, skipped.\n", kSkipPrefix);
        return;
    };

    // Turns m_commandBuffer into a protected command buffer
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    VkBufferObj indirect_buffer;
    indirect_buffer.init(*m_device, sizeof(VkDrawIndirectCommand), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj indexed_indirect_buffer;
    indexed_indirect_buffer.init(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj index_buffer;
    index_buffer.init(*m_device, sizeof(uint32_t), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-commandBuffer-02711");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, 1,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();

    // Query should be outside renderpass
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-commandBuffer-01885");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-commandBuffer-01886");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndQuery-None-01923");
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, InvalidMixingProtectedResources) {
    TEST_DESCRIPTION("Test where there is mixing of protectedMemory backed resource in command buffers");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        printf("%s CreateImageView calls crash ShieldTV, skipped for this platform.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto protected_memory_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&protected_memory_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        printf("%s protectedMemory feature not supported, skipped.\n", kSkipPrefix);
        return;
    };

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties =
        LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&protected_memory_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    // Turns m_commandBuffer into a unprotected command buffer without passing in a VkCommandPoolCreateFlags
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkCommandPoolObj protectedCommandPool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_PROTECTED_BIT);
    VkCommandBufferObj protectedCommandBuffer(m_device, &protectedCommandPool);

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    // Create actual protected and unprotected buffers
    VkBuffer buffer_protected = VK_NULL_HANDLE;
    VkBuffer buffer_unprotected = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_protected);
    buffer_create_info.flags = 0;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_unprotected);

    // Create actual protected and unprotected images
    const VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_protected(m_device);
    VkImageObj image_unprotected(m_device);
    VkImageObj image_protected_descriptor(m_device);
    VkImageObj image_unprotected_descriptor(m_device);
    VkImageView image_views[2];
    VkImageView image_views_descriptor[2];
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = image_format;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    image_protected.init_no_mem(*m_device, image_create_info);
    image_protected_descriptor.init_no_mem(*m_device, image_create_info);

    image_create_info.flags = 0;
    image_unprotected.init_no_mem(*m_device, image_create_info);
    image_unprotected_descriptor.init_no_mem(*m_device, image_create_info);

    // Create protected and unproteced memory
    VkDeviceMemory memory_protected = VK_NULL_HANDLE;
    VkDeviceMemory memory_unprotected = VK_NULL_HANDLE;

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = 0;

    // set allocationSize to buffer as it will be larger than the image, but query image to avoid BP warning
    VkMemoryRequirements mem_reqs_protected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_protected);
    vk::GetBufferMemoryRequirements(device(), buffer_protected, &mem_reqs_protected);
    VkMemoryRequirements mem_reqs_unprotected;
    vk::GetImageMemoryRequirements(device(), image_unprotected.handle(), &mem_reqs_unprotected);
    vk::GetBufferMemoryRequirements(device(), buffer_unprotected, &mem_reqs_unprotected);

    // Get memory index for a protected and unprotected memory
    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
    uint32_t memory_type_protected = phys_mem_props.memoryTypeCount + 1;
    uint32_t memory_type_unprotected = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        if ((mem_reqs_unprotected.memoryTypeBits & (1 << i)) &&
            ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memory_type_unprotected = i;
        }
        // Check just protected bit is in type at all
        if ((mem_reqs_protected.memoryTypeBits & (1 << i)) &&
            ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0)) {
            memory_type_protected = i;
        }
    }
    if ((memory_type_protected >= phys_mem_props.memoryTypeCount) || (memory_type_unprotected >= phys_mem_props.memoryTypeCount)) {
        printf("%s No valid memory type index could be found; skipped.\n", kSkipPrefix);
        vk::DestroyBuffer(device(), buffer_protected, nullptr);
        vk::DestroyBuffer(device(), buffer_unprotected, nullptr);
        return;
    }

    alloc_info.memoryTypeIndex = memory_type_protected;
    alloc_info.allocationSize = mem_reqs_protected.size;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory_protected);

    alloc_info.allocationSize = mem_reqs_unprotected.size;
    alloc_info.memoryTypeIndex = memory_type_unprotected;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory_unprotected);

    vk::BindBufferMemory(device(), buffer_protected, memory_protected, 0);
    vk::BindBufferMemory(device(), buffer_unprotected, memory_unprotected, 0);
    vk::BindImageMemory(device(), image_protected.handle(), memory_protected, 0);
    vk::BindImageMemory(device(), image_unprotected.handle(), memory_unprotected, 0);
    vk::BindImageMemory(device(), image_protected_descriptor.handle(), memory_protected, 0);
    vk::BindImageMemory(device(), image_unprotected_descriptor.handle(), memory_unprotected, 0);

    // Change layout once memory is bound
    image_protected.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_protected_descriptor.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_unprotected.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_unprotected_descriptor.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    // need memory bound at image view creation time
    image_views[0] = image_protected.targetView(image_format);
    image_views[1] = image_unprotected.targetView(image_format);
    image_views_descriptor[0] = image_protected_descriptor.targetView(image_format);
    image_views_descriptor[1] = image_unprotected_descriptor.targetView(image_format);

    // A renderpass and framebuffer that contains a protected and unprotected image view
    VkAttachmentDescription attachments[2] = {
        {0, image_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, image_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference references[2] = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                           {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 2, references, nullptr, nullptr, 0, nullptr};
    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attachments, 1, &subpass, 1, &dependency};
    VkRenderPass render_pass;
    ASSERT_VK_SUCCESS(vk::CreateRenderPass(device(), &render_pass_create_info, nullptr, &render_pass));
    VkFramebufferCreateInfo framebuffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, render_pass, 2, image_views, 8, 8, 1};
    VkFramebuffer framebuffer;
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &framebuffer_create_info, nullptr, &framebuffer));

    // Various structs used for commands
    VkImageSubresourceLayers image_subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageBlit blit_region = {};
    blit_region.srcSubresource = image_subresource;
    blit_region.dstSubresource = image_subresource;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {8, 8, 1};
    blit_region.dstOffsets[0] = {0, 8, 0};
    blit_region.dstOffsets[1] = {8, 8, 1};
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkBufferCopy buffer_copy = {0, 0, 64};
    VkBufferImageCopy buffer_image_copy = {};
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = image_subresource;
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {1, 1, 1};
    buffer_image_copy.bufferOffset = 0;
    VkImageCopy image_copy = {};
    image_copy.srcSubresource = image_subresource;
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = image_subresource;
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {1, 1, 1};
    uint32_t update_data[4] = {0, 0, 0, 0};
    VkRect2D render_area = {{0, 0}, {8, 8}};
    VkRenderPassBeginInfo render_pass_begin = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, render_pass, framebuffer, render_area, 0, nullptr};
    VkClearAttachment clear_attachments[2] = {{VK_IMAGE_ASPECT_COLOR_BIT, 0, {m_clear_color}},
                                              {VK_IMAGE_ASPECT_COLOR_BIT, 1, {m_clear_color}}};
    VkClearRect clear_rect[2] = {{render_area, 0, 1}, {render_area, 0, 1}};

    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { int x; int y; } bar;
        layout(set=0, binding=1, rgba8) uniform image2D si1;
        layout(location=0) out vec4 x;
        void main(){
           x = vec4(bar.y);
           imageStore(si1, ivec2(0), vec4(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this, 2u);
    g_pipe.InitInfo();
    g_pipe.gp_ci_.renderPass = render_pass;
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    VkSampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkResult err = vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler);
    ASSERT_VK_SUCCESS(err);

    // Use protected resources in unprotected command buffer
    g_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_protected, 0, 1024);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_views_descriptor[0], sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    // will get undefined values, but not invalid if protectedNoFault is supported
    // Will still create an empty command buffer to test submit VUs if protectedNoFault is supported
    if (!protected_memory_properties.protectedNoFault) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-commandBuffer-01834");
        vk::CmdBlitImage(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_unprotected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-commandBuffer-01835");
        vk::CmdBlitImage(m_commandBuffer->handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-commandBuffer-01805");
        vk::CmdClearColorImage(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1,
                               &subresource_range);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-commandBuffer-01822");
        vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_protected, buffer_unprotected, 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-commandBuffer-01823");
        vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_unprotected, buffer_protected, 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-01828");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_protected, image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-01829");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_unprotected, image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-01825");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_unprotected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-01826");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_unprotected,
                                 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_protected,
                                 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commandBuffer-01811");
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer_protected, 0, 4, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-commandBuffer-01813");
        vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer_protected, 0, 4, (void *)update_data);
        m_errorMonitor->VerifyFound();

        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-commandBuffer-02504");
        vk::CmdClearAttachments(m_commandBuffer->handle(), 2, clear_attachments, 2, clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &g_pipe.descriptor_set_->set_, 0, nullptr);
        VkDeviceSize offset = 0;
        vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer_protected, &offset);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer_protected, 0, VK_INDEX_TYPE_UINT16);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // color attachment
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // buffer descriptorSet
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // image descriptorSet
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // vertex
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // index

        vk::CmdDrawIndexed(m_commandBuffer->handle(), 1, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRenderPass(m_commandBuffer->handle());
    }
    m_commandBuffer->end();

    // Use unprotected resources in protected command buffer
    g_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_unprotected, 0, 1024);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_views_descriptor[1], sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    protectedCommandBuffer.begin();
    if (!protected_memory_properties.protectedNoFault) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-commandBuffer-01836");
        vk::CmdBlitImage(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-commandBuffer-01806");
        vk::CmdClearColorImage(protectedCommandBuffer.handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color,
                               1, &subresource_range);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-commandBuffer-01824");
        vk::CmdCopyBuffer(protectedCommandBuffer.handle(), buffer_protected, buffer_unprotected, 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-01830");
        vk::CmdCopyBufferToImage(protectedCommandBuffer.handle(), buffer_protected, image_unprotected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-01827");
        vk::CmdCopyImage(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833");
        vk::CmdCopyImageToBuffer(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_unprotected, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commandBuffer-01812");
        vk::CmdFillBuffer(protectedCommandBuffer.handle(), buffer_unprotected, 0, 4, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-commandBuffer-01814");
        vk::CmdUpdateBuffer(protectedCommandBuffer.handle(), buffer_unprotected, 0, 4, (void *)update_data);
        m_errorMonitor->VerifyFound();

        vk::CmdBeginRenderPass(protectedCommandBuffer.handle(), &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-commandBuffer-02505");
        vk::CmdClearAttachments(protectedCommandBuffer.handle(), 2, clear_attachments, 2, clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdBindPipeline(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
        vk::CmdBindDescriptorSets(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  g_pipe.pipeline_layout_.handle(), 0, 1, &g_pipe.descriptor_set_->set_, 0, nullptr);
        VkDeviceSize offset = 0;
        vk::CmdBindVertexBuffers(protectedCommandBuffer.handle(), 0, 1, &buffer_unprotected, &offset);
        vk::CmdBindIndexBuffer(protectedCommandBuffer.handle(), buffer_unprotected, 0, VK_INDEX_TYPE_UINT16);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02712");  // color attachment
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02712");  // descriptorSet
        vk::CmdDrawIndexed(protectedCommandBuffer.handle(), 1, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRenderPass(protectedCommandBuffer.handle());
    }
    protectedCommandBuffer.end();

    // Try submitting together to test only 1 error occurs for the corresponding command buffer
    VkCommandBuffer comman_buffers[2] = {m_commandBuffer->handle(), protectedCommandBuffer.handle()};

    VkProtectedSubmitInfo protected_submit_info = LvlInitStruct<VkProtectedSubmitInfo>();
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&protected_submit_info);
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = comman_buffers;

    protected_submit_info.protectedSubmit = VK_TRUE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-queue-06448");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-04148");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    protected_submit_info.protectedSubmit = VK_FALSE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-04120");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(device(), buffer_protected, nullptr);
    vk::DestroyBuffer(device(), buffer_unprotected, nullptr);
    vk::FreeMemory(device(), memory_protected, nullptr);
    vk::FreeMemory(device(), memory_unprotected, nullptr);
    vk::DestroyFramebuffer(device(), framebuffer, nullptr);
    vk::DestroyRenderPass(device(), render_pass, nullptr);
}

TEST_F(VkLayerTest, InvalidStorageAtomicOperation) {
    TEST_DESCRIPTION(
        "If storage view use atomic operation, the view's format MUST support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT or "
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT ");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto atomic_float_features = lvl_init_struct<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&atomic_float_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (atomic_float_features.shaderImageFloat32Atomics == VK_FALSE) {
        printf("%s shaderImageFloat32Atomics not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();
    VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT to
                                                       // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    auto image_ci = VkImageObj::ImageCreateInfo2D(64, 64, 1, 1, image_format, usage, VK_IMAGE_TILING_OPTIMAL);

    if (ImageFormatAndFeaturesSupported(instance(), gpu(), image_ci, VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        printf("%s Cannot make VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    VkFormat buffer_view_format =
        VK_FORMAT_R8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT to
                             // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    if (BufferFormatAndFeaturesSupported(gpu(), buffer_view_format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
        printf("%s Cannot make VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }
    m_errorMonitor->SetUnexpectedError("VUID-VkBufferViewCreateInfo-buffer-00934");
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (!device_features.vertexPipelineStoresAndAtomics || !device_features.fragmentStoresAndAtomics) {
        printf("%s vertexPipelineStoresAndAtomics & fragmentStoresAndAtomics NOT supported. skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView image_view = image.targetView(image_format);

    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);

    VkBufferObj buffer;
    buffer.init(*m_device, 64, 0, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
    bvci.buffer = buffer.handle();
    bvci.format = buffer_view_format;
    bvci.range = VK_WHOLE_SIZE;
    VkBufferView buffer_view;
    vk::CreateBufferView(m_device->device(), &bvci, NULL, &buffer_view);

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=3, r32f) uniform image2D si0;
        layout(set=0, binding=2, r32f) uniform image2D si1[2];
        layout(set = 0, binding = 1, r32f) uniform imageBuffer stb2;
        layout(set = 0, binding = 0, r32f) uniform imageBuffer stb3[2];
        void main() {
              imageAtomicExchange(si0, ivec2(0), 1);
              imageAtomicExchange(si1[0], ivec2(0), 1);
              imageAtomicExchange(si1[1], ivec2(0), 1);
              imageAtomicExchange(stb2, 0, 1);
              imageAtomicExchange(stb3[0], 0, 1);
              imageAtomicExchange(stb3[1], 0, 1);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, image_view, sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 0, 2);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(1, buffer_view);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0, 2);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02691");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02691");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-None-MismatchAtomicBufferFeature");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-None-MismatchAtomicBufferFeature");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    vk::DestroyBufferView(m_device->handle(), buffer_view, nullptr);
    vk::DestroySampler(m_device->handle(), sampler, nullptr);
}

TEST_F(VkLayerTest, DrawWithoutUpdatePushConstants) {
    TEST_DESCRIPTION("Not every bytes in used push constant ranges has been set before Draw ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // push constant range: 0-99
    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo {
           bool b;
           float f2[3];
           vec3 v;
           vec4 v2[2];
           mat3 m;
        } constants;
        void func1( float f ){
           // use the whole v2[1]. byte: 48-63.
           vec2 v2 = constants.v2[1].yz;
        }
        void main(){
            // use only v2[0].z. byte: 40-43.
            func1( constants.v2[0].z);
            // index of m is variable. The all m is used. byte: 64-99.
            for(int i=1;i<2;++i) {
                vec3 v3 = constants.m[i];
            }
        }
    )glsl";

    // push constant range: 0 - 95
    char const *const fsSource = R"glsl(
        #version 450
        struct foo1{
           int i[4];
        }f;
        layout(push_constant, std430) uniform foo {
           float x[2][2][2];
           foo1 s;
           foo1 ss[3];
        } constants;
        void main(){
            // use s. byte: 32-47.
            f = constants.s;
            // use every i[3] in ss. byte: 60-63, 76-79, 92-95.
            for(int i=1;i<2;++i) {
                int ii = constants.ss[i].i[3];
            }
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128};
    VkPushConstantRange push_constant_range_small = {VK_SHADER_STAGE_VERTEX_BIT, 4, 4};

    auto pipeline_layout_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    VkPipelineLayout pipeline_layout;
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ci_ = pipeline_layout_info;
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    pipeline_layout_info.pPushConstantRanges = &push_constant_range_small;
    VkPipelineLayout pipeline_layout_small;
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout_small);

    CreatePipelineHelper g_pipe_small_range(*this);
    g_pipe_small_range.InitInfo();
    g_pipe_small_range.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe_small_range.pipeline_layout_ci_ = pipeline_layout_info;
    g_pipe_small_range.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
    g_pipe_small_range.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-maintenance4-06425");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    const float dummy_values[128] = {};

    // NOTE: these are commented out due to ambiguity around VUID 02698 and push constant lifetimes
    //       See https://gitlab.khronos.org/vulkan/vulkan/-/issues/2602 and
    //       https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2689
    //       for more details.
    // m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-maintenance4-06425");
    // vk::CmdPushConstants(m_commandBuffer->handle(), g_pipe.pipeline_layout_.handle(),
    //                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 96, dummy_values);
    // vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    // m_errorMonitor->VerifyFound();

    // m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-maintenance4-06425");
    // vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_small, VK_SHADER_STAGE_VERTEX_BIT, 4, 4, dummy_values);
    // vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    // m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 32,
                         68, dummy_values);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, VerifyVertextBinding) {
    TEST_DESCRIPTION("Verify if VkPipelineVertexInputStateCreateInfo matches vkCmdBindVertexBuffers");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(32, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    // CmdBindVertexBuffers only has binding:1. It causes 04007 & 04008 desired fail.
    VkVertexInputBindingDescription vtx_binding_des[3] = {
        {0, 64, VK_VERTEX_INPUT_RATE_VERTEX}, {1, 64, VK_VERTEX_INPUT_RATE_VERTEX}, {2, 64, VK_VERTEX_INPUT_RATE_VERTEX}};

    // CmdBindVertexBuffers only has binding:1. It causes twice 02721 desired fail.
    // Plus, binding:1's offset is wrong. It causes 02721 desired fail, again.
    VkVertexInputAttributeDescription vtx_attri_des[3] = {{0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 10},
                                                          {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 10},
                                                          {2, 2, VK_FORMAT_R32G32B32A32_SFLOAT, 10}};
    pipe.vi_ci_.vertexBindingDescriptionCount = 3;
    pipe.vi_ci_.pVertexBindingDescriptions = vtx_binding_des;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.vi_ci_.pVertexAttributeDescriptions = vtx_attri_des;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vtx_buf.handle(), &offset);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-04008");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-04007");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, VerifyDynamicStateSettingCommands) {
    TEST_DESCRIPTION("Verify if pipeline doesn't setup dynamic state, but set dynamic commands");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();

    std::vector<VkDynamicState> dyn_states = {VK_DYNAMIC_STATE_VIEWPORT};

    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
    dyn_state_ci.pDynamicStates = dyn_states.data();
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdSetLineWidth(m_commandBuffer->handle(), 1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02859");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, VerifyFilterCubicSamplerInCmdDraw) {
    TEST_DESCRIPTION("Verify if sampler is filter cubic, image view needs to support it.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &format_props);
    if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT) == 0) {
        GTEST_SKIP() << "SAMPLED_IMAGE_FILTER_CUBIC_BIT for format is not supported.";
    }

    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;

    auto imageview_format_info = LvlInitStruct<VkPhysicalDeviceImageViewImageFormatInfoEXT>();
    imageview_format_info.imageViewType = imageViewType;
    auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&imageview_format_info);
    image_format_info.type = image_ci.imageType;
    image_format_info.format = image_ci.format;
    image_format_info.tiling = image_ci.tiling;
    image_format_info.usage = image_ci.usage;
    image_format_info.flags = image_ci.flags;

    auto filter_cubic_props = LvlInitStruct<VkFilterCubicImageViewImageFormatPropertiesEXT>();
    auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>(&filter_cubic_props);

    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_format_info, &image_format_properties);

    if (filter_cubic_props.filterCubic || filter_cubic_props.filterCubicMinmax) {
        GTEST_SKIP() << "Image and ImageView supports filter cubic ; skipped.";
    }

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView imageView = image.targetView(format, imageViewType);

    auto sampler_ci = LvlInitStruct<VkSamplerCreateInfo>();
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    vk_testing::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    auto reduction_mode_ci = LvlInitStruct<VkSamplerReductionModeCreateInfo>();
    reduction_mode_ci.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
    sampler_ci.pNext = &reduction_mode_ci;
    vk_testing::Sampler sampler_reduction(*m_device, sampler_ci);

    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler_reduction.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-filterCubicMinmax-02695");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->reset();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-filterCubic-02694");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, VerifyImgFilterCubicSamplerInCmdDraw) {
    TEST_DESCRIPTION("Verify if sampler is filter cubic with the VK_IMG_filter cubic extension that it's a valid ImageViewType.");

    AddRequiredExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vk_testing::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = format;
    image_ci.extent.width = 128;
    image_ci.extent.height = 128;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = usage;
    VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_3D;

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView imageView = image.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                             VK_REMAINING_ARRAY_LAYERS, imageViewType);

    auto sampler_ci = LvlInitStruct<VkSamplerCreateInfo>();
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    VkSampler sampler;
    vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);

    static const char fs_src[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 x;
        void main(){
            x = texture(s, vec3(1));
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02693");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, VerifyMaxMultiviewInstanceIndex) {
    TEST_DESCRIPTION("Verify if instance index in CmdDraw is greater than maxMultiviewInstanceIndex.");
    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
        return;
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    multiview_features.multiview = VK_TRUE;
    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&multiview_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&multiview_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    if (multiview_props.maxMultiviewInstanceIndex == std::numeric_limits<uint32_t>::max()) {
        printf("%s maxMultiviewInstanceIndex is uint32_t max, skipping tests\n", kSkipPrefix);
        return;
    }
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");
    m_commandBuffer->Draw(1, multiview_props.maxMultiviewInstanceIndex + 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidSetFragmentShadingRateValues) {
    TEST_DESCRIPTION("Specify invalid fragment shading rate values");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    fsr_features.pipelineFragmentShadingRate = true;

    VkPhysicalDeviceFeatures2 device_features = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fsr_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &device_features));

    // Find address of extension call and make the call
    PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR =
        (PFN_vkCmdSetFragmentShadingRateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetFragmentShadingRateKHR");
    ASSERT_TRUE(vkCmdSetFragmentShadingRateKHR != nullptr);

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    fragmentSize.width = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;

    fragmentSize.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;

    fragmentSize.width = 8;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 8;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidSetFragmentShadingRateValuesNoFeatures) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Find address of extension call and make the call
    PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR =
        (PFN_vkCmdSetFragmentShadingRateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetFragmentShadingRateKHR");
    ASSERT_TRUE(vkCmdSetFragmentShadingRateKHR != nullptr);

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidSetFragmentShadingRateCombinerOpsNoFeatures) {
    TEST_DESCRIPTION("Specify combiner operations when only pipeline rate is supported");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);

    fsr_features.pipelineFragmentShadingRate = VK_TRUE;
    fsr_features.primitiveFragmentShadingRate = VK_FALSE;
    fsr_features.attachmentFragmentShadingRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Find address of extension call and make the call
    PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR =
        (PFN_vkCmdSetFragmentShadingRateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetFragmentShadingRateKHR");
    ASSERT_TRUE(vkCmdSetFragmentShadingRateKHR != nullptr);

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();

    combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidSetFragmentShadingRateCombinerOpsNoPipelineRate) {
    TEST_DESCRIPTION("Specify pipeline rate when only attachment or primitive rate are supported");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (fsr_features.attachmentFragmentShadingRate == VK_FALSE && fsr_features.primitiveFragmentShadingRate == VK_FALSE) {
        printf("%s requires attachmentFragmentShadingRate or primitiveFragmentShadingRate.\n", kSkipPrefix);
        return;
    }

    fsr_features.pipelineFragmentShadingRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Find address of extension call and make the call
    PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR =
        (PFN_vkCmdSetFragmentShadingRateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetFragmentShadingRateKHR");
    ASSERT_TRUE(vkCmdSetFragmentShadingRateKHR != nullptr);

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    fragmentSize.width = 2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508");
    vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;
}

TEST_F(VkLayerTest, InvalidSetFragmentShadingRateCombinerOpsLimit) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    if (fsr_properties.fragmentShadingRateNonTrivialCombinerOps) {
        printf("%s requires fragmentShadingRateNonTrivialCombinerOps to be unsupported.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!fsr_features.primitiveFragmentShadingRate && !fsr_features.attachmentFragmentShadingRate) {
        printf("%s requires primitiveFragmentShadingRate or attachmentFragmentShadingRate to be supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Find address of extension call and make the call
    PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR =
        (PFN_vkCmdSetFragmentShadingRateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetFragmentShadingRateKHR");
    ASSERT_TRUE(vkCmdSetFragmentShadingRateKHR != nullptr);

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    if (fsr_features.primitiveFragmentShadingRate) {
        combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512");
        vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
        m_errorMonitor->VerifyFound();
        combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }

    if (fsr_features.attachmentFragmentShadingRate) {
        combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512");
        vkCmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
        m_errorMonitor->VerifyFound();
        combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidPrimitiveFragmentShadingRateWriteMultiViewportLimitDynamic) {
    TEST_DESCRIPTION("Test dynamic validation of the primitiveFragmentShadingRateWithMultipleViewports limit");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        GTEST_SKIP() << "Test requires primitiveFragmentShadingRateWithMultipleViewports to be unsupported.";
    }

    auto eds_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(&eds_features);
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (!fsr_features.primitiveFragmentShadingRate) {
        GTEST_SKIP() << "Test requires primitiveFragmentShadingRate to be supported.";
    }

    if (!features2.features.multiViewport) {
        GTEST_SKIP() << "%s requires multiViewport to be supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_fragment_shading_rate : enable
        void main() {
            gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
        }
    )glsl";

    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    std::vector<VkRect2D> scissors = {{{0, 0}, {16, 16}}, {{1, 1}, {16, 16}}};
    pipe.SetScissor(scissors);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT);
    const VkPipelineLayoutObj pl(m_device);
    {
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        pipe.AddShader(&vs);
        VkResult err = pipe.CreateVKPipeline(pl.handle(), renderPass());
        ASSERT_VK_SUCCESS(err);
    }
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    VkViewport viewports[] = {{0, 0, 16, 16, 0, 1}, {1, 1, 16, 16, 0, 1}};
    PFN_vkCmdSetViewportWithCountEXT vkCmdSetViewportWithCountEXT =
        (PFN_vkCmdSetViewportWithCountEXT)vk::GetDeviceProcAddr(device(), "vkCmdSetViewportWithCountEXT");
    vkCmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2, viewports);

    // error produced here.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdUpdateBufferSize) {
    TEST_DESCRIPTION("Update buffer with invalid dataSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t update_data[4] = {0, 0, 0, 0};
    VkDeviceSize dataSize = sizeof(uint32_t) * 4;
    VkMemoryPropertyFlags reqs = 0;
    VkBufferObj buffer;
    buffer.init_as_src_and_dst(*m_device, dataSize, reqs);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-dataSize-00033");
    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer.handle(), sizeof(uint32_t), dataSize, (void *)update_data);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdUpdateBufferDstOffset) {
    TEST_DESCRIPTION("Update buffer with invalid dst offset");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t update_data[4] = {0, 0, 0, 0};
    VkDeviceSize dataSize = sizeof(uint32_t) * 4;
    VkMemoryPropertyFlags reqs = 0;
    VkBufferObj buffer;
    buffer.init_as_src_and_dst(*m_device, dataSize, reqs);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-dstOffset-00032");
    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer.handle(), sizeof(uint32_t) * 8, dataSize, (void *)update_data);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDescriptorSetPipelineBindPoint) {
    TEST_DESCRIPTION(
        "Attempt to bind descriptor set to a bind point not supported by command pool the command buffer was allocated from");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t no_gfx_qfi = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    const uint32_t INVALID_QUEUE = std::numeric_limits<uint32_t>::max();
    if (INVALID_QUEUE == no_gfx_qfi) {
        printf("%s No compute and transfer only queue family, skipping bindpoint and queue tests.\n", kSkipPrefix);
        return;
    }

    VkCommandPoolObj command_pool(m_device, no_gfx_qfi);
    ASSERT_TRUE(command_pool.initialized());
    VkCommandBufferObj command_buffer(m_device, &command_pool);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, nullptr, &ds_pool);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);

    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {dsl_binding});
    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout});

    command_buffer.begin();
    // Set invalid set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pipelineBindPoint-00361");
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, nullptr);
    m_errorMonitor->VerifyFound();
    command_buffer.end();
}

TEST_F(VkLayerTest, CommandBufferMissingOcclusionQueryEnabled) {
    TEST_DESCRIPTION(
        "Test executing secondary command buffer without VkCommandBufferInheritanceInfo::occlusionQueryEnable enabled while "
        "occlusion query is active.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &qpci, nullptr, &query_pool);

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;
    cbii.occlusionQueryEnable = VK_FALSE;  // Invalid

    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-00102");
    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, CmdClearColorImageNullColor) {
    TEST_DESCRIPTION("Test invalid null entries for clear color");

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageSubresourceRange isr = {};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.baseArrayLayer = 0;
    isr.baseMipLevel = 0;
    isr.layerCount = 1;
    isr.levelCount = 1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pColor-04961");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, nullptr, 1, &isr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidClearColorAttachmentsWithMultiview) {
    TEST_DESCRIPTION("Test cmdClearAttachments with active render pass that uses multiview");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (!multiview_features.multiview) {
        GTEST_SKIP() << "VkPhysicalDeviceMultiviewFeatures::multiview not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>(&renderPassMultiviewCreateInfo);
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                             VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>();
    framebufferCreateInfo.width = 32;
    framebufferCreateInfo.height = 32;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);

    // Start no RenderPass
    m_commandBuffer->begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;

    VkClearRect clear_rect = {};
    clear_rect.rect.extent.width = 32;
    clear_rect.rect.extent.height = 32;

    VkRenderPassBeginInfo render_pass_begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    render_pass_begin_info.renderPass = renderPass;
    render_pass_begin_info.framebuffer = framebuffer;
    render_pass_begin_info.renderArea.extent.width = 32;
    render_pass_begin_info.renderArea.extent.height = 32;

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-baseArrayLayer-00018");
    clear_rect.layerCount = 2;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-baseArrayLayer-00018");
    clear_rect.baseArrayLayer = 1;
    clear_rect.layerCount = 1;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestEndCommandBufferWithConditionalRendering) {
    TEST_DESCRIPTION("Call EndCommandBuffer when conditional rendering is active");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)vk::GetInstanceProcAddr(instance(), "vkCmdBeginConditionalRenderingEXT");

    auto buffer_ci =
        vk_testing::Buffer::create_info(32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT);
    vk_testing::Buffer buffer(*m_device, buffer_ci);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = LvlInitStruct<VkConditionalRenderingBeginInfoEXT>();
    conditional_rendering_begin.buffer = buffer.handle();

    VkCommandBufferBeginInfo command_buffer_begin = LvlInitStruct<VkCommandBufferBeginInfo>();

    vk::BeginCommandBuffer(m_commandBuffer->handle(), &command_buffer_begin);
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-None-01978");
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BindPipelineDuringTransformFeedback) {
    TEST_DESCRIPTION("Call CmdBindPipeline when transform feedback is active");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto xfb_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(xfb_features);
    if (!xfb_features.transformFeedback) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe_one(*this);
    pipe_one.InitInfo();
    pipe_one.InitState();
    pipe_one.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_two(*this);
    pipe_two.InitInfo();
    pipe_two.InitState();
    pipe_two.CreateGraphicsPipeline();

    auto vkCmdBeginTransformFeedbackEXT =
        (PFN_vkCmdBeginTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginTransformFeedbackEXT");
    ASSERT_TRUE(vkCmdBeginTransformFeedbackEXT != nullptr);
    auto vkCmdEndTransformFeedbackEXT =
        (PFN_vkCmdEndTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndTransformFeedbackEXT");
    ASSERT_TRUE(vkCmdEndTransformFeedbackEXT != nullptr);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_one.pipeline_);
    vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-None-02323");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_two.pipeline_);
    m_errorMonitor->VerifyFound();
    vkCmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DrawBlendEnabledFormatFeatures) {
    TEST_DESCRIPTION("Test pipeline blend enabled with missing image views format features");

    ASSERT_NO_FATAL_FAILURE(Init());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    VkFormat render_format = VkTestFramework::GetFormat(instance_, m_device);

    // Set format features from being found
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), render_format, &formatProps);
    if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        printf("%s Required linear tiling features not supported.\n", kSkipPrefix);
        return;
    }
    // Gets pass pipeline creation but not the actual tiling used
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    // will be caught at draw time that feature for optimal image is not set
    // InitRenderTarget() should be setting color attachment as VK_IMAGE_TILING_LINEAR
    formatProps.linearTilingFeatures &= ~VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), render_format, formatProps);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.cb_attachments_[0].blendEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-blendEnable-04727");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidEndConditionalRendering) {
    TEST_DESCRIPTION("Invalid calls to vkCmdEndConditionalRenderingEXT.");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dep = {0,
                               1,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &imageView;
    fbci.width = 32;
    fbci.height = 32;
    fbci.layers = 1;
    VkFramebuffer framebuffer;
    vk::CreateFramebuffer(device(), &fbci, nullptr, &framebuffer);

    PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBeginConditionalRenderingEXT");
    PFN_vkCmdEndConditionalRenderingEXT vkCmdEndConditionalRenderingEXT =
        (PFN_vkCmdEndConditionalRenderingEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdEndConditionalRenderingEXT");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = LvlInitStruct<VkConditionalRenderingBeginInfoEXT>();
    conditional_rendering_begin.buffer = buffer.handle();

    VkClearValue clear_value;
    clear_value.color = m_clear_color;

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.renderPass = render_pass.handle();
    rpbi.framebuffer = framebuffer;
    rpbi.renderArea = {{0, 0}, {32, 32}};
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = &clear_value;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndConditionalRenderingEXT-None-01985");
    vkCmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndConditionalRenderingEXT-None-01986");
    vkCmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    vkCmdEndConditionalRenderingEXT(m_commandBuffer->handle());

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndConditionalRenderingEXT-None-01987");
    vkCmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());
}

TEST_F(VkLayerTest, InvalidBeginTransformFeedbackInMultiviewRenderPass) {
    TEST_DESCRIPTION("Test beginning transform feedback in a render pass with multiview enabled");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto mv_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>();
    auto tf_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(&mv_features);
    auto pd_features = GetPhysicalDeviceFeatures2(tf_features);

    if (!tf_features.transformFeedback) {
        GTEST_SKIP() << "transformFeedback not supported; skipped.";
    }
    if (!mv_features.multiview) {
        GTEST_SKIP() << "multiview not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>(&renderPassMultiviewCreateInfo);
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, renderPassCreateInfo);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    auto image_view_ci = image.TargetViewCI(VK_FORMAT_R8G8B8A8_UNORM);
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    VkImageView imageView = image.targetView(image_view_ci);

    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>();
    framebufferCreateInfo.width = 32;
    framebufferCreateInfo.height = 32;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, framebufferCreateInfo);

    VkRenderPassBeginInfo render_pass_begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.framebuffer = framebuffer.handle();
    render_pass_begin_info.renderArea.extent.width = 32;
    render_pass_begin_info.renderArea.extent.height = 32;

    auto vkCmdBeginTransformFeedbackEXT =
        (PFN_vkCmdBeginTransformFeedbackEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginTransformFeedbackEXT");
    ASSERT_TRUE(vkCmdBeginTransformFeedbackEXT != nullptr);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_begin_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-None-02373");
    vkCmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BeginRenderingWithSecondaryContents) {
    TEST_DESCRIPTION("Test that an error is produced when a secondary command buffer calls BeginRendering with secondary contents");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-commandBuffer-06068");
    secondary.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    secondary.end();
}

TEST_F(VkLayerTest, BadRenderPassContentsWhenCallingCmdExecuteCommandsWithBeginRenderPass) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRenderPass that hasn't set "
        "VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        m_renderPass,
        0,  // subpass
        m_framebuffer,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    const VkRenderPassBeginInfo rp_bi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                      nullptr,  // pNext
                                      m_renderPass,
                                      m_framebuffer,
                                      {{0, 0}, {32, 32}},
                                      static_cast<uint32_t>(m_renderPassClearValues.size()),
                                      m_renderPassClearValues.data()};

    m_commandBuffer->BeginRenderPass(rp_bi);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-contents-06018");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-contents-00095");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BadRenderPassContentsWhenCallingCmdExecuteCommandsWithBeginRendering) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that hasn't set "
        "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-flags-06024");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BadExecuteCommandsSubpassIndices) {
    TEST_DESCRIPTION("Test invalid subpass when calling CmdExecuteCommands");

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependencies = {
        0,                                     // srcSubpass
        1,                                     // dstSubpass
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,    // srcStageMask
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,    // dstStageMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,  // srcAccessMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,  // dstAccessMask
        0,                                     // dependencyFlags
    };

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependencies;
    vk_testing::RenderPass render_pass(*m_device, rpci);
    ASSERT_TRUE(render_pass.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, render_pass.handle(), 1, &imageView, 32, 32, 1};
    vk_testing::Framebuffer framebuffer(*m_device, fbci);
    ASSERT_TRUE(framebuffer.initialized());

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        render_pass.handle(),
        1,  // subpass
        framebuffer.handle(),
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    const VkRenderPassBeginInfo rp_bi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                      nullptr,  // pNext
                                      render_pass.handle(),
                                      framebuffer.handle(),
                                      {{0, 0}, {32, 32}},
                                      0,
                                      nullptr};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00097");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IncompatibleRenderPassesInExecuteCommands) {
    TEST_DESCRIPTION("Test invalid subpass when calling CmdExecuteCommands");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};
    VkRenderPass render_pass_1;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &render_pass_1);
    ASSERT_VK_SUCCESS(err);

    VkRenderPassCreateInfo rpci_2 = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 2, subpasses, 0, nullptr};
    VkRenderPass render_pass_2;
    vk::CreateRenderPass(m_device->device(), &rpci_2, nullptr, &render_pass_2);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, render_pass_1, 1, &imageView, 32, 32, 1};
    VkFramebuffer framebuffer;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &framebuffer);
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        render_pass_2,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    const VkRenderPassBeginInfo rp_bi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                      nullptr,  // pNext
                                      render_pass_1,
                                      framebuffer,
                                      {{0, 0}, {32, 32}},
                                      0,
                                      nullptr};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pInheritanceInfo-00098");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyRenderPass(m_device->device(), render_pass_2, nullptr);
    vk::DestroyRenderPass(m_device->device(), render_pass_1, nullptr);
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithNonNullRenderPass) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that hasn't set "
        "renderPass to VK_NULL_HANDLE in pInheritanceInfo");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

   VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 2, subpasses, 0, nullptr};
    VkRenderPass render_pass;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &render_pass);
    ASSERT_VK_SUCCESS(err);

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        render_pass,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06025");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    vk::DestroyRenderPass(m_device->device(), render_pass, nullptr);
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingFlags) {
    TEST_DESCRIPTION("Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching flags");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags =
        VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR | VK_RENDERING_SUSPENDING_BIT_KHR | VK_RENDERING_RESUMING_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-flags-06026");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingColorAttachmentCount) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching colorAttachmentCount");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-colorAttachmentCount-06027");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingColorImageViewFormat) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching color image view format");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat bad_color_formats = {VK_FORMAT_R8G8B8A8_UINT};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &bad_color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-imageView-06028");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingDepthStencilImageViewFormat) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching depth/stencil image view "
        "format");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_stencil_format) {
        printf("%s Couldn't depth stencil image format.\n", kSkipPrefix);
        return;
    }
    image.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = imageView;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_rendering_info.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pDepthAttachment-06029");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pStencilAttachment-06030");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingViewMask) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching viewMask format");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto mv_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>(&mv_features);
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering , skipping.";
    }
    if (!mv_features.multiview) {
        GTEST_SKIP() << "multiview feature not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.viewMask = 0;
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.viewMask = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-viewMask-06031");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingImageViewRasterizationSamples) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching rasterization samples");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    // color samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06035");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    VkImageObj depthStencilImage(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_stencil_format) {
        printf("%s Couldn't depth stencil image format.\n", kSkipPrefix);
        return;
    }
    depthStencilImage.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                           0);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.depthAttachmentFormat = depth_stencil_format;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // depth samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06036");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    inheritance_rendering_info.stencilAttachmentFormat = depth_stencil_format;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // stencil samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06037");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingAndExecuteCommandsWithMismatchingImageViewAttachmentSamples) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching that has mismatching "
        "attachment samples");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    bool amd_samples = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
        amd_samples = true;
    }

    bool nv_samples = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
        nv_samples = true;
    }

    if (!amd_samples && !nv_samples) {
        printf("%s Test requires either VK_AMD_mixed_attachment_samples or VK_NV_framebuffer_mixed_samples, skipping\n",
               kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkSampleCountFlagBits counts = {VK_SAMPLE_COUNT_2_BIT};
    auto samples_info = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();
    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = &counts;

    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>(&samples_info);
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    // color samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06032");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    VkImageObj depthStencilImage(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_stencil_format) {
        printf("%s Couldn't depth stencil image format.\n", kSkipPrefix);
        return;
    }
    depthStencilImage.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                           0);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    samples_info.colorAttachmentCount = 0;
    samples_info.pColorAttachmentSamples = nullptr;
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.depthAttachmentFormat = depth_stencil_format;
    samples_info.depthStencilAttachmentSamples = VK_SAMPLE_COUNT_2_BIT;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // depth samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06033");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    inheritance_rendering_info.stencilAttachmentFormat = depth_stencil_format;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // stencil samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06034");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyCommands2V13) {
    TEST_DESCRIPTION("Ensure copy_commands2 promotions are validated");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageObj image2(m_device);
    image2.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkBufferObj dst_buffer;
    VkMemoryPropertyFlags reqs = 0;
    dst_buffer.init_as_dst(*m_device, 128 * 128, reqs);
    VkBufferObj src_buffer;
    src_buffer.init_as_src(*m_device, 128 * 128, reqs);
    auto copy_region = LvlInitStruct<VkImageCopy2>();
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.dstOffset = {4, 4, 0};
    copy_region.extent.width = 1;
    copy_region.extent.height = 1;
    copy_region.extent.depth = 1;
    auto copy_image_info = LvlInitStruct<VkCopyImageInfo2>();
    copy_image_info.srcImage = image.handle();
    copy_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info.dstImage = image.handle();
    copy_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info.regionCount = 1;
    copy_image_info.pRegions = &copy_region;
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImage-00131");
    vk::CmdCopyImage2(m_commandBuffer->handle(), &copy_image_info);
    m_errorMonitor->VerifyFound();
    auto copy_buffer = LvlInitStruct<VkBufferCopy2>();
    copy_buffer.dstOffset = 4;
    copy_buffer.size = 4;
    auto copy_buffer_info = LvlInitStruct<VkCopyBufferInfo2>();
    copy_buffer_info.srcBuffer = dst_buffer.handle();
    copy_buffer_info.dstBuffer = dst_buffer.handle();
    copy_buffer_info.regionCount = 1;
    copy_buffer_info.pRegions = &copy_buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferInfo2-srcBuffer-00118");
    vk::CmdCopyBuffer2(m_commandBuffer->handle(), &copy_buffer_info);
    m_errorMonitor->VerifyFound();
    auto bic_region = LvlInitStruct<VkBufferImageCopy2>();
    bic_region.bufferRowLength = 128;
    bic_region.bufferImageHeight = 128;
    bic_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bic_region.imageSubresource.layerCount = 1;
    bic_region.imageExtent.height = 4;
    bic_region.imageExtent.width = 4;
    bic_region.imageExtent.depth = 1;
    VkCopyBufferToImageInfo2KHR buffer_image_info = LvlInitStruct<VkCopyBufferToImageInfo2>();
    buffer_image_info.srcBuffer = src_buffer.handle();
    buffer_image_info.dstImage = image.handle();
    buffer_image_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_info.regionCount = 1;
    buffer_image_info.pRegions = &bic_region;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-dstImage-00177");
    vk::CmdCopyBufferToImage2(m_commandBuffer->handle(), &buffer_image_info);
    m_errorMonitor->VerifyFound();
    auto image_buffer_info = LvlInitStruct<VkCopyImageToBufferInfo2>();
    image_buffer_info.dstBuffer = src_buffer.handle();
    image_buffer_info.srcImage = image.handle();
    image_buffer_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_buffer_info.regionCount = 1;
    image_buffer_info.pRegions = &bic_region;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToBufferInfo2-dstBuffer-00191");
    vk::CmdCopyImageToBuffer2(m_commandBuffer->handle(), &image_buffer_info);
    m_errorMonitor->VerifyFound();
    auto blit_region = LvlInitStruct<VkImageBlit2>();
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {31, 31, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    auto blit_image_info = LvlInitStruct<VkBlitImageInfo2>();
    blit_image_info.srcImage = image.handle();
    blit_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    blit_image_info.dstImage = image.handle();
    blit_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    blit_image_info.regionCount = 1;
    blit_image_info.pRegions = &blit_region;
    blit_image_info.filter = VK_FILTER_NEAREST;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-dstImage-00224");
    vk::CmdBlitImage2(m_commandBuffer->handle(), &blit_image_info);
    m_errorMonitor->VerifyFound();
    auto resolve_region = LvlInitStruct<VkImageResolve2>();
    resolve_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve_region.srcSubresource.mipLevel = 0;
    resolve_region.srcSubresource.baseArrayLayer = 0;
    resolve_region.srcSubresource.layerCount = 1;
    resolve_region.srcOffset.x = 0;
    resolve_region.srcOffset.y = 0;
    resolve_region.srcOffset.z = 0;
    resolve_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve_region.dstSubresource.mipLevel = 0;
    resolve_region.dstSubresource.baseArrayLayer = 0;
    resolve_region.dstSubresource.layerCount = 1;
    resolve_region.dstOffset.x = 0;
    resolve_region.dstOffset.y = 0;
    resolve_region.dstOffset.z = 0;
    resolve_region.extent.width = 1;
    resolve_region.extent.height = 1;
    resolve_region.extent.depth = 1;
    auto resolve_image_info = LvlInitStruct<VkResolveImageInfo2>();
    resolve_image_info.srcImage = image.handle();
    resolve_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_image_info.dstImage = image2.handle();
    resolve_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_image_info.regionCount = 1;
    resolve_image_info.pRegions = &resolve_region;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkResolveImageInfo2-srcImage-00257");
    vk::CmdResolveImage2(m_commandBuffer->handle(), &resolve_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateMultiviewUnboundResourcesAfterBeginRenderPassAndNextSubpass) {
    TEST_DESCRIPTION(
        "Validate all required resources are bound if multiview is enabled after vkCmdBeginRenderPass and vkCmdNextSubpass");

    m_errorMonitor->ExpectSuccess();

    constexpr unsigned multiview_count = 2u;
    constexpr unsigned extra_subpass_count = multiview_count - 1u;

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Device does not support multiview.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    m_renderPass_subpasses.resize(multiview_count);
    for (unsigned i = 0; i < multiview_count; ++i) {
        m_renderPass_subpasses[i].colorAttachmentCount = 1;
        m_renderPass_subpasses[i].pColorAttachments = &colorAttachmentReference;
    }

    uint32_t viewMasks[multiview_count] = {};
    for (unsigned i = 0; i < multiview_count; ++i) {
        viewMasks[i] = 1u << i;
    }
    auto renderPassMultiviewCreateInfo = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    renderPassMultiviewCreateInfo.subpassCount = multiview_count;
    renderPassMultiviewCreateInfo.pViewMasks = viewMasks;

    m_renderPass_info = LvlInitStruct<VkRenderPassCreateInfo>(&renderPassMultiviewCreateInfo);
    m_renderPass_info.attachmentCount = 1;
    m_renderPass_info.pAttachments = &attachmentDescription;
    m_renderPass_info.subpassCount = m_renderPass_subpasses.size();
    m_renderPass_info.pSubpasses = m_renderPass_subpasses.data();

    m_renderPass_dependencies.resize(extra_subpass_count);
    for (unsigned i = 0; i < m_renderPass_dependencies.size(); ++i) {
        auto &subpass_dep = m_renderPass_dependencies[i];
        subpass_dep.srcSubpass = i;
        subpass_dep.dstSubpass = i + 1;

        subpass_dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        subpass_dep.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dep.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        subpass_dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    m_renderPass_info.dependencyCount = static_cast<uint32_t>(m_renderPass_dependencies.size());
    m_renderPass_info.pDependencies = m_renderPass_dependencies.data();

    vk::CreateRenderPass(m_device->handle(), &m_renderPass_info, nullptr, &m_renderPass);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = static_cast<uint32_t>(m_width);
    image_create_info.extent.height = static_cast<uint32_t>(m_height);
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = multiview_count;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                             VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    auto framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>();
    framebufferCreateInfo.width = static_cast<uint32_t>(m_width);
    framebufferCreateInfo.height = static_cast<uint32_t>(m_height);
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView;

    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &m_framebuffer);

    VkClearValue clear{};
    clear.color = m_clear_color;
    m_renderPassClearValues.emplace_back(clear);
    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = m_framebuffer;
    m_renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(m_width);
    m_renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(m_height);
    m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();

    // Pipeline not bound test
    {
        // No need to create individual pipelines for each subpass since we are checking no bound pipeline
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        // This bind should not be valid after we begin the renderpass
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02700");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            m_errorMonitor->ExpectSuccess();

            // This bind should not be valid for next subpass
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
            vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02700");
            m_commandBuffer->Draw(1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->ExpectSuccess();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Dynamic state (checking with line width)
    {
        // Pipeline for subpass 0
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();

        VkDynamicState dyn_states = VK_DYNAMIC_STATE_LINE_WIDTH;
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        pipe.dyn_state_ci_ = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        pipe.dyn_state_ci_.dynamicStateCount = 1;
        pipe.dyn_state_ci_.pDynamicStates = &dyn_states;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vk_testing::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }

        m_commandBuffer->begin();
        // This line width set should not be valid for next subpass
        vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-commandBuffer-02701");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            m_errorMonitor->ExpectSuccess();

            // This line width set should not be valid for next subpass
            vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
            vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-commandBuffer-02701");
            m_commandBuffer->Draw(1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->ExpectSuccess();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Push constants
    {
        char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo {
           mat3 m;
        } constants;
        void main(){
            vec3 v3 = constants.m[0];
        }
    )glsl";

        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj const fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 16};
        auto pipeline_layout_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        vk_testing::PipelineLayout layout;
        layout.init(*m_device, pipeline_layout_info, std::vector<const vk_testing::DescriptorSetLayout *>{});

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vk_testing::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        const float dummy_values[16] = {};
        m_commandBuffer->begin();
        // This push constants should not be counted when render pass begins
        vk::CmdPushConstants(m_commandBuffer->handle(), layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, push_constant_range.offset,
                             push_constant_range.size, dummy_values);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-maintenance4-06425");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            m_errorMonitor->ExpectSuccess();

            // This push constants should not be counted when we change subpass
            vk::CmdPushConstants(m_commandBuffer->handle(), layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, push_constant_range.offset,
                                 push_constant_range.size, dummy_values);
            vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-maintenance4-06425");
            m_commandBuffer->Draw(1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->ExpectSuccess();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Descriptor sets
    {
        OneOffDescriptorSet descriptor_set{m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}}};

        auto bci = LvlInitStruct<VkBufferCreateInfo>();
        bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bci.size = 8;
        VkBufferObj buffer;
        buffer.init(*m_device, bci);
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        auto pipeline_layout_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &descriptor_set.layout_.handle();

        vk_testing::PipelineLayout layout;
        layout.init(*m_device, pipeline_layout_info, std::vector<vk_testing::DescriptorSetLayout const *>{});

        VkShaderObj const vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj const fs(this, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vk_testing::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        m_commandBuffer->begin();
        // This descriptor bind should not be counted when render pass begins
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02697");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotBound");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            m_errorMonitor->ExpectSuccess();

            // This descriptor bind should not be counted when next subpass begins
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0,
                                      1, &descriptor_set.set_, 0, nullptr);
            vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02697");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotBound");
            m_commandBuffer->Draw(1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->ExpectSuccess();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Vertex buffer
    {
        float const vertex_data[] = {1.0f, 0.0f};
        VkConstantBufferObj vbo(m_device, static_cast<int>(sizeof(vertex_data)), reinterpret_cast<const void *>(vertex_data),
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        VkVertexInputBindingDescription input_binding{};
        input_binding.binding = 0;
        input_binding.stride = sizeof(vertex_data);
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription input_attribs{};
        input_attribs.binding = 0;
        input_attribs.location = 0;
        input_attribs.format = VK_FORMAT_R32G32_SFLOAT;
        input_attribs.offset = 0;

        char const *const vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        void main(){
           gl_Position = vec4(input0.x, input0.y, 0.0f, 1.0f);
        }
    )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.vi_ci_.vertexBindingDescriptionCount = 1;
        pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
        pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vk_testing::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete
        VkDeviceSize offset = 0;

        m_commandBuffer->begin();
        // This vertex buffer bind should not be counted when render pass begins
        vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04007");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            m_errorMonitor->ExpectSuccess();

            // This vertex buffer bind should not be counted when next subpass begins
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04007");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");
            m_commandBuffer->Draw(1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->ExpectSuccess();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Index buffer
    {
        float const vertex_data[] = {1.0f, 0.0f};
        VkConstantBufferObj vbo(m_device, static_cast<int>(sizeof(vertex_data)), reinterpret_cast<const void *>(vertex_data),
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        uint32_t const index_data[] = {0};
        VkConstantBufferObj ibo(m_device, static_cast<int>(sizeof(index_data)), reinterpret_cast<const void *>(index_data),
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        VkVertexInputBindingDescription input_binding{};
        input_binding.binding = 0;
        input_binding.stride = sizeof(vertex_data);
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription input_attribs{};
        input_attribs.binding = 0;
        input_attribs.location = 0;
        input_attribs.format = VK_FORMAT_R32G32_SFLOAT;
        input_attribs.offset = 0;

        char const *const vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        void main(){
           gl_Position = vec4(input0.x, input0.y, 0.0f, 1.0f);
        }
    )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.vi_ci_.vertexBindingDescriptionCount = 1;
        pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
        pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vk_testing::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        VkDeviceSize offset = 0;
        m_commandBuffer->begin();
        // This index buffer bind should not be counted when render pass begins
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
        m_errorMonitor->VerifyNotFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02701");
        m_commandBuffer->DrawIndexed(0, 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            m_errorMonitor->ExpectSuccess();

            // This index buffer bind should not be counted when next subpass begins
            vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);
            vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02701");
            m_commandBuffer->DrawIndexed(0, 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->ExpectSuccess();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, TestCommandBufferInheritanceWithInvalidDepthFormat) {
    TEST_DESCRIPTION(
        "Test VkCommandBufferInheritanceRenderingInfoKHR with depthAttachmentFormat that does not include depth aspect");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (!stencil_format) {
        printf("%s Couldn't stencil image format.\n", kSkipPrefix);
        return;
    }

    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.depthAttachmentFormat = stencil_format;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    auto cmdbuf_ii = LvlInitStruct<VkCommandBufferInheritanceInfo>(&inheritance_rendering_info);
    auto cmdbuf_bi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmdbuf_bi.pInheritanceInfo = &cmdbuf_ii;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540");

    vk::BeginCommandBuffer(secondary.handle(), &cmdbuf_bi);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ResolveInvalidUsage) {
    TEST_DESCRIPTION("Resolve image with missing usage flags.");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(Init());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    VkFormat src_format = VK_FORMAT_R8_UNORM;
    VkFormat dst_format = VK_FORMAT_R8_SNORM;

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), src_format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), src_format, formatProps);
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), dst_format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), dst_format, formatProps);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;

    VkImageObj srcImage(m_device);
    srcImage.init(&image_create_info);
    ASSERT_TRUE(srcImage.initialized());

    image_create_info.format = dst_format;
    VkImageObj srcImage2(m_device);
    srcImage2.init(&image_create_info);
    ASSERT_TRUE(srcImage2.initialized());

    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj invalidSrcImage(m_device);
    invalidSrcImage.init(&image_create_info);
    ASSERT_TRUE(invalidSrcImage.initialized());

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.format = src_format;
    VkImageObj invalidSrcImage2(m_device);
    invalidSrcImage2.init(&image_create_info);
    ASSERT_TRUE(invalidSrcImage2.initialized());

    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageObj dstImage(m_device);
    dstImage.init(&image_create_info);
    ASSERT_TRUE(dstImage.initialized());

    image_create_info.format = src_format;
    VkImageObj dstImage2(m_device);
    dstImage2.init(&image_create_info);
    ASSERT_TRUE(dstImage2.initialized());

    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkImageObj invalidDstImage(m_device);
    invalidDstImage.init(&image_create_info);
    ASSERT_TRUE(invalidDstImage.initialized());

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.format = dst_format;
    VkImageObj invalidDstImage2(m_device);
    invalidDstImage2.init(&image_create_info);
    ASSERT_TRUE(invalidDstImage2.initialized());

    m_commandBuffer->begin();
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-06762");
    m_commandBuffer->ResolveImage(invalidSrcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-06764");
    m_commandBuffer->ResolveImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, invalidDstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-06763");
    m_commandBuffer->ResolveImage(invalidSrcImage2.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                  1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-06765");
    m_commandBuffer->ResolveImage(srcImage2.handle(), VK_IMAGE_LAYOUT_GENERAL, invalidDstImage2.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                  1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}
