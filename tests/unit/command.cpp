/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

TEST_F(NegativeCommand, CommandPoolConsistency) {
    TEST_DESCRIPTION("Allocate command buffers from one command pool and attempt to delete them from another.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeCommandBuffers-pCommandBuffers-parent");

    RETURN_IF_SKIP(Init())

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkt::CommandPool command_pool_1(*m_device, pool_create_info);
    vkt::CommandPool command_pool_2(*m_device, pool_create_info);

    VkCommandBuffer cb;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool_1.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &cb);

    vk::FreeCommandBuffers(m_device->device(), command_pool_2.handle(), 1, &cb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, SecondaryCommandBufferBarrier) {
    TEST_DESCRIPTION("Add an invalid image barrier in a secondary command buffer");
    RETURN_IF_SKIP(Init())

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
    vkt::RenderPass rp(*m_device, rpci);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    // Second image that img_barrier will incorrectly use
    VkImageObj image2(m_device);
    image2.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fbci);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
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
}

TEST_F(NegativeCommand, IndexBufferNotBound) {
    TEST_DESCRIPTION("Run an indexed draw call without an index buffer bound.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    vkt::Buffer index_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-None-07312");
    // Use DrawIndexed w/o an index buffer bound
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 3, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IndexBufferSizeOffset) {
    TEST_DESCRIPTION("Run bind index buffer with an offset greater than the size of the index buffer.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    vkt::Buffer index_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 512, VK_INDEX_TYPE_UINT16);

    // draw one past the end of the buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-robustBufferAccess2-07825");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 256, 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // draw one too many indices
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-robustBufferAccess2-07825");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 257, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    // draw one too many indices
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-robustBufferAccess2-07825");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 513, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // draw one past the end of the buffer using the offset
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-robustBufferAccess2-07825");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 512, 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MissingClearAttachment) {
    TEST_DESCRIPTION("Points to a wrong colorAttachment index in a VkClearAttachment structure passed to vkCmdClearAttachments");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    VkClearAttachment color_attachment = {};
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.colorAttachment = 2;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07271");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, SecondaryCommandbufferAsPrimary) {
    TEST_DESCRIPTION("Create a secondary command buffer and pass it to QueueSubmit.");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-00075");

    RETURN_IF_SKIP(Init())

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    secondary.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &secondary.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, Sync2SecondaryCommandbufferAsPrimary) {
    TEST_DESCRIPTION("Create a secondary command buffer and pass it to QueueSubmit2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferSubmitInfo-commandBuffer-03890");

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    secondary.end();

    VkCommandBufferSubmitInfoKHR cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = secondary.handle();

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CommandBufferTwoSubmits) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // We luck out b/c by default the framework creates CB w/ the
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set
    m_commandBuffer->begin();
    m_commandBuffer->end();

    // Bypass framework since it does the waits automatically
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    // Cause validation error by re-submitting cmd buffer that should only be
    // submitted once
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, Sync2CommandBufferTwoSubmits) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation");
    InitRenderTarget();

    // We luck out b/c by default the framework creates CB w/ the
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set
    m_commandBuffer->begin();
    m_commandBuffer->end();

    // Bypass framework since it does the waits automatically
    VkCommandBufferSubmitInfoKHR cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = m_commandBuffer->handle();

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    // Cause validation error by re-submitting cmd buffer that should only be
    // submitted once
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, PushConstants) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineLayout pipeline_layout;
    VkPushConstantRange pc_range = {};
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &pc_range;

    //
    // Check for invalid push constant ranges in pipeline layouts.
    //
    struct PipelineLayoutTestCase {
        VkPushConstantRange const range;
        char const *msg;
    };

    const uint32_t too_big = m_device->phy().limits_.maxPushConstantsSize + 0x4;
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
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
             }},
            {{{VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4},
              {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4}},
             {
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
             }},
            {{{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
              {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4}},
             {
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
             }},
        },
    };

    for (const auto &iter : duplicate_stageFlags_tests) {
        pipeline_layout_ci.pPushConstantRanges = iter.ranges;
        pipeline_layout_ci.pushConstantRangeCount = ranges_per_test;
        std::for_each(iter.msg.begin(), iter.msg.end(),
                      [&](const char *vuid) { m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid); });
        vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    //
    // CmdPushConstants tests
    //

    // Setup a pipeline layout with ranges: [0,32) [16,80)
    const std::vector<VkPushConstantRange> pc_range2 = {{VK_SHADER_STAGE_VERTEX_BIT, 16, 64},
                                                        {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 32}};
    const vkt::PipelineLayout pipeline_layout_obj(*m_device, {}, pc_range2);

    const uint8_t dummy_values[100] = {};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Check for invalid stage flag
    // Note that VU 07790 isn't reached due to parameter validation
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-stageFlags-requiredbitmask");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), 0, 0, 16, dummy_values);
    m_errorMonitor->VerifyFound();

    // Positive tests for the overlapping ranges
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16,
                         dummy_values);
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_VERTEX_BIT, 32, 48, dummy_values);
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout_obj.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 16, 16, dummy_values);

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

TEST_F(NegativeCommand, NoBeginCommandBuffer) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-commandBuffer-00059");

    RETURN_IF_SKIP(Init())
    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);
    // Call EndCommandBuffer() w/o calling BeginCommandBuffer()
    vk::EndCommandBuffer(commandBuffer.handle());

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, SecondaryCommandBufferRerecordedExplicitReset) {
    RETURN_IF_SKIP(Init())

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-recording");

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

TEST_F(NegativeCommand, SecondaryCommandBufferRerecordedNoReset) {
    RETURN_IF_SKIP(Init())

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-recording");

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

TEST_F(NegativeCommand, CascadedInvalidation) {
    RETURN_IF_SKIP(Init())

    VkEventCreateInfo eci = vku::InitStructHelper();
    eci.flags = 0;
    VkEvent event;
    vk::CreateEvent(m_device->device(), &eci, nullptr, &event);

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin();
    vk::CmdSetEvent(secondary.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->end();

    // destroying the event should invalidate both primary and secondary CB
    vk::DestroyEvent(m_device->device(), event, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CommandBufferReset) {
    // Cause error due to Begin while recording CB
    // Then cause 2 errors for attempting to reset CB w/o having
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT set for the pool from
    // which CBs were allocated. Note that this bit is off by default.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBeginCommandBuffer-commandBuffer-00049");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState(nullptr, nullptr, 0));

    // Calls AllocateCommandBuffers
    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);

    // Force the failure by setting the Renderpass and Framebuffer fields with (fake) data
    VkCommandBufferInheritanceInfo cmd_buf_hinfo = vku::InitStructHelper();
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
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

TEST_F(NegativeCommand, CommandBufferPrimaryFlags) {
    RETURN_IF_SKIP(Init())

    // Calls AllocateCommandBuffers
    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);

    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBeginCommandBuffer-commandBuffer-02840");
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearColorAttachmentsOutsideRenderPass) {
    // Call CmdClearAttachmentss outside of an active RenderPass

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-renderpass");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

TEST_F(NegativeCommand, ClearColorAttachmentsZeroLayercount) {
    TEST_DESCRIPTION("Call CmdClearAttachments with a pRect having a layerCount of zero.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-layerCount-01934");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

TEST_F(NegativeCommand, ClearColorAttachmentsZeroExtent) {
    TEST_DESCRIPTION("Call CmdClearAttachments with a pRect having a rect2D extent of zero.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

TEST_F(NegativeCommand, ClearAttachmentsAspectMasks) {
    TEST_DESCRIPTION("Check VkClearAttachment invalid aspect masks.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

TEST_F(NegativeCommand, ClearAttachmentsImplicitCheck) {
    TEST_DESCRIPTION("Check VkClearAttachment implicit VUs.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

TEST_F(NegativeCommand, ClearAttachmentsDepth) {
    TEST_DESCRIPTION("Call CmdClearAttachments with invalid depth aspect masks.");

    RETURN_IF_SKIP(Init())
    m_depth_stencil_fmt = FindSupportedStencilOnlyFormat(gpu());
    if (m_depth_stencil_fmt == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view = m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);

    attachment.clearValue.depthStencil.depth = 0.0f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07884");
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsStencil) {
    TEST_DESCRIPTION("Call CmdClearAttachments with invalid stencil aspect masks.");

    RETURN_IF_SKIP(Init())
    m_depth_stencil_fmt = FindSupportedDepthOnlyFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view = m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT);
    InitRenderTarget(&depth_image_view);

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);

    attachment.clearValue.depthStencil.depth = 0.0f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07885");
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsOutsideRenderPass) {
    TEST_DESCRIPTION("Call CmdClearAttachments outside renderpass");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    m_commandBuffer->begin();
    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-renderpass");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DrawOutsideRenderPass) {
    TEST_DESCRIPTION("call vkCmdDraw without renderpass");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderpass");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawDrawOutsideRenderPass) {
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(multi_draw_features);
    if (!multi_draw_features.multiDraw) {
        GTEST_SKIP() << "Test requires (unsupported) multiDraw";
    }
    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));
    InitRenderTarget();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-renderpass");
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ExecuteCommandsPrimaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a primary command buffer (should only be secondary)");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // An empty primary command buffer
    vkt::CommandBuffer cb(m_device, m_commandPool);
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

TEST_F(NegativeCommand, DISABLED_ExecuteCommandsToSecondaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands to a Secondary command buffer");

    RETURN_IF_SKIP(Init())

    vkt::CommandBuffer main_cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary_cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.begin();
    secondary_cb.end();

    main_cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-bufferlevel");
    vk::CmdExecuteCommands(main_cb.handle(), 1, &secondary_cb.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, SimultaneousUseSecondaryTwoExecutes) {
    RETURN_IF_SKIP(Init())

    const char *simultaneous_use_message = "VUID-vkCmdExecuteCommands-pCommandBuffers-00092";

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper();
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

TEST_F(NegativeCommand, SimultaneousUseSecondarySingleExecute) {
    RETURN_IF_SKIP(Init())

    // variation on previous test executing the same CB twice in the same
    // CmdExecuteCommands call

    const char *simultaneous_use_message = "VUID-vkCmdExecuteCommands-pCommandBuffers-00093";

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper();
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

TEST_F(NegativeCommand, SimultaneousUseOneShot) {
    TEST_DESCRIPTION("Submit the same command buffer twice in one submit looking for simultaneous use and one time submit errors");
    const char *simultaneous_use_message = "is already in use and is not marked for simultaneous use";
    RETURN_IF_SKIP(Init())

    VkCommandBuffer cmd_bufs[2];
    VkCommandBufferAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.commandBufferCount = 2;
    alloc_info.commandPool = m_commandPool->handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &alloc_info, cmd_bufs);

    VkCommandBufferBeginInfo cb_binfo = vku::InitStructHelper();
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    vk::BeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(cmd_bufs[0], 0, 1, &viewport);
    vk::EndCommandBuffer(cmd_bufs[0]);
    VkCommandBuffer duplicates[2] = {cmd_bufs[0], cmd_bufs[0]};

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = duplicates;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, simultaneous_use_message);
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);

    // Set one time use and now look for one time submit
    duplicates[0] = duplicates[1] = cmd_bufs[1];
    cb_binfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    vk::CmdSetViewport(cmd_bufs[1], 0, 1, &viewport);
    vk::EndCommandBuffer(cmd_bufs[1]);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00071");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeCommand, DrawTimeImageViewTypeMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo image_info = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_GENERAL};

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(image_info);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkDescriptorSet set_obj = descriptorSet.GetDescriptorSetHandle();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.GetPipelineLayout(), 0, 1,
                              &set_obj, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewType-07752");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DrawTimeImageViewTypeMismatchWithPipelineUpdateAfterBind) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(indexing_features);
    if (!indexing_features.descriptorBindingSampledImageUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported)  descriptorBindingSampledImageUpdateAfterBind";
    }

    RETURN_IF_SKIP(InitState(nullptr, &indexing_features));
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBindingFlagsEXT binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &binding_flags;

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}},
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &flags_create_info,
                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorImageInfo image_info = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_GENERAL};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewType-07752");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DrawTimeImageMultisampleMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when a multisampled images are consumed via singlesample images types in the shader, or "
        "vice versa.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2DMS s;
        layout(location=0) out vec4 color;
        void main() {
           color = texelFetch(s, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo image_info = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_GENERAL};

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(image_info);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDescriptorSet set_obj = descriptorSet.GetDescriptorSetHandle();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.GetPipelineLayout(), 0, 1,
                              &set_obj, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-samples-08726");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DrawTimeImageComponentTypeMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the component type of an imageview disagrees with the type in the shader.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform isampler2D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texelFetch(s, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo image_info = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_GENERAL};

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(image_info);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDescriptorSet set_obj = descriptorSet.GetDescriptorSetHandle();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.GetPipelineLayout(), 0, 1,
                              &set_obj, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-format-07753");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageLayerCountMismatch) {
    TEST_DESCRIPTION(
        "Try to copy between images with the source subresource having a different layerCount than the destination subresource");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool maintenance1 =
        IsExtensionsEnabled(VK_KHR_MAINTENANCE_1_EXTENSION_NAME) || DeviceValidationVersion() >= VK_API_VERSION_1_1;
    RETURN_IF_SKIP(InitState())

    VkFormat image_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_format, &format_props);
    if ((format_props.optimalTilingFeatures & (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) == 0) {
        GTEST_SKIP() << "Transfer for format is not supported";
    }

    // Create two images to copy between
    VkImageObj src_image_obj(m_device);
    VkImageObj dst_image_obj(m_device);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    const char *vuid = (maintenance1 == true) ? "VUID-vkCmdCopyImage-srcImage-08793" : "VUID-VkImageCopy-apiVersion-07941";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image_obj.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image_obj.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CompressedImageMipCopy) {
    TEST_DESCRIPTION("Image/Buffer copies for higher mip levels");

    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    VkFormat compressed_format = VK_FORMAT_UNDEFINED;
    if (device_features.textureCompressionBC) {
        compressed_format = VK_FORMAT_BC3_SRGB_BLOCK;
    } else if (device_features.textureCompressionETC2) {
        compressed_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    } else if (device_features.textureCompressionASTC_LDR) {
        compressed_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    } else {
        GTEST_SKIP() << "No compressed formats supported - CompressedImageMipCopyTests skipped";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
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
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_1024(*m_device, 1024, transfer_usage);
    vkt::Buffer buffer_64(*m_device, 64, transfer_usage);
    vkt::Buffer buffer_16(*m_device, 16, transfer_usage);
    vkt::Buffer buffer_8(*m_device, 8, transfer_usage);

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
    mem_barriers[0] = vku::InitStructHelper();
    mem_barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[1] = vku::InitStructHelper();
    mem_barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2] = vku::InitStructHelper();
    mem_barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // Mip level copies that work - 5 levels

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-00207");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-00207");  // width not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Copy height < compressed block size but not the full mip height
    region.imageExtent = {2, 1, 1};
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-00208");  // height not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-00208");  // height not a multiple of compressed block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Offsets must be multiple of compressed block size
    region.imageOffset = {1, 1, 0};
    region.imageExtent = {1, 1, 1};
    // imageOffset not a multiple of block size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-07274");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-07275");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // imageOffset not a multiple of block size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07274");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07275");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        // imageOffset not a multiple of block size
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-dstImage-07274");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-dstImage-07275");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage2-imageOffset-07738");  // image transfer granularity
        vk::CmdCopyBufferToImage2KHR(m_commandBuffer->handle(), &copy_buffer_to_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Offset + extent width = mip width - should succeed
    region.imageOffset = {4, 4, 0};
    region.imageExtent = {3, 4, 1};
    region.imageSubresource.mipLevel = 2;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1,
                             &region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);

    // Offset + extent width < mip width and not a multiple of block width - should fail
    region.imageExtent = {3, 3, 1};
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-00208");  // offset+extent not a multiple of block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-00208");  // offset+extent not a multiple of block width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ImageBufferCopy) {
    TEST_DESCRIPTION("Image to buffer and buffer to image tests");

    // Enable KHR multiplane req'd extensions for multi-planar copy tests
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    // Bail if any dimension of transfer granularity is 0.
    auto index = m_device->graphics_queue_node_index_;
    auto queue_family_properties = m_device->phy().queue_properties_;
    if ((queue_family_properties[index].minImageTransferGranularity.depth == 0) ||
        (queue_family_properties[index].minImageTransferGranularity.width == 0) ||
        (queue_family_properties[index].minImageTransferGranularity.height == 0)) {
        GTEST_SKIP() << "Subresource copies are disallowed when xfer granularity (x|y|z) is 0";
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
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_256k(*m_device, 262144, transfer_usage);  // 256k
    vkt::Buffer buffer_128k(*m_device, 131072, transfer_usage);  // 128k
    vkt::Buffer buffer_64k(*m_device, 65536, transfer_usage);    // 64k
    vkt::Buffer buffer_16k(*m_device, 16384, transfer_usage);    // 16k

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 0;

    VkMemoryBarrier mem_barriers[3];
    mem_barriers[0] = vku::InitStructHelper();
    mem_barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[1] = vku::InitStructHelper();
    mem_barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2] = vku::InitStructHelper();
    mem_barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    if (missing_rgba_support) {
        printf("R8G8B8A8_UINT transfer unsupported - skipping RGBA tests.\n");

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

        // image/buffer too small (extent too large) on copy to image
        region.imageExtent = {65, 64, 1};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-pRegions-00171");  // buffer too small
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-imageSubresource-07970");  // image too small
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

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07972");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyBufferToImage-imageSubresource-07970");  // image too small
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
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07972");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyImageToBuffer-imageSubresource-07970");  // image too small
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
                                                 "VUID-VkBufferImageCopy-aspectMask-09103");  // more than 1 aspect bit set
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_depth.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit,
                "VUID-vkCmdCopyImageToBuffer-imageSubresource-09105");  // different mis-matched aspect
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_depth.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdCopyImageToBuffer-imageSubresource-09105");  // mis-matched aspect
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // Out-of-range mip levels should fail
        region.imageSubresource.mipLevel = image_16k.create_info().mipLevels + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageSubresource-07967");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07971");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07972");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageOffset-09104");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyImageToBuffer-imageSubresource-07970");  // unavoidable "region exceeds image bounds" for non-existent
                                                                    // mip
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageSubresource-07967");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07972");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageOffset-09104");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdCopyBufferToImage-imageSubresource-07970");  // unavoidable "region exceeds image bounds" for non-existent
                                                                    // mip
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        region.imageSubresource.mipLevel = 0;

        // Out-of-range array layers should fail
        region.imageSubresource.baseArrayLayer = image_16k.create_info().arrayLayers;
        region.imageSubresource.layerCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-imageSubresource-07968");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                                 &region);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageSubresource-07968");
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
        printf("Depth / Stencil formats unsupported - skipping D/S tests.\n");
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
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_256k.handle(), 1, &ds_region);

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_256k.handle(), 1, &ds_region);

        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_128k.handle(), 1, &ds_region);

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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-07978");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_128k.handle(), 1, &ds_region);
        m_errorMonitor->VerifyFound();
        ds_region.imageExtent = {256, 256, 1};

        // Stencil copies that should succeed
        ds_region.bufferOffset = 0;
        ds_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &mem_barriers[0], 0, nullptr, 0, nullptr);
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), ds_image_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 buffer_64k.handle(), 1, &ds_region);

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
    GetPhysicalDeviceFeatures(&device_features);
    if (!(device_features.textureCompressionBC || device_features.textureCompressionETC2 ||
          device_features.textureCompressionASTC_LDR)) {
        printf("No compressed formats supported - block compression tests skipped.\n");
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
            printf("No compressed formats transfers bits are supported - block compression tests skipped.\n");
        } else {
            ASSERT_TRUE(image_16k_4x4comp.initialized());
            // Just fits
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                                   &mem_barriers[0], 0, nullptr, 0, nullptr);
            region.imageExtent = {128, 128, 1};
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);

            // with offset, too big for buffer
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-pRegions-00183");
            region.bufferOffset = 16;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
            region.bufferOffset = 0;

            // extents that are not a multiple of compressed block size
            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-00207");  // extent width not a multiple of block size
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
            region.imageExtent.width = 66;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
            region.imageExtent.width = 128;

            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-00208");  // extent height not a multiple of block size
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
            region.imageExtent.height = 2;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
            region.imageExtent.height = 128;

            // TODO: All available compressed formats are 2D, with block depth of 1. Unable to provoke VU_01277.

            // non-multiple extents are allowed if at the far edge of a non-block-multiple image - these should pass
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
            region.imageOffset = {0, 0, 0};

            // buffer offset must be a multiple of texel block size (16)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-srcImage-07975");
            region.imageExtent = {64, 64, 1};
            region.bufferOffset = 24;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_16k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();

            // rowlength not a multiple of block width (4)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09106");
            region.bufferOffset = 0;
            region.bufferRowLength = 130;
            region.bufferImageHeight = 0;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_64k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();

            // imageheight not a multiple of block height (4)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-09107");
            region.bufferRowLength = 0;
            region.bufferImageHeight = 130;
            vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                     buffer_64k.handle(), 1, &region);
            m_errorMonitor->VerifyFound();
        }
    }

    // Test multi-planar formats, if supported
    if (!mp_extensions) {
        printf("multi-planar extensions not supported; skipped.\n");
    } else {
        // Try to use G8_B8R8_2PLANE_420_UNORM because need 2-plane format for some tests and likely supported due to copy support
        // being required with samplerYcbcrConversion feature
        bool missing_mp_support = false;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &props);
        missing_mp_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
        missing_mp_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
        missing_mp_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

        if (missing_mp_support) {
            printf("VK_FORMAT_G8_B8R8_2PLANE_420_UNORM transfer not supported; skipped.\n");
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
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                                   &mem_barriers[2], 0, nullptr, 0, nullptr);
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);

            // uses plane_2 without being 3 planar format
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07981");
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyFound();

            // uses single-plane aspect mask
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07981");
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyFound();

            // buffer offset must be a multiple of texel block size for VK_FORMAT_R8G8_UNORM (2)
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07976");
            mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
            mp_region.bufferOffset = 5;
            mp_region.imageExtent = {8, 8, 1};
            vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_16k.handle(), image_multi_planar.handle(),
                                     VK_IMAGE_LAYOUT_GENERAL, 1, &mp_region);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeCommand, MiscImageLayer) {
    TEST_DESCRIPTION("Image-related tests that don't belong elsewhere");

    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R16G16B16A16_UINT features not supported";
    } else if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8G8_UNORM features not supported";
    }

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);  // 64bpp
    vkt::Buffer buffer(*m_device, 128 * 128 * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0);
    ASSERT_TRUE(image.initialized());
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
    vkt::Buffer buffer2(*m_device, 128 * 128 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0);
    m_commandBuffer->begin();

    // Image must have offset.z of 0 and extent.depth of 1
    // Introduce failure by setting imageExtent.depth to 0
    region.imageExtent.depth = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07980");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageExtent.depth = 1;

    // Image must have offset.z of 0 and extent.depth of 1
    // Introduce failure by setting imageOffset.z to 4
    // Note: Also (unavoidably) triggers 'region exceeds image' #1228
    region.imageOffset.z = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07980");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-09104");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageSubresource-07970");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageOffset.z = 0;
    // BufferOffset must be a multiple of the calling command's VkImage parameter's texel size
    // Introduce failure by setting bufferOffset to 1 and 1/2 texels
    region.bufferOffset = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07975");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
    region.bufferOffset = 0;
    region.imageExtent.height = 128;
    region.imageExtent.width = 128;
    // Introduce failure by setting bufferRowLength > 0 but less than width
    region.bufferRowLength = 64;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-bufferRowLength-09101");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
    region.bufferRowLength = 128;
    // Introduce failure by setting bufferRowHeight > 0 but less than height
    region.bufferImageHeight = 64;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferImageCopy-bufferImageHeight-09102");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CopyImageTypeExtentMismatch) {
    TEST_DESCRIPTION("Image copy tests where format type and extents don't match");
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    // Tests are designed to run without Maintenance1 which was promoted in 1.1
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
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
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // 1D texture w/ offset.y > 0. Source = VU 09c00124, dest = 09c00130
    copy_region.srcOffset.y = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");  // also y-dim overrun
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    copy_region.srcOffset.y = 0;
    copy_region.dstOffset.y = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-00152");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");  // also y-dim overrun
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    copy_region.dstOffset.y = 0;

    // 1D texture w/ extent.height > 1. Source = VU 09c00124, dest = 09c00130
    copy_region.extent.height = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-00146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");  // also y-dim overrun
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-00152");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");  // also y-dim overrun
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    copy_region.extent.height = 1;

    // 1D texture w/ offset.z > 0. Source = VU 09c00df2, dest = 09c00df4
    copy_region.srcOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01785");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset.z = 0;
    copy_region.dstOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01786");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
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
                                         "VUID-vkCmdCopyImage-apiVersion-08969");             // 2D needs to be 1 pre-Vulkan 1.1
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01786");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun (src)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-apiVersion-08969");             // 2D needs to be 1 pre-Vulkan 1.1
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_1D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.depth = 1;

    // 2D texture w/ offset.z > 0. Source = VU 09c00df6, dest = 09c00df8
    copy_region.extent = {16, 16, 1};
    copy_region.srcOffset.z = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01787");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-00147");  // also z-dim overrun (src)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D.image(), VK_IMAGE_LAYOUT_GENERAL, image_3D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset.z = 0;
    copy_region.dstOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01788");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-00153");  // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset.z = 0;

    // 3D texture accessing an array layer other than 0. VU 09c0011a
    copy_region.extent = {4, 4, 1};
    copy_region.srcSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07932");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcSubresource-07968");         // also 'too many layers'
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_commandBuffer->handle(), image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcSubresource.baseArrayLayer = 0;

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageTypeExtentMismatchMaintenance1) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo ci = vku::InitStructHelper();
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
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-07743");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Copy from layer not present
    copy_region.srcSubresource.baseArrayLayer = 4;
    copy_region.srcSubresource.layerCount = 6;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01791");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcSubresource-07968");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, image_3D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;

    // Copy to layer not present
    copy_region.dstSubresource.baseArrayLayer = 1;
    copy_region.dstSubresource.layerCount = 8;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01792");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstSubresource-07968");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.layerCount = 1;

    // both 2D and extent.depth not 1
    // Need two 2D array images to prevent other errors
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01790");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array_2.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    // 2D src / 3D dst and depth not equal to src layerCount
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01791");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-08793");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2D_array.image(), VK_IMAGE_LAYOUT_GENERAL, image_3D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    // 3D src / 2D dst and depth not equal to dst layerCount
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-01792");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-08793");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_3D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageCompressedBlockAlignment) {
    // Image copy tests on compressed images with block alignment errors
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    // Select a compressed format and verify support
    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    VkFormat compressed_format = VK_FORMAT_UNDEFINED;
    if (device_features.textureCompressionBC) {
        compressed_format = VK_FORMAT_BC3_SRGB_BLOCK;
    } else if (device_features.textureCompressionETC2) {
        compressed_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    } else if (device_features.textureCompressionASTC_LDR) {
        compressed_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
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
        GTEST_SKIP() << "No compressed formats supported - CopyImageCompressedBlockAlignment skipped";
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
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Sanity check
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);

    std::string vuid;
    bool ycbcr =
        (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    // Src, Dest offsets must be multiples of compressed block sizes {4, 4, 1}
    // Image transfer granularity gets set to compressed block size, so an ITG error is also (unavoidably) triggered.
    copy_region.srcOffset = {2, 4, 0};  // source width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-pRegions-07278");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {12, 1, 0};  // source height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-pRegions-07279");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {0, 0, 0};

    copy_region.dstOffset = {1, 0, 0};  // dest width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-pRegions-07281");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dstOffset image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {4, 1, 0};  // dest height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-pRegions-07282");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dstOffset image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {0, 0, 0};

    // Copy extent must be multiples of compressed block sizes {4, 4, 1} if not full width/height
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01728" : "VUID-vkCmdCopyImage-srcImage-01728";
    copy_region.extent = {62, 60, 1};  // source width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01729" : "VUID-vkCmdCopyImage-srcImage-01729";
    copy_region.extent = {60, 62, 1};  // source height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1.image(), VK_IMAGE_LAYOUT_GENERAL, image_2.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();

    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01732" : "VUID-vkCmdCopyImage-dstImage-01732";
    copy_region.extent = {62, 60, 1};  // dest width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2.image(), VK_IMAGE_LAYOUT_GENERAL, image_1.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
    copy_region.extent = {60, 62, 1};  // dest height
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    vk::CmdCopyImage(m_commandBuffer->handle(), image_2.image(), VK_IMAGE_LAYOUT_GENERAL, image_1.image(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Note: "VUID-vkCmdCopyImage-srcImage-01730", "VUID-vkCmdCopyImage-dstImage-01734", "VUID-vkCmdCopyImage-srcImage-01730",
    // "VUID-vkCmdCopyImage-dstImage-01734"
    //       There are currently no supported compressed formats with a block depth other than 1,
    //       so impossible to create a 'not a multiple' condition for depth.
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageSrcSizeExceeded) {
    // Image copy with source region specified greater than src image size
    RETURN_IF_SKIP(Init())

    // Create images with full mip chain
    VkImageCreateInfo ci = vku::InitStructHelper();
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
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Source exceeded in x-dim
    copy_region.srcOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00144");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in x-dim in negative direction (since offset is a signed in)
    copy_region.extent.width = 4;
    copy_region.srcOffset.x = -8;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00144");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.width = 32;

    // Source exceeded in y-dim
    copy_region.srcOffset.x = 0;
    copy_region.extent.height = 48;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Source exceeded in z-dim
    copy_region.extent = {4, 4, 4};
    copy_region.srcSubresource.mipLevel = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00147");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageDstSizeExceeded) {
    // Image copy with dest region specified greater than dest image size
    RETURN_IF_SKIP(Init())

    // Create images with full mip chain
    VkImageCreateInfo ci = vku::InitStructHelper();
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
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Dest exceeded in x-dim
    copy_region.dstOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in x-dim in negative direction (since offset is a signed in)
    copy_region.extent.width = 4;
    copy_region.dstOffset.x = -8;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.width = 32;

    copy_region.dstOffset.x = 0;
    copy_region.extent.height = 48;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in z-dim
    copy_region.extent = {4, 4, 4};
    copy_region.dstSubresource.mipLevel = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00153");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageZeroSize) {
    TEST_DESCRIPTION("Image Copy with empty regions");
    RETURN_IF_SKIP(Init())

    // Create images with full mip chain
    VkImageCreateInfo ci = vku::InitStructHelper();
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

    // large enough for image
    vkt::Buffer buffer(*m_device, 16384, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    copy_region.extent = {4, 4, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-06670");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.extent = {0, 0, 4};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-06668");  // width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCopy-extent-06669");  // height
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
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

TEST_F(NegativeCommand, CopyImageMultiPlaneSizeExceeded) {
    TEST_DESCRIPTION("Image Copy for multi-planar format that exceed size of plane for both src and dst");

    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    // Try to use VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM because need multi-plane format for some tests and likely supported due to
    // copy support being required with samplerYcbcrConversion feature
    VkFormatProperties props = {0, 0, 0};
    bool missing_format_support = false;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, &props);
    missing_format_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_format_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_format_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (missing_format_support == true) {
        GTEST_SKIP() << "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM transfer not supported";
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
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Should be able to do a 64x64 copy from plane 0 -> Plane 0
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // Should be able to do a 64x64 copy from plane 0 -> Plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Should be able to do a 64x64 copy from plane 0 -> Plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Should be able to do a 128x64 copy from plane 0 -> Plane 0
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // 128x64 copy from plane 0 -> Plane 1
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // 128x64 copy from plane 1 -> Plane 0
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00144");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // src exceeded in y-dim from offset
    copy_region = original_region;
    copy_region.srcOffset.y = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcOffset-00145");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // dst exceeded in y-dim from offset
    copy_region = original_region;
    copy_region.dstOffset.y = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstOffset-00151");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.image(), VK_IMAGE_LAYOUT_GENERAL, dst_image.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageFormatSizeMismatch) {
    // Enable KHR multiplane req'd extensions
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR mp_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(mp_features);

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UNORM features not supported";
    } else if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UINT features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    if (FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
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
        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_uint.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01548");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_32b_unorm.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        // Swap src and dst
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01548");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_32b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }

    // DstImage is a mismatched plane of a multi-planar format
    if (!mp_features.samplerYcbcrConversion) {
        printf("No multi-planar support; section of tests skipped.\n");
    } else if (FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                          VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
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
        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        image_8b_16b_420_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);

        // Make sure no false postiives if Compatible format
        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_uint.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        // Plane 1 is VK_FORMAT_R8G8_UNORM so this should fail
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-None-01549");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        // Same tests but swap src and dst
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        image_8b_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                          VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_8b_16b_420_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);

        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        image_8b_16b_420_unorm.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);

        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_8b_uint.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-None-01549");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeCommand, CopyImageDepthStencilFormatMismatch) {
    RETURN_IF_SKIP(Init())
    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    VkFormatProperties properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D32_SFLOAT, &properties);
    if (properties.optimalTilingFeatures == 0) {
        GTEST_SKIP() << "Image format not supported";
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01548");
    vk::CmdCopyImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CopyImageSampleCountMismatch) {
    TEST_DESCRIPTION("Image copies with sample count mis-matches");

    RETURN_IF_SKIP(Init())

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0,
                                               &image_format_properties);

    if ((0 == (VK_SAMPLE_COUNT_2_BIT & image_format_properties.sampleCounts)) ||
        (0 == (VK_SAMPLE_COUNT_4_BIT & image_format_properties.sampleCounts))) {
        GTEST_SKIP() << "Image multi-sample support not found";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
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

TEST_F(NegativeCommand, CopyImageLayerCount) {
    TEST_DESCRIPTION("Check layerCount in vkCmdCopyImage");

    RETURN_IF_SKIP(Init())

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image(m_device);
    image.init(&ci);

    m_commandBuffer->begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 0;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = copyRegion.srcSubresource;
    copyRegion.dstOffset = {32, 32, 0};
    copyRegion.extent = {16, 16, 1};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-01700");  // src
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-01700");  // dst
    vk::CmdCopyImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copyRegion);
    m_errorMonitor->VerifyFound();

    copyRegion.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copyRegion.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-09243");  // src
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-09243");  // dst
    vk::CmdCopyImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copyRegion);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageAspectMismatch) {
    TEST_DESCRIPTION("Image copies with aspect mask errors");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    auto ds_format = FindSupportedDepthStencilFormat(gpu());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(gpu(), ds_format, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required Depth/Stencil Format features not supported";
    }

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-recording");
    vk::CmdCopyImage(m_commandBuffer->handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Src and dest aspect masks don't match
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01551");
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();
    // same test for dstSubresource
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;  // color must be alone
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00167");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00143");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Metadata aspect is illegal
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00168");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();
    // same test for dstSubresource
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-00168");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Aspect Memory Plane mask is illegal
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-aspectMask-02247");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Aspect mask doesn't match source image format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00142");
    // Again redundant but unavoidable
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01548");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Aspect mask doesn't match dest image format
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspectMask-00143");
    // Again redundant but unavoidable
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-01548");
    vk::CmdCopyImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Check no performance warnings regarding layout are thrown when copying from and to the same image
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdCopyImage(m_commandBuffer->handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ResolveImageLowSampleCount) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-00257");

    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of sample count 1 and try to Resolve between them

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ResolveImageHighSampleCount) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00259");

    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of sample count 4 and try to Resolve between them

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ResolveImageFormatMismatch) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-01386");

    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ResolveImageLayoutMismatch) {
    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vk::CmdClearColorImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1,
                           &subresource);
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
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage.image(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    // dst image layout mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImageLayout-00262");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ResolveInvalidSubresource) {
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vk::CmdClearColorImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1,
                           &subresource);
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
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        vk::CmdResolveImage2KHR(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.srcSubresource.mipLevel = 0;
    // invalid dest mip level
    resolveRegion.dstSubresource.mipLevel = image_create_info.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstSubresource-01710");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        vk::CmdResolveImage2KHR(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.dstSubresource.mipLevel = 0;
    // invalid source array layer range
    resolveRegion.srcSubresource.baseArrayLayer = image_create_info.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcSubresource-01711");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        vk::CmdResolveImage2KHR(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.srcSubresource.baseArrayLayer = 0;
    // invalid dest array layer range
    resolveRegion.dstSubresource.baseArrayLayer = image_create_info.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstSubresource-01712");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
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
        vk::CmdResolveImage2KHR(m_commandBuffer->handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.dstSubresource.baseArrayLayer = 0;

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ResolveImageImageType) {
    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create images of different types and try to resolve between them
    VkImageObj srcImage2D(m_device);
    VkImageObj dstImage1D(m_device);
    VkImageObj dstImage3D(m_device);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    // layerCount is not 1
    resolveRegion.srcSubresource.layerCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageResolve-layerCount-08803");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-04446");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage3D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcSubresource.layerCount = 1;

    // Set height with 1D dstImage
    resolveRegion.extent.height = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00276");
    // Also exceed height of both images
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00270");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00275");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage1D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.extent.height = 1;

    // Set depth with 1D dstImage and 2D srcImage
    resolveRegion.extent.depth = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00278");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-00273");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage1D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.extent.depth = 1;

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ResolveImageSizeExceeded) {
    TEST_DESCRIPTION("Resolve Image with subresource region greater than size of src/dst image");
    RETURN_IF_SKIP(Init())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageObj srcImage2D(m_device);
    VkImageObj dstImage2D(m_device);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);

    // srcImage exceeded in x-dim
    resolveRegion.srcOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00269");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.x = 0;

    // dstImage exceeded in x-dim
    resolveRegion.dstOffset.x = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00274");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.x = 0;

    // both image exceeded in y-dim
    resolveRegion.srcOffset.y = 32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00270");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.y = 0;

    resolveRegion.dstOffset.y = 32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00275");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.y = 0;

    // srcImage exceeded in z-dim
    resolveRegion.srcOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcOffset-00272");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-00273");  // because it's a 2d image
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.z = 0;

    // dstImage exceeded in z-dim
    resolveRegion.dstOffset.z = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstOffset-00277");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-00278");  // because it's a 2d image
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2D.image(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.image(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.z = 0;

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearImage) {
    TEST_DESCRIPTION("Call ClearColorImage w/ a depth|stencil image and ClearDepthStencilImage with a color image.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    m_commandBuffer->begin();

    // Color image
    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);
    const VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t img_width = 32;
    const int32_t img_height = 32;
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vkt::Image color_image_no_transfer;
    color_image_no_transfer.init(*m_device, image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image color_image;
    color_image.init(*m_device, image_create_info);

    const VkImageSubresourceRange color_range = vkt::Image::subresource_range(image_create_info, VK_IMAGE_ASPECT_COLOR_BIT);

    // Depth/Stencil image
    VkClearDepthStencilValue clear_value = {0};
    VkImageCreateInfo ds_image_create_info = vkt::Image::create_info();
    ds_image_create_info.imageType = VK_IMAGE_TYPE_2D;
    ds_image_create_info.format = VK_FORMAT_D16_UNORM;
    ds_image_create_info.extent.width = 64;
    ds_image_create_info.extent.height = 64;
    ds_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    ds_image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    vkt::Image ds_image;
    ds_image.init(*m_device, ds_image_create_info);

    const VkImageSubresourceRange ds_range = vkt::Image::subresource_range(ds_image_create_info, VK_IMAGE_ASPECT_DEPTH_BIT);

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

TEST_F(NegativeCommand, CommandQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics and try to issue a graphics-only command");

    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> queueFamilyIndex = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!queueFamilyIndex) {
        GTEST_SKIP() << "Non-graphics queue family not found";
    }

    // Create command pool on a non-graphics queue
    vkt::CommandPool command_pool(*m_device, queueFamilyIndex.value());

    // Setup command buffer on pool
    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    // Issue a graphics only command
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-commandBuffer-cmdpool");
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(command_buffer.handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DepthStencilImageCopyNoGraphicsQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics and try to issue a depth/stencil image copy to "
        "buffer");

    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> no_gfx =
        m_device->QueueFamilyMatching(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, false);
    if (!no_gfx) {
        GTEST_SKIP() << "Non-graphics queue family not found";
    }

    // Create Depth image
    const VkFormat ds_format = FindSupportedDepthOnlyFormat(gpu());

    VkImageObj ds_image(m_device);
    ds_image.Init(64, 64, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                  VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(ds_image.initialized());

    // 256k to have more then enough to copy
    vkt::Buffer buffer(*m_device, 262144, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 0;

    // Create command pool on a non-graphics queue
    vkt::CommandPool command_pool(*m_device, no_gfx.value());

    // Setup command buffer on pool
    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-07739");
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), ds_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ImageCopyTransferQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics/compute and try to issue an invalid image copy to "
        "buffer");

    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> transfer =
        m_device->QueueFamilyMatching(VK_QUEUE_TRANSFER_BIT, (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT));
    if (!transfer) {
        GTEST_SKIP() << "Non-graphics/compute queue family not found";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    // 256k to have more then enough to copy
    vkt::Buffer buffer(*m_device, 262144, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {16, 16, 1};
    region.bufferOffset = 5;

    // Create command pool on a non-graphics queue
    vkt::CommandPool command_pool(*m_device, transfer.value());

    // Setup command buffer on pool
    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-07975");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-07737");
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ExecuteDiffertQueueFlagsSecondaryCB) {
    TEST_DESCRIPTION("Allocate a command buffer from two different queues and try to use a secondary command buffer");

    RETURN_IF_SKIP(Init())

    if (m_device->phy().queue_properties_.size() < 2) {
        GTEST_SKIP() << "Need 2 different queues for testing skipping.";
    }

    // First two queue families
    uint32_t queue_index_a = 0;
    uint32_t queue_index_b = 1;

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.flags = 0;

    pool_create_info.queueFamilyIndex = queue_index_a;
    vkt::CommandPool command_pool_a(*m_device, pool_create_info);
    ASSERT_TRUE(command_pool_a.initialized());

    pool_create_info.queueFamilyIndex = queue_index_b;
    vkt::CommandPool command_pool_b(*m_device, pool_create_info);
    ASSERT_TRUE(command_pool_b.initialized());

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.commandPool = command_pool_a.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkt::CommandBuffer command_buffer_primary(*m_device, command_buffer_allocate_info);
    ASSERT_TRUE(command_buffer_primary.initialized());

    command_buffer_allocate_info.commandPool = command_pool_b.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    vkt::CommandBuffer command_buffer_secondary(*m_device, command_buffer_allocate_info);

    VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper();
    cmdbuff_ii.renderPass = m_renderPass;
    cmdbuff_ii.subpass = 0;
    cmdbuff_ii.framebuffer = m_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
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

TEST_F(NegativeCommand, ExecuteUnrecordedSecondaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a CB in the initial state");
    RETURN_IF_SKIP(Init())
    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    // never record secondary

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00089");
    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ExecuteSecondaryCBWithLayoutMismatch) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a CB with incorrect initial layout.");

    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    auto pipeline = [&image_barrier](const vkt::CommandBuffer &cb, VkImageLayout old_layout, VkImageLayout new_layout) {
        image_barrier.oldLayout = old_layout;
        image_barrier.newLayout = new_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
    };

    // Validate that mismatched use of image layout in secondary command buffer is caught at record time
    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
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
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, RenderPassScopeSecondaryCmdBuffer) {
    TEST_DESCRIPTION(
        "Test secondary buffers executed in wrong render pass scope wrt VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer sec_cmdbuff_inside_rp(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer sec_cmdbuff_outside_rp(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

    VkRenderPassBeginInfo rp_bi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, m_renderPass, m_framebuffer, VkRect2D{{0, 0}, {32u, 32u}},
                                             static_cast<uint32_t>(m_renderPassClearValues.size()), m_renderPassClearValues.data());
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00096");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &sec_cmdbuff_outside_rp.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, SecondaryCommandBufferClearColorAttachmentsRenderArea) {
    TEST_DESCRIPTION(
        "Create a secondary command buffer with CmdClearAttachments call that has a rect outside of renderPass renderArea");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_command_buffer(*m_device, command_buffer_allocate_info);
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = vku::InitStructHelper();
    command_buffer_inheritance_info.renderPass = m_renderPass;
    command_buffer_inheritance_info.framebuffer = m_framebuffer;

    VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    secondary_command_buffer.begin(&command_buffer_begin_info);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    // x extent of 257 exceeds render area of 256
    VkClearRect clear_rect = {{{0, 0}, {257, 32}}, 0, 1};
    vk::CmdClearAttachments(secondary_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary_command_buffer.end();
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00016");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_command_buffer.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, MultiDraw) {
    TEST_DESCRIPTION("Test validation of multi_draw extension");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(multi_draw_features);

    VkPhysicalDeviceMultiDrawPropertiesEXT multi_draw_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multi_draw_properties);

    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));
    InitRenderTarget();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-08606");
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-07312");  // missing index buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-08606");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    multi_draw_indices[2].indexCount = 511;
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 2, VK_INDEX_TYPE_UINT16);
    // This first should be fine
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    // Fail with index offset
    multi_draw_indices[2].firstIndex = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    // Fail with index count
    multi_draw_indices[2].firstIndex = 0;
    multi_draw_indices[2].indexCount = 512;
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
    multi_draw_indices[2].indexCount = 1;

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-stride-04936");
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT) + 1);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-stride-04941");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT) + 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-drawCount-04935");
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, nullptr, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, nullptr, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    if (multi_draw_properties.maxMultiDrawCount < vvl::kU32Max) {
        uint32_t draw_count = multi_draw_properties.maxMultiDrawCount + 1;
        std::vector<VkMultiDrawInfoEXT> max_multi_draws(draw_count);
        std::vector<VkMultiDrawIndexedInfoEXT> max_multi_indexed_draws(draw_count);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-drawCount-04934");
        vk::CmdDrawMultiEXT(m_commandBuffer->handle(), draw_count, max_multi_draws.data(), 1, 0, sizeof(VkMultiDrawInfoEXT));
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939");
        vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), draw_count, max_multi_indexed_draws.data(), 1, 0,
                                   sizeof(VkMultiDrawIndexedInfoEXT), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCommand, MultiDrawMaintenance5) {
    TEST_DESCRIPTION("Test validation of multi_draw extension with VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper(&maintenance5_features);
    GetPhysicalDeviceFeatures2(multi_draw_features);
    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // Use non-power-of-2 size to
    vkt::Buffer buffer(*m_device, 2048, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 514, 0};  // overflow

    // same as calling vkCmdBindIndexBuffer (size of the buffer creation)
    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 2, 1024, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    multi_draw_indices.indexCount = 256;  // only uses [0 - 512]
    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 2, 508, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawWholeSizeMaintenance5) {
    TEST_DESCRIPTION("Test validation of multi_draw extension with VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper(&maintenance5_features);
    GetPhysicalDeviceFeatures2(multi_draw_features);
    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // Use non-power-of-2 size to
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 514, 0};  // overflow

    // VK_WHOLE_SIZE also full size of the buffer
    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 2, VK_WHOLE_SIZE, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawMaintenance5Mixed) {
    TEST_DESCRIPTION("Test vkCmdBindIndexBuffer2KHR with vkCmdBindIndexBuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper(&maintenance5_features);
    GetPhysicalDeviceFeatures2(multi_draw_features);
    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 511, 0};

    // valid
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    // should be overwritten with smaller size
    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 0, 1000, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawFeatures) {
    TEST_DESCRIPTION("Test validation of multi draw feature enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-04933");
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-04937");
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IndirectDraw) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirect and vkCmdDrawIndexedIndirect");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    if (m_device->phy().features().multiDrawIndirect == VK_FALSE) {
        GTEST_SKIP() << "multiDrawIndirect feature is disabled";
    }
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    vkt::Buffer draw_buffer(*m_device, buffer_create_info);

    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer_correct(*m_device, buffer_create_info);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-buffer-02709");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-drawCount-00488");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer_correct.handle(), 0, 2, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), draw_buffer_correct.handle(), 0, 2, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-offset-02710");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), draw_buffer_correct.handle(), 2, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, MultiDrawIndirectFeature) {
    TEST_DESCRIPTION("use vkCmdDrawIndexedIndirect without MultiDrawIndirect");

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    features.multiDrawIndirect = VK_FALSE;
    RETURN_IF_SKIP(InitState(&features));
    InitRenderTarget();

    if (m_device->phy().limits_.maxDrawIndirectCount < 2) {
        GTEST_SKIP() << "maxDrawIndirectCount is too low";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer draw_buffer(*m_device, sizeof(VkDrawIndirectCommand) * 3, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 0, sizeof(VkDrawIndexedIndirectCommand));
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-drawCount-02718");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 2, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DrawIndirectCountKHR) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirectCountKHR");

    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer;
    draw_buffer.init_no_mem(*m_device, buffer_create_info);
    ASSERT_TRUE(draw_buffer.initialized());

    VkDeviceSize count_buffer_size = 128;
    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = count_buffer_size;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-buffer-02708");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    draw_buffer.allocate_and_bind_memory(*m_device);

    vkt::Buffer count_buffer_unbound;
    count_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(count_buffer_unbound.initialized());

    vkt::Buffer count_buffer_wrong;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    count_buffer_wrong.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02714");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " VUID-vkCmdDrawIndirectCount-countBuffer-02715");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_wrong.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-offset-02710");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 1, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBufferOffset-02716");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 1, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBufferOffset-04129");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), count_buffer_size, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-stride-03110");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DrawIndexedIndirectCountKHR) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndexedIndirectCountKHR");

    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info);

    VkDeviceSize count_buffer_size = 128;
    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = count_buffer_size;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info);

    VkBufferCreateInfo index_buffer_create_info = vku::InitStructHelper();
    index_buffer_create_info.size = sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer index_buffer(*m_device, index_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-07312");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    vkt::Buffer draw_buffer_unbound;
    draw_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(draw_buffer_unbound.initialized());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer_unbound.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    vkt::Buffer count_buffer_unbound;
    count_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(count_buffer_unbound.initialized());

    vkt::Buffer count_buffer_wrong;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    count_buffer_wrong.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_wrong.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-offset-02710");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 1, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 1, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-04129");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), count_buffer_size,
                                       1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142");
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DrawIndirectCountFeature) {
    TEST_DESCRIPTION("Test covered valid usage for the 1.2 drawIndirectCount feature");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
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

TEST_F(NegativeCommand, ExclusiveScissorNV) {
    TEST_DESCRIPTION("Test VK_NV_scissor_exclusive with multiViewport disabled.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Create a device that enables exclusive scissor but disables multiViewport
    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusive_scissor_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(exclusive_scissor_features);
    features2.features.multiViewport = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    if (m_device->phy().limits_.maxViewports <= 1) {
        GTEST_SKIP() << "Device doesn't support multiple viewports";
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
                vku::InitStructHelper();

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
        const VkRect2D scissor = {{0, 0}, {16, 16}};
        const VkRect2D scissors[] = {scissor, scissor};

        m_commandBuffer->begin();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 1, 1, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-arraylength");
        vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 0, nullptr);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036");
        vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 2, scissors);
        m_errorMonitor->VerifyFound();

        // This VU gets triggered before VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-arraylength");
        vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 1, 0, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036");
        vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 1, 2, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetExclusiveScissorNV-pExclusiveScissors-parameter");
        vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 1, nullptr);
        m_errorMonitor->VerifyFound();

        struct TestCase {
            VkRect2D scissor;
            std::string vuid;
        };

        std::vector<TestCase> test_cases = {
            {{{-1, 0}, {16, 16}}, "VUID-vkCmdSetExclusiveScissorNV-x-02037"},
            {{{0, -1}, {16, 16}}, "VUID-vkCmdSetExclusiveScissorNV-x-02037"},
            {{{1, 0}, {vvl::kI32Max, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{vvl::kI32Max, 0}, {1, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{0, 0}, {uint32_t{vvl::kI32Max} + 1, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{0, 1}, {16, vvl::kI32Max}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"},
            {{{0, vvl::kI32Max}, {16, 1}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"},
            {{{0, 0}, {16, uint32_t{vvl::kI32Max} + 1}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"}};

        for (const auto &test_case : test_cases) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.vuid);
            vk::CmdSetExclusiveScissorNV(m_commandBuffer->handle(), 0, 1, &test_case.scissor);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->end();
    }
}

TEST_F(NegativeCommand, ViewportWScalingNV) {
    TEST_DESCRIPTION("Verify VK_NV_clip_space_w_scaling");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    if (m_device->phy().features().multiViewport == VK_FALSE) {
        GTEST_SKIP() << "multiViewport feature is not supported";
    }
    InitRenderTarget();

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
    constexpr std::array scale = {VkViewportWScalingNV{-0.2f, -0.2f}, VkViewportWScalingNV{0.2f, -0.2f},
                                  VkViewportWScalingNV{-0.2f, 0.2f}, VkViewportWScalingNV{0.2f, 0.2f}};

    const uint32_t vp_count = static_cast<uint32_t>(vp.size());

    VkPipelineViewportWScalingStateCreateInfoNV vpsi = vku::InitStructHelper();
    vpsi.viewportWScalingEnable = VK_TRUE;
    vpsi.viewportCount = vp_count;
    vpsi.pViewportWScalings = scale.data();

    VkPipelineViewportStateCreateInfo vpci = vku::InitStructHelper(&vpsi);
    vpci.viewportCount = vp_count;
    vpci.pViewports = vp.data();
    vpci.scissorCount = vp_count;
    vpci.pScissors = sc.data();

    const auto set_vpci = [&vpci](CreatePipelineHelper &helper) { helper.vp_state_ci_ = vpci; };

    // Make sure no errors show up when creating the pipeline with w-scaling enabled
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit);

    // Create pipeline with w-scaling enabled but without a valid scaling array
    vpsi.pViewportWScalings = nullptr;
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit,
                                      vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01715"}));

    vpsi.pViewportWScalings = scale.data();

    // Create pipeline with w-scaling enabled but without matching viewport counts
    vpsi.viewportCount = 1;
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineViewportStateCreateInfo-viewportWScalingEnable-01726"}));

    VkShaderObj vs(this, vs_src, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    vpsi.viewportCount = vp_count;
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_ = vpci;
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_dynamic(*this);
    pipe_dynamic.InitState();
    pipe_dynamic.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe_dynamic.vp_state_ci_ = vpci;
    pipe_dynamic.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
    pipe_dynamic.CreateGraphicsPipeline();

    m_commandBuffer->begin();

    // Bind pipeline without dynamic w scaling enabled
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Bind pipeline that has dynamic w-scaling enabled
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_dynamic.Handle());

    const auto max_vps = m_device->phy().limits_.maxViewports;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWScalingNV-firstViewport-01324");
    vk::CmdSetViewportWScalingNV(m_commandBuffer->handle(), 1, max_vps, scale.data());
    m_errorMonitor->VerifyFound();

    vk::CmdSetViewportWScalingNV(m_commandBuffer->handle(), 0, vp_count, scale.data());

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, FilterCubicSamplerInCmdDraw) {
    TEST_DESCRIPTION("Verify if sampler is filter cubic, image view needs to support it.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &format_props);
    if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT) == 0) {
        GTEST_SKIP() << "SAMPLED_IMAGE_FILTER_CUBIC_BIT for format is not supported.";
    }

    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;

    VkPhysicalDeviceImageViewImageFormatInfoEXT imageview_format_info = vku::InitStructHelper();
    imageview_format_info.imageViewType = imageViewType;
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper(&imageview_format_info);
    image_format_info.type = image_ci.imageType;
    image_format_info.format = image_ci.format;
    image_format_info.tiling = image_ci.tiling;
    image_format_info.usage = image_ci.usage;
    image_format_info.flags = image_ci.flags;

    VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&filter_cubic_props);

    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_format_info, &image_format_properties);

    if (filter_cubic_props.filterCubic || filter_cubic_props.filterCubicMinmax) {
        GTEST_SKIP() << "Image and ImageView supports filter cubic ; skipped.";
    }

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView imageView = image.targetView(format, imageViewType);

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    vkt::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    VkSamplerReductionModeCreateInfo reduction_mode_ci = vku::InitStructHelper();
    reduction_mode_ci.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
    sampler_ci.pNext = &reduction_mode_ci;
    vkt::Sampler sampler_reduction(*m_device, sampler_ci);

    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler_reduction.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-filterCubicMinmax-02695");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
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
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ImageFilterCubicSamplerInCmdDraw) {
    TEST_DESCRIPTION("Verify if sampler is filter cubic with the VK_IMG_filter cubic extension that it's a valid ImageViewType.");

    AddRequiredExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::create_info();
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

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    vkt::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

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
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02693");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CmdUpdateBufferSize) {
    TEST_DESCRIPTION("Update buffer with invalid dataSize");

    RETURN_IF_SKIP(Init())

    uint32_t update_data[4] = {0, 0, 0, 0};
    VkDeviceSize dataSize = sizeof(uint32_t) * 4;
    vkt::Buffer buffer(*m_device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-dataSize-00033");
    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer.handle(), sizeof(uint32_t), dataSize, (void *)update_data);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CmdUpdateBufferDstOffset) {
    TEST_DESCRIPTION("Update buffer with invalid dst offset");

    RETURN_IF_SKIP(Init())

    uint32_t update_data[4] = {0, 0, 0, 0};
    VkDeviceSize dataSize = sizeof(uint32_t) * 4;
    vkt::Buffer buffer(*m_device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-dstOffset-00032");
    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer.handle(), sizeof(uint32_t) * 8, dataSize, (void *)update_data);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DescriptorSetPipelineBindPoint) {
    TEST_DESCRIPTION(
        "Attempt to bind descriptor set to a bind point not supported by command pool the command buffer was allocated from");

    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> no_gfx_qfi = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx_qfi) {
        GTEST_SKIP() << "No compute and transfer only queue family, skipping bindpoint and queue tests.";
    }

    vkt::CommandPool command_pool(*m_device, no_gfx_qfi.value());
    ASSERT_TRUE(command_pool.initialized());
    vkt::CommandBuffer command_buffer(m_device, &command_pool);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);
    ASSERT_TRUE(ds_pool.initialized());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);

    const vkt::DescriptorSetLayout descriptor_set_layout(*m_device, {dsl_binding});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout});

    command_buffer.begin();
    // Set invalid set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pipelineBindPoint-00361");
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, nullptr);
    m_errorMonitor->VerifyFound();
    command_buffer.end();
}

TEST_F(NegativeCommand, CmdClearColorImageNullColor) {
    TEST_DESCRIPTION("Test invalid null entries for clear color");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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

TEST_F(NegativeCommand, EndCommandBufferWithConditionalRendering) {
    TEST_DESCRIPTION("Call EndCommandBuffer when conditional rendering is active");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto buffer_ci =
        vkt::Buffer::create_info(32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT);
    vkt::Buffer buffer(*m_device, buffer_ci);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = vku::InitStructHelper();
    conditional_rendering_begin.buffer = buffer.handle();

    VkCommandBufferBeginInfo command_buffer_begin = vku::InitStructHelper();

    vk::BeginCommandBuffer(m_commandBuffer->handle(), &command_buffer_begin);
    vk::CmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-None-01978");
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DrawBlendEnabledFormatFeatures) {
    TEST_DESCRIPTION("Test pipeline blend enabled with missing image views format features");

    RETURN_IF_SKIP(Init())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    const VkFormat render_format = GetRenderTargetFormat();

    // Set format features from being found
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), render_format, &formatProps);
    if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Required linear tiling features not supported";
    }
    // Gets pass pipeline creation but not the actual tiling used
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    // will be caught at draw time that feature for optimal image is not set
    // InitRenderTarget() should be setting color attachment as VK_IMAGE_TILING_LINEAR
    formatProps.linearTilingFeatures &= ~VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), render_format, formatProps);

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
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

TEST_F(NegativeCommand, EndConditionalRendering) {
    TEST_DESCRIPTION("Invalid calls to vkCmdEndConditionalRenderingEXT.");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    vkt::RenderPass render_pass(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &imageView;
    fbci.width = 32;
    fbci.height = 32;
    fbci.layers = 1;
    vkt::Framebuffer framebuffer(*m_device, fbci);
    ASSERT_TRUE(framebuffer.initialized());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    vkt::Buffer buffer(*m_device, buffer_create_info);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = vku::InitStructHelper();
    conditional_rendering_begin.buffer = buffer.handle();

    VkClearValue clear_value;
    clear_value.color = m_clear_color;

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.renderPass = render_pass.handle();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderArea = {{0, 0}, {32, 32}};
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = &clear_value;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndConditionalRenderingEXT-None-01985");
    vk::CmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vk::CmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndConditionalRenderingEXT-None-01986");
    vk::CmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->NextSubpass();
    m_commandBuffer->EndRenderPass();
    vk::CmdEndConditionalRenderingEXT(m_commandBuffer->handle());

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_commandBuffer->NextSubpass();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndConditionalRenderingEXT-None-01987");
    vk::CmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
}

TEST_F(NegativeCommand, RenderPassContentsWhenCallingCmdExecuteCommandsWithBeginRenderPass) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRenderPass that hasn't set "
        "VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

    VkRenderPassBeginInfo rp_bi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, m_renderPass, m_framebuffer, VkRect2D{{0, 0}, {32u, 32u}},
                                             static_cast<uint32_t>(m_renderPassClearValues.size()), m_renderPassClearValues.data());
    m_commandBuffer->BeginRenderPass(rp_bi);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-contents-06018");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ExecuteCommandsSubpassIndices) {
    TEST_DESCRIPTION("Test invalid subpass when calling CmdExecuteCommands");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependencies;
    vkt::RenderPass render_pass(*m_device, rpci);
    ASSERT_TRUE(render_pass.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, render_pass.handle(), 1, &imageView, 32, 32, 1};
    vkt::Framebuffer framebuffer(*m_device, fbci);
    ASSERT_TRUE(framebuffer.initialized());

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

    const auto rp_bi = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, render_pass.handle(), framebuffer.handle(),
                                                                             VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IncompatibleRenderPassesInExecuteCommands) {
    TEST_DESCRIPTION("Test invalid subpass when calling CmdExecuteCommands");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

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
    vkt::RenderPass render_pass_1(*m_device, rpci);
    rpci.subpassCount = 2;
    vkt::RenderPass render_pass_2(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, render_pass_1.handle(), 1, &imageView, 32, 32, 1};
    vkt::Framebuffer framebuffer(*m_device, fbci);

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        render_pass_2.handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    const auto rp_bi = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, render_pass_1.handle(), framebuffer.handle(),
                                                                             VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyCommands2V13) {
    TEST_DESCRIPTION("Ensure copy_commands2 promotions are validated");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init())
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageObj image2(m_device);
    image2.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    vkt::Buffer dst_buffer(*m_device, 128 * 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer src_buffer(*m_device, 128 * 128, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkImageCopy2 copy_region = vku::InitStructHelper();
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.dstOffset = {4, 4, 0};
    copy_region.extent.width = 1;
    copy_region.extent.height = 1;
    copy_region.extent.depth = 1;
    VkCopyImageInfo2 copy_image_info = vku::InitStructHelper();
    copy_image_info.srcImage = image.handle();
    copy_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info.dstImage = image.handle();
    copy_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info.regionCount = 1;
    copy_image_info.pRegions = &copy_region;
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspect-06663");
    vk::CmdCopyImage2(m_commandBuffer->handle(), &copy_image_info);
    m_errorMonitor->VerifyFound();
    VkBufferCopy2 copy_buffer = vku::InitStructHelper();
    copy_buffer.dstOffset = 4;
    copy_buffer.size = 4;
    VkCopyBufferInfo2 copy_buffer_info = vku::InitStructHelper();
    copy_buffer_info.srcBuffer = dst_buffer.handle();
    copy_buffer_info.dstBuffer = dst_buffer.handle();
    copy_buffer_info.regionCount = 1;
    copy_buffer_info.pRegions = &copy_buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferInfo2-srcBuffer-00118");
    vk::CmdCopyBuffer2(m_commandBuffer->handle(), &copy_buffer_info);
    m_errorMonitor->VerifyFound();
    VkBufferImageCopy2 bic_region = vku::InitStructHelper();
    bic_region.bufferRowLength = 128;
    bic_region.bufferImageHeight = 128;
    bic_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bic_region.imageSubresource.layerCount = 1;
    bic_region.imageExtent.height = 4;
    bic_region.imageExtent.width = 4;
    bic_region.imageExtent.depth = 1;
    VkCopyBufferToImageInfo2KHR buffer_image_info = vku::InitStructHelper();
    buffer_image_info.srcBuffer = src_buffer.handle();
    buffer_image_info.dstImage = image.handle();
    buffer_image_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_info.regionCount = 1;
    buffer_image_info.pRegions = &bic_region;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-dstImage-00177");
    vk::CmdCopyBufferToImage2(m_commandBuffer->handle(), &buffer_image_info);
    m_errorMonitor->VerifyFound();
    VkCopyImageToBufferInfo2 image_buffer_info = vku::InitStructHelper();
    image_buffer_info.dstBuffer = src_buffer.handle();
    image_buffer_info.srcImage = image.handle();
    image_buffer_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_buffer_info.regionCount = 1;
    image_buffer_info.pRegions = &bic_region;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToBufferInfo2-dstBuffer-00191");
    vk::CmdCopyImageToBuffer2(m_commandBuffer->handle(), &image_buffer_info);
    m_errorMonitor->VerifyFound();
    VkImageBlit2 blit_region = vku::InitStructHelper();
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
    VkBlitImageInfo2 blit_image_info = vku::InitStructHelper();
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
    VkImageResolve2 resolve_region = vku::InitStructHelper();
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
    VkResolveImageInfo2 resolve_image_info = vku::InitStructHelper();
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

TEST_F(NegativeCommand, ResolveUsage) {
    TEST_DESCRIPTION("Resolve image with missing usage flags.");

    RETURN_IF_SKIP(Init())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
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

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    // Some implementations don't support multisampling, check that image format is valid
    VkImageFormatProperties image_format_props{};
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties(gpu(), dst_format, image_create_info.imageType, image_create_info.tiling,
                                                   image_create_info.usage, image_create_info.flags, &image_format_props);
    bool src_image_2_tests_valid = false;
    VkImageObj srcImage2(m_device);
    if ((result == VK_SUCCESS) && (image_format_props.sampleCounts & VK_SAMPLE_COUNT_4_BIT) != 0) {
        srcImage2.init(&image_create_info);
        ASSERT_TRUE(srcImage2.initialized());
        src_image_2_tests_valid = true;
    }

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
    vk::CmdResolveImage(m_commandBuffer->handle(), invalidSrcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-06764");
    vk::CmdResolveImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, invalidDstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-srcImage-06763");
    vk::CmdResolveImage(m_commandBuffer->handle(), invalidSrcImage2.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    if (src_image_2_tests_valid) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-06765");
        vk::CmdResolveImage(m_commandBuffer->handle(), srcImage2.handle(), VK_IMAGE_LAYOUT_GENERAL, invalidDstImage2.handle(),
                            VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, CopyImageRemainingLayers) {
    TEST_DESCRIPTION("Test copying an image with VkImageSubresourceLayers.layerCount = VK_REMAINING_ARRAY_LAYERS");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = image_format;
    ci.extent = {32, 32, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 8;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // 2D images
    VkImageObj image_a(m_device);
    VkImageObj image_b(m_device);

    // Copy from a to b
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_a.init(&ci);
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_b.init(&ci);

    ASSERT_TRUE(image_a.initialized());
    ASSERT_TRUE(image_b.initialized());

    m_commandBuffer->begin();

    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region{};
    copy_region.extent = ci.extent;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 7;
    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;  // This value is unsupported by VkImageSubresourceLayer
    copy_region.dstSubresource = copy_region.srcSubresource;

    // These vuids will trigger a special message stating that VK_REMAINING_ARRAY_LAYERS is unsupported
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-09243");  // src
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-09243");  // dst
    vk::CmdCopyImage(m_commandBuffer->handle(), image_a.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.image(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.size = 32 * 32 * 4;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer(*m_device, bci);

    VkBufferImageCopy buffer_copy{};
    buffer_copy.bufferImageHeight = ci.extent.height;
    buffer_copy.bufferRowLength = ci.extent.width;
    buffer_copy.imageExtent = ci.extent;
    buffer_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_copy.imageSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;  // This value is unsupported by VkImageSubresourceLayers
    buffer_copy.imageSubresource.mipLevel = 0;
    buffer_copy.imageSubresource.baseArrayLayer = 5;

    // This error will trigger first stating that the copy is too big for the buffer, because of VK_REMAINING_ARRAY_LAYERS
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-pRegions-00171");
    // This error will trigger second stating that VK_REMAINING_ARRAY_LAYERS is unsupported here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceLayers-layerCount-09243");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image_b.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_copy);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DepthStencilStateForReadOnlyLayout) {
    TEST_DESCRIPTION("invalid depth stencil state for subpass that uses read only image layout.");

    RETURN_IF_SKIP(Init())

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    VkImageObj ds_image(m_device);
    ds_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                  VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(ds_image.initialized());

    VkImageView image_view = ds_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkAttachmentDescription attachment = {};
    attachment.flags = 0;
    attachment.format = ds_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference ds_attachment_ref = {};
    ds_attachment_ref.attachment = 0;
    ds_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &ds_attachment_ref;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkPipelineDepthStencilStateCreateInfo depth_state_info = vku::InitStructHelper();
    depth_state_info.depthTestEnable = VK_TRUE;
    depth_state_info.depthWriteEnable = VK_TRUE;

    VkPipelineDepthStencilStateCreateInfo stencil_state_info = vku::InitStructHelper();
    stencil_state_info.front.failOp = VK_STENCIL_OP_ZERO;
    stencil_state_info.front.writeMask = 1;
    stencil_state_info.back.writeMask = 1;

    VkPipelineDepthStencilStateCreateInfo stencil_disabled_state_info = vku::InitStructHelper();
    stencil_disabled_state_info.front.failOp = VK_STENCIL_OP_ZERO;
    stencil_disabled_state_info.front.writeMask = 1;
    stencil_disabled_state_info.back.writeMask = 0;

    CreatePipelineHelper depth_pipe(*this);
    depth_pipe.InitState();
    depth_pipe.LateBindPipelineInfo();
    depth_pipe.gp_ci_.renderPass = render_pass.handle();
    depth_pipe.gp_ci_.pDepthStencilState = &depth_state_info;
    depth_pipe.CreateGraphicsPipeline(false);

    CreatePipelineHelper stencil_pipe(*this);
    stencil_pipe.InitState();
    stencil_pipe.LateBindPipelineInfo();
    stencil_pipe.gp_ci_.renderPass = render_pass.handle();
    stencil_pipe.gp_ci_.pDepthStencilState = &stencil_state_info;
    stencil_pipe.CreateGraphicsPipeline(false);

    CreatePipelineHelper stencil_disabled_pipe(*this);
    stencil_disabled_pipe.InitState();
    stencil_disabled_pipe.LateBindPipelineInfo();
    stencil_disabled_pipe.gp_ci_.renderPass = render_pass.handle();
    stencil_disabled_pipe.gp_ci_.pDepthStencilState = &stencil_disabled_state_info;
    stencil_disabled_pipe.CreateGraphicsPipeline(false);

    CreatePipelineHelper stencil_dynamic_pipe(*this);
    stencil_dynamic_pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    stencil_dynamic_pipe.InitState();
    stencil_dynamic_pipe.LateBindPipelineInfo();
    stencil_dynamic_pipe.gp_ci_.renderPass = render_pass.handle();
    stencil_dynamic_pipe.gp_ci_.pDepthStencilState = &stencil_state_info;
    stencil_dynamic_pipe.CreateGraphicsPipeline(false);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &image_view;

    vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
    ASSERT_TRUE(framebuffer.initialized());
    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.framebuffer = framebuffer.handle();
    render_pass_begin_info.renderArea.extent.width = 32;
    render_pass_begin_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_begin_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06886");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06887");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // valid since writeMask is set to zero
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_disabled_pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_dynamic_pipe.pipeline_);
    vk::CmdSetStencilWriteMask(m_commandBuffer->handle(), VK_STENCIL_FACE_FRONT_AND_BACK, 1);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06887");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // valid since writeMask is set to zero
    vk::CmdSetStencilWriteMask(m_commandBuffer->handle(), VK_STENCIL_FACE_FRONT_BIT, 0);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearColorImageWithRange) {
    TEST_DESCRIPTION("Record clear color with an invalid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    m_commandBuffer->begin();
    const auto cb_handle = m_commandBuffer->handle();

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseMipLevel-01470");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseMipLevel-01470");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-levelCount-01720");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-layerCount-01721");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCommand, ClearDepthStencilWithAspect) {
    TEST_DESCRIPTION("Verify ClearDepth with an invalid VkImageAspectFlags.");

    AddOptionalExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool separate_stencil_usage_supported = IsExtensionsEnabled(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_commandBuffer->begin();

    if (!separate_stencil_usage_supported) {
        printf("VK_EXT_separate_stencil_usage Extension not supported, skipping part of test\n");
    } else {
        VkImageStencilUsageCreateInfoEXT image_stencil_create_info = vku::InitStructHelper();
        image_stencil_create_info.stencilUsage =
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // not VK_IMAGE_USAGE_TRANSFER_DST_BIT

        image_create_info.pNext = &image_stencil_create_info;

        VkImageObj image(m_device);
        image.init(&image_create_info);
        ASSERT_TRUE(image.initialized());

        // Element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT, and image was created with separate stencil usage,
        // VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageStencilUsageCreateInfo::stencilUsage used to create image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02658");
        // ... since VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageCreateInfo::usage used to create image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02659");
        vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    image_create_info.pNext = nullptr;

    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());

    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the
    // VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the
    // VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02660");
    // ... since VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image.handle(), image.Layout(), &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    // Using stencil aspect when format only have depth
    const VkFormat depth_only_format = FindSupportedDepthOnlyFormat(gpu());
    if (depth_only_format != VK_FORMAT_UNDEFINED) {
        VkImageObj depth_image(m_device);
        image_create_info.format = depth_only_format;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        depth_image.init(&image_create_info);
        ASSERT_TRUE(depth_image.initialized());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-02825");
        vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), depth_image.handle(), depth_image.Layout(), &clear_value, 1,
                                      &range);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearDepthStencilWithRange) {
    TEST_DESCRIPTION("Record clear depth with an invalid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());
    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearDepthStencilValue clear_value = {};

    m_commandBuffer->begin();
    const auto cb_handle = m_commandBuffer->handle();

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        const VkImageSubresourceRange range = {ds_aspect, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 1, 1, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-levelCount-01720");
        const VkImageSubresourceRange range = {ds_aspect, 0, 0, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 0, 2, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-layerCount-01721");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 0};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 2};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCommand, ClearColorImageWithinRenderPass) {
    // Call CmdClearColorImage within an active RenderPass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-renderpass");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageObj dstImage(m_device);
    dstImage.init(&image_create_info);

    const VkImageSubresourceRange range = VkImageObj::subresource_range(image_create_info, VK_IMAGE_ASPECT_COLOR_BIT);

    vk::CmdClearColorImage(m_commandBuffer->handle(), dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearDepthStencilImage) {
    // Hit errors related to vk::CmdClearDepthStencilImage()
    // 1. Use an image that doesn't have VK_IMAGE_USAGE_TRANSFER_DST_BIT set
    // 2. Call CmdClearDepthStencilImage within an active RenderPass

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    VkClearDepthStencilValue clear_value = {0};
    VkImageCreateInfo image_create_info = VkImageObj::create_info();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Error here is that VK_IMAGE_USAGE_TRANSFER_DST_BIT is excluded for DS image that we'll call Clear on below
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageObj dst_image_bad_usage(m_device);
    dst_image_bad_usage.init(&image_create_info);
    const VkImageSubresourceRange range = VkImageObj::subresource_range(image_create_info, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_commandBuffer->begin();
    // need to handle since an element of pRanges includes VK_IMAGE_ASPECT_DEPTH_BIT without VkImageCreateInfo::usage having
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT being set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02660");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), dst_image_bad_usage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &range);
    m_errorMonitor->VerifyFound();

    // Fix usage for next test case
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj dst_image(m_device);
    dst_image.init(&image_create_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-renderpass");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearDepthRangeUnrestricted) {
    TEST_DESCRIPTION("Test clearing without VK_EXT_depth_range_unrestricted");

    // Extension doesn't have feature bit, so not enabling extension invokes restrictions
    RETURN_IF_SKIP(Init())

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    int depth_attachment_index = 1;
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearDepthStencilValue-depth-00022");
    const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    const VkClearDepthStencilValue bad_clear_value = {1.5f, 0};
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), m_depthStencil->handle(), VK_IMAGE_LAYOUT_GENERAL, &bad_clear_value, 1,
                                  &range);
    m_errorMonitor->VerifyFound();

    m_renderPassClearValues[depth_attachment_index].depthStencil.depth = 1.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearDepthStencilValue-depth-00022");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    // set back to normal
    m_renderPassClearValues[depth_attachment_index].depthStencil.depth = 1.0f;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearDepthStencilValue-depth-00022");
    VkClearAttachment clear_attachment;
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    clear_attachment.clearValue.depthStencil.depth = 1.5f;
    clear_attachment.clearValue.depthStencil.stencil = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearColorImageImageLayout) {
    TEST_DESCRIPTION("Check ClearImage layouts with SHARED_PRESENTABLE_IMAGE extension active.");

    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    VkImageObj dst_image(m_device);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;

    dst_image.init(&image_create_info);
    m_commandBuffer->begin();

    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail by using bad layout for color clear (GENERAL, SHARED_PRESENT or TRANSFER_DST are permitted).
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-01394");
    vk::CmdClearColorImage(m_commandBuffer->handle(), dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CmdClearAttachmentTests) {
    TEST_DESCRIPTION("Various tests for validating usage of vkCmdClearAttachments");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkImageFormatProperties image_format_properties{};
    ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), m_renderTargets[0]->format(),
                                                                 VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                                 m_renderTargets[0]->usage(), 0, &image_format_properties));
    if (image_format_properties.maxArrayLayers < 4) {
        GTEST_SKIP() << "Test needs to create image 2D array of 4 image view, but VkImageFormatProperties::maxArrayLayers is < 4. "
                        "Skipping test.";
    }

    // Create frame buffer with 2 layers, and image view with 4 layers,
    // to make sure that considered layer count is the one coming from frame buffer
    // (test would not fail if layer count used to do validation was 4)
    VkImageObj render_target(m_device);
    assert(!m_renderTargets.empty());
    const auto render_target_ci = VkImageObj::ImageCreateInfo2D(
        m_renderTargets[0]->width(), m_renderTargets[0]->height(), m_renderTargets[0]->create_info().mipLevels, 4,
        m_renderTargets[0]->format(), m_renderTargets[0]->usage(), VK_IMAGE_TILING_OPTIMAL);
    render_target.Init(render_target_ci, 0);
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = render_target.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = render_target_ci.format;
    ivci.subresourceRange.layerCount = render_target_ci.arrayLayers;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.components.r = VK_COMPONENT_SWIZZLE_R;
    ivci.components.g = VK_COMPONENT_SWIZZLE_G;
    ivci.components.b = VK_COMPONENT_SWIZZLE_B;
    ivci.components.a = VK_COMPONENT_SWIZZLE_A;
    vkt::ImageView render_target_view(*m_device, ivci);
    VkFramebufferCreateInfo fb_info = m_framebuffer_info;
    fb_info.layers = 2;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &render_target_view.handle();
    vkt::Framebuffer framebuffer(*m_device, fb_info);
    m_renderPassBeginInfo.framebuffer = framebuffer.handle();

    // Create secondary command buffer
    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_commandPool->handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);
    VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info = vku::InitStructHelper();
    secondary_cmd_buffer_inheritance_info.renderPass = m_renderPass;
    secondary_cmd_buffer_inheritance_info.framebuffer = framebuffer.handle();

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper();
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &secondary_cmd_buffer_inheritance_info;

    // Create clear rect
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    auto clear_cmds = [this, &color_attachment](VkCommandBuffer cmd_buffer, VkClearRect clear_rect) {
        // extent too wide
        VkClearRect clear_rect_too_large = clear_rect;
        clear_rect_too_large.rect.extent.width = renderPassBeginInfo().renderArea.extent.width + 4;
        clear_rect_too_large.rect.extent.height = clear_rect_too_large.rect.extent.height / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00016");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect_too_large);

        // baseLayer < render pass instance layer count
        clear_rect.baseArrayLayer = 1;
        clear_rect.layerCount = 1;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount <= render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 2;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer >= render pass instance layer count
        clear_rect.baseArrayLayer = 2;
        clear_rect.layerCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount > render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 4;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);
    };

    // Register clear commands to secondary command buffer
    secondary_cmd_buffer.begin(&secondary_cmd_buffer_begin_info);
    clear_cmds(secondary_cmd_buffer.handle(), clear_rect);
    secondary_cmd_buffer.end();

    m_commandBuffer->begin();

    // Execute secondary command buffer
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    // Execute same commands as previously, but in a primary command buffer
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    clear_cmds(m_commandBuffer->handle(), clear_rect);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, BindVertexIndexBufferUsage) {
    TEST_DESCRIPTION("Bad usage flags for binding the Vertex and Index buffer");
    RETURN_IF_SKIP(Init())

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindIndexBuffer-buffer-08784");
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->VerifyFound();

    VkDeviceSize offsets = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers-pBuffers-00627");
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offsets);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeCommand, BindIndexBufferHandles) {
    TEST_DESCRIPTION("call vkCmdBindIndexBuffer with bad Handles");
    RETURN_IF_SKIP(Init())
    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_commandBuffer->begin();
    VkBuffer bad_buffer = CastToHandle<VkBuffer, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindIndexBuffer-buffer-parameter");
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), bad_buffer, 0, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearImageAspectMask) {
    TEST_DESCRIPTION("Need to use VK_IMAGE_ASPECT_COLOR_BIT.");

    RETURN_IF_SKIP(Init())

    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);
    const VkImageSubresourceRange color_range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-aspectMask-02498");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &color_range);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, DebugLabelSecondaryCommandBuffer) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    vkt::CommandBuffer cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01913");
    vk::CmdEndDebugUtilsLabelEXT(cb);
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(NegativeCommand, RenderPassContinueNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use render pass continue bit with unsupported command pool.");

    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(m_device, &command_pool);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-09123");
    vk::BeginCommandBuffer(command_buffer.handle(), &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CopyDifferentFormatTexelBlockExtent) {
    TEST_DESCRIPTION("Copy bewteen compress images with different texel block extent.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init())

    VkFormat src_format = VK_FORMAT_BC7_UNORM_BLOCK;
    VkFormat dst_format = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), src_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0) {
        GTEST_SKIP() << "Src transfer for format is not supported";
    }
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), dst_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP() << "Dst transfer for format is not supported";
    }

    VkImageObj src_image(m_device);
    src_image.Init(32, 32, 1, src_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageObj dst_image(m_device);
    dst_image.Init(32, 32, 1, dst_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageCopy region;
    region.extent = {32, 32, 1};
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.dstSubresource = region.srcSubresource;
    region.srcOffset = {0, 0, 0};
    region.dstOffset = {0, 0, 0};
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-09247");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearDepthStencilImageWithInvalidAspect) {
    TEST_DESCRIPTION("Use vkCmdClearDepthStencilImage with invalid image aspect.");

    RETURN_IF_SKIP(Init())

    VkFormat format = FindSupportedDepthStencilFormat(m_device->phy().handle());
    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
               VK_IMAGE_TILING_OPTIMAL, 0);

    VkClearDepthStencilValue clear_value = {0};
    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0u;
    range.layerCount = 1u;
    range.baseArrayLayer = 0u;
    range.levelCount = 1u;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-aspectMask-02824");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1u, &range);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearColorImageWithMissingFeature) {
    TEST_DESCRIPTION("Use vkCmdClearColorImage with image format that doesnt have VK_FORMAT_FEATURE_TRANSFER_DST_BIT.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), format, formatProps);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0u;
    range.layerCount = 1u;
    range.baseArrayLayer = 0u;
    range.levelCount = 1u;

    VkClearColorValue clear_value = {{0, 0, 0, 0}};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-image-01993");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1u, &range);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeCommand, ClearDsImageWithInvalidAspect) {
    TEST_DESCRIPTION("Attempt to clear color aspect of depth/stencil image.");

    RETURN_IF_SKIP(Init());

    for (uint32_t i = 0; i < 2; ++i) {
        bool missing_depth = i == 0;
        auto format = missing_depth ? FindSupportedStencilOnlyFormat(m_device->phy().handle())
                                    : FindSupportedDepthOnlyFormat(m_device->phy().handle());
        if (format == VK_FORMAT_UNDEFINED) {
            continue;
        }
        VkImageAspectFlags image_aspect = missing_depth ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
        VkImageAspectFlags clear_aspect = missing_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_STENCIL_BIT;

        VkImageObj image(m_device);
        image.Init(32, 32, 1, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        VkImageView image_view = image.targetView(format, image_aspect);

        VkAttachmentReference ds_attachment_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass = {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &ds_attachment_ref, 0, nullptr,
        };

        VkAttachmentDescription attachment = {
            0,
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr);
        vkt::RenderPass render_pass(*m_device, rpci);

        auto fbci = vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0, render_pass.handle(), 1, &image_view, 32, 32, 1);
        vkt::Framebuffer framebuffer(*m_device, fbci);

        VkClearValue clear_value = {};
        clear_value.depthStencil = {0};

        VkClearAttachment clear_attachment = {};
        clear_attachment.aspectMask = clear_aspect;
        clear_attachment.colorAttachment = 0u;
        clear_attachment.clearValue.depthStencil.depth = 0.0f;
        VkClearRect clear_rect;
        clear_rect.rect = {{0, 0}, {32u, 32u}};
        clear_rect.baseArrayLayer = 0u;
        clear_rect.layerCount = 1u;

        VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
        render_pass_begin_info.renderPass = render_pass.handle();
        render_pass_begin_info.framebuffer = framebuffer.handle();
        render_pass_begin_info.renderArea = {{0, 0}, {32, 32}};
        render_pass_begin_info.clearValueCount = 1u;
        render_pass_begin_info.pClearValues = &clear_value;

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        if (missing_depth) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07884");
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07885");
        }
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1u, &clear_attachment, 1u, &clear_rect);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
}
