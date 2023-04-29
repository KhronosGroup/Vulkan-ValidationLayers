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

class PositiveSyncVal : public VkSyncValTest {};

TEST_F(PositiveSyncVal, CmdClearAttachmentLayer) {
    TEST_DESCRIPTION(
        "Clear one attachment layer and copy to a different one."
        "This checks for bug regression that produced a false-positive WAW hazard.");

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    const uint32_t width = 256;
    const uint32_t height = 128;
    const uint32_t layers = 2;
    const VkFormat rt_format = VK_FORMAT_B8G8R8A8_UNORM;
    const auto transfer_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto rt_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transfer_usage;

    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, rt_format, transfer_usage, VK_IMAGE_TILING_OPTIMAL);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj rt(m_device);
    rt.InitNoLayout(VkImageObj::ImageCreateInfo2D(width, height, 1, layers, rt_format, rt_usage, VK_IMAGE_TILING_OPTIMAL));
    rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        return attachment;
    };
    const VkAttachmentDescription attachment = attachment_without_load_store(rt_format);
    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.flags = 0;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &rt.targetView(rt_format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
    fbci.width = width;
    fbci.height = height;
    fbci.layers = layers;
    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer;
    rpbi.renderPass = render_pass;
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = render_pass;
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline());

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0, 1};
    copy_region.dstSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0 /* copy only to layer 0 */, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {width, height, 1};

    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {width, height};
    clear_rect.baseArrayLayer = 1;  // skip layer 0, clear only layer 1
    clear_rect.layerCount = 1;
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT};

    m_commandBuffer->begin();
    // Write 1: Copy to render target's layer 0
    vk::CmdCopyImage(*m_commandBuffer, image, VK_IMAGE_LAYOUT_GENERAL, rt, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    // Write 2: Clear render target's layer 1
    vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_device->m_queue);
}
