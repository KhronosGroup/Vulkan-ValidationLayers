/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/shader_helper.h"
#include "../framework/feature_requirements.h"
#include "generated/vk_extension_helper.h"
#include "../layers/utils/vk_layer_utils.h"
#include "../framework/descriptor_helper.h"

void RayTracingTest::InitFrameworkForRayTracingTest(VkValidationFeaturesEXT* enabled_features /*= nullptr*/) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(enabled_features));
}

TEST_F(PositiveRayTracing, GetAccelerationStructureBuildSizes) {
    TEST_DESCRIPTION("Test enabled features for GetAccelerationStructureBuildSizes");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    uint32_t max_primitives_count = 0;
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info = vku::InitStructHelper();
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
}

TEST_F(PositiveRayTracing, AccelerationStructureReference) {
    TEST_DESCRIPTION("Test device side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    m_commandBuffer->begin();
    // Build Bottom Level Acceleration Structure
    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
    blas->BuildCmdBuffer(m_commandBuffer->handle());
    m_commandBuffer->end();

    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();

    m_commandBuffer->begin();
    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);
    tlas.BuildCmdBuffer(m_commandBuffer->handle());
    m_commandBuffer->end();

    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(PositiveRayTracing, HostAccelerationStructureReference) {
    TEST_DESCRIPTION("Test host side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    // Build Bottom Level Acceleration Structure
    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device));
    blas->BuildHost();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostTopLevel(*m_device, blas);
    tlas.BuildHost();
}

TEST_F(PositiveRayTracing, CreateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer.handle();
    as_create_info.size = 4096;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

TEST_F(PositiveRayTracing, StridedDeviceAddressRegion) {
    TEST_DESCRIPTION("Test different valid VkStridedDeviceAddressRegionKHR");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Create ray tracing pipeline
    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;
    {
        VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
        VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

        const vkt::PipelineLayout pipeline_layout(*m_device, {});

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
        raytracing_pipeline_ci.layout = pipeline_layout.handle();

        const VkResult result = vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                                 &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
        ASSERT_EQ(VK_SUCCESS, result);
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
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
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &valid_region, &stridebufregion, &stridebufregion, 100,
                            100, 1);
    }

    // pRayGenShaderBindingTable->size == 0, deviceAddress is invalid => region is considered unused so no error
    {
        VkStridedDeviceAddressRegionKHR empty_region = stridebufregion;
        empty_region.deviceAddress += buffer.create_info().size + 128;
        empty_region.size = 0;
        empty_region.stride = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &empty_region, &stridebufregion, &stridebufregion, 100,
                            100, 1);
    }

    m_commandBuffer->end();

    m_commandBuffer->QueueCommandBuffer(true);

    m_device->wait();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

TEST_F(PositiveRayTracing, BarrierAccessMaskAccelerationStructureRayQueryEnabledRTXDisabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is enabled, as well as feature."
        "RTX extensions are disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    vkt::Buffer buffer(*m_device, 32);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    m_commandBuffer->begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();
}

TEST_F(PositiveRayTracing, BarrierAccessMaskAccelerationStructureRayQueryEnabledRTXEnabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is enabled, as well as feature."
        "RTX extensions are enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    vkt::Buffer buffer(*m_device, 32);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    m_commandBuffer->begin();

    // specify VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR as srcStageMask and dstStageMask
    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    image_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();
}

TEST_F(PositiveRayTracing, BarrierSync1NoCrash) {
    TEST_DESCRIPTION("Regression test for nullptr crash when Sync1 barrier API is used for acceleration structure accesses");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // This stage can not be used with ACCELERATION_STRUCTURE_READ access when ray query is disabled, but VVL also should not crash.
    constexpr VkPipelineStageFlags invalid_src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_errorMonitor->SetUnexpectedError("VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), invalid_src_stage, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 1, &barrier, 0,
                           nullptr, 0, nullptr);
    m_commandBuffer->end();
}

TEST_F(PositiveRayTracing, BuildAccelerationStructuresList) {
    TEST_DESCRIPTION("Build a list of destination acceleration structures, then do an update build on that same list");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr size_t blas_count = 10;

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
    for (size_t i = 0; i < blas_count; ++i) {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
        blas_vec.emplace_back(std::move(blas));
    }

    m_commandBuffer->begin();
    vkt::as::BuildAccelerationStructuresKHR(m_commandBuffer->handle(), blas_vec);

    for (auto& blas : blas_vec) {
        blas.SetSrcAS(blas.GetDstAS());
        blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
        blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    }

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();

    m_commandBuffer->begin();
    vkt::as::BuildAccelerationStructuresKHR(m_commandBuffer->handle(), blas_vec);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(PositiveRayTracing, BuildAccelerationStructuresDeferredOperation) {
    TEST_DESCRIPTION("Call vkBuildAccelerationStructuresKHR with a valid VkDeferredOperationKHR object");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "vkGetDeferredOperationResultKHR not supported by MockICD";
    }

    VkDeferredOperationKHR deferred_op = VK_NULL_HANDLE;
    vk::CreateDeferredOperationKHR(m_device->handle(), 0, &deferred_op);

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetDeferredOp(deferred_op);
    blas.BuildHost();

    vk::DestroyDeferredOperationKHR(m_device->handle(), deferred_op, nullptr);
}

TEST_F(PositiveRayTracing, AccelerationStructuresOverlappingMemory) {
    TEST_DESCRIPTION(
        "Validate acceleration structure building when source/destination acceleration structures and scratch buffers may "
        "overlap.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr size_t blas_count = 3;

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = (1u << 18) * blas_count;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test using non overlapping memory chunks from the same buffer in multiple builds
    // The scratch buffer is used in multiple builds but bound at different offsets, so no validation error should be issued
    {
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        auto scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, scratch_buffer_ci, vkt::no_mem);
        vk::BindBufferMemory(device(), scratch_buffer->handle(), buffer_memory.handle(), 0);
        std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
        VkDeviceSize consumed_buffer_size = 0;
        for (size_t i = 0; i < blas_count; ++i) {
            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.SetScratchBuffer(scratch_buffer);
            blas.SetDeviceScratchOffset(consumed_buffer_size);
            consumed_buffer_size = blas.GetSizeInfo().buildScratchSize;
            consumed_buffer_size = Align<VkDeviceSize>(consumed_buffer_size, 4096);
            blas_vec.emplace_back(std::move(blas));
        }

        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(m_commandBuffer->handle(), blas_vec);
        m_commandBuffer->end();

        m_commandBuffer->QueueCommandBuffer();
        m_device->wait();
    }
}

TEST_F(PositiveRayTracing, AccelerationStructuresReuseScratchMemory) {
    TEST_DESCRIPTION("Repro https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Allocate a memory chunk that will be used as backing memory for scratch buffer
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 1u << 18;
    vkt::DeviceMemory common_scratch_memory(*m_device, alloc_info);

    vkt::CommandBuffer cmd_buffer_frame_0(*m_device, m_commandPool);
    vkt::CommandBuffer cmd_buffer_frame_1(*m_device, m_commandPool);
    vkt::CommandBuffer cmd_buffer_frame_2(*m_device, m_commandPool);

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_0;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_1;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_2;

    auto scratch_buffer_frame_0 = std::make_shared<vkt::Buffer>();
    auto scratch_buffer_frame_1 = std::make_shared<vkt::Buffer>();
    auto scratch_buffer_frame_2 = std::make_shared<vkt::Buffer>();

    vkt::Fence fence_frame_0(*m_device);
    vkt::Fence fence_frame_1(*m_device);
    vkt::Fence fence_frame_2(*m_device);

    // Frame 0
    {
        // Nothing to wait for, resources used in frame 0 will be released in frame 2

        // Create scratch buffer
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scratch_buffer_frame_0->init_no_mem(*m_device, scratch_buffer_ci);

        // Bind memory to scratch buffer
        vk::BindBufferMemory(device(), scratch_buffer_frame_0->handle(), common_scratch_memory.handle(), 0);

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(scratch_buffer_frame_0);
        blas_vec_frame_0.emplace_back(std::move(blas));
        cmd_buffer_frame_0.begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_0.handle(), blas_vec_frame_0);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = scratch_buffer_frame_0->handle();
        barrier.size = scratch_buffer_ci.size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_0.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_0.end();
        m_default_queue->submit(cmd_buffer_frame_0, fence_frame_0);
    }

    // Frame 1
    {
        // Still nothing to wait for

        // Create scratch buffer
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scratch_buffer_frame_1->init_no_mem(*m_device, scratch_buffer_ci);

        // Bind memory to scratch buffer
        vk::BindBufferMemory(device(), scratch_buffer_frame_1->handle(), common_scratch_memory.handle(), 0);

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(scratch_buffer_frame_1);
        blas_vec_frame_1.emplace_back(std::move(blas));
        cmd_buffer_frame_1.begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_1.handle(), blas_vec_frame_1);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = scratch_buffer_frame_1->handle();
        barrier.size = scratch_buffer_ci.size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_1.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_1.end();
        m_default_queue->submit(cmd_buffer_frame_1, fence_frame_1);
    }

    // Frame 2
    {
        // Free resources from frame 0
        fence_frame_0.wait(kWaitTimeout);
        // Destroying buffer triggers VUID-vkDestroyBuffer-buffer-00922, it is still considered in use by cmd_buffer_frame_0 this
        // should not happen assuming synchronization is correct
        // Adding "fence_frame_1.wait(kWaitTimeout);" used to solve this issue.
        // Using a dedicated memory chunk for each scratch buffer also used to solve it.
        // The issue was that when recording a acceleration structure build command,
        // any buffer indirectly mentioned through a device address used to be added using a call to GetBuffersByAddress.
        // So when recording the build happening on frame 1, given that all scratch buffers have the same base device address,
        // scratch_buffer_frame_0 was *also* be added as a child to cmd_buffer_frame_1.
        // So when destroying it hereinafter, since frame 1 is still in flight, scratch_buffer_frame_0 is still
        // considered in use, so 00922 is triggered.
        // => Solution: buffers obtained through a call to GetBuffersByAddress should not get added as children,
        // since there is no 1 to 1 mapping between a device address and a buffer.
        scratch_buffer_frame_0 = nullptr;  // Remove reference
        blas_vec_frame_0.clear();          // scratch_buffer_frame_0 will be destroyed in this call

        // Create scratch buffer
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scratch_buffer_frame_2->init_no_mem(*m_device, scratch_buffer_ci);

        // Bind memory to scratch buffer
        vk::BindBufferMemory(device(), scratch_buffer_frame_2->handle(), common_scratch_memory.handle(), 0);

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(scratch_buffer_frame_2);
        blas_vec_frame_2.emplace_back(std::move(blas));
        cmd_buffer_frame_2.begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_2.handle(), blas_vec_frame_2);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = scratch_buffer_frame_2->handle();
        barrier.size = scratch_buffer_ci.size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_2.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_2.end();
        m_default_queue->submit(cmd_buffer_frame_2, fence_frame_2);
    }

    fence_frame_1.wait(kWaitTimeout);
    fence_frame_2.wait(kWaitTimeout);
}

TEST_F(PositiveRayTracing, AccelerationStructuresDedicatedScratchMemory) {
    TEST_DESCRIPTION(
        "Repro https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461"
        "This time, each scratch buffer has its own memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::CommandBuffer cmd_buffer_frame_0(*m_device, m_commandPool);
    vkt::CommandBuffer cmd_buffer_frame_1(*m_device, m_commandPool);
    vkt::CommandBuffer cmd_buffer_frame_2(*m_device, m_commandPool);

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_0;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_1;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_2;

    vkt::Fence fence_frame_0(*m_device);
    vkt::Fence fence_frame_1(*m_device);
    vkt::Fence fence_frame_2(*m_device);

    // Frame 0
    {
        // Nothing to wait for, resources used in frame 0 will be released in frame 2

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

        blas_vec_frame_0.emplace_back(std::move(blas));
        cmd_buffer_frame_0.begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_0.handle(), blas_vec_frame_0);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = blas_vec_frame_0[0].GetScratchBuffer()->handle();
        barrier.size = blas_vec_frame_0[0].GetScratchBuffer()->create_info().size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_0.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_0.end();
        m_default_queue->submit(cmd_buffer_frame_0, fence_frame_0);
    }

    // Frame 1
    {
        // Still nothing to wait for

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_vec_frame_1.emplace_back(std::move(blas));
        cmd_buffer_frame_1.begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_1.handle(), blas_vec_frame_1);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = blas_vec_frame_1[0].GetScratchBuffer()->handle();
        barrier.size = blas_vec_frame_1[0].GetScratchBuffer()->create_info().size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_1.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_1.end();
        m_default_queue->submit(cmd_buffer_frame_1, fence_frame_1);
    }

    // Frame 2
    {
        // Free resources from frame 0
        fence_frame_0.wait(kWaitTimeout);
        blas_vec_frame_0.clear();  // No validation error

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_vec_frame_2.emplace_back(std::move(blas));
        cmd_buffer_frame_2.begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_2.handle(), blas_vec_frame_2);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = blas_vec_frame_2[0].GetScratchBuffer()->handle();
        barrier.size = blas_vec_frame_2[0].GetScratchBuffer()->create_info().size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_2.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_2.end();
        m_default_queue->submit(cmd_buffer_frame_2, fence_frame_2);
    }

    fence_frame_1.wait(kWaitTimeout);
    fence_frame_2.wait(kWaitTimeout);
}

TEST_F(PositiveRayTracing, CmdBuildAccelerationStructuresIndirect) {
    TEST_DESCRIPTION("basic usage of vkCmdBuildAccelerationStructuresIndirectKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureIndirectBuild);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    m_commandBuffer->begin();
    blas.BuildCmdBufferIndirect(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(PositiveRayTracing, ScratchBufferCorrectAddressSpaceOpBuild) {
    TEST_DESCRIPTION(
        "Have two scratch buffers bound to the same memory, with one of them being not big enough for an acceleration structure "
        "build, but the other one is. If the buffer addresses of those buffers are the same, 03671 should not fire");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    auto size_info = blas.GetSizeInfo();
    if (size_info.buildScratchSize <= 64) {
        GTEST_SKIP() << "Need a big scratch size, skipping test.";
    }

    VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_struct_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(acc_struct_properties);
    VkDeviceSize scratch_size = size_info.buildScratchSize + acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
    scratch_size = Align<VkDeviceSize>(scratch_size, acc_struct_properties.minAccelerationStructureScratchOffsetAlignment);

    // Allocate buffer memory separately so that it can be large enough. Scratch buffer size will be smaller.
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = scratch_size;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    VkBufferCreateInfo small_buffer_ci = vku::InitStructHelper();
    small_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    small_buffer_ci.size = 64;
    small_buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    auto small_scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, small_buffer_ci, vkt::no_mem);
    small_scratch_buffer->bind_memory(buffer_memory, 0);

    small_buffer_ci.size = alloc_info.allocationSize;
    auto big_scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, small_buffer_ci, vkt::no_mem);
    big_scratch_buffer->bind_memory(buffer_memory, 0);
    const VkDeviceAddress big_scratch_address = big_scratch_buffer->address();
    if (big_scratch_address != small_scratch_buffer->address()) {
        GTEST_SKIP() << "Binding two buffers to the same memory does not yield identical buffer addresses, skipping test.";
    }

    m_commandBuffer->begin();
    blas.SetScratchBuffer(small_scratch_buffer);
    blas.BuildCmdBuffer(*m_commandBuffer);
    m_commandBuffer->end();
}

TEST_F(PositiveRayTracing, BasicTraceRays) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    layout(location = 0) rayPayloadEXT vec3 hit;

    void main() {
      traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
    }
)glsl";
    pipeline.SetRayGenShader(ray_gen);

    const char* miss = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(location = 0) rayPayloadInEXT vec3 hit;

    void main() {
        hit = vec3(0.1, 0.2, 0.3);
    }
)glsl";
    pipeline.AddMissShader(miss);

    const char* closest_hit = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(location = 0) rayPayloadInEXT vec3 hit;
    hitAttributeEXT vec2 baryCoord;

    void main() {
      const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
      hit = barycentricCoords;

    }
)glsl";
    pipeline.AddClosestHitShader(closest_hit);

    // Add TLAS binding
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet()->set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}
