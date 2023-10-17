/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(NegativeImagelessFramebuffer, RenderPassBeginImageViewMismatch) {
    TEST_DESCRIPTION(
        "Begin a renderPass where the image views specified do not match the parameters used to create the framebuffer and render "
        "pass.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(imageless_features);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_features));

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    VkFormat framebufferAttachmentFormats[3] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormats[0];
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpassDescription;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachmentDescription;
    vkt::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 2;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = nullptr;
    framebufferCreateInfo.renderPass = rp.handle();

    VkImageFormatListCreateInfoKHR imageFormatListCreateInfo = vku::InitStructHelper();
    imageFormatListCreateInfo.viewFormatCount = 2;
    imageFormatListCreateInfo.pViewFormats = attachmentFormats;
    VkImageCreateInfo imageCreateInfo = vku::InitStructHelper(&imageFormatListCreateInfo);
    imageCreateInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.extent.width = attachmentWidth;
    imageCreateInfo.extent.height = attachmentHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.mipLevels = 10;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.format = attachmentFormats[0];

    VkImageObj imageObject(m_device);
    imageObject.init(&imageCreateInfo);
    VkImage image = imageObject.image();

    // Only use the subset without the TRANSFER bit
    VkImageViewUsageCreateInfo image_view_usage_create_info = vku::InitStructHelper();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo imageViewCreateInfo = vku::InitStructHelper(&image_view_usage_create_info);
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = attachmentFormats[0];
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Has subset of usage flags
    vkt::ImageView imageViewSubset(*m_device, imageViewCreateInfo);
    ASSERT_TRUE(imageViewSubset.initialized());

    imageViewCreateInfo.pNext = nullptr;
    vkt::ImageView imageView(*m_device, imageViewCreateInfo);
    ASSERT_TRUE(imageView.initialized());

    VkImageView image_views[2] = {imageView.handle(), imageView.handle()};
    VkRenderPassAttachmentBeginInfoKHR renderPassAttachmentBeginInfo = vku::InitStructHelper();
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = image_views;
    VkRenderPassBeginInfo renderPassBeginInfo = vku::InitStructHelper(&renderPassAttachmentBeginInfo);
    renderPassBeginInfo.renderPass = rp.handle();
    renderPassBeginInfo.renderArea.extent.width = attachmentWidth;
    renderPassBeginInfo.renderArea.extent.height = attachmentHeight;

    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    // Positive test first
    {
        framebufferCreateInfo.pAttachments = nullptr;
        framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        m_commandBuffer->begin(&cmd_begin_info);
        m_commandBuffer->BeginRenderPass(renderPassBeginInfo);
        m_commandBuffer->reset();
    }

    // Imageless framebuffer creation bit not present
    {
        framebufferCreateInfo.pAttachments = &imageView.handle();
        framebufferCreateInfo.flags = 0;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03207", "VUID-VkRenderPassBeginInfo-framebuffer-03207");
    }
    {
        framebufferCreateInfo.pAttachments = nullptr;
        framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassAttachmentBeginInfo.attachmentCount = 2;
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03208", "VUID-VkRenderPassBeginInfo-framebuffer-03208");
        renderPassAttachmentBeginInfo.attachmentCount = 1;
    }

    // Mismatched number of attachments
    {
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassAttachmentBeginInfo.attachmentCount = 2;
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03208", "VUID-VkRenderPassBeginInfo-framebuffer-03208");
        renderPassAttachmentBeginInfo.attachmentCount = 1;
    }

    // Mismatched flags
    {
        framebufferAttachmentImageInfo.flags = 0;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03209", "VUID-VkRenderPassBeginInfo-framebuffer-03209");
        framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    // Mismatched usage
    {
        framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-04627", "VUID-VkRenderPassBeginInfo-framebuffer-04627");
        framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Mismatched usage because VkImageViewUsageCreateInfo restricted to TRANSFER
    {
        renderPassAttachmentBeginInfo.pAttachments = &imageViewSubset.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-04627", "VUID-VkRenderPassBeginInfo-framebuffer-04627");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
    }

    // Mismatched width
    {
        framebufferAttachmentImageInfo.width += 1;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03211", "VUID-VkRenderPassBeginInfo-framebuffer-03211");
        framebufferAttachmentImageInfo.width -= 1;
    }

    // Mismatched height
    {
        framebufferAttachmentImageInfo.height += 1;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03212", "VUID-VkRenderPassBeginInfo-framebuffer-03212");
        framebufferAttachmentImageInfo.height -= 1;
    }

    // Mismatched layer count
    {
        framebufferAttachmentImageInfo.layerCount += 1;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03213", "VUID-VkRenderPassBeginInfo-framebuffer-03213");
        framebufferAttachmentImageInfo.layerCount -= 1;
    }

    // Mismatched view format count
    {
        framebufferAttachmentImageInfo.viewFormatCount = 3;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03214", "VUID-VkRenderPassBeginInfo-framebuffer-03214");
        framebufferAttachmentImageInfo.viewFormatCount = 2;
    }

    // Mismatched format lists
    {
        framebufferAttachmentFormats[1] = VK_FORMAT_B8G8R8A8_SRGB;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03215", "VUID-VkRenderPassBeginInfo-framebuffer-03215");
        framebufferAttachmentFormats[1] = VK_FORMAT_B8G8R8A8_UNORM;
    }

    // Mismatched formats
    {
        imageViewCreateInfo.format = attachmentFormats[1];
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        ASSERT_TRUE(imageView2.initialized());
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03216", "VUID-VkRenderPassBeginInfo-framebuffer-03216");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.format = attachmentFormats[0];
    }

    // Mismatched sample counts
    {
        imageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
        imageCreateInfo.mipLevels = 1;
        VkImageObj imageObject2(m_device);
        imageObject2.init(&imageCreateInfo);
        imageViewCreateInfo.image = imageObject2.image();
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        ASSERT_TRUE(imageView2.initialized());
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-09047", "VUID-VkRenderPassBeginInfo-framebuffer-09047");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.image = imageObject.image();
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.mipLevels = 10;
    }

    // Mismatched level counts
    {
        imageViewCreateInfo.subresourceRange.levelCount = 2;
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        ASSERT_TRUE(imageView2.initialized());
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218",
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.subresourceRange.levelCount = 1;
    }

    // Non-identity component swizzle
    {
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_A;
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        ASSERT_TRUE(imageView2.initialized());
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219",
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    }

    {
        imageViewCreateInfo.subresourceRange.baseMipLevel = 1;
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        ASSERT_TRUE(imageView2.initialized());
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        framebufferAttachmentImageInfo.height = framebufferAttachmentImageInfo.height / 2;
        framebufferAttachmentImageInfo.width = framebufferAttachmentImageInfo.width / 2;
        framebufferCreateInfo.height = framebufferCreateInfo.height / 2;
        framebufferCreateInfo.width = framebufferCreateInfo.width / 2;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        ASSERT_TRUE(framebuffer.initialized());
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        renderPassBeginInfo.renderArea.extent.height = renderPassBeginInfo.renderArea.extent.height / 2;
        renderPassBeginInfo.renderArea.extent.width = renderPassBeginInfo.renderArea.extent.width / 2;
        m_commandBuffer->begin(&cmd_begin_info);
        m_commandBuffer->BeginRenderPass(renderPassBeginInfo);
        m_commandBuffer->reset();
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        framebufferAttachmentImageInfo.height = framebufferAttachmentImageInfo.height * 2;
        framebufferAttachmentImageInfo.width = framebufferAttachmentImageInfo.width * 2;
    }
}

TEST_F(NegativeImagelessFramebuffer, FeatureEnable) {
    TEST_DESCRIPTION("Use imageless framebuffer functionality without enabling the feature");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    vkt::RenderPass render_pass(*m_device, renderPassCreateInfo);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 1;

    // Imageless framebuffer creation bit not present
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03189");
    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, BasicUsage) {
    TEST_DESCRIPTION("Create an imageless framebuffer in various invalid ways");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeaturesKHR mv_features = vku::InitStructHelper();
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_features = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        imageless_features.pNext = &mv_features;
    }
    GetPhysicalDeviceFeatures2(imageless_features);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_features));
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    vkt::RenderPass render_pass(*m_device, renderPassCreateInfo);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 1;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Attachments info not present
    framebufferCreateInfo.pNext = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03190");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;

    // Mismatched attachment counts
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[2] = {framebufferAttachmentImageInfo,
                                                                              framebufferAttachmentImageInfo};
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03191");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;

    // Mismatched format list
    attachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched format list
    attachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched layer count, multiview disabled
    framebufferCreateInfo.layers = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-04546");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.layers = 1;

    // Mismatched width
    framebufferCreateInfo.width += 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04541");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.width -= 1;

    // Mismatched height
    framebufferCreateInfo.height += 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04542");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.height -= 1;
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImageUsageMismatch) {
    TEST_DESCRIPTION("Create an imageless framebuffer with mismatched attachment image usage");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(imageless_features);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_features));

    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat colorAndInputAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthStencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    VkAttachmentDescription attachmentDescriptions[4] = {};
    // Color attachment
    attachmentDescriptions[0].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Color resolve attachment
    attachmentDescriptions[1].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth stencil attachment
    attachmentDescriptions[2].format = depthStencilAttachmentFormat;
    attachmentDescriptions[2].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Input attachment
    attachmentDescriptions[3].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[3].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;
    VkAttachmentReference colorResolveAttachmentReference = {};
    colorResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorResolveAttachmentReference.attachment = 1;
    VkAttachmentReference depthStencilAttachmentReference = {};
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 2;
    VkAttachmentReference inputAttachmentReference = {};
    inputAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    inputAttachmentReference.attachment = 3;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;
    subpassDescription.pResolveAttachments = &colorResolveAttachmentReference;
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
    subpassDescription.inputAttachmentCount = 1;
    subpassDescription.pInputAttachments = &inputAttachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper();
    renderPassCreateInfo.attachmentCount = 4;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    vkt::RenderPass render_pass(*m_device, renderPassCreateInfo);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 1;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 1;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 1;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 1;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched usage
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Color resolve attachment, mismatched usage
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Depth stencil attachment, mismatched usage
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03202");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Color attachment, mismatched usage
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03204");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
}

TEST_F(NegativeImagelessFramebuffer, AttachmentMultiviewImageLayerCountMismatch) {
    TEST_DESCRIPTION("Create an imageless framebuffer against a multiview-enabled render pass with mismatched layer counts");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeaturesKHR mv_features = vku::InitStructHelper();
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_features = vku::InitStructHelper(&mv_features);
    GetPhysicalDeviceFeatures2(imageless_features);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_features));

    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat colorAndInputAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthStencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    VkAttachmentDescription attachmentDescriptions[4] = {};
    // Color attachment
    attachmentDescriptions[0].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Color resolve attachment
    attachmentDescriptions[1].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth stencil attachment
    attachmentDescriptions[2].format = depthStencilAttachmentFormat;
    attachmentDescriptions[2].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Input attachment
    attachmentDescriptions[3].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[3].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;
    VkAttachmentReference colorResolveAttachmentReference = {};
    colorResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorResolveAttachmentReference.attachment = 1;
    VkAttachmentReference depthStencilAttachmentReference = {};
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 2;
    VkAttachmentReference inputAttachmentReference = {};
    inputAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    inputAttachmentReference.attachment = 3;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;
    subpassDescription.pResolveAttachments = &colorResolveAttachmentReference;
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
    subpassDescription.inputAttachmentCount = 1;
    subpassDescription.pInputAttachments = &inputAttachmentReference;

    uint32_t viewMask = 0x3u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;
    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper(&renderPassMultiviewCreateInfo);
    renderPassCreateInfo.attachmentCount = 4;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    vkt::RenderPass render_pass(*m_device, renderPassCreateInfo);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 2;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 2;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebufferAttachmentImageInfos[0].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[0].layerCount = 2;

    // Color resolve attachment, mismatched layer count
    framebufferAttachmentImageInfos[1].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[1].layerCount = 2;

    // Depth stencil attachment, mismatched layer count
    framebufferAttachmentImageInfos[2].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[2].layerCount = 2;

    // Input attachment, mismatched layer count
    framebufferAttachmentImageInfos[3].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, DepthStencilResolveAttachment) {
    TEST_DESCRIPTION(
        "Create an imageless framebuffer against a render pass using depth stencil resolve, with mismatched information");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeaturesKHR mv_features = vku::InitStructHelper();
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_features = vku::InitStructHelper(&mv_features);
    GetPhysicalDeviceFeatures2(imageless_features);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_features));

    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = FindSupportedDepthStencilFormat(gpu());

    VkAttachmentDescription2KHR attachmentDescriptions[2] = {};
    // Depth/stencil attachment
    attachmentDescriptions[0] = vku::InitStructHelper();
    attachmentDescriptions[0].format = attachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1] = vku::InitStructHelper();
    attachmentDescriptions[1].format = attachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference2KHR depthStencilAttachmentReference = vku::InitStructHelper();
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 0;
    VkAttachmentReference2KHR depthStencilResolveAttachmentReference = vku::InitStructHelper();
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 1;
    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve =
        vku::InitStructHelper();
    subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    subpassDescriptionDepthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    VkSubpassDescription2KHR subpassDescription = vku::InitStructHelper(&subpassDescriptionDepthStencilResolve);
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
    subpassDescription.viewMask = 0x3u;

    VkRenderPassCreateInfo2KHR renderPassCreateInfo = vku::InitStructHelper();
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    vkt::RenderPass render_pass(*m_device, renderPassCreateInfo, true);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[2] = {};
    // Depth/stencil attachment
    framebufferAttachmentImageInfos[0] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &attachmentFormat;
    // Depth/stencil resolve attachment
    framebufferAttachmentImageInfos[1] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = nullptr;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebufferAttachmentImageInfos[0].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[0].layerCount = 2;

    // Depth resolve attachment, mismatched image usage
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03203");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Depth resolve attachment, mismatched layer count
    framebufferAttachmentImageInfos[1].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkPhysicalDeviceImagelessFramebufferFeatures if_features = vku::InitStructHelper();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper(&if_features);
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "requires attachmentFragmentShadingRate feature";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentReference2 attach = vku::InitStructHelper();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = vku::InitStructHelper();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = vku::InitStructHelper(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04549");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkPhysicalDeviceMultiviewFeaturesKHR mv_features = vku::InitStructHelper();
    VkPhysicalDeviceImagelessFramebufferFeatures if_features = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        if_features.pNext = &mv_features;
    }
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper(&if_features);
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "attachmentFragmentShadingRate feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentReference2 attach = vku::InitStructHelper();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = vku::InitStructHelper();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = vku::InitStructHelper(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass rp(*m_device, rpci, true);

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04543");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    {
        fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04544");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    {
        fbai_info.layerCount = 2;
        fb_info.layers = 3;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04545");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.layers = 1;
    fbai_info.layerCount = 1;

    if (fsr_properties.layeredShadingRateAttachments && mv_features.multiview) {
        subpass.viewMask = 0x4;
        vkt::RenderPass rp2(*m_device, rpci, true);
        ASSERT_TRUE(rp2.initialized());
        subpass.viewMask = 0;

        fbai_info.layerCount = 2;
        fb_info.renderPass = rp2.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-08921");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImagelessFramebuffer, RenderPassBeginImageView3D) {
    TEST_DESCRIPTION("Misuse of VK_IMAGE_VIEW_TYPE_3D.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR imageless_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(imageless_features);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_features));
    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormats[1] = {VK_FORMAT_R8G8B8A8_UNORM};
    VkFormat framebufferAttachmentFormats[1] = {VK_FORMAT_R8G8B8A8_UNORM};

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormats[0];
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    vkt::RenderPass renderPass(*m_device, renderPassCreateInfo);
    ASSERT_TRUE(renderPass.initialized());

    // Create Attachments
    VkImageCreateInfo imageCreateInfo = vku::InitStructHelper();
    imageCreateInfo.flags = 0;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageCreateInfo.extent.width = attachmentWidth;
    imageCreateInfo.extent.height = attachmentHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.format = attachmentFormats[0];

    VkImageObj image3D(m_device);
    image3D.init(&imageCreateInfo);

    VkImageViewCreateInfo imageViewCreateInfo = vku::InitStructHelper();
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    imageViewCreateInfo.format = attachmentFormats[0];
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    VkImageView imageView3D = image3D.targetView(imageViewCreateInfo);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.flags = 0;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper();
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.renderPass = renderPass.handle();

    // Try to use 3D Image View without imageless flag
    {
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.pAttachments = &imageView3D;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04113");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }

    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.pAttachments = nullptr;
    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
    ASSERT_TRUE(framebuffer.initialized());

    VkRenderPassAttachmentBeginInfoKHR renderPassAttachmentBeginInfo = vku::InitStructHelper();
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = &imageView3D;
    VkRenderPassBeginInfo renderPassBeginInfo = vku::InitStructHelper(&renderPassAttachmentBeginInfo);
    renderPassBeginInfo.renderPass = renderPass.handle();
    renderPassBeginInfo.renderArea.extent.width = attachmentWidth;
    renderPassBeginInfo.renderArea.extent.height = attachmentHeight;
    renderPassBeginInfo.framebuffer = framebuffer.handle();

    // Try to use 3D Image View with imageless flag
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114",
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114");
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImagePNext) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // random invalid struct for a framebuffer pNext change
    VkCommandPoolCreateInfo invalid_struct = vku::InitStructHelper();

    VkFormat attachment_format = VK_FORMAT_R8G8B8A8_UNORM;
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

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&fb_aci_fdm);
    framebufferCreateInfo.width = 64;
    framebufferCreateInfo.height = 64;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(m_framebuffer_attachments.size());
    framebufferCreateInfo.pAttachments = m_framebuffer_attachments.data();

    // VkFramebufferCreateInfo -pNext-> VkFramebufferAttachmentsCreateInfo
    //                                             |-> VkFramebufferAttachmentImageInfo -pNext-> INVALID
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferAttachmentImageInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }

    // VkFramebufferCreateInfo -pNext-> VkFramebufferAttachmentsCreateInfo -pNext-> INVALID
    {
        fb_fdm.pNext = nullptr;
        fb_aci_fdm.pNext = &invalid_struct;
        // Has parent struct name in VUID since child stucture don't have a pNext VU
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }

    // VkFramebufferCreateInfo -pNext-> INVALID
    {
        fb_aci_fdm.pNext = nullptr;
        framebufferCreateInfo.pNext = &invalid_struct;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }
}
