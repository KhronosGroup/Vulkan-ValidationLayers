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

class NegativeRayTracing : public VkLayerTest {};

TEST_F(NegativeRayTracing, BarrierAccessAccelerationStructure) {
    TEST_DESCRIPTION("Test barrier with access ACCELERATION_STRUCTURE bit.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    auto mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    auto dependency_info = LvlInitStruct<VkDependencyInfo>();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_commandBuffer->begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03927");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03928");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03927");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03928");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierAccessMaskAccelerationStructureRayQueryDisabledRTXEnabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is disabled."
        "RTX extension enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    auto mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkBufferObj buffer(*m_device, 32);

    auto buffer_barrier = LvlInitStruct<VkBufferMemoryBarrier2>();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    auto image_barrier = LvlInitStruct<VkImageMemoryBarrier2>();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto dependency_info = LvlInitStruct<VkDependencyInfo>();
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-06254");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcAccessMask-06254");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcAccessMask-06254");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-06254");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-dstAccessMask-06254");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-dstAccessMask-06254");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierAccessMaskAccelerationStructureRayQueryDisabledRTXDisabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is disabled."
        "RTX extensions are disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    auto mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkBufferObj buffer(*m_device, 32);

    auto buffer_barrier = LvlInitStruct<VkBufferMemoryBarrier2>();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    auto image_barrier = LvlInitStruct<VkImageMemoryBarrier2>();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto dependency_info = LvlInitStruct<VkDependencyInfo>();
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-06255");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcAccessMask-06255");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcAccessMask-06255");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-06255");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-dstAccessMask-06255");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-dstAccessMask-06255");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierAccessMaskAccelerationStructureRayQueryEnabledRTXEnabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is enabled, as well as feature."
        "RTX extension enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    // Enable ray query extension, but not the feature
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    auto mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkBufferObj buffer(*m_device, 32);

    auto buffer_barrier = LvlInitStruct<VkBufferMemoryBarrier2>();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    auto image_barrier = LvlInitStruct<VkImageMemoryBarrier2>();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto dependency_info = LvlInitStruct<VkDependencyInfo>();
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-dstAccessMask-06256");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierAccessMaskAccelerationStructureRayQueryEnabledRTXDisabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is enabled, as well as feature."
        "RTX extensions are disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    // Enable ray query extension, but not the feature
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        GTEST_SKIP() << "Synchronization2 not supported";
    }

    auto mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkBufferObj buffer(*m_device, 32);

    auto buffer_barrier = LvlInitStruct<VkBufferMemoryBarrier2>();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    auto image_barrier = LvlInitStruct<VkImageMemoryBarrier2>();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto dependency_info = LvlInitStruct<VkDependencyInfo>();
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-dstAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-dstAccessMask-06257");
    m_commandBuffer->PipelineBarrier2KHR(&dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, DescriptorBindingUpdateAfterBindWithAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout setLayout;
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570");
    vk::CreateDescriptorSetLayout(device(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, AccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto accel_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto ray_tracing_pipeline_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&accel_struct_features);
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_pipeline_features);

    if (!accel_struct_features.accelerationStructure) {
        GTEST_SKIP() << "accelerationStructure not supported, skipping test";
    }
    if (!ray_tracing_pipeline_features.rayTracingPipeline) {
        GTEST_SKIP() << "rayTracingPipeline not supported, skipping test";
    }

    auto accel_struct_props = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    GetPhysicalDeviceProperties2(accel_struct_props);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Create one descriptor set layout holding (maxPerStageDescriptorAccelerationStructures + 1) bindings
    // for the same shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxPerStageDescriptorAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03571 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03571 requires "
                "maxPerStageDescriptorAccelerationStructures >= 1, skipping test");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }

    // Create one descriptor set layout with flag VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT holding
    // (maxPerStageDescriptorUpdateAfterBindAccelerationStructures + 1) bindings for the same shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03572 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03572 requires "
                "maxPerStageDescriptorUpdateAfterBindAccelerationStructures >= 1, skipping test");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
            ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }

    // Create one descriptor set layout holding (maxDescriptorSetAccelerationStructures + 1) bindings
    // in total for two different shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxDescriptorSetAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03573 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03573 requires "
                "maxDescriptorSetAccelerationStructures >= 1, skipping test");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = (i % 2) ? VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR : VK_SHADER_STAGE_CALLABLE_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }

    // Create one descriptor set layout with flag VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT holding
    // (maxDescriptorSetUpdateAfterBindAccelerationStructures + 1) bindings in total for two different shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxDescriptorSetUpdateAfterBindAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03574 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03574 requires "
                "maxDescriptorSetUpdateAfterBindAccelerationStructures >= 1, skipping test");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = (i % 2) ? VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR : VK_SHADER_STAGE_CALLABLE_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
            ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeRayTracing, BeginQueryQueryPoolType) {
    TEST_DESCRIPTION("Test CmdBeginQuery with invalid queryPool queryType");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }

    const bool khr_acceleration_structure = IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    const bool nv_ray_tracing = IsExtensionsEnabled(VK_NV_RAY_TRACING_EXTENSION_NAME);
    const bool ext_transform_feedback = IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    const bool rt_maintenance_1 = IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    if (!khr_acceleration_structure && !nv_ray_tracing) {
        GTEST_SKIP() << "Extensions " << VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME << " and " << VK_NV_RAY_TRACING_EXTENSION_NAME
                     << " are not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (khr_acceleration_structure) {
        auto cmd_begin_query = [this, ext_transform_feedback](VkQueryType query_type, auto vuid_begin_query,
                                                              auto vuid_begin_query_indexed) {
            VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
            query_pool_ci.queryCount = 1;

            query_pool_ci.queryType = query_type;
            vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
            ASSERT_TRUE(query_pool.initialized());

            m_commandBuffer->begin();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid_begin_query);
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid_begin_query_indexed);
                vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        };

        cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, "VUID-vkCmdBeginQuery-queryType-04728",
                        "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");
        cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, "VUID-vkCmdBeginQuery-queryType-04728",
                        "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");

        if (rt_maintenance_1) {
            cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, "VUID-vkCmdBeginQuery-queryType-06741",
                            "VUID-vkCmdBeginQueryIndexedEXT-queryType-06741");
            cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
                            "VUID-vkCmdBeginQuery-queryType-06741", "VUID-vkCmdBeginQueryIndexedEXT-queryType-06741");
        }
    }
    if (nv_ray_tracing) {
        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;
        query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04729");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        if (ext_transform_feedback) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04729");
            vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();
    }
}

TEST_F(NegativeRayTracing, CopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyQueryPoolResults with unsupported query type");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto as_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&as_features);
    auto features2 = GetPhysicalDeviceFeatures2(bda_features);

    if (as_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto blas_no_mem = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(DeviceValidationVersion(), 4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Build(*m_device);

    auto valid_blas = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(DeviceValidationVersion(), 4096);
    valid_blas->Build(*m_device);

    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureInfoKHR>();
    copy_info.src = blas_no_mem->handle();
    copy_info.dst = valid_blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = valid_blas->handle();
    copy_info.dst = blas_no_mem->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, CmdCopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffers not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&bda_features);
    auto features2 = GetPhysicalDeviceFeatures2(accel_features);

    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress feature is not supported";
    }
    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Init a non host visible buffer
    VkBufferObj buffer;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vk_testing::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), device_memory.handle(), 0);

    auto blas = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(DeviceValidationVersion(), 4096);
    blas->SetDeviceBuffer(std::move(buffer));
    blas->Build(*m_device);

    auto blas_no_mem = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(DeviceValidationVersion(), 4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Build(*m_device);

    auto blas_host_mem = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(DeviceValidationVersion(), 4096);
    blas_host_mem->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    blas_host_mem->Build(*m_device);

    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureInfoKHR>();
    copy_info.src = blas_no_mem->handle();
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = blas->handle();
    copy_info.dst = blas_no_mem->handle();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = blas->handle();
    copy_info.dst = blas_host_mem->handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03727");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = blas_host_mem->handle();
    copy_info.dst = blas->handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03728");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdCopyMemoryToAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate CmdCopyMemoryToAccelerationStructureKHR with dst buffer not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_tracing_features);
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj src_buffer;
    src_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);

    VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBufferObj dst_buffer;
    dst_buffer.init_no_mem(*m_device, buffer_ci);

    auto blas = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(VK_API_VERSION_1_2, 0);
    blas->SetDeviceBuffer(std::move(dst_buffer));
    blas->Build(*m_device);

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = LvlInitStruct<VkCopyMemoryToAccelerationStructureInfoKHR>();
    copy_info.src.deviceAddress = src_buffer.address();
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // Acceleration structure buffer is not bound to memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    m_commandBuffer->begin();
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate buffers used in vkBuildAccelerationStructureKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_tracing_features);
    auto features2 = GetPhysicalDeviceFeatures2(acc_structure_features);
    if (acc_structure_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Init a non host visible buffer
    VkBufferObj non_host_visible_buffer;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vk_testing::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    VkBufferObj host_buffer;
    host_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    auto bot_level_as = rt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(DeviceValidationVersion(), *m_device);
    bot_level_as.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    bot_level_as.BuildHost(instance(), *m_device);
    m_errorMonitor->VerifyFound();

    auto host_cached_blas = rt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(DeviceValidationVersion(), *m_device);

    host_cached_blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    host_cached_blas.SetSrcAS(rt::as::blueprint::AccelStructSimpleOnHostBottomLevel(DeviceValidationVersion(), 4096));
    host_cached_blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // .mode is UPDATE and .srcAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
    host_cached_blas.BuildHost(instance(), *m_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructureMemory) {
    TEST_DESCRIPTION("Test memory in vkWriteAccelerationStructuresPropertiesKHR is host visible");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>();
    auto as_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    auto features2 = GetPhysicalDeviceFeatures2(as_features);

    if (as_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature is not supported";
    }
    if (as_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Init a non host visible buffer
    VkBufferObj non_host_visible_buffer;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vk_testing::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    auto build_geometry_info = rt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(DeviceValidationVersion(), *m_device);
    build_geometry_info.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    build_geometry_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    build_geometry_info.BuildHost(instance(), *m_device);
    m_errorMonitor->VerifyFound();

    std::vector<uint32_t> data(4096);
    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");
    vk::WriteAccelerationStructuresPropertiesKHR(device(), 1, &build_geometry_info.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data.size(), data.data(),
                                                 data.size());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CopyMemoryToAsBuffer) {
    TEST_DESCRIPTION("Test invalid buffer used in vkCopyMemoryToAccelerationStructureKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(accel_features);
    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Init a non host visible buffer
    VkBufferObj non_host_visible_buffer;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vk_testing::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    auto blas = rt::as::blueprint::AccelStructSimpleOnHostBottomLevel(DeviceValidationVersion(), buffer_ci.size);
    blas->SetDeviceBuffer(std::move(non_host_visible_buffer));
    blas->Build(*m_device);

    uint8_t output[4096];
    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(output);

    auto info = LvlInitStruct<VkCopyMemoryToAccelerationStructureInfoKHR>();
    info.dst = blas->handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, ArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "Core validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders.");

    OOBRayTracingShadersTestBody(false);
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&ray_tracing_features);
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&acc_struct_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    if (ray_query_features.rayQuery == VK_FALSE && ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        GTEST_SKIP() << "Both of the required features rayQuery and rayTracing are not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer.handle()};
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);
    // invalid buffer;
    {
        VkBufferObj invalid_buffer;
        invalid_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.buffer = invalid_buffer.handle();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid deviceAddress and flag;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.deviceAddress = device_address;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();

        invalid_as_create_info.deviceAddress = 0;
        invalid_as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-parameter");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid size and offset;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.size = 4097;  // buffer size is 4096
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-offset-03616");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid sType;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-sType-sType");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid type;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-type-parameter");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHRReplayFeature) {
    TEST_DESCRIPTION("Validate acceleration structure creation replay feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&acc_struct_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    acc_struct_features.accelerationStructureCaptureReplay = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);

    VkBufferDeviceAddressInfo device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    device_address_info.buffer = buffer.handle();
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = device_address;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488");
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdTraceRaysKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysKHR.");

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
    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03694");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03690");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03686");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride =
            ray_tracing_properties.maxShaderGroupStride + (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04041");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04035");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04029");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    VkBufferObj buffer_missing_flag;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_missing_flag.init_no_mem(*m_device, buffer_ci);
    vk::BindBufferMemory(device(), buffer_missing_flag.handle(), mem.handle(), 0);
    const VkDeviceAddress device_address_missing_flag = buffer_missing_flag.address();

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        // This address is the same as the one from the first (valid) buffer, so no validation error
        invalid_stride.deviceAddress = device_address_missing_flag;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                            100, 1);
    }

    // pRayGenShaderBindingTable address range and stride are invalid
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = 8128;
        invalid_stride.size = 8128;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkStridedDeviceAddressRegionKHR-size-04631");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkStridedDeviceAddressRegionKHR-size-04632");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }

    // pMissShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }
    // pHitShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                            100, 1);
    }
    // pCallableShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                            100, 1);
    }

    m_commandBuffer->end();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirectKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&ray_tracing_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    if (ray_tracing_features.rayTracingPipelineTraceRaysIndirect == VK_FALSE) {
        GTEST_SKIP() << "rayTracingIndirectTraceRays not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer(*m_device, buf_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);

    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    GetPhysicalDeviceProperties2(properties2);

    const VkDeviceAddress device_address = buffer.address(DeviceValidationVersion());

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_commandBuffer->begin();
    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                        &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    // Given the invalid stride computation, stride is likely to also be misaligned, so allow above errors
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        if (ray_tracing_properties.maxShaderGroupStride ==
            std::numeric_limits<decltype(ray_tracing_properties.maxShaderGroupStride)>::max()) {
            printf("ray_tracing_properties.maxShaderGroupStride has maximum possible value, skipping related tests\n");
        } else {
            invalid_stride.stride = ray_tracing_properties.maxShaderGroupStride + 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                        &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirect2KHRFeatureDisabled) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirect2KHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    // auto maintenance1_features = LvlInitStruct<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << "not supported, skipping test";
    }
    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkBufferCreateInfo buffer_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_info.size = 4096;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer(*m_device, buffer_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);

    const VkDeviceAddress device_address = buffer.address(DeviceValidationVersion());

    m_commandBuffer->begin();
    // No VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR was in the device create info pNext chain
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637");
        vk::CmdTraceRaysIndirect2KHR(m_commandBuffer->handle(), device_address);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirect2KHRAddress) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirect2KHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    auto maintenance1_features = LvlInitStruct<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&maintenance1_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << "not supported, skipping test";
    }
    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress not supported";
    }
    if (maintenance1_features.rayTracingPipelineTraceRaysIndirect2 == VK_FALSE) {
        GTEST_SKIP() << "rayTracingPipelineTraceRaysIndirect2 not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkBufferCreateInfo buffer_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_info.size = 4096;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer(*m_device, buffer_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);

    const VkDeviceAddress device_address = buffer.address(DeviceValidationVersion());

    m_commandBuffer->begin();
    // indirectDeviceAddress is not a multiple of 4
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634");
        vk::CmdTraceRaysIndirect2KHR(m_commandBuffer->handle(), device_address + 1);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, AccelerationStructureVersionInfoKHR) {
    TEST_DESCRIPTION("Validate VkAccelerationStructureVersionInfoKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        GTEST_SKIP() << "rayTracing not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ray_tracing_features));

    VkAccelerationStructureVersionInfoKHR valid_version = LvlInitStruct<VkAccelerationStructureVersionInfoKHR>();
    VkAccelerationStructureCompatibilityKHR compatablity;
    uint8_t mode[] = {VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR};
    valid_version.pVersionData = mode;
    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-sType-sType");
        vk::GetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.pVersionData = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-pVersionData-parameter");
        vk::GetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, CmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    AddOptionalExtensions(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&bda_features);
    ray_query_features.rayQuery = VK_TRUE;
    accel_features.accelerationStructureIndirectBuild = VK_TRUE;
    accel_features.accelerationStructureHostCommands = VK_TRUE;
    bda_features.bufferDeviceAddress = VK_TRUE;

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_query_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    const bool index_type_uint8 = IsExtensionsEnabled(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);

    auto acc_struct_properties = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    GetPhysicalDeviceProperties2(acc_struct_properties);

    // Command buffer not in recording mode
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // dstAccelerationStructure == VK_NULL_HANDLE
    {
        // Command buffer build
        {
            auto build_info_null_dst =
                rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
            build_info_null_dst.SetDstAS(rt::as::blueprint::AccelStructNull(DeviceValidationVersion()));

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
            build_info_null_dst.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }

        // Command buffer indirect build
        if (accel_features.accelerationStructureIndirectBuild == VK_TRUE) {
            auto build_info_null_dst =
                rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
            build_info_null_dst.SetDstAS(rt::as::blueprint::AccelStructNull(DeviceValidationVersion()));
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800");
            build_info_null_dst.BuildCmdBufferIndirect(instance(), *m_device, m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }

        // Host build
        if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
            auto build_info_null_dst =
                rt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(DeviceValidationVersion(), *m_device);
            build_info_null_dst.SetDstAS(rt::as::blueprint::AccelStructNull(DeviceValidationVersion()));
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
            build_info_null_dst.BuildHost(instance(), *m_device);
            m_errorMonitor->VerifyFound();
        }
    }

    // Positive build tests
    {
        m_commandBuffer->begin();
        auto build_info_ppGeometries =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info_ppGeometries.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle(), true);
        m_commandBuffer->end();
    }

    {
        m_commandBuffer->begin();
        auto build_info_pGeometries =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info_pGeometries.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle(), false);
        m_commandBuffer->end();
    }

    m_commandBuffer->begin();

    // Invalid info count
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetInfoCount(0);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid pInfos
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetNullInfos(true);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-parameter");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid ppBuildRangeInfos
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetNullBuildRangeInfos(true);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-parameter");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // must be called outside renderpass
    {
        ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-renderpass");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
    }
    // Invalid flags
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid dst buffer
    {
        auto buffer_ci =
            VkBufferObj::create_info(4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        VkBufferObj invalid_buffer;
        invalid_buffer.init_no_mem(*m_device, buffer_ci);
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.GetDstAS()->SetDeviceBuffer(std::move(invalid_buffer));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid sType
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.GetInfo().sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid Type
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();

        build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Total number of triangles in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
            build_info.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795");
            build_info.GetSizeInfo(m_device->handle());
            m_errorMonitor->VerifyFound();
        }
    }
    // Total number of AABBs in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device,
                                                                                            rt::as::GeometryKHR::Type::AABB);
            build_info.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794");
            build_info.GetSizeInfo(m_device->handle());
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride in pGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device,
                                                                                        rt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        build_info.GetSizeInfo(m_device->handle(), false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device,
                                                                                        rt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        build_info.GetSizeInfo(m_device->handle(), true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in pGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device,
                                                                                        rt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        build_info.GetSizeInfo(m_device->handle(), false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device,
                                                                                        rt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        build_info.GetSizeInfo(m_device->handle(), true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex stride
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.GetGeometries()[0].SetStride(VkDeviceSize(vvl::kU32Max) + 1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid index type
    if (index_type_uint8) {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.GetGeometries()[0].SetTrianglesIndexType(VK_INDEX_TYPE_UINT8_EXT);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // ppGeometries and pGeometries both valid pointer
    {
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        std::vector<VkAccelerationStructureGeometryKHR> geometries;
        for (const auto &geometry : build_info.GetGeometries()) {
            geometries.emplace_back(geometry.GetVkObj());
        }
        build_info.GetInfo().pGeometries = geometries.data();  // .ppGeometries is set in .BuildCmdBuffer()
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
        // computed scratch buffer size will be 0 since vkGetAccelerationStructureBuildSizesKHR fails
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Buffer is missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR usage flag
    {
        VkBufferObj bad_usage_buffer;
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        bad_usage_buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);
        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(bad_usage_buffer), 3);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag
    {
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        VkBufferObj bad_scratch;
        bad_scratch.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                         &alloc_flags);

        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is 0
    {
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        VkBufferObj bad_scratch;
        // no VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT => scratch address will be set to 0
        bad_scratch.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &alloc_flags);

        auto build_info = rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(DeviceValidationVersion(), *m_device);
        build_info.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, ObjInUseCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_tracing_features);
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    rt::as::BuildGeometryInfoKHR build_geometry_info =
        rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
    m_commandBuffer->begin();
    build_geometry_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), build_geometry_info.GetGeometries()[0].GetTriangles().device_vertex_buffer.handle(),
                      nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), build_geometry_info.GetGeometries()[0].GetTriangles().device_index_buffer.handle(),
                      nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    vk::DestroyAccelerationStructureKHR(m_device->handle(), build_geometry_info.GetDstAS()->handle(), nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), build_geometry_info.GetDstAS()->GetBuffer().handle(), nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), build_geometry_info.GetScratchBuffer().handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(NegativeRayTracing, CmdCopyAccelerationStructureToMemoryKHR) {
    TEST_DESCRIPTION("Validate CmdCopyAccelerationStructureToMemoryKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&ray_tracing_features);
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    GetPhysicalDeviceFeatures2(acc_struct_features);
    if (ray_query_features.rayQuery == VK_FALSE && ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        GTEST_SKIP() << "Both of the required features rayQuery and rayTracing are not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &acc_struct_features));

    constexpr VkDeviceSize buffer_size = 4096;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = buffer_size;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_ci);

    auto as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.pNext = NULL;
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkAccelerationStructureKHR as;
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);

    constexpr intptr_t alignment_padding = 256 - 1;
    int8_t output[buffer_size + alignment_padding];
    VkDeviceOrHostAddressKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(((intptr_t)output + alignment_padding) & ~alignment_padding);
    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureToMemoryInfoKHR>();
    copy_info.src = as;
    copy_info.dst = output_data;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    VkCommandBufferObj cb(m_device, m_commandPool);
    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-None-03559");
    vk::CmdCopyAccelerationStructureToMemoryKHR(cb.handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    cb.end();

    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

TEST_F(NegativeRayTracing, UpdateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Test for updating an acceleration structure without a srcAccelerationStructure");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto buffer_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&ray_tracing_features);
    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&buffer_address_features);
    auto features2 = GetPhysicalDeviceFeatures2(acc_structure_features);
    if (acc_structure_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature not supported";
    }
    if (buffer_address_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_commandBuffer->begin();

    rt::as::BuildGeometryInfoKHR build_geometry_info =
        rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
    build_geometry_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    // computed scratch buffer size is empty, so scratch buffer address can be 0 and invalid
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
    // Update acceleration structure, with .srcAccelerationStructure == VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630");
    build_geometry_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuffersAndBufferDeviceAddressesMapping) {
    TEST_DESCRIPTION(
        "Test that buffers and buffer device addresses mapping is correctly handled."
        "Bound multiple buffers to the same memory so that they have the same buffer device address."
        "Some buffers are valid for use in vkCmdBuildAccelerationStructuresKHR, others are not."
        "Using buffer device addresses obtained from invalid buffers will result in a valid call to "
        "vkCmdBuildAccelerationStructuresKHR,"
        "because for this call to be valid, at least one buffer retrieved from the buffer device addresses must be valid."
        "Valid and invalid buffers having the same address, the call is valid."
        "Removing those valid buffers should cause calls to vkCmdBuildAccelerationStructuresKHR to be invalid,"
        "as long as valid buffers are correctly removed from the internal buffer device addresses to buffers mapping.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&accel_features);
    auto buffer_addr_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&ray_tracing_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&buffer_addr_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Allocate common buffer memory
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vk_testing::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers, with correct and incorrect usage
    constexpr size_t N = 3;
    std::array<std::unique_ptr<rt::as::BuildGeometryInfoKHR>, N> build_geometry_info_vec{};
    const VkBufferUsageFlags good_buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags bad_buffer_usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = good_buffer_usage;
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            buffer_ci.usage = bad_buffer_usage;
        }

        VkBufferObj vbo;
        vbo.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), vbo.handle(), buffer_memory.handle(), 0);

        VkBufferObj ibo;
        ibo.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), ibo.handle(), buffer_memory.handle(), 0);

        // Those calls to vkGetBufferDeviceAddressKHR will internally record vbo and ibo device addresses
        {
            const VkDeviceAddress vbo_address = vbo.address(DeviceValidationVersion());
            const VkDeviceAddress ibo_address = ibo.address(DeviceValidationVersion());
            if (vbo_address != ibo_address) {
                GTEST_SKIP()
                    << "Bounding two buffers to the same memory location does not result in identical buffer device addresses";
            }
        }
        build_geometry_info_vec[i] = std::make_unique<rt::as::BuildGeometryInfoKHR>(
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device));
        build_geometry_info_vec[i]->GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(vbo), 2);
        build_geometry_info_vec[i]->GetGeometries()[0].SetTrianglesDeviceIndexBuffer(std::move(ibo));
    }

    // The first series of calls to vkCmdBuildAccelerationStructuresKHR should succeed,
    // since the first vbo and ibo do have the VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR flag.
    // After deleting the valid vbo and ibo, calls are expected to fail.

    for (size_t i = 0; i < N; ++i) {
        m_commandBuffer->begin();
        build_geometry_info_vec[i]->BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_commandBuffer->end();
    }

    for (size_t i = 0; i < N; ++i) {
        m_commandBuffer->begin();
        if (i > 0) {
            // for vbo
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
            // for ibo
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        }
        build_geometry_info_vec[i]->VkCmdBuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle());
        if (i > 0) {
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();

        build_geometry_info_vec[i] = nullptr;
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresProperties) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&bda_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_query_features);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    GetPhysicalDeviceFeatures2(features2);
    if (accel_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure not supported";
    }
    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress not supported";
    }
    if (ray_query_features.rayQuery == VK_FALSE) {
        GTEST_SKIP() << "rayQuery not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    const bool rt_maintenance_1 = IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    // On host query with invalid query type
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        rt::as::BuildGeometryInfoKHR as_build_info =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
        as_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        as_build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = 1;
        constexpr size_t data_size = sizeof(uint32_t) * stride;
        uint8_t data[data_size];
        // Incorrect query type
        const char *vuid = (rt_maintenance_1) ? "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742"
                                              : "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03432";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_OCCLUSION, data_size, data, stride);
        m_errorMonitor->VerifyFound();

        // query types not known without extension
        if (rt_maintenance_1) {
            // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but stride is not a multiple of the size of VkDeviceSize
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731");
            vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                         VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, data_size, data, stride);
            m_errorMonitor->VerifyFound();

            // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but stride is not a
            // multiple of the size of VkDeviceSize
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733");
            vk::WriteAccelerationStructuresPropertiesKHR(
                m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, data_size, data, stride);
            m_errorMonitor->VerifyFound();
        }
    }

    // On device query with invalid query type
    {
        rt::as::BuildGeometryInfoKHR as_build_info =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
        as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();

        as_build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        // Incorrect query type
        const char *vuid = (rt_maintenance_1) ? "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742"
                                              : "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-03432";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vk::CmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                        VK_QUERY_TYPE_OCCLUSION, query_pool.handle(), 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesMaintenance1) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&bda_features);
    auto ray_tracing_maintenance1 = LvlInitStruct<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>(&ray_query_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_maintenance1);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    GetPhysicalDeviceFeatures2(features2);
    if (accel_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure not supported";
    }
    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress not supported";
    }
    if (ray_query_features.rayQuery == VK_FALSE) {
        GTEST_SKIP() << "rayQuery not supported";
    }
    if (ray_tracing_maintenance1.rayTracingMaintenance1 == VK_FALSE) {
        GTEST_SKIP() << "rayTracingMaintenance1 not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // On host query with invalid query type
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        rt::as::BuildGeometryInfoKHR blas_build_info =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = sizeof(VkDeviceSize);
        constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
        uint8_t data[data_size];
        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_OCCLUSION, data_size, data, stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with missing BLAS flag
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        rt::as::BuildGeometryInfoKHR blas_build_info =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // missing flag
        // blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = sizeof(VkDeviceSize);
        constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
        uint8_t data[data_size];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with invalid stride
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        rt::as::BuildGeometryInfoKHR blas_build_info =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = 1;
        constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
        uint8_t data[data_size];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();
    }

    // On device query with invalid query type
    {
        rt::as::BuildGeometryInfoKHR blas_build_info =
            rt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(VK_API_VERSION_1_1, *m_device);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();

        blas_build_info.BuildCmdBuffer(instance(), *m_device, m_commandBuffer->handle());
        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::CmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                        VK_QUERY_TYPE_OCCLUSION, query_pool.handle(), 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeRayTracing, AccelerationStructureBindingsNV) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ray_tracing_props = LvlInitStruct<VkPhysicalDeviceRayTracingPropertiesNV>();
    GetPhysicalDeviceProperties2(ray_tracing_props);

    ASSERT_NO_FATAL_FAILURE(InitState());

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

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02381");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, ValidateGeometryNV) {
    TEST_DESCRIPTION("Validate acceleration structure geometries.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    vbo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferObj ibo;
    ibo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferObj tbo;
    tbo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferObj aabbbo;
    aabbbo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferCreateInfo unbound_buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    unbound_buffer_ci.size = 1024;
    unbound_buffer_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    VkBufferObj unbound_buffer;
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

    VkGeometryNV valid_geometry_triangles = LvlInitStruct<VkGeometryNV>();
    valid_geometry_triangles.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    valid_geometry_triangles.geometry.triangles = LvlInitStruct<VkGeometryTrianglesNV>();
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
    valid_geometry_triangles.geometry.aabbs = LvlInitStruct<VkGeometryAABBNV>();

    VkGeometryNV valid_geometry_aabbs = LvlInitStruct<VkGeometryNV>();
    valid_geometry_aabbs.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    valid_geometry_aabbs.geometry.triangles = LvlInitStruct<VkGeometryTrianglesNV>();
    valid_geometry_aabbs.geometry.aabbs = LvlInitStruct<VkGeometryAABBNV>();
    valid_geometry_aabbs.geometry.aabbs.aabbData = aabbbo.handle();
    valid_geometry_aabbs.geometry.aabbs.numAABBs = 1;
    valid_geometry_aabbs.geometry.aabbs.offset = 0;
    valid_geometry_aabbs.geometry.aabbs.stride = 24;

    const auto GetCreateInfo = [](const VkGeometryNV &geometry) {
        VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
        as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
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

TEST_F(NegativeRayTracing, ValidateCreateAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();

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
        VkGeometryNV aabb_geometry = LvlInitStruct<VkGeometryNV>();
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        aabb_geometry.geometry.triangles = LvlInitStruct<VkGeometryTrianglesNV>();
        aabb_geometry.geometry.aabbs = LvlInitStruct<VkGeometryAABBNV>();
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

TEST_F(NegativeRayTracing, ValidateBindAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;
    as_create_info.info.instanceCount = 0;

    VkAccelerationStructureObj as(*m_device, as_create_info, false);

    VkMemoryRequirements as_memory_requirements = as.memory_requirements().memoryRequirements;

    VkBindAccelerationStructureMemoryInfoNV as_bind_info = LvlInitStruct<VkBindAccelerationStructureMemoryInfoNV>();
    as_bind_info.accelerationStructure = as.handle();

    VkMemoryAllocateInfo as_memory_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    as_memory_alloc.allocationSize = as_memory_requirements.size;
    ASSERT_TRUE(m_device->phy().set_memory_type(as_memory_requirements.memoryTypeBits, &as_memory_alloc, 0));

    // Can not bind already freed memory
    {
        VkDeviceMemory as_memory_freed = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_freed));
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
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc_bad_alignment, NULL, &as_memory_bad_alignment));

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
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

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
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

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
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc_bad_type, NULL, &as_memory_bad_type));

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
        VkAccelerationStructureObj as_twice(*m_device, as_create_info, false);

        VkDeviceMemory as_memory_twice_1 = VK_NULL_HANDLE;
        VkDeviceMemory as_memory_twice_2 = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_1));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_2));
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_1 = as_bind_info;
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_2 = as_bind_info;
        as_bind_info_twice_1.accelerationStructure = as_twice.handle();
        as_bind_info_twice_2.accelerationStructure = as_twice.handle();
        as_bind_info_twice_1.memory = as_memory_twice_1;
        as_bind_info_twice_2.memory = as_memory_twice_2;

        ASSERT_VK_SUCCESS(vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_1));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(NegativeRayTracing, ValidateWriteDescriptorSetAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
                           });

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkWriteDescriptorSetAccelerationStructureNV acc = LvlInitStruct<VkWriteDescriptorSetAccelerationStructureNV>();
    acc.accelerationStructureCount = 1;
    VkAccelerationStructureCreateInfoNV top_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    top_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);

    acc.pAccelerationStructures = &top_level_as.handle();
    descriptor_write.pNext = &acc;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(NegativeRayTracing, ValidateCmdBuildAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);

    const VkBufferObj bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device);

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
    VkBufferCreateInfo too_small_scratch_buffer_info = LvlInitStruct<VkBufferCreateInfo>();
    too_small_scratch_buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    too_small_scratch_buffer_info.size = 1;
    VkBufferObj too_small_scratch_buffer(*m_device, too_small_scratch_buffer_info);
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
    VkAccelerationStructureObj bot_level_as_updated(*m_device, bot_level_as_create_info);
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
    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    const VkBufferObj bot_level_as_invalid_scratch = bot_level_as.create_scratch_buffer(*m_device, &create_info);
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
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-renderpass");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, ObjInUseCmdBuildAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);

    const VkBufferObj bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device);

    m_commandBuffer->begin();
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

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

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(NegativeRayTracing, ValidateGetAccelerationStructureHandleNV) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    // Not enough space for the handle
    {
        VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint8_t), &handle);
        m_errorMonitor->VerifyFound();
    }

    // No memory bound to acceleration structure
    {
        VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info, /*init_memory=*/false);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint64_t), &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, ValidateCmdCopyAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure copying.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.instanceCount = 0;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureObj src_as(*m_device, as_create_info);
    VkAccelerationStructureObj dst_as(*m_device, as_create_info);
    VkAccelerationStructureObj dst_as_without_mem(*m_device, as_create_info, false);

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
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-renderpass");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}
