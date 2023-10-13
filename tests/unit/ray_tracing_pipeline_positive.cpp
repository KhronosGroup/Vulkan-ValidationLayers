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
#include "../framework/ray_tracing_nv.h"
#include "../framework/pipeline_helper.h"

TEST_F(PositiveRayTracingPipeline, ShaderGroupsKHR) {
    TEST_DESCRIPTION("Test that no warning is produced when a library is referenced in the raytracing shader groups.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true))

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(ray_tracing_features);
    RETURN_IF_SKIP(InitState(nullptr, &ray_tracing_features));

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipeline pipeline = VK_NULL_HANDLE;

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = vku::InitStructHelper();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
    stage_create_infos[0] = vku::InitStructHelper();
    stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_infos[0].module = rgen_shader.handle();
    stage_create_infos[0].pName = "main";

    stage_create_infos[1] = vku::InitStructHelper();
    stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_infos[1].module = chit_shader.handle();
    stage_create_infos[1].pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
    group_create_infos[0] = vku::InitStructHelper();
    group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_infos[0].generalShader = 0;
    group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    group_create_infos[1] = vku::InitStructHelper();
    group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to the closest hit shader from the library
    group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 2;
    pipeline_ci.pStages = stage_create_infos;
    pipeline_ci.groupCount = 2;
    pipeline_ci.pGroups = group_create_infos;
    pipeline_ci.layout = empty_pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkResult err =
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_NE(pipeline, VK_NULL_HANDLE);

    vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    vk::DestroyPipeline(m_device->handle(), library, nullptr);
}

TEST_F(PositiveRayTracingPipeline, CacheControl) {
    TEST_DESCRIPTION("Create ray tracing pipeline with VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true))

    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper();
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper(&features13);
    GetPhysicalDeviceFeatures2(ray_tracing_features);
    RETURN_IF_SKIP(InitState(nullptr, &ray_tracing_features));

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);
    vk::DestroyPipeline(device(), library, nullptr);
}

TEST_F(PositiveRayTracingPipelineNV, BasicUsage) {
    TEST_DESCRIPTION("Test VK_NV_ray_tracing.");
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))

    VkPhysicalDeviceRayTracingPropertiesNV rtnv_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rtnv_props);
    if (rtnv_props.maxDescriptorSetAccelerationStructures < 1) {
        GTEST_SKIP() << "VkPhysicalDeviceRayTracingPropertiesNV::maxDescriptorSetAccelerationStructures < 1";
    }

    RETURN_IF_SKIP(InitState())

    auto ignore_update = [](RayTracingPipelineHelper &helper) {};
    RayTracingPipelineHelper::OneshotPositiveTest(*this, ignore_update);
}

TEST_F(PositiveRayTracingPipeline, GetCaptureReplayShaderGroupHandlesKHR) {
    TEST_DESCRIPTION(
        "Regression test for issue 6282: make sure that when validating vkGetRayTracingCaptureReplayShaderGroupHandlesKHR on a "
        "pipeline created using pipeline libraries, the total shader group count is computed using info from the libraries.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt_pipeline_features = vku::InitStructHelper();
    rt_pipeline_features.rayTracingPipelineShaderGroupHandleCaptureReplay = VK_TRUE;
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper(&rt_pipeline_features);
    gpl_features.graphicsPipelineLibrary = VK_TRUE;
    VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT pipeline_group_handle_features = vku::InitStructHelper(&gpl_features);
    pipeline_group_handle_features.pipelineLibraryGroupHandles = VK_TRUE;
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&pipeline_group_handle_features);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))

    features2 = GetPhysicalDeviceFeatures2(pipeline_group_handle_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "graphicsPipelineLibrary feature not supported, skipping test";
    }
    if (!pipeline_group_handle_features.pipelineLibraryGroupHandles) {
        GTEST_SKIP() << "pipelineLibraryGroupHandles feature not supported, skipping test";
    }
    if (!rt_pipeline_features.rayTracingPipelineShaderGroupHandleCaptureReplay) {
        GTEST_SKIP() << "rayTracingShaderGroupHandleCaptureReplay not supported, skipping test";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    RayTracingPipelineHelper rt_pipeline_lib(*this);
    rt_pipeline_lib.InitLibraryInfoKHR(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipeline_lib.InitState();
    ASSERT_EQ(VK_SUCCESS, rt_pipeline_lib.CreateKHRRayTracingPipeline());

    RayTracingPipelineHelper rt_pipe(*this);
    rt_pipe.InitLibraryInfoKHR(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe.rp_ci_KHR_.stageCount = 0;
    rt_pipe.rp_ci_KHR_.groupCount = 0;
    rt_pipe.AddLibrary(rt_pipeline_lib);
    rt_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, rt_pipe.CreateKHRRayTracingPipeline());

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, buf_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);

    // dataSize must be at least groupCount * VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleCaptureReplaySize
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 0, 3,
                                                        3 * ray_tracing_properties.shaderGroupHandleCaptureReplaySize, &buffer);
}
