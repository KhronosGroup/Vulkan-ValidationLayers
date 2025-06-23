/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

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