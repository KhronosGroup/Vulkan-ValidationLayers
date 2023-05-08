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
#include "../framework/ray_tracing_objects.h"
#include "generated/vk_extension_helper.h"

class PositiveRayTracing : public VkPositiveLayerTest {};

TEST_F(PositiveRayTracing, GetAccelerationStructureBuildSizes) {
    TEST_DESCRIPTION("Test enabled features for GetAccelerationStructureBuildSizes");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    // Crashes without any warnings
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Test does not run on AMD proprietary driver";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>();
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&ray_query_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&ray_tracing_features);
    GetPhysicalDeviceFeatures2(features2);

    if (ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        GTEST_SKIP() << "rayTracingPipeline feature not supported";
    }

    ray_query_features.rayQuery = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkGetAccelerationStructureBuildSizesKHR =
        GetInstanceProcAddr<PFN_vkGetAccelerationStructureBuildSizesKHR>("vkGetAccelerationStructureBuildSizesKHR");

    auto build_info = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    uint32_t max_primitives_count;
    auto build_sizes_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
    vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                            &max_primitives_count, &build_sizes_info);
}

TEST_F(PositiveRayTracing, AccelerationStructureReference) {
    TEST_DESCRIPTION("Test device side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&bda_features);
    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&acc_structure_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress feature is not supported";
    }
    if (acc_structure_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_commandBuffer->begin();
    // Build Bottom Level Acceleration Structure
    auto bot_level_build_geometry = std::make_shared<rt::as::BuildGeometryInfoKHR>(
        rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device));
    bot_level_build_geometry->BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());

    // Build Top Level Acceleration Structure
    rt::as::BuildGeometryInfoKHR top_level_build_geometry =
        rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(DeviceValidationVersion(), *m_device, bot_level_build_geometry);
    top_level_build_geometry.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());

    m_commandBuffer->end();
}

TEST_F(PositiveRayTracing, HostAccelerationStructureReference) {
    TEST_DESCRIPTION("Test host side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>();
    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&acc_structure_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (acc_structure_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Build Bottom Level Acceleration Structure
    auto bot_level_build_geometry = std::make_shared<rt::as::BuildGeometryInfoKHR>(
        rt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(DeviceValidationVersion(), *m_device));
    bot_level_build_geometry->BuildHost(instance(), *m_device);

    // Build Top Level Acceleration Structure
    rt::as::BuildGeometryInfoKHR top_level_build_geometry =
        rt::as::blueprint::BuildGeometryInfoSimpleOnHostTopLevel(DeviceValidationVersion(), *m_device, bot_level_build_geometry);
    top_level_build_geometry.BuildHost(instance(), *m_device);
}

TEST_F(PositiveRayTracing, StridedDeviceAddressRegion) {
    TEST_DESCRIPTION("Test different valid VkStridedDeviceAddressRegionKHR");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    bda_features.bufferDeviceAddress = VK_TRUE;
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&bda_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    // Needed for Ray Tracing
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Create ray tracing pipeline
    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;
    {
        const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
        VkShaderObj rgen_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
        VkShaderObj chit_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

        const VkPipelineLayoutObj pipeline_layout(m_device, {});

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
        shader_stages[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        shader_stages[0].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        shader_stages[0].module = chit_shader.handle();
        shader_stages[0].pName = "main";

        shader_stages[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        shader_stages[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        shader_stages[1].module = rgen_shader.handle();
        shader_stages[1].pName = "main";

        std::array<VkRayTracingShaderGroupCreateInfoKHR, 1> shader_groups;
        shader_groups[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        shader_groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shader_groups[0].generalShader = 1;
        shader_groups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        shader_groups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        shader_groups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        raytracing_pipeline_ci.flags = 0;
        raytracing_pipeline_ci.stageCount = static_cast<uint32_t>(shader_stages.size());
        raytracing_pipeline_ci.pStages = shader_stages.data();
        raytracing_pipeline_ci.pGroups = shader_groups.data();
        raytracing_pipeline_ci.groupCount = shader_groups.size();
        raytracing_pipeline_ci.layout = pipeline_layout.handle();

        const VkResult result = vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                                 &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
        ASSERT_VK_SUCCESS(result);
    }

    VkBufferObj buffer;
    VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vk_testing::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);

    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    const VkDeviceAddress device_address = buffer.address();

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion, 100, 100,
                        1);

    // pRayGenShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR valid_region = stridebufregion;
        valid_region.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &valid_region, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                            100, 1);
    }

    // pRayGenShaderBindingTable->size == 0, deviceAddress is invalid => region is considered unused so no error
    {
        VkStridedDeviceAddressRegionKHR empty_region = stridebufregion;
        empty_region.deviceAddress += buffer.create_info().size + 128;
        empty_region.size = 0;
        empty_region.stride = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &empty_region, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                            100, 1);
    }

    m_commandBuffer->end();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}
