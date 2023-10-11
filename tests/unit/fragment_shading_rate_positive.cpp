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
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR shading_rate_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(shading_rate_features);
    if (shading_rate_features.attachmentFragmentShadingRate == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) attachmentFragmentShadingRate";
    }
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper(&shading_rate_features);
    sync2_features.synchronization2 = VK_TRUE;  // sync2 extension guarantees feature support
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    const vkt::QueryPool query_pool(*m_device, query_pool_create_info);
    const vkt::Event event(*m_device);
    const vkt::Event event2(*m_device);

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
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(fsr_features);
    if (fsr_features.attachmentFragmentShadingRate == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) attachmentFragmentShadingRate";
    }
    fsr_features.pipelineFragmentShadingRate = VK_FALSE;
    fsr_features.primitiveFragmentShadingRate = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &fsr_features));

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

    VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper(&fsr_features);
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
        return;
    }
    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentReference2 attach = vku::InitStructHelper();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = vku::InitStructHelper();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    VkSubpassDescription2 subpass = vku::InitStructHelper(&fsr_attachment);
    subpass.viewMask = 0x2;

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

    VkImageObj image(m_device);
    image.InitNoLayout(1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, VK_IMAGE_TILING_OPTIMAL,
                       0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8_UINT);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    vkt::Framebuffer fb(*m_device, fb_info);
    ASSERT_TRUE(fb.initialized());
}
