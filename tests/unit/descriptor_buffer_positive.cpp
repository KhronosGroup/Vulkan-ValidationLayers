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

TEST_F(PositiveDescriptorBuffer, BindBufferAndSetOffset) {
    TEST_DESCRIPTION("Bind descriptor buffer and set descriptor offset.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    InitBasicDescriptorBuffer(&buffer_device_address_features);
    if (::testing::Test::IsSkipped()) return;

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    auto allocate_flag_info = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vk_testing::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

    auto buffer_binding_info = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
    buffer_binding_info.address = buffer.address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const VkDescriptorSetLayoutObj set_layout(m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&set_layout});

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_commandBuffer->begin();
    vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    m_commandBuffer->end();
}
