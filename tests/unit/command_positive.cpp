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
#include "../framework/pipeline_helper.h"
#include "../framework/thread_timeout_helper.h"
#include "generated/vk_extension_helper.h"

TEST_F(PositiveCommand, SecondaryCommandBufferBarrier) {
    TEST_DESCRIPTION("Add a pipeline barrier in a secondary command buffer");
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
                                           rp.handle(),
                                           0,
                                           VK_NULL_HANDLE,  // Set to NULL FB handle intentionally to flesh out any errors
                                           VK_FALSE,
                                           0,
                                           0};
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                                     &cbii};
    vk::BeginCommandBuffer(secondary.handle(), &cbbi);
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vk::CmdPipelineBarrier(secondary.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr, 0, nullptr);

    image.ImageMemoryBarrier(&secondary, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    secondary.end();

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveCommand, ClearAttachmentsCalledInSecondaryCB) {
    TEST_DESCRIPTION(
        "This test is to verify that when vkCmdClearAttachments is called by a secondary commandbuffer, the validation layers do "
        "not throw an error if the primary commandbuffer begins a renderpass before executing the secondary commandbuffer.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;
    hinfo.renderPass = renderPass();
    hinfo.subpass = 0;
    hinfo.framebuffer = m_framebuffer;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    secondary.begin(&info);
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0.0;
    color_attachment.clearValue.color.float32[1] = 0.0;
    color_attachment.clearValue.color.float32[2] = 0.0;
    color_attachment.clearValue.color.float32[3] = 0.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    vk::CmdClearAttachments(secondary.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary.end();
    // Modify clear rect here to verify that it doesn't cause validation error
    clear_rect = {{{0, 0}, {99999999, 99999999}}, 0, 0};

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, ClearAttachmentsCalledWithoutFbInSecondaryCB) {
    TEST_DESCRIPTION(
        "Verify calling vkCmdClearAttachments inside secondary commandbuffer without linking framebuffer pointer to"
        "inheritance struct");

    RETURN_IF_SKIP(Init())

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    hinfo.renderPass = renderPass();

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;

    secondary.begin(&info);
    VkClearAttachment color_attachment = {};
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0.0;
    color_attachment.clearValue.color.float32[1] = 0.0;
    color_attachment.clearValue.color.float32[2] = 0.0;
    color_attachment.clearValue.color.float32[3] = 0.0;
    color_attachment.colorAttachment = 0;

    VkClearAttachment ds_attachment = {};
    ds_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    ds_attachment.clearValue.depthStencil.depth = 0.2f;
    ds_attachment.clearValue.depthStencil.stencil = 1;

    const std::array clear_attachments = {color_attachment, ds_attachment};
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    vk::CmdClearAttachments(secondary.handle(), size32(clear_attachments), clear_attachments.data(), 1, &clear_rect);
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, CommandPoolDeleteWithReferences) {
    TEST_DESCRIPTION("Ensure the validation layers bookkeeping tracks the implicit command buffer frees.");
    RETURN_IF_SKIP(Init())

    VkCommandPoolCreateInfo cmd_pool_info = vku::InitStructHelper();
    cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_info.flags = 0;

    VkCommandPool secondary_cmd_pool;
    VkResult res = vk::CreateCommandPool(m_device->handle(), &cmd_pool_info, NULL, &secondary_cmd_pool);
    ASSERT_EQ(VK_SUCCESS, res);

    VkCommandBufferAllocateInfo cmdalloc = vkt::CommandBuffer::create_info(secondary_cmd_pool);
    cmdalloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    VkCommandBuffer secondary_cmds;
    res = vk::AllocateCommandBuffers(m_device->handle(), &cmdalloc, &secondary_cmds);

    VkCommandBufferInheritanceInfo cmd_buf_inheritance_info = vku::InitStructHelper();
    cmd_buf_inheritance_info.renderPass = VK_NULL_HANDLE;
    cmd_buf_inheritance_info.subpass = 0;
    cmd_buf_inheritance_info.framebuffer = VK_NULL_HANDLE;
    cmd_buf_inheritance_info.occlusionQueryEnable = VK_FALSE;
    cmd_buf_inheritance_info.queryFlags = 0;
    cmd_buf_inheritance_info.pipelineStatistics = 0;

    VkCommandBufferBeginInfo secondary_begin = vku::InitStructHelper();
    secondary_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    secondary_begin.pInheritanceInfo = &cmd_buf_inheritance_info;

    res = vk::BeginCommandBuffer(secondary_cmds, &secondary_begin);
    ASSERT_EQ(VK_SUCCESS, res);
    vk::EndCommandBuffer(secondary_cmds);

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmds);
    m_commandBuffer->end();

    // DestroyCommandPool *implicitly* frees the command buffers allocated from it
    vk::DestroyCommandPool(m_device->handle(), secondary_cmd_pool, NULL);
    // If bookkeeping has been lax, validating the reset will attempt to touch deleted data
    res = vk::ResetCommandPool(m_device->handle(), m_commandPool->handle(), 0);
    ASSERT_EQ(VK_SUCCESS, res);
}

TEST_F(PositiveCommand, SecondaryCommandBufferClearColorAttachments) {
    TEST_DESCRIPTION("Create a secondary command buffer and record a CmdClearAttachments call into it");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_EQ(VK_SUCCESS, vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer));
    VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = vku::InitStructHelper();
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
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(secondary_command_buffer, 1, &color_attachment, 1, &clear_rect);
    vk::EndCommandBuffer(secondary_command_buffer);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_command_buffer);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, SecondaryCommandBufferImageLayoutTransitions) {
    TEST_DESCRIPTION("Perform an image layout transition in a secondary command buffer followed by a transition in the primary.");
    RETURN_IF_SKIP(Init())
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    InitRenderTarget();
    // Allocate a secondary and primary cmd buffer
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer);
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer primary_command_buffer;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &primary_command_buffer);
    VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = vku::InitStructHelper();
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    vk::BeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(secondary_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    vk::EndCommandBuffer(secondary_command_buffer);

    // Now update primary cmd buffer to execute secondary and transitions image
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    vk::BeginCommandBuffer(primary_command_buffer, &command_buffer_begin_info);
    vk::CmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);
    VkImageMemoryBarrier img_barrier2 = vku::InitStructHelper();
    img_barrier2.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier2.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    img_barrier2.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    img_barrier2.image = image.handle();
    img_barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    img_barrier2.subresourceRange.baseArrayLayer = 0;
    img_barrier2.subresourceRange.baseMipLevel = 0;
    img_barrier2.subresourceRange.layerCount = 1;
    img_barrier2.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(primary_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier2);
    vk::EndCommandBuffer(primary_command_buffer);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &primary_command_buffer;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::DeviceWaitIdle(m_device->device());
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &secondary_command_buffer);
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &primary_command_buffer);
}

TEST_F(PositiveCommand, DrawIndirectCountWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_draw_indirect_count in 1.1 before drawIndirectCount feature was added");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests requires Vulkan 1.1 exactly";
    }

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Make calls to valid commands
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, DrawIndirectCountWithoutFeature12) {
    TEST_DESCRIPTION("Use VK_KHR_draw_indirect_count in 1.2 using the extension");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
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

    // Make calls to valid commands
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDrawIndirectCount(m_commandBuffer->handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, DrawIndirectCountWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_draw_indirect_count in 1.2 with feature bit enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.drawIndirectCount = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features12.drawIndirectCount != VK_TRUE) {
        printf("drawIndirectCount not supported, skipping test\n");
        return;
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
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

    // Make calls to valid commands
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDrawIndirectCount(m_commandBuffer->handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, CommandBufferSimultaneousUseSync) {
    RETURN_IF_SKIP(Init())

    // Record (empty!) command buffer that can be submitted multiple times
    // simultaneously.
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
    m_commandBuffer->begin(&cbbi);
    m_commandBuffer->end();

    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence fence;
    vk::CreateFence(m_device->device(), &fci, nullptr, &fence);

    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    VkSemaphore s1, s2;
    vk::CreateSemaphore(m_device->device(), &sci, nullptr, &s1);
    vk::CreateSemaphore(m_device->device(), &sci, nullptr, &s2);

    // Submit CB once signaling s1, with fence so we can roll forward to its retirement.
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &m_commandBuffer->handle(), 1, &s1};
    vk::QueueSubmit(m_default_queue, 1, &si, fence);

    // Submit CB again, signaling s2.
    si.pSignalSemaphores = &s2;
    vk::QueueSubmit(m_default_queue, 1, &si, VK_NULL_HANDLE);

    // Wait for fence.
    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, kWaitTimeout);

    // CB is still in flight from second submission, but semaphore s1 is no
    // longer in flight. delete it.
    vk::DestroySemaphore(m_device->device(), s1, nullptr);

    // Force device idle and clean up remaining objects
    vk::DeviceWaitIdle(m_device->device());
    vk::DestroySemaphore(m_device->device(), s2, nullptr);
    vk::DestroyFence(m_device->device(), fence, nullptr);
}

TEST_F(PositiveCommand, FramebufferBindingDestroyCommandPool) {
    TEST_DESCRIPTION(
        "This test should pass. Create a Framebuffer and command buffer, bind them together, then destroy command pool and "
        "framebuffer and verify there are no errors.");

    RETURN_IF_SKIP(Init())

    // A renderpass with one color attachment.
    VkAttachmentDescription attachment = {0,
                                          VK_FORMAT_R8G8B8A8_UNORM,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_STORE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &view, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    // Explicitly create a command buffer to bind the FB to so that we can then
    //  destroy the command pool in order to implicitly free command buffer
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    // Begin our cmd buffer with renderpass using our framebuffer
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    vk::BeginCommandBuffer(command_buffer, &begin_info);

    vk::CmdBeginRenderPass(command_buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(command_buffer);
    vk::EndCommandBuffer(command_buffer);
    // Destroy command pool to implicitly free command buffer
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);
}

TEST_F(PositiveCommand, ClearRectWith2DArray) {
    TEST_DESCRIPTION("Test using VkClearRect with an image that is of a 2D array type.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    for (uint32_t i = 0; i < 2; ++i) {
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_ci.extent.width = 32;
        image_ci.extent.height = 32;
        image_ci.extent.depth = 4;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageObj image(m_device);
        image.init(&image_ci);
        uint32_t layer_count = i == 0 ? image_ci.extent.depth : VK_REMAINING_ARRAY_LAYERS;
        VkImageView image_view =
            image.targetView(image_ci.format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layer_count, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

        VkAttachmentDescription attach_desc = {};
        attach_desc.format = image_ci.format;
        attach_desc.samples = image_ci.samples;
        attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference attachment = {};
        attachment.layout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.attachment = 0;

        VkSubpassDescription subpass = {};
        subpass.pColorAttachments = &attachment;
        subpass.colorAttachmentCount = 1;

        VkRenderPassCreateInfo rpci = vku::InitStructHelper();
        rpci.attachmentCount = 1;
        rpci.pAttachments = &attach_desc;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;

        vkt::RenderPass render_pass(*m_device, rpci);

        VkFramebufferCreateInfo fbci = vku::InitStructHelper();
        fbci.renderPass = render_pass.handle();
        fbci.attachmentCount = 1;
        fbci.pAttachments = &image_view;
        fbci.width = image_ci.extent.width;
        fbci.height = image_ci.extent.height;
        fbci.layers = image_ci.extent.depth;

        vkt::Framebuffer framebuffer(*m_device, fbci);

        VkClearAttachment color_attachment;
        color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_attachment.clearValue.color.float32[0] = 0.0;
        color_attachment.clearValue.color.float32[1] = 0.0;
        color_attachment.clearValue.color.float32[2] = 0.0;
        color_attachment.clearValue.color.float32[3] = 0.0;
        color_attachment.colorAttachment = 0;

        VkClearValue clearValue;
        clearValue.color = {{0, 0, 0, 0}};

        VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
        rpbi.renderPass = render_pass.handle();
        rpbi.framebuffer = framebuffer.handle();
        rpbi.renderArea = {{0, 0}, {image_ci.extent.width, image_ci.extent.height}};
        rpbi.clearValueCount = 1;
        rpbi.pClearValues = &clearValue;

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(rpbi);

        VkClearRect clear_rect = {{{0, 0}, {image_ci.extent.width, image_ci.extent.height}}, 0, image_ci.extent.depth};
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        m_commandBuffer->reset();
    }
}

TEST_F(PositiveCommand, EventStageMaskSecondaryCommandBuffer) {
    TEST_DESCRIPTION("Check secondary command buffers transfer event data when executed by primary ones");
    RETURN_IF_SKIP(Init())

    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);
    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    vkt::Event event(*m_device);

    secondary.begin();
    vk::CmdSetEvent(secondary.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    secondary.end();

    commandBuffer.begin();
    vk::CmdExecuteCommands(commandBuffer.handle(), 1, &secondary.handle());
    vk::CmdWaitEvents(commandBuffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    VkCommandBuffer handles[] = {commandBuffer.handle()};
    submit_info.pCommandBuffers = handles;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveCommand, EventsInSecondaryCommandBuffers) {
    TEST_DESCRIPTION("Test setting and waiting for an event in a secondary command buffer");
    RETURN_IF_SKIP(Init())

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_portability_subset enabled, skipping.\n";
    }

    vkt::Event ev(*m_device);
    VkEvent ev_handle = ev.handle();
    vkt::CommandBuffer secondary_cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb = secondary_cb.handle();
    secondary_cb.begin();
    vk::CmdSetEvent(scb, ev_handle, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    vk::CmdWaitEvents(scb, 1, &ev_handle, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0,
                      nullptr, 0, nullptr);
    secondary_cb.end();
    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &scb);
    m_commandBuffer->end();
    VkCommandBuffer cmdBuffer = m_commandBuffer->handle();
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmdBuffer;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveCommand, ThreadedCommandBuffersWithLabels) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    constexpr int worker_count = 8;
    ThreadTimeoutHelper timeout_helper(worker_count);

    auto worker_thread = [&](int worker_index) {
        auto timeout_guard = timeout_helper.ThreadGuard();
        // Command pool per worker thread
        vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

        constexpr int command_buffers_per_pool = 4;
        VkCommandBufferAllocateInfo commands_allocate_info = vku::InitStructHelper();
        commands_allocate_info.commandPool = pool.handle();
        commands_allocate_info.commandBufferCount = command_buffers_per_pool;
        commands_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        const VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

        VkDebugUtilsLabelEXT label = vku::InitStructHelper();
        label.pLabelName = "Test label";

        constexpr int iteration_count = 1000;
        for (int frame = 0; frame < iteration_count; frame++) {
            std::array<VkCommandBuffer, command_buffers_per_pool> command_buffers;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateCommandBuffers(m_device->device(), &commands_allocate_info, command_buffers.data()));
            for (int i = 0; i < command_buffers_per_pool; i++) {
                ASSERT_EQ(VK_SUCCESS, vk::BeginCommandBuffer(command_buffers[i], &begin_info));
                // Record debug label. It's a required step to reproduce the original issue
                vk::CmdInsertDebugUtilsLabelEXT(command_buffers[i], &label);
                ASSERT_EQ(VK_SUCCESS, vk::EndCommandBuffer(command_buffers[i]));
            }
            vk::FreeCommandBuffers(m_device->device(), pool.handle(), command_buffers_per_pool, command_buffers.data());
        }
    };
    std::vector<std::thread> workers;
    for (int i = 0; i < worker_count; i++) workers.emplace_back(worker_thread, i);
    constexpr int wait_time = 60;
    if (!timeout_helper.WaitForThreads(wait_time))
        ADD_FAILURE() << "The waiting time for the worker threads exceeded the maximum limit: " << wait_time << " seconds.";
    for (auto &worker : workers) worker.join();
}

TEST_F(PositiveCommand, ClearAttachmentsDepthStencil) {
    TEST_DESCRIPTION("Call CmdClearAttachments with no depth/stencil attachment.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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
    // Aspect Mask can be non-color because pDepthStencilAttachment is null
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &attachment, 1, &clear_rect);
}

TEST_F(PositiveCommand, ClearColorImageWithValidRange) {
    TEST_DESCRIPTION("Record clear color with a valid VkImageSubresourceRange");

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

    // Try good case
    {
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
    }

    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Try good case with VK_REMAINING
    {
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
    }
}

TEST_F(PositiveCommand, ClearDepthStencilWithValidRange) {
    TEST_DESCRIPTION("Record clear depth with a valid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());
    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearDepthStencilValue clear_value = {};

    m_commandBuffer->begin();
    const auto cb_handle = m_commandBuffer->handle();

    // Try good case
    {
        VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
    }

    image.ImageMemoryBarrier(m_commandBuffer, ds_aspect, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Try good case with VK_REMAINING
    {
        VkImageSubresourceRange range = {ds_aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
    }
}

TEST_F(PositiveCommand, FillBufferCmdPoolTransferQueue) {
    TEST_DESCRIPTION(
        "Use a command buffer with vkCmdFillBuffer that was allocated from a command pool that does not support graphics or "
        "compute opeartions");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> transfer =
        m_device->QueueFamilyMatching(VK_QUEUE_TRANSFER_BIT, (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT));
    if (!transfer) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }
    vkt::Queue *queue = m_device->queue_family_queues(transfer.value())[0].get();

    vkt::CommandPool pool(*m_device, transfer.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, queue);
    vkt::Buffer buffer(*m_device, 20, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    cb.begin();
    vk::CmdFillBuffer(cb.handle(), buffer.handle(), 0, 12, 0x11111111);
    cb.end();
}

TEST_F(PositiveCommand, DebugLabelPrimaryCommandBuffer) {
    TEST_DESCRIPTION("Create a debug label on a primary command buffer");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    m_commandBuffer->begin();

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vk::CmdBeginDebugUtilsLabelEXT(*m_commandBuffer, &label);
    vk::CmdEndDebugUtilsLabelEXT(*m_commandBuffer);

    m_commandBuffer->end();

    const std::array command_buffers = {m_commandBuffer->handle()};

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = size32(command_buffers);
    submit_info.pCommandBuffers = command_buffers.data();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveCommand, DebugLabelPrimaryCommandBuffers) {
    TEST_DESCRIPTION("Begin a debug label on 1 command buffer, then end it on another command buffer");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    vkt::CommandBuffer command_buffer_start(m_device, m_commandPool);
    vkt::CommandBuffer command_buffer_end(m_device, m_commandPool);

    // Start DebugLabel on 1 command buffer
    {
        VkDebugUtilsLabelEXT label = vku::InitStructHelper();
        label.pLabelName = "test";
        command_buffer_start.begin();
        vk::CmdBeginDebugUtilsLabelEXT(command_buffer_start.handle(), &label);
        command_buffer_start.end();
    }

    // End DebugLabel on another command buffer
    {
        command_buffer_end.begin();
        vk::CmdEndDebugUtilsLabelEXT(command_buffer_end.handle());
        command_buffer_end.end();
    }

    const std::array command_buffers = {command_buffer_start.handle(), command_buffer_end.handle()};

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = size32(command_buffers);
    submit_info.pCommandBuffers = command_buffers.data();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveCommand, DebugLabelSecondaryCommandBuffer) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    vkt::CommandBuffer cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    cb.begin();
    {
        VkDebugUtilsLabelEXT label = vku::InitStructHelper();
        label.pLabelName = "test";
        vk::CmdBeginDebugUtilsLabelEXT(cb, &label);
        vk::CmdEndDebugUtilsLabelEXT(cb);
    }
    cb.end();
}

TEST_F(PositiveCommand, CopyImageRemainingLayersMaintenance5) {
    TEST_DESCRIPTION(
        "Test copying an image with VkImageSubresourceLayers.layerCount = VK_REMAINING_ARRAY_LAYERS using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    m_commandBuffer->begin();
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region{};
    copy_region.extent = ci.extent;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 2;
    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copy_region.dstSubresource = copy_region.srcSubresource;

    vk::CmdCopyImage(m_commandBuffer->handle(), image_a.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.image(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    // layerCount can explicitly list value
    copy_region.dstSubresource.layerCount = 2;
    vk::CmdCopyImage(m_commandBuffer->handle(), image_a.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.image(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, CopyImageTypeExtentMismatchMaintenance5) {
    TEST_DESCRIPTION("Test copying an image with extent mistmatch using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

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

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_commandBuffer->begin();
    vk::CmdCopyImage(m_commandBuffer->handle(), image_1D.image(), VK_IMAGE_LAYOUT_GENERAL, image_2D.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, MultiDrawMaintenance5) {
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

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 511, 0};

    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 0, 1024, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);

    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 2, 1022, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);

    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 0, VK_WHOLE_SIZE, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);

    multi_draw_indices.firstIndex = 16;
    multi_draw_indices.indexCount = 128;
    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 32, 288, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
}

TEST_F(PositiveCommand, MultiDrawMaintenance5Mixed) {
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

    // too small
    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 0, 1000, VK_INDEX_TYPE_UINT16);
    // should be overwritten
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
}

TEST_F(PositiveCommand, CopyImageLayerCount) {
    TEST_DESCRIPTION("Check layerCount in vkCmdCopyImage");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

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
    copyRegion.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = copyRegion.srcSubresource;
    copyRegion.dstOffset = {32, 32, 0};
    copyRegion.extent = {16, 16, 1};
    vk::CmdCopyImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copyRegion);
    m_commandBuffer->end();
}

TEST_F(PositiveCommand, CopyBufferToRemaingImageLayers) {
    TEST_DESCRIPTION("Test vkCmdCopyBufferToImage2 with VK_REMAINING_ARRAY_LAYERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance_5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance_5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance_5_features));

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkBufferImageCopy2 region = vku::InitStructHelper();
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyBufferToImageInfo2 copy_buffer_to_image = vku::InitStructHelper();
    copy_buffer_to_image.srcBuffer = buffer.handle();
    copy_buffer_to_image.dstImage = image.handle();
    copy_buffer_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_buffer_to_image.regionCount = 1u;
    copy_buffer_to_image.pRegions = &region;

    m_commandBuffer->begin();
    vk::CmdCopyBufferToImage2KHR(m_commandBuffer->handle(), &copy_buffer_to_image);
    m_commandBuffer->end();
}
