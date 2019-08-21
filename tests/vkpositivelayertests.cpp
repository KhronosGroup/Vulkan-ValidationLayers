/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google, Inc.
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

#include "cast_utils.h"
#include "layer_validation_tests.h"
//
// POSITIVE VALIDATION TESTS
//
// These tests do not expect to encounter ANY validation errors pass only if this is true

TEST_F(VkPositiveLayerTest, NullFunctionPointer) {
    TEST_DESCRIPTION("On 1_0 instance , call GetDeviceProcAddr on promoted 1_1 device-level entrypoint");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, "VK_KHR_get_memory_requirements2")) {
        m_device_extension_names.push_back("VK_KHR_get_memory_requirements2");
    } else {
        printf("%s VK_KHR_get_memory_reqirements2 extension not supported, skipping NullFunctionPointer test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess();
    auto fpGetBufferMemoryRequirements =
        (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(m_device->device(), "vkGetBufferMemoryRequirements2");
    if (fpGetBufferMemoryRequirements) {
        m_errorMonitor->SetError("Null was expected!");
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, SecondaryCommandBufferBarrier) {
    TEST_DESCRIPTION("Add a pipeline barrier in a secondary command buffer");
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->ExpectSuccess();

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

    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
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

    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

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
    vkBeginCommandBuffer(secondary.handle(), &cbbi);
    VkMemoryBarrier mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mem_barrier.pNext = NULL;
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vkCmdPipelineBarrier(secondary.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr, 0, nullptr);

    image.ImageMemoryBarrier(&secondary, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    secondary.end();

    vkCmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    m_errorMonitor->VerifyNotFound();
}

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
    vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyNotFound();
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
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
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
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
    err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Record a single command buffer which uses this renderpass twice. The
    // bug is triggered at the beginning of the second renderpass, when the
    // command buffer already has a layout recorded for the attachment.
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyNotFound();

    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkDestroyImageView(m_device->device(), view, nullptr);
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
    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;

    // Now create RenderPass and verify no errors
    VkRenderPass rp;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyNotFound();

    vkDestroyRenderPass(m_device->device(), rp, NULL);
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
    err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Record a single command buffer which issues a pipeline barrier w/
    // image memory barrier for the attachment. This detects the previously
    // missing tracking of the subpass layout by throwing a validation error
    // if it doesn't occur.
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassBeginTransitionsAttachmentUnused) {
    TEST_DESCRIPTION(
        "Ensure that layout transitions work correctly without errors, when an attachment reference is VK_ATTACHMENT_UNUSED");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    // A renderpass with no attachments
    VkAttachmentReference att_ref = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &subpass, 0, nullptr};

    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 0, nullptr, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Record a command buffer which just begins and ends the renderpass. The
    // bug manifests in BeginRenderPass.
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
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
    vkGetPhysicalDeviceImageFormatProperties(gpu(), depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
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
    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    result = vkCreateRenderPass(device(), &rp_info, NULL, &rp);
    ASSERT_VK_SUCCESS(result);

    VkImageView *depthView = m_depthStencil->BindInfo();
    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = depthView;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;
    VkFramebuffer fb;
    result = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    ASSERT_VK_SUCCESS(result);

    VkRenderPassBeginInfo rpbinfo = {};
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = &clear;
    rpbinfo.pNext = NULL;
    rpbinfo.renderPass = rp;
    rpbinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
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
    fence.wait(VK_TRUE, UINT64_MAX);
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

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmdbuf.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    m_errorMonitor->ExpectSuccess();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    vkQueueWaitIdle(m_device->m_queue);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(VkPositiveLayerTest, RenderPassBeginInlineAndSecondaryCommandBuffers) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyNotFound();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyNotFound();
    vkCmdEndRenderPass(m_commandBuffer->handle());
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
    vkGetPhysicalDeviceImageFormatProperties(gpu(), depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
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
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
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
    err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyNotFound();

    // Cleanup
    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroyRenderPass(m_device->device(), rp, NULL);
    vkDestroyFramebuffer(m_device->device(), fb, NULL);
}

TEST_F(VkPositiveLayerTest, DestroyPipelineRenderPass) {
    TEST_DESCRIPTION("Draw using a pipeline whose create renderPass has been destroyed.");
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
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

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    VkRenderPass rp;
    err = vkCreateRenderPass(device(), &rp_info, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // Destroy renderPass before pipeline is used in Draw
    //  We delay until after CmdBindPipeline to verify that invalid binding isn't
    //  created between CB & renderPass, which we used to do.
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();
    vkQueueWaitIdle(m_device->m_queue);
}

TEST_F(VkPositiveLayerTest, BasicQuery) {
    TEST_DESCRIPTION("Use a couple occlusion queries");
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.size = 4 * sizeof(uint64_t);
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.init(*m_device, bci, mem_props);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_info;
    query_pool_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_info.pNext = NULL;
    query_pool_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = 2;
    query_pool_info.pipelineStatistics = 0;

    VkResult res = vkCreateQueryPool(m_device->handle(), &query_pool_info, NULL, &query_pool);
    ASSERT_VK_SUCCESS(res);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vkCmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 2);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vkCmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vkCmdBeginQuery(m_commandBuffer->handle(), query_pool, 1, 0);
    vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    vkCmdEndQuery(m_commandBuffer->handle(), query_pool, 1);
    vkCmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 2, buffer.handle(), 0, sizeof(uint64_t),
                              VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vkQueueWaitIdle(m_device->m_queue);
    uint64_t samples_passed[4];
    res = vkGetQueryPoolResults(m_device->handle(), query_pool, 0, 2, sizeof(samples_passed), samples_passed, sizeof(uint64_t),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    ASSERT_VK_SUCCESS(res);
    m_errorMonitor->VerifyNotFound();
    vkDestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkPositiveLayerTest, MultiplaneGetImageSubresourceLayout) {
    TEST_DESCRIPTION("Positive test, query layout of a single plane of a multiplane image. (repro Github #2530)");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_LINEAR;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    VkImage image;
    VkResult err = vkCreateImage(device(), &ci, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    // Query layout of 3rd plane
    VkImageSubresource subres = {};
    subres.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    subres.mipLevel = 0;
    subres.arrayLayer = 0;
    VkSubresourceLayout layout = {};

    m_errorMonitor->ExpectSuccess();
    vkGetImageSubresourceLayout(device(), image, &subres, &layout);
    m_errorMonitor->VerifyNotFound();

    vkDestroyImage(device(), image, NULL);
}

TEST_F(VkPositiveLayerTest, OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

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
    auto image_barrier = image.image_memory_barrier(0, 0, image.Layout(), image.Layout(), image_subres);
    image_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.dstQueueFamilyIndex = no_gfx;

    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, nullptr, &image_barrier);

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx;
    image_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.oldLayout = image.Layout();
    // Make sure the new layout is different from the old
    if (image_barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, nullptr, &image_barrier);
}

TEST_F(VkPositiveLayerTest, OwnershipTranfersBuffer) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

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
    auto buffer_barrier = buffer.buffer_memory_barrier(0, 0, 0, VK_WHOLE_SIZE);

    // Let gfx own it.
    buffer_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ValidOwnershipTransferOp(m_errorMonitor, m_commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             &buffer_barrier, nullptr);

    // Transfer it to non-gfx
    buffer_barrier.dstQueueFamilyIndex = no_gfx;
    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, &buffer_barrier, nullptr);

    // Transfer it to gfx
    buffer_barrier.srcQueueFamilyIndex = no_gfx;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, &buffer_barrier, nullptr);
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

    VkImageMemoryBarrier barrier = {};
    VkImageSubresourceRange range;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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

TEST_F(VkPositiveLayerTest, CopyNonupdatedDescriptors) {
    TEST_DESCRIPTION("Copy non-updated descriptors");
    unsigned int i;

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet src_descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                         {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                         {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });
    OneOffDescriptorSet dst_descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                         {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

    m_errorMonitor->ExpectSuccess();

    const unsigned int copy_size = 2;
    VkCopyDescriptorSet copy_ds_update[copy_size];
    memset(copy_ds_update, 0, sizeof(copy_ds_update));
    for (i = 0; i < copy_size; i++) {
        copy_ds_update[i].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        copy_ds_update[i].srcSet = src_descriptor_set.set_;
        copy_ds_update[i].srcBinding = i;
        copy_ds_update[i].dstSet = dst_descriptor_set.set_;
        copy_ds_update[i].dstBinding = i;
        copy_ds_update[i].descriptorCount = 1;
    }
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, copy_size, copy_ds_update);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ConfirmNoVLErrorWhenVkCmdClearAttachmentsCalledInSecondaryCB) {
    TEST_DESCRIPTION(
        "This test is to verify that when vkCmdClearAttachments is called by a secondary commandbuffer, the validation layers do "
        "not throw an error if the primary commandbuffer begins a renderpass before executing the secondary commandbuffer.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferBeginInfo info = {};
    VkCommandBufferInheritanceInfo hinfo = {};
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pInheritanceInfo = &hinfo;
    hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    hinfo.pNext = NULL;
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
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};
    vkCmdClearAttachments(secondary.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary.end();
    // Modify clear rect here to verify that it doesn't cause validation error
    clear_rect = {{{0, 0}, {99999999, 99999999}}, 0, 0};

    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineComplexTypes) {
    TEST_DESCRIPTION("Smoke test for complex types across VS/FS boundary");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().tessellationShader) {
        printf("%s Device does not support tessellation shaders; skipped.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj tcs(m_device, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj tes(m_device, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};
    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderRelaxedBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to relax block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires relaxed block layout.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // The Relaxed Block Layout extension was promoted to core in 1.1.
    // Go ahead and check for it and turn it on in case a 1.0 device has it.
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix, VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring relaxed layout.
    // Without relaxed layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const std::string spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";
    m_errorMonitor->ExpectSuccess();
    VkShaderObj vs(m_device, spv_source, VK_SHADER_STAGE_VERTEX_BIT, this);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderUboStd430Layout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires UBO std430 layout.");
    // Enable req'd extensions
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for the UBO standard block layout extension and turn it on if it's available
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto uniform_buffer_standard_layout_features = lvl_init_struct<VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR>(NULL);
    uniform_buffer_standard_layout_features.uniformBufferStandardLayout = VK_TRUE;
    auto query_features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&uniform_buffer_standard_layout_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &query_features2);

    auto set_features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&uniform_buffer_standard_layout_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &set_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring std430 in a uniform buffer.
    // Without uniform buffer standard layout, we would expect a message like:
    // "Structure id 3 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 0 is an array
    // with stride 4 not satisfying alignment to 16"

    const std::string spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 460
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %foo 0 Offset 0
               OpDecorate %foo Block
               OpDecorate %b DescriptorSet 0
               OpDecorate %b Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
        %foo = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
          %b = OpVariable %_ptr_Uniform_foo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    std::vector<unsigned int> spv;
    VkShaderModuleCreateInfo module_create_info;
    VkShaderModule shader_module;
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.pNext = NULL;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, spv_source.data(), spv);
    module_create_info.pCode = spv.data();
    module_create_info.codeSize = spv.size() * sizeof(unsigned int);
    module_create_info.flags = 0;

    m_errorMonitor->ExpectSuccess();
    VkResult err = vkCreateShaderModule(m_device->handle(), &module_create_info, NULL, &shader_module);
    m_errorMonitor->VerifyNotFound();
    if (err == VK_SUCCESS) {
        vkDestroyShaderModule(m_device->handle(), shader_module, NULL);
    }
}

TEST_F(VkPositiveLayerTest, ShaderScalarBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires scalar block layout.");
    // Enable req'd extensions
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for the Scalar Block Layout extension and turn it on if it's available
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto scalar_block_features = lvl_init_struct<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT>(NULL);
    scalar_block_features.scalarBlockLayout = VK_TRUE;
    auto query_features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&scalar_block_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &query_features2);

    auto set_features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&scalar_block_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &set_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring scalar layout.
    // Without scalar layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const std::string spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpMemberDecorate %S 2 Offset 8
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";

    m_errorMonitor->ExpectSuccess();
    VkShaderObj vs(m_device, spv_source, VK_SHADER_STAGE_VERTEX_BIT, this);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, SpirvGroupDecorations) {
    TEST_DESCRIPTION("Test shader validation support for group decorations.");
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string spv_source = R"(
              OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpName %main "main"
               OpName %gl_GlobalInvocationID "gl_GlobalInvocationID"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %_runtimearr_float ArrayStride 4
               OpDecorate %4 BufferBlock
               OpDecorate %5 Offset 0
          %4 = OpDecorationGroup
          %5 = OpDecorationGroup
               OpGroupDecorate %4 %_struct_6 %_struct_7 %_struct_8 %_struct_9 %_struct_10 %_struct_11
               OpGroupMemberDecorate %5 %_struct_6 0 %_struct_7 0 %_struct_8 0 %_struct_9 0 %_struct_10 0 %_struct_11 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 NonWritable
               OpDecorate %13 Restrict
         %14 = OpDecorationGroup
         %12 = OpDecorationGroup
         %13 = OpDecorationGroup
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpDecorate %15 DescriptorSet 0
               OpDecorate %15 Binding 5
               OpGroupDecorate %14 %16
               OpDecorate %16 DescriptorSet 0
               OpDecorate %16 Binding 0
               OpGroupDecorate %12 %17
               OpDecorate %17 Binding 1
               OpGroupDecorate %13 %18 %19
               OpDecorate %18 Binding 2
               OpDecorate %19 Binding 3
               OpGroupDecorate %14 %20
               OpGroupDecorate %12 %20
               OpGroupDecorate %13 %20
               OpDecorate %20 Binding 4
       %bool = OpTypeBool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %v3uint = OpTypeVector %uint 3
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_runtimearr_int = OpTypeRuntimeArray %int
%_runtimearr_float = OpTypeRuntimeArray %float
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
      %int_0 = OpConstant %int 0
  %_struct_6 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
         %15 = OpVariable %_ptr_Uniform__struct_6 Uniform
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
         %16 = OpVariable %_ptr_Uniform__struct_7 Uniform
  %_struct_8 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_8 = OpTypePointer Uniform %_struct_8
         %17 = OpVariable %_ptr_Uniform__struct_8 Uniform
  %_struct_9 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
         %18 = OpVariable %_ptr_Uniform__struct_9 Uniform
 %_struct_10 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %19 = OpVariable %_ptr_Uniform__struct_10 Uniform
 %_struct_11 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_11 = OpTypePointer Uniform %_struct_11
         %20 = OpVariable %_ptr_Uniform__struct_11 Uniform
       %main = OpFunction %void None %23
         %40 = OpLabel
         %41 = OpLoad %v3uint %gl_GlobalInvocationID
         %42 = OpCompositeExtract %uint %41 0
         %43 = OpAccessChain %_ptr_Uniform_float %16 %int_0 %42
         %44 = OpAccessChain %_ptr_Uniform_float %17 %int_0 %42
         %45 = OpAccessChain %_ptr_Uniform_float %18 %int_0 %42
         %46 = OpAccessChain %_ptr_Uniform_float %19 %int_0 %42
         %47 = OpAccessChain %_ptr_Uniform_float %20 %int_0 %42
         %48 = OpAccessChain %_ptr_Uniform_float %15 %int_0 %42
         %49 = OpLoad %float %43
         %50 = OpLoad %float %44
         %51 = OpLoad %float %45
         %52 = OpLoad %float %46
         %53 = OpLoad %float %47
         %54 = OpFAdd %float %49 %50
         %55 = OpFAdd %float %54 %51
         %56 = OpFAdd %float %55 %52
         %57 = OpFAdd %float %56 %53
               OpStore %48 %57
               OpReturn
               OpFunctionEnd
)";

    // CreateDescriptorSetLayout
    VkDescriptorSetLayoutBinding dslb[6] = {};
    size_t dslb_size = size(dslb);
    for (size_t i = 0; i < dslb_size; i++) {
        dslb[i].binding = i;
        dslb[i].descriptorCount = 1;
        dslb[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[i].pImmutableSamplers = NULL;
        dslb[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_ALL;
    }
    if (m_device->props.limits.maxPerStageDescriptorStorageBuffers < dslb_size) {
        printf("%sNeeded storage buffer bindings exceeds this devices limit.  Skipping tests.\n", kSkipPrefix);
        return;
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(dslb_size);
    memcpy(pipe.dsl_bindings_.data(), dslb, dslb_size * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(m_device, bindStateMinimalShaderText, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineCheckShaderCapabilityExtension1of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 1 of 2");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    // These tests require that the device support multiViewport
    if (!m_device->phy().features().multiViewport) {
        printf("%s Device does not support multiViewport, test skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader using viewport array capability
    char const *vsSource =
        "#version 450\n"
        "#extension GL_ARB_shader_viewport_layer_array : enable\n"
        "void main() {\n"
        "    gl_ViewportIndex = 1;\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineCheckShaderCapabilityExtension2of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 2 of 2");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    // These tests require that the device support multiViewport
    if (!m_device->phy().features().multiViewport) {
        printf("%s Device does not support multiViewport, test skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader using viewport array capability
    char const *vsSource =
        "#version 450\n"
        "#extension GL_ARB_shader_viewport_layer_array : enable\n"
        "void main() {\n"
        "    gl_ViewportIndex = 1;\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineFragmentOutputNotWrittenButMasked) {
    TEST_DESCRIPTION(
        "Test that no error is produced when the fragment shader fails to declare an output, but the corresponding attachment's "
        "write mask is 0.");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *fsSource =
        "#version 450\n"
        "\n"
        "void main(){\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    /* set up CB 0, not written, but also masked */
    pipe.AddDefaultColorAttachment(0);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, StatelessValidationDisable) {
    TEST_DESCRIPTION("Specify a non-zero value for a reserved parameter with stateless validation disabled");

    VkValidationFeatureDisableEXT disables[] = {VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.disabledValidationFeatureCount = 1;
    features.pDisabledValidationFeatures = disables;
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, pool_flags, &features));

    m_errorMonitor->ExpectSuccess();
    // Specify 0 for a reserved VkFlags parameter. Normally this is expected to trigger an stateless validation error, but this
    // validation was disabled via the features extension, so no errors should be forthcoming.
    VkEvent event_handle = VK_NULL_HANDLE;
    VkEventCreateInfo event_info = {};
    event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    event_info.flags = 1;
    vkCreateEvent(device(), &event_info, NULL, &event_handle);
    vkDestroyEvent(device(), event_handle, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, PointSizeWriteInFunction) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize in vertex shader function.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(m_device, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj ps(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, PointSizeGeomShaderSuccess) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, and write in the final geometry stage.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();

    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        printf("%s Device does not support the required geometry shader features; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and writing to it
    VkShaderObj vs(m_device, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj gs(m_device, bindStateGeomPointSizeShaderText, VK_SHADER_STAGE_GEOMETRY_BIT, this);
    VkShaderObj ps(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
    // Set Input Assembly to TOPOLOGY POINT LIST
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, LoosePointSizeWrite) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize outside of a structure.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    const std::string LoosePointSizeWrite = R"(
                                       OpCapability Shader
                                  %1 = OpExtInstImport "GLSL.std.450"
                                       OpMemoryModel Logical GLSL450
                                       OpEntryPoint Vertex %main "main" %glposition %glpointsize %gl_VertexIndex
                                       OpSource GLSL 450
                                       OpName %main "main"
                                       OpName %vertices "vertices"
                                       OpName %glposition "glposition"
                                       OpName %glpointsize "glpointsize"
                                       OpName %gl_VertexIndex "gl_VertexIndex"
                                       OpDecorate %glposition BuiltIn Position
                                       OpDecorate %glpointsize BuiltIn PointSize
                                       OpDecorate %gl_VertexIndex BuiltIn VertexIndex
                               %void = OpTypeVoid
                                  %3 = OpTypeFunction %void
                              %float = OpTypeFloat 32
                            %v2float = OpTypeVector %float 2
                               %uint = OpTypeInt 32 0
                             %uint_3 = OpConstant %uint 3
                %_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %_ptr_Private__arr_v2float_uint_3 = OpTypePointer Private %_arr_v2float_uint_3
                           %vertices = OpVariable %_ptr_Private__arr_v2float_uint_3 Private
                                %int = OpTypeInt 32 1
                              %int_0 = OpConstant %int 0
                           %float_n1 = OpConstant %float -1
                                 %16 = OpConstantComposite %v2float %float_n1 %float_n1
               %_ptr_Private_v2float = OpTypePointer Private %v2float
                              %int_1 = OpConstant %int 1
                            %float_1 = OpConstant %float 1
                                 %21 = OpConstantComposite %v2float %float_1 %float_n1
                              %int_2 = OpConstant %int 2
                            %float_0 = OpConstant %float 0
                                 %25 = OpConstantComposite %v2float %float_0 %float_1
                            %v4float = OpTypeVector %float 4
            %_ptr_Output_gl_Position = OpTypePointer Output %v4float
                         %glposition = OpVariable %_ptr_Output_gl_Position Output
           %_ptr_Output_gl_PointSize = OpTypePointer Output %float
                        %glpointsize = OpVariable %_ptr_Output_gl_PointSize Output
                     %_ptr_Input_int = OpTypePointer Input %int
                     %gl_VertexIndex = OpVariable %_ptr_Input_int Input
                              %int_3 = OpConstant %int 3
                %_ptr_Output_v4float = OpTypePointer Output %v4float
                  %_ptr_Output_float = OpTypePointer Output %float
                               %main = OpFunction %void None %3
                                  %5 = OpLabel
                                 %18 = OpAccessChain %_ptr_Private_v2float %vertices %int_0
                                       OpStore %18 %16
                                 %22 = OpAccessChain %_ptr_Private_v2float %vertices %int_1
                                       OpStore %22 %21
                                 %26 = OpAccessChain %_ptr_Private_v2float %vertices %int_2
                                       OpStore %26 %25
                                 %33 = OpLoad %int %gl_VertexIndex
                                 %35 = OpSMod %int %33 %int_3
                                 %36 = OpAccessChain %_ptr_Private_v2float %vertices %35
                                 %37 = OpLoad %v2float %36
                                 %38 = OpCompositeExtract %float %37 0
                                 %39 = OpCompositeExtract %float %37 1
                                 %40 = OpCompositeConstruct %v4float %38 %39 %float_0 %float_1
                                 %42 = OpAccessChain %_ptr_Output_v4float %glposition
                                       OpStore %42 %40
                                       OpStore %glpointsize %float_1
                                       OpReturn
                                       OpFunctionEnd
        )";

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(m_device, LoosePointSizeWrite, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj ps(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        // Set Input Assembly to TOPOLOGY POINT LIST
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, UncompressedToCompressedImageCopy) {
    TEST_DESCRIPTION("Image copies between compressed and uncompressed images");
    ASSERT_NO_FATAL_FAILURE(Init());

    // Verify format support
    // Size-compatible (64-bit) formats. Uncompressed is 64 bits per texel, compressed is 64 bits per 4x4 block (or 4bpt).
    if (!ImageFormatAndFeaturesSupported(gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR) ||
        !ImageFormatAndFeaturesSupported(gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR)) {
        printf("%s Required formats/features not supported - UncompressedToCompressedImageCopy skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj uncomp_10x10t_image(m_device);       // Size = 10 * 10 * 64 = 6400
    VkImageObj comp_10x10b_40x40t_image(m_device);  // Size = 40 * 40 * 4  = 6400

    uncomp_10x10t_image.Init(10, 10, 1, VK_FORMAT_R16G16B16A16_UINT,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    comp_10x10b_40x40t_image.Init(40, 40, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);

    if (!uncomp_10x10t_image.initialized() || !comp_10x10b_40x40t_image.initialized()) {
        printf("%s Unable to initialize surfaces - UncompressedToCompressedImageCopy skipped.\n", kSkipPrefix);
        return;
    }

    // Both copies represent the same number of bytes. Bytes Per Texel = 1 for bc6, 16 for uncompressed
    // Copy compressed to uncompressed
    VkImageCopy copy_region = {};
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
    m_commandBuffer->begin();

    // Copy from uncompressed to compressed
    copy_region.extent = {10, 10, 1};  // Dimensions in (uncompressed) texels
    vkCmdCopyImage(m_commandBuffer->handle(), uncomp_10x10t_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                   comp_10x10b_40x40t_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // And from compressed to uncompressed
    copy_region.extent = {40, 40, 1};  // Dimensions in (compressed) texels
    vkCmdCopyImage(m_commandBuffer->handle(), comp_10x10b_40x40t_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                   uncomp_10x10t_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, DeleteDescriptorSetLayoutsBeforeDescriptorSets) {
    TEST_DESCRIPTION("Create DSLayouts and DescriptorSets and then delete the DSLayouts before the DescriptorSets.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkResult err;

    m_errorMonitor->ExpectSuccess();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool_one;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool_one);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    VkDescriptorSet descriptorSet;
    {
        const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = ds_pool_one;
        alloc_info.pSetLayouts = &ds_layout.handle();
        err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);
        ASSERT_VK_SUCCESS(err);
    }  // ds_layout destroyed
    err = vkFreeDescriptorSets(m_device->device(), ds_pool_one, 1, &descriptorSet);

    vkDestroyDescriptorPool(m_device->device(), ds_pool_one, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CommandPoolDeleteWithReferences) {
    TEST_DESCRIPTION("Ensure the validation layers bookkeeping tracks the implicit command buffer frees.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = NULL;
    cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_info.flags = 0;

    VkCommandPool secondary_cmd_pool;
    VkResult res = vkCreateCommandPool(m_device->handle(), &cmd_pool_info, NULL, &secondary_cmd_pool);
    ASSERT_VK_SUCCESS(res);

    VkCommandBufferAllocateInfo cmdalloc = vk_testing::CommandBuffer::create_info(secondary_cmd_pool);
    cmdalloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    VkCommandBuffer secondary_cmds;
    res = vkAllocateCommandBuffers(m_device->handle(), &cmdalloc, &secondary_cmds);

    VkCommandBufferInheritanceInfo cmd_buf_inheritance_info = {};
    cmd_buf_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cmd_buf_inheritance_info.pNext = NULL;
    cmd_buf_inheritance_info.renderPass = VK_NULL_HANDLE;
    cmd_buf_inheritance_info.subpass = 0;
    cmd_buf_inheritance_info.framebuffer = VK_NULL_HANDLE;
    cmd_buf_inheritance_info.occlusionQueryEnable = VK_FALSE;
    cmd_buf_inheritance_info.queryFlags = 0;
    cmd_buf_inheritance_info.pipelineStatistics = 0;

    VkCommandBufferBeginInfo secondary_begin = {};
    secondary_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    secondary_begin.pNext = NULL;
    secondary_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    secondary_begin.pInheritanceInfo = &cmd_buf_inheritance_info;

    res = vkBeginCommandBuffer(secondary_cmds, &secondary_begin);
    ASSERT_VK_SUCCESS(res);
    vkEndCommandBuffer(secondary_cmds);

    m_commandBuffer->begin();
    vkCmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmds);
    m_commandBuffer->end();

    // DestroyCommandPool *implicitly* frees the command buffers allocated from it
    vkDestroyCommandPool(m_device->handle(), secondary_cmd_pool, NULL);
    // If bookkeeping has been lax, validating the reset will attempt to touch deleted data
    res = vkResetCommandPool(m_device->handle(), m_commandPool->handle(), 0);
    ASSERT_VK_SUCCESS(res);
}

TEST_F(VkPositiveLayerTest, SecondaryCommandBufferClearColorAttachments) {
    TEST_DESCRIPTION("Create a secondary command buffer and record a CmdClearAttachments call into it");
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer));
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = {};
    command_buffer_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    command_buffer_inheritance_info.renderPass = m_renderPass;
    command_buffer_inheritance_info.framebuffer = m_framebuffer;

    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    vkBeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vkCmdClearAttachments(secondary_command_buffer, 1, &color_attachment, 1, &clear_rect);
    vkEndCommandBuffer(secondary_command_buffer);
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_command_buffer);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, SecondaryCommandBufferImageLayoutTransitions) {
    TEST_DESCRIPTION("Perform an image layout transition in a secondary command buffer followed by a transition in the primary.");
    VkResult err;
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s Couldn't find depth stencil format.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    // Allocate a secondary and primary cmd buffer
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer));
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer primary_command_buffer;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &primary_command_buffer));
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = {};
    command_buffer_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    err = vkBeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    ASSERT_VK_SUCCESS(err);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
    vkCmdPipelineBarrier(secondary_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &img_barrier);
    err = vkEndCommandBuffer(secondary_command_buffer);
    ASSERT_VK_SUCCESS(err);

    // Now update primary cmd buffer to execute secondary and transitions image
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    err = vkBeginCommandBuffer(primary_command_buffer, &command_buffer_begin_info);
    ASSERT_VK_SUCCESS(err);
    vkCmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);
    VkImageMemoryBarrier img_barrier2 = {};
    img_barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
    vkCmdPipelineBarrier(primary_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &img_barrier2);
    err = vkEndCommandBuffer(primary_command_buffer);
    ASSERT_VK_SUCCESS(err);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &primary_command_buffer;
    err = vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    err = vkDeviceWaitIdle(m_device->device());
    ASSERT_VK_SUCCESS(err);
    vkFreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &secondary_command_buffer);
    vkFreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &primary_command_buffer);
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, IgnoreUnrelatedDescriptor) {
    TEST_DESCRIPTION(
        "Ensure that the vkUpdateDescriptorSets validation code is ignoring VkWriteDescriptorSet members that are not related to "
        "the descriptor type specified by VkWriteDescriptorSet::descriptorType.  Correct validation behavior will result in the "
        "test running to completion without validation errors.");

    const uintptr_t invalid_ptr = 0xcdcdcdcd;

    ASSERT_NO_FATAL_FAILURE(Init());

    // Verify VK_FORMAT_R8_UNORM supports VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    const VkFormat format_texel_case = VK_FORMAT_R8_UNORM;
    const char *format_texel_case_string = "VK_FORMAT_R8_UNORM";
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), format_texel_case, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        printf("%s Test requires %s to support VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT\n", kSkipPrefix, format_texel_case_string);
        return;
    }

    // Image Case
    {
        m_errorMonitor->ExpectSuccess();

        VkImageObj image(m_device);
        image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

        VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet descriptor_write;
        memset(&descriptor_write, 0, sizeof(descriptor_write));
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_write.pImageInfo = &image_info;

        // Set pBufferInfo and pTexelBufferView to invalid values, which should
        // be
        //  ignored for descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE.
        // This will most likely produce a crash if the parameter_validation
        // layer
        // does not correctly ignore pBufferInfo.
        descriptor_write.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo *>(invalid_ptr);
        descriptor_write.pTexelBufferView = reinterpret_cast<const VkBufferView *>(invalid_ptr);

        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_errorMonitor->VerifyNotFound();
    }

    // Buffer Case
    {
        m_errorMonitor->ExpectSuccess();

        uint32_t queue_family_index = 0;
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = 1024;

        VkWriteDescriptorSet descriptor_write;
        memset(&descriptor_write, 0, sizeof(descriptor_write));
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        // Set pImageInfo and pTexelBufferView to invalid values, which should
        // be
        //  ignored for descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER.
        // This will most likely produce a crash if the parameter_validation
        // layer
        // does not correctly ignore pImageInfo.
        descriptor_write.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo *>(invalid_ptr);
        descriptor_write.pTexelBufferView = reinterpret_cast<const VkBufferView *>(invalid_ptr);

        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_errorMonitor->VerifyNotFound();
    }

    // Texel Buffer Case
    {
        m_errorMonitor->ExpectSuccess();

        uint32_t queue_family_index = 0;
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        VkBufferViewCreateInfo buff_view_ci = {};
        buff_view_ci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        buff_view_ci.buffer = buffer.handle();
        buff_view_ci.format = format_texel_case;
        buff_view_ci.range = VK_WHOLE_SIZE;
        VkBufferView buffer_view;
        VkResult err = vkCreateBufferView(m_device->device(), &buff_view_ci, NULL, &buffer_view);
        ASSERT_VK_SUCCESS(err);
        OneOffDescriptorSet descriptor_set(m_device,
                                           {
                                               {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           });

        VkWriteDescriptorSet descriptor_write;
        memset(&descriptor_write, 0, sizeof(descriptor_write));
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        descriptor_write.pTexelBufferView = &buffer_view;

        // Set pImageInfo and pBufferInfo to invalid values, which should be
        //  ignored for descriptorType ==
        //  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER.
        // This will most likely produce a crash if the parameter_validation
        // layer
        // does not correctly ignore pImageInfo and pBufferInfo.
        descriptor_write.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo *>(invalid_ptr);
        descriptor_write.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo *>(invalid_ptr);

        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_errorMonitor->VerifyNotFound();

        vkDestroyBufferView(m_device->device(), buffer_view, NULL);
    }
}

TEST_F(VkPositiveLayerTest, ImmutableSamplerOnlyDescriptor) {
    TEST_DESCRIPTION("Bind a DescriptorSet with only an immutable sampler and make sure that we don't warn for no update.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkResult err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->VerifyNotFound();

    vkDestroySampler(m_device->device(), sampler, NULL);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, EmptyDescriptorUpdateTest) {
    TEST_DESCRIPTION("Update last descriptor in a set that includes an empty binding");
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();

    // Create layout with two uniform buffer descriptors w/ empty binding between them
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                         {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 /*!*/, 0, nullptr},
                                         {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                     });

    // Create a buffer to be used for update
    VkBufferCreateInfo buff_ci = {};
    buff_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buff_ci.size = 256;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer buffer;
    err = vkCreateBuffer(m_device->device(), &buff_ci, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);
    // Have to bind memory to buffer before descriptor update
    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 512;  // one allocation for both buffers
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    if (!pass) {
        printf("%s Failed to allocate memory.\n", kSkipPrefix);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    // Make sure allocation is sufficiently large to accommodate buffer requirements
    if (mem_reqs.size > mem_alloc.allocationSize) {
        mem_alloc.allocationSize = mem_reqs.size;
    }

    VkDeviceMemory mem;
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    err = vkBindBufferMemory(m_device->device(), buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    // Only update the descriptor at binding 2
    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer;
    buff_info.offset = 0;
    buff_info.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = ds.set_;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_errorMonitor->VerifyNotFound();
    // Cleanup
    vkFreeMemory(m_device->device(), mem, NULL);
    vkDestroyBuffer(m_device->device(), buffer, NULL);
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, PushDescriptorNullDstSetTest) {
    TEST_DESCRIPTION("Use null dstSet in CmdPushDescriptorSetKHR");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s Push Descriptors Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    m_errorMonitor->ExpectSuccess();

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});
    // Create push descriptor set layout
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.InitState();
    helper.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&push_ds_layout, &ds_layout});
    helper.CreateGraphicsPipeline();

    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo.handle();
    buff_info.offset = 0;
    buff_info.range = sizeof(vbo_data);
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    // Find address of extension call and make the call
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    assert(vkCmdPushDescriptorSetKHR != nullptr);

    m_commandBuffer->begin();

    // In Intel GPU, it needs to bind pipeline before push descriptor set.
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_layout_.handle(), 0, 1,
                              &descriptor_write);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, PushDescriptorUnboundSetTest) {
    TEST_DESCRIPTION("Ensure that no validation errors are produced for not bound push descriptor sets");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s Push Descriptors Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->ExpectSuccess();

    // Create descriptor set layout
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    OneOffDescriptorSet descriptor_set(m_device, {dsl_binding}, 0, nullptr, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                       nullptr);

    // Create push descriptor set layout
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Create PSO
    char const fsSource[] =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 x;\n"
        "layout(set=0) layout(binding=2) uniform foo1 { float x; } bar1;\n"
        "layout(set=1) layout(binding=2) uniform foo2 { float y; } bar2;\n"
        "void main(){\n"
        "   x = vec4(bar1.x) + vec4(bar2.y);\n"
        "}\n";
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    // Now use the descriptor layouts to create a pipeline layout
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&push_ds_layout, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    const float bo_data[1] = {1.f};
    VkConstantBufferObj buffer(m_device, sizeof(bo_data), (const void *)&bo_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Update descriptor set
    descriptor_set.WriteDescriptorBufferInfo(2, buffer.handle(), sizeof(bo_data));
    descriptor_set.UpdateDescriptorSets();

    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    assert(vkCmdPushDescriptorSetKHR != nullptr);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // Push descriptors and bind descriptor set
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              descriptor_set.descriptor_writes.data());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 1, 1,
                            &descriptor_set.set_, 0, NULL);

    // No errors should be generated.
    vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, PushDescriptorSetUpdatingSetNumber) {
    TEST_DESCRIPTION(
        "Ensure that no validation errors are produced when the push descriptor set number changes "
        "between two vkCmdPushDescriptorSetKHR calls.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
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
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->ExpectSuccess();

    // Create a descriptor to push
    const uint32_t buffer_data[4] = {4, 5, 6, 7};
    VkConstantBufferObj buffer_obj(
        m_device, sizeof(buffer_data), &buffer_data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ASSERT_TRUE(buffer_obj.initialized());

    VkDescriptorBufferInfo buffer_info = {buffer_obj.handle(), 0, VK_WHOLE_SIZE};

    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    ASSERT_TRUE(vkCmdPushDescriptorSetKHR != nullptr);

    const VkDescriptorSetLayoutBinding ds_binding_0 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       nullptr};
    const VkDescriptorSetLayoutBinding ds_binding_1 = {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       nullptr};
    const VkDescriptorSetLayoutObj ds_layout(m_device, {ds_binding_0, ds_binding_1});
    ASSERT_TRUE(ds_layout.initialized());

    const VkDescriptorSetLayoutBinding push_ds_binding_0 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                            nullptr};
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {push_ds_binding_0},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    ASSERT_TRUE(push_ds_layout.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkPipelineObj pipe0(m_device);
    VkPipelineObj pipe1(m_device);
    {
        // Note: the push descriptor set is set number 2.
        const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout, &ds_layout, &push_ds_layout, &ds_layout});
        ASSERT_TRUE(pipeline_layout.initialized());

        char const *fsSource =
            "#version 450\n"
            "\n"
            "layout(location=0) out vec4 x;\n"
            "layout(set=2) layout(binding=0) uniform foo { vec4 y; } bar;\n"
            "void main(){\n"
            "   x = bar.y;\n"
            "}\n";

        VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
        VkPipelineObj &pipe = pipe0;
        pipe.SetViewport(m_viewports);
        pipe.SetScissor(m_scissors);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.AddDefaultColorAttachment();
        pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

        const VkWriteDescriptorSet descriptor_write = vk_testing::Device::write_descriptor_set(
            vk_testing::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

        // Note: pushing to desciptor set number 2.
        vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 2, 1,
                                  &descriptor_write);
        vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }

    m_errorMonitor->VerifyNotFound();

    {
        // Note: the push descriptor set is now set number 3.
        const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout, &ds_layout, &ds_layout, &push_ds_layout});
        ASSERT_TRUE(pipeline_layout.initialized());

        const VkWriteDescriptorSet descriptor_write = vk_testing::Device::write_descriptor_set(
            vk_testing::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

        char const *fsSource =
            "#version 450\n"
            "\n"
            "layout(location=0) out vec4 x;\n"
            "layout(set=3) layout(binding=0) uniform foo { vec4 y; } bar;\n"
            "void main(){\n"
            "   x = bar.y;\n"
            "}\n";

        VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
        VkPipelineObj &pipe = pipe1;
        pipe.SetViewport(m_viewports);
        pipe.SetScissor(m_scissors);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.AddDefaultColorAttachment();
        pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

        // Note: now pushing to desciptor set number 3.
        vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 3, 1,
                                  &descriptor_write);
        vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }

    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, TestAliasedMemoryTracking) {
    VkResult err;
    bool pass;

    TEST_DESCRIPTION(
        "Create a buffer, allocate memory, bind memory, destroy the buffer, create an image, and bind the same memory to it");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    VkBuffer buffer;
    VkImage image;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = 256;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    err = vkCreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vkGetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    alloc_info.allocationSize = 0x10000;

    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to allocate memory.\n", kSkipPrefix);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }

    err = vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    uint8_t *pData;
    err = vkMapMemory(m_device->device(), mem, 0, mem_reqs.size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);

    memset(pData, 0xCADECADE, static_cast<size_t>(mem_reqs.size));

    vkUnmapMemory(m_device->device(), mem);

    err = vkBindBufferMemory(m_device->device(), buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    // NOW, destroy the buffer. Obviously, the resource no longer occupies this
    // memory. In fact, it was never used by the GPU.
    // Just be sure, wait for idle.
    vkDestroyBuffer(m_device->device(), buffer, NULL);
    vkDeviceWaitIdle(m_device->device());

    // Use optimal as some platforms report linear support but then fail image creation
    VkImageTiling image_tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageFormatProperties image_format_properties;
    vkGetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, image_tiling,
                                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, &image_format_properties);
    if (image_format_properties.maxExtent.width == 0) {
        printf("%s Image format not supported; skipped.\n", kSkipPrefix);
        vkFreeMemory(m_device->device(), mem, NULL);
        return;
    }
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = image_tiling;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = 0;

    /* Create a mappable image.  It will be the texture if linear images are OK
     * to be textures or it will be the staging image if they are not.
     */
    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vkGetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = mem_reqs.size;

    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to allocate memory.\n", kSkipPrefix);
        vkFreeMemory(m_device->device(), mem, NULL);
        vkDestroyImage(m_device->device(), image, NULL);
        return;
    }

    // VALIDATION FAILURE:
    err = vkBindImageMemory(m_device->device(), image, mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->VerifyNotFound();

    vkFreeMemory(m_device->device(), mem, NULL);
    vkDestroyImage(m_device->device(), image, NULL);
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, TestDestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    vkDestroyBuffer(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyBufferView(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyCommandPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyDescriptorPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyDescriptorSetLayout(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyDevice(VK_NULL_HANDLE, NULL);
    vkDestroyEvent(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyFence(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyFramebuffer(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyImage(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyImageView(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyInstance(VK_NULL_HANDLE, NULL);
    vkDestroyPipeline(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyPipelineCache(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyPipelineLayout(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyQueryPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyRenderPass(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroySampler(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroySemaphore(m_device->device(), VK_NULL_HANDLE, NULL);
    vkDestroyShaderModule(m_device->device(), VK_NULL_HANDLE, NULL);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffers[1]);
    vkFreeCommandBuffers(m_device->device(), command_pool, 3, command_buffers);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[3] = {};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_VK_SUCCESS(err);
    vkFreeDescriptorSets(m_device->device(), ds_pool, 3, descriptor_sets);
    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);

    vkFreeMemory(m_device->device(), VK_NULL_HANDLE, NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, QueueSubmitSemaphoresAndLayoutTracking) {
    TEST_DESCRIPTION("Submit multiple command buffers with chained semaphore signals and layout transitions");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkCommandBuffer cmd_bufs[4];
    VkCommandBufferAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.commandBufferCount = 4;
    alloc_info.commandPool = m_commandPool->handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &alloc_info, cmd_bufs);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
               (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkCommandBufferBeginInfo cb_binfo;
    cb_binfo.pNext = NULL;
    cb_binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    // Use 4 command buffers, each with an image layout transition, ColorAO->General->ColorAO->TransferSrc->TransferDst
    vkBeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.pNext = NULL;
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
    vkCmdPipelineBarrier(cmd_bufs[0], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    vkEndCommandBuffer(cmd_bufs[0]);
    vkBeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkCmdPipelineBarrier(cmd_bufs[1], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    vkEndCommandBuffer(cmd_bufs[1]);
    vkBeginCommandBuffer(cmd_bufs[2], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vkCmdPipelineBarrier(cmd_bufs[2], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    vkEndCommandBuffer(cmd_bufs[2]);
    vkBeginCommandBuffer(cmd_bufs[3], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkCmdPipelineBarrier(cmd_bufs[3], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    vkEndCommandBuffer(cmd_bufs[3]);

    // Submit 4 command buffers in 3 submits, with submits 2 and 3 waiting for semaphores from submits 1 and 2
    VkSemaphore semaphore1, semaphore2;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore1);
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore2);
    VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    VkSubmitInfo submit_info[3];
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].pNext = nullptr;
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = &cmd_bufs[0];
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore1;
    submit_info[0].waitSemaphoreCount = 0;
    submit_info[0].pWaitDstStageMask = nullptr;
    submit_info[0].pWaitDstStageMask = flags;
    submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[1].pNext = nullptr;
    submit_info[1].commandBufferCount = 1;
    submit_info[1].pCommandBuffers = &cmd_bufs[1];
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore1;
    submit_info[1].signalSemaphoreCount = 1;
    submit_info[1].pSignalSemaphores = &semaphore2;
    submit_info[1].pWaitDstStageMask = flags;
    submit_info[2].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[2].pNext = nullptr;
    submit_info[2].commandBufferCount = 2;
    submit_info[2].pCommandBuffers = &cmd_bufs[2];
    submit_info[2].waitSemaphoreCount = 1;
    submit_info[2].pWaitSemaphores = &semaphore2;
    submit_info[2].signalSemaphoreCount = 0;
    submit_info[2].pSignalSemaphores = nullptr;
    submit_info[2].pWaitDstStageMask = flags;
    vkQueueSubmit(m_device->m_queue, 3, submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);

    vkDestroySemaphore(m_device->device(), semaphore1, NULL);
    vkDestroySemaphore(m_device->device(), semaphore2, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, DynamicOffsetWithInactiveBinding) {
    // Create a descriptorSet w/ dynamic descriptors where 1 binding is inactive
    // We previously had a bug where dynamic offset of inactive bindings was still being used
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    // Create two buffers to update the descriptors with
    // The first will be 2k and used for bindings 0 & 1, the second is 1k for binding 2
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 2048;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBufferObj dynamic_uniform_buffer_1, dynamic_uniform_buffer_2;
    dynamic_uniform_buffer_1.init(*m_device, buffCI);
    buffCI.size = 1024;
    dynamic_uniform_buffer_2.init(*m_device, buffCI);

    // Update descriptors
    const uint32_t BINDING_COUNT = 3;
    VkDescriptorBufferInfo buff_info[BINDING_COUNT] = {};
    buff_info[0].buffer = dynamic_uniform_buffer_1.handle();
    buff_info[0].offset = 0;
    buff_info[0].range = 256;
    buff_info[1].buffer = dynamic_uniform_buffer_1.handle();
    buff_info[1].offset = 256;
    buff_info[1].range = 512;
    buff_info[2].buffer = dynamic_uniform_buffer_2.handle();
    buff_info[2].offset = 0;
    buff_info[2].range = 512;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = BINDING_COUNT;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptor_write.pBufferInfo = buff_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Create PSO to be used for draw-time errors below
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 x;\n"
        "layout(set=0) layout(binding=0) uniform foo1 { int x; int y; } bar1;\n"
        "layout(set=0) layout(binding=2) uniform foo2 { int x; int y; } bar2;\n"
        "void main(){\n"
        "   x = vec4(bar1.y) + vec4(bar2.y);\n"
        "}\n";
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    // This update should succeed, but offset of inactive binding 1 oversteps binding 2 buffer size
    //   we used to have a bug in this case.
    uint32_t dyn_off[BINDING_COUNT] = {0, 1024, 256};
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                            &descriptor_set.set_, BINDING_COUNT, dyn_off);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, NonCoherentMemoryMapping) {
    TEST_DESCRIPTION(
        "Ensure that validations handling of non-coherent memory mapping while using VK_WHOLE_SIZE does not cause access "
        "violations");
    VkResult err;
    uint8_t *pData;
    ASSERT_NO_FATAL_FAILURE(Init());

    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;
    mem_reqs.memoryTypeBits = 0xFFFFFFFF;
    const VkDeviceSize atom_size = m_device->props.limits.nonCoherentAtomSize;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    static const VkDeviceSize allocation_size = 32 * atom_size;
    alloc_info.allocationSize = allocation_size;

    // Find a memory configurations WITHOUT a COHERENT bit, otherwise exit
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!pass) {
        pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!pass) {
            pass = m_device->phy().set_memory_type(
                mem_reqs.memoryTypeBits, &alloc_info,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            if (!pass) {
                printf("%s Couldn't find a memory type wihtout a COHERENT bit.\n", kSkipPrefix);
                return;
            }
        }
    }

    err = vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    // Map/Flush/Invalidate using WHOLE_SIZE and zero offsets and entire mapped range
    m_errorMonitor->ExpectSuccess();
    err = vkMapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    VkMappedMemoryRange mmr = {};
    mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mmr.memory = mem;
    mmr.offset = 0;
    mmr.size = VK_WHOLE_SIZE;
    err = vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    err = vkInvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    vkUnmapMemory(m_device->device(), mem);

    // Map/Flush/Invalidate using WHOLE_SIZE and an offset and entire mapped range
    m_errorMonitor->ExpectSuccess();
    err = vkMapMemory(m_device->device(), mem, 5 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mmr.memory = mem;
    mmr.offset = 6 * atom_size;
    mmr.size = VK_WHOLE_SIZE;
    err = vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    err = vkInvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    vkUnmapMemory(m_device->device(), mem);

    // Map with offset and size
    // Flush/Invalidate subrange of mapped area with offset and size
    m_errorMonitor->ExpectSuccess();
    err = vkMapMemory(m_device->device(), mem, 3 * atom_size, 9 * atom_size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mmr.memory = mem;
    mmr.offset = 4 * atom_size;
    mmr.size = 2 * atom_size;
    err = vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    err = vkInvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    vkUnmapMemory(m_device->device(), mem);

    // Map without offset and flush WHOLE_SIZE with two separate offsets
    m_errorMonitor->ExpectSuccess();
    err = vkMapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mmr.memory = mem;
    mmr.offset = allocation_size - (4 * atom_size);
    mmr.size = VK_WHOLE_SIZE;
    err = vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    mmr.offset = allocation_size - (6 * atom_size);
    mmr.size = VK_WHOLE_SIZE;
    err = vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    vkUnmapMemory(m_device->device(), mem);

    vkFreeMemory(m_device->device(), mem, NULL);
}

// This is a positive test. We used to expect error in this case but spec now allows it
TEST_F(VkPositiveLayerTest, ResetUnsignaledFence) {
    m_errorMonitor->ExpectSuccess();
    vk_testing::Fence testFence;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;

    ASSERT_NO_FATAL_FAILURE(Init());
    testFence.init(*m_device, fenceInfo);
    VkFence fences[1] = {testFence.handle()};
    VkResult result = vkResetFences(m_device->device(), 1, fences);
    ASSERT_VK_SUCCESS(result);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CommandBufferSimultaneousUseSync) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkResult err;

    // Record (empty!) command buffer that can be submitted multiple times
    // simultaneously.
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
    m_commandBuffer->begin(&cbbi);
    m_commandBuffer->end();

    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence fence;
    err = vkCreateFence(m_device->device(), &fci, nullptr, &fence);
    ASSERT_VK_SUCCESS(err);

    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    VkSemaphore s1, s2;
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &s1);
    ASSERT_VK_SUCCESS(err);
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &s2);
    ASSERT_VK_SUCCESS(err);

    // Submit CB once signaling s1, with fence so we can roll forward to its retirement.
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &m_commandBuffer->handle(), 1, &s1};
    err = vkQueueSubmit(m_device->m_queue, 1, &si, fence);
    ASSERT_VK_SUCCESS(err);

    // Submit CB again, signaling s2.
    si.pSignalSemaphores = &s2;
    err = vkQueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Wait for fence.
    err = vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    ASSERT_VK_SUCCESS(err);

    // CB is still in flight from second submission, but semaphore s1 is no
    // longer in flight. delete it.
    vkDestroySemaphore(m_device->device(), s1, nullptr);

    m_errorMonitor->VerifyNotFound();

    // Force device idle and clean up remaining objects
    vkDeviceWaitIdle(m_device->device());
    vkDestroySemaphore(m_device->device(), s2, nullptr);
    vkDestroyFence(m_device->device(), fence, nullptr);
}

TEST_F(VkPositiveLayerTest, FenceCreateSignaledWaitHandling) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    VkResult err;

    // A fence created signaled
    VkFenceCreateInfo fci1 = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    VkFence f1;
    err = vkCreateFence(m_device->device(), &fci1, nullptr, &f1);
    ASSERT_VK_SUCCESS(err);

    // A fence created not
    VkFenceCreateInfo fci2 = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence f2;
    err = vkCreateFence(m_device->device(), &fci2, nullptr, &f2);
    ASSERT_VK_SUCCESS(err);

    // Submit the unsignaled fence
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 0, nullptr, 0, nullptr};
    err = vkQueueSubmit(m_device->m_queue, 1, &si, f2);

    // Wait on both fences, with signaled first.
    VkFence fences[] = {f1, f2};
    vkWaitForFences(m_device->device(), 2, fences, VK_TRUE, UINT64_MAX);

    // Should have both retired!
    vkDestroyFence(m_device->device(), f1, nullptr);
    vkDestroyFence(m_device->device(), f2, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreateImageViewFollowsParameterCompatibilityRequirements) {
    TEST_DESCRIPTION("Verify that creating an ImageView with valid usage does not generate validation errors.");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->ExpectSuccess();

    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {128, 128, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&imgInfo);
    ASSERT_TRUE(image.initialized());
    image.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ValidUsage) {
    TEST_DESCRIPTION("Verify that creating an image view from an image with valid usage doesn't generate validation errors");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->ExpectSuccess();
    // Verify that we can create a view with usage INPUT_ATTACHMENT
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView imageView;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCreateImageView(m_device->device(), &ivci, NULL, &imageView);
    m_errorMonitor->VerifyNotFound();
    vkDestroyImageView(m_device->device(), imageView, NULL);
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, BindSparse) {
    TEST_DESCRIPTION("Bind 2 memory ranges to one image using vkQueueBindSparse, destroy the image and then free the memory");

    ASSERT_NO_FATAL_FAILURE(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        printf("%s Graphics queue does not have sparse binding bit.\n", kSkipPrefix);
        return;
    }
    if (!m_device->phy().features().sparseBinding) {
        printf("%s Device does not support sparse bindings.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    VkImage image;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    VkResult err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory memory_one, memory_two;
    bool pass;
    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = NULL;
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    // Find an image big enough to allow sparse mapping of 2 memory regions
    // Increase the image size until it is at least twice the
    // size of the required alignment, to ensure we can bind both
    // allocated memory blocks to the image on aligned offsets.
    while (memory_reqs.size < (memory_reqs.alignment * 2)) {
        vkDestroyImage(m_device->device(), image, nullptr);
        image_create_info.extent.width *= 2;
        image_create_info.extent.height *= 2;
        err = vkCreateImage(m_device->device(), &image_create_info, nullptr, &image);
        ASSERT_VK_SUCCESS(err);
        vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    }
    // Allocate 2 memory regions of minimum alignment size, bind one at 0, the other
    // at the end of the first
    memory_info.allocationSize = memory_reqs.alignment;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &memory_one);
    ASSERT_VK_SUCCESS(err);
    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &memory_two);
    ASSERT_VK_SUCCESS(err);
    VkSparseMemoryBind binds[2];
    binds[0].flags = 0;
    binds[0].memory = memory_one;
    binds[0].memoryOffset = 0;
    binds[0].resourceOffset = 0;
    binds[0].size = memory_info.allocationSize;
    binds[1].flags = 0;
    binds[1].memory = memory_two;
    binds[1].memoryOffset = 0;
    binds[1].resourceOffset = memory_info.allocationSize;
    binds[1].size = memory_info.allocationSize;

    VkSparseImageOpaqueMemoryBindInfo opaqueBindInfo;
    opaqueBindInfo.image = image;
    opaqueBindInfo.bindCount = 2;
    opaqueBindInfo.pBinds = binds;

    VkFence fence = VK_NULL_HANDLE;
    VkBindSparseInfo bindSparseInfo = {};
    bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
    bindSparseInfo.imageOpaqueBindCount = 1;
    bindSparseInfo.pImageOpaqueBinds = &opaqueBindInfo;

    vkQueueBindSparse(m_device->m_queue, 1, &bindSparseInfo, fence);
    vkQueueWaitIdle(m_device->m_queue);
    vkDestroyImage(m_device->device(), image, NULL);
    vkFreeMemory(m_device->device(), memory_one, NULL);
    vkFreeMemory(m_device->device(), memory_two, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, BindSparseMetadata) {
    TEST_DESCRIPTION("Bind memory for the metadata aspect of a sparse image");

    ASSERT_NO_FATAL_FAILURE(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        printf("%s Graphics queue does not have sparse binding bit.\n", kSkipPrefix);
        return;
    }
    if (!m_device->phy().features().sparseResidencyImage2D) {
        printf("%s Device does not support sparse residency for images.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    // Create a sparse image
    VkImage image;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    VkResult err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    // Query image memory requirements
    VkMemoryRequirements memory_reqs;
    vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);

    // Query sparse memory requirements
    uint32_t sparse_reqs_count = 0;
    vkGetImageSparseMemoryRequirements(m_device->device(), image, &sparse_reqs_count, nullptr);
    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(sparse_reqs_count);
    vkGetImageSparseMemoryRequirements(m_device->device(), image, &sparse_reqs_count, sparse_reqs.data());

    // Find requirements for metadata aspect
    const VkSparseImageMemoryRequirements *metadata_reqs = nullptr;
    for (auto const &aspect_sparse_reqs : sparse_reqs) {
        if (aspect_sparse_reqs.formatProperties.aspectMask == VK_IMAGE_ASPECT_METADATA_BIT) {
            metadata_reqs = &aspect_sparse_reqs;
        }
    }

    if (!metadata_reqs) {
        printf("%s Sparse image does not require memory for metadata.\n", kSkipPrefix);
    } else {
        // Allocate memory for the metadata
        VkDeviceMemory metadata_memory = VK_NULL_HANDLE;
        VkMemoryAllocateInfo metadata_memory_info = {};
        metadata_memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        metadata_memory_info.allocationSize = metadata_reqs->imageMipTailSize;
        m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &metadata_memory_info, 0);
        err = vkAllocateMemory(m_device->device(), &metadata_memory_info, NULL, &metadata_memory);
        ASSERT_VK_SUCCESS(err);

        // Bind metadata
        VkSparseMemoryBind sparse_bind = {};
        sparse_bind.resourceOffset = metadata_reqs->imageMipTailOffset;
        sparse_bind.size = metadata_reqs->imageMipTailSize;
        sparse_bind.memory = metadata_memory;
        sparse_bind.memoryOffset = 0;
        sparse_bind.flags = VK_SPARSE_MEMORY_BIND_METADATA_BIT;

        VkSparseImageOpaqueMemoryBindInfo opaque_bind_info = {};
        opaque_bind_info.image = image;
        opaque_bind_info.bindCount = 1;
        opaque_bind_info.pBinds = &sparse_bind;

        VkBindSparseInfo bind_info = {};
        bind_info.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
        bind_info.imageOpaqueBindCount = 1;
        bind_info.pImageOpaqueBinds = &opaque_bind_info;

        vkQueueBindSparse(m_device->m_queue, 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyNotFound();

        // Cleanup
        vkQueueWaitIdle(m_device->m_queue);
        vkFreeMemory(m_device->device(), metadata_memory, NULL);
    }

    vkDestroyImage(m_device->device(), image, NULL);
}

TEST_F(VkPositiveLayerTest, FramebufferBindingDestroyCommandPool) {
    TEST_DESCRIPTION(
        "This test should pass. Create a Framebuffer and command buffer, bind them together, then destroy command pool and "
        "framebuffer and verify there are no errors.");

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
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Explicitly create a command buffer to bind the FB to so that we can then
    //  destroy the command pool in order to implicitly free command buffer
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    // Begin our cmd buffer with renderpass using our framebuffer
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    vkCmdBeginRenderPass(command_buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
    // Destroy command pool to implicitly free command buffer
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, FramebufferCreateDepthStencilLayoutTransitionForDepthOnlyImageView) {
    TEST_DESCRIPTION(
        "Validate that when an imageView of a depth/stencil image is used as a depth/stencil framebuffer attachment, the "
        "aspectMask is ignored and both depth and stencil image subresources are used.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        printf("%s Image format does not support sampling.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

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

    VkResult err;
    VkRenderPass rp;
    err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
                       0x26,  // usage
                       VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    image.SetLayout(0x6, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkImageView view = image.targetView(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();

    VkImageMemoryBarrier imb = {};
    imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imb.pNext = nullptr;
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

    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imb);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyNotFound();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, BarrierLayoutToImageUsage) {
    TEST_DESCRIPTION("Ensure barriers' new and old VkImageLayout are compatible with their images' VkImageUsageFlags");

    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.pNext = NULL;
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    {
        VkImageObj img_color(m_device);
        img_color.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_color.initialized());

        VkImageObj img_ds1(m_device);
        img_ds1.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds1.initialized());

        VkImageObj img_ds2(m_device);
        img_ds2.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds2.initialized());

        VkImageObj img_xfer_src(m_device);
        img_xfer_src.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_src.initialized());

        VkImageObj img_xfer_dst(m_device);
        img_xfer_dst.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_dst.initialized());

        VkImageObj img_sampled(m_device);
        img_sampled.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_sampled.initialized());

        VkImageObj img_input(m_device);
        img_input.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_input.initialized());

        const struct {
            VkImageObj &image_obj;
            VkImageLayout old_layout;
            VkImageLayout new_layout;
        } buffer_layouts[] = {
            // clang-format off
            {img_color,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_ds1,      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL},
            {img_ds2,      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  VK_IMAGE_LAYOUT_GENERAL},
            {img_sampled,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_input,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             VK_IMAGE_LAYOUT_GENERAL},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(buffer_layouts) / sizeof(buffer_layouts[0]);

        m_commandBuffer->begin();
        for (uint32_t i = 0; i < layout_count; ++i) {
            img_barrier.image = buffer_layouts[i].image_obj.handle();
            const VkImageUsageFlags usage = buffer_layouts[i].image_obj.usage();
            img_barrier.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                          ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                          : VK_IMAGE_ASPECT_COLOR_BIT;

            img_barrier.oldLayout = buffer_layouts[i].old_layout;
            img_barrier.newLayout = buffer_layouts[i].new_layout;
            vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &img_barrier);

            img_barrier.oldLayout = buffer_layouts[i].new_layout;
            img_barrier.newLayout = buffer_layouts[i].old_layout;
            vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &img_barrier);
        }
        m_commandBuffer->end();

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, WaitEventThenSet) {
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");

    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(Init());

    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        vkCmdWaitEvents(command_buffer, 1, &event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0,
                        nullptr, 0, nullptr);
        vkCmdResetEvent(command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        vkEndCommandBuffer(command_buffer);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    { vkSetEvent(m_device->device(), event); }

    vkQueueWaitIdle(queue);

    vkDestroyEvent(m_device->device(), event, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

    m_errorMonitor->VerifyNotFound();
}
// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, QueryAndCopySecondaryCommandBuffers) {
    TEST_DESCRIPTION("Issue a query on a secondary command buffer and copy it on a primary.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    VkCommandPoolObj command_pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj primary_buffer(m_device, &command_pool);
    VkCommandBufferObj secondary_buffer(m_device, &command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = {};
    buff_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;

    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    VkCommandBufferInheritanceInfo hinfo = {};
    hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pInheritanceInfo = &hinfo;
        secondary_buffer.begin(&begin_info);
        vkCmdResetQueryPool(secondary_buffer.handle(), query_pool, 0, 1);
        vkCmdWriteTimestamp(secondary_buffer.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
        secondary_buffer.end();

        primary_buffer.begin();
        vkCmdExecuteCommands(primary_buffer.handle(), 1, &secondary_buffer.handle());
        vkCmdCopyQueryPoolResults(primary_buffer.handle(), query_pool, 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);
        primary_buffer.end();
    }

    primary_buffer.QueueCommandBuffer();
    vkQueueWaitIdle(queue);

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, QueryAndCopyMultipleCommandBuffers) {
    TEST_DESCRIPTION("Issue a query and copy from it on a second command buffer.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = {};
    buff_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;

    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdResetQueryPool(command_buffer[0], query_pool, 0, 1);
        vkCmdWriteTimestamp(command_buffer[0], VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);

        vkEndCommandBuffer(command_buffer[0]);

        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        vkCmdCopyQueryPoolResults(command_buffer[1], query_pool, 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);

        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 2;
        submit_info.pCommandBuffers = command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vkQueueWaitIdle(queue);

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, command_buffer);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    static const uint32_t NUM_OBJECTS = 2;
    static const uint32_t NUM_FRAMES = 3;
    VkCommandBuffer cmd_buffers[NUM_OBJECTS] = {};
    VkFence fences[NUM_OBJECTS] = {};

    VkCommandPool cmd_pool;
    VkCommandPoolCreateInfo cmd_pool_ci = {};
    cmd_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult err = vkCreateCommandPool(m_device->device(), &cmd_pool_ci, nullptr, &cmd_pool);
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferAllocateInfo cmd_buf_info = {};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buf_info.commandPool = cmd_pool;
    cmd_buf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_info.commandBufferCount = 1;

    VkFenceCreateInfo fence_ci = {};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.pNext = nullptr;
    fence_ci.flags = 0;

    for (uint32_t i = 0; i < NUM_OBJECTS; ++i) {
        err = vkAllocateCommandBuffers(m_device->device(), &cmd_buf_info, &cmd_buffers[i]);
        ASSERT_VK_SUCCESS(err);
        err = vkCreateFence(m_device->device(), &fence_ci, nullptr, &fences[i]);
        ASSERT_VK_SUCCESS(err);
    }

    for (uint32_t frame = 0; frame < NUM_FRAMES; ++frame) {
        for (uint32_t obj = 0; obj < NUM_OBJECTS; ++obj) {
            // Create empty cmd buffer
            VkCommandBufferBeginInfo cmdBufBeginDesc = {};
            cmdBufBeginDesc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            err = vkBeginCommandBuffer(cmd_buffers[obj], &cmdBufBeginDesc);
            ASSERT_VK_SUCCESS(err);
            err = vkEndCommandBuffer(cmd_buffers[obj]);
            ASSERT_VK_SUCCESS(err);

            VkSubmitInfo submit_info = {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &cmd_buffers[obj];
            // Submit cmd buffer and wait for fence
            err = vkQueueSubmit(queue, 1, &submit_info, fences[obj]);
            ASSERT_VK_SUCCESS(err);
            err = vkWaitForFences(m_device->device(), 1, &fences[obj], VK_TRUE, UINT64_MAX);
            ASSERT_VK_SUCCESS(err);
            err = vkResetFences(m_device->device(), 1, &fences[obj]);
            ASSERT_VK_SUCCESS(err);
        }
    }
    m_errorMonitor->VerifyNotFound();
    vkDestroyCommandPool(m_device->device(), cmd_pool, NULL);
    for (uint32_t i = 0; i < NUM_OBJECTS; ++i) {
        vkDestroyFence(m_device->device(), fences[i], nullptr);
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
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vkQueueWaitIdle(m_device->m_queue);

    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vkQueueWaitIdle(m_device->m_queue);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &q1);
    ASSERT_NE(q1, nullptr);

    // An (empty) command buffer. We must have work in the first submission --
    // the layer treats unfenced work differently from fenced work.
    VkCommandPoolCreateInfo cpci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, 0, 0};
    VkCommandPool pool;
    err = vkCreateCommandPool(m_device->device(), &cpci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);
    VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool,
                                        VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    VkCommandBuffer cb;
    err = vkAllocateCommandBuffers(m_device->device(), &cbai, &cb);
    ASSERT_VK_SUCCESS(err);
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr};
    err = vkBeginCommandBuffer(cb, &cbbi);
    ASSERT_VK_SUCCESS(err);
    err = vkEndCommandBuffer(cb);
    ASSERT_VK_SUCCESS(err);

    // A semaphore
    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    VkSemaphore s;
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &s);
    ASSERT_VK_SUCCESS(err);

    // First submission, to q0
    VkSubmitInfo s0 = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cb, 1, &s};

    err = vkQueueSubmit(q0, 1, &s0, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Second submission, to q1, waiting on s
    VkFlags waitmask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // doesn't really matter what this value is.
    VkSubmitInfo s1 = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &s, &waitmask, 0, nullptr, 0, nullptr};

    err = vkQueueSubmit(q1, 1, &s1, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Wait for q0 idle
    err = vkQueueWaitIdle(q0);
    ASSERT_VK_SUCCESS(err);

    // Command buffer should have been completed (it was on q0); reset the pool.
    vkFreeCommandBuffers(m_device->device(), pool, 1, &cb);

    m_errorMonitor->VerifyNotFound();

    // Force device completely idle and clean up resources
    vkDeviceWaitIdle(m_device->device());
    vkDestroyCommandPool(m_device->device(), pool, nullptr);
    vkDestroySemaphore(m_device->device(), s, nullptr);
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
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphore;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = VK_NULL_HANDLE;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = VK_NULL_HANDLE;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vkQueueSubmit(m_device->m_queue, 0, NULL, fence);

    VkResult err = vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    ASSERT_VK_SUCCESS(err);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

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
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[0];
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = VK_NULL_HANDLE;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer[1];
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = VK_NULL_HANDLE;
        submit_info.pWaitDstStageMask = flags;
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

    m_errorMonitor->VerifyNotFound();
}

// This is a positive test.  No errors should be generated.
TEST_F(VkPositiveLayerTest, TwoSubmitInfosWithSemaphoreOneQueueSubmitsOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers each in a separate SubmitInfo sent in a single QueueSubmit call followed by a WaitForFences call.");
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        vkCmdPipelineBarrier(command_buffer[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                             nullptr, 0, nullptr, 0, nullptr);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info[2];
        VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

        submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info[0].pNext = NULL;
        submit_info[0].commandBufferCount = 1;
        submit_info[0].pCommandBuffers = &command_buffer[0];
        submit_info[0].signalSemaphoreCount = 1;
        submit_info[0].pSignalSemaphores = &semaphore;
        submit_info[0].waitSemaphoreCount = 0;
        submit_info[0].pWaitSemaphores = NULL;
        submit_info[0].pWaitDstStageMask = 0;

        submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info[1].pNext = NULL;
        submit_info[1].commandBufferCount = 1;
        submit_info[1].pCommandBuffers = &command_buffer[1];
        submit_info[1].waitSemaphoreCount = 1;
        submit_info[1].pWaitSemaphores = &semaphore;
        submit_info[1].pWaitDstStageMask = flags;
        submit_info[1].signalSemaphoreCount = 0;
        submit_info[1].pSignalSemaphores = NULL;
        vkQueueSubmit(m_device->m_queue, 2, &submit_info[0], fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineAttribMatrixType) {
    TEST_DESCRIPTION("Test that pipeline validation accepts matrices passed as vertex attributes");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[2];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 2; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in mat2x4 x;\n"
        "void main(){\n"
        "   gl_Position = x[0] + x[1];\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    /* expect success */
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineAttribArrayType) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[2];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 2; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in vec4 x[2];\n"
        "void main(){\n"
        "   gl_Position = x[0] + x[1];\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineAttribComponents) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts consuming a vertex attribute through multiple vertex shader inputs, each consuming "
        "a different subset of the components, and that fragment shader-attachment validation tolerates multiple duplicate "
        "location outputs");
    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[3];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 3; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in vec4 x;\n"
        "layout(location=1) in vec3 y1;\n"
        "layout(location=1, component=3) in float y2;\n"
        "layout(location=2) in vec4 z;\n"
        "void main(){\n"
        "   gl_Position = x + vec4(y1, y2) + z;\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0, component=0) out float color0;\n"
        "layout(location=0, component=1) out float color1;\n"
        "layout(location=0, component=2) out float color2;\n"
        "layout(location=0, component=3) out float color3;\n"
        "layout(location=1, component=0) out vec2 second_color0;\n"
        "layout(location=1, component=2) out vec2 second_color1;\n"
        "void main(){\n"
        "   color0 = float(1);\n"
        "   second_color0 = vec2(1);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineObj pipe(m_device);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    // Create a renderPass with two color attachments
    VkAttachmentReference attachments[2] = {};
    attachments[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    attachments[1].attachment = 1;
    attachments[1].layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = attachments;
    subpass.colorAttachmentCount = 2;

    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;

    VkAttachmentDescription attach_desc[2] = {};
    attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    rpci.pAttachments = attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    VkRenderPass renderpass;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &renderpass);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    VkPipelineColorBlendAttachmentState att_state1 = {};
    att_state1.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state1.blendEnable = VK_FALSE;

    pipe.AddColorAttachment(0, att_state1);
    pipe.AddColorAttachment(1, att_state1);
    pipe.AddVertexInputBindings(&input_binding, 1);
    pipe.AddVertexInputAttribs(input_attribs, 3);
    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass);
    vkDestroyRenderPass(m_device->device(), renderpass, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineSimplePositive) {
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineRelaxedTypeMatch) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts the relaxed type matching rules set out in 14.1.3: fundamental type must match, and "
        "producer side must have at least as many components");
    m_errorMonitor->ExpectSuccess();

    // VK 1.0.8 Specification, 14.1.3 "Additionally,..." block

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "layout(location=0) out vec3 x;\n"
        "layout(location=1) out ivec3 y;\n"
        "layout(location=2) out vec3 z;\n"
        "void main(){\n"
        "   gl_Position = vec4(0);\n"
        "   x = vec3(0); y = ivec3(0); z = vec3(0);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "layout(location=0) in float x;\n"
        "layout(location=1) flat in int y;\n"
        "layout(location=2) in vec2 z;\n"
        "void main(){\n"
        "   color = vec4(1 + x + y + z.x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineTessPerVertex) {
    TEST_DESCRIPTION("Test that pipeline validation accepts per-vertex variables passed between the TCS and TES stages");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().tessellationShader) {
        printf("%s Device does not support tessellation shaders; skipped.\n", kSkipPrefix);
        return;
    }

    char const *tcsSource =
        "#version 450\n"
        "layout(location=0) out int x[];\n"
        "layout(vertices=3) out;\n"
        "void main(){\n"
        "   gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;\n"
        "   gl_TessLevelInner[0] = 1;\n"
        "   x[gl_InvocationID] = gl_InvocationID;\n"
        "}\n";
    char const *tesSource =
        "#version 450\n"
        "layout(triangles, equal_spacing, cw) in;\n"
        "layout(location=0) in int x[];\n"
        "void main(){\n"
        "   gl_Position.xyz = gl_TessCoord;\n"
        "   gl_Position.w = x[0] + x[1] + x[2];\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateMinimalShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj tcs(m_device, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj tes(m_device, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineGeometryInputBlockPositive) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a user-defined interface block passed into the geometry shader. This is interesting "
        "because the 'extra' array level is not present on the member type, but on the block instance.");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().geometryShader) {
        printf("%s Device does not support geometry shaders; skipped.\n", kSkipPrefix);
        return;
    }

    char const *gsSource =
        "#version 450\n"
        "layout(triangles) in;\n"
        "layout(triangle_strip, max_vertices=3) out;\n"
        "layout(location=0) in VertexData { vec4 x; } gs_in[];\n"
        "void main() {\n"
        "   gl_Position = gs_in[0].x;\n"
        "   EmitVertex();\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj gs(m_device, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipeline64BitAttributesPositive) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts basic use of 64bit vertex attributes. This is interesting because they consume "
        "multiple locations.");
    m_errorMonitor->ExpectSuccess();

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().shaderFloat64) {
        printf("%s Device does not support 64bit vertex attributes; skipped.\n", kSkipPrefix);
        return;
    }
    // Set 64bit format to support VTX Buffer feature
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        return;
    }
    VkFormatProperties format_props;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, &format_props);
    format_props.bufferFeatures |= VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, format_props);

    VkVertexInputBindingDescription input_bindings[1];
    memset(input_bindings, 0, sizeof(input_bindings));

    VkVertexInputAttributeDescription input_attribs[4];
    memset(input_attribs, 0, sizeof(input_attribs));
    input_attribs[0].location = 0;
    input_attribs[0].offset = 0;
    input_attribs[0].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[1].location = 2;
    input_attribs[1].offset = 32;
    input_attribs[1].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[2].location = 4;
    input_attribs[2].offset = 64;
    input_attribs[2].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[3].location = 6;
    input_attribs[3].offset = 96;
    input_attribs[3].format = VK_FORMAT_R64G64B64A64_SFLOAT;

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in dmat4 x;\n"
        "void main(){\n"
        "   gl_Position = vec4(x[0][0]);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.vi_ci_.pVertexBindingDescriptions = input_bindings;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 4;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineInputAttachmentPositive) {
    TEST_DESCRIPTION("Positive test for a correctly matched input attachment");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;\n"
        "layout(location=0) out vec4 color;\n"
        "void main() {\n"
        "   color = subpassLoad(x);\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkAttachmentDescription descs[2] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkAttachmentReference color = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference input = {
        1,
        VK_IMAGE_LAYOUT_GENERAL,
    };

    VkSubpassDescription sd = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &input, 1, &color, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, descs, 1, &sd, 0, nullptr};
    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // should be OK. would go wrong here if it's going to...
    pipe.CreateVKPipeline(pl.handle(), rp);

    m_errorMonitor->VerifyNotFound();

    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkPositiveLayerTest, CreateComputePipelineMissingDescriptorUnusedPositive) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a compute pipeline which declares a descriptor-backed resource which is not "
        "provided, but the shader does not statically use it. This is interesting because it requires compute pipelines to have a "
        "proper descriptor use walk, which they didn't for some time.");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *csSource =
        "#version 450\n"
        "\n"
        "layout(local_size_x=1) in;\n"
        "layout(set=0, binding=0) buffer block { vec4 x; };\n"
        "void main(){\n"
        "   // x is not used.\n"
        "}\n";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    pipe.CreateComputePipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreateComputePipelineCombinedImageSamplerConsumedAsSampler) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a shader consuming only the sampler portion of a combined image + sampler");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource =
        "#version 450\n"
        "\n"
        "layout(local_size_x=1) in;\n"
        "layout(set=0, binding=0) uniform sampler s;\n"
        "layout(set=0, binding=1) uniform texture2D t;\n"
        "layout(set=0, binding=2) buffer block { vec4 x; };\n"
        "void main() {\n"
        "   x = texture(sampler2D(t, s), vec2(0));\n"
        "}\n";
    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreateComputePipelineCombinedImageSamplerConsumedAsImage) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a shader consuming only the image portion of a combined image + sampler");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource =
        "#version 450\n"
        "\n"
        "layout(local_size_x=1) in;\n"
        "layout(set=0, binding=0) uniform texture2D t;\n"
        "layout(set=0, binding=1) uniform sampler s;\n"
        "layout(set=0, binding=2) buffer block { vec4 x; };\n"
        "void main() {\n"
        "   x = texture(sampler2D(t, s), vec2(0));\n"
        "}\n";
    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreateComputePipelineCombinedImageSamplerConsumedAsBoth) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a shader consuming both the sampler and the image of a combined image+sampler but "
        "via separate variables");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource =
        "#version 450\n"
        "\n"
        "layout(local_size_x=1) in;\n"
        "layout(set=0, binding=0) uniform texture2D t;\n"
        "layout(set=0, binding=0) uniform sampler s;  // both binding 0!\n"
        "layout(set=0, binding=1) buffer block { vec4 x; };\n"
        "void main() {\n"
        "   x = texture(sampler2D(t, s), vec2(0));\n"
        "}\n";
    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreateDescriptorSetBindingWithIgnoredSamplers) {
    TEST_DESCRIPTION("Test that layers conditionally do ignore the pImmutableSamplers on vkCreateDescriptorSetLayout");

    bool prop2_found = false;
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        prop2_found = true;
    } else {
        printf("%s %s Extension not supported, skipping push descriptor sub-tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool push_descriptor_found = false;
    if (prop2_found && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

        // In addition to the extension being supported we need to have at least one available
        // Some implementations report an invalid maxPushDescriptors of 0
        push_descriptor_found = GetPushDescriptorProperties(instance(), gpu()).maxPushDescriptors > 0;
    } else {
        printf("%s %s Extension not supported, skipping push descriptor sub-tests\n", kSkipPrefix,
               VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    const uint64_t fake_address_64 = 0xCDCDCDCDCDCDCDCD;
    const uint64_t fake_address_32 = 0xCDCDCDCD;
    const void *fake_pointer =
        sizeof(void *) == 8 ? reinterpret_cast<void *>(fake_address_64) : reinterpret_cast<void *>(fake_address_32);
    const VkSampler *hopefully_undereferencable_pointer = reinterpret_cast<const VkSampler *>(fake_pointer);

    // regular descriptors
    m_errorMonitor->ExpectSuccess();
    {
        const VkDescriptorSetLayoutBinding non_sampler_bindings[] = {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {8, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
        };
        const VkDescriptorSetLayoutCreateInfo dslci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                       static_cast<uint32_t>(size(non_sampler_bindings)), non_sampler_bindings};
        VkDescriptorSetLayout dsl;
        const VkResult err = vkCreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        ASSERT_VK_SUCCESS(err);
        vkDestroyDescriptorSetLayout(m_device->device(), dsl, nullptr);
    }
    m_errorMonitor->VerifyNotFound();

    if (push_descriptor_found) {
        // push descriptors
        m_errorMonitor->ExpectSuccess();
        {
            const VkDescriptorSetLayoutBinding non_sampler_bindings[] = {
                {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
                {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
                {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
                {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
                {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
                {6, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            };
            const VkDescriptorSetLayoutCreateInfo dslci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr,
                                                           VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
                                                           static_cast<uint32_t>(size(non_sampler_bindings)), non_sampler_bindings};
            VkDescriptorSetLayout dsl;
            const VkResult err = vkCreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
            ASSERT_VK_SUCCESS(err);
            vkDestroyDescriptorSetLayout(m_device->device(), dsl, nullptr);
        }
        m_errorMonitor->VerifyNotFound();
    }
}
TEST_F(VkPositiveLayerTest, GpuValidationInlineUniformBlock) {
    TEST_DESCRIPTION("GPU validation: Make sure inline uniform blocks don't generate false validation errors");
    m_errorMonitor->ExpectSuccess();
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;
    bool descriptor_indexing = CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names,
                                                                              m_device_extension_names, &features, m_errorMonitor);
    if (DeviceIsMockICD() || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto inline_uniform_block_features = lvl_init_struct<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>(&indexing_features);
    bool inline_uniform_block = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    if (!(descriptor_indexing && inline_uniform_block)) {
        printf("Descriptor indexing and/or inline uniform block not supported Skipping test\n");
        return;
    }
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&inline_uniform_block_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!indexing_features.descriptorBindingPartiallyBound || !inline_uniform_block_features.inlineUniformBlock) {
        printf("Not all features supported, skipping test\n");
        return;
    }
    auto inline_uniform_props = lvl_init_struct<VkPhysicalDeviceInlineUniformBlockPropertiesEXT>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&inline_uniform_props);
    vkGetPhysicalDeviceProperties2(gpu(), &prop2);

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s GPU-Assisted validation test requires Vulkan 1.1+.\n", kSkipPrefix);
        return;
    }
    auto c_queue = m_device->GetDefaultComputeQueue();
    if (nullptr == c_queue) {
        printf("Compute not supported, skipping test\n");
        return;
    }

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 4;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    layout_createinfo_binding_flags[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    layout_createinfo_binding_flags[0].pNext = NULL;
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, 20, VK_SHADER_STAGE_ALL,
                                            nullptr},  // 16 bytes for ivec4, 4 more for int
                                       },
                                       0, layout_createinfo_binding_flags, 0);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    const uint32_t test_data = 0xdeadca7;
    VkWriteDescriptorSetInlineUniformBlockEXT write_inline_uniform = {};
    write_inline_uniform.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT;
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &test_data;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 16;  // Skip first 16 bytes (dummy)
    descriptor_writes[1].descriptorCount = 4;   // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    descriptor_writes[1].pNext = &write_inline_uniform;
    vkUpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    char const *csSource =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n "
        "layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;"
        "layout(set = 0, binding = 1) uniform inlineubodef { ivec4 dummy; int val; } inlineubo;\n"

        "void main() {\n"
        "    u_index.index = inlineubo.val;\n"
        "}\n";

    auto shader_module = new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this);

    VkPipelineShaderStageCreateInfo stage;
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pNext = nullptr;
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module->handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = nullptr;
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = stage;

    VkPipeline c_pipeline;
    vkCreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);

    m_commandBuffer->begin();
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 0, nullptr);
    vkCmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyNotFound();
    vkDestroyPipeline(m_device->handle(), c_pipeline, NULL);
    vkDestroyShaderModule(m_device->handle(), shader_module->handle(), NULL);

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    ASSERT_TRUE(*data = test_data);
    buffer0.memory().unmap();
}

TEST_F(VkPositiveLayerTest, Maintenance1Tests) {
    TEST_DESCRIPTION("Validate various special cases for the Maintenance1_KHR extension");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    } else {
        printf("%s Maintenance1 Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess();

    VkCommandBufferObj cmd_buf(m_device, m_commandPool);
    cmd_buf.begin();
    // Set Negative height, should give error if Maintenance 1 is not enabled
    VkViewport viewport = {0, 0, 16, -16, 0, 1};
    vkCmdSetViewport(cmd_buf.handle(), 0, 1, &viewport);
    cmd_buf.end();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ValidStructPNext) {
    TEST_DESCRIPTION("Verify that a valid pNext value is handled correctly");

    // Positive test to check parameter_validation and unique_objects support for NV_dedicated_allocation
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME);
    } else {
        printf("%s VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess();

    VkDedicatedAllocationBufferCreateInfoNV dedicated_buffer_create_info = {};
    dedicated_buffer_create_info.sType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV;
    dedicated_buffer_create_info.pNext = nullptr;
    dedicated_buffer_create_info.dedicatedAllocation = VK_TRUE;

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = &dedicated_buffer_create_info;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    VkBuffer buffer;
    VkResult err = vkCreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    vkGetBufferMemoryRequirements(m_device->device(), buffer, &memory_reqs);

    VkDedicatedAllocationMemoryAllocateInfoNV dedicated_memory_info = {};
    dedicated_memory_info.sType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV;
    dedicated_memory_info.pNext = nullptr;
    dedicated_memory_info.buffer = buffer;
    dedicated_memory_info.image = VK_NULL_HANDLE;

    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = &dedicated_memory_info;
    memory_info.allocationSize = memory_reqs.size;

    bool pass;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    VkDeviceMemory buffer_memory;
    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &buffer_memory);
    ASSERT_VK_SUCCESS(err);

    err = vkBindBufferMemory(m_device->device(), buffer, buffer_memory, 0);
    ASSERT_VK_SUCCESS(err);

    vkDestroyBuffer(m_device->device(), buffer, NULL);
    vkFreeMemory(m_device->device(), buffer_memory, NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, PSOPolygonModeValid) {
    TEST_DESCRIPTION("Verify that using a solid polygon fill mode works correctly.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();
    // Artificially disable support for non-solid fill modes
    features.fillModeNonSolid = false;
    // The sacrificial device object
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkRenderpassObj render_pass(&test_device);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    VkPipelineRasterizationStateCreateInfo rs_ci = {};
    rs_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_ci.pNext = nullptr;
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = false;

    VkShaderObj vs(&test_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(&test_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    // Set polygonMode=FILL. No error is expected
    m_errorMonitor->ExpectSuccess();
    {
        VkPipelineObj pipe(&test_device);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.AddDefaultColorAttachment();
        // Set polygonMode to a good value
        rs_ci.polygonMode = VK_POLYGON_MODE_FILL;
        pipe.SetRasterization(&rs_ci);
        pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    }
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
        err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &semaphore);
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
        err = vkQueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
    }

    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence fence;
    err = vkCreateFence(m_device->device(), &fci, nullptr, &fence);
    ASSERT_VK_SUCCESS(err);
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &semaphores.back(), &flags, 0, nullptr, 0, nullptr};
    err = vkQueueSubmit(m_device->m_queue, 1, &si, fence);
    ASSERT_VK_SUCCESS(err);

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    for (auto semaphore : semaphores) vkDestroySemaphore(m_device->device(), semaphore, nullptr);

    vkDestroyFence(m_device->device(), fence, nullptr);

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
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

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
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
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
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &export_semaphore);
    ASSERT_VK_SUCCESS(err);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    VkSemaphore import_semaphore;
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &import_semaphore);
    ASSERT_VK_SUCCESS(err);

#ifdef _WIN32
    // Export semaphore payload to an opaque handle
    HANDLE handle = nullptr;
    VkSemaphoreGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_semaphore,
                                            handle_type};
    auto vkGetSemaphoreWin32HandleKHR =
        (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetSemaphoreWin32HandleKHR");
    err = vkGetSemaphoreWin32HandleKHR(m_device->device(), &ghi, &handle);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above
    VkImportSemaphoreWin32HandleInfoKHR ihi = {
        VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR, nullptr, import_semaphore, 0, handle_type, handle, nullptr};
    auto vkImportSemaphoreWin32HandleKHR =
        (PFN_vkImportSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportSemaphoreWin32HandleKHR");
    err = vkImportSemaphoreWin32HandleKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#else
    // Export semaphore payload to an opaque handle
    int fd = 0;
    VkSemaphoreGetFdInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR, nullptr, export_semaphore, handle_type};
    auto vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetSemaphoreFdKHR");
    err = vkGetSemaphoreFdKHR(m_device->device(), &ghi, &fd);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above
    VkImportSemaphoreFdInfoKHR ihi = {
        VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR, nullptr, import_semaphore, 0, handle_type, fd};
    auto vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportSemaphoreFdKHR");
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
    err = vkQueueSubmit(m_device->m_queue, 4, si, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    if (m_device->phy().features().sparseBinding) {
        // Signal the imported semaphore and wait on the exported semaphore
        VkBindSparseInfo bi[] = {
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &import_semaphore},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &export_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &import_semaphore},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &export_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
        };
        err = vkQueueBindSparse(m_device->m_queue, 4, bi, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
    }

    // Cleanup
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vkDestroySemaphore(m_device->device(), export_semaphore, nullptr);
    vkDestroySemaphore(m_device->device(), import_semaphore, nullptr);

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
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

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
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR");
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
        err = vkCreateFence(m_device->device(), &fci, nullptr, &export_fence);
        ASSERT_VK_SUCCESS(err);
    }

    // Create a fence to import payload into
    VkFence import_fence;
    {
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        err = vkCreateFence(m_device->device(), &fci, nullptr, &import_fence);
        ASSERT_VK_SUCCESS(err);
    }

#ifdef _WIN32
    // Export fence payload to an opaque handle
    HANDLE handle = nullptr;
    {
        VkFenceGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceWin32HandleKHR =
            (PFN_vkGetFenceWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetFenceWin32HandleKHR");
        err = vkGetFenceWin32HandleKHR(m_device->device(), &ghi, &handle);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceWin32HandleInfoKHR ifi = {
            VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR, nullptr, import_fence, 0, handle_type, handle, nullptr};
        auto vkImportFenceWin32HandleKHR =
            (PFN_vkImportFenceWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportFenceWin32HandleKHR");
        err = vkImportFenceWin32HandleKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#else
    // Export fence payload to an opaque handle
    int fd = 0;
    {
        VkFenceGetFdInfoKHR gfi = {VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetFenceFdKHR");
        err = vkGetFenceFdKHR(m_device->device(), &gfi, &fd);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceFdInfoKHR ifi = {VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR, nullptr, import_fence, 0, handle_type, fd};
        auto vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportFenceFdKHR");
        err = vkImportFenceFdKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#endif

    // Signal the exported fence and wait on the imported fence
    vkQueueSubmit(m_device->m_queue, 0, nullptr, export_fence);
    vkWaitForFences(m_device->device(), 1, &import_fence, VK_TRUE, 1000000000);
    vkResetFences(m_device->device(), 1, &import_fence);
    vkQueueSubmit(m_device->m_queue, 0, nullptr, export_fence);
    vkWaitForFences(m_device->device(), 1, &import_fence, VK_TRUE, 1000000000);
    vkResetFences(m_device->device(), 1, &import_fence);

    // Signal the imported fence and wait on the exported fence
    vkQueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    vkWaitForFences(m_device->device(), 1, &export_fence, VK_TRUE, 1000000000);
    vkResetFences(m_device->device(), 1, &export_fence);
    vkQueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    vkWaitForFences(m_device->device(), 1, &export_fence, VK_TRUE, 1000000000);
    vkResetFences(m_device->device(), 1, &export_fence);

    // Cleanup
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vkDestroyFence(m_device->device(), export_fence, nullptr);
    vkDestroyFence(m_device->device(), import_fence, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ThreadNullFenceCollision) {
    test_platform_thread thread;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "THREADING ERROR");

    ASSERT_NO_FATAL_FAILURE(Init());

    struct thread_data_struct data;
    data.device = m_device->device();
    data.bailout = false;
    m_errorMonitor->SetBailout(&data.bailout);

    // Call vkDestroyFence of VK_NULL_HANDLE repeatedly using multiple threads.
    // There should be no validation error from collision of that non-object.
    test_platform_thread_create(&thread, ReleaseNullFence, (void *)&data);
    for (int i = 0; i < 40000; i++) {
        vkDestroyFence(m_device->device(), VK_NULL_HANDLE, NULL);
    }
    test_platform_thread_join(thread, NULL);

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ClearColorImageWithValidRange) {
    TEST_DESCRIPTION("Record clear color with a valid VkImageSubresourceRange");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }

    // Try good case with VK_REMAINING
    {
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }
}

TEST_F(VkPositiveLayerTest, ClearDepthStencilWithValidRange) {
    TEST_DESCRIPTION("Record clear depth with a valid VkImageSubresourceRange");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

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
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }

    // Try good case with VK_REMAINING
    {
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {ds_aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }
}

TEST_F(VkPositiveLayerTest, CreateGraphicsPipelineWithIgnoredPointers) {
    TEST_DESCRIPTION("Create Graphics Pipeline with pointers that must be ignored by layers");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);

    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    const uint64_t fake_address_64 = 0xCDCDCDCDCDCDCDCD;
    const uint64_t fake_address_32 = 0xCDCDCDCD;
    void *hopefully_undereferencable_pointer =
        sizeof(void *) == 8 ? reinterpret_cast<void *>(fake_address_64) : reinterpret_cast<void *>(fake_address_32);

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);

    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,  // pNext
        0,        // flags
        0,
        nullptr,  // bindings
        0,
        nullptr  // attributes
    };

    const VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,  // pNext
        0,        // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE  // primitive restart
    };

    const VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info_template{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,   // pNext
        0,         // flags
        VK_FALSE,  // depthClamp
        VK_FALSE,  // rasterizerDiscardEnable
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,  // depthBias
        0.0f,
        0.0f,
        0.0f,  // depthBias params
        1.0f   // lineWidth
    };

    VkPipelineLayout pipeline_layout;
    {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            0,
            nullptr,  // layouts
            0,
            nullptr  // push constants
        };

        VkResult err = vkCreatePipelineLayout(m_device->device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
        ASSERT_VK_SUCCESS(err);
    }

    // try disabled rasterizer and no tessellation
    {
        m_errorMonitor->ExpectSuccess();

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
            pipeline_rasterization_state_create_info_template;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_TRUE;

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,        // stageCount
            &vs.GetStageCreateInfo(),
            &pipeline_vertex_input_state_create_info,
            &pipeline_input_assembly_state_create_info,
            reinterpret_cast<const VkPipelineTessellationStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineViewportStateCreateInfo *>(hopefully_undereferencable_pointer),
            &pipeline_rasterization_state_create_info,
            reinterpret_cast<const VkPipelineMultisampleStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineDepthStencilStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineColorBlendStateCreateInfo *>(hopefully_undereferencable_pointer),
            nullptr,  // dynamic states
            pipeline_layout,
            m_renderPass,
            0,  // subpass
            VK_NULL_HANDLE,
            0};

        VkPipeline pipeline;
        vkCreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);

        m_errorMonitor->VerifyNotFound();

        vkDestroyPipeline(m_device->handle(), pipeline, nullptr);
    }

    const VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,  // pNext
        0,        // flags
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,  // sample shading
        0.0f,      // minSampleShading
        nullptr,   // pSampleMask
        VK_FALSE,  // alphaToCoverageEnable
        VK_FALSE   // alphaToOneEnable
    };

    // try enabled rasterizer but no subpass attachments
    {
        m_errorMonitor->ExpectSuccess();

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
            pipeline_rasterization_state_create_info_template;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

        VkViewport viewport = {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
        VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}};

        const VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,
            &viewport,
            1,
            &scissor};

        VkRenderPass render_pass;
        {
            VkSubpassDescription subpass_desc = {};

            VkRenderPassCreateInfo render_pass_create_info{
                VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                nullptr,  // pNext
                0,        // flags
                0,
                nullptr,  // attachments
                1,
                &subpass_desc,
                0,
                nullptr  // subpass dependencies
            };

            VkResult err = vkCreateRenderPass(m_device->handle(), &render_pass_create_info, nullptr, &render_pass);
            ASSERT_VK_SUCCESS(err);
        }

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,        // stageCount
            &vs.GetStageCreateInfo(),
            &pipeline_vertex_input_state_create_info,
            &pipeline_input_assembly_state_create_info,
            nullptr,
            &pipeline_viewport_state_create_info,
            &pipeline_rasterization_state_create_info,
            &pipeline_multisample_state_create_info,
            reinterpret_cast<const VkPipelineDepthStencilStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineColorBlendStateCreateInfo *>(hopefully_undereferencable_pointer),
            nullptr,  // dynamic states
            pipeline_layout,
            render_pass,
            0,  // subpass
            VK_NULL_HANDLE,
            0};

        VkPipeline pipeline;
        vkCreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);

        m_errorMonitor->VerifyNotFound();

        vkDestroyPipeline(m_device->handle(), pipeline, nullptr);
        vkDestroyRenderPass(m_device->handle(), render_pass, nullptr);
    }

    // try dynamic viewport and scissor
    {
        m_errorMonitor->ExpectSuccess();

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
            pipeline_rasterization_state_create_info_template;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

        const VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,
            reinterpret_cast<const VkViewport *>(hopefully_undereferencable_pointer),
            1,
            reinterpret_cast<const VkRect2D *>(hopefully_undereferencable_pointer)};

        const VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
        };

        const VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state = {};

        const VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            VK_FALSE,
            VK_LOGIC_OP_CLEAR,
            1,
            &pipeline_color_blend_attachment_state,
            {0.0f, 0.0f, 0.0f, 0.0f}};

        const VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        const VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            2, dynamic_states};

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                                   nullptr,  // pNext
                                                                   0,        // flags
                                                                   1,        // stageCount
                                                                   &vs.GetStageCreateInfo(),
                                                                   &pipeline_vertex_input_state_create_info,
                                                                   &pipeline_input_assembly_state_create_info,
                                                                   nullptr,
                                                                   &pipeline_viewport_state_create_info,
                                                                   &pipeline_rasterization_state_create_info,
                                                                   &pipeline_multisample_state_create_info,
                                                                   &pipeline_depth_stencil_state_create_info,
                                                                   &pipeline_color_blend_state_create_info,
                                                                   &pipeline_dynamic_state_create_info,  // dynamic states
                                                                   pipeline_layout,
                                                                   m_renderPass,
                                                                   0,  // subpass
                                                                   VK_NULL_HANDLE,
                                                                   0};

        VkPipeline pipeline;
        vkCreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);

        m_errorMonitor->VerifyNotFound();

        vkDestroyPipeline(m_device->handle(), pipeline, nullptr);
    }

    vkDestroyPipelineLayout(m_device->handle(), pipeline_layout, nullptr);
}

TEST_F(VkPositiveLayerTest, ExternalMemory) {
    TEST_DESCRIPTION("Perform a copy through a pair of buffers linked by external memory");

#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    // Check for external memory instance extensions
    std::vector<const char *> reqd_instance_extensions = {
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}};
    for (auto extension_name : reqd_instance_extensions) {
        if (InstanceExtensionSupported(extension_name)) {
            m_instance_extension_names.push_back(extension_name);
        } else {
            printf("%s Required instance extension %s not supported, skipping test\n", kSkipPrefix, extension_name);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for import/export capability
    VkPhysicalDeviceExternalBufferInfoKHR ebi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR, nullptr, 0,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, handle_type};
    VkExternalBufferPropertiesKHR ebp = {VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR, nullptr, {0, 0, 0}};
    auto vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    ASSERT_TRUE(vkGetPhysicalDeviceExternalBufferPropertiesKHR != nullptr);
    vkGetPhysicalDeviceExternalBufferPropertiesKHR(gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External buffer does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    // Check if dedicated allocation is required
    bool dedicated_allocation =
        ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_KHR;
    if (dedicated_allocation) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        } else {
            printf("%s Dedicated allocation extension not supported, skipping test\n", kSkipPrefix);
            return;
        }
    }

    // Check for external memory device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, ext_mem_extension_name)) {
        m_device_extension_names.push_back(ext_mem_extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    } else {
        printf("%s External memory extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    const VkExternalMemoryBufferCreateInfoKHR external_buffer_info = {VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR,
                                                                      nullptr, handle_type};
    auto buffer_info = VkBufferObj::create_info(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.pNext = &external_buffer_info;
    VkBufferObj buffer_export;
    buffer_export.init_no_mem(*m_device, buffer_info);
    VkBufferObj buffer_import;
    buffer_import.init_no_mem(*m_device, buffer_info);

    // Allocation info
    auto alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_export.memory_requirements(), mem_flags);

    // Add export allocation info to pNext chain
    VkExportMemoryAllocateInfoKHR export_info = {VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, nullptr, handle_type};
    alloc_info.pNext = &export_info;

    // Add dedicated allocation info to pNext chain if required
    VkMemoryDedicatedAllocateInfoKHR dedicated_info = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR, nullptr,
                                                       VK_NULL_HANDLE, buffer_export.handle()};
    if (dedicated_allocation) {
        export_info.pNext = &dedicated_info;
    }

    // Allocate memory to be exported
    vk_testing::DeviceMemory memory_export;
    memory_export.init(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.bind_memory(memory_export, 0);

#ifdef _WIN32
    // Export memory to handle
    auto vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetInstanceProcAddr(instance(), "vkGetMemoryWin32HandleKHR");
    ASSERT_TRUE(vkGetMemoryWin32HandleKHR != nullptr);
    VkMemoryGetWin32HandleInfoKHR mghi = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr, memory_export.handle(),
                                          handle_type};
    HANDLE handle;
    ASSERT_VK_SUCCESS(vkGetMemoryWin32HandleKHR(m_device->device(), &mghi, &handle));

    VkImportMemoryWin32HandleInfoKHR import_info = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr, handle_type,
                                                    handle};
#else
    // Export memory to fd
    auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetInstanceProcAddr(instance(), "vkGetMemoryFdKHR");
    ASSERT_TRUE(vkGetMemoryFdKHR != nullptr);
    VkMemoryGetFdInfoKHR mgfi = {VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR, nullptr, memory_export.handle(), handle_type};
    int fd;
    ASSERT_VK_SUCCESS(vkGetMemoryFdKHR(m_device->device(), &mgfi, &fd));

    VkImportMemoryFdInfoKHR import_info = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR, nullptr, handle_type, fd};
#endif

    // Import memory
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_import.memory_requirements(), mem_flags);
    alloc_info.pNext = &import_info;
    vk_testing::DeviceMemory memory_import;
    memory_import.init(*m_device, alloc_info);

    // Bind imported memory
    buffer_import.bind_memory(memory_import, 0);

    // Create test buffers and fill input buffer
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkBufferObj buffer_input;
    buffer_input.init_as_src_and_dst(*m_device, buffer_size, mem_prop);
    auto input_mem = (uint8_t *)buffer_input.memory().map();
    for (uint32_t i = 0; i < buffer_size; i++) {
        input_mem[i] = (i & 0xFF);
    }
    buffer_input.memory().unmap();
    VkBufferObj buffer_output;
    buffer_output.init_as_src_and_dst(*m_device, buffer_size, mem_prop);

    // Copy from input buffer to output buffer through the exported/imported memory
    m_commandBuffer->begin();
    VkBufferCopy copy_info = {0, 0, buffer_size};
    vkCmdCopyBuffer(m_commandBuffer->handle(), buffer_input.handle(), buffer_export.handle(), 1, &copy_info);
    // Insert memory barrier to guarantee copy order
    VkMemoryBarrier mem_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
                                   VK_ACCESS_TRANSFER_READ_BIT};
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                         &mem_barrier, 0, nullptr, 0, nullptr);
    vkCmdCopyBuffer(m_commandBuffer->handle(), buffer_import.handle(), buffer_output.handle(), 1, &copy_info);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ParameterLayerFeatures2Capture) {
    TEST_DESCRIPTION("Ensure parameter_validation_layer correctly captures physical device features");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    VkResult err;
    m_errorMonitor->ExpectSuccess();

    VkPhysicalDeviceFeatures2KHR features2;
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    features2.pNext = nullptr;

    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    // We're not creating a valid m_device, but the phy wrapper is useful
    vk_testing::PhysicalDevice physical_device(gpu());
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = &features2;
    dev_info.flags = 0;
    dev_info.queueCreateInfoCount = create_queue_infos.size();
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = 0;
    dev_info.ppEnabledExtensionNames = nullptr;
    dev_info.pEnabledFeatures = nullptr;

    VkDevice device;
    err = vkCreateDevice(gpu(), &dev_info, nullptr, &device);
    ASSERT_VK_SUCCESS(err);

    if (features2.features.samplerAnisotropy) {
        // Test that the parameter layer is caching the features correctly using CreateSampler
        VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
        // If the features were not captured correctly, this should cause an error
        sampler_ci.anisotropyEnable = VK_TRUE;
        sampler_ci.maxAnisotropy = physical_device.properties().limits.maxSamplerAnisotropy;

        VkSampler sampler = VK_NULL_HANDLE;
        err = vkCreateSampler(device, &sampler_ci, nullptr, &sampler);
        ASSERT_VK_SUCCESS(err);
        vkDestroySampler(device, sampler, nullptr);
    } else {
        printf("%s Feature samplerAnisotropy not enabled;  parameter_layer check skipped.\n", kSkipPrefix);
    }

    // Verify the core validation layer has captured the physical device features by creating a a query pool.
    if (features2.features.pipelineStatisticsQuery) {
        VkQueryPool query_pool;
        VkQueryPoolCreateInfo qpci{};
        qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        qpci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        qpci.queryCount = 1;
        err = vkCreateQueryPool(device, &qpci, nullptr, &query_pool);
        ASSERT_VK_SUCCESS(err);

        vkDestroyQueryPool(device, query_pool, nullptr);
    } else {
        printf("%s Feature pipelineStatisticsQuery not enabled;  core_validation_layer check skipped.\n", kSkipPrefix);
    }

    vkDestroyDevice(device, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, GetMemoryRequirements2) {
    TEST_DESCRIPTION(
        "Get memory requirements with VK_KHR_get_memory_requirements2 instead of core entry points and verify layers do not emit "
        "errors when objects are bound and used");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for VK_KHR_get_memory_requirementes2 extensions
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    // Create a test buffer
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device,
                       VkBufferObj::create_info(1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT));

    // Use extension to get buffer memory requirements
    auto vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(
        vkGetDeviceProcAddr(m_device->device(), "vkGetBufferMemoryRequirements2KHR"));
    ASSERT_TRUE(vkGetBufferMemoryRequirements2KHR != nullptr);
    VkBufferMemoryRequirementsInfo2KHR buffer_info = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr,
                                                      buffer.handle()};
    VkMemoryRequirements2KHR buffer_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR};
    vkGetBufferMemoryRequirements2KHR(m_device->device(), &buffer_info, &buffer_reqs);

    // Allocate and bind buffer memory
    vk_testing::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_reqs.memoryRequirements, 0));
    vkBindBufferMemory(m_device->device(), buffer.handle(), buffer_memory.handle(), 0);

    // Create a test image
    auto image_ci = vk_testing::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vk_testing::Image image;
    image.init_no_mem(*m_device, image_ci);

    // Use extension to get image memory requirements
    auto vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(
        vkGetDeviceProcAddr(m_device->device(), "vkGetImageMemoryRequirements2KHR"));
    ASSERT_TRUE(vkGetImageMemoryRequirements2KHR != nullptr);
    VkImageMemoryRequirementsInfo2KHR image_info = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr,
                                                    image.handle()};
    VkMemoryRequirements2KHR image_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR};
    vkGetImageMemoryRequirements2KHR(m_device->device(), &image_info, &image_reqs);

    // Allocate and bind image memory
    vk_testing::DeviceMemory image_memory;
    image_memory.init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image_reqs.memoryRequirements, 0));
    vkBindImageMemory(m_device->device(), image.handle(), image_memory.handle(), 0);

    // Now execute arbitrary commands that use the test buffer and image
    m_commandBuffer->begin();

    // Fill buffer with 0
    vkCmdFillBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_WHOLE_SIZE, 0);

    // Transition and clear image
    const auto subresource_range = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    const auto barrier = image.image_memory_barrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                    VK_IMAGE_LAYOUT_GENERAL, subresource_range);
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
    const VkClearColorValue color = {};
    vkCmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);

    // Submit and verify no validation errors
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, BindMemory2) {
    TEST_DESCRIPTION(
        "Bind memory with VK_KHR_bind_memory2 instead of core entry points and verify layers do not emit errors when objects are "
        "used");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for VK_KHR_get_memory_requirementes2 extensions
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    // Create a test buffer
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, VkBufferObj::create_info(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT));

    // Allocate buffer memory
    vk_testing::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(), 0));

    // Bind buffer memory with extension
    auto vkBindBufferMemory2KHR =
        reinterpret_cast<PFN_vkBindBufferMemory2KHR>(vkGetDeviceProcAddr(m_device->device(), "vkBindBufferMemory2KHR"));
    ASSERT_TRUE(vkBindBufferMemory2KHR != nullptr);
    VkBindBufferMemoryInfoKHR buffer_bind_info = {VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR, nullptr, buffer.handle(),
                                                  buffer_memory.handle(), 0};
    vkBindBufferMemory2KHR(m_device->device(), 1, &buffer_bind_info);

    // Create a test image
    auto image_ci = vk_testing::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vk_testing::Image image;
    image.init_no_mem(*m_device, image_ci);

    // Allocate image memory
    vk_testing::DeviceMemory image_memory;
    image_memory.init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), 0));

    // Bind image memory with extension
    auto vkBindImageMemory2KHR =
        reinterpret_cast<PFN_vkBindImageMemory2KHR>(vkGetDeviceProcAddr(m_device->device(), "vkBindImageMemory2KHR"));
    ASSERT_TRUE(vkBindImageMemory2KHR != nullptr);
    VkBindImageMemoryInfoKHR image_bind_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO_KHR, nullptr, image.handle(),
                                                image_memory.handle(), 0};
    vkBindImageMemory2KHR(m_device->device(), 1, &image_bind_info);

    // Now execute arbitrary commands that use the test buffer and image
    m_commandBuffer->begin();

    // Fill buffer with 0
    vkCmdFillBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_WHOLE_SIZE, 0);

    // Transition and clear image
    const auto subresource_range = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    const auto barrier = image.image_memory_barrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                    VK_IMAGE_LAYOUT_GENERAL, subresource_range);
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
    const VkClearColorValue color = {};
    vkCmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);

    // Submit and verify no validation errors
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineWithCoreChecksDisabled) {
    TEST_DESCRIPTION("Test CreatePipeline while the CoreChecks validation object is disabled");

    // Enable KHR validation features extension
    VkValidationFeatureDisableEXT disables[] = {VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.disabledValidationFeatureCount = 1;
    features.pDisabledValidationFeatures = disables;

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, pool_flags, &features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipeineWithTessellationDomainOrigin) {
    TEST_DESCRIPTION(
        "Test CreatePipeline when VkPipelineTessellationStateCreateInfo.pNext include "
        "VkPipelineTessellationDomainOriginStateCreateInfo");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().tessellationShader) {
        printf("%s Device does not support tessellation shaders; skipped.\n", kSkipPrefix);
        return;
    }

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj tcs(m_device, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj tes(m_device, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationDomainOriginStateCreateInfo tessellationDomainOriginStateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO, VK_NULL_HANDLE,
        VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                                               &tessellationDomainOriginStateInfo, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, MultiplaneImageCopyBufferToImage) {
    TEST_DESCRIPTION("Positive test of multiplane copy buffer to image");
    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;  // All planes of equal extent
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {16, 16, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    VkImageObj image(m_device);
    image.init(&ci);

    m_commandBuffer->reset();
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->begin();
    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    std::array<VkImageAspectFlagBits, 3> aspects = {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT,
                                                    VK_IMAGE_ASPECT_PLANE_2_BIT};
    std::array<VkBufferObj, 3> buffers;
    VkMemoryPropertyFlags reqs = 0;

    VkBufferImageCopy copy = {};
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent.depth = 1;
    copy.imageExtent.height = 16;
    copy.imageExtent.width = 16;

    for (size_t i = 0; i < aspects.size(); ++i) {
        buffers[i].init_as_src(*m_device, (VkDeviceSize)16 * 16 * 1, reqs);
        copy.imageSubresource.aspectMask = aspects[i];
        vkCmdCopyBufferToImage(m_commandBuffer->handle(), buffers[i].handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &copy);
    }
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, MultiplaneImageTests) {
    TEST_DESCRIPTION("Positive test of multiplane image operations");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create aliased function pointers for 1.0 and 1.1 contexts

    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
    PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2Function = nullptr;

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkBindImageMemory2Function = vkBindImageMemory2;
        vkGetImageMemoryRequirements2Function = vkGetImageMemoryRequirements2;
        vkGetPhysicalDeviceMemoryProperties2Function = vkGetPhysicalDeviceMemoryProperties2;
    } else {
        vkBindImageMemory2Function = (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
        vkGetImageMemoryRequirements2Function =
            (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
        vkGetPhysicalDeviceMemoryProperties2Function = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetDeviceProcAddr(
            m_device->handle(), "vkGetPhysicalDeviceMemoryProperties2KHR");
    }

    if (!vkBindImageMemory2Function || !vkGetImageMemoryRequirements2Function || !vkGetPhysicalDeviceMemoryProperties2Function) {
        printf("%s Did not find required device extension support; test skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;  // All planes of equal extent
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    VkImage image;
    ASSERT_VK_SUCCESS(vkCreateImage(device(), &ci, NULL, &image));

    // Allocate & bind memory
    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vkGetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(device(), image, &mem_reqs);
    VkDeviceMemory mem_obj = VK_NULL_HANDLE;
    VkMemoryPropertyFlagBits mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t type = 0; type < phys_mem_props.memoryTypeCount; type++) {
        if ((mem_reqs.memoryTypeBits & (1 << type)) &&
            ((phys_mem_props.memoryTypes[type].propertyFlags & mem_props) == mem_props)) {
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_reqs.size;
            alloc_info.memoryTypeIndex = type;
            ASSERT_VK_SUCCESS(vkAllocateMemory(device(), &alloc_info, NULL, &mem_obj));
            break;
        }
    }

    if (VK_NULL_HANDLE == mem_obj) {
        printf("%s Unable to allocate image memory. Skipping test.\n", kSkipPrefix);
        vkDestroyImage(device(), image, NULL);
        return;
    }
    ASSERT_VK_SUCCESS(vkBindImageMemory(device(), image, mem_obj, 0));

    // Copy plane 0 to plane 2
    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent.width = 128;
    copyRegion.extent.height = 128;
    copyRegion.extent.depth = 1;

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->begin();
    m_commandBuffer->CopyImage(image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    vkFreeMemory(device(), mem_obj, NULL);
    vkDestroyImage(device(), image, NULL);

    // Repeat bind test on a DISJOINT multi-planar image, with per-plane memory objects, using API2 variants
    //
    features |= VK_FORMAT_FEATURE_DISJOINT_BIT;
    ci.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    if (ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features)) {
        ASSERT_VK_SUCCESS(vkCreateImage(device(), &ci, NULL, &image));

        // Allocate & bind memory
        VkPhysicalDeviceMemoryProperties2 phys_mem_props2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        vkGetPhysicalDeviceMemoryProperties2Function(gpu(), &phys_mem_props2);
        VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_reqs2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};

        VkDeviceMemory p0_mem, p1_mem, p2_mem;
        mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

        // Plane 0
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_reqs2);
        uint32_t mem_type = 0;
        for (mem_type = 0; mem_type < phys_mem_props2.memoryProperties.memoryTypeCount; mem_type++) {
            if ((mem_reqs2.memoryRequirements.memoryTypeBits & (1 << mem_type)) &&
                ((phys_mem_props2.memoryProperties.memoryTypes[mem_type].propertyFlags & mem_props) == mem_props)) {
                alloc_info.memoryTypeIndex = mem_type;
                break;
            }
        }
        alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
        ASSERT_VK_SUCCESS(vkAllocateMemory(device(), &alloc_info, NULL, &p0_mem));

        // Plane 1 & 2 use same memory type
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_reqs2);
        alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
        ASSERT_VK_SUCCESS(vkAllocateMemory(device(), &alloc_info, NULL, &p1_mem));

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_reqs2);
        alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
        ASSERT_VK_SUCCESS(vkAllocateMemory(device(), &alloc_info, NULL, &p2_mem));

        // Set up 3-plane binding
        VkBindImageMemoryInfo bind_info[3];
        for (int plane = 0; plane < 3; plane++) {
            bind_info[plane].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_info[plane].pNext = nullptr;
            bind_info[plane].image = image;
            bind_info[plane].memoryOffset = 0;
        }
        bind_info[0].memory = p0_mem;
        bind_info[1].memory = p1_mem;
        bind_info[2].memory = p2_mem;

        m_errorMonitor->ExpectSuccess();
        vkBindImageMemory2Function(device(), 3, bind_info);
        m_errorMonitor->VerifyNotFound();

        vkFreeMemory(device(), p0_mem, NULL);
        vkFreeMemory(device(), p1_mem, NULL);
        vkFreeMemory(device(), p2_mem, NULL);
        vkDestroyImage(device(), image, NULL);
    }

    // Test that changing the layout of ASPECT_COLOR also changes the layout of the individual planes
    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src(*m_device, (VkDeviceSize)128 * 128 * 3, reqs);
    VkImageObj mpimage(m_device);
    mpimage.Init(256, 256, 1, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                 VK_IMAGE_TILING_OPTIMAL, 0);
    VkBufferImageCopy copy_region = {};
    copy_region.bufferRowLength = 128;
    copy_region.bufferImageHeight = 128;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.height = 64;
    copy_region.imageExtent.width = 64;
    copy_region.imageExtent.depth = 1;

    vkResetCommandBuffer(m_commandBuffer->handle(), 0);
    m_commandBuffer->begin();
    mpimage.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkCmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), mpimage.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &copy_region);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyNotFound();

    // Test to verify that views of multiplanar images have layouts tracked correctly
    // by changing the image's layout then using a view of that image
    VkImageView view;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = mpimage.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkCreateImageView(m_device->device(), &ivci, nullptr, &view);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.UpdateDescriptorSets();

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->begin();
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.image = mpimage.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    vkQueueWaitIdle(m_device->m_queue);
    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(VkPositiveLayerTest, ApiVersionZero) {
    TEST_DESCRIPTION("Check that apiVersion = 0 is valid.");
    m_errorMonitor->ExpectSuccess();
    app_info.apiVersion = 0U;
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, RayTracingPipelineNV) {
    TEST_DESCRIPTION("Test VK_NV_ray_tracing.");

    if (!CreateNVRayTracingPipelineHelper::InitInstanceExtensions(*this, m_instance_extension_names)) {
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    if (!CreateNVRayTracingPipelineHelper::InitDeviceExtensions(*this, m_device_extension_names)) {
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto ignore_update = [](CreateNVRayTracingPipelineHelper &helper) {};
    CreateNVRayTracingPipelineHelper::OneshotPositiveTest(*this, ignore_update);
}

TEST_F(VkPositiveLayerTest, ViewportArray2NV) {
    TEST_DESCRIPTION("Test to validate VK_NV_viewport_array2");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));

    if (!available_features.multiViewport) {
        printf("%s VkPhysicalDeviceFeatures::multiViewport is not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    if (!available_features.tessellationShader) {
        printf("%s VkPhysicalDeviceFeatures::tessellationShader is not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    if (!available_features.geometryShader) {
        printf("%s VkPhysicalDeviceFeatures::geometryShader is not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char tcs_src[] = R"(
        #version 450
        layout(vertices = 3) out;

        void main() {
            gl_TessLevelOuter[0] = 4.0f;
            gl_TessLevelOuter[1] = 4.0f;
            gl_TessLevelOuter[2] = 4.0f;
            gl_TessLevelInner[0] = 3.0f;

            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        })";

    // Create tessellation control and fragment shader here since they will not be
    // modified by the different test cases.
    VkShaderObj tcs(m_device, tcs_src, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    std::vector<VkViewport> vps = {{0.0f, 0.0f, m_width / 2.0f, m_height}, {m_width / 2.0f, 0.0f, m_width / 2.0f, m_height}};
    std::vector<VkRect2D> scs = {
        {{0, 0}, {static_cast<uint32_t>(m_width) / 2, static_cast<uint32_t>(m_height)}},
        {{static_cast<int32_t>(m_width) / 2, 0}, {static_cast<uint32_t>(m_width) / 2, static_cast<uint32_t>(m_height)}}};

    enum class TestStage { VERTEX = 0, TESSELLATION_EVAL = 1, GEOMETRY = 2 };
    std::array<TestStage, 3> vertex_stages = {{TestStage::VERTEX, TestStage::TESSELLATION_EVAL, TestStage::GEOMETRY}};

    // Verify that the usage of gl_ViewportMask[] in the allowed vertex processing
    // stages does not cause any errors.
    for (auto stage : vertex_stages) {
        m_errorMonitor->ExpectSuccess();

        VkPipelineInputAssemblyStateCreateInfo iaci = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        iaci.topology = (stage != TestStage::VERTEX) ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineTessellationStateCreateInfo tsci = {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
        tsci.patchControlPoints = 3;

        const VkPipelineLayoutObj pl(m_device);

        VkPipelineObj pipe(m_device);
        pipe.AddDefaultColorAttachment();
        pipe.SetInputAssembly(&iaci);
        pipe.SetViewport(vps);
        pipe.SetScissor(scs);
        pipe.AddShader(&fs);

        std::stringstream vs_src, tes_src, geom_src;

        vs_src << R"(
            #version 450
            #extension GL_NV_viewport_array2 : require

            vec2 positions[3] = { vec2( 0.0f, -0.5f),
                                  vec2( 0.5f,  0.5f),
                                  vec2(-0.5f,  0.5f)
                                };
            void main() {)";
        // Write viewportMask if the vertex shader is the last vertex processing stage.
        if (stage == TestStage::VERTEX) {
            vs_src << "gl_ViewportMask[0] = 3;\n";
        }
        vs_src << R"(
                gl_Position = vec4(positions[gl_VertexIndex % 3], 0.0, 1.0);
            })";

        VkShaderObj vs(m_device, vs_src.str().c_str(), VK_SHADER_STAGE_VERTEX_BIT, this);
        pipe.AddShader(&vs);

        std::unique_ptr<VkShaderObj> tes, geom;

        if (stage >= TestStage::TESSELLATION_EVAL) {
            tes_src << R"(
                #version 450
                #extension GL_NV_viewport_array2 : require
                layout(triangles) in;

                void main() {
                   gl_Position = (gl_in[0].gl_Position * gl_TessCoord.x +
                                  gl_in[1].gl_Position * gl_TessCoord.y +
                                  gl_in[2].gl_Position * gl_TessCoord.z);)";
            // Write viewportMask if the tess eval shader is the last vertex processing stage.
            if (stage == TestStage::TESSELLATION_EVAL) {
                tes_src << "gl_ViewportMask[0] = 3;\n";
            }
            tes_src << "}";

            tes = std::unique_ptr<VkShaderObj>(
                new VkShaderObj(m_device, tes_src.str().c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this));
            pipe.AddShader(tes.get());
            pipe.AddShader(&tcs);
            pipe.SetTessellation(&tsci);
        }

        if (stage >= TestStage::GEOMETRY) {
            geom_src << R"(
                #version 450
                #extension GL_NV_viewport_array2 : require
                layout(triangles)   in;
                layout(triangle_strip, max_vertices = 3) out;

                void main() {
                   gl_ViewportMask[0] = 3;
                   for(int i = 0; i < 3; ++i) {
                       gl_Position = gl_in[i].gl_Position;
                       EmitVertex();
                    }
                })";

            geom =
                std::unique_ptr<VkShaderObj>(new VkShaderObj(m_device, geom_src.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT, this));
            pipe.AddShader(geom.get());
        }

        pipe.CreateVKPipeline(pl.handle(), renderPass());
        m_errorMonitor->VerifyNotFound();
    }
}

TEST_F(VkPositiveLayerTest, HostQueryResetSuccess) {
    // This is a positive test. No failures are expected.
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT normally");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    m_errorMonitor->ExpectSuccess();

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineFragmentOutputNotConsumedButAlphaToCoverageEnabled) {
    TEST_DESCRIPTION(
        "Test that no warning is produced when writing to non-existing color attachment if alpha to coverage is enabled.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(0u));

    VkPipelineMultisampleStateCreateInfo ms_state_ci = {};
    ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.pipe_ms_state_ci_ = ms_state_ci;
        helper.cb_ci_.attachmentCount = 0;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, "", true);
}

TEST_F(VkPositiveLayerTest, UseFirstQueueUnqueried) {
    TEST_DESCRIPTION("Use first queue family and one queue without first querying with vkGetPhysicalDeviceQueueFamilyProperties");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = {};
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = {};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    m_errorMonitor->ExpectSuccess();
    VkDevice test_device;
    vkCreateDevice(gpu(), &device_ci, nullptr, &test_device);
    m_errorMonitor->VerifyNotFound();

    vkDestroyDevice(test_device, nullptr);
}

// Android loader returns an error in this case
#if !defined(ANDROID)
TEST_F(VkPositiveLayerTest, GetDevProcAddrNullPtr) {
    TEST_DESCRIPTION("Call GetDeviceProcAddr on an enabled instance extension expecting nullptr");
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_SURFACE_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->ExpectSuccess();
    auto fpDestroySurface = (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(m_device->device(), "vkDestroySurfaceKHR");
    if (fpDestroySurface) {
        m_errorMonitor->SetError("Null was expected!");
    }
    m_errorMonitor->VerifyNotFound();
}
#endif

TEST_F(VkPositiveLayerTest, CmdCopySwapchainImage) {
    TEST_DESCRIPTION("Run vkCmdCopyImage with a swapchain image");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    printf(
        "%s According to VUID-01631, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
        "skipping CmdCopySwapchainImage test\n",
        kSkipPrefix);
    return;
#endif

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping CmdCopySwapchainImage test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping CmdCopySwapchainImage test\n", kSkipPrefix);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s VkBindImageMemoryInfo requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping CmdCopySwapchainImage test\n", kSkipPrefix);
        return;
    }

    auto image_create_info = lvl_init_struct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj srcImage(m_device);
    srcImage.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    auto image_swapchain_create_info = lvl_init_struct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    VkImage image_from_swapchain;
    vkCreateImage(device(), &image_create_info, NULL, &image_from_swapchain);

    auto bind_swapchain_info = lvl_init_struct<VkBindImageMemorySwapchainInfoKHR>();
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    auto bind_info = lvl_init_struct<VkBindImageMemoryInfo>(&bind_swapchain_info);
    bind_info.image = image_from_swapchain;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vkBindImageMemory2(m_device->device(), 1, &bind_info);

    VkImageCopy copy_region = {};
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
    copy_region.extent = {10, 10, 1};

    m_commandBuffer->begin();

    m_errorMonitor->ExpectSuccess();
    vkCmdCopyImage(m_commandBuffer->handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, image_from_swapchain,
                   VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyNotFound();

    vkDestroyImage(m_device->device(), image_from_swapchain, NULL);
    DestroySwapchain();
}

TEST_F(VkPositiveLayerTest, TransferImageToSwapchainDeviceGroup) {
    TEST_DESCRIPTION("Transfer an image to a swapchain's image  between device group");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    printf(
        "%s According to VUID-01631, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
        "skipping test\n",
        kSkipPrefix);
    return;
#endif

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s VkBindImageMemoryInfo requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    uint32_t physical_device_group_count = 0;
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        printf("%s physical_device_group_count is 0, skipping test\n", kSkipPrefix);
        return;
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = {};
    create_device_pnext.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        printf("%s Cannot create surface or swapchain, skipping test\n", kSkipPrefix);
        return;
    }

    auto image_create_info = lvl_init_struct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj src_Image(m_device);
    src_Image.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_ALIAS_BIT;

    auto image_swapchain_create_info = lvl_init_struct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    VkImage peer_image;
    vkCreateImage(device(), &image_create_info, NULL, &peer_image);

    auto bind_devicegroup_info = lvl_init_struct<VkBindImageMemoryDeviceGroupInfo>();
    bind_devicegroup_info.deviceIndexCount = 2;
    std::array<uint32_t, 2> deviceIndices = {0, 0};
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    auto bind_swapchain_info = lvl_init_struct<VkBindImageMemorySwapchainInfoKHR>(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    auto bind_info = lvl_init_struct<VkBindImageMemoryInfo>(&bind_swapchain_info);
    bind_info.image = peer_image;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vkBindImageMemory2(m_device->device(), 1, &bind_info);

    uint32_t swapchain_images_count = 0;
    vkGetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vkGetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    m_commandBuffer->begin();

    auto img_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = swapchain_images[0];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &img_barrier);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    VkImageCopy copy_region = {};
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
    copy_region.extent = {10, 10, 1};
    vkCmdCopyImage(m_commandBuffer->handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_commandBuffer->end();
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyNotFound();

    vkDestroyImage(m_device->device(), peer_image, NULL);
    DestroySwapchain();
}

TEST_F(VkPositiveLayerTest, RenderPassValidStages) {
    TEST_DESCRIPTION("Create render pass with valid stages");

    bool rp2_supported = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (rp2_supported) m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (rp2_supported) rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription sci[2] = {};
    sci[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    // to be filled later by tests

    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
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

TEST_F(VkPositiveLayerTest, SampleMaskOverrideCoverageNV) {
    TEST_DESCRIPTION("Test to validate VK_NV_sample_mask_override_coverage");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    const char vs_src[] = R"(
        #version 450
        layout(location=0) out vec4  fragColor;

        const vec2 pos[3] = { vec2( 0.0f, -0.5f),
                              vec2( 0.5f,  0.5f),
                              vec2(-0.5f,  0.5f)
                            };
        void main()
        {
            gl_Position = vec4(pos[gl_VertexIndex % 3], 0.0f, 1.0f);
            fragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        })";

    const char fs_src[] = R"(
        #version 450
        #extension GL_NV_sample_mask_override_coverage : require

        layout(location = 0) in  vec4 fragColor;
        layout(location = 0) out vec4 outColor;

        layout(override_coverage) out int gl_SampleMask[];

        void main()
        {
            gl_SampleMask[0] = 0xff;
            outColor = fragColor;
        })";

    m_errorMonitor->ExpectSuccess();

    const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_8_BIT;

    VkAttachmentDescription cAttachment = {};
    cAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
    cAttachment.samples = sampleCount;
    cAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    cAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    cAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    cAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    cAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference cAttachRef = {};
    cAttachRef.attachment = 0;
    cAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &cAttachRef;

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpci.attachmentCount = 1;
    rpci.pAttachments = &cAttachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    VkRenderPass rp;
    vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);

    const VkPipelineLayoutObj pl(m_device);

    VkSampleMask sampleMask = 0x01;
    VkPipelineMultisampleStateCreateInfo msaa = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msaa.rasterizationSamples = sampleCount;
    msaa.sampleShadingEnable = VK_FALSE;
    msaa.pSampleMask = &sampleMask;

    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.SetMSAA(&msaa);

    VkShaderObj vs(m_device, vs_src, VK_SHADER_STAGE_VERTEX_BIT, this);
    pipe.AddShader(&vs);

    VkShaderObj fs(m_device, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    pipe.AddShader(&fs);

    // Create pipeline and make sure that the usage of NV_sample_mask_override_coverage
    // in the fragment shader does not cause any errors.
    pipe.CreateVKPipeline(pl.handle(), rp);

    vkDestroyRenderPass(m_device->device(), rp, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, TestRasterizationDiscardEnableTrue) {
    TEST_DESCRIPTION("Ensure it doesn't crash and trigger error msg when rasterizerDiscardEnable = true");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkAttachmentDescription att[1] = {{}};
    att[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    att[0].samples = VK_SAMPLE_COUNT_4_BIT;
    att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sp = {};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1;
    sp.pColorAttachments = &cr;
    VkRenderPassCreateInfo rpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpi.attachmentCount = 1;
    rpi.pAttachments = att;
    rpi.subpassCount = 1;
    rpi.pSubpasses = &sp;
    VkRenderPass rp;
    vkCreateRenderPass(m_device->device(), &rpi, nullptr, &rp);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pViewportState = nullptr;
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.gp_ci_.pDepthStencilState = nullptr;
    pipe.gp_ci_.pColorBlendState = nullptr;
    pipe.gp_ci_.renderPass = rp;

    m_errorMonitor->ExpectSuccess();
    // Skip the test in NexusPlayer. The driver crashes when pViewportState, pMultisampleState, pDepthStencilState, pColorBlendState
    // are NULL.
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkPositiveLayerTest, TestSamplerDataForCombinedImageSampler) {
    TEST_DESCRIPTION("Shader code uses sampler data for CombinedImageSampler");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string fsSource = R"(
                   OpCapability Shader
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint Fragment %main "main"
                   OpExecutionMode %main OriginUpperLeft

                   OpDecorate %InputData DescriptorSet 0
                   OpDecorate %InputData Binding 0
                   OpDecorate %SamplerData DescriptorSet 0
                   OpDecorate %SamplerData Binding 0

               %void = OpTypeVoid
                %f32 = OpTypeFloat 32
              %Image = OpTypeImage %f32 2D 0 0 0 1 Rgba32f
           %ImagePtr = OpTypePointer UniformConstant %Image
          %InputData = OpVariable %ImagePtr UniformConstant
            %Sampler = OpTypeSampler
         %SamplerPtr = OpTypePointer UniformConstant %Sampler
        %SamplerData = OpVariable %SamplerPtr UniformConstant
       %SampledImage = OpTypeSampledImage %Image

               %func = OpTypeFunction %void
               %main = OpFunction %void None %func
                 %40 = OpLabel
           %call_smp = OpLoad %Sampler %SamplerData
                   OpReturn
                   OpFunctionEnd)";

    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.shader_stages_ = {fs.GetStageCreateInfo(), pipe.vs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vkCreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler);

    uint32_t qfi = 0;
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &qfi;

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                            &pipe.descriptor_set_->set_, 0, NULL);

    m_errorMonitor->ExpectSuccess();
    vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyNotFound();

    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkPositiveLayerTest, NotPointSizeGeometryShaderSuccess) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST, but geometry shader doesn't include PointSize.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if ((!m_device->phy().features().geometryShader)) {
        printf("%s Device does not support the required geometry shader features; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj gs(m_device, bindStateGeomShaderText, VK_SHADER_STAGE_GEOMETRY_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.InitState();

    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, SubpassWithReadOnlyLayoutWithoutDependency) {
    TEST_DESCRIPTION("When both subpasses' attachments are the same and layouts are read-only, they don't need dependency.");
    ASSERT_NO_FATAL_FAILURE(Init());

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

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
    std::array<VkAttachmentDescription, size> attachments = {attachment, attachment};

    VkAttachmentReference att_ref_depth_stencil = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

    std::array<VkSubpassDescription, size> subpasses;
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, 0, nullptr, nullptr, &att_ref_depth_stencil, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, 0, nullptr, nullptr, &att_ref_depth_stencil, 0, nullptr};

    VkRenderPassCreateInfo rpci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, size, attachments.data(), size, subpasses.data(), 0, nullptr};

    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

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

    VkImageView view;
    err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);
    std::array<VkImageView, size> views = {view, view};

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, size, views.data(), 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {32, 32}}, 0, nullptr};
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkDestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(VkPositiveLayerTest, GeometryShaderPassthroughNV) {
    TEST_DESCRIPTION("Test to validate VK_NV_geometry_shader_passthrough");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));

    if (!available_features.geometryShader) {
        printf("%s VkPhysicalDeviceFeatures::geometryShader is not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char vs_src[] = R"(
        #version 450

        out gl_PerVertex {
            vec4 gl_Position;
        };

        layout(location = 0) out ColorBlock {vec4 vertexColor;};

        const vec2 positions[3] = { vec2( 0.0f, -0.5f),
                                    vec2( 0.5f,  0.5f),
                                    vec2(-0.5f,  0.5f)
                                  };

        const vec4 colors[3] = { vec4(1.0f, 0.0f, 0.0f, 1.0f),
                                 vec4(0.0f, 1.0f, 0.0f, 1.0f),
                                 vec4(0.0f, 0.0f, 1.0f, 1.0f)
                               };
        void main()
        {
            vertexColor = colors[gl_VertexIndex % 3];
            gl_Position = vec4(positions[gl_VertexIndex % 3], 0.0, 1.0);
        })";

    const char gs_src[] = R"(
        #version 450
        #extension GL_NV_geometry_shader_passthrough: require

        layout(triangles) in;
        layout(triangle_strip, max_vertices = 3) out;

        layout(passthrough) in gl_PerVertex {vec4 gl_Position;};
        layout(location = 0, passthrough) in ColorBlock {vec4 vertexColor;};

        void main()
        {
           gl_Layer = 0;
        })";

    const char fs_src[] = R"(
        #version 450

        layout(location = 0) in ColorBlock {vec4 vertexColor;};
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vertexColor;
        })";

    m_errorMonitor->ExpectSuccess();

    const VkPipelineLayoutObj pl(m_device);

    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();

    VkShaderObj vs(m_device, vs_src, VK_SHADER_STAGE_VERTEX_BIT, this);
    pipe.AddShader(&vs);

    VkShaderObj gs(m_device, gs_src, VK_SHADER_STAGE_GEOMETRY_BIT, this);
    pipe.AddShader(&gs);

    VkShaderObj fs(m_device, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    pipe.AddShader(&fs);

    // Create pipeline and make sure that the usage of NV_geometry_shader_passthrough
    // in the fragment shader does not cause any errors.
    pipe.CreateVKPipeline(pl.handle(), renderPass());

    m_errorMonitor->VerifyNotFound();
}
