/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
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
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

//
// POSITIVE VALIDATION TESTS
//
// These tests do not expect to encounter ANY validation errors pass only if this is true

TEST_F(VkPositiveLayerTest, StatelessValidationDisable) {
    TEST_DESCRIPTION("Specify a non-zero value for a reserved parameter with stateless validation disabled");

    VkValidationFeatureDisableEXT disables[] = {VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.disabledValidationFeatureCount = 1;
    features.pDisabledValidationFeatures = disables;
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, pool_flags, &features));

    // Specify 0 for a reserved VkFlags parameter. Normally this is expected to trigger an stateless validation error, but this
    // validation was disabled via the features extension, so no errors should be forthcoming.
    VkEvent event_handle = VK_NULL_HANDLE;
    VkEventCreateInfo event_info = LvlInitStruct<VkEventCreateInfo>();
    event_info.flags = 1;
    vk::CreateEvent(device(), &event_info, NULL, &event_handle);
    vk::DestroyEvent(device(), event_handle, NULL);
}

// This is a positive test. No failures are expected.
TEST_F(VkPositiveLayerTest, TestDestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    ASSERT_NO_FATAL_FAILURE(Init());
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
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
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
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_VK_SUCCESS(err);
    vk::FreeDescriptorSets(m_device->device(), ds_pool, 3, descriptor_sets);
    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);

    vk::FreeMemory(m_device->device(), VK_NULL_HANDLE, NULL);
}

TEST_F(VkPositiveLayerTest, Maintenance1Tests) {
    TEST_DESCRIPTION("Validate various special cases for the Maintenance1_KHR extension");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkCommandBufferObj cmd_buf(m_device, m_commandPool);
    cmd_buf.begin();
    // Set Negative height, should give error if Maintenance 1 is not enabled
    VkViewport viewport = {0, 0, 16, -16, 0, 1};
    vk::CmdSetViewport(cmd_buf.handle(), 0, 1, &viewport);
    cmd_buf.end();
}

TEST_F(VkPositiveLayerTest, ValidStructPNext) {
    TEST_DESCRIPTION("Verify that a valid pNext value is handled correctly");

    // Positive test to check parameter_validation and unique_objects support for NV_dedicated_allocation
    AddRequiredExtensions(VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDedicatedAllocationBufferCreateInfoNV dedicated_buffer_create_info = LvlInitStruct<VkDedicatedAllocationBufferCreateInfoNV>();
    dedicated_buffer_create_info.dedicatedAllocation = VK_TRUE;

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>(&dedicated_buffer_create_info);
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &memory_reqs);

    VkDedicatedAllocationMemoryAllocateInfoNV dedicated_memory_info = LvlInitStruct<VkDedicatedAllocationMemoryAllocateInfoNV>();
    dedicated_memory_info.buffer = buffer;
    dedicated_memory_info.image = VK_NULL_HANDLE;

    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>(&dedicated_memory_info);
    memory_info.allocationSize = memory_reqs.size;

    bool pass;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    VkDeviceMemory buffer_memory;
    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &buffer_memory);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindBufferMemory(m_device->device(), buffer, buffer_memory, 0);
    ASSERT_VK_SUCCESS(err);

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    vk::FreeMemory(m_device->device(), buffer_memory, NULL);
}

TEST_F(VkPositiveLayerTest, SurfacelessQueryTest) {
    TEST_DESCRIPTION("Ensure affected API calls can be made with surfacless query extension");

    AddRequiredExtensions(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Use the VK_GOOGLE_surfaceless_query extension to query the available formats and
    // colorspaces by using a VK_NULL_HANDLE for the VkSurfaceKHR handle.
    uint32_t count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), VK_NULL_HANDLE, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(count);
    vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), VK_NULL_HANDLE, &count, surface_formats.data());

    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), VK_NULL_HANDLE, &count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), VK_NULL_HANDLE, &count, present_modes.data());
}

TEST_F(VkPositiveLayerTest, ParameterLayerFeatures2Capture) {
    TEST_DESCRIPTION("Ensure parameter_validation_layer correctly captures physical device features");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkResult err;

    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>();
    GetPhysicalDeviceFeatures2(features2);

    // We're not creating a valid m_device, but the phy wrapper is useful
    vk_testing::PhysicalDevice physical_device(gpu());
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo dev_info = LvlInitStruct<VkDeviceCreateInfo>(&features2);
    dev_info.flags = 0;
    dev_info.queueCreateInfoCount = create_queue_infos.size();
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = 0;
    dev_info.ppEnabledExtensionNames = nullptr;
    dev_info.pEnabledFeatures = nullptr;

    VkDevice device;
    err = vk::CreateDevice(gpu(), &dev_info, nullptr, &device);
    ASSERT_VK_SUCCESS(err);

    if (features2.features.samplerAnisotropy) {
        // Test that the parameter layer is caching the features correctly using CreateSampler
        VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
        // If the features were not captured correctly, this should cause an error
        sampler_ci.anisotropyEnable = VK_TRUE;
        sampler_ci.maxAnisotropy = physical_device.properties().limits.maxSamplerAnisotropy;

        VkSampler sampler = VK_NULL_HANDLE;
        err = vk::CreateSampler(device, &sampler_ci, nullptr, &sampler);
        ASSERT_VK_SUCCESS(err);
        vk::DestroySampler(device, sampler, nullptr);
    } else {
        printf("Feature samplerAnisotropy not enabled;  parameter_layer check skipped.\n");
    }

    // Verify the core validation layer has captured the physical device features by creating a a query pool.
    if (features2.features.pipelineStatisticsQuery) {
        VkQueryPool query_pool;
        VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
        qpci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        qpci.queryCount = 1;
        err = vk::CreateQueryPool(device, &qpci, nullptr, &query_pool);
        ASSERT_VK_SUCCESS(err);

        vk::DestroyQueryPool(device, query_pool, nullptr);
    } else {
        printf("Feature pipelineStatisticsQuery not enabled;  core_validation_layer check skipped.\n");
    }

    vk::DestroyDevice(device, nullptr);
}

TEST_F(VkPositiveLayerTest, ApiVersionZero) {
    TEST_DESCRIPTION("Check that apiVersion = 0 is valid.");
    app_info_.apiVersion = 0U;
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
}

TEST_F(VkPositiveLayerTest, TestPhysicalDeviceSurfaceSupport) {
    TEST_DESCRIPTION("Test if physical device supports surface.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), 0, m_surface, &supported);

    if (supported) {
        uint32_t count;
        vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &count, nullptr);
    }
}

TEST_F(VkPositiveLayerTest, ModifyPnext) {
    TEST_DESCRIPTION("Make sure invalid values in pNext structures are ignored at query time");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_NV_FRAGMENT_SHADING_RATE_ENUMS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto shading = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV>();
    shading.maxFragmentShadingRateInvocationCount = static_cast<VkSampleCountFlagBits>(0);
    auto props = LvlInitStruct<VkPhysicalDeviceProperties2>(&shading);

    vk::GetPhysicalDeviceProperties2(gpu(), &props);
}

TEST_F(VkPositiveLayerTest, HostQueryResetSuccess) {
    // This is a positive test. No failures are expected.
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT normally");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkPositiveLayerTest, UseFirstQueueUnqueried) {
    TEST_DESCRIPTION("Use first queue family and one queue without first querying with vkGetPhysicalDeviceQueueFamilyProperties");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice test_device;
    vk::CreateDevice(gpu(), &device_ci, nullptr, &test_device);

    vk::DestroyDevice(test_device, nullptr);
}

// Android loader returns an error in this case
#if !defined(ANDROID)
TEST_F(VkPositiveLayerTest, GetDevProcAddrNullPtr) {
    TEST_DESCRIPTION("Call GetDeviceProcAddr on an enabled instance extension expecting nullptr");
    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpDestroySurface = (PFN_vkCreateValidationCacheEXT)vk::GetDeviceProcAddr(m_device->device(), "vkDestroySurfaceKHR");
    if (fpDestroySurface) {
        m_errorMonitor->SetError("Null was expected!");
    }
}

TEST_F(VkPositiveLayerTest, GetDevProcAddrExtensions) {
    TEST_DESCRIPTION("Call GetDeviceProcAddr with and without extension enabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkTrimCommandPool = vk::GetDeviceProcAddr(m_device->device(), "vkTrimCommandPool");
    auto vkTrimCommandPoolKHR = vk::GetDeviceProcAddr(m_device->device(), "vkTrimCommandPoolKHR");
    if (nullptr == vkTrimCommandPool) m_errorMonitor->SetError("Unexpected null pointer");
    if (nullptr != vkTrimCommandPoolKHR) m_errorMonitor->SetError("Didn't receive expected null pointer");

    const char *const extension = {VK_KHR_MAINTENANCE_1_EXTENSION_NAME};
    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.enabledExtensionCount = 1;
    device_ci.ppEnabledExtensionNames = &extension;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice device;
    vk::CreateDevice(gpu(), &device_ci, NULL, &device);

    vkTrimCommandPoolKHR = vk::GetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
    if (nullptr == vkTrimCommandPoolKHR) m_errorMonitor->SetError("Unexpected null pointer");
    vk::DestroyDevice(device, nullptr);
}
#endif

TEST_F(VkPositiveLayerTest, Vulkan12Features) {
    TEST_DESCRIPTION("Enable feature via Vulkan12features struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(bda_features);
    if (!bda_features.bufferDeviceAddress) {
        printf("Buffer Device Address feature not supported, skipping test\n");
        return;
    }

    VkPhysicalDeviceVulkan12Features features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    features12.bufferDeviceAddress = true;
    features2.pNext = &features12;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    bci.size = 8;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBuffer buffer;
    vk::CreateBuffer(m_device->device(), &bci, NULL, &buffer);
    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkMemoryAllocateFlagsInfo alloc_flags = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;
    VkDeviceMemory buffer_mem;
    VkResult err = vk::AllocateMemory(m_device->device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    VkBufferDeviceAddressInfo bda_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    bda_info.buffer = buffer;
    auto vkGetBufferDeviceAddress =
        (PFN_vkGetBufferDeviceAddress)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferDeviceAddress");
    ASSERT_TRUE(vkGetBufferDeviceAddress != nullptr);
    vkGetBufferDeviceAddress(m_device->device(), &bda_info);

    // Also verify that we don't get the KHR extension address without enabling the KHR extension
    auto vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferDeviceAddressKHR");
    if (nullptr != vkGetBufferDeviceAddressKHR) m_errorMonitor->SetError("Didn't receive expected null pointer");
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
}

TEST_F(VkPositiveLayerTest, QueueThreading) {
    TEST_DESCRIPTION("Test concurrent Queue access from vkGet and vkSubmit");

    using namespace std::chrono;
    using std::thread;
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    // Test randomly fails with VK_TIMEOUT, most likely a driver bug
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Test does not run on AMD proprietary driver";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto queue_family = DeviceObj()->GetDefaultQueue()->get_family_index();
    constexpr uint32_t queue_index = 0;
    VkCommandPoolObj command_pool(DeviceObj(), queue_family);

    const VkDevice device_h = device();
    VkQueue queue_h;
    vk::GetDeviceQueue(device(), queue_family, queue_index, &queue_h);
    VkQueueObj queue_o(queue_h, queue_family);

    const VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, command_pool.handle(),
                                              VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    vk_testing::CommandBuffer mock_cmdbuff(*DeviceObj(), cbai);
    const VkCommandBufferBeginInfo cbbi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
    mock_cmdbuff.begin(&cbbi);
    mock_cmdbuff.end();

    std::mutex queue_mutex;

    constexpr auto test_duration = seconds{2};
    const auto timer_begin = steady_clock::now();

    const auto &testing_thread1 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            VkQueue dummy_q;
            vk::GetDeviceQueue(device_h, queue_family, queue_index, &dummy_q);
        }
    };

    const auto &testing_thread2 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            VkSubmitInfo si = LvlInitStruct<VkSubmitInfo>();
            si.commandBufferCount = 1;
            si.pCommandBuffers = &mock_cmdbuff.handle();
            queue_mutex.lock();
            ASSERT_VK_SUCCESS(vk::QueueSubmit(queue_h, 1, &si, VK_NULL_HANDLE));
            queue_mutex.unlock();
        }
    };

    const auto &testing_thread3 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            queue_mutex.lock();
            ASSERT_VK_SUCCESS(vk::QueueWaitIdle(queue_h));
            queue_mutex.unlock();
        }
    };

    std::array<thread, 3> threads = {thread(testing_thread1), thread(testing_thread2), thread(testing_thread3)};
    for (auto &t : threads) t.join();

    vk::QueueWaitIdle(queue_h);
}

TEST_F(VkPositiveLayerTest, TestAcquiringSwapchainImages) {
    TEST_DESCRIPTION("Test acquiring swapchain images.");

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore acquire_semaphore;
    VkSemaphore submit_semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &acquire_semaphore));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &submit_semaphore));

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

    m_commandBuffer->begin();

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->end();

    VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &acquire_semaphore;
    submit_info.pWaitDstStageMask = &stage_mask;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &submit_semaphore;

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    VkPresentInfoKHR present = LvlInitStruct<VkPresentInfoKHR>();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    vk::QueuePresentKHR(m_device->m_queue, &present);

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroySemaphore(device(), submit_semaphore, nullptr);
    vk::DestroySemaphore(device(), acquire_semaphore, nullptr);
}

TEST_F(VkPositiveLayerTest, TestSwapchainImageFenceWait) {
    TEST_DESCRIPTION("Test waiting on swapchain image with a fence.");

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    VkFenceObj fence;
    fence.init(*m_device, VkFenceObj::create_info());
    VkFence fence_handle = fence.handle();

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence_handle, &image_index);
    vk::WaitForFences(device(), 1, &fence_handle, VK_TRUE, kWaitTimeout);

    m_commandBuffer->begin();

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->end();

    VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.pWaitDstStageMask = &stage_mask;

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    VkPresentInfoKHR present = LvlInitStruct<VkPresentInfoKHR>();
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    vk::QueuePresentKHR(m_device->m_queue, &present);

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkPositiveLayerTest, EnumeratePhysicalDeviceGroups) {
    TEST_DESCRIPTION("Test using VkPhysicalDevice handles obtained with vkEnumeratePhysicalDeviceGroups");

#ifdef __linux__
    if (std::getenv("NODEVICE_SELECT") == nullptr)
    {
        // Currently due to a bug in MESA this test will fail.
        // https://gitlab.freedesktop.org/mesa/mesa/-/commit/4588453815c58ec848b0ff6f18a08836e70f55df
        //
        // It's fixed as of v22.7.1:
        // https://gitlab.freedesktop.org/mesa/mesa/-/tree/mesa-22.1.7/src/vulkan/device-select-layer
        //
        // To avoid impacting local users, skip this TEST unless NODEVICE_SELECT is specified.
        // NODEVICE_SELECT enables/disables the implicit mesa layer which has illegal code:
        // https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/src/vulkan/device-select-layer/VkLayer_MESA_device_select.json
        GTEST_SKIP();
    }
#endif

    SetTargetApiVersion(VK_API_VERSION_1_1);

    auto ici = GetInstanceCreateInfo();

    VkInstance test_instance = VK_NULL_HANDLE;
    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &test_instance));
    DebugReporter debug_reporter;
    debug_reporter.Create(test_instance);

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(test_instance, &physical_device_group_count, nullptr);
    std::vector<VkPhysicalDeviceGroupProperties> device_groups(physical_device_group_count,
                                                               LvlInitStruct<VkPhysicalDeviceGroupProperties>());
    vk::EnumeratePhysicalDeviceGroups(test_instance, &physical_device_group_count, device_groups.data());

    if (physical_device_group_count > 0) {
        VkPhysicalDevice physicalDevice = device_groups[0].physicalDevices[0];

        uint32_t queueFamilyPropertyCount = 0;
        vk::GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertyCount, nullptr);
    }

    debug_reporter.Destroy(test_instance);
    vk::DestroyInstance(test_instance, nullptr);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
TEST_F(VkPositiveLayerTest, ExportMetalObjects) {
    TEST_DESCRIPTION("Test vkExportMetalObjectsEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_METAL_OBJECTS_EXTENSION_NAME);

    // Initialize framework
    {
        auto queue_info = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
        queue_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT;

        auto metal_info = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
        metal_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT;
        metal_info.pNext = &queue_info;

        ASSERT_NO_FATAL_FAILURE(InitFramework(nullptr, &metal_info));
        if (!AreRequiredExtensionsEnabled()) {
            GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
        }

        auto portability_features = LvlInitStruct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
        auto features2 = GetPhysicalDeviceFeatures2(portability_features);

        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    }

    auto vkExportMetalObjectsEXT =
        reinterpret_cast<PFN_vkExportMetalObjectsEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkExportMetalObjectsEXT"));
    ASSERT_TRUE(vkExportMetalObjectsEXT != nullptr);

    const VkDevice device = m_device->device();

    // Get Metal Device and Metal Command Queue in 1 call
    {
        const VkQueue queue = m_device->m_queue;

        auto queueInfo = LvlInitStruct<VkExportMetalCommandQueueInfoEXT>();
        queueInfo.queue = queue;
        auto deviceInfo = LvlInitStruct<VkExportMetalDeviceInfoEXT>(&queueInfo);
        auto objectsInfo = LvlInitStruct<VkExportMetalObjectsInfoEXT>(&deviceInfo);

        // This tests both device, queue, and pNext chaining
        vkExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(deviceInfo.mtlDevice != nullptr);
        ASSERT_TRUE(queueInfo.mtlCommandQueue != nullptr);
    }

    // Get Metal Buffer
    {
        auto metalBufferCreateInfo = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
        metalBufferCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT;

        VkMemoryAllocateInfo mem_info = LvlInitStruct<VkMemoryAllocateInfo>();
        mem_info.allocationSize = 1024;
        mem_info.pNext = &metalBufferCreateInfo;

        VkDeviceMemory memory;
        const VkResult err = vk::AllocateMemory(device, &mem_info, NULL, &memory);
        ASSERT_VK_SUCCESS(err);

        auto bufferInfo = LvlInitStruct<VkExportMetalBufferInfoEXT>();
        bufferInfo.memory = memory;
        auto objectsInfo = LvlInitStruct<VkExportMetalObjectsInfoEXT>(&bufferInfo);

        vkExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(bufferInfo.mtlBuffer != nullptr);

        vk::FreeMemory(device, memory, nullptr);
    }

    // Get Metal Texture and Metal IOSurfaceRef
    {
        auto metalSurfaceInfo = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
        metalSurfaceInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT;
        auto metalTextureCreateInfo = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>(&metalSurfaceInfo);
        metalTextureCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;

        // Image contents don't matter
        VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&metalTextureCreateInfo);
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = VK_FORMAT_B8G8R8A8_UNORM;
        ici.extent = {32, 32, 1};
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_LINEAR;
        ici.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkImageObj image(m_device);
        image.Init(ici);
        ASSERT_TRUE(image.initialized());

        auto surfaceInfo = LvlInitStruct<VkExportMetalIOSurfaceInfoEXT>();
        surfaceInfo.image = image.handle();
        auto textureInfo = LvlInitStruct<VkExportMetalTextureInfoEXT>(&surfaceInfo);
        textureInfo.image = image.handle();
        textureInfo.plane = VK_IMAGE_ASPECT_PLANE_0_BIT;  // Image is not multi-planar
        auto objectsInfo = LvlInitStruct<VkExportMetalObjectsInfoEXT>(&textureInfo);

        // This tests both texture, surface, and pNext chaining
        vkExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(textureInfo.mtlTexture != nullptr);
        ASSERT_TRUE(surfaceInfo.ioSurface != nullptr);
    }

    // Get Metal Shared Event
    {
        auto metalEventCreateInfo = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
        metalEventCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT;

        auto eventCreateInfo = LvlInitStruct<VkEventCreateInfo>(&metalEventCreateInfo);
        VkEventObj event(*m_device, eventCreateInfo);
        ASSERT_TRUE(event.initialized());

        auto eventInfo = LvlInitStruct<VkExportMetalSharedEventInfoEXT>();
        eventInfo.event = event.handle();
        auto objectsInfo = LvlInitStruct<VkExportMetalObjectsInfoEXT>(&eventInfo);

        vkExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(eventInfo.mtlSharedEvent != nullptr);
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT
