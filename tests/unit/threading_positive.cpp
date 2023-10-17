/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"

#if GTEST_IS_THREADSAFE
TEST_F(PositiveThreading, DisplayObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPropertiesKHR in thread-safety.");

    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPropertiesKHR(gpu(), &prop_count, nullptr);
    if (prop_count == 0) {
        GTEST_SKIP() << "No VkDisplayKHR properties to query";
    }

    std::vector<VkDisplayPropertiesKHR> display_props{prop_count};
    // Create a VkDisplayKHR object
    vk::GetPhysicalDeviceDisplayPropertiesKHR(gpu(), &prop_count, display_props.data());
    ASSERT_NE(prop_count, 0U);

    // Now use this new object in an API call that thread safety will track
    prop_count = 0;
    vk::GetDisplayModePropertiesKHR(gpu(), display_props[0].display, &prop_count, nullptr);
}

TEST_F(PositiveThreading, DisplayPlaneObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPlanePropertiesKHR in thread-safety.");

    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &prop_count, nullptr);
    if (prop_count != 0) {
        // only grab first plane property
        prop_count = 1;
        VkDisplayPlanePropertiesKHR display_plane_props = {};
        // Create a VkDisplayKHR object
        vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &prop_count, &display_plane_props);
        // Now use this new object in an API call
        prop_count = 0;
        vk::GetDisplayModePropertiesKHR(gpu(), display_plane_props.currentDisplay, &prop_count, nullptr);
    }
}

TEST_F(PositiveThreading, UpdateDescriptorUpdateAfterBindNoCollision) {
    TEST_DESCRIPTION("Two threads updating the same UAB descriptor set, expected not to generate a threading error");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    // Create a device that enables descriptorBindingStorageBufferUpdateAfterBind
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);
    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    std::array<VkDescriptorBindingFlagsEXT, 2> flags = {
        {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT}};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = (uint32_t)flags.size();
    flags_create_info.pBindingFlags = flags.data();

    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              },
                                              VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &flags_create_info,
                                              VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

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

TEST_F(PositiveThreading, NullFenceCollision) {
    RETURN_IF_SKIP(Init())

    ThreadTestData data;
    data.device = m_device->device();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Call vk::DestroyFence of VK_NULL_HANDLE repeatedly using multiple threads.
    // There should be no validation error from collision of that non-object.
    std::thread thread(ReleaseNullFence, &data);
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(m_device->device(), VK_NULL_HANDLE, NULL);
    }
    thread.join();

    m_errorMonitor->SetBailout(NULL);
}
#endif  // GTEST_IS_THREADSAFE
