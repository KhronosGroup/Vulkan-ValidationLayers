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
 * Author: Camden Stocker <camden@lunarg.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

void VkBestPracticesLayerTest::InitBestPracticesFramework() {
    // Enable all vendor-specific checks
    VkLayerSettingValueDataEXT bp_setting_string_value{};
    bp_setting_string_value.arrayString.pCharArray = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL";
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
    VkApplicationInfo* new_info = new VkApplicationInfo;
    new_info->apiVersion = VK_API_VERSION_1_0;
    new_info->pApplicationName = ici.pApplicationInfo->pApplicationName;
    new_info->applicationVersion = ici.pApplicationInfo->applicationVersion;
    new_info->pEngineName = ici.pApplicationInfo->pEngineName;
    new_info->engineVersion = ici.pApplicationInfo->engineVersion;
    ici.pApplicationInfo = new_info;
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

TEST_F(VkBestPracticesLayerTest, VtxBufferBadIndex) {
    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-VtxIndexOutOfBounds");

    // This test may also trigger other warnings
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");

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

// Tests for Arm-specific best practices

TEST_F(VkArmBestPracticesLayerTest, TooManySamples) {
    TEST_DESCRIPTION("Test for multisampled images with too many samples");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateImage-too-large-sample-count");

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.extent = {1920, 1080, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_8_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImage image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->device(), &image_info, nullptr, &image);

    m_errorMonitor->VerifyFound();

    if (image) {
        vk::DestroyImage(m_device->device(), image, nullptr);
    }
}

TEST_F(VkArmBestPracticesLayerTest, NonTransientMSImage) {
    TEST_DESCRIPTION("Test for non-transient multisampled images");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateImage-non-transient-ms-image");

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.extent = {1920, 1080, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImage image;
    vk::CreateImage(m_device->device(), &image_info, nullptr, &image);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, SamplerCreation) {
    TEST_DESCRIPTION("Test for various checks during sampler creation");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-different-wrapping-modes");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-lod-clamping");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-lod-bias");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-border-clamp-color");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-unnormalized-coordinates");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-anisotropy");

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 4.0f;
    sampler_info.mipLodBias = 1.0f;
    sampler_info.unnormalizedCoordinates = VK_TRUE;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 4.0f;

    VkSampler sampler = VK_NULL_HANDLE;
    vk::CreateSampler(m_device->device(), &sampler_info, nullptr, &sampler);

    m_errorMonitor->VerifyFound();

    if (sampler) {
        vk::DestroySampler(m_device->device(), sampler, nullptr);
    }
}

TEST_F(VkArmBestPracticesLayerTest, MultisampledBlending) {
    TEST_DESCRIPTION("Test for multisampled blending");

    InitBestPracticesFramework();
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreatePipelines-multisampled-blending");

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    vk::CreateRenderPass(m_device->device(), &rp_info, nullptr, &m_renderPass);
    m_renderPass_info = rp_info;

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

    VkPipelineColorBlendAttachmentState blend_att = {};
    blend_att.blendEnable = VK_TRUE;
    blend_att.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo pipe_cb_state_ci = {};
    pipe_cb_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipe_cb_state_ci.attachmentCount = 1;
    pipe_cb_state_ci.pAttachments = &blend_att;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    pipe.cb_ci_ = pipe_cb_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, AttachmentNeedsReadback) {
    TEST_DESCRIPTION("Test for attachments that need readback");

    InitBestPracticesFramework();
    InitState();

    m_clear_via_load_op = false;  // Force LOAD_OP_LOAD
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, ManySmallIndexedDrawcalls) {
    InitBestPracticesFramework();
    InitState();

    if (IsPlatform(kNexusPlayer) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls");

    // This test may also trigger other warnings
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");

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

    for (int i = 0; i < 10; i++) {
        m_commandBuffer->DrawIndexed(3, 1, 0, 0, 0);
    }

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkArmBestPracticesLayerTest, SuboptimalDescriptorReuseTest) {
    TEST_DESCRIPTION("Test for validation warnings of potentially suboptimal re-use of descriptor set allocations");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 3;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 6;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    VkResult err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding ds_binding = {};
    ds_binding.binding = 0;
    ds_binding.descriptorCount = 1;
    ds_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    VkDescriptorSetLayoutCreateInfo ds_layout_info = {};
    ds_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_info.bindingCount = 1;
    ds_layout_info.pBindings = &ds_binding;

    VkDescriptorSetLayout ds_layout;
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_info, nullptr, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    auto ds_layouts = std::vector<VkDescriptorSetLayout>(ds_pool_ci.maxSets, ds_layout);

    std::vector<VkDescriptorSet> descriptor_sets = {};
    descriptor_sets.resize(ds_layouts.size());

    // allocate N/2 descriptor sets
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.descriptorSetCount = descriptor_sets.size() / 2;
    alloc_info.pSetLayouts = ds_layouts.data();

    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets.data());
    ASSERT_VK_SUCCESS(err);

    // free one descriptor set
    VkDescriptorSet* ds = descriptor_sets.data();
    err = vk::FreeDescriptorSets(m_device->device(), ds_pool, 1, ds);

    // the previous allocate and free should not cause any warning
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();

    // allocate the previously freed descriptor set
    alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = ds_layouts.data();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkAllocateDescriptorSets-suboptimal-reuse");

    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, ds);

    // this should create a validation warning, in addition to the appropriate warning message
    m_errorMonitor->VerifyFound();

    // allocate the remaining descriptor sets (N - (N/2))
    alloc_info.descriptorSetCount = descriptor_sets.size() - (descriptor_sets.size() / 2);
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, ds);

    // this should create no validation warnings
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkArmBestPracticesLayerTest, SparseIndexBufferTest) {
    TEST_DESCRIPTION(
        "Test for appropriate warnings to be thrown when recording an indexed draw call with sparse/non-sparse index buffers.");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    // create a non-sparse index buffer
    std::vector<uint16_t> nonsparse_indices;
    nonsparse_indices.resize(128);
    for (unsigned i = 0; i < nonsparse_indices.size(); i++) {
        nonsparse_indices[i] = i;
    }

    std::vector<uint16_t> sparse_indices = nonsparse_indices;
    // The buffer (0, 1, 2, ..., n) is completely un-sparse. However, if n < 0xFFFF, by adding 0xFFFF at the end, we
    // should trigger a warning due to loading all the indices in the range 0 to 0xFFFF, despite indices in the range
    // (n+1) to (0xFFFF - 1) not being used.
    sparse_indices[sparse_indices.size() - 1] = 0xFFFF;

    VkConstantBufferObj nonsparse_ibo(m_device, nonsparse_indices.size() * sizeof(uint16_t), nonsparse_indices.data(),
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkConstantBufferObj sparse_ibo(m_device, sparse_indices.size() * sizeof(uint16_t), sparse_indices.data(),
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.ia_ci_.primitiveRestartEnable = VK_FALSE;
    pipe.CreateGraphicsPipeline();

    // pipeline with primitive restarts enabled
    CreatePipelineHelper pr_pipe(*this);
    pr_pipe.InitInfo();
    pr_pipe.InitState();
    pr_pipe.ia_ci_.primitiveRestartEnable = VK_TRUE;
    pr_pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    auto test_pipelines = [&](VkConstantBufferObj& ibo, size_t index_count, bool expect_error) -> void {
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_commandBuffer->BindIndexBuffer(&ibo, static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);
        m_errorMonitor->VerifyNotFound();

        // the validation layer will only be able to analyse mapped memory, it's too expensive otherwise to do in the layer itself
        ibo.memory().map();
        if (expect_error)
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                                 "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer");
        m_commandBuffer->DrawIndexed(index_count, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();
        ibo.memory().unmap();

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pr_pipe.pipeline_);
        m_commandBuffer->BindIndexBuffer(&ibo, static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);
        m_errorMonitor->VerifyNotFound();

        ibo.memory().map();
        if (expect_error)
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                                 "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer");
        m_commandBuffer->DrawIndexed(index_count, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();
        ibo.memory().unmap();
    };

    // our non-sparse indices should not trigger a warning for both pipelines in this case
    test_pipelines(nonsparse_ibo, nonsparse_indices.size(), false);
    // our sparse indices should trigger warnings for both pipelines in this case
    test_pipelines(sparse_ibo, sparse_indices.size(), true);
}

TEST_F(VkArmBestPracticesLayerTest, PostTransformVertexCacheThrashingIndicesTest) {
    TEST_DESCRIPTION(
        "Test for appropriate warnings to be thrown when recording an indexed draw call where the indices thrash the "
        "post-transform vertex cache.");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    std::vector<uint16_t> worst_indices;
    worst_indices.resize(128 * 16);
    for (size_t i = 0; i < 16; i++) {
        for (size_t j = 0; j < 128; j++) {
            // worst case index buffer sequence for re-use
            // (0, 1, 2, 3, ..., 127, 0, 1, 2, 3, ..., 127, 0, 1, 2, ...<x16>)
            worst_indices[j + i * 128] = j;
        }
    }

    std::vector<uint16_t> best_indices;
    best_indices.resize(128 * 16);
    for (size_t i = 0; i < 16; i++) {
        for (size_t j = 0; j < 128; j++) {
            // best case index buffer sequence for re-use
            // (0, 0, 0, ...<x16>, 1, 1, 1, ...<x16>, 2, 2, 2, ...<x16> , ..., 127)
            best_indices[i + j * 16] = j;
        }
    }

    // make sure the worst-case indices throw a warning
    VkConstantBufferObj worst_ibo(m_device, worst_indices.size() * sizeof(uint16_t), worst_indices.data(),
                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    m_commandBuffer->BindIndexBuffer(&worst_ibo, static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);
    m_errorMonitor->VerifyNotFound();

    // the validation layer will only be able to analyse mapped memory, it's too expensive otherwise to do in the layer itself
    worst_ibo.memory().map();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCmdDrawIndexed-post-transform-cache-thrashing");
    m_commandBuffer->DrawIndexed(worst_indices.size(), 0, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    worst_ibo.memory().unmap();

    // make sure that the best-case indices don't throw a warning
    VkConstantBufferObj best_ibo(m_device, best_indices.size() * sizeof(uint16_t), best_indices.data(),
                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    m_commandBuffer->BindIndexBuffer(&best_ibo, static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);
    m_errorMonitor->VerifyNotFound();

    best_ibo.memory().map();
    m_commandBuffer->DrawIndexed(best_indices.size(), 0, 0, 0, 0);
    m_errorMonitor->VerifyNotFound();
    best_ibo.memory().unmap();
}

TEST_F(VkBestPracticesLayerTest, TripleBufferingTest) {
    TEST_DESCRIPTION("Test for usage of triple buffering");

    AddSurfaceInstanceExtension();
    InitBestPracticesFramework();
    AddSwapchainDeviceExtension();
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count");
    InitSurface();
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkSurfaceCapabilitiesKHR capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->phy().handle(), m_surface, &capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats;
    if (format_count != 0) {
        formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), m_surface, &present_mode_count, nullptr);
    vector<VkPresentModeKHR> present_modes;
    if (present_mode_count != 0) {
        present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), m_surface, &present_mode_count, present_modes.data());
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 2;
    swapchain_create_info.imageFormat = formats[0].format;
    swapchain_create_info.imageColorSpace = formats[0].colorSpace;
    swapchain_create_info.imageExtent = {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
    swapchain_create_info.presentMode = present_modes[0];
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    VkResult err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count");
    swapchain_create_info.minImageCount = 3;
    err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err)
    DestroySwapchain();
}

TEST_F(VkArmBestPracticesLayerTest, PresentModeTest) {
    TEST_DESCRIPTION("Test for usage of Presentation Modes");

    AddSurfaceInstanceExtension();
    InitBestPracticesFramework();
    AddSwapchainDeviceExtension();
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");
    InitSurface();
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkSurfaceCapabilitiesKHR capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->phy().handle(), m_surface, &capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats;
    if (format_count != 0) {
        formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), m_surface, &present_mode_count, nullptr);
    vector<VkPresentModeKHR> present_modes;
    if (present_mode_count != 0) {
        present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), m_surface, &present_mode_count, present_modes.data());
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = capabilities.minImageCount;
    swapchain_create_info.imageFormat = formats[0].format;
    swapchain_create_info.imageColorSpace = formats[0].colorSpace;
    swapchain_create_info.imageExtent = {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
    if (present_modes.size() <= 1) {
        printf("TEST SKIPPED: Only %i presentation mode is available!", int(present_modes.size()));
        return;
    }

    for (size_t i = 0; i < present_modes.size(); i++) {
        if (present_modes[i] != VK_PRESENT_MODE_FIFO_KHR) {
            swapchain_create_info.presentMode = present_modes[i];
            break;
        }
    }

    VkResult err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    err = vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err)
    DestroySwapchain();
}

TEST_F(VkArmBestPracticesLayerTest, PipelineDepthBiasZeroTest) {
    TEST_DESCRIPTION("Test for unnecessary rasterization due to using 0 for depthBiasConstantFactor and depthBiasSlopeFactor");

    InitBestPracticesFramework();
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_.depthBiasConstantFactor = 0.0f;
    pipe.rs_state_ci_.depthBiasSlopeFactor = 0.0f;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreatePipelines-depthbias-zero");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.rs_state_ci_.depthBiasEnable = VK_FALSE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreatePipelines-depthbias-zero");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkArmBestPracticesLayerTest, RobustBufferAccessTest) {
    TEST_DESCRIPTION("Test for appropriate warnings to be thrown when robustBufferAccess is enabled.");

    InitBestPracticesFramework();

    VkDevice local_device;
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = nullptr;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = nullptr;
    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkPhysicalDeviceFeatures supported_features;
    vk::GetPhysicalDeviceFeatures(this->gpu(), &supported_features);
    if (supported_features.robustBufferAccess) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-vkCreateDevice-RobustBufferAccess");
        VkPhysicalDeviceFeatures device_features = {};
        device_features.robustBufferAccess = VK_TRUE;
        dev_info.pEnabledFeatures = &device_features;
        vk::CreateDevice(this->gpu(), &dev_info, nullptr, &local_device);
        m_errorMonitor->VerifyFound();
    } else {
        printf("%s robustBufferAccess is not available, skipping test\n", kSkipPrefix);
        return;
    }
}

TEST_F(VkArmBestPracticesLayerTest, DepthPrePassUsage) {
    InitBestPracticesFramework();
    InitState();

    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test crashes on the NexusPlayer platform\n", kSkipPrefix);
        return;
    }

    InitRenderTarget();

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.pNext = nullptr;

    VkRenderPass rp = VK_NULL_HANDLE;
    vk::CreateRenderPass(m_device->device(), &rp_info, nullptr, &rp);

    // set up pipelines

    VkPipelineColorBlendAttachmentState color_write_off = {};
    VkPipelineColorBlendAttachmentState color_write_on = {};
    color_write_on.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo cb_depth_only_ci = {};
    cb_depth_only_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_depth_only_ci.attachmentCount = 1;
    cb_depth_only_ci.pAttachments = &color_write_off;

    VkPipelineColorBlendStateCreateInfo cb_depth_equal_ci = {};
    cb_depth_equal_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_depth_equal_ci.attachmentCount = 1;
    cb_depth_equal_ci.pAttachments = &color_write_on;

    VkPipelineDepthStencilStateCreateInfo ds_depth_only_ci = {};
    ds_depth_only_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_depth_only_ci.depthTestEnable = VK_TRUE;
    ds_depth_only_ci.depthWriteEnable = VK_TRUE;
    ds_depth_only_ci.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineDepthStencilStateCreateInfo ds_depth_equal_ci = {};
    ds_depth_equal_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_depth_equal_ci.depthTestEnable = VK_TRUE;
    ds_depth_equal_ci.depthWriteEnable = VK_FALSE;
    ds_depth_equal_ci.depthCompareOp = VK_COMPARE_OP_EQUAL;

    CreatePipelineHelper pipe_depth_only(*this);
    pipe_depth_only.InitInfo();
    pipe_depth_only.gp_ci_.pColorBlendState = &cb_depth_only_ci;
    pipe_depth_only.gp_ci_.pDepthStencilState = &ds_depth_only_ci;
    pipe_depth_only.InitState();
    pipe_depth_only.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_depth_equal(*this);
    pipe_depth_equal.InitInfo();
    pipe_depth_equal.gp_ci_.pColorBlendState = &cb_depth_equal_ci;
    pipe_depth_equal.gp_ci_.pDepthStencilState = &ds_depth_equal_ci;
    pipe_depth_equal.InitState();
    pipe_depth_equal.CreateGraphicsPipeline();

    // create a simple index buffer

    std::vector<uint32_t> indices = {};
    indices.resize(3);

    VkConstantBufferObj ibo(m_device, sizeof(uint32_t) * indices.size(), indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    m_commandBuffer->begin();
    m_commandBuffer->BindIndexBuffer(&ibo, 0, VK_INDEX_TYPE_UINT32);

    // record a command buffer which doesn't use enough depth pre-passes or geometry to matter
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_only.pipeline_);
    for (size_t i = 0; i < 30; i++) m_commandBuffer->DrawIndexed(indices.size(), 10, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_equal.pipeline_);
    for (size_t i = 0; i < 30; i++) m_commandBuffer->DrawIndexed(indices.size(), 10, 0, 0, 0);

    m_commandBuffer->EndRenderPass();

    m_errorMonitor->VerifyNotFound();

    // record a command buffer which records a significant number of depth pre-passes
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCmdEndRenderPass-depth-pre-pass-usage");

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_only.pipeline_);
    for (size_t i = 0; i < 30; i++) m_commandBuffer->DrawIndexed(indices.size(), 1000, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_equal.pipeline_);
    for (size_t i = 0; i < 30; i++) m_commandBuffer->DrawIndexed(indices.size(), 1000, 0, 0, 0);

    m_commandBuffer->EndRenderPass();

    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}
