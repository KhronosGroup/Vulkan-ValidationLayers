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

TEST_F(PositiveFragmentShadingRate, StageInVariousAPIs) {
    TEST_DESCRIPTION("Specify shading rate pipeline stage with attachmentFragmentShadingRate feature enabled");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto shading_rate_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    GetPhysicalDeviceFeatures2(shading_rate_features);
    if (shading_rate_features.attachmentFragmentShadingRate == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) attachmentFragmentShadingRate";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>(&shading_rate_features);
    sync2_features.synchronization2 = VK_TRUE;  // sync2 extension guarantees feature support
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sync2_features));

    auto query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    const vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);
    const vk_testing::Event event(*m_device, LvlInitStruct<VkEventCreateInfo>());
    const vk_testing::Event event2(*m_device, LvlInitStruct<VkEventCreateInfo>());

    m_commandBuffer->begin();
    // Different API calls to cover three category of VUIDs: 07316, 07318, 07314
    vk::CmdResetEvent2KHR(*m_commandBuffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vk::CmdSetEvent(*m_commandBuffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vk::CmdWriteTimestamp(*m_commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_commandBuffer->end();
}

TEST_F(PositiveFragmentShadingRate, StageWithPipelineBarrier) {
    TEST_DESCRIPTION("Test pipeline barrier with VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR stage");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    GetPhysicalDeviceFeatures2(fsr_features);
    if (fsr_features.attachmentFragmentShadingRate == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) attachmentFragmentShadingRate";
    }
    fsr_features.pipelineFragmentShadingRate = VK_FALSE;
    fsr_features.primitiveFragmentShadingRate = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &fsr_features));

    VkImageFormatProperties format_props = {};
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties(
        m_device->phy().handle(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0, &format_props);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Image options not supported";
    }

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    auto imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &imageMemoryBarrier);
    m_commandBuffer->end();
}

TEST_F(PositiveFragmentShadingRate, Attachments) {
    TEST_DESCRIPTION("Create framebuffer with a fragment shading rate attachment that has layout count 1.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>(&fsr_features);
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
        return;
    }
    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2 attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);
    subpass.viewMask = 0x2;

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, VK_IMAGE_TILING_OPTIMAL,
                       0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8_UINT);

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    vk_testing::Framebuffer fb(*m_device, fb_info);
    ASSERT_TRUE(fb.initialized());
}
