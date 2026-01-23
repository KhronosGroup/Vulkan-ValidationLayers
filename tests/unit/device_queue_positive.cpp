/*
 * Copyright (c) 2023-2026 Valve Corporation
 * Copyright (c) 2023-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/shader_helper.h"

class PositiveDeviceQueue : public VkLayerTest {};

TEST_F(PositiveDeviceQueue, NoQueues) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    RETURN_IF_SKIP(InitFramework());

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.pNext = requested_features_.GetEnabledFeatures2();
    device_ci.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    vkt::Device device(gpu_, device_ci);

    VkShaderObj cs(device, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(device, bindings);
    const vkt::PipelineLayout pipeline_layout(device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout;

    VkPipeline pipeline;
    vk::CreateComputePipelines(device, VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &pipeline);
    vk::DestroyPipeline(device, pipeline, nullptr);
}

TEST_F(PositiveDeviceQueue, QueueFamilyIndexDifferentCreateFlags) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_INTERNALLY_SYNCHRONIZED_QUEUES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::internallySynchronizedQueues);
    RETURN_IF_SKIP(InitFramework());

    bool found_queue = false;
    VkQueueFamilyProperties queue_properties;
    uint32_t queue_family_index = 0u;
    uint32_t queue_family_count = 0u;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        if (queue_families[i].queueCount > 1) {
            found_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (found_queue == false) {
        GTEST_SKIP() << "test requires queue family with 2 queues, not available";
    }

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_ci[2];
    device_queue_ci[0] = vku::InitStructHelper();
    device_queue_ci[0].flags = 0u;
    device_queue_ci[0].queueFamilyIndex = queue_family_index;
    device_queue_ci[0].queueCount = 1u;
    device_queue_ci[0].pQueuePriorities = &queue_priority;
    device_queue_ci[1] = vku::InitStructHelper();
    device_queue_ci[1].flags = VK_DEVICE_QUEUE_CREATE_INTERNALLY_SYNCHRONIZED_BIT_KHR;
    device_queue_ci[1].queueFamilyIndex = queue_family_index;
    device_queue_ci[1].queueCount = 1u;
    device_queue_ci[1].pQueuePriorities = &queue_priority;

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 2u;
    device_ci.pQueueCreateInfos = device_queue_ci;
    device_ci.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &test_device);
    vk::DestroyDevice(test_device, nullptr);
}
