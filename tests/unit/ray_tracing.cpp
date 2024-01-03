/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
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
#include "../framework/descriptor_helper.h"
#include "../framework/shader_helper.h"
#include "../layers/utils/vk_layer_utils.h"

TEST_F(NegativeRayTracing, BarrierAccessAccelerationStructure) {
    TEST_DESCRIPTION("Test barrier with access ACCELERATION_STRUCTURE bit.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_commandBuffer->begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03927");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03928");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03927");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03928");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierSync2AccessAccelerationStructureRayQueryDisabled) {
    TEST_DESCRIPTION(
        "Test sync2 barrier with ACCELERATION_STRUCTURE_READ memory access."
        "Ray query feature is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
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

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    ASSERT_TRUE(image.initialized());

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-dstAccessMask-06256");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierSync1AccessAccelerationStructureRayQueryDisabled) {
    TEST_DESCRIPTION(
        "Test sync1 barrier with ACCELERATION_STRUCTURE_READ memory access."
        "Ray query feature is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkt::Buffer buffer(*m_device, 32);
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_commandBuffer->begin();

    // memory barrier
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-06257");
    // buffer barrier
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-06257");
    // image barrier
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-06257");

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, 0, 1, &memory_barrier, 1, &buffer_barrier, 1, &image_barrier);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, EventSync1AccessAccelerationStructureRayQueryDisabled) {
    TEST_DESCRIPTION(
        "Test sync1 event wait with ACCELERATION_STRUCTURE_READ memory access."
        "Ray query feature is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkt::Buffer buffer(*m_device, 32);
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_commandBuffer->begin();

    // memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstAccessMask-06257");
    // buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstAccessMask-06257");
    // image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstAccessMask-06257");

    m_commandBuffer->WaitEvents(1, &event.handle(), VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                1, &memory_barrier, 1, &buffer_barrier, 1, &image_barrier);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, DescriptorBindingUpdateAfterBindWithAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout setLayout;
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570");
    vk::CreateDescriptorSetLayout(device(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, MaxPerStageDescriptorAccelerationStructures) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceAccelerationStructurePropertiesKHR accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);

    // Create one descriptor set layout holding (maxPerStageDescriptorAccelerationStructures + 1) bindings
    // for the same shader stage
    const uint32_t max_accel_structs = accel_struct_props.maxPerStageDescriptorAccelerationStructures;
    if (max_accel_structs > 4096) {
        GTEST_SKIP() << "maxPerStageDescriptorAccelerationStructures is too large";
    } else if (max_accel_structs < 1) {
        GTEST_SKIP() << "maxPerStageDescriptorAccelerationStructures is 1";
    }
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

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, MaxPerStageDescriptorUpdateAfterBindAccelerationStructures) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceAccelerationStructurePropertiesKHR accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);

    // Create one descriptor set layout with flag VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT holding
    // (maxPerStageDescriptorUpdateAfterBindAccelerationStructures + 1) bindings for the same shader stage
    const uint32_t max_accel_structs = accel_struct_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    if (max_accel_structs > 4096) {
        GTEST_SKIP() << "maxPerStageDescriptorUpdateAfterBindAccelerationStructures is too large";
    } else if (max_accel_structs < 1) {
        GTEST_SKIP() << "maxPerStageDescriptorUpdateAfterBindAccelerationStructures is 1";
    }

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

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, MaxDescriptorSetAccelerationStructures) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceAccelerationStructurePropertiesKHR accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);

    // Create one descriptor set layout holding (maxDescriptorSetAccelerationStructures + 1) bindings
    // in total for two different shader stage
    const uint32_t max_accel_structs = accel_struct_props.maxDescriptorSetAccelerationStructures;
    if (max_accel_structs > 4096) {
        GTEST_SKIP() << "maxDescriptorSetAccelerationStructures is too large";
    } else if (max_accel_structs < 1) {
        GTEST_SKIP() << "maxDescriptorSetAccelerationStructures is 1";
    }

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

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, MaxDescriptorSetUpdateAfterBindAccelerationStructures) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceAccelerationStructurePropertiesKHR accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);

    // Create one descriptor set layout with flag VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT holding
    // (maxDescriptorSetUpdateAfterBindAccelerationStructures + 1) bindings in total for two different shader stage
    const uint32_t max_accel_structs = accel_struct_props.maxDescriptorSetUpdateAfterBindAccelerationStructures;
    if (max_accel_structs > 4096) {
        GTEST_SKIP() << "maxDescriptorSetUpdateAfterBindAccelerationStructures is too large";
    } else if (max_accel_structs < 1) {
        GTEST_SKIP() << "maxDescriptorSetUpdateAfterBindAccelerationStructures is 1";
    }

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

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BeginQueryQueryPoolType) {
    TEST_DESCRIPTION("Test CmdBeginQuery with invalid queryPool queryType");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    const bool khr_acceleration_structure = IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    const bool nv_ray_tracing = IsExtensionsEnabled(VK_NV_RAY_TRACING_EXTENSION_NAME);
    const bool ext_transform_feedback = IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    const bool rt_maintenance_1 = IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    if (!khr_acceleration_structure && !nv_ray_tracing) {
        GTEST_SKIP() << "Extensions " << VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME << " and " << VK_NV_RAY_TRACING_EXTENSION_NAME
                     << " are not supported.";
    }
    RETURN_IF_SKIP(InitState());

    if (khr_acceleration_structure) {
        auto cmd_begin_query = [this, ext_transform_feedback](VkQueryType query_type, auto vuid_begin_query,
                                                              auto vuid_begin_query_indexed) {
            vkt::QueryPool query_pool(*m_device, query_type, 1);

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
        vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV, 1);

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
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas_no_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Build();

    auto valid_blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    valid_blas->Build();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
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
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    vkt::Buffer buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), device_memory.handle(), 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->SetDeviceBuffer(std::move(buffer));
    blas->Build();

    auto blas_no_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Build();

    auto blas_host_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas_host_mem->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    blas_host_mem->Build();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
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

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer src_buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer dst_buffer;
    dst_buffer.init_no_mem(*m_device, buffer_ci);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 0);
    blas->SetDeviceBuffer(std::move(dst_buffer));
    blas->Build();

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
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
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    vkt::Buffer non_host_visible_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    vkt::Buffer host_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    auto bot_level_as = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    bot_level_as.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    bot_level_as.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructureModeUpdate) {
    TEST_DESCRIPTION("Validate buffers used in vkBuildAccelerationStructureKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    auto host_cached_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);

    host_cached_blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    host_cached_blas.SetSrcAS(vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, 4096));
    host_cached_blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03667");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03758");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03760");
    host_cached_blas.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructureMemory) {
    TEST_DESCRIPTION("Test memory in vkWriteAccelerationStructuresPropertiesKHR is host visible");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    vkt::Buffer non_host_visible_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    auto build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    build_geometry_info.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    build_geometry_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    build_geometry_info.BuildHost();
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
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    vkt::Buffer non_host_visible_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, buffer_ci.size);
    blas->SetDeviceBuffer(std::move(non_host_visible_buffer));
    blas->Build();

    uint8_t output[4096];
    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(output);

    VkCopyMemoryToAccelerationStructureInfoKHR info = vku::InitStructHelper();
    info.dst = blas->handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
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
        vkt::Buffer invalid_buffer(*m_device, 4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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

TEST_F(NegativeRayTracing, CreateAccelerationStructureFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddDisabledFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer.handle();
    as_create_info.size = 4096;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateAccelerationStructureKHR-accelerationStructure-03611");
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-08934");
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHRReplayFeature) {
    TEST_DESCRIPTION("Validate acceleration structure creation replay feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);

    AddDisabledFeature(vkt::Feature::accelerationStructureCaptureReplay);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    VkBufferDeviceAddressInfo device_address_info = vku::InitStructHelper();
    device_address_info.buffer = buffer.handle();
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
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

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Create ray tracing pipeline
    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;
    {
        const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
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

    vkt::Buffer buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer.init_no_mem(*m_device, buffer_ci);

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

    const VkStridedDeviceAddressRegionKHR stridebufregion = {device_address, ray_tracing_properties.shaderGroupHandleAlignment,
                                                             ray_tracing_properties.shaderGroupHandleAlignment};

    m_commandBuffer->begin();

    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03694");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03690");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
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
        invalid_stride.stride = static_cast<VkDeviceSize>(ray_tracing_properties.maxShaderGroupStride) +
                                (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04041");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04035");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04029");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    {
        vkt::Buffer buffer_missing_flag;
        buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        buffer_missing_flag.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), buffer_missing_flag.handle(), mem.handle(), 0);
        const VkDeviceAddress device_address_missing_flag = buffer_missing_flag.address();

        // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
        if (device_address_missing_flag == device_address) {
            VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
            // This address is the same as the one from the first (valid) buffer, so no validation error
            invalid_stride.deviceAddress = device_address_missing_flag;
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion,
                                100, 100, 1);
        }
    }

    // pRayGenShaderBindingTable address range and stride are invalid
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = 8128;
        invalid_stride.size = 8128;
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

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buf_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
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
            // TODO - Create minimal ray tracing pipeline helper - remove all extra `08606`
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                        &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
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

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                        &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
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

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddDisabledFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_info.size = 4096;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buffer_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    const VkDeviceAddress device_address = buffer.address();

    m_commandBuffer->begin();
    // No VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR was in the device create info pNext chain
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-None-08606");
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

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_info.size = 4096;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buffer_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    const VkDeviceAddress device_address = buffer.address();

    m_commandBuffer->begin();
    // indirectDeviceAddress is not a multiple of 4
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634");
        vk::CmdTraceRaysIndirect2KHR(m_commandBuffer->handle(), device_address + 1);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, AccelerationStructureVersionInfoKHR) {
    TEST_DESCRIPTION("Validate VkAccelerationStructureVersionInfoKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureVersionInfoKHR valid_version = vku::InitStructHelper();
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

TEST_F(NegativeRayTracing, IndirectCmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure indirect builds.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureIndirectBuild);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Command buffer indirect build
    auto build_info_null_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info_null_dst.SetDstAS(vkt::as::blueprint::AccelStructNull(*m_device));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800");
    build_info_null_dst.BuildCmdBufferIndirect(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, HostCmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure indirect builds.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Host build
    auto build_info_null_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    build_info_null_dst.SetDstAS(vkt::as::blueprint::AccelStructNull(*m_device));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
    build_info_null_dst.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    AddOptionalExtensions(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const bool index_type_uint8 = IsExtensionsEnabled(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);

    VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_struct_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(acc_struct_properties);

    // Command buffer not in recording mode
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // dstAccelerationStructure == VK_NULL_HANDLE
    {
        // Command buffer build
        {
            auto build_info_null_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info_null_dst.SetDstAS(vkt::as::blueprint::AccelStructNull(*m_device));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
    build_info_null_dst.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
        }
    }

    // Positive build tests
    {
        m_commandBuffer->begin();
        auto build_info_ppGeometries = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info_ppGeometries.BuildCmdBuffer(m_commandBuffer->handle(), true);
        m_commandBuffer->end();
    }

    {
        m_commandBuffer->begin();
        auto build_info_pGeometries = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info_pGeometries.BuildCmdBuffer(m_commandBuffer->handle(), false);
        m_commandBuffer->end();
    }

    m_commandBuffer->begin();

    // Invalid info count
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetInfoCount(0);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid pInfos
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetNullInfos(true);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-parameter");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid ppBuildRangeInfos
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetNullBuildRangeInfos(true);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-parameter");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // must be called outside renderpass
    {
        InitRenderTarget();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-renderpass");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
    }
    // Invalid flags
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid dst buffer
    {
        auto buffer_ci =
            vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        vkt::Buffer invalid_buffer;
        invalid_buffer.init_no_mem(*m_device, buffer_ci);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetDstAS()->SetDeviceBuffer(std::move(invalid_buffer));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid sType
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetInfo().sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid Type
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();

        build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Total number of triangles in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795");
            build_info.GetSizeInfo();
            m_errorMonitor->VerifyFound();
        }
    }
    // Total number of AABBs in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_info =
                vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
            build_info.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794");
            build_info.GetSizeInfo();
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride in pGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        build_info.GetSizeInfo(false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        build_info.GetSizeInfo(true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in pGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        build_info.GetSizeInfo(false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        build_info.GetSizeInfo(true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex stride
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetGeometries()[0].SetStride(VkDeviceSize(vvl::kU32Max) + 1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid index type
    if (index_type_uint8) {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetGeometries()[0].SetTrianglesIndexType(VK_INDEX_TYPE_UINT8_EXT);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // ppGeometries and pGeometries both valid pointer
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        std::vector<VkAccelerationStructureGeometryKHR> geometries;
        for (const auto &geometry : build_info.GetGeometries()) {
            geometries.emplace_back(geometry.GetVkObj());
        }
        build_info.GetInfo().pGeometries = geometries.data();  // .ppGeometries is set in .BuildCmdBuffer()
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
        // computed scratch buffer size will be 0 since vkGetAccelerationStructureBuildSizesKHR fails
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Buffer is missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR usage flag
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        vkt::Buffer bad_usage_buffer(*m_device, 1024,
                                     VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(bad_usage_buffer), 3);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        auto bad_scratch = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is 0
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        // no VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT => scratch address will be set to 0
        auto bad_scratch = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

// Partially disabled, see https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6040
TEST_F(NegativeRayTracing, AccelerationStructuresOverlappingMemory) {
    TEST_DESCRIPTION(
        "Validate acceleration structure building when source/destination acceleration structures and scratch buffers overlap.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr size_t build_info_count = 3;

    // All buffers used to back source/destination acceleration struct and scratch will be bound to this memory chunk
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 8192;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

#if 0
    // Test overlapping scratch buffers
    {
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = 4096;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (auto &scratch_buffer : scratch_buffers) {
            scratch_buffer = std::make_shared<vkt::Buffer>();
            scratch_buffer->init_no_mem(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(m_device->device(), scratch_buffer->handle(), buffer_memory.handle(), 0);

            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.SetScratchBuffer(std::move(scratch_buffer));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the scratch buffers are bound to the same memory, 03704 will be triggered for each pair of elements in
        // `build_infos`
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR( m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }
#endif

    // Test overlapping destination acceleration structures
    {
        VkBufferCreateInfo dst_blas_buffer_ci = vku::InitStructHelper();
        dst_blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        dst_blas_buffer_ci.size = 4096;
        dst_blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (auto &dst_blas_buffer : dst_blas_buffers) {
            dst_blas_buffer.init_no_mem(*m_device, dst_blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), dst_blas_buffer.handle(), buffer_memory.handle(), 0);

            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffer));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the destination acceleration structures are bound to the same memory, 03702 will be triggered for each pair of
        // elements in `build_infos`
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }
#if 0
    // Test overlapping destination acceleration structure and scratch buffer
    {
        VkBufferCreateInfo dst_blas_buffer_ci = vku::InitStructHelper();
        dst_blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        dst_blas_buffer_ci.size = 4096;
        dst_blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = 4096;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (size_t i = 0; i < build_info_count; ++i) {
            dst_blas_buffers[i].init_no_mem(*m_device, dst_blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), dst_blas_buffers[i].handle(), buffer_memory.handle(), 0);
            scratch_buffers[i] = std::make_shared<vkt::Buffer>();
            scratch_buffers[i]->init_no_mem(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(m_device->device(), scratch_buffers[i]->handle(), buffer_memory.handle(), 0);

            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffers[i]));
            build_info.GetDstAS()->SetSize(4096);
            build_info.SetScratchBuffer(std::move(scratch_buffers[i]));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the destination acceleration structures and scratch buffers are bound to the same memory, 03702, 03703 and
        // 03704 will be triggered for each pair of elements in `build_infos`. 03703 will also be triggered for individual elements.
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");
        }
        for (size_t i = 0; i < build_info_count; ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR( m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }
#endif

    // Test overlapping source acceleration structure and destination acceleration structures
    {
        VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
        blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        blas_buffer_ci.size = 4096;
        blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        std::vector<vkt::Buffer> src_blas_buffers(build_info_count);
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (size_t i = 0; i < build_info_count; ++i) {
            src_blas_buffers[i].init_no_mem(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), src_blas_buffers[i].handle(), buffer_memory.handle(), 0);

            dst_blas_buffers[i].init_no_mem(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), dst_blas_buffers[i].handle(), buffer_memory.handle(), 0);

            // 1st step: build destination acceleration struct
            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(src_blas_buffers[i]));
            build_info.GetDstAS()->SetSize(4096);
            build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
            m_commandBuffer->begin();
            build_info.BuildCmdBuffer(m_commandBuffer->handle());
            m_commandBuffer->end();

            // Possible 2nd step: insert memory barrier on acceleration structure buffer
            // => no need here since 2nd build call will not be executed by the driver

            // 3rd step: set destination acceleration struct as source, create new destination acceleration struct with its
            // underlying buffer bound to the same memory as the source acceleration struct, and build using update mode to trigger
            // 03701
            build_info.SetSrcAS(build_info.GetDstAS());
            build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
            build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
            build_info.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffers[i]));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the source and destination acceleration structures are bound to the same memory, 03701 and 03702 will be
        // triggered for each pair of elements in `build_infos`, and 03668 for each element
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        }
        for (size_t i = 0; i < build_info_count; ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03668");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }
#if 0
    // Test overlapping source acceleration structure and scratch buffer
    {
        VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
        blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        blas_buffer_ci.size = 4096;
        blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = 4096;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        std::vector<vkt::Buffer> src_blas_buffers(build_info_count);
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (size_t i = 0; i < build_info_count; ++i) {
            src_blas_buffers[i].init_no_mem(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), src_blas_buffers[i].handle(), buffer_memory.handle(), 0);

            scratch_buffers[i] = std::make_shared<vkt::Buffer>();
            scratch_buffers[i]->init_no_mem(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(m_device->device(), scratch_buffers[i]->handle(), buffer_memory.handle(), 0);

            // 1st step: build destination acceleration struct
            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(src_blas_buffers[i]));
            build_info.GetDstAS()->SetSize(4096);  // Do this to ensure dst accel struct buffer and scratch do overlap
            build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
            m_commandBuffer->begin();
            build_info.BuildCmdBuffer( m_commandBuffer->handle());
            m_commandBuffer->end();

            // Possible 2nd step: insert memory barrier on acceleration structure buffer
            // => no need here since 2nd build call will not be executed by the driver

            // 3rd step: set destination acceleration struct as source, create new destination acceleration struct,
            // bound scratch buffer to the same memory as the source acceleration struct, and build using update mode to trigger
            // 03705
            build_info.SetSrcAS(build_info.GetDstAS());
            build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
            build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
            build_info.SetScratchBuffer(std::move(scratch_buffers[i]));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the source and destination acceleration structures are bound to the same memory, 03701 and 03702 will be
        // triggered for each pair of elements in `build_infos`. 03705 will also be triggered for individual elements.
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705");
        }
        for (size_t i = 0; i < build_info_count; ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705");
        }

        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR( m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }
#endif
}

TEST_F(NegativeRayTracing, ObjInUseCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    m_commandBuffer->begin();
    build_geometry_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_commandBuffer->end();
    m_default_queue->submit(*m_commandBuffer);

    // This test used to destroy buffers used for building the acceleration structure,
    // to see if there life span was correctly tracked.
    // Following issue 6461, buffers associated to a device address are not tracked anymore, as it is impossible
    // to track the "correct" one: there is not a 1 to 1 mapping between a buffer and a device address.

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    vk::DestroyAccelerationStructureKHR(m_device->handle(), build_geometry_info.GetDstAS()->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->wait();
}

TEST_F(NegativeRayTracing, CmdCopyAccelerationStructureToMemoryKHR) {
    TEST_DESCRIPTION("Validate CmdCopyAccelerationStructureToMemoryKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize buffer_size = 4096;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = buffer_size;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_ci);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
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
    VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = as;
    copy_info.dst = output_data;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    vkt::CommandBuffer cb(m_device, m_commandPool);
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
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_commandBuffer->begin();

    vkt::as::BuildGeometryInfoKHR build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_geometry_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    // Update acceleration structure, with .srcAccelerationStructure == VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630");
    build_geometry_info.BuildCmdBuffer(m_commandBuffer->handle());
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

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Allocate common buffer memory
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers, with correct and incorrect usage
    constexpr size_t N = 3;
    std::array<std::unique_ptr<vkt::as::BuildGeometryInfoKHR>, N> build_geometry_info_vec{};
    const VkBufferUsageFlags good_buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags bad_buffer_usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = good_buffer_usage;
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            buffer_ci.usage = bad_buffer_usage;
        }

        vkt::Buffer vbo;
        vbo.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), vbo.handle(), buffer_memory.handle(), 0);

        vkt::Buffer ibo;
        ibo.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), ibo.handle(), buffer_memory.handle(), 0);

        // Those calls to vkGetBufferDeviceAddressKHR will internally record vbo and ibo device addresses
        {
            const VkDeviceAddress vbo_address = vbo.address();
            const VkDeviceAddress ibo_address = ibo.address();
            if (vbo_address != ibo_address) {
                GTEST_SKIP()
                    << "Bounding two buffers to the same memory location does not result in identical buffer device addresses";
            }
        }
        build_geometry_info_vec[i] = std::make_unique<vkt::as::BuildGeometryInfoKHR>(
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
        build_geometry_info_vec[i]->GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(vbo), 2);
        build_geometry_info_vec[i]->GetGeometries()[0].SetTrianglesDeviceIndexBuffer(std::move(ibo));
    }

    // The first series of calls to vkCmdBuildAccelerationStructuresKHR should succeed,
    // since the first vbo and ibo do have the VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR flag.
    // After deleting the valid vbo and ibo, calls are expected to fail.

    for (size_t i = 0; i < N; ++i) {
        m_commandBuffer->begin();
        build_geometry_info_vec[i]->BuildCmdBuffer(m_commandBuffer->handle());
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
        build_geometry_info_vec[i]->VkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle());
        if (i > 0) {
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();

        build_geometry_info_vec[i] = nullptr;
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesHost) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    // On host query with invalid query type
    vkt::as::BuildGeometryInfoKHR as_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    as_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_commandBuffer->begin();
    as_build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_commandBuffer->end();

    constexpr size_t stride = 1;
    constexpr size_t data_size = sizeof(uint32_t) * stride;
    uint8_t data[data_size];
    // Incorrect query type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
    vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_OCCLUSION, data_size, data, stride);
    m_errorMonitor->VerifyFound();

    // query types not known without extension
    if (IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME)) {
        // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but stride is not a multiple of the size of VkDeviceSize
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, data_size, data, stride);
        m_errorMonitor->VerifyFound();

        // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but stride is not a
        // multiple of the size of VkDeviceSize
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
                                                     data_size, data, stride);
        m_errorMonitor->VerifyFound();
    }
}

// TODO - 7230
// Crahses in BuildGeometryInfoKHR::GetSizeInfo calling vkGetAccelerationStructureBuildSizesKHR on Windows AMD and Android
TEST_F(NegativeRayTracing, DISABLED_WriteAccelerationStructuresPropertiesDevice) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    vkt::as::BuildGeometryInfoKHR as_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_commandBuffer->begin();

    as_build_info.BuildCmdBuffer(m_commandBuffer->handle());
    // Incorrect query type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                    VK_QUERY_TYPE_OCCLUSION, query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesMaintenance1Host) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    RETURN_IF_SKIP(Init());

    constexpr size_t stride = sizeof(VkDeviceSize);
    constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
    uint8_t data[data_size];

    // On host query with invalid query type
    {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_commandBuffer->end();

        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_OCCLUSION, data_size, data, stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with missing BLAS flag
    {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // missing flag
        blas_build_info.SetFlags(0);

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with invalid stride
    {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(m_commandBuffer->handle());
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, data_size, data,
                                                     1);
        m_errorMonitor->VerifyFound();
    }
}
// TODO - 7230
// Crahses in BuildGeometryInfoKHR::GetSizeInfo calling vkGetAccelerationStructureBuildSizesKHR on Windows AMD and Android
TEST_F(NegativeRayTracing, DISABLED_WriteAccelerationStructuresPropertiesMaintenance1Device) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    RETURN_IF_SKIP(Init());

    // On device query with invalid query type
    vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_commandBuffer->begin();

    blas_build_info.BuildCmdBuffer(m_commandBuffer->handle());
    // Incorrect query type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                    VK_QUERY_TYPE_OCCLUSION, query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresDeferredOperation) {
    TEST_DESCRIPTION("Call vkBuildAccelerationStructuresKHR with a valid VkDeferredOperationKHR object");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-deferredOperation-parameter");
    vkt::as::BuildGeometryInfoKHR as_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    as_build_info.SetDeferredOp(CastFromUint64<VkDeferredOperationKHR>(0xdeadbeef));
    as_build_info.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidMode) {
    TEST_DESCRIPTION("Build an acceleration structure with an invalid mode");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.SetMode(static_cast<VkBuildAccelerationStructureModeKHR>(42));
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-mode-04628");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryType) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list changing geometry type");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to geometry type
    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetType(vkt::as::GeometryKHR::Type::AABB);
    build_info.GetGeometries()[0].SetAABBsStride(4096);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03761");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryFlags) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "geometry flag");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to geometry flags
    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetFlags(VK_GEOMETRY_OPAQUE_BIT_KHR);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03762");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryTriaglesFormat) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "vertex format for the triangles");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to triangles vertex format
    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetTrianglesVertexFormat(VK_FORMAT_R16G16B16_SFLOAT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03763");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryTrianglesMaxVertex) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "maxVertex for triangles");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to triangles max vertex
    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetTrianglesMaxVertex(666);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03764");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryTrianglesIndexType) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different index "
        "type for the triangles");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to triangles index type
    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetTrianglesIndexType(VkIndexType::VK_INDEX_TYPE_UINT16);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03765");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryTrianglesTransformData) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "pointer for the transform data");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to triangles transform data
    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetTrianglesTransformatData(666 * 16);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03766");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometry) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with triangles "
        "transform data going from non NULL to NULL");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to triangles transform data
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                           VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    build_info.GetGeometries()[0].SetTrianglesTransformatData(buffer.address());

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    build_info.SetSrcAS(build_info.GetDstAS());
    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    build_info.GetGeometries()[0].SetTrianglesTransformatData(0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03767");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructureMode) {
    TEST_DESCRIPTION("Use invalid mode for vkBuildAccelerationStructureKHR");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    vkt::Buffer host_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    auto bot_level_as = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    bot_level_as.SetMode(static_cast<VkBuildAccelerationStructureModeKHR>(0xbadbeef0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-mode-04628");
    bot_level_as.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, InstanceBufferBadAddress) {
    TEST_DESCRIPTION("Use an invalid address for an instance buffer.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto bot_lvl_as =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));

    m_commandBuffer->begin();
    bot_lvl_as->BuildCmdBuffer(*m_commandBuffer);
    m_commandBuffer->end();

    m_commandBuffer->QueueCommandBuffer();
    vk::DeviceWaitIdle(*m_device);

    auto top_lvl_as = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, bot_lvl_as);

    m_commandBuffer->begin();
    top_lvl_as.SetupBuild(*m_device, true);

    top_lvl_as.GetGeometries()[0].SetInstanceDeviceAddress(0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03813");
    top_lvl_as.VkCmdBuildAccelerationStructuresKHR(*m_commandBuffer);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, InstanceBufferBadMemory) {
    TEST_DESCRIPTION("Use an instance buffer whose memory has been destroyed for an acceleration structure build operation.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto bot_lvl_as =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));

    m_commandBuffer->begin();
    bot_lvl_as->BuildCmdBuffer(*m_commandBuffer);
    m_commandBuffer->end();

    m_commandBuffer->QueueCommandBuffer();
    vk::DeviceWaitIdle(*m_device);

    auto top_lvl_as = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, bot_lvl_as);

    m_commandBuffer->begin();
    top_lvl_as.SetupBuild(*m_device, true);

    top_lvl_as.GetGeometries()[0].GetInstance().buffer.memory().destroy();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03814");
    top_lvl_as.VkCmdBuildAccelerationStructuresKHR(*m_commandBuffer);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, DynamicRayTracingPipelineStack) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline and acceleration structure with a dynamic ray tracing stack size, "
        "but do not set size before tracing rays");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.SetRayGenShader(kRayTracingMinimalGlsl);
    pipeline.AddDynamicState(VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR);
    pipeline.Build();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-None-09458");
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(NegativeRayTracing, UpdatedFirstVertex) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "VkAccelerationStructureBuildRangeInfoKHR::firstVertex");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_commandBuffer->begin();
    auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    build_info.BuildCmdBuffer(m_commandBuffer->handle());

    m_commandBuffer->end();

    m_commandBuffer->QueueCommandBuffer();
    vk::DeviceWaitIdle(*m_device);

    m_commandBuffer->begin();

    build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    build_info.SetSrcAS(build_info.GetDstAS());

    // Create custom build ranges, with the default valid as a template, then somehow supply it?
    auto build_range_infos = build_info.GetDefaultBuildRangeInfos();
    build_range_infos[0].firstVertex = 666;
    build_info.SetBuildRanges(build_range_infos);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-firstVertex-03770");
    build_info.BuildCmdBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}
