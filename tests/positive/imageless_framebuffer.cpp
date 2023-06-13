/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(PositiveImagelessFramebuffer, BasicUsage) {
    TEST_DESCRIPTION("Create an imageless framebuffer");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto imageless_features = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    GetPhysicalDeviceFeatures2(imageless_features);
    if (!imageless_features.imagelessFramebuffer) {
        GTEST_SKIP() << "imagelessFramebuffer not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &imageless_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachment_description = {};
    attachment_description.format = format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachment_reference = {};
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;
    auto rp_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_description;
    vk_testing::RenderPass render_pass(*m_device, rp_ci);

    auto fb_attachment_image_info = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fb_attachment_image_info.width = attachment_width;
    fb_attachment_image_info.height = attachment_height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 1;
    fb_attachment_image_info.pViewFormats = &format;
    auto fb_attachment_ci = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    fb_attachment_ci.attachmentImageInfoCount = 1;
    fb_attachment_ci.pAttachmentImageInfos = &fb_attachment_image_info;
    auto fb_ci = LvlInitStruct<VkFramebufferCreateInfo>(&fb_attachment_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    fb_ci.width = attachment_width;
    fb_ci.height = attachment_height;
    fb_ci.layers = 1;
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 1;

    fb_ci.pAttachments  = nullptr;
    vk_testing::Framebuffer framebuffer_null(*m_device, fb_ci);

    VkImageView image_views[2] = {
    m_renderTargets[0]->targetView(VK_FORMAT_B8G8R8A8_UNORM),
    CastToHandle<VkImageView, uintptr_t>(0xbaadbeef)
    };
    fb_ci.pAttachments = image_views;
    vk_testing::Framebuffer framebuffer_bad_image_view(*m_device, fb_ci);
}
