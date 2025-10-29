/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/render_pass_helper.h"

class NegativeImagelessFramebuffer : public VkLayerTest {};

TEST_F(NegativeImagelessFramebuffer, RenderPassBeginImageViewMismatch) {
    TEST_DESCRIPTION(
        "Begin a renderPass where the image views specified do not match the parameters used to create the framebuffer and render "
        "pass.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());

    bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat attachment_formats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    VkFormat framebuffer_attachment_formats[3] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachment_formats[0]);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    framebuffer_attachment_image_info.width = attachment_width;
    framebuffer_attachment_image_info.height = attachment_height;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 2;
    framebuffer_attachment_image_info.pViewFormats = framebuffer_attachment_formats;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 1;
    framebuffer_attachment_ci.pAttachmentImageInfos = &framebuffer_attachment_image_info;
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachment_ci);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = nullptr;
    framebuffer_ci.renderPass = rp;

    VkImageFormatListCreateInfo imageFormatListCreateInfo = vku::InitStructHelper();
    imageFormatListCreateInfo.viewFormatCount = 2;
    imageFormatListCreateInfo.pViewFormats = attachment_formats;
    VkImageCreateInfo image_ci = vku::InitStructHelper(&imageFormatListCreateInfo);
    image_ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_ci.extent = {attachment_width, attachment_height, 1};
    image_ci.arrayLayers = 1;
    image_ci.mipLevels = 10;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.format = attachment_formats[0];
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    // Only use the subset without the TRANSFER bit
    VkImageViewUsageCreateInfo image_view_usage_create_info = vku::InitStructHelper();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(&image_view_usage_create_info);
    image_view_ci.image = image;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = attachment_formats[0];
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // Has subset of usage flags
    vkt::ImageView image_view_subset(*m_device, image_view_ci);

    image_view_ci.pNext = nullptr;
    vkt::ImageView image_view(*m_device, image_view_ci);

    VkImageView image_views[2] = {image_view, image_view};
    VkRenderPassAttachmentBeginInfo rp_attachment_begin_info = vku::InitStructHelper();
    rp_attachment_begin_info.attachmentCount = 1;
    rp_attachment_begin_info.pAttachments = image_views;
    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper(&rp_attachment_begin_info);
    rp_begin_info.renderPass = rp;
    rp_begin_info.renderArea.extent = {attachment_width, attachment_height};

    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    // Positive test first
    {
        framebuffer_ci.pAttachments = nullptr;
        framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        m_command_buffer.Begin(&cmd_begin_info);
        m_command_buffer.BeginRenderPass(rp_begin_info);
        m_command_buffer.Reset();
    }

    // Imageless framebuffer creation bit not present
    {
        framebuffer_ci.pAttachments = &image_view.handle();
        framebuffer_ci.flags = 0;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03207",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03207");
    }
    {
        framebuffer_ci.pAttachments = nullptr;
        framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_attachment_begin_info.attachmentCount = 2;
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03208",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03208");
        rp_attachment_begin_info.attachmentCount = 1;
    }

    // Mismatched number of attachments
    {
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_attachment_begin_info.attachmentCount = 2;
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03208",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03208");
        rp_attachment_begin_info.attachmentCount = 1;
    }

    // Mismatched flags
    {
        framebuffer_attachment_image_info.flags = 0;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03209",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03209");
        framebuffer_attachment_image_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    // Mismatched usage
    {
        framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-04627",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-04627");
        framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Mismatched usage because VkImageViewUsageCreateInfo restricted to TRANSFER
    {
        rp_attachment_begin_info.pAttachments = &image_view_subset.handle();
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-04627",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-04627");
        rp_attachment_begin_info.pAttachments = &image_view.handle();
    }

    // Mismatched width
    {
        framebuffer_attachment_image_info.width += 1;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03211",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03211");
        framebuffer_attachment_image_info.width -= 1;
    }

    // Mismatched height
    {
        framebuffer_attachment_image_info.height += 1;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03212",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03212");
        framebuffer_attachment_image_info.height -= 1;
    }

    // Mismatched layer count
    {
        framebuffer_attachment_image_info.layerCount += 1;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03213",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03213");
        framebuffer_attachment_image_info.layerCount -= 1;
    }

    // Mismatched view format count
    {
        framebuffer_attachment_image_info.viewFormatCount = 3;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03214",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03214");
        framebuffer_attachment_image_info.viewFormatCount = 2;
    }

    // Mismatched format lists
    {
        framebuffer_attachment_formats[1] = VK_FORMAT_B8G8R8A8_SRGB;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03215",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03215");
        framebuffer_attachment_formats[1] = VK_FORMAT_B8G8R8A8_UNORM;
    }

    // Mismatched formats
    {
        image_view_ci.format = attachment_formats[1];
        vkt::ImageView image_view2(*m_device, image_view_ci);
        rp_attachment_begin_info.pAttachments = &image_view2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-03216",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-03216");
        rp_attachment_begin_info.pAttachments = &image_view.handle();
        image_view_ci.format = attachment_formats[0];
    }

    // Mismatched sample counts
    {
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.mipLevels = 1;
        vkt::Image imageObject2(*m_device, image_ci, vkt::set_layout);
        image_view_ci.image = imageObject2;
        vkt::ImageView image_view2(*m_device, image_view_ci);
        rp_attachment_begin_info.pAttachments = &image_view2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported, "VUID-VkRenderPassBeginInfo-framebuffer-09047",
                                  "VUID-VkRenderPassBeginInfo-framebuffer-09047");
        rp_attachment_begin_info.pAttachments = &image_view.handle();
        image_view_ci.image = image;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 10;
    }

    // Mismatched level counts
    {
        image_view_ci.subresourceRange.levelCount = 2;
        vkt::ImageView image_view2(*m_device, image_view_ci);
        rp_attachment_begin_info.pAttachments = &image_view2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported,
                                  "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218",
                                  "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218");
        rp_attachment_begin_info.pAttachments = &image_view.handle();
        image_view_ci.subresourceRange.levelCount = 1;
    }

    // Non-identity component swizzle
    {
        image_view_ci.components.r = VK_COMPONENT_SWIZZLE_A;
        vkt::ImageView image_view2(*m_device, image_view_ci);
        rp_attachment_begin_info.pAttachments = &image_view2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported,
                                  "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219",
                                  "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219");
        rp_attachment_begin_info.pAttachments = &image_view.handle();
        image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    }

    {
        image_view_ci.subresourceRange.baseMipLevel = 1;
        vkt::ImageView image_view2(*m_device, image_view_ci);
        rp_attachment_begin_info.pAttachments = &image_view2.handle();
        framebuffer_attachment_image_info.height = framebuffer_attachment_image_info.height / 2;
        framebuffer_attachment_image_info.width = framebuffer_attachment_image_info.width / 2;
        framebuffer_ci.height = framebuffer_ci.height / 2;
        framebuffer_ci.width = framebuffer_ci.width / 2;
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        rp_begin_info.framebuffer = framebuffer;
        rp_begin_info.renderArea.extent.height = rp_begin_info.renderArea.extent.height / 2;
        rp_begin_info.renderArea.extent.width = rp_begin_info.renderArea.extent.width / 2;
        m_command_buffer.Begin(&cmd_begin_info);
        m_command_buffer.BeginRenderPass(rp_begin_info);
        m_command_buffer.Reset();
        rp_attachment_begin_info.pAttachments = &image_view.handle();
        image_view_ci.subresourceRange.baseMipLevel = 0;
        framebuffer_attachment_image_info.height = framebuffer_attachment_image_info.height * 2;
        framebuffer_attachment_image_info.width = framebuffer_attachment_image_info.width * 2;
    }
}

TEST_F(NegativeImagelessFramebuffer, FeatureEnable) {
    TEST_DESCRIPTION("Use imageless framebuffer functionality without enabling the feature");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat attachment_format = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachment_format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = attachment_width;
    framebuffer_attachment_image_info.height = attachment_height;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &attachment_format;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 1;
    framebuffer_attachment_ci.pAttachmentImageInfos = &framebuffer_attachment_image_info;
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachment_ci);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = rp;
    framebuffer_ci.attachmentCount = 1;

    // Imageless framebuffer creation bit not present
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03189");
    vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, BasicUsage) {
    TEST_DESCRIPTION("Create an imageless framebuffer in various invalid ways");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat attachment_format = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachment_format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = attachment_width;
    framebuffer_attachment_image_info.height = attachment_height;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &attachment_format;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 1;
    framebuffer_attachment_ci.pAttachmentImageInfos = &framebuffer_attachment_image_info;
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachment_ci);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = rp;
    framebuffer_ci.attachmentCount = 1;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Attachments info not present
    framebuffer_ci.pNext = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03190");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_ci.pNext = &framebuffer_attachment_ci;

    // Mismatched attachment counts
    framebuffer_attachment_ci.attachmentImageInfoCount = 2;
    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_infos[2] = {framebuffer_attachment_image_info,
                                                                              framebuffer_attachment_image_info};
    framebuffer_attachment_ci.pAttachmentImageInfos = framebuffer_attachment_image_infos;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03191");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_ci.pAttachmentImageInfos = &framebuffer_attachment_image_info;
    framebuffer_attachment_ci.attachmentImageInfoCount = 1;

    // Mismatched format list
    attachment_format = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    attachment_format = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched format list
    attachment_format = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    attachment_format = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched layer count, multiview disabled
    framebuffer_ci.layers = 2;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-04546");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_ci.layers = 1;

    // Mismatched width
    framebuffer_ci.width += 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04541");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_ci.width -= 1;

    // Mismatched height
    framebuffer_ci.height += 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04542");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_ci.height -= 1;
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImageUsageMismatch) {
    TEST_DESCRIPTION("Create an imageless framebuffer with mismatched attachment image usage");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat color_input_attachment_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depth_stencil_attachment_format = VK_FORMAT_D32_SFLOAT_S8_UINT;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(color_input_attachment_format, VK_SAMPLE_COUNT_4_BIT);    // Color attachment
    rp.AddAttachmentDescription(color_input_attachment_format);                           // Color resolve attachment
    rp.AddAttachmentDescription(depth_stencil_attachment_format, VK_SAMPLE_COUNT_4_BIT);  // Depth stencil attachment
    rp.AddAttachmentDescription(color_input_attachment_format);                           // Input attachment
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({2, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({3, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.AddDepthStencilAttachment(2);
    rp.AddInputAttachment(3);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_infos[4] = {};
    // Color attachment
    framebuffer_attachment_image_infos[0] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[0].width = attachment_width;
    framebuffer_attachment_image_infos[0].height = attachment_height;
    framebuffer_attachment_image_infos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[0].layerCount = 1;
    framebuffer_attachment_image_infos[0].viewFormatCount = 1;
    framebuffer_attachment_image_infos[0].pViewFormats = &color_input_attachment_format;
    // Color resolve attachment
    framebuffer_attachment_image_infos[1] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[1].width = attachment_width;
    framebuffer_attachment_image_infos[1].height = attachment_height;
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[1].layerCount = 1;
    framebuffer_attachment_image_infos[1].viewFormatCount = 1;
    framebuffer_attachment_image_infos[1].pViewFormats = &color_input_attachment_format;
    // Depth stencil attachment
    framebuffer_attachment_image_infos[2] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[2].width = attachment_width;
    framebuffer_attachment_image_infos[2].height = attachment_height;
    framebuffer_attachment_image_infos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[2].layerCount = 1;
    framebuffer_attachment_image_infos[2].viewFormatCount = 1;
    framebuffer_attachment_image_infos[2].pViewFormats = &depth_stencil_attachment_format;
    // Input attachment
    framebuffer_attachment_image_infos[3] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[3].width = attachment_width;
    framebuffer_attachment_image_infos[3].height = attachment_height;
    framebuffer_attachment_image_infos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[3].layerCount = 1;
    framebuffer_attachment_image_infos[3].viewFormatCount = 1;
    framebuffer_attachment_image_infos[3].pViewFormats = &color_input_attachment_format;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 4;
    framebuffer_attachment_ci.pAttachmentImageInfos = framebuffer_attachment_image_infos;
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachment_ci);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = rp;
    framebuffer_ci.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched usage
    framebuffer_attachment_image_infos[0].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Color resolve attachment, mismatched usage
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Depth stencil attachment, mismatched usage
    framebuffer_attachment_image_infos[2].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03202");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Color attachment, mismatched usage
    framebuffer_attachment_image_infos[3].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03204");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
}

TEST_F(NegativeImagelessFramebuffer, AttachmentMultiviewImageLayerCountMismatch) {
    TEST_DESCRIPTION("Create an imageless framebuffer against a multiview-enabled render pass with mismatched layer counts");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat color_input_attachment_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depth_stencil_attachment_format = VK_FORMAT_D32_SFLOAT_S8_UINT;

    uint32_t view_mask = 0x3u;
    VkRenderPassMultiviewCreateInfo rp_multiview_ci = vku::InitStructHelper();
    rp_multiview_ci.subpassCount = 1;
    rp_multiview_ci.pViewMasks = &view_mask;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(color_input_attachment_format, VK_SAMPLE_COUNT_4_BIT);    // Color attachment
    rp.AddAttachmentDescription(color_input_attachment_format);                           // Color resolve attachment
    rp.AddAttachmentDescription(depth_stencil_attachment_format, VK_SAMPLE_COUNT_4_BIT);  // Depth stencil attachment
    rp.AddAttachmentDescription(color_input_attachment_format);                           // Input attachment
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({2, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({3, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.AddDepthStencilAttachment(2);
    rp.AddInputAttachment(3);
    rp.CreateRenderPass(&rp_multiview_ci);

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_infos[4] = {};
    // Color attachment
    framebuffer_attachment_image_infos[0] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[0].width = attachment_width;
    framebuffer_attachment_image_infos[0].height = attachment_height;
    framebuffer_attachment_image_infos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[0].layerCount = 2;
    framebuffer_attachment_image_infos[0].viewFormatCount = 1;
    framebuffer_attachment_image_infos[0].pViewFormats = &color_input_attachment_format;
    // Color resolve attachment
    framebuffer_attachment_image_infos[1] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[1].width = attachment_width;
    framebuffer_attachment_image_infos[1].height = attachment_height;
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[1].layerCount = 2;
    framebuffer_attachment_image_infos[1].viewFormatCount = 1;
    framebuffer_attachment_image_infos[1].pViewFormats = &color_input_attachment_format;
    // Depth stencil attachment
    framebuffer_attachment_image_infos[2] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[2].width = attachment_width;
    framebuffer_attachment_image_infos[2].height = attachment_height;
    framebuffer_attachment_image_infos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[2].layerCount = 2;
    framebuffer_attachment_image_infos[2].viewFormatCount = 1;
    framebuffer_attachment_image_infos[2].pViewFormats = &depth_stencil_attachment_format;
    // Input attachment
    framebuffer_attachment_image_infos[3] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[3].width = attachment_width;
    framebuffer_attachment_image_infos[3].height = attachment_height;
    framebuffer_attachment_image_infos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[3].layerCount = 2;
    framebuffer_attachment_image_infos[3].viewFormatCount = 1;
    framebuffer_attachment_image_infos[3].pViewFormats = &color_input_attachment_format;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 4;
    framebuffer_attachment_ci.pAttachmentImageInfos = framebuffer_attachment_image_infos;
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachment_ci);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = rp;
    framebuffer_ci.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebuffer_attachment_image_infos[0].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[0].layerCount = 2;

    // Color resolve attachment, mismatched layer count
    framebuffer_attachment_image_infos[1].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[1].layerCount = 2;

    // Depth stencil attachment, mismatched layer count
    framebuffer_attachment_image_infos[2].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[2].layerCount = 2;

    // Input attachment, mismatched layer count
    framebuffer_attachment_image_infos[3].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, DepthStencilResolveAttachment) {
    TEST_DESCRIPTION(
        "Create an imageless framebuffer against a render pass using depth stencil resolve, with mismatched information");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkResolveModeFlagBits depth_resolve_mode = FindSupportedDepthResolveMode();
    if (depth_resolve_mode == VK_RESOLVE_MODE_NONE) {
        GTEST_SKIP() << "Could not find a supported depth resolve mode.";
    }
    const VkResolveModeFlagBits stencil_resolve_mode = FindSupportedStencilResolveMode();
    if (stencil_resolve_mode == VK_RESOLVE_MODE_NONE) {
        GTEST_SKIP() << "Could not find a supported stencil resolve mode.";
    }

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat attachment_format = FindSupportedDepthStencilFormat(Gpu());

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachment_format, VK_SAMPLE_COUNT_4_BIT);  // Depth/stencil
    rp.AddAttachmentDescription(attachment_format, VK_SAMPLE_COUNT_1_BIT);  // Depth/stencil resolve
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddDepthStencilAttachment(0);
    rp.AddDepthStencilResolveAttachment(1, depth_resolve_mode, stencil_resolve_mode);
    rp.SetViewMask(0x3u);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_infos[2] = {};
    // Depth/stencil attachment
    framebuffer_attachment_image_infos[0] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[0].width = attachment_width;
    framebuffer_attachment_image_infos[0].height = attachment_height;
    framebuffer_attachment_image_infos[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[0].layerCount = 2;
    framebuffer_attachment_image_infos[0].viewFormatCount = 1;
    framebuffer_attachment_image_infos[0].pViewFormats = &attachment_format;
    // Depth/stencil resolve attachment
    framebuffer_attachment_image_infos[1] = vku::InitStructHelper();
    framebuffer_attachment_image_infos[1].width = attachment_width;
    framebuffer_attachment_image_infos[1].height = attachment_height;
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebuffer_attachment_image_infos[1].layerCount = 2;
    framebuffer_attachment_image_infos[1].viewFormatCount = 1;
    framebuffer_attachment_image_infos[1].pViewFormats = &attachment_format;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 2;
    framebuffer_attachment_ci.pAttachmentImageInfos = framebuffer_attachment_image_infos;
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachment_ci);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = rp;
    framebuffer_ci.attachmentCount = 2;
    framebuffer_ci.pAttachments = nullptr;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebuffer_attachment_image_infos[0].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[0].layerCount = 2;

    // Depth resolve attachment, mismatched image usage
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03203");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebuffer_attachment_image_infos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Depth resolve attachment, mismatched layer count
    framebuffer_attachment_image_infos[1].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.CreateRenderPass();

    VkFormat view_format = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &view_format;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04549");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.CreateRenderPass();

    VkFormat view_format = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &view_format;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    {
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04543");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    {
        fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04544");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    {
        fbai_info.layerCount = 2;
        fb_info.layers = 3;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04545");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateDimensionsMultiview) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage with imageless FB");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.SetViewMask(0x4);
    rp.CreateRenderPass();

    VkFormat view_format = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 2;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &view_format;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04587");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, RenderPassBeginImageView3D) {
    TEST_DESCRIPTION("Misuse of VK_IMAGE_VIEW_TYPE_3D.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());

    bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat attachment_formats[1] = {VK_FORMAT_R8G8B8A8_UNORM};
    VkFormat framebuffer_attachment_formats[1] = {VK_FORMAT_R8G8B8A8_UNORM};

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachment_formats[0], VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // Create Attachments
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.extent = {attachment_width, attachment_height, 1};
    image_ci.arrayLayers = 1;
    image_ci.mipLevels = 1;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.format = attachment_formats[0];
    vkt::Image image3D(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView imageView3D = image3D.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.flags = 0;
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = attachment_width;
    framebuffer_attachment_image_info.height = attachment_height;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = framebuffer_attachment_formats;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 1;
    framebuffer_attachment_ci.pAttachmentImageInfos = &framebuffer_attachment_image_info;

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.width = attachment_width;
    framebuffer_ci.height = attachment_height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.renderPass = rp;

    // Try to use 3D Image View without imageless flag
    {
        framebuffer_ci.pNext = nullptr;
        framebuffer_ci.flags = 0;
        framebuffer_ci.pAttachments = &imageView3D.handle();
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04113");
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        m_errorMonitor->VerifyFound();
    }

    framebuffer_ci.pNext = &framebuffer_attachment_ci;
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.pAttachments = nullptr;
    vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
    ASSERT_TRUE(framebuffer.initialized());

    VkRenderPassAttachmentBeginInfo rp_attachment_begin_info = vku::InitStructHelper();
    rp_attachment_begin_info.attachmentCount = 1;
    rp_attachment_begin_info.pAttachments = &imageView3D.handle();
    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper(&rp_attachment_begin_info);
    rp_begin_info.renderPass = rp;
    rp_begin_info.renderArea.extent = {attachment_width, attachment_height};
    rp_begin_info.framebuffer = framebuffer;

    // Try to use 3D Image View with imageless flag
    CreateRenderPassBeginTest(m_command_buffer, &rp_begin_info, rp2_supported,
                              "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114",
                              "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114");
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImagePNext) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 256, 256, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView imageView = image.CreateView();

    // random invalid struct for a framebuffer pNext change
    VkCommandPoolCreateInfo invalid_struct = vku::InitStructHelper();

    VkFormat attachment_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFramebufferAttachmentImageInfo fb_fdm = vku::InitStructHelper(&invalid_struct);
    fb_fdm.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    fb_fdm.width = 64;
    fb_fdm.height = 64;
    fb_fdm.layerCount = 1;
    fb_fdm.viewFormatCount = 1;
    fb_fdm.pViewFormats = &attachment_format;

    VkFramebufferAttachmentsCreateInfo fb_aci_fdm = vku::InitStructHelper();
    fb_aci_fdm.attachmentImageInfoCount = 1;
    fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&fb_aci_fdm);
    framebuffer_ci.width = 64;
    framebuffer_ci.height = 64;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = m_renderPass;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &imageView.handle();

    // VkFramebufferCreateInfo -pNext-> VkFramebufferAttachmentsCreateInfo
    //                                             |-> VkFramebufferAttachmentImageInfo -pNext-> INVALID
    {
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferAttachmentImageInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        m_errorMonitor->VerifyFound();
    }

    // VkFramebufferCreateInfo -pNext-> VkFramebufferAttachmentsCreateInfo -pNext-> INVALID
    {
        fb_fdm.pNext = nullptr;
        fb_aci_fdm.pNext = &invalid_struct;
        // Has parent struct name in VUID since child stucture don't have a pNext VU
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        m_errorMonitor->VerifyFound();
    }

    // VkFramebufferCreateInfo -pNext-> INVALID
    {
        fb_aci_fdm.pNext = nullptr;
        framebuffer_ci.pNext = &invalid_struct;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImageFormat) {
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 256, 256, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkFormat attachment_format = VK_FORMAT_UNDEFINED;
    VkFramebufferAttachmentImageInfo fb_fdm = vku::InitStructHelper();
    fb_fdm.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    fb_fdm.width = 64;
    fb_fdm.height = 64;
    fb_fdm.layerCount = 1;
    fb_fdm.viewFormatCount = 1;
    fb_fdm.pViewFormats = &attachment_format;

    VkFramebufferAttachmentsCreateInfo fb_aci_fdm = vku::InitStructHelper();
    fb_aci_fdm.attachmentImageInfoCount = 1;
    fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_aci_fdm);
    fb_ci.width = 64;
    fb_ci.height = 64;
    fb_ci.layers = 1;
    fb_ci.renderPass = m_renderPass;
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &image_view.handle();

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferAttachmentImageInfo-viewFormatCount-09536");
    vkt::Framebuffer framebuffer(*m_device, fb_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, MissingInheritanceRenderingInfo) {
    TEST_DESCRIPTION("Begin cmd buffer with imageless framebuffer and missing VkCommandBufferInheritanceRenderingInfo structure");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo fb_attachment_image_info = vku::InitStructHelper();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fb_attachment_image_info.width = attachment_width;
    fb_attachment_image_info.height = attachment_height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 1;
    fb_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo fb_attachment_ci = vku::InitStructHelper();
    fb_attachment_ci.attachmentImageInfoCount = 1;
    fb_attachment_ci.pAttachmentImageInfos = &fb_attachment_image_info;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_attachment_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_ci.width = attachment_width;
    fb_ci.height = attachment_height;
    fb_ci.layers = 1;
    fb_ci.renderPass = rp;
    fb_ci.attachmentCount = 1;

    fb_ci.pAttachments = nullptr;
    vkt::Framebuffer framebuffer_null(*m_device, fb_ci);

    vkt::ImageView rt_view = m_renderTargets[0]->CreateView();
    VkImageView image_views[2] = {rt_view, CastToHandle<VkImageView, uintptr_t>(0xbaadbeef)};
    fb_ci.pAttachments = image_views;
    vkt::Framebuffer framebuffer_bad_image_view(*m_device, fb_ci);

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();
    inheritance_info.framebuffer = framebuffer_null;

    VkCommandBufferBeginInfo cb_begin_info = vku::InitStructHelper();
    cb_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cb_begin_info.pInheritanceInfo = &inheritance_info;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-06002");
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-09240");
    vk::BeginCommandBuffer(secondary, &cb_begin_info);
    m_errorMonitor->VerifyFound();
}
