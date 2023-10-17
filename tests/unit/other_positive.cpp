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
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_safe_struct.h"
#include "generated/vk_extension_helper.h"

#include <cstdlib>

TEST_F(VkPositiveLayerTest, StatelessValidationDisable) {
    TEST_DESCRIPTION("Specify a non-zero value for a reserved parameter with stateless validation disabled");

    VkValidationFeatureDisableEXT disables[] = {VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.disabledValidationFeatureCount = 1;
    features.pDisabledValidationFeatures = disables;
    RETURN_IF_SKIP(Init(nullptr, nullptr, &features));

    // Specify 0 for a reserved VkFlags parameter. Normally this is expected to trigger an stateless validation error, but this
    // validation was disabled via the features extension, so no errors should be forthcoming.
    VkEventCreateInfo event_info = vku::InitStructHelper();
    event_info.flags = 1;
    vkt::Event event(*m_device, event_info);
}

TEST_F(VkPositiveLayerTest, TestDestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    RETURN_IF_SKIP(Init())
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
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[3] = {};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 3, descriptor_sets);

    vk::FreeMemory(m_device->device(), VK_NULL_HANDLE, NULL);
}

TEST_F(VkPositiveLayerTest, Maintenance1Tests) {
    TEST_DESCRIPTION("Validate various special cases for the Maintenance1_KHR extension");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    vkt::CommandBuffer cmd_buf(m_device, m_commandPool);
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
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkDedicatedAllocationBufferCreateInfoNV dedicated_buffer_create_info = vku::InitStructHelper();
    dedicated_buffer_create_info.dedicatedAllocation = VK_TRUE;

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&dedicated_buffer_create_info);
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkMemoryRequirements memory_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &memory_reqs);

    VkDedicatedAllocationMemoryAllocateInfoNV dedicated_memory_info = vku::InitStructHelper();
    dedicated_memory_info.buffer = buffer;
    dedicated_memory_info.image = VK_NULL_HANDLE;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper(&dedicated_memory_info);
    memory_info.allocationSize = memory_reqs.size;

    bool pass;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    VkDeviceMemory buffer_memory;
    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &buffer_memory);
    ASSERT_EQ(VK_SUCCESS, err);

    err = vk::BindBufferMemory(m_device->device(), buffer, buffer_memory, 0);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    vk::FreeMemory(m_device->device(), buffer_memory, NULL);
}

TEST_F(VkPositiveLayerTest, SurfacelessQueryTest) {
    TEST_DESCRIPTION("Ensure affected API calls can be made with surfacless query extension");

    AddRequiredExtensions(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "VK_GOOGLE_surfaceless_query not supported on desktop";
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
    RETURN_IF_SKIP(InitFramework())

    VkResult err;

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);

    // We're not creating a valid m_device, but the phy wrapper is useful
    vkt::PhysicalDevice physical_device(gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo dev_info = vku::InitStructHelper(&features2);
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
    ASSERT_EQ(VK_SUCCESS, err);

    if (features2.features.samplerAnisotropy) {
        // Test that the parameter layer is caching the features correctly using CreateSampler
        VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
        // If the features were not captured correctly, this should cause an error
        sampler_ci.anisotropyEnable = VK_TRUE;
        sampler_ci.maxAnisotropy = physical_device.limits_.maxSamplerAnisotropy;

        VkSampler sampler = VK_NULL_HANDLE;
        err = vk::CreateSampler(device, &sampler_ci, nullptr, &sampler);
        ASSERT_EQ(VK_SUCCESS, err);
        vk::DestroySampler(device, sampler, nullptr);
    } else {
        printf("Feature samplerAnisotropy not enabled;  parameter_layer check skipped.\n");
    }

    // Verify the core validation layer has captured the physical device features by creating a a query pool.
    if (features2.features.pipelineStatisticsQuery) {
        VkQueryPool query_pool;
        VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
        qpci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        qpci.queryCount = 1;
        err = vk::CreateQueryPool(device, &qpci, nullptr, &query_pool);
        ASSERT_EQ(VK_SUCCESS, err);

        vk::DestroyQueryPool(device, query_pool, nullptr);
    } else {
        printf("Feature pipelineStatisticsQuery not enabled;  core_validation_layer check skipped.\n");
    }

    vk::DestroyDevice(device, nullptr);
}

TEST_F(VkPositiveLayerTest, ApiVersionZero) {
    TEST_DESCRIPTION("Check that apiVersion = 0 is valid.");
    app_info_.apiVersion = 0U;
    RETURN_IF_SKIP(InitFramework())
}

TEST_F(VkPositiveLayerTest, TestPhysicalDeviceSurfaceSupport) {
    TEST_DESCRIPTION("Test if physical device supports surface.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV shading = vku::InitStructHelper();
    shading.maxFragmentShadingRateInvocationCount = static_cast<VkSampleCountFlagBits>(0);
    VkPhysicalDeviceProperties2 props = vku::InitStructHelper(&shading);

    vk::GetPhysicalDeviceProperties2(gpu(), &props);
}

TEST_F(VkPositiveLayerTest, HostQueryResetSuccess) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT normally");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(host_query_reset_features);
    RETURN_IF_SKIP(InitState(nullptr, &host_query_reset_features));

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);
    vk::ResetQueryPoolEXT(m_device->device(), query_pool.handle(), 0, 1);
}

TEST_F(VkPositiveLayerTest, UseFirstQueueUnqueried) {
    TEST_DESCRIPTION("Use first queue family and one queue without first querying with vkGetPhysicalDeviceQueueFamilyProperties");

    RETURN_IF_SKIP(InitFramework())

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice test_device;
    vk::CreateDevice(gpu(), &device_ci, nullptr, &test_device);

    vk::DestroyDevice(test_device, nullptr);
}

// Android loader returns an error in this case
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
TEST_F(VkPositiveLayerTest, GetDevProcAddrNullPtr) {
    TEST_DESCRIPTION("Call GetDeviceProcAddr on an enabled instance extension expecting nullptr");
    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto fpDestroySurface = (PFN_vkCreateValidationCacheEXT)vk::GetDeviceProcAddr(m_device->device(), "vkDestroySurfaceKHR");
    if (fpDestroySurface) {
        m_errorMonitor->SetError("Null was expected!");
    }
}

TEST_F(VkPositiveLayerTest, GetDevProcAddrExtensions) {
    TEST_DESCRIPTION("Call GetDeviceProcAddr with and without extension enabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto vkTrimCommandPool = vk::GetDeviceProcAddr(m_device->device(), "vkTrimCommandPool");
    auto vkTrimCommandPoolKHR = vk::GetDeviceProcAddr(m_device->device(), "vkTrimCommandPoolKHR");
    if (nullptr == vkTrimCommandPool) m_errorMonitor->SetError("Unexpected null pointer");
    if (nullptr != vkTrimCommandPoolKHR) m_errorMonitor->SetError("Didn't receive expected null pointer");

    const char *const extension = {VK_KHR_MAINTENANCE_1_EXTENSION_NAME};
    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
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

TEST_F(VkPositiveLayerTest, Vulkan12FeaturesBufferDeviceAddress) {
    TEST_DESCRIPTION("Enable bufferDeviceAddress feature via Vulkan12features struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceBufferDeviceAddressFeatures bda_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(bda_features);
    if (!bda_features.bufferDeviceAddress) {
        GTEST_SKIP() << "Buffer Device Address feature not supported, skipping test";
    }

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.bufferDeviceAddress = true;
    features2.pNext = &features12;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       &alloc_flags);
    (void)buffer.address();

    // Also verify that we don't get the KHR extension address without enabling the KHR extension
    auto vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferDeviceAddressKHR");
    if (nullptr != vkGetBufferDeviceAddressKHR) m_errorMonitor->SetError("Didn't receive expected null pointer");
}

TEST_F(VkPositiveLayerTest, QueueThreading) {
    TEST_DESCRIPTION("Test concurrent Queue access from vkGet and vkSubmit");

    using namespace std::chrono;
    using std::thread;
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    // Test randomly fails with VK_TIMEOUT, most likely a driver bug
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Test does not run on AMD proprietary driver";
    }
    RETURN_IF_SKIP(InitState())

    const auto queue_family = m_device->graphics_queues()[0]->get_family_index();
    constexpr uint32_t queue_index = 0;
    vkt::CommandPool command_pool(*DeviceObj(), queue_family);

    const VkDevice device_h = device();
    VkQueue queue_h;
    vk::GetDeviceQueue(device(), queue_family, queue_index, &queue_h);
    vkt::Queue queue_o(queue_h, queue_family);

    const VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, command_pool.handle(),
                                              VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    vkt::CommandBuffer mock_cmdbuff(*DeviceObj(), cbai);
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
            VkSubmitInfo si = vku::InitStructHelper();
            si.commandBufferCount = 1;
            si.pCommandBuffers = &mock_cmdbuff.handle();
            queue_mutex.lock();
            ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(queue_h, 1, &si, VK_NULL_HANDLE));
            queue_mutex.unlock();
        }
    };

    const auto &testing_thread3 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            queue_mutex.lock();
            ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(queue_h));
            queue_mutex.unlock();
        }
    };

    std::array<thread, 3> threads = {thread(testing_thread1), thread(testing_thread2), thread(testing_thread3)};
    for (auto &t : threads) t.join();

    vk::QueueWaitIdle(queue_h);
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
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &test_instance));
    for (const char *instance_ext_name : m_instance_extension_names) {
        vk::InitInstanceExtension(test_instance, instance_ext_name);
    }

    ErrorMonitor monitor = ErrorMonitor(false);
    monitor.CreateCallback(test_instance);

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(test_instance, &physical_device_group_count, nullptr);
    std::vector<VkPhysicalDeviceGroupProperties> device_groups(physical_device_group_count,
                                                               vku::InitStruct<VkPhysicalDeviceGroupProperties>());
    vk::EnumeratePhysicalDeviceGroups(test_instance, &physical_device_group_count, device_groups.data());

    if (physical_device_group_count > 0) {
        VkPhysicalDevice physicalDevice = device_groups[0].physicalDevices[0];

        uint32_t queueFamilyPropertyCount = 0;
        vk::GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertyCount, nullptr);
    }

    monitor.DestroyCallback(test_instance);
    vk::DestroyInstance(test_instance, nullptr);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
TEST_F(VkPositiveLayerTest, ExportMetalObjects) {
    TEST_DESCRIPTION("Test vkExportMetalObjectsEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_METAL_OBJECTS_EXTENSION_NAME);

    // Initialize framework
    {
        VkExportMetalObjectCreateInfoEXT queue_info = vku::InitStructHelper();
        queue_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT;

        VkExportMetalObjectCreateInfoEXT metal_info = vku::InitStructHelper();
        metal_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT;
        metal_info.pNext = &queue_info;

        RETURN_IF_SKIP(InitFramework(&metal_info));

        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_features = vku::InitStructHelper();
        auto features2 = GetPhysicalDeviceFeatures2(portability_features);

        RETURN_IF_SKIP(InitState(nullptr, &features2))
    }

    const VkDevice device = m_device->device();

    // Get Metal Device and Metal Command Queue in 1 call
    {
        const VkQueue queue = m_default_queue;

        VkExportMetalCommandQueueInfoEXT queueInfo = vku::InitStructHelper();
        queueInfo.queue = queue;
        VkExportMetalDeviceInfoEXT deviceInfo = vku::InitStructHelper(&queueInfo);
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&deviceInfo);

        // This tests both device, queue, and pNext chaining
        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(deviceInfo.mtlDevice != nullptr);
        ASSERT_TRUE(queueInfo.mtlCommandQueue != nullptr);
    }

    // Get Metal Buffer
    {
        VkExportMetalObjectCreateInfoEXT metalBufferCreateInfo = vku::InitStructHelper();
        metalBufferCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT;

        VkMemoryAllocateInfo mem_info = vku::InitStructHelper();
        mem_info.allocationSize = 1024;
        mem_info.pNext = &metalBufferCreateInfo;

        VkDeviceMemory memory;
        const VkResult err = vk::AllocateMemory(device, &mem_info, NULL, &memory);
        ASSERT_EQ(VK_SUCCESS, err);

        VkExportMetalBufferInfoEXT bufferInfo = vku::InitStructHelper();
        bufferInfo.memory = memory;
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&bufferInfo);

        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(bufferInfo.mtlBuffer != nullptr);

        vk::FreeMemory(device, memory, nullptr);
    }

    // Get Metal Texture and Metal IOSurfaceRef
    {
        VkExportMetalObjectCreateInfoEXT metalSurfaceInfo = vku::InitStructHelper();
        metalSurfaceInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT;
        VkExportMetalObjectCreateInfoEXT metalTextureCreateInfo = vku::InitStructHelper(&metalSurfaceInfo);
        metalTextureCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;

        // Image contents don't matter
        VkImageCreateInfo ici = vku::InitStructHelper(&metalTextureCreateInfo);
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

        VkExportMetalIOSurfaceInfoEXT surfaceInfo = vku::InitStructHelper();
        surfaceInfo.image = image.handle();
        VkExportMetalTextureInfoEXT textureInfo = vku::InitStructHelper(&surfaceInfo);
        textureInfo.image = image.handle();
        textureInfo.plane = VK_IMAGE_ASPECT_PLANE_0_BIT;  // Image is not multi-planar
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&textureInfo);

        // This tests both texture, surface, and pNext chaining
        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(textureInfo.mtlTexture != nullptr);
        ASSERT_TRUE(surfaceInfo.ioSurface != nullptr);
    }

    // Get Metal Shared Event
    {
        VkExportMetalObjectCreateInfoEXT metalEventCreateInfo = vku::InitStructHelper();
        metalEventCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT;

        VkEventCreateInfo eventCreateInfo = vku::InitStructHelper(&metalEventCreateInfo);
        vkt::Event event(*m_device, eventCreateInfo);
        ASSERT_TRUE(event.initialized());

        VkExportMetalSharedEventInfoEXT eventInfo = vku::InitStructHelper();
        eventInfo.event = event.handle();
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&eventInfo);

        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(eventInfo.mtlSharedEvent != nullptr);
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT

TEST_F(VkPositiveLayerTest, ExtensionXmlDependsLogic) {
    TEST_DESCRIPTION("Make sure the OR in 'depends' from XML is observed correctly");
    // VK_KHR_buffer_device_address requires
    // (VK_KHR_get_physical_device_properties2 AND VK_KHR_device_group) OR VK_VERSION_1_1
    // If Vulkan 1.1 is not supported, should still be valid
    SetTargetApiVersion(VK_API_VERSION_1_0);
    if (!InstanceExtensionSupported(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required instance extensions";
    }
    m_instance_extension_names.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    if (!DeviceExtensionSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) ||
        !DeviceExtensionSupported(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required device extensions";
    }

    m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitState())
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5112
TEST_F(VkPositiveLayerTest, SafeVoidPointerCopies) {
    TEST_DESCRIPTION("Ensure valid deep copy of pData / dataSize combination structures");

    // safe_VkSpecializationInfo, constructor
    {
        std::vector<std::byte> data(20, std::byte{0b11110000});

        VkSpecializationInfo info = {};
        info.dataSize = size32(data);
        info.pData = data.data();

        safe_VkSpecializationInfo safe(&info);

        ASSERT_TRUE(safe.pData != info.pData);
        ASSERT_TRUE(safe.dataSize == info.dataSize);

        data.clear();  // Invalidate any references, pointers, or iterators referring to contained elements.

        auto copied_bytes = reinterpret_cast<const std::byte *>(safe.pData);
        ASSERT_TRUE(copied_bytes[19] == std::byte{0b11110000});
    }

    // safe_VkPipelineExecutableInternalRepresentationKHR, initialize
    {
        std::vector<std::byte> data(11, std::byte{0b01001001});

        VkPipelineExecutableInternalRepresentationKHR info = {};
        info.dataSize = size32(data);
        info.pData = data.data();

        safe_VkPipelineExecutableInternalRepresentationKHR safe;

        safe.initialize(&info);

        ASSERT_TRUE(safe.dataSize == info.dataSize);
        ASSERT_TRUE(safe.pData != info.pData);

        data.clear();  // Invalidate any references, pointers, or iterators referring to contained elements.

        auto copied_bytes = reinterpret_cast<const std::byte *>(safe.pData);
        ASSERT_TRUE(copied_bytes[10] == std::byte{0b01001001});
    }
}

TEST_F(VkPositiveLayerTest, FormatProperties3FromProfiles) {
    // https://github.com/KhronosGroup/Vulkan-Profiles/pull/392
    TEST_DESCRIPTION("Make sure VkFormatProperties3KHR is overwritten correctly in Profiles layer");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8_UNORM, &fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);
}

TEST_F(VkPositiveLayerTest, GDPAWithMultiCmdExt) {
    TEST_DESCRIPTION("Use GetDeviceProcAddr on a function which is provided by multiple extensions");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto vkCmdSetColorBlendAdvancedEXT = GetDeviceProcAddr<PFN_vkCmdSetColorBlendAdvancedEXT>("vkCmdSetColorBlendAdvancedEXT");
    ASSERT_NE(vkCmdSetColorBlendAdvancedEXT, nullptr);
}

TEST_F(VkPositiveLayerTest, UseInteractionApi) {
    TEST_DESCRIPTION("Use an API that is provided by multiple extensions");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto vkGetDeviceGroupPresentCapabilitiesKHR =
        GetDeviceProcAddr<PFN_vkGetDeviceGroupPresentCapabilitiesKHR>("vkGetDeviceGroupPresentCapabilitiesKHR");
    ASSERT_NE(vkGetDeviceGroupPresentCapabilitiesKHR, nullptr);

    VkDeviceGroupPresentCapabilitiesKHR device_group_present_caps = vku::InitStructHelper();
    vkGetDeviceGroupPresentCapabilitiesKHR(m_device->device(), &device_group_present_caps);
}

TEST_F(VkPositiveLayerTest, ExtensionExpressions) {
    TEST_DESCRIPTION(
        "Enable an extension (e.g., VK_KHR_fragment_shading_rate) that depends on multiple core versions _or_ regular extensions");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(fsr_features);
    if (!fsr_features.pipelineFragmentShadingRate) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::pipelineFragmentShadingRate not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &fsr_features));

    VkExtent2D fragment_size = {1, 1};
    std::array combiner_ops = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    vk::CmdSetFragmentShadingRateKHR(*m_commandBuffer, &fragment_size, combiner_ops.data());
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, AllowedDuplicateStype) {
    TEST_DESCRIPTION("Pass duplicate structs to whose vk.xml definition contains allowduplicate=true");

    VkInstance instance;

    VkInstanceCreateInfo ici = vku::InitStructHelper();
    ici.enabledLayerCount = instance_layers_.size();
    ici.ppEnabledLayerNames = instance_layers_.data();

    VkDebugUtilsMessengerCreateInfoEXT dbgUtils0 = vku::InitStructHelper();
    VkDebugUtilsMessengerCreateInfoEXT dbgUtils1 = vku::InitStructHelper(&dbgUtils0);
    ici.pNext = &dbgUtils1;

    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &instance));

    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(instance, nullptr));
}

TEST_F(VkPositiveLayerTest, ExtensionsInCreateInstance) {
    TEST_DESCRIPTION("Test to see if instance extensions are called during CreateInstance.");

    // See https://github.com/KhronosGroup/Vulkan-Loader/issues/537 for more details.
    // This is specifically meant to ensure a crash encountered in profiles does not occur, but also to
    // attempt to ensure that no extension calls have been added to CreateInstance hooks.
    // NOTE: it is certainly possible that a layer will call an extension during the Createinstance hook
    //       and the loader will _not_ crash (e.g., nvidia, android seem to not crash in this case, but AMD does).
    //       So, this test will only catch an erroneous extension _if_ run on HW/a driver that crashes in this use
    //       case.

    for (const auto &ext : InstanceExtensions::get_info_map()) {
        // Add all "real" instance extensions
        if (InstanceExtensionSupported(ext.first.c_str())) {
            bool version_required = false;
            for (const auto &req : ext.second.requirements) {
                std::string name(req.name);
                if (name.find("VK_VERSION") != std::string::npos) {
                    version_required = true;
                    break;
                }
            }
            if (!version_required) {
                m_instance_extension_names.emplace_back(ext.first.c_str());
            }
        }
    }

    RETURN_IF_SKIP(InitFramework())
}

TEST_F(VkPositiveLayerTest, CustomSafePNextCopy) {
    TEST_DESCRIPTION("Check passing custom data down the pNext chain for safe struct construction");

    // This tests an additional "copy_state" parameter in the SafePNextCopy function that allows "customizing" safe_* struct
    // construction.. This is required for structs such as VkPipelineRenderingCreateInfo (which extend VkGraphicsPipelineCreateInfo)
    // whose members must be partially ignored depending on the graphics sub-state present.

    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo pri = vku::InitStructHelper();
    pri.colorAttachmentCount = 1;
    pri.pColorAttachmentFormats = &format;

    bool ignore_default_construction = true;
    PNextCopyState copy_state = {
        [&ignore_default_construction](VkBaseOutStructure *safe_struct, const VkBaseOutStructure *in_struct) -> bool {
            if (ignore_default_construction) {
                auto tmp = reinterpret_cast<safe_VkPipelineRenderingCreateInfo *>(safe_struct);
                tmp->colorAttachmentCount = 0;
                tmp->pColorAttachmentFormats = nullptr;
                return true;
            }
            return false;
        },
    };

    {
        VkGraphicsPipelineCreateInfo gpci = vku::InitStructHelper(&pri);
        safe_VkGraphicsPipelineCreateInfo safe_gpci(&gpci, false, false, &copy_state);

        auto safe_pri = reinterpret_cast<const safe_VkPipelineRenderingCreateInfo *>(safe_gpci.pNext);
        // Ensure original input struct was not modified
        ASSERT_EQ(pri.colorAttachmentCount, 1);
        ASSERT_EQ(pri.pColorAttachmentFormats, &format);

        // Ensure safe struct was modified
        ASSERT_EQ(safe_pri->colorAttachmentCount, 0);
        ASSERT_EQ(safe_pri->pColorAttachmentFormats, nullptr);
    }

    // Ensure PNextCopyState::init is also applied when there is more than one element in the pNext chain
    {
        VkGraphicsPipelineLibraryCreateInfoEXT gpl_info = vku::InitStructHelper(&pri);
        VkGraphicsPipelineCreateInfo gpci = vku::InitStructHelper(&gpl_info);

        safe_VkGraphicsPipelineCreateInfo safe_gpci(&gpci, false, false, &copy_state);

        auto safe_gpl_info = reinterpret_cast<const safe_VkGraphicsPipelineLibraryCreateInfoEXT *>(safe_gpci.pNext);
        auto safe_pri = reinterpret_cast<const safe_VkPipelineRenderingCreateInfo *>(safe_gpl_info->pNext);
        // Ensure original input struct was not modified
        ASSERT_EQ(pri.colorAttachmentCount, 1);
        ASSERT_EQ(pri.pColorAttachmentFormats, &format);

        // Ensure safe struct was modified
        ASSERT_EQ(safe_pri->colorAttachmentCount, 0);
        ASSERT_EQ(safe_pri->pColorAttachmentFormats, nullptr);
    }

    // Check that signaling to use the default constructor works
    {
        pri.colorAttachmentCount = 1;
        pri.pColorAttachmentFormats = &format;

        ignore_default_construction = false;
        VkGraphicsPipelineCreateInfo gpci = vku::InitStructHelper(&pri);
        safe_VkGraphicsPipelineCreateInfo safe_gpci(&gpci, false, false, &copy_state);

        auto safe_pri = reinterpret_cast<const safe_VkPipelineRenderingCreateInfo *>(safe_gpci.pNext);
        // Ensure original input struct was not modified
        ASSERT_EQ(pri.colorAttachmentCount, 1);
        ASSERT_EQ(pri.pColorAttachmentFormats, &format);

        // Ensure safe struct was modified
        ASSERT_EQ(safe_pri->colorAttachmentCount, 1);
        ASSERT_EQ(*safe_pri->pColorAttachmentFormats, format);
    }
}

TEST_F(VkPositiveLayerTest, FreeCommandBuffersNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeCommandBuffers");

    RETURN_IF_SKIP(Init())

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkCommandBuffer free_command_buffers[2] = {command_buffer, VK_NULL_HANDLE};
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, &free_command_buffers[0]);
}

TEST_F(VkPositiveLayerTest, FreeDescriptorSetsNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeDescriptorSets");

    RETURN_IF_SKIP(Init())

    VkDescriptorPoolSize ds_type_count = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1};

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                nullptr};

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    // Only set first set, second is still null
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[0]);
    vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 2, descriptor_sets);
}

TEST_F(VkPositiveLayerTest, ExclusiveScissorVersionCount) {
    TEST_DESCRIPTION("Test using vkCmdSetExclusiveScissorEnableNV.");

    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    uint32_t propertyCount = 0u;
    vk::EnumerateDeviceExtensionProperties(gpu_, nullptr, &propertyCount, nullptr);
    std::vector<VkExtensionProperties> properties(propertyCount);
    vk::EnumerateDeviceExtensionProperties(gpu_, nullptr, &propertyCount, properties.data());
    bool exclusiveScissor2 = false;
    for (const auto &prop : properties) {
        if (strcmp(prop.extensionName, VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME) == 0) {
            if (prop.specVersion >= 2) {
                exclusiveScissor2 = true;
            }
            break;
        }
    }
    if (!exclusiveScissor2) {
        GTEST_SKIP() << VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME << " version 2 not supported";
    }
    RETURN_IF_SKIP(InitState())

    m_commandBuffer->begin();
    VkBool32 exclusiveScissorEnable = VK_TRUE;
    vk::CmdSetExclusiveScissorEnableNV(m_commandBuffer->handle(), 0u, 1u, &exclusiveScissorEnable);
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, GetCalibratedTimestamps) {
    TEST_DESCRIPTION("Basic usage of vkGetCalibratedTimestampsEXT.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    uint32_t count = 0;
    vk::GetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu(), &count, nullptr);
    if (count < 2) {
        GTEST_SKIP() << "only 1 TimeDomain supported";
    }
    std::vector<VkTimeDomainEXT> time_domains(count);
    vk::GetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu(), &count, time_domains.data());

    VkCalibratedTimestampInfoEXT timestamp_infos[2];
    timestamp_infos[0] = vku::InitStructHelper();
    timestamp_infos[0].timeDomain = time_domains[0];
    timestamp_infos[1] = vku::InitStructHelper();
    timestamp_infos[1].timeDomain = time_domains[1];

    uint64_t timestamps[2];
    uint64_t max_deviation;
    vk::GetCalibratedTimestampsEXT(device(), 2, timestamp_infos, timestamps, &max_deviation);
}
