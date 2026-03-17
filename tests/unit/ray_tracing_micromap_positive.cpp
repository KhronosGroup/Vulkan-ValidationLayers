/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"

class PositiveRayTracingMicromap : public RayTracingTest {};

TEST_F(PositiveRayTracingMicromap, BasicEXT) {
    TEST_DESCRIPTION("Test building an opacity micromap then building an acceleration structure with that");

    // Mask data for 2 levels of subdivision. Middle triangle is index 1, so drop that one out.
    // Bit string for middle missing is '1011' (0 on the left). In number form, that's 0xd.
    // Extending the Sierpinski-esque pattern out one level is 0xdd0d
    uint32_t test_mask = 0xdd0d;

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::micromap);
    AddRequiredFeature(vkt::Feature::micromapHostCommands);

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VkMemoryAllocateFlagsInfo allocate_da_flag_info = vku::InitStructHelper();
    allocate_da_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    // Create a buffer with the mask and index data
    vkt::Buffer micromap_data_buffer(
        *m_device, 2 * 1048576 /*XXX*/,
        VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);

    VkDeviceAddress micromap_address = micromap_data_buffer.Address();

    // Fill out VkMicromapUsageEXT with size information
    VkMicromapUsageEXT mm_usage = {};
    mm_usage.count = 1;

    const int triangle_offset = 0;
    const int index_offset = 256;
    const int data_offset = 512;

    mm_usage.subdivisionLevel = 2;
    mm_usage.format = VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT;

    {
        uint32_t* data = (uint32_t*)micromap_data_buffer.Memory().Map();

        VkMicromapTriangleEXT* tri = (VkMicromapTriangleEXT*)&data[triangle_offset / 4];
        tri->dataOffset = 0;
        tri->subdivisionLevel = uint16_t(mm_usage.subdivisionLevel);
        tri->format = uint16_t(mm_usage.format);

        // Micromap data
        // Just replicate for testing higher subdivision
        {
            uint32_t mask_word = test_mask | (test_mask << 16);
            int words = ((1 << (2 * mm_usage.subdivisionLevel)) + 31) / 32;
            for (int i = 0; i < words; i++) {
                data[data_offset / 4 + i] = mask_word;
            }
        }

        // Index information
        data[index_offset / 4] = 0;
    }

    VkMicromapBuildInfoEXT mm_build_info = vku::InitStructHelper();

    mm_build_info.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    mm_build_info.flags = 0;
    mm_build_info.mode = VK_BUILD_MICROMAP_MODE_BUILD_EXT;
    mm_build_info.dstMicromap = VK_NULL_HANDLE;
    mm_build_info.usageCountsCount = 1;
    mm_build_info.pUsageCounts = &mm_usage;
    mm_build_info.data.deviceAddress = 0ull;
    mm_build_info.triangleArray.deviceAddress = 0ull;
    mm_build_info.triangleArrayStride = 0;

    VkMicromapBuildSizesInfoEXT size_info = vku::InitStructHelper();
    vk::GetMicromapBuildSizesEXT(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &mm_build_info, &size_info);

    // Create a buffer and micromap on top from the size
    vkt::Buffer micromap_buffer(*m_device, size_info.micromapSize, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);
    // Scratch buffer
    vkt::Buffer ms_buffer(*m_device, size_info.buildScratchSize > 4 ? size_info.buildScratchSize : 4,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_da_flag_info);

    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT ma_ci = vku::InitStructHelper();

    ma_ci.createFlags = 0;
    ma_ci.buffer = micromap_buffer;
    ma_ci.offset = 0;
    ma_ci.size = size_info.micromapSize;
    ma_ci.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    ma_ci.deviceAddress = 0ull;

    VkResult result = vk::CreateMicromapEXT(device(), &ma_ci, nullptr, &micromap);
    ASSERT_EQ(VK_SUCCESS, result);

    // Build the array with vkBuildmicromapsEXT
    {
        // Fill in the pointers we didn't have at size query
        mm_build_info.dstMicromap = micromap;
        mm_build_info.data.deviceAddress = micromap_address + data_offset;
        mm_build_info.triangleArray.deviceAddress = micromap_address + triangle_offset;
        mm_build_info.scratchData.deviceAddress = ms_buffer.Address();

        m_command_buffer.Begin();

        vk::CmdBuildMicromapsEXT(m_command_buffer, 1, &mm_build_info);

        {
            VkMemoryBarrier2 memory_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
                                               NULL,
                                               VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT,
                                               VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT,
                                               VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                               VK_ACCESS_2_MICROMAP_READ_BIT_EXT};
            m_command_buffer.BarrierKHR(memory_barrier);
        }
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    // Create a buffer with the triangle data in it
    static float const vertex_data[6 * 2] = {
        0.25, 0.75, 0.5, 0.25, 0.75, 0.75,
    };
    static uint32_t const index_data[6] = {0, 1, 2};

    vkt::Buffer vertex_buffer(
        *m_device, sizeof(vertex_data) + sizeof(index_data),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    VkDeviceAddress vertex_address = vertex_buffer.Address();

    // Upload data to the vertex buffer.
    {
        char* ptr;

        vk::MapMemory(device(), vertex_buffer.Memory(), 0, VK_WHOLE_SIZE, 0, (void**)&ptr);

        memcpy(ptr, &vertex_data[0], sizeof(vertex_data));
        memcpy(ptr + sizeof(vertex_data), &index_data[0], sizeof(index_data));

        vk::UnmapMemory(device(), vertex_buffer.Memory());
    }

    VkAccelerationStructureBuildSizesInfoKHR bottom_as_build_size_info = vku::InitStructHelper();
    VkAccelerationStructureBuildSizesInfoKHR top_as_build_size_info = vku::InitStructHelper();

    // Create a bottom-level acceleration structure with one triangle
    VkAccelerationStructureGeometryKHR bottom_as_geometry = vku::InitStructHelper();

    bottom_as_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    bottom_as_geometry.geometry.triangles = vku::InitStructHelper();
    bottom_as_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32_SFLOAT;
    bottom_as_geometry.geometry.triangles.vertexData.deviceAddress = vertex_address;
    bottom_as_geometry.geometry.triangles.vertexStride = 8;
    bottom_as_geometry.geometry.triangles.maxVertex = 3;
    bottom_as_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    bottom_as_geometry.geometry.triangles.indexData.deviceAddress = vertex_address + sizeof(vertex_data);
    bottom_as_geometry.geometry.triangles.transformData.deviceAddress = 0;
    bottom_as_geometry.flags = 0;

    VkAccelerationStructureTrianglesOpacityMicromapEXT opacity_geometry_micromap = vku::InitStructHelper();
    opacity_geometry_micromap.indexType = VK_INDEX_TYPE_UINT32;
    opacity_geometry_micromap.indexBuffer.deviceAddress = micromap_address + index_offset;
    opacity_geometry_micromap.indexStride = 0;
    opacity_geometry_micromap.baseTriangle = 0;
    opacity_geometry_micromap.micromap = micromap;
    bottom_as_geometry.geometry.triangles.pNext = &opacity_geometry_micromap;

    VkAccelerationStructureBuildGeometryInfoKHR bottomASInfo = vku::InitStructHelper();
    bottomASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    bottomASInfo.flags = 0;
    bottomASInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    bottomASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    bottomASInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    bottomASInfo.geometryCount = 1;
    bottomASInfo.pGeometries = &bottom_as_geometry;
    bottomASInfo.ppGeometries = NULL;
    bottomASInfo.scratchData.deviceAddress = 0;

    uint32_t bottom_max_primitive_counts = 1;
    vk::GetAccelerationStructureBuildSizesKHR(*m_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &bottomASInfo,
                                              &bottom_max_primitive_counts, &bottom_as_build_size_info);

    vkt::Buffer bottom_as_buffer(*m_device, bottom_as_build_size_info.accelerationStructureSize,
                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_da_flag_info);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.createFlags = 0;
    as_create_info.buffer = bottom_as_buffer;
    as_create_info.offset = 0;
    as_create_info.size = bottom_as_build_size_info.accelerationStructureSize;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkAccelerationStructureKHR bottomAS, topAS;

    result = vk::CreateAccelerationStructureKHR(*m_device, &as_create_info, NULL, &bottomAS);
    ASSERT_EQ(VK_SUCCESS, result);

    vkt::Buffer instance_buffer(
        *m_device, 2 * sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    {
        VkAccelerationStructureInstanceKHR* instance = (VkAccelerationStructureInstanceKHR*)instance_buffer.Memory().Map();

        memset(instance, 0, 2 * sizeof(VkAccelerationStructureInstanceKHR));

        instance[0].transform.matrix[0][0] = 1;
        instance[0].transform.matrix[0][1] = 0;
        instance[0].transform.matrix[0][2] = 0;
        instance[0].transform.matrix[0][3] = 0;

        instance[0].transform.matrix[1][0] = 0;
        instance[0].transform.matrix[1][1] = 1;
        instance[0].transform.matrix[1][2] = 0;
        instance[0].transform.matrix[1][3] = 0;

        instance[0].transform.matrix[2][0] = 0;
        instance[0].transform.matrix[2][1] = 0;
        instance[0].transform.matrix[2][2] = 1;
        instance[0].transform.matrix[2][3] = 0;

        instance[0].instanceCustomIndex = 0xdeadfe;
        instance[0].mask = 0xff;
        instance[0].instanceShaderBindingTableRecordOffset = 0;
        instance[0].flags = 0;

        VkAccelerationStructureDeviceAddressInfoKHR as_device_address_info = vku::InitStructHelper();
        as_device_address_info.accelerationStructure = bottomAS;
        instance[0].accelerationStructureReference =
            vk::GetAccelerationStructureDeviceAddressKHR(device(), &as_device_address_info);
    }

    VkAccelerationStructureGeometryKHR top_as_geometry = vku::InitStructHelper();

    top_as_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    top_as_geometry.geometry.instances = vku::InitStructHelper();
    top_as_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
    top_as_geometry.geometry.instances.data.deviceAddress = instance_buffer.Address();
    top_as_geometry.flags = 0;

    VkAccelerationStructureBuildGeometryInfoKHR topASInfo = vku::InitStructHelper();
    topASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    topASInfo.flags = 0;
    topASInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    topASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    topASInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    topASInfo.geometryCount = 1;
    topASInfo.pGeometries = &top_as_geometry;
    topASInfo.ppGeometries = NULL;
    topASInfo.scratchData.deviceAddress = 0;

    uint32_t top_max_primitive_counts = 1;
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &topASInfo,
                                              &top_max_primitive_counts, &top_as_build_size_info);

    vkt::Buffer top_as_buffer(*m_device, top_as_build_size_info.accelerationStructureSize,
                              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_da_flag_info);

    as_create_info.createFlags = 0;
    as_create_info.buffer = top_as_buffer;
    as_create_info.offset = 0;
    as_create_info.size = top_as_build_size_info.accelerationStructureSize;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    result = vk::CreateAccelerationStructureKHR(device(), &as_create_info, NULL, &topAS);
    ASSERT_EQ(VK_SUCCESS, result);

    vkt::Buffer scratch_buffer(*m_device,
                               std::max(bottom_as_build_size_info.buildScratchSize, top_as_build_size_info.buildScratchSize),
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_da_flag_info);

    VkDeviceAddress scratch_address = scratch_buffer.Address();

    // Build the bottom-level acceleration structure
    {
        bottomASInfo.dstAccelerationStructure = bottomAS;
        bottomASInfo.scratchData.deviceAddress = scratch_address;

        VkAccelerationStructureBuildRangeInfoKHR build_range_info = {1, 0, 0, 0};
        const VkAccelerationStructureBuildRangeInfoKHR* p_build_range_info = &build_range_info;

        m_command_buffer.Begin();
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &bottomASInfo, &p_build_range_info);
        VkMemoryBarrier memory_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, NULL, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                                          VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR};
        vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &memory_barrier, 0, 0, 0, 0);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    // Build the top-level acceleration structure
    {
        topASInfo.dstAccelerationStructure = topAS;
        topASInfo.scratchData.deviceAddress = scratch_address;

        VkAccelerationStructureBuildRangeInfoKHR build_range_info = {1, 0, 0, 0};
        const VkAccelerationStructureBuildRangeInfoKHR* p_build_range_info = &build_range_info;

        m_command_buffer.Begin();
        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer, 1, &topASInfo, &p_build_range_info);
        VkMemoryBarrier memory_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, NULL, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                                          VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR};
        vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 1, &memory_barrier, 0, 0, 0, 0);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    vk::DestroyAccelerationStructureKHR(*m_device, topAS, NULL);
    vk::DestroyAccelerationStructureKHR(*m_device, bottomAS, NULL);
    vk::DestroyMicromapEXT(*m_device, micromap, NULL);
}
