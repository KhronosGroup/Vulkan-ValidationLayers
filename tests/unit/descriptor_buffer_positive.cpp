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

void DescriptorBufferTest::InitBasicDescriptorBuffer(void* pNextFeatures) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < m_attempted_api_version) {
        GTEST_SKIP() << "At least Vulkan version 1." << m_attempted_api_version.minor() << " is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto descriptor_buffer_features = LvlInitStruct<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(pNextFeatures);
    GetPhysicalDeviceFeatures2(descriptor_buffer_features);
    if (!descriptor_buffer_features.descriptorBuffer) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBuffer , skipping.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &descriptor_buffer_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
}

TEST_F(PositiveDescriptorBuffer, BasicUsage) {
    TEST_DESCRIPTION("Create VkBuffer with extension.");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    // *descriptorBufferAddressSpaceSize properties are guaranteed to be 2^27
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;

    {
        buffer_ci.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        vk_testing::Buffer buffer(*m_device, buffer_ci);
    }

    {
        buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        vk_testing::Buffer buffer(*m_device, buffer_ci);
    }

    {
        buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                          VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        vk_testing::Buffer buffer(*m_device, buffer_ci);
    }
}