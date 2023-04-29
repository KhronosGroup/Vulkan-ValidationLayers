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

class PositiveDescriptorBuffer : public VkLayerTest {};

TEST_F(PositiveDescriptorBuffer, BasicUsage) {
    TEST_DESCRIPTION("Create VkBuffer with extension.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    // descriptorBuffer guaranteed to be enabled if extension is
    auto descriptor_buffer_features = LvlInitStruct<VkPhysicalDeviceDescriptorBufferFeaturesEXT>();
    GetPhysicalDeviceFeatures2(descriptor_buffer_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &descriptor_buffer_features));

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