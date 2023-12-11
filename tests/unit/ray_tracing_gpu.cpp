/*
 * Copyright (c) 2020-2022 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
 * Copyright (c) 2020-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_helper.h"
#include "../framework/gpu_av_helper.h"

TEST_F(NegativeGpuAVRayTracing, ArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders using gpu assited validation.");
    OOBRayTracingShadersTestBody(true);
}

TEST_F(NegativeGpuAVRayTracing, CmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Invalid parameters used in vkCmdTraceRaysIndirectKHR");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper();
    bda_features.bufferDeviceAddress = VK_TRUE;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper(&bda_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&ray_tracing_features);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2, &validation_features))

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    // Create ray tracing pipeline
    std::vector<VkDescriptorSetLayoutBinding> bindings(1);
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    const vkt::DescriptorSetLayout desc_set_layout(*m_device, bindings);
    OneOffDescriptorSet desc_set(m_device, bindings);

    const vkt::PipelineLayout rt_pipeline_layout(*m_device, {&desc_set_layout});

    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
    shader_stages[0] = vku::InitStructHelper();
    shader_stages[0].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    shader_stages[0].module = chit_shader.handle();
    shader_stages[0].pName = "main";

    shader_stages[1] = vku::InitStructHelper();
    shader_stages[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    shader_stages[1].module = rgen_shader.handle();
    shader_stages[1].pName = "main";

    std::array<VkRayTracingShaderGroupCreateInfoKHR, 1> shader_groups;
    shader_groups[0] = vku::InitStructHelper();
    shader_groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shader_groups[0].generalShader = 1;
    shader_groups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    shader_groups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    shader_groups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_ci = vku::InitStructHelper();
    raytracing_pipeline_ci.flags = 0;
    raytracing_pipeline_ci.stageCount = static_cast<uint32_t>(shader_stages.size());
    raytracing_pipeline_ci.pStages = shader_stages.data();
    raytracing_pipeline_ci.pGroups = shader_groups.data();
    raytracing_pipeline_ci.groupCount = shader_groups.size();
    raytracing_pipeline_ci.layout = rt_pipeline_layout;

    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;

    const VkResult result = vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                             &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
    ASSERT_EQ(VK_SUCCESS, result);

    // Create dummy shader binding table (SBT)
    vkt::Buffer sbt_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sbt_buffer.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), sbt_buffer.handle(), &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), sbt_buffer.handle(), mem.handle(), 0);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    const VkDeviceAddress sbt_address = sbt_buffer.address();

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = sbt_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    // Create and fill buffers storing indirect data (ray query dimensions)
    vkt::Buffer ray_query_dimensions_buffer_1(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    VkTraceRaysIndirectCommandKHR ray_query_dimensions{vvl::kU32Max, 1, 1};

    uint8_t *ray_query_dimensions_buffer_1_ptr = (uint8_t *)ray_query_dimensions_buffer_1.memory().map();
    std::memcpy(ray_query_dimensions_buffer_1_ptr, &ray_query_dimensions, sizeof(ray_query_dimensions));
    ray_query_dimensions_buffer_1.memory().unmap();

    ray_query_dimensions = {1, vvl::kU32Max, 1};

    vkt::Buffer ray_query_dimensions_buffer_2(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    uint8_t *ray_query_dimensions_buffer_2_ptr = (uint8_t *)ray_query_dimensions_buffer_2.memory().map();
    std::memcpy(ray_query_dimensions_buffer_2_ptr, &ray_query_dimensions, sizeof(ray_query_dimensions));
    ray_query_dimensions_buffer_2.memory().unmap();

    ray_query_dimensions = {1, 1, vvl::kU32Max};

    vkt::Buffer ray_query_dimensions_buffer_3(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    uint8_t *ray_query_dimensions_buffer_3_ptr = (uint8_t *)ray_query_dimensions_buffer_3.memory().map();
    std::memcpy(ray_query_dimensions_buffer_3_ptr, &ray_query_dimensions, sizeof(ray_query_dimensions));
    ray_query_dimensions_buffer_3.memory().unmap();

    // Trace rays
    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline_layout.handle(), 0, 1,
                              &desc_set.set_, 0, nullptr);

    vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion,
                                ray_query_dimensions_buffer_1.address());

    vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion,
                                ray_query_dimensions_buffer_2.address());

    vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion,
                                ray_query_dimensions_buffer_3.address());

    m_commandBuffer->end();

    if (uint64_t(physDevProps().limits.maxComputeWorkGroupCount[0]) * uint64_t(physDevProps().limits.maxComputeWorkGroupSize[0]) <
        uint64_t(vvl::kU32Max)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkTraceRaysIndirectCommandKHR-width-03638");
    }
    if (uint64_t(physDevProps().limits.maxComputeWorkGroupCount[1]) * uint64_t(physDevProps().limits.maxComputeWorkGroupSize[1]) <
        uint64_t(vvl::kU32Max)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkTraceRaysIndirectCommandKHR-height-03639");
    }
    if (uint64_t(physDevProps().limits.maxComputeWorkGroupCount[2]) * uint64_t(physDevProps().limits.maxComputeWorkGroupSize[2]) <
        uint64_t(vvl::kU32Max)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkTraceRaysIndirectCommandKHR-depth-03640");
    }
    m_commandBuffer->QueueCommandBuffer(true);
    m_errorMonitor->VerifyFound();

    vk::DeviceWaitIdle(m_device->handle());

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}
