/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2026 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "binding.h"
#include "layer_validation_tests.h"
#include "descriptor_helper.h"
#include "pipeline_helper.h"
#include "ray_tracing_objects.h"
#include "utils/math_utils.h"

class NegativeRayTracingMicromap : public RayTracingTest {
  public:
    void InitMicromapTest();
};

void NegativeRayTracingMicromap::InitMicromapTest() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::micromap);
    RETURN_IF_SKIP(Init());
}

TEST_F(NegativeRayTracingMicromap, CreateMicromapEXTFeatureDisable) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);
    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT mm_ci = vku::InitStructHelper();
    mm_ci.createFlags = 0;
    mm_ci.buffer = buffer;
    mm_ci.offset = 0;
    mm_ci.size = 0;
    mm_ci.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    mm_ci.deviceAddress = 0ull;
    m_errorMonitor->SetDesiredError("VUID-vkCreateMicromapEXT-micromap-11615");
    vk::CreateMicromapEXT(device(), &mm_ci, nullptr, &micromap);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, DestroyMicromapEXTFeatureDisable) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());
    m_errorMonitor->SetDesiredError("VUID-vkDestroyMicromapEXT-micromap-11619");
    vk::DestroyMicromapEXT(device(), VK_NULL_HANDLE, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, GetMicromapBuildSizesEXTFeatureDisable) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());

    VkMicromapUsageEXT usage = {};
    usage.count = 1;
    usage.format = VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT;
    usage.subdivisionLevel = 1;

    VkMicromapBuildInfoEXT build_info = vku::InitStructHelper();
    build_info.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    build_info.usageCountsCount = 1;
    build_info.pUsageCounts = &usage;

    VkMicromapBuildSizesInfoEXT size_info = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkGetMicromapBuildSizesEXT-micromap-11618");
    vk::GetMicromapBuildSizesEXT(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, &size_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, CmdBuildMicromapsEXTFeatureDisable) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);
    VkMicromapBuildInfoEXT build_info = vku::InitStructHelper();
    build_info.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    build_info.triangleArray.deviceAddress = as_buffer.Address();
    build_info.data.deviceAddress = as_buffer.Address();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildMicromapsEXT-micromap-11648");
    vk::CmdBuildMicromapsEXT(m_command_buffer, 1, &build_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracingMicromap, CaptureReplayFeatureDisableEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    VkPhysicalDeviceOpacityMicromapFeaturesEXT ext_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper(&ext_features);
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&accel_features);
    RETURN_IF_SKIP(InitFramework());
    vk::GetPhysicalDeviceFeatures2(Gpu(), &features2);
    if (!ext_features.micromap) {
        GTEST_SKIP() << "micromap feature not supported";
    }
    ext_features.micromap = VK_TRUE;
    ext_features.micromapCaptureReplay = VK_FALSE;
    accel_features.accelerationStructure = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);

    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT mm_ci = vku::InitStructHelper();
    mm_ci.createFlags = VK_MICROMAP_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT;
    mm_ci.buffer = buffer;
    mm_ci.offset = 0;
    mm_ci.size = 0;
    mm_ci.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    mm_ci.deviceAddress = 0ull;

    m_errorMonitor->SetDesiredError("VUID-VkMicromapCreateInfoEXT-micromapCaptureReplay-11616");
    vk::CreateMicromapEXT(device(), &mm_ci, nullptr, &micromap);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, CreateAccelerationStructureKHR) {
    RETURN_IF_SKIP(InitMicromapTest());

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);
    as_create_info.buffer = buffer;
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-type-11600");
    VkAccelerationStructureKHR as;
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, CreateAccelerationStructure2KHRFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);
    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfo2KHR asci = vku::InitStructHelper();
    asci.addressRange = as_buffer.AddressRange();
    asci.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-micromap-11609");
    vk::CreateAccelerationStructure2KHR(device(), &asci, NULL, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, CreateAccelerationStructure2KHRAdditionalFeatures) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::micromap);
    RETURN_IF_SKIP(Init());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfo2KHR asci = vku::InitStructHelper();
    asci.addressRange = as_buffer.AddressRange();
    asci.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;

    {
        asci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-11610");
        vk::CreateAccelerationStructure2KHR(device(), &asci, NULL, &as);
        m_errorMonitor->VerifyFound();
    }
    {
        asci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-11611");
        vk::CreateAccelerationStructure2KHR(device(), &asci, NULL, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureInvalidFlags) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArray = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = micromap_as->handle();
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    {
        VkAccelerationStructureBuildRangeInfoKHR range_info = {};
        range_info.primitiveCount = 1;
        const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
        m_command_buffer.Begin();
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_MICROMAP_LOSSY_BIT_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-11563");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-geometryType-11707");
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-11544");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;
        m_command_buffer.Begin();
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        build_info.srcAccelerationStructure = NULL;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11562");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureInvalidFlagsEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    VkPhysicalDeviceOpacityMicromapFeaturesEXT ext_features = vku::InitStructHelper();
    VkPhysicalDeviceOpacityMicromapFeaturesKHR khr_features = vku::InitStructHelper(&ext_features);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper(&khr_features);
    VkPhysicalDeviceBufferDeviceAddressFeatures buffer_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceDeviceAddressCommandsFeaturesKHR device_addr_features = vku::InitStructHelper(&buffer_features);
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&device_addr_features);
    RETURN_IF_SKIP(InitFramework());
    vk::GetPhysicalDeviceFeatures2(Gpu(), &features2);
    if (!ext_features.micromap) {
        GTEST_SKIP() << "micromap feature not supported";
    }
    ext_features.micromap = VK_TRUE;
    khr_features.micromap = VK_TRUE;
    accel_features.accelerationStructure = VK_TRUE;
    buffer_features.bufferDeviceAddress = VK_TRUE;
    device_addr_features.deviceAddressCommands = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArray = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = micromap_as->handle();
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_BIT_KHR |
                       VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_DATA_UPDATE_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11558");
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11562");
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureMode) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    // Created but intentionally never built, to test validation of unbuilt src AS
    auto unbuilt_micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    unbuilt_micromap_as->Create();

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArray = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = micromap_as->handle();
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    {
        m_command_buffer.Begin();
        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        build_info.srcAccelerationStructure = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630");
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-mode-11545");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        m_command_buffer.Begin();
        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        build_info.srcAccelerationStructure = unbuilt_micromap_as->handle();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-srcAccelerationStructure-11546");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureGeometry) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArray = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = micromap_as->handle();
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    {
        VkAccelerationStructureBuildRangeInfoKHR range_info = {};
        range_info.primitiveCount = 1;
        const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
        m_command_buffer.Begin();
        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        build_info.srcAccelerationStructure = NULL;
        VkAccelerationStructureGeometryKHR triangle_geometry = vku::InitStructHelper();
        triangle_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        triangle_geometry.geometry.triangles = vku::InitStructHelper();
        triangle_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
        triangle_geometry.geometry.triangles.maxVertex = 3;
        triangle_geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
        build_info.pGeometries = &triangle_geometry;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-geometryType-11561");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

        std::array<VkAccelerationStructureGeometryKHR, 2> geometry_array = {geometry, geometry};
        m_command_buffer.Begin();
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
        geometry.pNext = &micromap_data;
        build_info.geometryCount = 2;
        build_info.pGeometries = geometry_array.data();
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-geometryCount-11560");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureGeometryFlags) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArray = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = micromap_as->handle();
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    {
        m_command_buffer.Begin();
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        build_info.srcAccelerationStructure = VK_NULL_HANDLE;
        geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryKHR-flags-11569");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        geometry.flags &= ~VK_GEOMETRY_OPAQUE_BIT_KHR;
        m_command_buffer.End();
    }

    {
        m_command_buffer.Begin();
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
        build_info.srcAccelerationStructure = VK_NULL_HANDLE;
        geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
        geometry.pNext = NULL;
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryKHR-micromap-11568");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureMicromapData) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer micromap_buffer(*m_device, 4096,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArray = micromap_as->GetBufferDeviceAddress();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    {
        m_command_buffer.Begin();
        micromap_data.data = 0;
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11550");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        m_command_buffer.Begin();
        micromap_data.data = 0x1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11552");
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-11544");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelerationStructureMicromapData2) {
    RETURN_IF_SKIP(InitMicromapTest());

    // BLAS used as intentionally wrong dst type in block 3 to trigger dstAccelerationStructure validation
    auto blas2 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas2->Create();

    vkt::Buffer micromap_buffer(*m_device, 4096,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = micromap_buffer.Address();
    micromap_data.triangleArray = micromap_buffer.Address();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = blas2->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11551");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11551");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-11547");
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelerationStructureMicromapUsage) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkMicromapUsageKHR usage = {1024, 1, VK_OPACITY_MICROMAP_FORMAT_2_STATE_KHR};
    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = triangle_buffer.Address();
    micromap_data.triangleArray = triangle_buffer.Address();
    micromap_data.triangleArrayStride = 8;
    micromap_data.usageCountsCount = 1;
    micromap_data.pUsageCounts = &usage;
    micromap_data.ppUsageCounts = nullptr;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = micromap_as->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11554");
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelerationStructureTriangleMicromap) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    auto blas2 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas2->Create();

    // blas3 is used as an intentionally wrong micromap handle in block 2
    auto blas3 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas3->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer micromap_buffer(*m_device, 4096,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    {
        VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
        triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
        triangle_mm.indexBuffer = micromap_buffer.Address();
        triangle_mm.indexStride = 0;
        triangle_mm.micromap = NULL;

        VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
        geometry.geometry.triangles.maxVertex = 3;
        geometry.geometry.triangles.transformData.deviceAddress = triangle_buffer.Address();
        geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        build_info.srcAccelerationStructure = nullptr;
        build_info.dstAccelerationStructure = blas1->handle();
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexBuffer-11571");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-micromap-11579");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
        triangle_mm.indexType = VK_INDEX_TYPE_UINT16;
        triangle_mm.indexBuffer = 0;
        triangle_mm.indexStride = 1;
        triangle_mm.micromap = blas3->handle();  // intentionally wrong type

        VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
        geometry.geometry.triangles.maxVertex = 3;
        geometry.geometry.triangles.transformData.deviceAddress = triangle_buffer.Address();
        geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        build_info.srcAccelerationStructure = nullptr;
        build_info.dstAccelerationStructure = blas3->handle();
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexBuffer-11572");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexStride-11573");
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-11548");
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-11549");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
        triangle_mm.indexType = VK_INDEX_TYPE_UINT32;
        triangle_mm.indexBuffer = micromap_buffer.Address();
        triangle_mm.indexStride = UINT64_MAX;
        triangle_mm.micromap = micromap_as->handle();

        VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
        geometry.geometry.triangles.maxVertex = 3;
        geometry.geometry.triangles.transformData.deviceAddress = triangle_buffer.Address();
        geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        build_info.srcAccelerationStructure = nullptr;
        build_info.dstAccelerationStructure = blas2->handle();
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexStride-11574");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexStride-11573");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelerationStructureTriangleMicromapIndexValidation) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkMicromapUsageKHR usage = {1, 1, VK_OPACITY_MICROMAP_FORMAT_2_STATE_KHR};
    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = triangle_buffer.Address();
    micromap_data.triangleArray = triangle_buffer.Address();
    micromap_data.triangleArrayStride = 8;
    micromap_data.usageCountsCount = 1;
    micromap_data.pUsageCounts = &usage;
    micromap_data.ppUsageCounts = nullptr;

    VkAccelerationStructureGeometryKHR micromap_geometry = vku::InitStructHelper(&micromap_data);
    micromap_geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR micromap_build_info = vku::InitStructHelper();
    micromap_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    micromap_build_info.geometryCount = 1;
    micromap_build_info.pGeometries = &micromap_geometry;
    micromap_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    micromap_build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    micromap_build_info.dstAccelerationStructure = micromap_as->handle();
    micromap_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &micromap_build_info, &range_info_null);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles = vku::InitStructHelper();
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
    geometry.geometry.triangles.maxVertex = 5;
    geometry.geometry.triangles.transformData.deviceAddress = triangle_buffer.Address();
    geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = blas1->handle();
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 2;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    {
        vkt::Buffer small_index_buffer(*m_device, 4, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

        VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
        triangle_mm.indexType = VK_INDEX_TYPE_UINT32;
        triangle_mm.indexBuffer = small_index_buffer.Address();
        triangle_mm.indexStride = 4;
        triangle_mm.baseTriangle = 0;
        triangle_mm.micromap = micromap_as->handle();
        geometry.geometry.triangles.pNext = &triangle_mm;
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-indexBuffer-11577");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        range_info.primitiveCount = 1;
        VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
        triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
        triangle_mm.indexBuffer = 0;
        triangle_mm.baseTriangle = 2;
        triangle_mm.micromap = micromap_as->handle();
        geometry.geometry.triangles.pNext = &triangle_mm;
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-geometry-11576");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructureTriangleMicromapEXT) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureTrianglesOpacityMicromapEXT triangle_mm = vku::InitStructHelper();
    triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
    triangle_mm.micromap = VK_NULL_HANDLE;
    VkMicromapUsageEXT usage = {1, 1, VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT};
    VkMicromapUsageEXT* p_usage = &usage;
    triangle_mm.pUsageCounts = &usage;
    triangle_mm.ppUsageCounts = &p_usage;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
    geometry.geometry.triangles.maxVertex = 3;
    geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = blas1->handle();
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapEXT-pUsageCounts-07335");
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelerationStructureIndexType) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer micromap_buffer(*m_device, 4096,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                vkt::device_address);

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
    triangle_mm.indexType = VK_INDEX_TYPE_MAX_ENUM;
    triangle_mm.indexBuffer = micromap_buffer.Address();
    triangle_mm.indexStride = 4;
    triangle_mm.micromap = micromap_as->handle();

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
    geometry.geometry.triangles.maxVertex = 3;
    geometry.geometry.triangles.transformData.deviceAddress = triangle_buffer.Address();
    geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.srcAccelerationStructure = NULL;
    build_info.dstAccelerationStructure = blas1->handle();
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    m_command_buffer.Begin();
    // Need to skip stateless validation
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexType-parameter");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureTrianglesOpacityMicromapKHR-indexType-11570");
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructuresIndirectKHR) {
    AddRequiredFeature(vkt::Feature::accelerationStructureIndirectBuild);
    RETURN_IF_SKIP(InitMicromapTest());

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = VK_NULL_HANDLE;

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkAccelerationStructureBuildRangeInfoKHR), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                vkt::device_address);
    VkDeviceAddress indirect_device_address = indirect_buffer.Address();
    uint32_t stride = 8;

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-type-11557");
    uint32_t max_primitive_count = 1u;
    const uint32_t* p_max_primitive_counts = &max_primitive_count;
    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresIndirectKHR(m_command_buffer, 1u, &build_info, &indirect_device_address, &stride,
                                                  &p_max_primitive_counts);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracingMicromap, BuildAccelStructureFeatureNotPresent) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);

    RETURN_IF_SKIP(Init());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = as_buffer.Address();
    micromap_data.triangleArray = as_buffer.Address();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    {
        m_command_buffer.Begin();
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11562");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-micromap-11559");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11710");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        m_command_buffer.Begin();
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISABLE_OPACITY_MICROMAPS_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11562");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-micromap-11559");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11711");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracingMicromap, BuildAccelStructureFeatureNotPresentEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = as_buffer.Address();
    micromap_data.triangleArray = as_buffer.Address();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_DATA_UPDATE_BIT_EXT;

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11562");
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-micromap-11559");
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-11709");
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracingMicromap, CmdBuildAccelerationStructuresBuildSizesKHR) {
    RETURN_IF_SKIP(InitMicromapTest());

    vkt::Buffer as_buffer(*m_device, 4096,
                          VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                          vkt::device_address);

    VkMicromapUsageKHR micromap_usage{1, 1, VK_OPACITY_MICROMAP_FORMAT_2_STATE_KHR};

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = as_buffer.Address();
    micromap_data.triangleArray = as_buffer.Address();
    micromap_data.triangleArrayStride = 8;
    micromap_data.usageCountsCount = 1;
    micromap_data.pUsageCounts = &micromap_usage;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;

    uint32_t primitive_count = 1;
    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();

    {
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureBuildSizesKHR-buildType-11614");
        m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureBuildSizesKHR-pMaxPrimitiveCounts-11613");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &build_info,
                                                  &primitive_count, &size_info);
        m_errorMonitor->VerifyFound();
    }

    {
        micromap_usage = {1, UINT32_MAX, VK_OPACITY_MICROMAP_FORMAT_4_STATE_KHR};
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-subdivisionLevel-11564");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, NULL,
                                                  &size_info);
        m_errorMonitor->VerifyFound();
    }

    {
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_MICROMAP_LOSSY_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-subdivisionLevel-11565");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, NULL,
                                                  &size_info);
        m_errorMonitor->VerifyFound();
    }

    {
        micromap_usage = {1, 1, VK_OPACITY_MICROMAP_FORMAT_2_STATE_KHR};
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_MICROMAP_LOSSY_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-format-11712");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, NULL,
                                                  &size_info);
        m_errorMonitor->VerifyFound();
        build_info.flags &= ~VK_BUILD_ACCELERATION_STRUCTURE_MICROMAP_LOSSY_BIT_KHR;
    }

    {
        VkMicromapUsageKHR* mm_usage_ptr = &micromap_usage;
        micromap_data.ppUsageCounts = &mm_usage_ptr;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryMicromapDataKHR-pUsageCounts-11642");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, NULL,
                                                  &size_info);
        m_errorMonitor->VerifyFound();
        micromap_data.ppUsageCounts = NULL;
    }

    {
        micromap_usage = {1, UINT32_MAX, VK_OPACITY_MICROMAP_FORMAT_2_STATE_KHR};
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryMicromapDataKHR-subdivisionLevel-11645");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, NULL,
                                                  &size_info);
        m_errorMonitor->VerifyFound();
    }

    {
        micromap_usage = {UINT32_MAX, 1, VK_OPACITY_MICROMAP_FORMAT_2_STATE_KHR};
        micromap_data.triangleArrayStride = 7;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryMicromapDataKHR-triangleArrayStride-11646");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryMicromapDataKHR-triangleArrayStride-11647");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryMicromapDataKHR-count-11643");
        vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, NULL,
                                                  &size_info);
        m_errorMonitor->VerifyFound();
    }
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_WriteAccelerationStructuresProperties) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfo2KHR asci = vku::InitStructHelper();
    asci.addressRange = as_buffer.AddressRange();
    asci.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;

    vk::CreateAccelerationStructure2KHR(device(), &asci, NULL, &as);

    constexpr size_t stride = 1;
    constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
    uint8_t data[data_size];

    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-11591");
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-11592");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &as, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                                                 data_size, data, sizeof(VkDeviceSize));
    m_errorMonitor->VerifyFound();
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CopyAccelerationStructureToMemoryKHR) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfo2KHR asci = vku::InitStructHelper();
    asci.addressRange = as_buffer.AddressRange();
    asci.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;

    vk::CreateAccelerationStructure2KHR(device(), &asci, NULL, &as);

    std::vector<uint8_t> data(4096, 0);

    {
        VkDeviceOrHostAddressKHR output_data;
        output_data.hostAddress = reinterpret_cast<void*>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(data.data()), 16));
        VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src = as;
        copy_info.dst = output_data;
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCopyAccelerationStructureToMemoryKHR-src-11678");
        vk::CopyAccelerationStructureToMemoryKHR(*m_device, VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        VkDeviceOrHostAddressConstKHR output_data;
        output_data.hostAddress = reinterpret_cast<void*>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(data.data()), 16));
        VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src = output_data;
        copy_info.dst = as;
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCopyMemoryToAccelerationStructureKHR-dst-11677");
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CopyMemoryToAccelerationStructureKHR) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.dst = blas1->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    std::vector<uint8_t> header(1024, 0);

    {
        uint32_t offset = 2 * VK_UUID_SIZE + 2 * sizeof(uint64_t);
        uint64_t block_count = 1;
        std::memcpy(header.data() + offset, &block_count, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        uint32_t micromap_block_type = VK_ACCELERATION_STRUCTURE_SERIALIZED_BLOCK_TYPE_OPACITY_MICROMAP_KHR;
        std::memcpy(header.data() + offset, &micromap_block_type, sizeof(uint32_t));
        offset += sizeof(uint32_t) + 2 * sizeof(uint64_t);                           // skip type, offset, size
        VkDeviceAddress invalid_micromap_address = scratch_buffer.Address() + 8192;  // Address not pointing to a micromap
        std::memcpy(header.data() + offset, &invalid_micromap_address, sizeof(VkDeviceAddress));
        copy_info.src.hostAddress = header.data();
        m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-src-11585");
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
    }
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CopyMemoryToAccelerationStructureKHRFeatureDisabled) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.dst = blas1->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    std::vector<uint8_t> header(1024, 0);

    uint32_t offset = 2 * VK_UUID_SIZE + 2 * sizeof(uint64_t);
    uint64_t block_count = 1;
    std::memcpy(header.data() + offset, &block_count, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    uint32_t micromap_block_type = VK_ACCELERATION_STRUCTURE_SERIALIZED_BLOCK_TYPE_OPACITY_MICROMAP_KHR;
    std::memcpy(header.data() + offset, &micromap_block_type, sizeof(uint32_t));
    copy_info.src.hostAddress = header.data();
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-src-11584");
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CopyMemoryToFromAccelStructUnbound) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    auto buffer_ci = vkt::Buffer::CreateInfo(
        4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    vkt::Buffer unbound_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles = vku::InitStructHelper();
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    geometry.geometry.triangles.maxVertex = 3;
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = blas1->handle();
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    {
        std::vector<uint8_t> host_memory(4096);
        VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src = blas1->handle();
        copy_info.dst.hostAddress = host_memory.data();
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

        m_command_buffer.Begin();
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_ptr);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        VkAccelerationStructureKHR as_unbound;
        VkAccelerationStructureCreateInfoKHR as_ci = vku::InitStructHelper();
        as_ci.buffer = unbound_buffer.handle();
        as_ci.offset = 0;
        as_ci.size = 4096;
        as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        vk::CreateAccelerationStructureKHR(device(), &as_ci, nullptr, &as_unbound);

        copy_info.src = as_unbound;
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03731");
        m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-11714");
        vk::CopyAccelerationStructureToMemoryKHR(device(), VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
        vk::DestroyAccelerationStructureKHR(device(), as_unbound, nullptr);
    }

    {
        std::vector<uint8_t> host_memory(4096);
        VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src.hostAddress = host_memory.data();
        copy_info.dst = blas1->handle();
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

        VkAccelerationStructureKHR as_unbound;
        VkAccelerationStructureCreateInfoKHR as_ci = vku::InitStructHelper();
        as_ci.buffer = unbound_buffer.handle();
        as_ci.offset = 0;
        as_ci.size = 4096;
        as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        vk::CreateAccelerationStructureKHR(device(), &as_ci, nullptr, &as_unbound);

        copy_info.dst = as_unbound;
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
        m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-11717");
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
        vk::DestroyAccelerationStructureKHR(device(), as_unbound, nullptr);
    }
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelerationStructureUpdateMicromapConsistency) {
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    // Created but intentionally never built, to test validation of unbuilt micromap
    auto unbuilt_micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    unbuilt_micromap_as->Create();

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    auto blas2 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas2->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer micromap_buffer(*m_device, 4096,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = triangle_buffer.Address();
    micromap_data.triangleArray = triangle_buffer.Address();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR micromap_geometry = vku::InitStructHelper();
    micromap_geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    micromap_geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR micromap_build_info = vku::InitStructHelper();
    micromap_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    micromap_build_info.geometryCount = 1;
    micromap_build_info.pGeometries = &micromap_geometry;
    micromap_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    micromap_build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    micromap_build_info.dstAccelerationStructure = micromap_as->handle();
    micromap_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &micromap_build_info, &range_info_null);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
    triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
    triangle_mm.micromap = micromap_as->handle();

    VkAccelerationStructureGeometryKHR triangle_geometry = vku::InitStructHelper();
    triangle_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    triangle_geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
    triangle_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangle_geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    triangle_geometry.geometry.triangles.vertexStride = 12;
    triangle_geometry.geometry.triangles.maxVertex = 3;
    triangle_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info = vku::InitStructHelper();
    blas_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR |
                            VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_BIT_KHR;
    blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blas_build_info.geometryCount = 1;
    blas_build_info.pGeometries = &triangle_geometry;
    blas_build_info.dstAccelerationStructure = blas1->handle();
    blas_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    {
        m_command_buffer.Begin();
        triangle_mm.micromap = VK_NULL_HANDLE;
        triangle_mm.indexType = VK_INDEX_TYPE_UINT32;
        triangle_mm.indexBuffer = micromap_buffer.Address();
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        blas_build_info.srcAccelerationStructure = blas1->handle();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11625");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        triangle_mm.micromap = VK_NULL_HANDLE;
        triangle_mm.indexType = VK_INDEX_TYPE_UINT32;
        triangle_mm.indexBuffer = micromap_buffer.Address();
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        blas_build_info.srcAccelerationStructure = VK_NULL_HANDLE;
        m_command_buffer.Begin();
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        m_command_buffer.Begin();
        triangle_mm.micromap = micromap_as->handle();
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        blas_build_info.srcAccelerationStructure = blas1->handle();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11626");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        triangle_geometry.geometry.triangles.pNext = nullptr;
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        blas_build_info.srcAccelerationStructure = VK_NULL_HANDLE;
        m_command_buffer.Begin();
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        m_command_buffer.Begin();
        triangle_mm.micromap = micromap_as->handle();
        triangle_geometry.geometry.triangles.pNext = &triangle_mm;
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        blas_build_info.srcAccelerationStructure = blas1->handle();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11627");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        triangle_mm.micromap = micromap_as->handle();
        triangle_geometry.geometry.triangles.pNext = &triangle_mm;
        blas_build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        blas_build_info.srcAccelerationStructure = VK_NULL_HANDLE;
        blas_build_info.dstAccelerationStructure = blas2->handle();
        m_command_buffer.Begin();
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        m_command_buffer.Begin();
        triangle_mm.micromap = unbuilt_micromap_as->handle();
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        blas_build_info.srcAccelerationStructure = blas2->handle();
        blas_build_info.dstAccelerationStructure = blas2->handle();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11629");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdBuildAccelStructureUpdateMicromapDeserialized) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    auto micromap_as = vkt::as::blueprint::AccelStructSimpleOnDeviceMicromap(*m_device, 4096);
    micromap_as->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    vkt::Buffer blas1_buffer(*m_device, 4096,
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             vkt::device_address);

    VkAccelerationStructureCreateInfoKHR blas1_ci = vku::InitStructHelper();
    blas1_ci.buffer = blas1_buffer;
    blas1_ci.offset = 0;
    blas1_ci.size = 4096;
    blas1_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR blas1_handle;
    vk::CreateAccelerationStructureKHR(device(), &blas1_ci, nullptr, &blas1_handle);

    vkt::Buffer blas2_buffer(*m_device, 4096,
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             vkt::device_address);

    VkAccelerationStructureCreateInfoKHR blas2_ci = vku::InitStructHelper();
    blas2_ci.buffer = blas2_buffer;
    blas2_ci.offset = 0;
    blas2_ci.size = 4096;
    blas2_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR blas2_handle;
    vk::CreateAccelerationStructureKHR(device(), &blas2_ci, nullptr, &blas2_handle);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = triangle_buffer.Address();
    micromap_data.triangleArray = triangle_buffer.Address();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR micromap_geometry = vku::InitStructHelper();
    micromap_geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;
    micromap_geometry.pNext = &micromap_data;

    VkAccelerationStructureBuildGeometryInfoKHR micromap_build_info = vku::InitStructHelper();
    micromap_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    micromap_build_info.geometryCount = 1;
    micromap_build_info.pGeometries = &micromap_geometry;
    micromap_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    micromap_build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    micromap_build_info.dstAccelerationStructure = micromap_as->handle();
    micromap_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &micromap_build_info, &range_info_null);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
    triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
    triangle_mm.micromap = micromap_as->handle();

    VkAccelerationStructureGeometryKHR triangle_geometry = vku::InitStructHelper();
    triangle_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    triangle_geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
    triangle_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangle_geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    triangle_geometry.geometry.triangles.vertexStride = 12;
    triangle_geometry.geometry.triangles.maxVertex = 3;
    triangle_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info = vku::InitStructHelper();
    blas_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blas_build_info.geometryCount = 1;
    blas_build_info.pGeometries = &triangle_geometry;
    blas_build_info.dstAccelerationStructure = blas1_handle;
    blas_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    std::vector<uint8_t> serialized_data(4096, 0);
    VkDeviceOrHostAddressKHR output_data;
    output_data.hostAddress = reinterpret_cast<void*>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(serialized_data.data()), 16));

    VkCopyAccelerationStructureToMemoryInfoKHR copy_to_mem_info = vku::InitStructHelper();
    copy_to_mem_info.src = blas1_handle;
    copy_to_mem_info.dst = output_data;
    copy_to_mem_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    vk::CopyAccelerationStructureToMemoryKHR(device(), VK_NULL_HANDLE, &copy_to_mem_info);

    VkDeviceOrHostAddressConstKHR input_data;
    input_data.hostAddress = output_data.hostAddress;

    VkCopyMemoryToAccelerationStructureInfoKHR copy_from_mem_info = vku::InitStructHelper();
    copy_from_mem_info.src = input_data;
    copy_from_mem_info.dst = blas2_handle;
    copy_from_mem_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    vkt::Buffer separate_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = separate_buffer;
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 4096;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkAccelerationStructureKHR as_separate = VK_NULL_HANDLE;
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as_separate);

    {
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_from_mem_info);
        m_command_buffer.Begin();
        triangle_geometry.geometry.triangles.pNext = nullptr;
        blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        blas_build_info.srcAccelerationStructure = blas2_handle;
        blas_build_info.dstAccelerationStructure = blas2_handle;
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-micromap-11628");
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_from_mem_info);
        VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src = blas2_handle;
        copy_info.dst = as_separate;
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureKHR-src-11634");
        vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_from_mem_info);
        VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src = blas2_handle;
        copy_info.dst = as_separate;
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;
        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCopyAccelerationStructureKHR-src-11587");
        vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
        vk::DestroyAccelerationStructureKHR(device(), as_separate, nullptr);
    }

    {
        vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_from_mem_info);
        std::vector<uint8_t> out_data(4096, 0);
        VkDeviceOrHostAddressKHR out_addr = {};
        out_addr.hostAddress = reinterpret_cast<void*>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(out_data.data()), 16));
        VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
        copy_info.src = blas2_handle;
        copy_info.dst = out_addr;
        copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-11582");
        vk::CopyAccelerationStructureToMemoryKHR(device(), VK_NULL_HANDLE, &copy_info);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyAccelerationStructureKHR(device(), blas1_handle, nullptr);
    vk::DestroyAccelerationStructureKHR(device(), blas2_handle, nullptr);
}

TEST_F(NegativeRayTracingMicromap, BuildAccelStructInstanceMicromapFeatureDisabled) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    VkAccelerationStructureInstanceKHR instance = {};
    instance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPACITY_MICROMAP_2_STATE_BIT_KHR;

    VkAccelerationStructureGeometryInstancesDataKHR instances = vku::InitStructHelper();
    instances.data.hostAddress = &instance;
    instances.arrayOfPointers = VK_FALSE;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances = instances;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;

    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
    uint32_t primitive_count = 1;
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &build_info,
                                              &primitive_count, &size_info);

    std::vector<uint8_t> scratch_memory(size_info.buildScratchSize);
    build_info.scratchData.hostAddress = scratch_memory.data();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInstanceKHR-micromap-11580");
    vk::BuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info, &range_info_ptr);
    m_errorMonitor->VerifyFound();
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_BuildAccelStructInstanceDisableBuildFlag) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryKHR triangle_geometry = vku::InitStructHelper();
    triangle_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    triangle_geometry.geometry.triangles = vku::InitStructHelper();
    triangle_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangle_geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    triangle_geometry.geometry.triangles.vertexStride = 12;
    triangle_geometry.geometry.triangles.maxVertex = 3;
    triangle_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info = vku::InitStructHelper();
    blas_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blas_build_info.geometryCount = 1;
    blas_build_info.pGeometries = &triangle_geometry;
    blas_build_info.dstAccelerationStructure = blas1->handle();
    blas_build_info.scratchData.hostAddress = scratch_buffer.Memory().Map();

    VkAccelerationStructureBuildRangeInfoKHR blas_range_info = {};
    blas_range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* p_blas_range_info = &blas_range_info;

    vk::BuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &blas_build_info, &p_blas_range_info);
    scratch_buffer.Memory().Unmap();

    VkAccelerationStructureInstanceKHR instance = {};
    instance.flags = VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_BIT_KHR;
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0;
    instance.accelerationStructureReference = CastToUint64(blas1->handle());

    VkAccelerationStructureGeometryInstancesDataKHR instances = vku::InitStructHelper();
    instances.data.hostAddress = &instance;
    instances.arrayOfPointers = VK_FALSE;

    VkAccelerationStructureGeometryKHR tlas_geometry = vku::InitStructHelper();
    tlas_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    tlas_geometry.geometry.instances = instances;

    VkAccelerationStructureBuildGeometryInfoKHR tlas_build_info = vku::InitStructHelper();
    tlas_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    tlas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    tlas_build_info.geometryCount = 1;
    tlas_build_info.pGeometries = &tlas_geometry;

    VkAccelerationStructureBuildSizesInfoKHR tlas_size_info = vku::InitStructHelper();
    uint32_t tlas_primitive_count = 1;
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &tlas_build_info,
                                              &tlas_primitive_count, &tlas_size_info);

    vkt::Buffer tlas_buffer(*m_device, tlas_size_info.accelerationStructureSize,
                            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkAccelerationStructureCreateInfoKHR tlas_ci = vku::InitStructHelper();
    tlas_ci.buffer = tlas_buffer;
    tlas_ci.offset = 0;
    tlas_ci.size = tlas_size_info.accelerationStructureSize;
    tlas_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    VkAccelerationStructureKHR tlas_handle;
    vk::CreateAccelerationStructureKHR(device(), &tlas_ci, nullptr, &tlas_handle);

    tlas_build_info.dstAccelerationStructure = tlas_handle;

    std::vector<uint8_t> tlas_scratch_memory(tlas_size_info.buildScratchSize);
    tlas_build_info.scratchData.hostAddress = tlas_scratch_memory.data();

    VkAccelerationStructureBuildRangeInfoKHR tlas_range_info = {};
    tlas_range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* p_tlas_range_info = &tlas_range_info;

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInstanceKHR-flags-11581");
    vk::BuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &tlas_build_info, &p_tlas_range_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyAccelerationStructureKHR(device(), tlas_handle, nullptr);
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_CmdCopyAccelStructToMemoryAlignment) {
    RETURN_IF_SKIP(InitMicromapTest());

    // as_buffer provides the misaligned address for the micromap AS
    vkt::Buffer as_buffer(*m_device, 4096,
                          VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                          vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureKHR misaligned_micromap = VK_NULL_HANDLE;
    VkDeviceAddressRangeKHR misaligned_address_range = {};
    misaligned_address_range.address = as_buffer.Address() + 64;
    misaligned_address_range.size = 4096 - 64;

    VkAccelerationStructureCreateInfo2KHR misaligned_asci = vku::InitStructHelper();
    misaligned_asci.addressRange = misaligned_address_range;
    misaligned_asci.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11605");
    vk::CreateAccelerationStructure2KHR(device(), &misaligned_asci, nullptr, &misaligned_micromap);

    VkAccelerationStructureGeometryMicromapDataKHR micromap_data = vku::InitStructHelper();
    micromap_data.data = as_buffer.Address();
    micromap_data.triangleArray = as_buffer.Address();
    micromap_data.triangleArrayStride = 8;

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper(&micromap_data);
    geometry.geometryType = VK_GEOMETRY_TYPE_MICROMAP_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = misaligned_micromap;
    build_info.scratchData.deviceAddress = scratch_buffer.Address();

    const VkAccelerationStructureBuildRangeInfoKHR* range_info_null = nullptr;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &build_info, &range_info_null);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    std::vector<uint8_t> serialized_data(4096, 0);
    VkDeviceOrHostAddressKHR dst_addr = {};
    dst_addr.deviceAddress = scratch_buffer.Address();

    VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = misaligned_micromap;
    copy_info.dst = dst_addr;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-11700");
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    vk::DestroyAccelerationStructureKHR(device(), misaligned_micromap, nullptr);
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_DescriptorAccessDeserializedAS) {
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitMicromapTest());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    auto blas2 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas2->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
    triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
    triangle_mm.micromap = VK_NULL_HANDLE;

    VkAccelerationStructureGeometryKHR triangle_geometry = vku::InitStructHelper();
    triangle_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    triangle_geometry.geometry.triangles = vku::InitStructHelper(&triangle_mm);
    triangle_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangle_geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    triangle_geometry.geometry.triangles.vertexStride = 12;
    triangle_geometry.geometry.triangles.maxVertex = 3;
    triangle_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info = vku::InitStructHelper();
    blas_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blas_build_info.geometryCount = 1;
    blas_build_info.pGeometries = &triangle_geometry;
    blas_build_info.dstAccelerationStructure = blas1->handle();
    blas_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    std::vector<uint8_t> serialized_data(4096, 0);
    VkDeviceOrHostAddressKHR dst_addr = {};
    dst_addr.hostAddress = reinterpret_cast<void*>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(serialized_data.data()), 16));

    VkCopyAccelerationStructureToMemoryInfoKHR copy_to_mem = vku::InitStructHelper();
    copy_to_mem.src = blas1->handle();
    copy_to_mem.dst = dst_addr;
    copy_to_mem.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    vk::CopyAccelerationStructureToMemoryKHR(device(), VK_NULL_HANDLE, &copy_to_mem);

    VkDeviceOrHostAddressConstKHR src_addr = {};
    src_addr.hostAddress = dst_addr.hostAddress;

    VkCopyMemoryToAccelerationStructureInfoKHR copy_from_mem = vku::InitStructHelper();
    copy_from_mem.src = src_addr;
    copy_from_mem.dst = blas2->handle();
    copy_from_mem.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_from_mem);

    const char* cs_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require
        layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
        layout(set = 0, binding = 1) buffer SSBO { float result; };
        void main() {
            rayQueryEXT rq;
            rayQueryInitializeEXT(rq, topLevelAS, gl_RayFlagsOpaqueEXT, 0xFF,
                                  vec3(0.0), 0.0, vec3(0.0, 0.0, 1.0), 1000.0);
            while (rayQueryProceedEXT(rq)) {}
            result = 1.0;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                  });

    VkAccelerationStructureKHR blas2_handle = blas2->handle();
    descriptor_set.WriteDescriptorAccelStruct(0, 1, &blas2_handle);

    vkt::Buffer result_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    descriptor_set.WriteDescriptorBufferInfo(1, result_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-micromap-11637");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// TODO - core checks
TEST_F(NegativeRayTracingMicromap, DISABLED_TraceRaysOMMPipelineFlagMissing) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::micromap);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    auto blas1 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas1->Create();

    vkt::Buffer triangle_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, 4096,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    VkAccelerationStructureGeometryKHR triangle_geometry = vku::InitStructHelper();
    triangle_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    triangle_geometry.geometry.triangles = vku::InitStructHelper();
    triangle_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangle_geometry.geometry.triangles.vertexData.deviceAddress = triangle_buffer.Address();
    triangle_geometry.geometry.triangles.vertexStride = 12;
    triangle_geometry.geometry.triangles.maxVertex = 3;
    triangle_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

    VkAccelerationStructureTrianglesOpacityMicromapKHR triangle_mm = vku::InitStructHelper();
    triangle_mm.indexType = VK_INDEX_TYPE_NONE_KHR;
    triangle_mm.micromap = VK_NULL_HANDLE;
    triangle_geometry.geometry.triangles.pNext = &triangle_mm;

    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info = vku::InitStructHelper();
    blas_build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blas_build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blas_build_info.geometryCount = 1;
    blas_build_info.pGeometries = &triangle_geometry;
    blas_build_info.dstAccelerationStructure = blas1->handle();
    blas_build_info.scratchData.deviceAddress = scratch_buffer.Address();

    VkAccelerationStructureBuildRangeInfoKHR range_info = {};
    range_info.primitiveCount = 1;
    const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &blas_build_info, &range_info_ptr);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    const char* rgen_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
        layout(location = 0) rayPayloadEXT float payload;
        void main() {
            traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0,
                        vec3(0.0), 0.0, vec3(0.0, 0.0, 1.0), 1000.0, 0);
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
                  });

    VkAccelerationStructureKHR blas1_handle = blas1->handle();
    descriptor_set.WriteDescriptorAccelStruct(0, 1, &blas1_handle);
    descriptor_set.UpdateDescriptorSets();

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkShaderObj rgen_shader(*m_device, rgen_source, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper();
    stage_ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_ci.module = rgen_shader;
    stage_ci.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_ci = vku::InitStructHelper();
    group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_ci.generalShader = 0;
    group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR rt_pipeline_ci = vku::InitStructHelper();
    rt_pipeline_ci.stageCount = 1;
    rt_pipeline_ci.pStages = &stage_ci;
    rt_pipeline_ci.groupCount = 1;
    rt_pipeline_ci.pGroups = &group_ci;
    rt_pipeline_ci.maxPipelineRayRecursionDepth = 1;
    rt_pipeline_ci.layout = pipeline_layout;

    VkPipeline rt_pipeline = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rt_pipeline_ci, nullptr, &rt_pipeline);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rt_props);

    const uint32_t handle_size = rt_props.shaderGroupHandleSize;
    const uint32_t handle_alignment = rt_props.shaderGroupHandleAlignment;
    const uint32_t aligned_handle_size = Align(handle_size, handle_alignment);

    std::vector<uint8_t> handles(handle_size);
    vk::GetRayTracingShaderGroupHandlesKHR(device(), rt_pipeline, 0, 1, handle_size, handles.data());

    vkt::Buffer sbt_buffer(*m_device, aligned_handle_size, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, vkt::device_address);
    uint8_t* sbt_data = static_cast<uint8_t*>(sbt_buffer.Memory().Map());
    std::memcpy(sbt_data, handles.data(), handle_size);
    sbt_buffer.Memory().Unmap();

    VkStridedDeviceAddressRegionKHR rgen_region = {};
    rgen_region.deviceAddress = sbt_buffer.Address();
    rgen_region.stride = aligned_handle_size;
    rgen_region.size = aligned_handle_size;
    VkStridedDeviceAddressRegionKHR empty_region = {};

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_layout, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-micromap-11640");
    vk::CmdTraceRaysKHR(m_command_buffer, &rgen_region, &empty_region, &empty_region, &empty_region, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    vk::DestroyPipeline(device(), rt_pipeline, nullptr);
}
