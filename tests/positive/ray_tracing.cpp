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
#include "vk_extension_helper.h"

TEST_F(VkPositiveLayerTest, RayTracingValidateGetAccelerationStructureBuildSizes) {
    TEST_DESCRIPTION("Test enabled features for GetAccelerationStructureBuildSizes");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Crashes without any warnings
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Test does not run on AMD proprietary driver";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
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

TEST_F(VkPositiveLayerTest, RayTracingAccelerationStructureReference) {
    TEST_DESCRIPTION("Test host side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>();
    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    acc_structure_features.pNext = &ray_query_features;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&acc_structure_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (acc_structure_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkGetAccelerationStructureBuildSizesKHR =
        GetInstanceProcAddr<PFN_vkGetAccelerationStructureBuildSizesKHR>("vkGetAccelerationStructureBuildSizesKHR");
    const auto vkBuildAccelerationStructuresKHR =
        GetInstanceProcAddr<PFN_vkBuildAccelerationStructuresKHR>("vkBuildAccelerationStructuresKHR");

    // Build Bottom Level Acceleration Structure
    const std::vector<float> vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    const std::vector<uint32_t> indices = {0, 1, 2};
    VkAccelerationStructureGeometryKHR blas_geometry = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    blas_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    blas_geometry.flags = 0;
    blas_geometry.geometry.triangles = LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
    blas_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    blas_geometry.geometry.triangles.vertexData.hostAddress = vertices.data();
    blas_geometry.geometry.triangles.maxVertex = vertices.size() - 1;
    blas_geometry.geometry.triangles.vertexStride = 12;
    blas_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    blas_geometry.geometry.triangles.indexData.hostAddress = indices.data();
    blas_geometry.geometry.triangles.transformData.hostAddress = nullptr;

    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    blas_build_info_khr.flags = 0;
    blas_build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blas_build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    blas_build_info_khr.dstAccelerationStructure = VK_NULL_HANDLE;
    blas_build_info_khr.geometryCount = 1;
    blas_build_info_khr.pGeometries = &blas_geometry;
    blas_build_info_khr.ppGeometries = nullptr;

    VkAccelerationStructureBuildSizesInfoKHR blas_build_sizes = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
    uint32_t max_primitive_count = 1;
    vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &blas_build_info_khr,
                                            &max_primitive_count, &blas_build_sizes);

    VkBufferObj blas_buffer;
    blas_buffer.init(*m_device, blas_build_sizes.accelerationStructureSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    VkAccelerationStructureCreateInfoKHR blas_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    blas_create_info.buffer = blas_buffer.handle();
    blas_create_info.createFlags = 0;
    blas_create_info.offset = 0;
    blas_create_info.size = blas_build_sizes.accelerationStructureSize;
    blas_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj blas(*m_device, blas_create_info, false);
    blas_build_info_khr.dstAccelerationStructure = blas.handle();

    std::vector<uint8_t> blas_scratch(static_cast<size_t>(blas_build_sizes.buildScratchSize));
    blas_build_info_khr.scratchData.hostAddress = blas_scratch.data();

    VkAccelerationStructureBuildRangeInfoKHR blas_build_range_info;
    blas_build_range_info.firstVertex = 0;
    blas_build_range_info.primitiveCount = 1;
    blas_build_range_info.primitiveOffset = 0;
    blas_build_range_info.transformOffset = 0;
    VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &blas_build_range_info;

    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &blas_build_info_khr, &pBuildRangeInfos);

    // Build Top Level Acceleration Structure
    VkAccelerationStructureInstanceKHR tlas_instance = {};
    tlas_instance.accelerationStructureReference = (uint64_t)blas_build_info_khr.dstAccelerationStructure;

    VkAccelerationStructureGeometryKHR tlas_geometry = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    tlas_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    tlas_geometry.flags = 0;
    tlas_geometry.geometry.instances = LvlInitStruct<VkAccelerationStructureGeometryInstancesDataKHR>();
    tlas_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
    tlas_geometry.geometry.instances.data.hostAddress = &tlas_instance;

    VkAccelerationStructureBuildGeometryInfoKHR tlas_build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    tlas_build_info_khr.flags = 0;
    tlas_build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    tlas_build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    tlas_build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    tlas_build_info_khr.dstAccelerationStructure = VK_NULL_HANDLE;
    tlas_build_info_khr.geometryCount = 1;
    tlas_build_info_khr.pGeometries = &tlas_geometry;
    tlas_build_info_khr.ppGeometries = nullptr;

    VkAccelerationStructureBuildSizesInfoKHR tlas_build_sizes = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
    vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &tlas_build_info_khr,
                                            &max_primitive_count, &tlas_build_sizes);

    VkBufferObj tlas_buffer;
    tlas_buffer.init(*m_device, tlas_build_sizes.accelerationStructureSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    VkAccelerationStructureCreateInfoKHR tlas_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    tlas_create_info.buffer = tlas_buffer.handle();
    tlas_create_info.createFlags = 0;
    tlas_create_info.offset = 0;
    tlas_create_info.size = tlas_build_sizes.accelerationStructureSize;
    tlas_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    tlas_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj tlas(*m_device, tlas_create_info, false);
    tlas_build_info_khr.dstAccelerationStructure = tlas.handle();

    std::vector<uint8_t> tlas_scratch(static_cast<size_t>(tlas_build_sizes.buildScratchSize));
    tlas_build_info_khr.scratchData.hostAddress = tlas_scratch.data();

    VkAccelerationStructureBuildRangeInfoKHR tlas_build_range_info;
    tlas_build_range_info.primitiveCount = 1;
    tlas_build_range_info.primitiveOffset = 0;
    pBuildRangeInfos = &tlas_build_range_info;

    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &tlas_build_info_khr, &pBuildRangeInfos);
}
