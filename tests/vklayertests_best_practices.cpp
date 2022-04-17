/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
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
    VkLayerSettingsEXT bp_settings{static_cast<VkStructureType>(VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT), nullptr, 1,
                                   &bp_vendor_all_setting_val};
    features_.pNext = &bp_settings;
    InitFramework(m_errorMonitor, &features_);
}

TEST_F(VkBestPracticesLayerTest, ValidateReturnCodes) {
    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping CmdCopySwapchainImage test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping CmdCopySwapchainImage test\n", kSkipPrefix);
        return;
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

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping test case.\n", kSkipPrefix);
        return;
    }

    // Force a non-success success code by only asking for a subset of query results
    uint32_t format_count;
    std::vector<VkSurfaceFormatKHR> formats;
    result = vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, NULL);
    if (result != VK_SUCCESS || format_count <= 1) {
        printf("%s test requires 2 or more extensions available, skipping test.\n", kSkipPrefix);
        return;
    }
    format_count -= 1;
    formats.resize(format_count);

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "UNASSIGNED-BestPractices-NonSuccess-Result");
    result = vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &format_count, formats.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, UseDeprecatedInstanceExtensions) {
    TEST_DESCRIPTION("Create an instance with a deprecated extension.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find %s extension, skipped.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

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
    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
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
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkBestPracticesLayerTest, UseDeprecatedDeviceExtensions) {
    TEST_DESCRIPTION("Create a device with a deprecated extension.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find %s extension, skipped.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required for device, skipping test\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        return;
    }

    VkDevice local_device;
    VkDeviceCreateInfo dev_info = {};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = nullptr;
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

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping test\n", kSkipPrefix, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
        return;
    }

    VkDevice local_device;
    VkDeviceCreateInfo dev_info = {};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = nullptr;
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
        printf("%s Test CmdClearAttachmentTestSecondary is unstable on ShieldTV, skipping test.\n", kSkipPrefix);
        return;
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
        m_errorMonitor->VerifyNotFound();
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
        m_errorMonitor->VerifyNotFound();
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

// This is a positive test. No failures are expected.
TEST_F(VkBestPracticesLayerTest, TestDestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    InitBestPracticesFramework();
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->ExpectSuccess();

    vk::DestroyBuffer(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyBufferView(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyCommandPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorSetLayout(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDevice(VK_NULL_HANDLE, NULL);
    vk::DestroyEvent(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFence(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFramebuffer(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImage(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImageView(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyInstance(VK_NULL_HANDLE, NULL);
    vk::DestroyPipeline(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineCache(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineLayout(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyQueryPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyRenderPass(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroySampler(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroySemaphore(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyShaderModule(m_device->device(), VK_NULL_HANDLE, NULL);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);

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
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
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
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_VK_SUCCESS(err);
    vk::FreeDescriptorSets(m_device->device(), ds_pool, 3, descriptor_sets);
    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);

    vk::FreeMemory(m_device->device(), VK_NULL_HANDLE, NULL);

    m_errorMonitor->VerifyNotFound();
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

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;

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
        printf("%s This test seems super-picky on Android platforms\n", kSkipPrefix);
        return;
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

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;

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

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<VkVertexInputBindingDescription> bindings(2, VkVertexInputBindingDescription{});
    std::vector<VkVertexInputAttributeDescription> attributes(2, VkVertexInputAttributeDescription{});

    bindings[0].binding = 0;
    bindings[0].stride = 4;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributes[0].binding = 0;

    bindings[1].binding = 1;
    bindings[1].stride = 8;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributes[1].binding = 1;

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
        printf("%s Test CmdClearAttachmentAfterLoadSecondary is unstable on ShieldTV, skipping test.\n", kSkipPrefix);
        return;
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
        m_errorMonitor->VerifyNotFound();
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
        m_errorMonitor->VerifyNotFound();
    }
    m_commandBuffer->EndRenderPass();
}

TEST_F(VkBestPracticesLayerTest, TripleBufferingTest) {
    TEST_DESCRIPTION("Test for usage of triple buffering");

    AddSurfaceInstanceExtension();
    InitBestPracticesFramework();
    AddSwapchainDeviceExtension();
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count");
    if (!InitSurface()) {
        printf("%s Cannot create surface, skipping test\n", kSkipPrefix);
        return;
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        printf("%s Graphics queue does not support present, skipping test\n", kSkipPrefix);
        return;
    }

    bool fifo_present = false;
    for (const auto &present_mode : m_surface_present_modes) {
        if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            fifo_present = true;
            break;
        }
    }
    if (!fifo_present) {
        printf("%s fifo present mode not supported, skipping test.\n", kSkipPrefix);
        return;
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

    VkResult err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count");
    swapchain_create_info.minImageCount = 3;
    err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err)
    DestroySwapchain();
}

TEST_F(VkBestPracticesLayerTest, SwapchainCreationTest) {
    TEST_DESCRIPTION("Test for correct swapchain creation");

    AddSurfaceInstanceExtension();
    InitBestPracticesFramework();
    AddSwapchainDeviceExtension();
    InitState();
    if (!InitSurface()) {
        printf("%s Cannot create surface, skipping test\n", kSkipPrefix);
        return;
    }

    // GetPhysicalDeviceSurfaceCapabilitiesKHR() not called before trying to create a swapchain
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-surface-not-retrieved");

    // GetPhysicalDeviceSurfaceFormatsKHR() not called before trying to create a swapchain
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-surface-not-retrieved");

    // GetPhysicalDeviceSurfacePresentModesKHR() not called before trying to create a swapchain
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-surface-not-retrieved");

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

    // Set unexpected error because warning is thrown any time the present mode is not VK_PRESENT_MODE_FIFO_KHR
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");

    VkResult err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    ASSERT_TRUE(err == VK_ERROR_VALIDATION_FAILED_EXT);
    m_errorMonitor->VerifyFound();

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
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    m_errorMonitor->ExpectSuccess(kWarningBit);
    err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyNotFound();

    DestroySwapchain();
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

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    // Ensure we can find a graphics queue family.
    uint32_t queue_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props.resize(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());

    // Now  for GetPhysicalDeviceQueueFamilyProperties2
    std::vector<VkQueueFamilyProperties2> queue_family_props2;
    vk::GetPhysicalDeviceQueueFamilyProperties2(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props2.resize(queue_count);
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

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-CoreValidation-DevLimit-MissingQueryCount");
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());
    m_errorMonitor->VerifyFound();

    // Now get information correctly
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    vk_testing::QueueCreateInfoArray queue_info(phys_device_obj.queue_properties());
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t j = 0; j < queue_info.size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }
    m_errorMonitor->VerifyNotFound();

    VkPhysicalDeviceFeatures all_features;
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

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping test\n", kSkipPrefix);
        return;
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
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
        return;
    }

    const std::string spv_source = R"(
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

    m_errorMonitor->ExpectSuccess();

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyNotFound();
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
    test_platform_thread thread;
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredInstanceExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Descriptor Indexing or Maintenance3 Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables descriptorBindingStorageBufferUpdateAfterBind
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        printf("%s Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind, skipping\n", kSkipPrefix);
        return;
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

    struct thread_data_struct data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    bool bailout = false;
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    test_platform_thread_create(&thread, UpdateDescriptor, (void *)&data);
    // Update descriptors from this thread at the same time.

    struct thread_data_struct data2;
    data2.device = device();
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    test_platform_thread_join(thread, NULL);

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyNotFound();
}
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkBestPracticesLayerTest, TransitionFromUndefinedToReadOnly) {
    TEST_DESCRIPTION("Transition image layout from undefined to read only");

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

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
    img_barrier.dstAccessMask = 0;
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

    AddSurfaceInstanceExtension();
    InitBestPracticesFramework();
    AddSwapchainDeviceExtension();
    InitState();
    if (!InitSurface()) {
        printf("%s Cannot create surface, skipping test\n", kSkipPrefix);
        return;
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        printf("%s Graphics queue does not support present, skipping test\n", kSkipPrefix);
        return;
    }

    bool fifo_relaxed = false;
    for (const auto& present_mode : m_surface_present_modes) {
        if (present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            fifo_relaxed = true;
            break;
        }
    }
    if (!fifo_relaxed) {
        printf("%s fifo relaxed present mode not supported, skipping test\n", kSkipPrefix);
        return;
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

    m_errorMonitor->ExpectSuccess(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyNotFound();
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

    m_errorMonitor->ExpectSuccess(kWarningBit);
    signal_submit_info.signalSemaphoreCount = 1;
    vk::QueueSubmit(m_device->m_queue, 1, &signal_submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    VkSubmitInfo wait_submit_info = LvlInitStruct<VkSubmitInfo>();
    wait_submit_info.waitSemaphoreCount = 0;
    wait_submit_info.pWaitSemaphores = &semaphore_handle;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-SemaphoreCount");
    vk::QueueSubmit(m_device->m_queue, 1, &wait_submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, OverAllocateFromDescriptorPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");
    VkResult err;
    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
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

    VkDescriptorPool ds_pool;
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

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
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "UNASSIGNED-BestPractices-EmptyDescriptorPool");
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();
}
