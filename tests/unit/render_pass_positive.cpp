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
#include "../framework/descriptor_helper.h"

TEST_F(PositiveRenderPass, AttachmentUsedTwiceOK) {
    TEST_DESCRIPTION("Attachment is used simultaneously as color and input, with the same layout. This is OK.");

    RETURN_IF_SKIP(Init())

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);
}

TEST_F(PositiveRenderPass, InitialLayoutUndefined) {
    TEST_DESCRIPTION(
        "Ensure that CmdBeginRenderPass with an attachment's initialLayout of VK_IMAGE_LAYOUT_UNDEFINED works when the command "
        "buffer has prior knowledge of that attachment's layout.");

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

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    vkt::ImageView view(*m_device, ivci);

    VkFramebufferCreateInfo fci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &view.handle(), 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    // Record a single command buffer which uses this renderpass twice. The
    // bug is triggered at the beginning of the second renderpass, when the
    // command buffer already has a layout recorded for the attachment.
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, AttachmentLayoutWithLoadOpThenReadOnly) {
    TEST_DESCRIPTION(
        "Positive test where we create a renderpass with an attachment that uses LOAD_OP_CLEAR, the first subpass has a valid "
        "layout, and a second subpass then uses a valid *READ_ONLY* layout.");
    RETURN_IF_SKIP(Init())
    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    VkAttachmentReference attach[2] = {};
    attach[0].attachment = 0;
    attach[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach[1].attachment = 0;
    attach[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkSubpassDescription subpasses[2] = {};
    // First subpass clears DS attach on load
    subpasses[0].pDepthStencilAttachment = &attach[0];
    // 2nd subpass reads in DS as input attachment
    subpasses[1].inputAttachmentCount = 1;
    subpasses[1].pInputAttachments = &attach[1];
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = depth_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;

    // Now create RenderPass and verify no errors
    vkt::RenderPass rp(*m_device, rpci);
}

TEST_F(PositiveRenderPass, BeginSubpassZeroTransitionsApplied) {
    TEST_DESCRIPTION("Ensure that CmdBeginRenderPass applies the layout transitions for the first subpass");

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

    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 1, &dep};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &view, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    // Record a single command buffer which issues a pipeline barrier w/
    // image memory barrier for the attachment. This detects the previously
    // missing tracking of the subpass layout by throwing a validation error
    // if it doesn't occur.
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, BeginTransitionsAttachmentUnused) {
    TEST_DESCRIPTION(
        "Ensure that layout transitions work correctly without errors, when an attachment reference is VK_ATTACHMENT_UNUSED");

    RETURN_IF_SKIP(Init())

    // A renderpass with no attachments
    VkAttachmentReference att_ref = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &subpass, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible framebuffer.
    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 0, nullptr, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    // Record a command buffer which just begins and ends the renderpass. The
    // bug manifests in BeginRenderPass.
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, BeginStencilLoadOp) {
    TEST_DESCRIPTION("Create a stencil-only attachment with a LOAD_OP set to CLEAR. stencil[Load|Store]Op used to be ignored.");
    RETURN_IF_SKIP(Init())
    VkFormat depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageFormatProperties formatProps;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), depth_stencil_fmt, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0,
                                               &formatProps);
    if (formatProps.maxExtent.width < 100 || formatProps.maxExtent.height < 100) {
        GTEST_SKIP() << "Image format max extent is too small";
    }

    m_depthStencil->Init(100, 100, 1, depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    VkAttachmentDescription att = {};
    VkAttachmentReference ref = {};
    att.format = depth_stencil_fmt;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkClearValue clear;
    clear.depthStencil.depth = 1.0;
    clear.depthStencil.stencil = 0;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = NULL;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &ref;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    vkt::RenderPass rp(*m_device, rp_info);

    VkImageView depth_image_view =
        m_depthStencil->targetView(depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &depth_image_view;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;
    vkt::Framebuffer fb(*m_device, fb_info);

    VkRenderPassBeginInfo rpbinfo = vku::InitStructHelper();
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = &clear;
    rpbinfo.renderPass = rp.handle();
    rpbinfo.renderArea.extent.width = 100;
    rpbinfo.renderArea.extent.height = 100;
    rpbinfo.renderArea.offset.x = 0;
    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.framebuffer = fb.handle();

    vkt::Fence fence;
    fence.init(*m_device, vkt::Fence::create_info());
    ASSERT_TRUE(fence.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(fence);

    VkImageObj destImage(m_device);
    destImage.Init(100, 100, 1, depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                   VK_IMAGE_TILING_OPTIMAL, 0);
    fence.wait(kWaitTimeout);
    vkt::CommandBuffer cmdbuf(m_device, m_commandPool);
    cmdbuf.begin();

    m_depthStencil->ImageMemoryBarrier(&cmdbuf, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                       VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                       VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    destImage.ImageMemoryBarrier(&cmdbuf, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                 VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageCopy cregion;
    cregion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    cregion.srcSubresource.mipLevel = 0;
    cregion.srcSubresource.baseArrayLayer = 0;
    cregion.srcSubresource.layerCount = 1;
    cregion.srcOffset.x = 0;
    cregion.srcOffset.y = 0;
    cregion.srcOffset.z = 0;
    cregion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    cregion.dstSubresource.mipLevel = 0;
    cregion.dstSubresource.baseArrayLayer = 0;
    cregion.dstSubresource.layerCount = 1;
    cregion.dstOffset.x = 0;
    cregion.dstOffset.y = 0;
    cregion.dstOffset.z = 0;
    cregion.extent.width = 100;
    cregion.extent.height = 100;
    cregion.extent.depth = 1;
    vk::CmdCopyImage(cmdbuf.handle(), m_depthStencil->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destImage.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cregion);
    cmdbuf.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmdbuf.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveRenderPass, BeginInlineAndSecondaryCommandBuffers) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    m_commandBuffer->begin();

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_commandBuffer->EndRenderPass();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, BeginDepthStencilLayoutTransitionFromUndefined) {
    TEST_DESCRIPTION(
        "Create a render pass with depth-stencil attachment where layout transition from UNDEFINED TO DS_READ_ONLY_OPTIMAL is set "
        "by render pass and verify that transition has correctly occurred at queue submit time with no validation errors.");

    RETURN_IF_SKIP(Init())
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    VkImageFormatProperties format_props;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &format_props);
    if (format_props.maxExtent.width < 32 || format_props.maxExtent.height < 32) {
        GTEST_SKIP() << "Depth extent too small";
    }

    InitRenderTarget();

    // A renderpass with one depth/stencil attachment.
    VkAttachmentDescription attachment = {0,
                                          depth_format,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &att_ref, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible ds image.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        depth_format,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
    };
    vkt::ImageView view(*m_device, ivci);

    VkFramebufferCreateInfo fci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &view.handle(), 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
}

TEST_F(PositiveRenderPass, DestroyPipeline) {
    TEST_DESCRIPTION("Draw using a pipeline whose create renderPass has been destroyed.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkResult err;

    // Create a renderPass that's compatible with Draw-time renderPass
    VkAttachmentDescription att = {};
    att.format = m_render_target_fmt;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ref = {};
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ref.attachment = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    subpass.pResolveAttachments = NULL;

    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    VkRenderPass rp;
    err = vk::CreateRenderPass(device(), &rp_info, NULL, &rp);
    ASSERT_EQ(VK_SUCCESS, err);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.renderPass = rp;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Destroy renderPass before pipeline is used in Draw
    //  We delay until after CmdBindPipeline to verify that invalid binding isn't
    //  created between CB & renderPass, which we used to do.
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveRenderPass, ImagelessFramebufferNonZeroBaseMip) {
    TEST_DESCRIPTION("Use a 1D image view for an imageless framebuffer with base mip level > 0.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR pd_imageless_fb_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(pd_imageless_fb_features);
    RETURN_IF_SKIP(InitState(nullptr, &pd_imageless_fb_features));

    constexpr uint32_t width = 512;
    constexpr uint32_t height = 1;
    VkFormat formats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    VkFormat fb_attachments[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    constexpr uint32_t base_mip = 1;

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachment_desc = {};
    attachment_desc.format = formats[0];
    attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachment_ref = {};
    attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass_desc = {};
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &attachment_ref;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    vkt::RenderPass rp(*m_device, rp_ci);

    VkFramebufferAttachmentImageInfoKHR fb_attachment_image_info = vku::InitStructHelper();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    fb_attachment_image_info.width = width;
    fb_attachment_image_info.height = height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 2;
    fb_attachment_image_info.pViewFormats = fb_attachments;
    fb_attachment_image_info.height = 1;
    fb_attachment_image_info.width = width >> base_mip;

    VkFramebufferAttachmentsCreateInfoKHR fb_attachments_ci = vku::InitStructHelper();
    fb_attachments_ci.attachmentImageInfoCount = 1;
    fb_attachments_ci.pAttachmentImageInfos = &fb_attachment_image_info;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_attachments_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    fb_ci.width = width >> base_mip;
    fb_ci.height = height;
    fb_ci.layers = 1;
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = nullptr;
    fb_ci.renderPass = rp.handle();
    vkt::Framebuffer fb(*m_device, fb_ci);
    ASSERT_TRUE(fb.initialized());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_ci.extent.width = width;
    image_ci.extent.height = 1;
    image_ci.extent.depth = 1;
    image_ci.arrayLayers = 1;
    image_ci.mipLevels = 2;
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.format = formats[0];

    VkImageObj image_object(m_device);
    image_object.init(&image_ci);
    VkImage image = image_object.image();

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    image_view_ci.format = formats[0];
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseMipLevel = base_mip;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView image_view_obj(*m_device, image_view_ci);
    VkImageView image_view = image_view_obj.handle();

    VkRenderPassAttachmentBeginInfoKHR rp_attachment_begin_info = vku::InitStructHelper();
    rp_attachment_begin_info.attachmentCount = 1;
    rp_attachment_begin_info.pAttachments = &image_view;
    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper(&rp_attachment_begin_info);
    rp_begin_info.renderPass = rp.handle();
    rp_begin_info.renderArea.extent.width = width >> base_mip;
    rp_begin_info.renderArea.extent.height = height;
    rp_begin_info.framebuffer = fb.handle();

    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    m_commandBuffer->begin(&cmd_begin_info);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

TEST_F(PositiveRenderPass, ValidStages) {
    TEST_DESCRIPTION("Create render pass with valid stages");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitState())

    VkSubpassDescription sci[2] = {};
    sci[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    // to be filled later by tests

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 2;
    rpci.pSubpasses = sci;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    const VkPipelineStageFlags kGraphicsStages =
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    dependency.srcSubpass = 0;
    dependency.dstSubpass = 1;
    dependency.srcStageMask = kGraphicsStages;
    dependency.dstStageMask = kGraphicsStages;
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = kGraphicsStages | VK_PIPELINE_STAGE_HOST_BIT;
    dependency.dstStageMask = kGraphicsStages;
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);

    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = kGraphicsStages;
    dependency.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);
}

TEST_F(PositiveRenderPass, SingleMipTransition) {
    TEST_DESCRIPTION("Ensure that the validation message contains the correct miplevel");

    RETURN_IF_SKIP(Init())

    // Create RenderPass.

    VkAttachmentDescription attachments[2] = {
        {
            0,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
        },
        {0, VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL}};

    VkAttachmentReference att_refs[2] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
    };

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_refs[0], nullptr, &att_refs[1], 0,
                                    nullptr};

    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_SHADER_READ_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attachments, 1, &subpass, 1, &dep};
    vkt::RenderPass rp(*m_device, rpci);

    // Create Framebuffer.

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(colorImage.initialized());

    VkImageObj depthImage(m_device);
    depthImage.Init(32, 32, 2, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(depthImage.initialized());

    VkImageView baseViews[] = {
        colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, /*baseMipLevel*/ 0, /*levelCount*/ 1),
        depthImage.targetView(VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, /*baseMipLevel*/ 0, /*levelCount*/ 1),
    };

    VkImageViewCreateInfo vinfo = {};
    vinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vinfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vinfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vinfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vinfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vinfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

    vinfo.image = colorImage.handle();
    vinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkt::ImageView fullView0(*m_device, vinfo);

    vinfo.image = depthImage.handle();
    vinfo.format = VK_FORMAT_D32_SFLOAT;
    vinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vkt::ImageView fullView1(*m_device, vinfo);

    VkImageView fullViews[] = {fullView0.handle(), fullView1.handle()};

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 2, baseViews, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    // Create shader modules

    char const fsSource[] = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0, binding=2) uniform sampler2D depth;
        void main() {
           x = texture(depth, vec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create descriptor set and friends.
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet::Bindings binding_defs = {{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, binding_defs);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});
    OneOffDescriptorSet descriptor_set(m_device, binding_defs);

    VkDescriptorImageInfo image_info = {
        sampler.handle(),
        fullViews[1],
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet descriptor_writes[1] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 2;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &image_info;

    // Create Pipeline.

    VkPipelineDepthStencilStateCreateInfo ds_ci = {};
    ds_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_LESS;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.ds_ci_ = ds_ci;
    pipe.CreateGraphicsPipeline();

    // Start pushing commands.

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, nullptr);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();

    // At this point the first miplevel should be in GENERAL due to the "finalLayout" in the render pass.
    // Note that these image barriers attempt to transition *all* miplevels, even though only 1 miplevel has transitioned.

    colorImage.Layout(VK_IMAGE_LAYOUT_GENERAL);

    colorImage.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    depthImage.Layout(VK_IMAGE_LAYOUT_GENERAL);

    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, ViewMasks) {
    TEST_DESCRIPTION("Create render pass with view mask, with multiview feature enabled in Vulkan11Features.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan11Features vulkan_11_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(vulkan_11_features);
    if (vulkan_11_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported, skipping test.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);
}

TEST_F(PositiveRenderPass, BeginWithViewMasks) {
    TEST_DESCRIPTION("Begin render pass with view mask and a push descriptor.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceVulkan11Features vulkan_11_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(vulkan_11_features);
    if (vulkan_11_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported, skipping test.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    std::array subpasses = {vku::InitStruct<VkSubpassDescription2>(), vku::InitStruct<VkSubpassDescription2>()};
    subpasses[0].viewMask = 0x1;
    subpasses[1].viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = subpasses.size();
    render_pass_ci.pSubpasses = subpasses.data();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView view(*m_device, ivci);

    VkFramebufferCreateInfo fci = vku::InitStructHelper();
    fci.renderPass = render_pass.handle();
    fci.attachmentCount = 1;
    fci.pAttachments = &view.handle();
    fci.width = 32;
    fci.height = 32;
    fci.layers = 1;
    vkt::Framebuffer fb(*m_device, fci);

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.renderPass = render_pass.handle();
    rpbi.framebuffer = fb.handle();
    rpbi.renderArea = {{0, 0}, {32, 32}};

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});
    // Create push descriptor set layout
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {dsl_binding},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.InitState();
    helper.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&push_ds_layout, &ds_layout});
    helper.gp_ci_.renderPass = render_pass.handle();
    helper.CreateGraphicsPipeline();

    const uint32_t data_size = sizeof(float) * 3;
    vkt::Buffer vbo(*m_device, data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo.handle();
    buff_info.offset = 0;
    buff_info.range = data_size;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_layout_.handle(), 0, 1,
                                &descriptor_write);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->NextSubpass();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, BeginDedicatedStencilLayout) {
    TEST_DESCRIPTION("Render pass using a dedicated stencil layout, different from depth layout");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features vulkan_12_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(vulkan_12_features);
    if (vulkan_12_features.separateDepthStencilLayouts == VK_FALSE) {
        GTEST_SKIP() << "separateDepthStencilLayouts feature not supported, skipping test.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &vulkan_12_features));

    // Create depth stencil image
    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    VkImageObj ds_image(m_device);
    ds_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                  VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(ds_image.initialized());

    VkImageView ds_view = ds_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    // Create depth stencil attachment
    VkAttachmentDescriptionStencilLayoutKHR attachment_desc_stencil_layout = vku::InitStructHelper();
    attachment_desc_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentDescription2 ds_attachment_desc = vku::InitStructHelper(&attachment_desc_stencil_layout);
    ds_attachment_desc.format = ds_format;
    ds_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    ds_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ds_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    ds_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ds_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentReferenceStencilLayoutKHR attachment_ref_stencil_layout = vku::InitStructHelper();
    attachment_ref_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference2 ds_attachment_ref = vku::InitStructHelper(&attachment_ref_stencil_layout);
    ds_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    // Create render pass
    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &ds_attachment_ref;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &ds_attachment_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    // Create framebuffer
    VkFramebufferCreateInfo fci = vku::InitStructHelper();
    fci.renderPass = render_pass.handle();
    fci.attachmentCount = 1;
    fci.pAttachments = &ds_view;
    fci.width = ds_image.width();
    fci.height = ds_image.height();
    fci.layers = 1;
    vkt::Framebuffer fb(*m_device, fci);

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.renderPass = render_pass.handle();
    rpbi.framebuffer = fb.handle();
    rpbi.renderArea = {{0, 0}, {fci.width, fci.height}};

    // Use helper to create graphics pipeline
    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    // One stencil op is not OP_KEEP, both write mask are not 0
    ds_state.front.failOp = VK_STENCIL_OP_ZERO;
    ds_state.front.writeMask = 0x1;
    ds_state.back.writeMask = 0x1;
    CreatePipelineHelper helper(*this);
    helper.InitState();
    helper.gp_ci_.pDepthStencilState = &ds_state;
    helper.gp_ci_.renderPass = render_pass.handle();
    helper.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    // If the stencil layout was not specified separately using the separateDepthStencilLayouts feature,
    // and used in the validation code, 06887 would trigger with the following draw call
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, QueriesInMultiview) {
    TEST_DESCRIPTION("Use queries in a render pass instance with multiview enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceVulkan11Features vulkan_11_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(vulkan_11_features);

    if (vulkan_11_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported, skipping test.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

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

    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmvci = vku::InitStructHelper();
    rpmvci.subpassCount = 1;
    rpmvci.pViewMasks = viewMasks;
    rpmvci.correlationMaskCount = 1;
    rpmvci.pCorrelationMasks = correlationMasks;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpmvci);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    vkt::RenderPass rp(*m_device, rpci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.arrayLayers = 3;
    image_ci.mipLevels = 2;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageObj image(m_device);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 3};
    vkt::ImageView view(*m_device, ivci);
    VkImageView image_view_handle = view.handle();

    VkFramebufferCreateInfo fci = vku::InitStructHelper();
    fci.renderPass = rp.handle();
    fci.attachmentCount = 1;
    fci.pAttachments = &image_view_handle;
    fci.width = 32;
    fci.height = 32;
    fci.layers = 1;
    vkt::Framebuffer fb(*m_device, fci);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;
    vkt::QueryPool query_pool(*m_device, qpci);

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.renderPass = rp.handle();
    rpbi.framebuffer = fb.handle();
    rpbi.renderArea = {{0, 0}, {32, 32}};

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 2);

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->EndRenderPass();

    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 2, buffer.handle(), 0, 0, 0);
    m_commandBuffer->end();

    VkCommandBuffer handle = m_commandBuffer->handle();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &handle;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveRenderPass, StoreOpNoneExt) {
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // A renderpass with one color attachment.
    VkAttachmentDescription attachment = {0,
                                          VK_FORMAT_R8G8B8A8_UNORM,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_NONE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr};

    vkt::RenderPass rp(*m_device, rpci);
}

TEST_F(PositiveRenderPass, FramebufferCreateDepthStencilLayoutTransitionForDepthOnlyImageView) {
    TEST_DESCRIPTION(
        "Validate that when an imageView of a depth/stencil image is used as a depth/stencil framebuffer attachment, the "
        "aspectMask is ignored and both depth and stencil image subresources are used.");

    RETURN_IF_SKIP(Init())
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Image format does not support sampling";
    }

    VkAttachmentDescription attachment = {0,
                                          VK_FORMAT_D32_SFLOAT_S8_UINT,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_STORE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &att_ref, 0, nullptr};

    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 1, &dep};
    vkt::RenderPass rp(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
                       0x26,  // usage
                       VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    image.SetLayout(0x6, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkImageView view = image.targetView(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &view, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    m_commandBuffer->begin();

    VkImageMemoryBarrier imb = vku::InitStructHelper();
    imb.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imb.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imb.srcQueueFamilyIndex = 0;
    imb.dstQueueFamilyIndex = 0;
    imb.image = image.handle();
    imb.subresourceRange.aspectMask = 0x6;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 0x1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 0x1;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imb);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
}

TEST_F(PositiveRenderPass, FramebufferWithAttachmentsTo3DImageMultipleSubpasses) {
    TEST_DESCRIPTION(
        "Test no false overlap is reported with multi attachment framebuffer (attachments are slices of a 3D image). Multiple "
        "subpasses that draw to a single slice of a 3D image");

    AddRequiredExtensions(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    constexpr unsigned depth_count = 2u;

    // 3D image with 2 depths
    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_info.imageType = VK_IMAGE_TYPE_3D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {64, 64, depth_count};
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image_3d{m_device};
    image_3d.init(&image_info);
    image_3d.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // 2D image views to be used as color attchments for framebuffer
    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.image = image_3d.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView image_views[depth_count];
    VkImageView views[depth_count] = {VK_NULL_HANDLE};
    for (unsigned i = 0; i < depth_count; ++i) {
        view_info.subresourceRange.baseArrayLayer = i;
        image_views[i].init(*m_device, view_info);
        views[i] = image_views[i].handle();
    }

    // Render pass with 2 subpasses
    VkAttachmentReference attach[depth_count] = {};
    VkSubpassDescription subpasses[depth_count] = {};

    for (unsigned i = 0; i < depth_count; ++i) {
        attach[i].attachment = i;
        attach[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpasses[i].pColorAttachments = &attach[i];
        subpasses[i].colorAttachmentCount = 1;
    }

    VkAttachmentDescription attach_desc[depth_count] = {};
    for (unsigned i = 0; i < depth_count; ++i) {
        attach_desc[i].format = image_info.format;
        attach_desc[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attach_desc[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attach_desc[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.subpassCount = depth_count;
    rp_info.pSubpasses = subpasses;
    rp_info.attachmentCount = depth_count;
    rp_info.pAttachments = attach_desc;

    vkt::RenderPass renderpass(*m_device, rp_info);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = renderpass.handle();
    fb_info.attachmentCount = depth_count;
    fb_info.pAttachments = views;
    fb_info.width = image_info.extent.width;
    fb_info.height = image_info.extent.height;
    fb_info.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, fb_info);

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = renderpass.handle();
    rp_begin_info.framebuffer = framebuffer.handle();
    rp_begin_info.renderArea = {{0, 0}, {image_info.extent.width, image_info.extent.height}};

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    for (unsigned i = 0; i < (depth_count - 1); ++i) {
        m_commandBuffer->NextSubpass();
    }
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, ImageLayoutTransitionOf3dImageWith2dViews) {
    TEST_DESCRIPTION(
        "Test that transitioning the layout of a mip level of a 3D image using a view of one of its slice applies to the entire 3D "
        "image: all views referencing different slices of the same mip level should also see their layout transitioned");

    AddRequiredExtensions(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
        GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.imageView2DOn3DImage) {
            GTEST_SKIP() << "imageView2DOn3DImage not supported, skipping test";
        }
    }

    constexpr unsigned image_depth = 2u;

    // 3D image
    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_info.imageType = VK_IMAGE_TYPE_3D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {64, 64, image_depth};
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image_3d{m_device};
    image_3d.init(&image_info);
    image_3d.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 2D image views for each slice of the 3D image
    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.image = image_3d.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView image_views[image_depth];
    VkImageView views[image_depth] = {VK_NULL_HANDLE};
    for (unsigned i = 0; i < image_depth; ++i) {
        view_info.subresourceRange.baseArrayLayer = i;
        image_views[i].init(*m_device, view_info);
        views[i] = image_views[i].handle();
    }

    // Render pass 1, referencing first slice
    VkAttachmentReference attachment_ref_1{};
    attachment_ref_1.attachment = 0;
    attachment_ref_1.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkSubpassDescription subpass_1{};
    subpass_1.pInputAttachments = &attachment_ref_1;
    subpass_1.inputAttachmentCount = 1;

    VkAttachmentDescription attach_desc_1{};
    attach_desc_1.format = image_info.format;
    attach_desc_1.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc_1.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attach_desc_1.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass_1;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attach_desc_1;
    vkt::RenderPass renderpass_1(*m_device, rp_info);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = renderpass_1.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &views[0];
    fb_info.width = image_info.extent.width;
    fb_info.height = image_info.extent.height;
    fb_info.layers = 1;
    vkt::Framebuffer framebuffer_1(*m_device, fb_info);

    VkRenderPassBeginInfo rp_begin_info_1 = vku::InitStructHelper();
    rp_begin_info_1.renderPass = renderpass_1.handle();
    rp_begin_info_1.framebuffer = framebuffer_1.handle();
    rp_begin_info_1.renderArea = {{0, 0}, {image_info.extent.width, image_info.extent.height}};

    // Render pass 2, referencing second slice
    VkAttachmentReference attachment_ref_2 = {};
    attachment_ref_2.attachment = 0;
    attachment_ref_2.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass_2 = {};
    subpass_2.pColorAttachments = &attachment_ref_2;
    subpass_2.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc_2{};
    attach_desc_2.format = image_info.format;
    attach_desc_2.samples = VK_SAMPLE_COUNT_1_BIT;
    // Since the previous render pass' framebuffer was using a 2D view of the first slice of the 3D image,
    // the layout transition should have applied to all the slices of the 3D image,
    // thus the 2nd image view created on the second slice of the 3D image should found its image layout to be
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    attach_desc_2.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attach_desc_2.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderPassCreateInfo rp_info_2 = vku::InitStructHelper();
    rp_info_2.subpassCount = 1;
    rp_info_2.pSubpasses = &subpass_2;
    rp_info_2.attachmentCount = 1;
    rp_info_2.pAttachments = &attach_desc_2;

    vkt::RenderPass renderpass_2(*m_device, rp_info_2);
    VkFramebufferCreateInfo fb_info_2 = vku::InitStructHelper();
    fb_info_2.renderPass = renderpass_2;
    fb_info_2.attachmentCount = 1;
    fb_info_2.pAttachments = &views[1];
    fb_info_2.width = image_info.extent.width;
    fb_info_2.height = image_info.extent.height;
    fb_info_2.layers = 1;
    vkt::Framebuffer framebuffer_2(*m_device, fb_info_2);

    VkRenderPassBeginInfo rp_begin_info_2 = vku::InitStructHelper();
    rp_begin_info_2.renderPass = renderpass_2;
    rp_begin_info_2.framebuffer = framebuffer_2;
    rp_begin_info_2.renderArea = rp_begin_info_1.renderArea;

    m_commandBuffer->begin();

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info_1, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info_2, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, SubpassWithReadOnlyLayoutWithoutDependency) {
    TEST_DESCRIPTION("When both subpasses' attachments are the same and layouts are read-only, they don't need dependency.");
    RETURN_IF_SKIP(Init())

    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    // A renderpass with one color attachment.
    VkAttachmentDescription attachment = {0,
                                          depth_format,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_STORE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
    const int size = 2;
    std::array<VkAttachmentDescription, size> attachments = {{attachment, attachment}};

    VkAttachmentReference att_ref_depth_stencil = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

    std::array<VkSubpassDescription, size> subpasses;
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, 0, nullptr, nullptr, &att_ref_depth_stencil, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, 0, nullptr, nullptr, &att_ref_depth_stencil, 0, nullptr};

    VkRenderPassCreateInfo rpci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, size, attachments.data(), size, subpasses.data(), 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depth_format,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    vkt::ImageView view(*m_device, ivci);
    std::array<VkImageView, size> views = {{view.handle(), view.handle()}};

    VkFramebufferCreateInfo fci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), size, views.data(), 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fci);

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->NextSubpass();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveRenderPass, SeparateDepthStencilSubresourceLayout) {
    TEST_DESCRIPTION("Test that separate depth stencil layouts are tracked correctly.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separate_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(separate_features);
    RETURN_IF_SKIP(InitState(nullptr, &separate_features));

    VkFormat ds_format = VK_FORMAT_D24_UNORM_S8_UINT;
    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), ds_format, &props);
    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
        ds_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        vk::GetPhysicalDeviceFormatProperties(gpu(), ds_format, &props);
        ASSERT_TRUE((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);
    }

    auto image_ci = vkt::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 6;
    image_ci.format = ds_format;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image;
    image.init(*m_device, image_ci);

    const auto depth_range = image.subresource_range(VK_IMAGE_ASPECT_DEPTH_BIT);
    const auto stencil_range = image.subresource_range(VK_IMAGE_ASPECT_STENCIL_BIT);
    const auto depth_stencil_range = image.subresource_range(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.image = image.handle();
    view_info.subresourceRange = depth_stencil_range;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    view_info.format = ds_format;
    vkt::ImageView view(*m_device, view_info);

    std::vector<VkImageMemoryBarrier> barriers;

    {
        m_commandBuffer->begin();
        auto depth_barrier =
            image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, depth_range);
        auto stencil_barrier =
            image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, stencil_range);
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &depth_barrier);
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &stencil_barrier);
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer(false);
        m_commandBuffer->reset();
    }

    m_commandBuffer->begin();

    // Test that we handle initial layout in command buffer.
    barriers.push_back(image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depth_stencil_range));

    // Test that we can transition aspects separately and use specific layouts.
    barriers.push_back(image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, depth_range));

    barriers.push_back(image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, stencil_range));

    // Test that transition from UNDEFINED on depth aspect does not clobber stencil layout.
    barriers.push_back(
        image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, depth_range));

    // Test that we can transition aspects separately and use combined layouts. (Only care about the aspect in question).
    barriers.push_back(image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, depth_range));

    barriers.push_back(image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, stencil_range));

    // Test that we can transition back again with combined layout.
    barriers.push_back(image.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depth_stencil_range));

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    VkRenderPassCreateInfo2 rp2 = vku::InitStructHelper();
    VkAttachmentDescription2 desc = vku::InitStructHelper();
    VkSubpassDescription2 sub = vku::InitStructHelper();
    VkAttachmentReference2 att = vku::InitStructHelper();
    VkAttachmentDescriptionStencilLayout stencil_desc = vku::InitStructHelper();
    VkAttachmentReferenceStencilLayout stencil_att = vku::InitStructHelper();
    // Test that we can discard stencil layout.
    stencil_desc.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    stencil_desc.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    stencil_att.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

    desc.format = ds_format;
    desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    desc.samples = VK_SAMPLE_COUNT_1_BIT;
    desc.pNext = &stencil_desc;

    att.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    att.attachment = 0;
    att.pNext = &stencil_att;

    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.pDepthStencilAttachment = &att;
    rp2.subpassCount = 1;
    rp2.pSubpasses = &sub;
    rp2.attachmentCount = 1;
    rp2.pAttachments = &desc;
    vkt::RenderPass render_pass_separate(*m_device, rp2, true);

    desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    desc.finalLayout = desc.initialLayout;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.pNext = nullptr;
    att.layout = desc.initialLayout;
    att.pNext = nullptr;
    vkt::RenderPass render_pass_combined(*m_device, rp2, true);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = render_pass_separate.handle();
    fb_info.width = 1;
    fb_info.height = 1;
    fb_info.layers = 1;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &view.handle();
    vkt::Framebuffer framebuffer_separate(*m_device, fb_info);

    fb_info.renderPass = render_pass_combined.handle();
    vkt::Framebuffer framebuffer_combined(*m_device, fb_info);

    for (auto &barrier : barriers) {
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &barrier);
    }

    rp_begin_info.renderPass = render_pass_separate.handle();
    rp_begin_info.framebuffer = framebuffer_separate.handle();
    rp_begin_info.renderArea.extent = {1, 1};
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();

    rp_begin_info.renderPass = render_pass_combined.handle();
    rp_begin_info.framebuffer = framebuffer_combined.handle();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
}

TEST_F(PositiveRenderPass, InputResolve) {
    TEST_DESCRIPTION("Create render pass where input attachment == resolve attachment");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitState())

    std::vector<VkAttachmentDescription> attachments = {
        // input attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    std::vector<VkAttachmentReference> input = {
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    std::vector<VkAttachmentReference> color = {
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    std::vector<VkAttachmentReference> resolve = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    VkSubpassDescription subpass = {0,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    (uint32_t)input.size(),
                                    input.data(),
                                    (uint32_t)color.size(),
                                    color.data(),
                                    resolve.data(),
                                    nullptr,
                                    0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   (uint32_t)attachments.size(),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported);
}
