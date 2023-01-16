/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Camden Stocker <camden@lunarg.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"
#include "best_practices_error_enums.h"
#include "core_validation_error_enums.h"

void VkBestPracticesLayerTest::InitBestPracticesFramework() {
    // Enable all vendor-specific checks
    InitBestPracticesFramework("");
}

void VkBestPracticesLayerTest::InitBestPracticesFramework(const char* vendor_checks_to_enable) {
    // Enable the vendor-specific checks spcified by vendor_checks_to_enable
    VkLayerSettingValueDataEXT bp_setting_string_value{};
    bp_setting_string_value.arrayString.pCharArray = vendor_checks_to_enable;
    bp_setting_string_value.arrayString.count = sizeof(bp_setting_string_value.arrayString.pCharArray);
    VkLayerSettingValueEXT bp_vendor_all_setting_val = {"enables", VK_LAYER_SETTING_VALUE_TYPE_STRING_ARRAY_EXT,
                                                        bp_setting_string_value};
    VkLayerSettingsEXT bp_settings{VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT, nullptr, 1, &bp_vendor_all_setting_val};
    features_.pNext = &bp_settings;

    InitFramework(m_errorMonitor, &features_);
}

TEST_F(VkBestPracticesLayerTest, ValidateReturnCodes) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required, skipping test.";
    }

    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping CmdCopySwapchainImage test";
    }

    // Attempt to force an invalid return code for an unsupported format
    VkImageFormatProperties2 image_format_prop = {};
    image_format_prop.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    VkPhysicalDeviceImageFormatInfo2 image_format_info = {};
    image_format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    image_format_info.format = VK_FORMAT_R32G32B32_SFLOAT;
    image_format_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_format_info.type = VK_IMAGE_TYPE_3D;
    image_format_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);
    // Only run this test if this super-wierd format is not supported
    if (VK_SUCCESS != result) {
        m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-Error-Result");
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);
        m_errorMonitor->VerifyFound();
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Force a non-success success code by only asking for a subset of query results
    uint32_t format_count;
    std::vector<VkSurfaceFormatKHR> formats;
    result = vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, NULL);
    if (result != VK_SUCCESS || format_count <= 1) {
        GTEST_SKIP() << "test requires 2 or more extensions available";
    }
    format_count -= 1;
    formats.resize(format_count);

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "UNASSIGNED-BestPractices-NonSuccess-Result");
    result = vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, formats.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, UseDeprecatedInstanceExtensions) {
    TEST_DESCRIPTION("Create an instance with a deprecated extension.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    // TODO - Currently not working on MockICD with Profiles
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    // Create a 1.1 vulkan instance and request an extension promoted to core in 1.1
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateInstance-deprecated-extension");
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-debugging");
    VkInstance dummy;
    auto features = features_;
    auto ici = GetInstanceCreateInfo();
    features.pNext = ici.pNext;
    ici.pNext = &features;
    vk::CreateInstance(&ici, nullptr, &dummy);
    m_errorMonitor->VerifyFound();

    // Create a 1.0 vulkan instance and request an extension promoted to core in 1.1
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-khronos-Validation-debug-build-warning-message");
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-khronos-Validation-fine-grained-locking-warning-message");
    VkApplicationInfo new_info{};
    new_info.apiVersion = VK_API_VERSION_1_0;
    new_info.pApplicationName = ici.pApplicationInfo->pApplicationName;
    new_info.applicationVersion = ici.pApplicationInfo->applicationVersion;
    new_info.pEngineName = ici.pApplicationInfo->pEngineName;
    new_info.engineVersion = ici.pApplicationInfo->engineVersion;
    ici.pApplicationInfo = &new_info;
    vk::CreateInstance(&ici, nullptr, &dummy);
    vk::DestroyInstance(dummy, nullptr);
}

TEST_F(VkBestPracticesLayerTest, UseDeprecatedDeviceExtensions) {
    TEST_DESCRIPTION("Create a device with a deprecated extension.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkDevice local_device;
    VkDeviceCreateInfo dev_info = {};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    float qp = 1;
    queue_info.pQueuePriorities = &qp;
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateDevice-deprecated-extension");
    vk::CreateDevice(this->gpu(), &dev_info, NULL, &local_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SpecialUseExtensions) {
    TEST_DESCRIPTION("Create a device with a 'specialuse' extension.");

    AddRequiredExtensions(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkDevice local_device;
    VkDeviceCreateInfo dev_info = {};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    float qp = 1;
    queue_info.pQueuePriorities = &qp;
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-d3demulation");
    vk::CreateDevice(this->gpu(), &dev_info, NULL, &local_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, CmdClearAttachmentTest) {
    TEST_DESCRIPTION("Test for validating usage of vkCmdClearAttachments");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Main thing we care about for this test is that the VkImage obj we're
    // clearing matches Color Attachment of FB
    //  Also pass down other dummy params to keep driver and paramchecker happy
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    // Call for full-sized FB Color attachment prior to issuing a Draw
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");

    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, CmdClearAttachmentTestSecondary) {
    TEST_DESCRIPTION("Test for validating usage of vkCmdClearAttachments with secondary command buffers");

    InitBestPracticesFramework();
    InitState();

    if (IsPlatform(PlatformType::kShieldTV) || IsPlatform(PlatformType::kShieldTVb)) {
        GTEST_SKIP() << "Test CmdClearAttachmentTestSecondary is unstable on ShieldTV";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkCommandBufferObj secondary_full_clear(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferObj secondary_small_clear(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT |
                       VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    VkCommandBufferInheritanceInfo inherit_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
    begin_info.pInheritanceInfo = &inherit_info;
    inherit_info.subpass = 0;
    inherit_info.renderPass = m_renderPassBeginInfo.renderPass;

    // Main thing we care about for this test is that the VkImage obj we're
    // clearing matches Color Attachment of FB
    //  Also pass down other dummy params to keep driver and paramchecker happy
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect_small = {{{0, 0}, {(uint32_t)m_width - 1u, (uint32_t)m_height - 1u}}, 0, 1};
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    {
        // Small clears which don't cover the render area should not trigger the warning.
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect_small);
        // Call for full-sized FB Color attachment prior to issuing a Draw
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
        // This test may also trigger other warnings
        m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers");
        m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSmallSecondaryCmdBuffers");

        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->EndRenderPass();

    secondary_small_clear.begin(&begin_info);
    secondary_full_clear.begin(&begin_info);
    vk::CmdClearAttachments(secondary_small_clear.handle(), 1, &color_attachment, 1, &clear_rect_small);
    vk::CmdClearAttachments(secondary_full_clear.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary_small_clear.end();
    secondary_full_clear.end();

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    {
        // Small clears which don't cover the render area should not trigger the warning.
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_small_clear.handle());
        // Call for full-sized FB Color attachment prior to issuing a Draw
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
        // This test may also trigger other warnings
        m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers");
        m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSmallSecondaryCmdBuffers");
        m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-CmdPool-DisparateSizedCmdBuffers");

        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_full_clear.handle());
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->EndRenderPass();
}

TEST_F(VkBestPracticesLayerTest, CmdResolveImageTypeMismatch) {
    InitBestPracticesFramework();
    InitState();

    // Create two images of different types and try to copy between them
    VkImageObj srcImage(m_device);
    VkImageObj dstImage(m_device);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    srcImage.init(&image_create_info);

    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dstImage.init(&image_create_info);

    m_commandBuffer->begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount = 1;
    resolveRegion.srcOffset.x = 0;
    resolveRegion.srcOffset.y = 0;
    resolveRegion.srcOffset.z = 0;
    resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.mipLevel = 0;
    resolveRegion.dstSubresource.baseArrayLayer = 0;
    resolveRegion.dstSubresource.layerCount = 1;
    resolveRegion.dstOffset.x = 0;
    resolveRegion.dstOffset.y = 0;
    resolveRegion.dstOffset.z = 0;
    resolveRegion.extent.width = 1;
    resolveRegion.extent.height = 1;
    resolveRegion.extent.depth = 1;

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-MismatchedImageType");
    m_commandBuffer->ResolveImage(srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                                  &resolveRegion);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkBestPracticesLayerTest, ZeroSizeBlitRegion) {
    TEST_DESCRIPTION("vkCmdBlitImage with a zero area region");

    InitBestPracticesFramework();
    InitState();

    VkImageObj image_src(m_device);
    image_src.Init(128, 128, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    image_src.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkImageObj image_dst(m_device);
    image_dst.Init(128, 128, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    image_dst.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageBlit blit_region = {};
    blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0};
    blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0};
    blit_region.srcOffsets[0] = {128, 0, 0};
    blit_region.srcOffsets[1] = {128, 128, 1};
    blit_region.dstOffsets[0] = {0, 128, 0};
    blit_region.dstOffsets[1] = {128, 128, 1};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-DrawState-InvalidExtents");
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-DrawState-InvalidExtents");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_src.handle(), image_src.Layout(), image_dst.handle(), image_dst.Layout(), 1,
                     &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkBestPracticesLayerTest, CmdBeginRenderPassZeroSizeRenderArea) {
    TEST_DESCRIPTION("Test for getting warned when render area is 0 in VkRenderPassBeginInfo during vkCmdBeginRenderPass");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-zero-size-render-area");

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderArea.extent.width = 0;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, VtxBufferBadIndex) {
    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-VtxIndexOutOfBounds");

    // This test may also trigger other warnings
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers");

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.pNext = NULL;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    // Don't care about actual data, just need to get to draw to flag error
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void*)&vbo_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_commandBuffer->BindVertexBuffer(&vbo, (VkDeviceSize)0, 1);  // VBO idx 1, but no VBO in PSO
    m_commandBuffer->Draw(1, 0, 0, 0);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkBestPracticesLayerTest, CommandBufferReset) {
    TEST_DESCRIPTION("Test for validating usage of vkCreateCommandPool with COMMAND_BUFFER_RESET_BIT");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateCommandPool-command-buffer-reset");

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SecondaryCommandBuffer) {
    TEST_DESCRIPTION("Test for validating usage of vkCreateCommandPool with VK_COMMAND_BUFFER_LEVEL_SECONDARY");

    InitBestPracticesFramework();
    InitState();

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_props;
    queue_family_props.resize(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_family_props.data());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    const VkQueueFlags sec_cmd_buf_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if ((queue_family_props[i].queueFlags & sec_cmd_buf_queue_flags) == 0) {
            queue_family_index = i;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "No queue family found without support for secondary command buffers";
    }

    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = queue_family_index;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo alloc_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkAllocateCommandBuffers-unusable-secondary");
    vk::AllocateCommandBuffers(m_device->device(), &alloc_info, &command_buffer);
    m_errorMonitor->VerifyFound();

    vk::FreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vk::DestroyCommandPool(m_device->device(), command_pool, nullptr);
}

TEST_F(VkBestPracticesLayerTest, SimultaneousUse) {
    TEST_DESCRIPTION("Test for validating usage of vkBeginCommandBuffer with SIMULTANEOUS_USE");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkBeginCommandBuffer-simultaneous-use");

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBeginCommandBuffer-one-time-submit");

    VkCommandBufferBeginInfo cmd_begin_info{};
    cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_begin_info);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SmallAllocation) {
    TEST_DESCRIPTION("Test for small memory allocations");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");

    // Find appropriate memory type for given reqs
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkPhysicalDeviceMemoryProperties dev_mem_props = m_device->phy().memory_properties();

    uint32_t mem_type_index = 0;
    for (mem_type_index = 0; mem_type_index < dev_mem_props.memoryTypeCount; ++mem_type_index) {
        if (mem_props == (mem_props & dev_mem_props.memoryTypes[mem_type_index].propertyFlags)) break;
    }
    EXPECT_LT(mem_type_index, dev_mem_props.memoryTypeCount) << "Could not find a suitable memory type.";

    const uint32_t kSmallAllocationSize = 1024;

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = kSmallAllocationSize;
    alloc_info.memoryTypeIndex = mem_type_index;

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &alloc_info, nullptr, &memory);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SmallDedicatedAllocation) {
    TEST_DESCRIPTION("Test for small dedicated memory allocations");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    // Create a small image with a dedicated allocation
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_info);

    vk_testing::DeviceMemory mem;
    mem.init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(),
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    vk::BindImageMemory(device(), image.handle(), mem.handle(), 0);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, MSImageRequiresMemory) {
    TEST_DESCRIPTION("Test for MS image that requires memory");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateRenderPass-image-requires-memory");

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    attachment.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sd{};

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &sd;

    VkRenderPass rp;
    vk::CreateRenderPass(m_device->device(), &rp_info, nullptr, &rp);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, AttachmentShouldNotBeTransient) {
    TEST_DESCRIPTION("Test for non-lazy multisampled images");

    InitBestPracticesFramework();
    InitState();

    if (IsPlatform(kPixel2XL) || IsPlatform(kPixel3) || IsPlatform(kPixel3aXL) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb) ||
        IsPlatform(kNexusPlayer)) {
        GTEST_SKIP() << "This test seems super-picky on Android platforms";
    }

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateFramebuffer-attachment-should-not-be-transient");

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindImageMemory-non-lazy-transient-image");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkImage-AvoidGeneral");

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sd{};

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &sd;

    VkRenderPass rp = VK_NULL_HANDLE;
    vk::CreateRenderPass(m_device->device(), &rp_info, nullptr, &rp);

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.extent = {1920, 1080, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImageObj image(m_device);
    image.init(&image_info);

    VkImageViewCreateInfo iv_info{};
    iv_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    iv_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    iv_info.image = image.handle();
    iv_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    iv_info.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    VkImageView image_view = VK_NULL_HANDLE;
    vk::CreateImageView(m_device->device(), &iv_info, nullptr, &image_view);

    VkFramebufferCreateInfo fb_info{};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.renderPass = rp;
    fb_info.layers = 1;
    fb_info.width = 1920;
    fb_info.height = 1080;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &image_view;

    VkFramebuffer fb = VK_NULL_HANDLE;
    vk::CreateFramebuffer(m_device->device(), &fb_info, nullptr, &fb);

    m_errorMonitor->VerifyFound();
    vk::DestroyImageView(m_device->device(), image_view, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkBestPracticesLayerTest, TooManyInstancedVertexBuffers) {
    TEST_DESCRIPTION("Test for too many instanced vertex buffers");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers");

    // This test may also trigger the small allocation warnings
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers");

    // This test does not need for the shader to consume the vertex input
    m_errorMonitor->SetAllowedFailureMsg(kVUID_Core_Shader_OutputNotConsumed);

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<VkVertexInputBindingDescription> bindings(2, VkVertexInputBindingDescription{});
    std::vector<VkVertexInputAttributeDescription> attributes(2, VkVertexInputAttributeDescription{});

    bindings[0].binding = 0;
    bindings[0].stride = 4;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32_SFLOAT;

    bindings[1].binding = 1;
    bindings[1].stride = 8;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributes[1].binding = 1;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32_SFLOAT;

    VkPipelineVertexInputStateCreateInfo vi_state_ci{};
    vi_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi_state_ci.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    vi_state_ci.pVertexBindingDescriptions = bindings.data();
    vi_state_ci.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vi_state_ci.pVertexAttributeDescriptions = attributes.data();

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.vi_ci_ = vi_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, ClearAttachmentsAfterLoad) {
    TEST_DESCRIPTION("Test for clearing attachments after load");

    InitBestPracticesFramework();
    InitState();

    m_clear_via_load_op = false;  // Force LOAD_OP_LOAD
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load");

    // On tiled renderers, this can also trigger a warning about LOAD_OP_LOAD causing a readback
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-RenderPass-redundant-store");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-RenderPass-redundant-clear");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-RenderPass-inefficient-clear");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, ClearAttachmentsAfterLoadSecondary) {
    TEST_DESCRIPTION("Test for clearing attachments after load with secondary command buffers");

    InitBestPracticesFramework();
    InitState();

    if (IsPlatform(PlatformType::kShieldTV) || IsPlatform(PlatformType::kShieldTVb)) {
        GTEST_SKIP() << "Test CmdClearAttachmentAfterLoadSecondary is unstable on ShieldTV";
    }

    m_clear_via_load_op = false;  // Force LOAD_OP_LOAD
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // On tiled renderers, this can also trigger a warning about LOAD_OP_LOAD causing a readback
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-RenderPass-redundant-store");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-RenderPass-redundant-clear");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-RenderPass-inefficient-clear");

    CreatePipelineHelper pipe_masked(*this);
    pipe_masked.InitInfo();
    pipe_masked.InitState();
    pipe_masked.cb_attachments_[0].colorWriteMask = 0;
    pipe_masked.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_writes(*this);
    pipe_writes.InitInfo();
    pipe_writes.InitState();
    pipe_writes.cb_attachments_[0].colorWriteMask = 0xf;
    pipe_writes.CreateGraphicsPipeline();

    m_commandBuffer->begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    // Plain clear after load.
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load");
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
    {
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->EndRenderPass();

    // Test that a masked write is ignored before clear
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load");
    {
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_masked.pipeline_);
        m_commandBuffer->Draw(1, 0, 0, 0);
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->EndRenderPass();

    // Test that an actual write will not trigger the clear warning
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    {
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_writes.pipeline_);
        m_commandBuffer->Draw(1, 0, 0, 0);
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    }
    m_commandBuffer->EndRenderPass();

    // Try the same thing, but now with secondary command buffers.
    VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT |
                       VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    VkCommandBufferInheritanceInfo inherit_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
    begin_info.pInheritanceInfo = &inherit_info;
    inherit_info.subpass = 0;
    inherit_info.renderPass = m_renderPassBeginInfo.renderPass;

    VkCommandBufferObj secondary_clear(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferObj secondary_draw_masked(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferObj secondary_draw_write(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary_clear.begin(&begin_info);
    secondary_draw_masked.begin(&begin_info);
    secondary_draw_write.begin(&begin_info);

    vk::CmdClearAttachments(secondary_clear.handle(), 1, &color_attachment, 1, &clear_rect);

    vk::CmdBindPipeline(secondary_draw_masked.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_masked.pipeline_);
    secondary_draw_masked.Draw(1, 0, 0, 0);

    vk::CmdBindPipeline(secondary_draw_write.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_writes.pipeline_);
    secondary_draw_write.Draw(1, 0, 0, 0);

    secondary_clear.end();
    secondary_draw_masked.end();
    secondary_draw_write.end();

    // Plain clear after load.
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load");
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
    {
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_clear.handle());
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->EndRenderPass();

    // Test that a masked write is ignored before clear
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load");
    {
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_draw_masked.handle());
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_clear.handle());
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->EndRenderPass();

    // Test that an actual write will not trigger the clear warning
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    {
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_draw_write.handle());
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_clear.handle());
    }
    m_commandBuffer->EndRenderPass();
}

TEST_F(VkBestPracticesLayerTest, TripleBufferingTest) {
    TEST_DESCRIPTION("Test for usage of triple buffering");

    AddSurfaceExtension();
    InitBestPracticesFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count");
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    bool fifo_present = false;
    for (const auto &present_mode : m_surface_present_modes) {
        if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            fifo_present = true;
            break;
        }
    }
    if (!fifo_present) {
        GTEST_SKIP() << "fifo present mode not supported";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 2;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    swapchain_create_info.minImageCount = 3;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkBestPracticesLayerTest, SwapchainCreationTest) {
    TEST_DESCRIPTION("Test for correct swapchain creation");

    AddSurfaceExtension();
    InitBestPracticesFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    InitState();
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    m_surface_composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    m_surface_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 3;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    // Test for successful swapchain creation when GetPhysicalDeviceSurfaceCapabilitiesKHR() and
    // GetPhysicalDeviceSurfaceFormatsKHR() are queried as expected and GetPhysicalDeviceSurfacePresentModesKHR() is not called but
    // the present mode is VK_PRESENT_MODE_FIFO_KHR
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &m_surface_capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, nullptr);
    if (format_count != 0) {
        m_surface_formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, m_surface_formats.data());
    }

    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};

    // GetPhysicalDeviceSurfacePresentModesKHR() not called before trying to create a swapchain
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-surface-not-retrieved");

    // Warning is thrown any time the present mode is not VK_PRESENT_MODE_FIFO_KHR, but only with ARM BP
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkBestPracticesLayerTest, ExpectedQueryDetails) {
    TEST_DESCRIPTION("Check that GetPhysicalDeviceQueueFamilyProperties is working as expected");

    // Vulkan 1.1 required to test vkGetPhysicalDeviceQueueFamilyProperties2
    app_info_.apiVersion = VK_API_VERSION_1_1;
    // VK_KHR_get_physical_device_properties2 required to test vkGetPhysicalDeviceQueueFamilyProperties2KHR
    instance_extensions_.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    const vk_testing::PhysicalDevice phys_device_obj(gpu_);

    std::vector<VkQueueFamilyProperties> queue_family_props;

    // Ensure we can find a graphics queue family.
    uint32_t queue_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props.resize(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());

    // Now  for GetPhysicalDeviceQueueFamilyProperties2
    std::vector<VkQueueFamilyProperties2> queue_family_props2;
    vk::GetPhysicalDeviceQueueFamilyProperties2(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props2.resize(queue_count);
    for (uint32_t i = 0; i < queue_count; i++) {
        queue_family_props2[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }
    vk::GetPhysicalDeviceQueueFamilyProperties2(phys_device_obj.handle(), &queue_count, queue_family_props2.data());

    // And for GetPhysicalDeviceQueueFamilyProperties2KHR
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR vkGetPhysicalDeviceQueueFamilyProperties2KHR =
        reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceQueueFamilyProperties2KHR"));
    if (vkGetPhysicalDeviceQueueFamilyProperties2KHR) {
        vkGetPhysicalDeviceQueueFamilyProperties2KHR(phys_device_obj.handle(), &queue_count, nullptr);

        queue_family_props2.resize(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties2KHR(phys_device_obj.handle(), &queue_count, queue_family_props2.data());
    }

    vk_testing::Device device(phys_device_obj.handle());
    device.init();
}

TEST_F(VkBestPracticesLayerTest, MissingQueryDetails) {
    TEST_DESCRIPTION("Check that GetPhysicalDeviceQueueFamilyProperties generates appropriate query warning");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    const vk_testing::PhysicalDevice phys_device_obj(gpu_);

    std::vector<VkQueueFamilyProperties> queue_family_props(1);
    uint32_t queue_count = static_cast<uint32_t>(queue_family_props.size());

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-DevLimit-MissingQueryCount");
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());
    m_errorMonitor->VerifyFound();

    // Now get information correctly
    vk_testing::QueueCreateInfoArray queue_info(phys_device_obj.queue_properties());
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t j = 0; j < queue_info.size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    VkPhysicalDeviceFeatures all_features{};
    VkDeviceCreateInfo device_ci = {};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.pNext = nullptr;
    device_ci.queueCreateInfoCount = create_queue_infos.size();
    device_ci.pQueueCreateInfos = create_queue_infos.data();
    device_ci.enabledLayerCount = 0;
    device_ci.ppEnabledLayerNames = NULL;
    device_ci.enabledExtensionCount = 0;
    device_ci.ppEnabledExtensionNames = nullptr;
    device_ci.pEnabledFeatures = &all_features;

    // vkGetPhysicalDeviceFeatures has not been called, so this should produce a warning
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateDevice-physical-device-features-not-retrieved");
    VkDevice device;
    vk::CreateDevice(phys_device_obj.handle(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, GetSwapchainImagesInvalidCount) {
    TEST_DESCRIPTION("Pass an 'incorrect' count to the second GetSwapchainImagesKHR call");

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, kVUID_BestPractices_Swapchain_InvalidCount);
    ++swapchain_images_count;  // Set the image count to something greater (i.e., "invalid") than what was returned
    std::vector<VkImage> swapchain_images(swapchain_images_count, VK_NULL_HANDLE);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, DepthBiasNoAttachment) {
    TEST_DESCRIPTION("Enable depthBias without a depth attachment");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_.depthBiasConstantFactor = 1.0f;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, kVUID_BestPractices_DepthBiasNoAttachment);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkBestPracticesLayerTest, CreatePipelineVsFsTypeMismatchArraySize) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched array sizes across the vertex->fragment shader interface");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float x[2];
        void main(){
           x[0] = 0; x[1] = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x[1];
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x[0]);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit,
                                      "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
}

TEST_F(VkBestPracticesLayerTest, WorkgroupSizeDeprecated) {
    TEST_DESCRIPTION("SPIR-V 1.6 deprecated WorkgroupSize build-in.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kWarningBit,
                                             "UNASSIGNED-BestPractices-SpirvDeprecated_WorkgroupSize");
}

TEST_F(VkBestPracticesLayerTest, CreatePipelineWithoutRenderPass) {
    TEST_DESCRIPTION("Test creating a graphics pipeline with no render pass");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    // This test checks that no BP messages are incorrectly triggered, but triggers core errors
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06603");
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06574");

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkBestPracticesLayerTest, ImageExtendedUsageWithoutMutableFormat) {
    TEST_DESCRIPTION("Create image with extended usage bit but not mutable format bit.");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.flags = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 256;
    image_ci.extent.height = 256;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, kVUID_BestPractices_ImageCreateFlags);
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

#if GTEST_IS_THREADSAFE
TEST_F(VkBestPracticesLayerTest, ThreadUpdateDescriptorUpdateAfterBindNoCollision) {
    TEST_DESCRIPTION("Two threads updating the same UAB descriptor set, expected not to generate a threading error");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables descriptorBindingStorageBufferUpdateAfterBind
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);

    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::array<VkDescriptorBindingFlagsEXT, 2> flags = {
        {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT}};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = (uint32_t)flags.size();
    flags_create_info.pBindingFlags = flags.data();

    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              },
                                              VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &flags_create_info,
                                              VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);

    VkBufferObj buffer;
    buffer.init(*m_device, 256, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    ThreadTestData data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    std::thread thread(UpdateDescriptor, &data);
    // Update descriptors from this thread at the same time.

    ThreadTestData data2;
    data2.device = device();
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    thread.join();

    m_errorMonitor->SetBailout(NULL);
}
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkBestPracticesLayerTest, TransitionFromUndefinedToReadOnly) {
    TEST_DESCRIPTION("Transition image layout from undefined to read only");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkClearColorValue color_clear_value = {};
    color_clear_value.uint32[0] = 255;
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();

    m_commandBuffer->ClearColorImage(image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-TransitionUndefinedToReadOnly");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkBestPracticesLayerTest, CreateFifoRelaxedSwapchain) {
    TEST_DESCRIPTION("Test creating fifo relaxed swapchain");

    AddSurfaceExtension();
    InitBestPracticesFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    InitState();
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    bool fifo_relaxed = false;
    for (const auto& present_mode : m_surface_present_modes) {
        if (present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            fifo_relaxed = true;
            break;
        }
    }
    if (!fifo_relaxed) {
        GTEST_SKIP() << "fifo relaxed present mode not supported";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 2;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkBestPracticesLayerTest, SemaphoreSetWhenCountIsZero) {
    TEST_DESCRIPTION("Set semaphore in SubmitInfo but count is 0");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());


    auto semaphore_ci = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore;
    semaphore.init(*m_device, semaphore_ci);
    VkSemaphore semaphore_handle = semaphore.handle();

    VkSubmitInfo signal_submit_info = LvlInitStruct<VkSubmitInfo>();
    signal_submit_info.signalSemaphoreCount = 0;
    signal_submit_info.pSignalSemaphores = &semaphore_handle;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-SemaphoreCount");
    vk::QueueSubmit(m_device->m_queue, 1, &signal_submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signal_submit_info.signalSemaphoreCount = 1;
    vk::QueueSubmit(m_device->m_queue, 1, &signal_submit_info, VK_NULL_HANDLE);

    VkSubmitInfo wait_submit_info = LvlInitStruct<VkSubmitInfo>();
    wait_submit_info.waitSemaphoreCount = 0;
    wait_submit_info.pWaitSemaphores = &semaphore_handle;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-SemaphoreCount");
    vk::QueueSubmit(m_device->m_queue, 1, &wait_submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkBestPracticesLayerTest, OverAllocateFromDescriptorPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    // Create Pool w/ 1 Sampler descriptor, but try to alloc Uniform Buffer
    // descriptor from it
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_samp(m_device, {dsl_binding_samp});

    // Try to allocate 2 sets when pool only has 1 set
    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout_samp.handle(), ds_layout_samp.handle()};
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-EmptyDescriptorPool");
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, RenderPassClearWithoutLoadOpClear) {
    TEST_DESCRIPTION("Test for clearing a RenderPass with non-zero clearValueCount without any VK_ATTACHMENT_LOAD_OP_CLEAR");


    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Setup necessary objects correctly

    // Bigger size to avoid small allocation best practices warning
    const unsigned int w = 1920;
    const unsigned int h = 1080;

    // Setup Image
    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.extent = {w, h, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImageObj image(m_device);
    image.init(&image_info);

    const auto image_view = image.targetView(image_info.format);

    // Setup RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // Specify that we do nothing with the contents of the attached image
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    attachment.format = image_info.format;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vk_testing::RenderPass rp(*m_device, rp_info);

    // Setup Framebuffer
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.width = w;
    fb_info.height = h;
    fb_info.layers = 1;
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &image_view;

    vk_testing::Framebuffer fb(*m_device, fb_info);

    m_commandBuffer->begin();

    // Create a useless VkClearValue
    VkClearValue cv{};
    cv.color = VkClearColorValue{};
    std::fill(std::begin(cv.color.float32), std::begin(cv.color.float32) + 4, 0.0f);

    VkRenderPassBeginInfo begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    begin_info.clearValueCount = 1;  // Pass one clearValue, in conflict with attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE
    begin_info.pClearValues = &cv;
    begin_info.renderPass = rp.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    // Setup finished

    // TEST :

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, kVUID_BestPractices_ClearValueWithoutLoadOpClear);

    // This should give a warning
    m_commandBuffer->BeginRenderPass(begin_info);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}


TEST_F(VkBestPracticesLayerTest, RenderPassClearValueCountHigherThanAttachmentCount) {
    TEST_DESCRIPTION("Test for beginning a RenderPass with VkRenderPassBeginInfo.clearValueCount > VkRenderPassCreateInfo.attachmentCount");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Setup necessary objects correctly

    // Bigger size to avoid small allocation best practices warning
    const unsigned int w = 1920;
    const unsigned int h = 1080;

    // Setup Image
    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.extent = {w, h, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImageObj image(m_device);
    image.init(&image_info);

    const auto image_view = image.targetView(image_info.format);

    // Setup RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.format = image_info.format;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_info.attachmentCount = 1; // There is only one attachment
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vk_testing::RenderPass rp(*m_device, rp_info);

    // Setup Framebuffer
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.width = w;
    fb_info.height = h;
    fb_info.layers = 1;
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &image_view;

    vk_testing::Framebuffer fb(*m_device, fb_info);

    m_commandBuffer->begin();

    // Create two VkClearValues
    VkClearValue cv[2];

    // Create a useful VkClearValue
    cv[0].color = VkClearColorValue{};
    std::fill(std::begin(cv[0].color.float32), std::begin(cv[0].color.float32) + 4, 0.0f);

    // Create a useless VkClearValue
    cv[1].color = VkClearColorValue{};
    std::fill(std::begin(cv[1].color.float32), std::begin(cv[1].color.float32) + 4, 0.0f);

    VkRenderPassBeginInfo begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    begin_info.clearValueCount = 2;  // Pass 2 clearValues, in conflict with VkRenderPassCreateInfo.attachmentCount == 1 meaning the
                                     // second clearValue will be ignored
    begin_info.pClearValues = cv;
    begin_info.renderPass = rp.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    // Setup finished

    // TEST :

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, kVUID_BestPractices_ClearValueCountHigherThanAttachmentCount);

    // This should give a warning
    m_commandBuffer->BeginRenderPass(begin_info);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkBestPracticesLayerTest, DontCareThenLoad) {
    TEST_DESCRIPTION("Test for storing an attachment with STORE_OP_DONT_CARE then loading with LOAD_OP_LOAD");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Setup necessary objects correctly

    const unsigned int w = 100;
    const unsigned int h = 100;

    // Setup Image
    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.extent = {w, h, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImageObj image(m_device);
    image.init(&image_info);

    const auto image_view = image.targetView(image_info.format);

    // Setup first RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // Clearing as only modification
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // Dont care even though we will load afterwards
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.format = image_info.format;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vk_testing::RenderPass rp1(*m_device, rp_info);

    // Setup second RenderPass
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;  // Loading even though was stored with dont care
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    vk_testing::RenderPass rp2(*m_device, rp_info);

    // Setup Framebuffer
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.width = w;
    fb_info.height = h;
    fb_info.layers = 1;
    fb_info.renderPass = rp1.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &image_view;

    vk_testing::Framebuffer fb(*m_device, fb_info);

    m_commandBuffer->begin();

    // All white
    VkClearValue cv;
    cv.color = VkClearColorValue{};
    std::fill(std::begin(cv.color.float32), std::begin(cv.color.float32) + 4, 1.0f);

    // Begin first renderpass
    VkRenderPassBeginInfo begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &cv;
    begin_info.renderPass = rp1.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    m_commandBuffer->BeginRenderPass(begin_info);

    m_commandBuffer->EndRenderPass();

    // Begin second renderpass
    begin_info.clearValueCount = 0;
    begin_info.pClearValues = nullptr;
    begin_info.renderPass = rp2.handle();

    m_commandBuffer->BeginRenderPass(begin_info);

    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Setup finished

    // TEST :
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, kVUID_BestPractices_StoreOpDontCareThenLoadOpLoad);

    // This should give a warning
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkBestPracticesLayerTest, LoadDeprecatedExtension) {
    TEST_DESCRIPTION("Test for loading a vk1.3 deprecated extension with a 1.3 instance on a 1.2 or less device");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    const char *extension = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;

    if (!DeviceExtensionSupported(extension)) {
        GTEST_SKIP() << extension << " not supported.";
    }

    VkDeviceQueueCreateInfo qci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    qci.queueFamilyIndex = 0;
    float priority = 1;
    qci.pQueuePriorities = &priority;
    qci.queueCount = 1;

    VkDeviceCreateInfo dev_info = LvlInitStruct<VkDeviceCreateInfo>();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &qci;
    dev_info.enabledExtensionCount = 1;
    dev_info.ppEnabledExtensionNames = &extension;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateDevice-deprecated-extension");
    // api version != device version
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCreateDevice-API-version-mismatch");

    VkDevice device = VK_NULL_HANDLE;
    vk::CreateDevice(gpu(), &dev_info, nullptr, &device);

    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        m_errorMonitor->VerifyFound();
    }

    if (device) vk::DestroyDevice(device, nullptr);
}

TEST_F(VkBestPracticesLayerTest, ExclusiveImageMultiQueueUsage) {
    TEST_DESCRIPTION("Test for using a queue exclusive image on multiple queues");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkQueueObj *graphics_queue = m_device->GetDefaultQueue();

    VkQueueObj *compute_queue = nullptr;
    for (uint32_t i = 0; i < m_device->compute_queues().size(); ++i) {
        auto cqi = m_device->compute_queues()[i];
        if (cqi->get_family_index() != graphics_queue->get_family_index()) {
            compute_queue = cqi;
            break;
        }
    }

    if (compute_queue == nullptr) {
        GTEST_SKIP() << "No separate queue family from graphics queue";
    }

    // Setup necessary objects correctly

    const unsigned int w = 100;
    const unsigned int h = 100;

    // Setup Image
    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.extent = {w, h, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImageObj image(m_device);
    image.init(&image_info);

    const auto image_view = image.targetView(image_info.format);

    // Prepare graphics

    // Setup RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;    // Clearing so warning will not trigger on second pass
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // Store written image for next queue family
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.format = image_info.format;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vk_testing::RenderPass rp(*m_device, rp_info);

    // Setup Framebuffer
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.width = w;
    fb_info.height = h;
    fb_info.layers = 1;
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &image_view;

    vk_testing::Framebuffer fb(*m_device, fb_info);

    VkCommandPoolObj graphics_pool(m_device, graphics_queue->get_family_index());

    VkCommandBufferObj graphics_buffer(m_device, &graphics_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, graphics_queue);

    VkClearValue cv;
    cv.color = VkClearColorValue{};
    std::fill(std::begin(cv.color.float32), std::begin(cv.color.float32) + 4, 1.0f);

    VkRenderPassBeginInfo begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &cv;
    begin_info.renderPass = rp.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    // Prepare compute

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1, local_size_y=1) in;
    layout(set=0, binding=0, rgba32f) uniform image2D img;
    void main(){
        vec4 v = imageLoad(img, ivec2(gl_GlobalInvocationID.xy));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitDescriptorSetInfo();
    pipe.dsl_bindings_[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    pipe.dsl_bindings_[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkSamplerObj sampler(m_device);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkCommandPoolObj compute_pool(m_device, compute_queue->get_family_index());

    VkCommandBufferObj compute_buffer(m_device, &compute_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, compute_queue);

    // Record command buffers without queue transition

    // Record graphics command buffer
    graphics_buffer.begin();

    graphics_buffer.BeginRenderPass(begin_info);

    graphics_buffer.EndRenderPass();

    graphics_buffer.end();

    graphics_buffer.QueueCommandBuffer();

    // Record compute command buffer
    compute_buffer.begin();

    vk::CmdBindPipeline(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    vk::CmdBindDescriptorSets(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDispatch(compute_buffer.handle(), w, h, 1);

    compute_buffer.end();

    // Warning should trigger as we are potentially accessing undefined resources
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-ConcurrentUsageOfExclusiveImage");
    compute_buffer.QueueCommandBuffer();
    m_errorMonitor->VerifyFound();

    vk::ResetCommandPool(device(), graphics_pool.handle(), 0);
    vk::ResetCommandPool(device(), compute_pool.handle(), 0);

    // Record command buffers with queue transition

    // Queue transition barrier, same for release and acquire
    VkImageMemoryBarrier barrier = LvlInitStruct<VkImageMemoryBarrier>();
    barrier.image = image.handle();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // only matters for release
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;             // only matters for acquire
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = graphics_queue->get_family_index();
    barrier.dstQueueFamilyIndex = compute_queue->get_family_index();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    // Record graphics command buffer
    graphics_buffer.begin();

    graphics_buffer.BeginRenderPass(begin_info);

    graphics_buffer.EndRenderPass();

    vk::CmdPipelineBarrier(graphics_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

    graphics_buffer.end();

    graphics_buffer.QueueCommandBuffer();

    // Record compute command buffer
    compute_buffer.begin();

    vk::CmdPipelineBarrier(compute_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

    vk::CmdBindPipeline(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    vk::CmdBindDescriptorSets(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDispatch(compute_buffer.handle(), w, h, 1);

    compute_buffer.end();

    // Warning shouldn't trigger
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-ConcurrentUsageOfExclusiveImage");
    compute_buffer.QueueCommandBuffer();
    m_errorMonitor->Finish();
}

TEST_F(VkBestPracticesLayerTest, ImageMemoryBarrierAccessLayoutCombinations) {
    TEST_DESCRIPTION("Transition image layout from undefined to read only");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    bool sync2 = AreRequiredExtensionsEnabled();

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_IMAGE_TILING_OPTIMAL, 0);

    VkClearColorValue color_clear_value = {};
    color_clear_value.uint32[0] = 255;
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    auto img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();

    m_commandBuffer->ClearColorImage(image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);

    // GENERAL - Any
    // note: the table in PR 2918 originally said that 0 was not allowed, but this was incorrect. See Issue #4735
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // Every table entry includes an implicit "can be 0"
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // PRESENT_SRC_KHR	- Must be 0
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-ImageBarrierAccessLayout");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    if (sync2) {
        auto vkCmdPipelineBarrier2KHR =
            reinterpret_cast<PFN_vkCmdPipelineBarrier2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdPipelineBarrier2KHR"));

        auto img_barrier2 = LvlInitStruct<VkImageMemoryBarrier2KHR>();
        img_barrier2.srcAccessMask = 0;
        img_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_barrier2.image = image.handle();
        img_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        img_barrier2.subresourceRange.baseArrayLayer = 0;
        img_barrier2.subresourceRange.baseMipLevel = 0;
        img_barrier2.subresourceRange.layerCount = 1;
        img_barrier2.subresourceRange.levelCount = 1;

        auto dependency_info = LvlInitStruct<VkDependencyInfoKHR>();
        dependency_info.imageMemoryBarrierCount = 1;
        dependency_info.pImageMemoryBarriers = &img_barrier2;

        img_barrier2.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        img_barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-ImageBarrierAccessLayout");
        vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);
        m_errorMonitor->VerifyFound();

        img_barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        img_barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

        // make sure bits above UINT32_MAX are detected
        img_barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        img_barrier2.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-ImageBarrierAccessLayout");
        vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);
        m_errorMonitor->VerifyFound();

        img_barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        img_barrier2.dstAccessMask = 0;
        vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

        m_commandBuffer->end();
    }
}
