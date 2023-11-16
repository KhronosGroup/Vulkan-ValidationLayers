/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/ray_tracing_helper_nv.h"
#include "../framework/descriptor_helper.h"
#include "../layers/utils/vk_layer_utils.h"

TEST_F(NegativeRayTracingNV, AccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));

    VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_props);

    RETURN_IF_SKIP(InitState());

    uint32_t maxBlocks = ray_tracing_props.maxDescriptorSetAccelerationStructures;
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of descriptor set acceleration structures, skipping tests";
    }
    if (maxBlocks < 1) {
        GTEST_SKIP() << "Test requires maxDescriptorSetAccelerationStructures >= 1";
    }

    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutBinding dslb = {};
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    for (uint32_t i = 0; i <= maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02381");
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, ValidateGeometry) {
    TEST_DESCRIPTION("Validate acceleration structure geometries.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer ibo;
    ibo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer tbo;
    tbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer aabbbo;
    aabbbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkBufferCreateInfo unbound_buffer_ci = vku::InitStructHelper();
    unbound_buffer_ci.size = 1024;
    unbound_buffer_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    vkt::Buffer unbound_buffer;
    unbound_buffer.init_no_mem(*m_device, unbound_buffer_ci);

    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indicies = {0, 1, 2};
    constexpr std::array aabbs = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    constexpr std::array transforms = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo.memory().map();
    std::memcpy(mapped_vbo_buffer_data, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo.memory().unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo.memory().map();
    std::memcpy(mapped_ibo_buffer_data, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo.memory().unmap();

    uint8_t *mapped_tbo_buffer_data = (uint8_t *)tbo.memory().map();
    std::memcpy(mapped_tbo_buffer_data, (uint8_t *)transforms.data(), sizeof(float) * transforms.size());
    tbo.memory().unmap();

    uint8_t *mapped_aabbbo_buffer_data = (uint8_t *)aabbbo.memory().map();
    std::memcpy(mapped_aabbbo_buffer_data, (uint8_t *)aabbs.data(), sizeof(float) * aabbs.size());
    aabbbo.memory().unmap();

    VkGeometryNV valid_geometry_triangles = vku::InitStructHelper();
    valid_geometry_triangles.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    valid_geometry_triangles.geometry.triangles = vku::InitStructHelper();
    valid_geometry_triangles.geometry.triangles.vertexData = vbo.handle();
    valid_geometry_triangles.geometry.triangles.vertexOffset = 0;
    valid_geometry_triangles.geometry.triangles.vertexCount = 3;
    valid_geometry_triangles.geometry.triangles.vertexStride = 12;
    valid_geometry_triangles.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    valid_geometry_triangles.geometry.triangles.indexData = ibo.handle();
    valid_geometry_triangles.geometry.triangles.indexOffset = 0;
    valid_geometry_triangles.geometry.triangles.indexCount = 3;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.transformData = tbo.handle();
    valid_geometry_triangles.geometry.triangles.transformOffset = 0;
    valid_geometry_triangles.geometry.aabbs = vku::InitStructHelper();

    VkGeometryNV valid_geometry_aabbs = vku::InitStructHelper();
    valid_geometry_aabbs.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    valid_geometry_aabbs.geometry.triangles = vku::InitStructHelper();
    valid_geometry_aabbs.geometry.aabbs = vku::InitStructHelper();
    valid_geometry_aabbs.geometry.aabbs.aabbData = aabbbo.handle();
    valid_geometry_aabbs.geometry.aabbs.numAABBs = 1;
    valid_geometry_aabbs.geometry.aabbs.offset = 0;
    valid_geometry_aabbs.geometry.aabbs.stride = 24;

    const auto GetCreateInfo = [](const VkGeometryNV &geometry) {
        VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
        as_create_info.info = vku::InitStructHelper();
        as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        as_create_info.info.instanceCount = 0;
        as_create_info.info.geometryCount = 1;
        as_create_info.info.pGeometries = &geometry;
        return as_create_info;
    };

    VkAccelerationStructureNV as;

    // Invalid vertex format.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R64_UINT;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexFormat-02430");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - not multiple of component size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02429");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 12 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex buffer - no such buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = VkBuffer(123456789);

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexData-parameter");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#if 0
    // XXX Subtest disabled because this is the wrong VUID.
    // No VUIDs currently exist to require memory is bound (spec bug).
    // Invalid vertex buffer - no memory bound.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = unbound_buffer.handle();

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#endif

    // Invalid index offset - not multiple of index size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02432");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02431");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index count - must be 0 if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = VK_NULL_HANDLE;
        geometry.geometry.triangles.indexCount = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexCount-02436");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index data - must be VK_NULL_HANDLE if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = ibo.handle();
        geometry.geometry.triangles.indexCount = 0;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexData-02434");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid transform offset - not multiple of 16.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02438");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid transform offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02437");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid aabb offset - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02440");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 8 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02439");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb stride - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-stride-02441");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV or VK_GEOMETRY_TYPE_AABBS_NV
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryNV-geometryType-03503");
        m_errorMonitor->SetUnexpectedError("VUID-VkGeometryNV-geometryType-parameter");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateCreateAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();

    VkAccelerationStructureNV as = VK_NULL_HANDLE;

    // Top level can not have geometry
    {
        VkAccelerationStructureCreateInfoNV bad_top_level_create_info = as_create_info;
        bad_top_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
        bad_top_level_create_info.info.instanceCount = 0;
        bad_top_level_create_info.info.geometryCount = 1;
        bad_top_level_create_info.info.pGeometries = &geometry;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-02425");
        vk::CreateAccelerationStructureNV(device(), &bad_top_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Bot level can not have instances
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_bot_level_create_info.info.instanceCount = 1;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-02426");
        vk::CreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Type must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR;
        bad_bot_level_create_info.info.instanceCount = 0;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-04623");
        vk::CreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not prefer both fast trace and fast build
    {
        VkAccelerationStructureCreateInfoNV bad_flags_level_create_info = as_create_info;
        bad_flags_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_flags_level_create_info.info.instanceCount = 0;
        bad_flags_level_create_info.info.geometryCount = 1;
        bad_flags_level_create_info.info.pGeometries = &geometry;
        bad_flags_level_create_info.info.flags =
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-flags-02592");
        vk::CreateAccelerationStructureNV(device(), &bad_flags_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not have geometry or instance for compacting
    {
        VkAccelerationStructureCreateInfoNV bad_compacting_as_create_info = as_create_info;
        bad_compacting_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_compacting_as_create_info.info.instanceCount = 0;
        bad_compacting_as_create_info.info.geometryCount = 1;
        bad_compacting_as_create_info.info.pGeometries = &geometry;
        bad_compacting_as_create_info.info.flags = 0;
        bad_compacting_as_create_info.compactedSize = 1024;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421");
        vk::CreateAccelerationStructureNV(device(), &bad_compacting_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not mix different geometry types into single bottom level acceleration structure
    {
        VkGeometryNV aabb_geometry = vku::InitStructHelper();
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        aabb_geometry.geometry.triangles = vku::InitStructHelper();
        aabb_geometry.geometry.aabbs = vku::InitStructHelper();
        // Buffer contents do not matter for this test.
        aabb_geometry.geometry.aabbs.aabbData = geometry.geometry.triangles.vertexData;
        aabb_geometry.geometry.aabbs.numAABBs = 1;
        aabb_geometry.geometry.aabbs.offset = 0;
        aabb_geometry.geometry.aabbs.stride = 24;

        std::vector<VkGeometryNV> geometries = {geometry, aabb_geometry};

        VkAccelerationStructureCreateInfoNV mix_geometry_types_as_create_info = as_create_info;
        mix_geometry_types_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        mix_geometry_types_as_create_info.info.instanceCount = 0;
        mix_geometry_types_as_create_info.info.geometryCount = static_cast<uint32_t>(geometries.size());
        mix_geometry_types_as_create_info.info.pGeometries = geometries.data();
        mix_geometry_types_as_create_info.info.flags = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureInfoNV-type-02786");
        vk::CreateAccelerationStructureNV(device(), &mix_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateBindAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;
    as_create_info.info.instanceCount = 0;

    vkt::AccelerationStructure as(*m_device, as_create_info, false);

    VkMemoryRequirements as_memory_requirements = as.memory_requirements().memoryRequirements;

    VkBindAccelerationStructureMemoryInfoNV as_bind_info = vku::InitStructHelper();
    as_bind_info.accelerationStructure = as.handle();

    VkMemoryAllocateInfo as_memory_alloc = vku::InitStructHelper();
    as_memory_alloc.allocationSize = as_memory_requirements.size;
    ASSERT_TRUE(m_device->phy().set_memory_type(as_memory_requirements.memoryTypeBits, &as_memory_alloc, 0));

    // Can not bind already freed memory
    {
        VkDeviceMemory as_memory_freed = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_freed));
        vk::FreeMemory(device(), as_memory_freed, NULL);

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_freed = as_bind_info;
        as_bind_info_freed.memory = as_memory_freed;
        as_bind_info_freed.memoryOffset = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-parameter");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_freed);
        m_errorMonitor->VerifyFound();
    }

    // Can not bind with bad alignment
    if (as_memory_requirements.alignment > 1) {
        VkMemoryAllocateInfo as_memory_alloc_bad_alignment = as_memory_alloc;
        as_memory_alloc_bad_alignment.allocationSize += 1;

        VkDeviceMemory as_memory_bad_alignment = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc_bad_alignment, NULL, &as_memory_bad_alignment));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_alignment = as_bind_info;
        as_bind_info_bad_alignment.memory = as_memory_bad_alignment;
        as_bind_info_bad_alignment.memoryOffset = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_alignment);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_alignment, NULL);
    }

    // Can not bind with offset outside the allocation
    {
        VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
        as_bind_info_bad_offset.memory = as_memory_bad_offset;
        as_bind_info_bad_offset.memoryOffset =
            (as_memory_alloc.allocationSize + as_memory_requirements.alignment) & ~(as_memory_requirements.alignment - 1);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03621");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_offset, NULL);
    }

    // Can not bind with offset that doesn't leave enough size
    {
        VkDeviceSize offset = (as_memory_requirements.size - 1) & ~(as_memory_requirements.alignment - 1);
        if (offset > 0 && (as_memory_requirements.size < (as_memory_alloc.allocationSize - as_memory_requirements.alignment))) {
            VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
            as_bind_info_bad_offset.memory = as_memory_bad_offset;
            as_bind_info_bad_offset.memoryOffset = offset;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624");
            vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_offset, NULL);
        }
    }

    // Can not bind with memory that has unsupported memory type
    {
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);

        uint32_t supported_memory_type_bits = as_memory_requirements.memoryTypeBits;
        uint32_t unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~supported_memory_type_bits;
        if (unsupported_mem_type_bits != 0) {
            VkMemoryAllocateInfo as_memory_alloc_bad_type = as_memory_alloc;
            ASSERT_TRUE(m_device->phy().set_memory_type(unsupported_mem_type_bits, &as_memory_alloc_bad_type, 0));

            VkDeviceMemory as_memory_bad_type = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc_bad_type, NULL, &as_memory_bad_type));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_type = as_bind_info;
            as_bind_info_bad_type.memory = as_memory_bad_type;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
            vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_type);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_type, NULL);
        }
    }

    // Can not bind memory twice
    {
        vkt::AccelerationStructure as_twice(*m_device, as_create_info, false);

        VkDeviceMemory as_memory_twice_1 = VK_NULL_HANDLE;
        VkDeviceMemory as_memory_twice_2 = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_1));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_2));
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_1 = as_bind_info;
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_2 = as_bind_info;
        as_bind_info_twice_1.accelerationStructure = as_twice.handle();
        as_bind_info_twice_2.accelerationStructure = as_twice.handle();
        as_bind_info_twice_1.memory = as_memory_twice_1;
        as_bind_info_twice_2.memory = as_memory_twice_2;

        ASSERT_EQ(VK_SUCCESS, vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_1));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(NegativeRayTracingNV, ValidateWriteDescriptorSetAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
                           });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkWriteDescriptorSetAccelerationStructureNV acc = vku::InitStructHelper();
    acc.accelerationStructureCount = 1;
    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    acc.pAccelerationStructures = &top_level_as.handle();
    descriptor_write.pNext = &acc;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(NegativeRayTracingNV, ValidateCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

    const vkt::Buffer bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device);

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-recording");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Incompatible type
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_type = bot_level_as_create_info.info;
    as_build_info_with_incompatible_type.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    as_build_info_with_incompatible_type.instanceCount = 1;
    as_build_info_with_incompatible_type.geometryCount = 0;

    // This is duplicated since it triggers one error for different types and one error for lower instance count - the
    // build info is incompatible but still needs to be valid to get past the stateless checks.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_type, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible flags
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_flags = bot_level_as_create_info.info;
    as_build_info_with_incompatible_flags.flags = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_flags, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible build size
    VkGeometryNV geometry_with_more_vertices = geometry;
    geometry_with_more_vertices.geometry.triangles.vertexCount += 1;

    VkAccelerationStructureInfoNV as_build_info_with_incompatible_geometry = bot_level_as_create_info.info;
    as_build_info_with_incompatible_geometry.pGeometries = &geometry_with_more_vertices;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_geometry, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer too small
    VkBufferCreateInfo too_small_scratch_buffer_info = vku::InitStructHelper();
    too_small_scratch_buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    too_small_scratch_buffer_info.size = 1;
    vkt::Buffer too_small_scratch_buffer(*m_device, too_small_scratch_buffer_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, too_small_scratch_buffer.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer with offset too small
    VkDeviceSize scratch_buffer_offset = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(),
                                        scratch_buffer_offset);
    m_errorMonitor->VerifyFound();

    // Src must have been built before
    vkt::AccelerationStructure bot_level_as_updated(*m_device, bot_level_as_create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02489");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                        bot_level_as_updated.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Src must have been built before with the VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV flag
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02490");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                        bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid scratch buffer (invalid usage)
    VkBufferCreateInfo create_info = vku::InitStructHelper();
    create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    const vkt::Buffer bot_level_as_invalid_scratch = bot_level_as.create_scratch_buffer(*m_device, &create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-scratch-02781");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_invalid_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid instance data.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-instanceData-02782");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info,
                                        bot_level_as_invalid_scratch.handle(), 0, VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE,
                                        bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // must be called outside renderpass
    InitRenderTarget();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-renderpass");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, ObjInUseCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

    const vkt::Buffer bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device);

    m_commandBuffer->begin();
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), ibo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), vbo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), bot_level_as_scratch.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    vk::DestroyAccelerationStructureNV(device(), bot_level_as.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeRayTracingNV, ValidateGetAccelerationStructureHandle) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    // Not enough space for the handle
    {
        vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint8_t), &handle);
        m_errorMonitor->VerifyFound();
    }

    // No memory bound to acceleration structure
    {
        vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info, /*init_memory=*/false);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint64_t), &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateCmdCopyAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure copying.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.instanceCount = 0;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructure src_as(*m_device, as_create_info);
    vkt::AccelerationStructure dst_as(*m_device, as_create_info);
    vkt::AccelerationStructure dst_as_without_mem(*m_device, as_create_info, false);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    const vkt::Buffer bot_level_as_scratch = src_as.create_scratch_buffer(*m_device);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-src-04963");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        src_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1u, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-recording");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Src must have been created with allow compaction flag
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-src-03411");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV);
    m_errorMonitor->VerifyFound();

    // Dst must have been bound with memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-dst-07792");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as_without_mem.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);

    m_errorMonitor->VerifyFound();

    // mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-03410");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR);
    m_errorMonitor->VerifyFound();

    // mode must be a valid VkCopyAccelerationStructureModeKHR value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-parameter");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR);
    m_errorMonitor->VerifyFound();

    // This command must only be called outside of a render pass instance
    InitRenderTarget();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-renderpass");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();

    vkt::DeviceMemory host_memory;
    host_memory.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(
                                    *m_device, dst_as_without_mem.memory_requirements().memoryRequirements,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    VkBindAccelerationStructureMemoryInfoNV bind_info = vku::InitStructHelper();
    bind_info.accelerationStructure = dst_as_without_mem.handle();
    bind_info.memory = host_memory.handle();
    vk::BindAccelerationStructureMemoryNV(*m_device, 1, &bind_info);

    uint64_t handle;
    vk::GetAccelerationStructureHandleNV(*m_device, dst_as_without_mem.handle(), sizeof(uint64_t), &handle);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-buffer-03719");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as_without_mem.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-buffer-03718");
    const vkt::Buffer bot_level_as_scratch2 = dst_as_without_mem.create_scratch_buffer(*m_device);
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        dst_as_without_mem.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), src_as.handle(), dst_as_without_mem.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();
}
