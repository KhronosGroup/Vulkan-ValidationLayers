/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google, Inc.
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
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s test requires VK_KHR_imageless_framebuffer, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures = {};
    physicalDeviceImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = &physicalDeviceImagelessFramebufferFeatures;

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
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, NULL, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = {};
    framebufferAttachmentImageInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 2;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = {};
    framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    VkFramebuffer framebuffer;

    VkImageFormatListCreateInfoKHR imageFormatListCreateInfo = {};
    imageFormatListCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR;
    imageFormatListCreateInfo.viewFormatCount = 2;
    imageFormatListCreateInfo.pViewFormats = attachmentFormats;
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = &imageFormatListCreateInfo;
    imageCreateInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = attachmentFormats[0];
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageView imageView;
    vk::CreateImageView(m_device->device(), &imageViewCreateInfo, NULL, &imageView);

    VkRenderPassAttachmentBeginInfoKHR renderPassAttachmentBeginInfo = {};
    renderPassAttachmentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR;
    renderPassAttachmentBeginInfo.pNext = nullptr;
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = &imageView;
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = &renderPassAttachmentBeginInfo;
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
                        "VUID-VkRenderPassBeginInfo-framebuffer-03210", "VUID-VkRenderPassBeginInfo-framebuffer-03210");
    vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
    //    vkDestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    vk::DestroyImageView(m_device->device(), imageView, nullptr);
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
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
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
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, NULL, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = {};
    framebufferAttachmentImageInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = {};
    framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
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
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures = {};
    physicalDeviceImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = &physicalDeviceImagelessFramebufferFeatures;
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
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, NULL, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = {};
    framebufferAttachmentImageInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = {};
    framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
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
        multiviewSupported ? "VUID-VkFramebufferCreateInfo-renderPass-03199" : "VUID-VkFramebufferCreateInfo-flags-03200";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, mismatchedLayersNoMultiviewVuid);
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.layers = 1;

    // Mismatched width
    framebufferCreateInfo.width += 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03192");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.width -= 1;

    // Mismatched height
    framebufferCreateInfo.height += 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-03193");
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
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures = {};
    physicalDeviceImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = &physicalDeviceImagelessFramebufferFeatures;
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

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.attachmentCount = 4;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 1;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 1;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 1;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 1;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = {};
    framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
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
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures = {};
    physicalDeviceImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = &physicalDeviceImagelessFramebufferFeatures;
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
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = {};
    renderPassMultiviewCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.pNext = &renderPassMultiviewCreateInfo;
    renderPassCreateInfo.attachmentCount = 4;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass renderPass;
    vk::CreateRenderPass(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);

    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 2;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 2;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = {};
    framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
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
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures = {};
    physicalDeviceImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = &physicalDeviceImagelessFramebufferFeatures;
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
    attachmentDescriptions[0].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
    attachmentDescriptions[0].format = attachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
    attachmentDescriptions[1].format = attachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference2KHR depthStencilAttachmentReference = {};
    depthStencilAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 0;
    VkAttachmentReference2KHR depthStencilResolveAttachmentReference = {};
    depthStencilResolveAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 1;
    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve = {};
    subpassDescriptionDepthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
    subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    subpassDescriptionDepthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    VkSubpassDescription2KHR subpassDescription = {};
    subpassDescription.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
    subpassDescription.pNext = &subpassDescriptionDepthStencilResolve;
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
    subpassDescription.viewMask = 0x3u;

    VkRenderPassCreateInfo2KHR renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
    renderPassCreateInfo.pNext = nullptr;
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
    framebufferAttachmentImageInfos[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &attachmentFormat;
    // Depth/stencil resolve attachment
    framebufferAttachmentImageInfos[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = {};
    framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
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

    // Try creating Framebuffer with images, but with invalid image create usage flags
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = attachmentFormat;
    image_create_info.extent.width = attachmentWidth;
    image_create_info.extent.height = attachmentHeight;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    VkImageObj ds_image(m_device);
    ds_image.init(&image_create_info);
    ASSERT_TRUE(ds_image.initialized());

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = 0;
    VkImageObj ds_resolve_image(m_device);
    ds_resolve_image.init(&image_create_info);
    ASSERT_TRUE(ds_resolve_image.initialized());

    VkImageView image_views[2];
    image_views[0] = ds_image.targetView(attachmentFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    image_views[1] = ds_resolve_image.targetView(attachmentFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.pAttachments = image_views;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-02634");
    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    if (framebuffer != VK_NULL_HANDLE) {
        vk::DestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
    framebufferCreateInfo.pAttachments = nullptr;

    vk::DestroyRenderPass(m_device->device(), renderPass, nullptr);
}
