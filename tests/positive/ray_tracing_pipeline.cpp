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
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "../layer_validation_tests.h"

TEST_F(VkPositiveLayerTest, RayTracingPipelineShaderGroupsKHR) {
    TEST_DESCRIPTION("Test that no warning is produced when a library is referenced in the raytracing shader groups.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");
    const auto vkDestroyPipeline = GetInstanceProcAddr<PFN_vkDestroyPipeline>("vkDestroyPipeline");

    VkPipeline pipeline = VK_NULL_HANDLE;

    const VkPipelineLayoutObj pipeline_layout(m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = LvlInitStruct<VkRayTracingPipelineInterfaceCreateInfoKHR>();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
    stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_infos[0].module = rgen_shader.handle();
    stage_create_infos[0].pName = "main";

    stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_infos[1].module = chit_shader.handle();
    stage_create_infos[1].pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
    group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_infos[0].generalShader = 0;
    group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to the closest hit shader from the library
    group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 2;
    pipeline_ci.pStages = stage_create_infos;
    pipeline_ci.groupCount = 2;
    pipeline_ci.pGroups = group_create_infos;
    pipeline_ci.layout = empty_pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkResult err =
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_VK_SUCCESS(err);
    ASSERT_NE(pipeline, VK_NULL_HANDLE);

    vkDestroyPipeline(m_device->handle(), pipeline, nullptr);
    vkDestroyPipeline(m_device->handle(), library, nullptr);
}

TEST_F(VkPositiveLayerTest, RayTracingPipelineCacheControl) {
    TEST_DESCRIPTION("Create ray tracing pipeline with VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&features13);
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }
    if (!features13.pipelineCreationCacheControl) {
        GTEST_SKIP() << "Feature pipelineCreationCacheControl is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");

    const VkPipelineLayoutObj pipeline_layout(m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = LvlInitStruct<VkRayTracingPipelineInterfaceCreateInfoKHR>();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);
    vk::DestroyPipeline(device(), library, nullptr);
}

TEST_F(VkPositiveLayerTest, NVRayTracingPipeline) {
    TEST_DESCRIPTION("Test VK_NV_ray_tracing.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto rtnv_props = LvlInitStruct<VkPhysicalDeviceRayTracingPropertiesNV>();
    GetPhysicalDeviceProperties2(rtnv_props);
    if (rtnv_props.maxDescriptorSetAccelerationStructures < 1) {
        GTEST_SKIP() << "VkPhysicalDeviceRayTracingPropertiesNV::maxDescriptorSetAccelerationStructures < 1";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto ignore_update = [](CreateNVRayTracingPipelineHelper &helper) {};
    CreateNVRayTracingPipelineHelper::OneshotPositiveTest(*this, ignore_update);
}
