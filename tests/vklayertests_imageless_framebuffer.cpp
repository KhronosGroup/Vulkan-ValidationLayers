/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, ImagelessFramebufferRenderPassBeginImageViewMismatchTests) {
    TEST_DESCRIPTION(
        "Begin a renderPass where the image views specified do not match the parameters used to create the framebuffer and render "
        "pass.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s test requires VK_KHR_imageless_framebuffer, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        LvlInitStruct<VkPhysicalDeviceFeatures2>(&physicalDeviceImagelessFramebufferFeatures);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &physicalDeviceFeatures2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    VkFormat framebufferAttachmentFormats[3] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormats[0];
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 2;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    VkFramebuffer framebuffer;

    VkImageFormatListCreateInfoKHR imageFormatListCreateInfo = LvlInitStruct<VkImageFormatListCreateInfo>();
    imageFormatListCreateInfo.viewFormatCount = 2;
    imageFormatListCreateInfo.pViewFormats = attachmentFormats;
    VkImageCreateInfo imageCreateInfo = LvlInitStruct<VkImageCreateInfo>(&imageFormatListCreateInfo);
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
    VkImageViewUsageCreateInfo image_view_usage_create_info = LvlInitStruct<VkImageViewUsageCreateInfo>();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo imageViewCreateInfo = LvlInitStruct<VkImageViewCreateInfo>();
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = attachmentFormats[0];
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Has subset of usage flags
    VkImageView imageViewSubset;
    imageViewCreateInfo.pNext = &image_view_usage_create_info;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageViewSubset);
    imageViewCreateInfo.pNext = nullptr;

    VkImageView imageView;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView);

    VkRenderPassAttachmentBeginInfoKHR renderPassAttachmentBeginInfo = LvlInitStruct<VkRenderPassAttachmentBeginInfo>();
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    VkRenderPassBeginInfo renderPassBeginInfo = LvlInitStruct<VkRenderPassBeginInfo>(&renderPassAttachmentBeginInfo);
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.extent.width = attachmentWidth;
    renderPassBeginInfo.renderArea.extent.height = attachmentHeight;

    // Positive test first
    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
    framebufferCreateInfo.pAttachments = nullptr;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_begin_info);
    m_errorMonitor->ExpectSuccess();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyNotFound();
    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);

    // Imageless framebuffer creation bit not present
    framebufferCreateInfo.pAttachments = &imageView;
    framebufferCreateInfo.flags = 0;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03207", "VUID-VkRenderPassBeginInfo-framebuffer-03207");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferCreateInfo.pAttachments = nullptr;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;

    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassAttachmentBeginInfo.attachmentCount = 2;
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03208", "VUID-VkRenderPassBeginInfo-framebuffer-03208");
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);

    // Mismatched number of attachments
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassAttachmentBeginInfo.attachmentCount = 2;
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03208", "VUID-VkRenderPassBeginInfo-framebuffer-03208");
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);

    // Mismatched flags
    framebufferAttachmentImageInfo.flags = 0;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03209", "VUID-VkRenderPassBeginInfo-framebuffer-03209");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    // Mismatched usage
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-04627", "VUID-VkRenderPassBeginInfo-framebuffer-04627");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // Mismatched usage because VkImageViewUsageCreateInfo restricted to TRANSFER
    renderPassAttachmentBeginInfo.pAttachments = &imageViewSubset;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-04627", "VUID-VkRenderPassBeginInfo-framebuffer-04627");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    renderPassAttachmentBeginInfo.pAttachments = &imageView;

    // Mismatched width
    framebufferAttachmentImageInfo.width += 1;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03211", "VUID-VkRenderPassBeginInfo-framebuffer-03211");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.width -= 1;

    // Mismatched height
    framebufferAttachmentImageInfo.height += 1;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03212", "VUID-VkRenderPassBeginInfo-framebuffer-03212");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.height -= 1;

    // Mismatched layer count
    framebufferAttachmentImageInfo.layerCount += 1;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03213", "VUID-VkRenderPassBeginInfo-framebuffer-03213");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.layerCount -= 1;

    // Mismatched view format count
    framebufferAttachmentImageInfo.viewFormatCount = 3;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03214", "VUID-VkRenderPassBeginInfo-framebuffer-03214");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.viewFormatCount = 2;

    // Mismatched format lists
    framebufferAttachmentFormats[1] = VK_FORMAT_B8G8R8A8_SRGB;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03215", "VUID-VkRenderPassBeginInfo-framebuffer-03215");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentFormats[1] = VK_FORMAT_B8G8R8A8_UNORM;

    // Mismatched formats
    VkImageView imageView2;
    imageViewCreateInfo.format = attachmentFormats[1];
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView2);
    renderPassAttachmentBeginInfo.pAttachments = &imageView2;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03216", "VUID-VkRenderPassBeginInfo-framebuffer-03216");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView2, nullptr);
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    imageViewCreateInfo.format = attachmentFormats[0];

    // Mismatched sample counts
    imageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
    imageCreateInfo.mipLevels = 1;
    VkImageObj imageObject2(m_device);
    imageObject2.init(&imageCreateInfo);
    imageViewCreateInfo.image = imageObject2.image();
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView2);
    renderPassAttachmentBeginInfo.pAttachments = &imageView2;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-framebuffer-03217", "VUID-VkRenderPassBeginInfo-framebuffer-03217");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView2, nullptr);
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    imageViewCreateInfo.image = imageObject.image();
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.mipLevels = 10;

    // Mismatched level counts
    imageViewCreateInfo.subresourceRange.levelCount = 2;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView2);
    renderPassAttachmentBeginInfo.pAttachments = &imageView2;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218",
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView2, nullptr);
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    imageViewCreateInfo.subresourceRange.levelCount = 1;

    // Non-identity component swizzle
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_A;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView2);
    renderPassAttachmentBeginInfo.pAttachments = &imageView2;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219",
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView2, nullptr);
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;

    imageViewCreateInfo.subresourceRange.baseMipLevel = 1;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView2);
    renderPassAttachmentBeginInfo.pAttachments = &imageView2;
    framebufferAttachmentImageInfo.height = framebufferAttachmentImageInfo.height / 2;
    framebufferAttachmentImageInfo.width = framebufferAttachmentImageInfo.width / 2;
    framebufferCreateInfo.height = framebufferCreateInfo.height / 2;
    framebufferCreateInfo.width = framebufferCreateInfo.width / 2;
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer;
    renderPassBeginInfo.renderArea.extent.height = renderPassBeginInfo.renderArea.extent.height / 2;
    renderPassBeginInfo.renderArea.extent.width = renderPassBeginInfo.renderArea.extent.width / 2;
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_begin_info);
    m_errorMonitor->ExpectSuccess();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyNotFound();
    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView2, nullptr);
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    framebufferAttachmentImageInfo.height = framebufferAttachmentImageInfo.height * 2;
    framebufferAttachmentImageInfo.width = framebufferAttachmentImageInfo.width * 2;

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
    vk::DestroyImageView(m_device->device(), imageView, nullptr);
    vk::DestroyImageView(m_device->device(), imageViewSubset, nullptr);
}

TEST_F(VkLayerTest, ImagelessFramebufferFeatureEnableTest) {
    TEST_DESCRIPTION("Use imageless framebuffer functionality without enabling the feature");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Imageless framebuffer creation bit not present
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03189");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
}

TEST_F(VkLayerTest, ImagelessFramebufferCreationTests) {
    TEST_DESCRIPTION("Create an imageless framebuffer in various invalid ways");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    bool multiviewSupported = rp2Supported;
    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
            multiviewSupported = true;
        }
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        LvlInitStruct<VkPhysicalDeviceFeatures2>(&physicalDeviceImagelessFramebufferFeatures);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &physicalDeviceFeatures2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Attachments info not present
    framebufferCreateInfo.pNext = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03190");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;

    // Mismatched attachment counts
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[2] = {framebufferAttachmentImageInfo,
                                                                              framebufferAttachmentImageInfo};
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03191");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;

    // Mismatched format list
    attachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched format list
    attachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched layer count, multiview disabled
    framebufferCreateInfo.layers = 2;
    const char* mismatchedLayersNoMultiviewVuid =
        multiviewSupported ? "VUID-VkFramebufferCreateInfo-renderPass-04546" : "VUID-VkFramebufferCreateInfo-flags-04547";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, mismatchedLayersNoMultiviewVuid);
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.layers = 1;

    // Mismatched width
    framebufferCreateInfo.width += 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04541");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.width -= 1;

    // Mismatched height
    framebufferCreateInfo.height += 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04542");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.height -= 1;

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
}

TEST_F(VkLayerTest, ImagelessFramebufferAttachmentImageUsageMismatchTests) {
    TEST_DESCRIPTION("Create an imageless framebuffer with mismatched attachment image usage");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        LvlInitStruct<VkPhysicalDeviceFeatures2>(&physicalDeviceImagelessFramebufferFeatures);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &physicalDeviceFeatures2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat colorAndInputAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthStencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    VkAttachmentDescription attachmentDescriptions[4] = {};
    // Color attachment
    attachmentDescriptions[0].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Color resolve attachment
    attachmentDescriptions[1].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth stencil attachment
    attachmentDescriptions[2].format = depthStencilAttachmentFormat;
    attachmentDescriptions[2].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Input attachment
    attachmentDescriptions[3].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
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

    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>();
    renderPassCreateInfo.attachmentCount = 4;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 1;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 1;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 1;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 1;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched usage
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Color resolve attachment, mismatched usage
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Depth stencil attachment, mismatched usage
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03202");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Color attachment, mismatched usage
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03204");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
}

TEST_F(VkLayerTest, ImagelessFramebufferAttachmentMultiviewImageLayerCountMismatchTests) {
    TEST_DESCRIPTION("Create an imageless framebuffer against a multiview-enabled render pass with mismatched layer counts");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        LvlInitStruct<VkPhysicalDeviceFeatures2>(&physicalDeviceImagelessFramebufferFeatures);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &physicalDeviceFeatures2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat colorAndInputAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthStencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    VkAttachmentDescription attachmentDescriptions[4] = {};
    // Color attachment
    attachmentDescriptions[0].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Color resolve attachment
    attachmentDescriptions[1].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth stencil attachment
    attachmentDescriptions[2].format = depthStencilAttachmentFormat;
    attachmentDescriptions[2].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Input attachment
    attachmentDescriptions[3].format = colorAndInputAttachmentFormat;
    attachmentDescriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
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
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;
    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>(&renderPassMultiviewCreateInfo);
    renderPassCreateInfo.attachmentCount = 4;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 2;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 2;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebufferAttachmentImageInfos[0].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[0].layerCount = 2;

    // Color resolve attachment, mismatched layer count
    framebufferAttachmentImageInfos[1].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[1].layerCount = 2;

    // Depth stencil attachment, mismatched layer count
    framebufferAttachmentImageInfos[2].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[2].layerCount = 2;

    // Input attachment, mismatched layer count
    framebufferAttachmentImageInfos[3].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[3].layerCount = 2;

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
}

TEST_F(VkLayerTest, ImagelessFramebufferDepthStencilResolveAttachmentTests) {
    TEST_DESCRIPTION(
        "Create an imageless framebuffer against a render pass using depth stencil resolve, with mismatched information");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    if (!rp2Supported) {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        LvlInitStruct<VkPhysicalDeviceFeatures2>(&physicalDeviceImagelessFramebufferFeatures);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &physicalDeviceFeatures2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = FindSupportedDepthStencilFormat(gpu());
    if (attachmentFormat == VK_FORMAT_UNDEFINED) {
        printf("%s Did not find a supported depth stencil format; skipped.\n", kSkipPrefix);
        return;
    }

    VkAttachmentDescription2KHR attachmentDescriptions[2] = {};
    // Depth/stencil attachment
    attachmentDescriptions[0] = LvlInitStruct<VkAttachmentDescription2>();
    attachmentDescriptions[0].format = attachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1] = LvlInitStruct<VkAttachmentDescription2>();
    attachmentDescriptions[1].format = attachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference2KHR depthStencilAttachmentReference = LvlInitStruct<VkAttachmentReference2>();
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 0;
    VkAttachmentReference2KHR depthStencilResolveAttachmentReference = LvlInitStruct<VkAttachmentReference2>();
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 1;
    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve =
        LvlInitStruct<VkSubpassDescriptionDepthStencilResolveKHR>();
    subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    subpassDescriptionDepthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    VkSubpassDescription2KHR subpassDescription = LvlInitStruct<VkSubpassDescription2>(&subpassDescriptionDepthStencilResolve);
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
    subpassDescription.viewMask = 0x3u;

    VkRenderPassCreateInfo2KHR renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo2>();
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    VkRenderPass renderPass;
    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[2] = {};
    // Depth/stencil attachment
    framebufferAttachmentImageInfos[0] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &attachmentFormat;
    // Depth/stencil resolve attachment
    framebufferAttachmentImageInfos[1] = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = nullptr;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebufferAttachmentImageInfos[0].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[0].layerCount = 2;

    // Depth resolve attachment, mismatched image usage
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03203");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Depth resolve attachment, mismatched layer count
    framebufferAttachmentImageInfos[1].layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferAttachmentImageInfos[1].layerCount = 2;

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateImagelessFramebufferUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceImagelessFramebufferFeatures if_features = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeatures>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(&if_features);
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        printf("%s requires attachmentFragmentShadingRate feature.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2 attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = {};
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass rp;

    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");
    VkResult err = vkCreateRenderPass2KHR(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = LvlInitStruct<VkFramebufferAttachmentImageInfo>();
    fbai_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = LvlInitStruct<VkFramebufferAttachmentsCreateInfo>();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = nullptr;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    VkFramebuffer fb;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04549");
    err = vk::CreateFramebuffer(device(), &fb_info, nullptr, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    }
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateImagelessFramebufferDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceImagelessFramebufferFeatures if_features = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeatures>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(&if_features);
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        printf("%s requires attachmentFragmentShadingRate feature.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2 attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = {};
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass rp;

    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");
    VkResult err = vkCreateRenderPass2KHR(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = LvlInitStruct<VkFramebufferAttachmentImageInfo>();
    fbai_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = LvlInitStruct<VkFramebufferAttachmentsCreateInfo>();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = nullptr;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    VkFramebuffer fb;

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04543");
    err = vk::CreateFramebuffer(device(), &fb_info, nullptr, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    }
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04544");
    err = vk::CreateFramebuffer(device(), &fb_info, nullptr, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    }
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    fbai_info.layerCount = 2;
    fb_info.layers = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04545");
    err = vk::CreateFramebuffer(device(), &fb_info, nullptr, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    }
    fb_info.layers = 1;
    fbai_info.layerCount = 1;

    vk::DestroyRenderPass(m_device->device(), rp, nullptr);

    if (fsr_properties.layeredShadingRateAttachments == VK_TRUE) {
        subpass.viewMask = 0x4;
        err = vkCreateRenderPass2KHR(m_device->device(), &rpci, nullptr, &rp);
        ASSERT_VK_SUCCESS(err);
        subpass.viewMask = 0;

        fbai_info.layerCount = 2;
        fb_info.renderPass = rp;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUIDUndefined);
        err = vk::CreateFramebuffer(device(), &fb_info, nullptr, &fb);
        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
        }
        fbai_info.layerCount = 1;

        vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    }
}

TEST_F(VkLayerTest, ImagelessFramebufferRenderPassBeginImageView3D) {
    TEST_DESCRIPTION("Misuse of VK_IMAGE_VIEW_TYPE_3D.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s test requires VK_KHR_imageless_framebuffer, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        LvlInitStruct<VkPhysicalDeviceFeatures2>(&physicalDeviceImagelessFramebufferFeatures);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &physicalDeviceFeatures2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormats[1] = {VK_FORMAT_R8G8B8A8_UNORM};
    VkFormat framebufferAttachmentFormats[1] = {VK_FORMAT_R8G8B8A8_UNORM};

    // Create a renderPass with a single attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = attachmentFormats[0];
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentReference attachmentReference = {};
    attachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    VkRenderPassCreateInfo renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo>();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    // Create Attachments
    VkImageCreateInfo imageCreateInfo = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo imageViewCreateInfo = LvlInitStruct<VkImageViewCreateInfo>();
    imageViewCreateInfo.image = image3D.handle();
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    imageViewCreateInfo.format = attachmentFormats[0];
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    VkImageView imageView3D;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, nullptr, &imageView3D);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
    framebufferAttachmentImageInfo.flags = 0;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;

    VkFramebuffer framebuffer;
    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>();
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.renderPass = renderPass;

    // Try to use 3D Image View without imageless flag
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.pAttachments = &imageView3D;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04113");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.pAttachments = nullptr;
    m_errorMonitor->ExpectSuccess();
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyNotFound();

    VkRenderPassAttachmentBeginInfoKHR renderPassAttachmentBeginInfo = LvlInitStruct<VkRenderPassAttachmentBeginInfo>();
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = &imageView3D;
    VkRenderPassBeginInfo renderPassBeginInfo = LvlInitStruct<VkRenderPassBeginInfo>(&renderPassAttachmentBeginInfo);
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.extent.width = attachmentWidth;
    renderPassBeginInfo.renderArea.extent.height = attachmentHeight;
    renderPassBeginInfo.framebuffer = framebuffer;

    // Try to use 3D Image View with imageless flag
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114",
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114");

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView3D, nullptr);
}

TEST_F(VkLayerTest, FramebufferAttachmentImageInfoPNext) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AddRequiredDeviceExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)) {
        printf("%s test requires VK_KHR_imageless_framebuffer, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat attachment_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFramebufferAttachmentImageInfo fb_fdm = LvlInitStruct<VkFramebufferAttachmentImageInfo>();
    fb_fdm.pNext = &fb_fdm;
    fb_fdm.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    fb_fdm.width = 64;
    fb_fdm.height = 64;
    fb_fdm.layerCount = 1;
    fb_fdm.viewFormatCount = 1;
    fb_fdm.pViewFormats = &attachment_format;

    VkFramebufferAttachmentsCreateInfo fb_aci_fdm = LvlInitStruct<VkFramebufferAttachmentsCreateInfo>();
    fb_aci_fdm.attachmentImageInfoCount = 1;
    fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

    VkFramebufferCreateInfo framebufferCreateInfo = LvlInitStruct<VkFramebufferCreateInfo>(&fb_aci_fdm);
    framebufferCreateInfo.width = 64;
    framebufferCreateInfo.height = 64;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(m_framebuffer_attachments.size());
    framebufferCreateInfo.pAttachments = m_framebuffer_attachments.data();

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferAttachmentImageInfo-pNext-pNext");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DescriptorUpdateTemplateEntryWithInlineUniformBlock) {
    TEST_DESCRIPTION("Test VkDescriptorUpdateTemplateEntry with descriptor type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT");

    // GPDDP2 needed for push descriptors support below
    bool gpdp2_support = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION);
    if (gpdp2_support) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME)) {
        printf("%s Descriptor Update Template Extensions not supported, skipped.\n", kSkipPrefix);
        return;
    } else {
        m_device_extension_names.push_back(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    }
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        printf("%s %s not supported, skipped.\n", kSkipPrefix, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        return;
    } else {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    }
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME)) {
        printf("%s %s not supported, skipped.\n", kSkipPrefix, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
        return;
    } else {
        m_device_extension_names.push_back(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    }

    // Note: Includes workaround for some implementations which incorrectly return 0 maxPushDescriptors
    bool push_descriptor_support = gpdp2_support &&
                                   DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) &&
                                   (GetPushDescriptorProperties(instance(), gpu()).maxPushDescriptors > 0);
    if (push_descriptor_support) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s Push Descriptor Extension not supported, push descriptor cases skipped.\n", kSkipPrefix);
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    // Create a buffer to be used for invalid updates
    VkBufferCreateInfo buff_ci = LvlInitStruct<VkBufferCreateInfo>();
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buff_ci.size = m_device->props.limits.minUniformBufferOffsetAlignment;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer;
    buffer.init(*m_device, buff_ci);

    // Relying on the "return nullptr for non-enabled extensions
    auto vkCreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateDescriptorUpdateTemplateKHR");
    auto vkDestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkDestroyDescriptorUpdateTemplateKHR");
    auto vkUpdateDescriptorSetWithTemplateKHR =
        (PFN_vkUpdateDescriptorSetWithTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkUpdateDescriptorSetWithTemplateKHR");

    ASSERT_NE(vkCreateDescriptorUpdateTemplateKHR, nullptr);
    ASSERT_NE(vkDestroyDescriptorUpdateTemplateKHR, nullptr);
    ASSERT_NE(vkUpdateDescriptorSetWithTemplateKHR, nullptr);

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 2;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    auto update_template_ci = LvlInitStruct<VkDescriptorUpdateTemplateCreateInfoKHR>();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorUpdateTemplateEntry-descriptor-02226");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorUpdateTemplateEntry-descriptor-02227");
    vkCreateDescriptorUpdateTemplateKHR(m_device->device(), &update_template_ci, nullptr, &update_template);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RenderPassCreateFragmentDensityMapReferenceToInvalidAttachment) {
    TEST_DESCRIPTION(
        "Test creating a framebuffer with fragment density map reference to an attachment with layer count different from 1");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        printf("%s %s extension not supported skipped.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        printf("%s Tests requires a Vulkan version prior to 1.1, skipping test\n", kSkipPrefix);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fdm_features);
    fdm_features.fragmentDensityMap = true;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, 0));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkAttachmentReference ref;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    rpfdmi.fragmentDensityMapAttachment = ref;

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_R8G8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &ref;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    VkRenderPass renderPass;
    vk::CreateRenderPass(device(), &rpci, nullptr, &renderPass);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 4);

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = renderPass;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-02744");
    vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}
