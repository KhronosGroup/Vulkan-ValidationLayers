/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

TEST_F(PositiveRobustness, WriteDescriptorSetAccelerationStructureNVNullDescriptor) {
    TEST_DESCRIPTION("Validate using NV acceleration structure descriptor writing with null descriptor.");

    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(robustness2_features);
    if (robustness2_features.nullDescriptor != VK_TRUE) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_MISS_BIT_NV, nullptr},
                                     });

    VkAccelerationStructureNV top_level_as = VK_NULL_HANDLE;

    VkWriteDescriptorSetAccelerationStructureNV acc = LvlInitStruct<VkWriteDescriptorSetAccelerationStructureNV>();
    acc.accelerationStructureCount = 1;
    acc.pAccelerationStructures = &top_level_as;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>(&acc);
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);
}

TEST_F(PositiveRobustness, BindVertexBuffers2EXTNullDescriptors) {
    TEST_DESCRIPTION("Test nullDescriptor works wih CmdBindVertexBuffers variants");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(robustness2_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkBufferView buffer_view = VK_NULL_HANDLE;
    descriptor_set.WriteDescriptorBufferView(2, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.descriptor_writes.clear();

    m_commandBuffer->begin();
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer, &offset);
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer, &offset, nullptr, nullptr);
    m_commandBuffer->end();
}

TEST_F(PositiveRobustness, PipelineRobustnessRobustImageAccessExposed) {
    TEST_DESCRIPTION("Check if VK_EXT_image_robustness is exposed feature doesn't need to be enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    // 1.3 exposes VK_EXT_image_robustness and the feature for us
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto pipeline_robustness_features = LvlInitStruct<VkPhysicalDevicePipelineRobustnessFeaturesEXT>();
    GetPhysicalDeviceFeatures2(pipeline_robustness_features);
    if (!pipeline_robustness_features.pipelineRobustness) {
        GTEST_SKIP() << "pipelineRobustness is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pipeline_robustness_features));

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    auto pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;
    pipe.CreateComputePipeline();
}