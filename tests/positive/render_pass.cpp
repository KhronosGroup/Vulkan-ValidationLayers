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

TEST_F(VkPositiveLayerTest, RenderPassCreateAttachmentUsedTwiceOK) {
    TEST_DESCRIPTION("Attachment is used simultaneously as color and input, with the same layout. This is OK.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};
    VkRenderPass rp;

    m_errorMonitor->ExpectSuccess();
    vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyNotFound();
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassCreateInitialLayoutUndefined) {
    TEST_DESCRIPTION(
        "Ensure that CmdBeginRenderPass with an attachment's initialLayout of VK_IMAGE_LAYOUT_UNDEFINED works when the command "
        "buffer has prior knowledge of that attachment's layout.");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

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

    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

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
    VkImageView view;
    err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Record a single command buffer which uses this renderpass twice. The
    // bug is triggered at the beginning of the second renderpass, when the
    // command buffer already has a layout recorded for the attachment.
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyNotFound();

    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::DestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassCreateAttachmentLayoutWithLoadOpThenReadOnly) {
    TEST_DESCRIPTION(
        "Positive test where we create a renderpass with an attachment that uses LOAD_OP_CLEAR, the first subpass has a valid "
        "layout, and a second subpass then uses a valid *READ_ONLY* layout.");
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

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
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;

    // Now create RenderPass and verify no errors
    VkRenderPass rp;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyNotFound();

    vk::DestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkPositiveLayerTest, RenderPassBeginSubpassZeroTransitionsApplied) {
    TEST_DESCRIPTION("Ensure that CmdBeginRenderPass applies the layout transitions for the first subpass");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

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

    VkResult err;
    VkRenderPass rp;
    err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Record a single command buffer which issues a pipeline barrier w/
    // image memory barrier for the attachment. This detects the previously
    // missing tracking of the subpass layout by throwing a validation error
    // if it doesn't occur.
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassBeginTransitionsAttachmentUnused) {
    TEST_DESCRIPTION(
        "Ensure that layout transitions work correctly without errors, when an attachment reference is VK_ATTACHMENT_UNUSED");

    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    m_errorMonitor->ExpectSuccess();

    // A renderpass with no attachments
    VkAttachmentReference att_ref = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &subpass, 0, nullptr};

    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 0, nullptr, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Record a command buffer which just begins and ends the renderpass. The
    // bug manifests in BeginRenderPass.
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassBeginStencilLoadOp) {
    TEST_DESCRIPTION("Create a stencil-only attachment with a LOAD_OP set to CLEAR. stencil[Load|Store]Op used to be ignored.");
    VkResult result = VK_SUCCESS;
    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    VkImageFormatProperties formatProps;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0,
                                               &formatProps);
    if (formatProps.maxExtent.width < 100 || formatProps.maxExtent.height < 100) {
        printf("%s Image format max extent is too small.\n", kSkipPrefix);
        return;
    }

    VkFormat depth_stencil_fmt = depth_format;
    m_depthStencil->Init(m_device, 100, 100, depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    VkAttachmentDescription att = {};
    VkAttachmentReference ref = {};
    att.format = depth_stencil_fmt;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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

    VkRenderPass rp;
    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    result = vk::CreateRenderPass(device(), &rp_info, NULL, &rp);
    ASSERT_VK_SUCCESS(result);

    VkImageView *depthView = m_depthStencil->BindInfo();
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = depthView;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;
    VkFramebuffer fb;
    result = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    ASSERT_VK_SUCCESS(result);

    VkRenderPassBeginInfo rpbinfo = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = &clear;
    rpbinfo.renderPass = rp;
    rpbinfo.renderArea.extent.width = 100;
    rpbinfo.renderArea.extent.height = 100;
    rpbinfo.renderArea.offset.x = 0;
    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.framebuffer = fb;

    VkFenceObj fence;
    fence.init(*m_device, VkFenceObj::create_info());
    ASSERT_TRUE(fence.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(fence);

    VkImageObj destImage(m_device);
    destImage.Init(100, 100, 1, depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                   VK_IMAGE_TILING_OPTIMAL, 0);
    fence.wait(UINT64_MAX);
    VkCommandBufferObj cmdbuf(m_device, m_commandPool);
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
    cmdbuf.CopyImage(m_depthStencil->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destImage.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cregion);
    cmdbuf.end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmdbuf.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    vk::QueueWaitIdle(m_device->m_queue);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassBeginInlineAndSecondaryCommandBuffers) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyNotFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, RenderPassBeginDepthStencilLayoutTransitionFromUndefined) {
    TEST_DESCRIPTION(
        "Create a render pass with depth-stencil attachment where layout transition from UNDEFINED TO DS_READ_ONLY_OPTIMAL is set "
        "by render pass and verify that transition has correctly occurred at queue submit time with no validation errors.");

    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    VkImageFormatProperties format_props;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &format_props);
    if (format_props.maxExtent.width < 32 || format_props.maxExtent.height < 32) {
        printf("%s Depth extent too small, RenderPassDepthStencilLayoutTransition skipped.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);
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
    VkImageView view;
    err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyNotFound();

    // Cleanup
    vk::DestroyImageView(m_device->device(), view, NULL);
    vk::DestroyRenderPass(m_device->device(), rp, NULL);
    vk::DestroyFramebuffer(m_device->device(), fb, NULL);
}

TEST_F(VkPositiveLayerTest, DestroyPipelineRenderPass) {
    TEST_DESCRIPTION("Draw using a pipeline whose create renderPass has been destroyed.");
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    m_renderPassClearValues.clear();
    VkClearValue clear = {};
    clear.color = m_clear_color;

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

    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    VkRenderPass rp;
    err = vk::CreateRenderPass(device(), &rp_info, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {{0, 0}, {64, 64}};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    const VkPipelineLayoutObj pl(m_device);
    pipe.CreateVKPipeline(pl.handle(), rp);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // Destroy renderPass before pipeline is used in Draw
    //  We delay until after CmdBindPipeline to verify that invalid binding isn't
    //  created between CB & renderPass, which we used to do.
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkPositiveLayerTest, ImagelessFramebufferNonZeroBaseMip) {
    TEST_DESCRIPTION("Use a 1D image view for an imageless framebuffer with base mip level > 0.");
    m_errorMonitor->ExpectSuccess();

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    auto pd_imageless_fb_features = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    pd_imageless_fb_features.imagelessFramebuffer = VK_TRUE;
    auto pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&pd_imageless_fb_features);
    if (!InitFrameworkAndRetrieveFeatures(pd_features2)) {
        GTEST_SKIP() << "Failed to initialize physical device and query features";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (pd_imageless_fb_features.imagelessFramebuffer != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceImagelessFramebufferFeaturesKHR::imagelessFramebuffer feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

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

    VkRenderPassCreateInfo rp_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    vk_testing::RenderPass rp(*m_device, rp_ci);

    auto fb_attachment_image_info = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    fb_attachment_image_info.width = width;
    fb_attachment_image_info.height = height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 2;
    fb_attachment_image_info.pViewFormats = fb_attachments;
    fb_attachment_image_info.height = 1;
    fb_attachment_image_info.width = width >> base_mip;

    auto fb_attachments_ci = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    fb_attachments_ci.attachmentImageInfoCount = 1;
    fb_attachments_ci.pAttachmentImageInfos = &fb_attachment_image_info;

    auto fb_ci = LvlInitStruct<VkFramebufferCreateInfo>(&fb_attachments_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    fb_ci.width = width >> base_mip;
    fb_ci.height = height;
    fb_ci.layers = 1;
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = nullptr;
    fb_ci.renderPass = rp.handle();
    vk_testing::Framebuffer fb(*m_device, fb_ci);
    ASSERT_TRUE(fb.initialized());

    auto image_ci = LvlInitStruct<VkImageCreateInfo>();
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

    auto image_view_ci = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_ci.image = image;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    image_view_ci.format = formats[0];
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseMipLevel = base_mip;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView image_view_obj(*m_device, image_view_ci);
    VkImageView image_view = image_view_obj.handle();

    auto rp_attachment_begin_info = LvlInitStruct<VkRenderPassAttachmentBeginInfoKHR>();
    rp_attachment_begin_info.attachmentCount = 1;
    rp_attachment_begin_info.pAttachments = &image_view;
    auto rp_begin_info = LvlInitStruct<VkRenderPassBeginInfo>(&rp_attachment_begin_info);
    rp_begin_info.renderPass = rp.handle();
    rp_begin_info.renderArea.extent.width = width >> base_mip;
    rp_begin_info.renderArea.extent.height = height;
    rp_begin_info.framebuffer = fb.handle();

    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    m_commandBuffer->begin(&cmd_begin_info);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

TEST_F(VkPositiveLayerTest, RenderPassValidStages) {
    TEST_DESCRIPTION("Create render pass with valid stages");

    bool rp2_supported = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (rp2_supported) m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (rp2_supported) rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription sci[2] = {};
    sci[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    // to be filled later by tests

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
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
    PositiveTestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported);

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = kGraphicsStages | VK_PIPELINE_STAGE_HOST_BIT;
    dependency.dstStageMask = kGraphicsStages;
    PositiveTestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported);

    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = kGraphicsStages;
    dependency.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    PositiveTestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported);
}

TEST_F(VkPositiveLayerTest, RenderPassSingleMipTransition) {
    TEST_DESCRIPTION("Ensure that the validation message contains the correct miplevel");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

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

    VkResult err;
    VkRenderPass rp;
    err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

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

    vk_testing::ImageView fullView0;
    fullView0.init(*m_device, vinfo);

    vinfo.image = depthImage.handle();
    vinfo.format = VK_FORMAT_D32_SFLOAT;
    vinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vk_testing::ImageView fullView1;
    fullView1.init(*m_device, vinfo);

    VkImageView fullViews[] = {fullView0.handle(), fullView1.handle()};

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 2, baseViews, 32, 32, 1};

    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Create shader modules

    char const fsSource[] = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0, binding=2) uniform sampler2D depth;
        void main() {
           x = texture(depth, vec2(0));
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create descriptor set and friends.

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkSampler sampler = VK_NULL_HANDLE;
    err = vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet::Bindings binding_defs = {{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    const VkDescriptorSetLayoutObj pipeline_dsl(m_device, binding_defs);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&pipeline_dsl});
    OneOffDescriptorSet descriptor_set(m_device, binding_defs);

    VkDescriptorImageInfo image_info = {
        sampler,
        fullViews[1],
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet descriptor_writes[1] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
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

    m_viewports.push_back(VkViewport{0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f});
    m_scissors.push_back(VkRect2D{{0, 0}, {64, 64}});

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.AddDefaultColorAttachment();
    pipe.SetDepthStencil(&ds_ci);

    pipe.CreateVKPipeline(pipeline_layout.handle(), rp);

    // Start pushing commands.

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, nullptr);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    vk::CmdEndRenderPass(m_commandBuffer->handle());

    // At this point the first miplevel should be in GENERAL due to the "finalLayout" in the render pass.
    // Note that these image barriers attempt to transition *all* miplevels, even though only 1 miplevel has transitioned.

    colorImage.Layout(VK_IMAGE_LAYOUT_GENERAL);

    colorImage.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    m_errorMonitor->VerifyNotFound();

    depthImage.Layout(VK_IMAGE_LAYOUT_GENERAL);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "cannot transition the layout of aspect=2 level=1 layer=0");

    depthImage.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_DEPTH_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::DestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(VkPositiveLayerTest, CreateRenderPassWithViewMask) {
    TEST_DESCRIPTION("Create render pass with view mask, with multiview feature enabled in Vulkan11Features.");

    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vulkan_11_features = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vulkan_11_features);

    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (vulkan_11_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCreateRenderPass2KHR =
        reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR"));

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.viewMask = 0x1;

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo2>();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    vkCreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyNotFound();
}
