/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include "../framework/ray_tracing_objects.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_helper.h"
#include "../layers/utils/math_utils.h"

// Compute a binomial coefficient
template <typename T>
constexpr T binom(T n, T k) {
    static_assert(std::numeric_limits<T>::is_integer, "Unsigned integer required.");
    static_assert(std::is_unsigned<T>::value, "Unsigned integer required.");
    assert(n >= k);
    if (n == 0) {
        return 0;
    }
    if (k == 0) {
        return 1;
    }

    T numerator = 1;
    T denominator = 1;
    for (T i = 1; i <= k; ++i) {
        numerator *= n - i + 1;
        denominator *= i;
    }

    return numerator / denominator;
}

class NegativeRayTracing : public RayTracingTest {};

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

    m_command_buffer.Begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03927");
    m_command_buffer.BarrierKHR(mem_barrier);

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03928");
    m_command_buffer.BarrierKHR(mem_barrier);

    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcStageMask-10751");
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03927");
    m_command_buffer.BarrierKHR(mem_barrier);

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcStageMask-10751");
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03928");
    m_command_buffer.BarrierKHR(mem_barrier);

    m_command_buffer.End();

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

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer;
    buffer_barrier.size = 32;

    vkt::Image image(*m_device, 128, 128, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    m_command_buffer.Begin();

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
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-dstAccessMask-06256");
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dependency_info);

    m_command_buffer.End();

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

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.buffer = buffer;
    buffer_barrier.size = VK_WHOLE_SIZE;

    vkt::Image image(*m_device, 128, 128, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();

    // memory barrier
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstAccessMask-06257");
    // buffer barrier
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstAccessMask-06257");
    // image barrier
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstAccessMask-06257");

    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, 0, 1,
                           &memory_barrier, 1, &buffer_barrier, 1, &image_barrier);
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

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.buffer = buffer;
    buffer_barrier.size = VK_WHOLE_SIZE;

    vkt::Image image(*m_device, 28, 128, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();

    // memory
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-dstAccessMask-06257");
    // buffer
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-dstAccessMask-06257");
    // image
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-dstAccessMask-06257");

    m_command_buffer.WaitEvents(1, &event.handle(), VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
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

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_ALL, nullptr};

    VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout set_layout;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570");
    vk::CreateDescriptorSetLayout(device(), &create_info, nullptr, &set_layout);
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

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
    // Lazy way to not query
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

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
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

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
    // Lazy way to not query
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

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
    // Lazy way to not query
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

            m_command_buffer.Begin();
            m_errorMonitor->SetDesiredError(vuid_begin_query);
            vk::CmdBeginQuery(m_command_buffer, query_pool, 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredError(vuid_begin_query_indexed);
                vk::CmdBeginQueryIndexedEXT(m_command_buffer, query_pool, 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_command_buffer.End();
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

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-04729");
        vk::CmdBeginQuery(m_command_buffer, query_pool, 0, 0);
        m_errorMonitor->VerifyFound();

        if (ext_transform_feedback) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-04729");
            vk::CmdBeginQueryIndexedEXT(m_command_buffer, query_pool, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }
        m_command_buffer.End();
    }
}

TEST_F(NegativeRayTracing, CopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffer not bound to memory");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas_no_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Create();

    auto valid_blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    valid_blas->Create();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_no_mem->handle();
    copy_info.dst = valid_blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = valid_blas->handle();
    copy_info.dst = blas_no_mem->handle();
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CopyAccelerationStructureOverlappingMemory) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with both acceleration structures bound to the same memory");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas_1 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    const VkAccelerationStructureBuildSizesInfoKHR size_info = blas_1.GetSizeInfo();

    VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
    blas_buffer_ci.size = size_info.accelerationStructureSize;
    blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                           VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer_1(*m_device, blas_buffer_ci, vkt::no_mem);
    vkt::Buffer buffer_2(*m_device, blas_buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs{};
    vk::GetBufferMemoryRequirements(device(), buffer_1, &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);

    alloc_info.allocationSize = std::max(size_info.accelerationStructureSize, mem_reqs.size);
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    buffer_1.BindMemory(buffer_memory, 0);
    buffer_2.BindMemory(buffer_memory, 0);

    blas_1.GetDstAS()->SetDeviceBuffer(std::move(buffer_1));
    m_command_buffer.Begin();
    blas_1.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_device->Wait();

    auto empty_blas_2 = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, size_info.accelerationStructureSize);
    empty_blas_2->SetDeviceBuffer(std::move(buffer_2));
    empty_blas_2->Create();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_1.GetDstAS()->handle();
    copy_info.dst = empty_blas_2->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-dst-07791");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdCopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffers not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);
    VkMemoryRequirements memory_requirements = buffer.MemoryRequirements();
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper(&alloc_flags);
    memory_alloc.allocationSize = memory_requirements.size;
    const bool memory_found =
        m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_alloc, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!memory_found) {
        GTEST_SKIP() << "Could not find suitable memory type, skipping test";
    }
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(*m_device, buffer, device_memory, 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->SetDeviceBuffer(std::move(buffer));
    blas->Create();

    auto blas_no_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Create();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_no_mem->handle();
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    copy_info.src = blas->handle();
    copy_info.dst = blas_no_mem->handle();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CopyAccelerationStructureNoHostMem) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffers not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);
    VkMemoryRequirements memory_requirements = buffer.MemoryRequirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_alloc, 0,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(*m_device, buffer, device_memory, 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, 4096);
    blas->Create();

    auto blas_no_host_mem = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, 4096);
    blas_no_host_mem->SetDeviceBuffer(std::move(buffer));
    blas_no_host_mem->Create();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    copy_info.src = blas_no_host_mem->handle();
    copy_info.dst = blas->handle();

    m_errorMonitor->SetDesiredError("VUID-vkCopyAccelerationStructureKHR-buffer-03727");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = blas->handle();
    copy_info.dst = blas_no_host_mem->handle();

    m_errorMonitor->SetDesiredError("VUID-vkCopyAccelerationStructureKHR-buffer-03728");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdCopyMemoryToAccelerationStructure) {
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
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer src_buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer dst_buffer(*m_device, buffer_ci, vkt::no_mem);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 0);
    blas->SetDeviceBuffer(std::move(dst_buffer));
    blas->Create();

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src.deviceAddress = src_buffer.Address();
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // Acceleration structure buffer is not bound to memory
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    m_command_buffer.Begin();
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate buffers used in vkBuildAccelerationStructureKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer non_host_visible_buffer(*m_device, buffer_ci, vkt::no_mem);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.MemoryRequirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_alloc, 0,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(*m_device, non_host_visible_buffer, device_memory, 0);

    vkt::Buffer host_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    blas.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    blas.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructureModeUpdate) {
    TEST_DESCRIPTION("In an acceleration structure update, source acceleration structure was not built");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(Init());

    auto host_cached_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);

    host_cached_blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    host_cached_blas.SetSrcAS(vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, 4096));
    host_cached_blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03667");
    host_cached_blas.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructureMemory) {
    TEST_DESCRIPTION("Test memory in vkWriteAccelerationStructuresPropertiesKHR is host visible");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    // Init a non host visible buffer
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer non_host_visible_buffer(*m_device, buffer_ci, vkt::no_mem);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.MemoryRequirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_alloc, 0,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(*m_device, non_host_visible_buffer, device_memory, 0);

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    blas.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    blas.BuildHost();
    m_errorMonitor->VerifyFound();

    std::vector<uint32_t> data(4096);
    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");
    vk::WriteAccelerationStructuresPropertiesKHR(device(), 1, &blas.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data.size(), data.data(),
                                                 data.size());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CopyMemoryToAccelerationStructureHostAddress) {
    TEST_DESCRIPTION("vkCopyMemoryToAccelerationStructureKHR but the hostAddress is not aligned to 16.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    auto blas = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, 4096);
    blas->Create();

    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(0x00000021);

    VkCopyMemoryToAccelerationStructureInfoKHR info = vku::InitStructHelper();
    info.dst = blas->handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03750");
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
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
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer non_host_visible_buffer(*m_device, buffer_ci, vkt::no_mem);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.MemoryRequirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_alloc, 0,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(*m_device, non_host_visible_buffer, device_memory, 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, buffer_ci.size);
    blas->SetDeviceBuffer(std::move(non_host_visible_buffer));
    blas->Create();

    uint8_t output[4096];
    uint8_t *aligned_output = reinterpret_cast<uint8_t *>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(output), 16));
    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = aligned_output;

    VkCopyMemoryToAccelerationStructureInfoKHR info = vku::InitStructHelper();
    info.dst = blas->handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredError("VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, NullCreateAccelerationStructureKHR) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-vkCreateAccelerationStructureKHR-pCreateInfo-parameter");
    vk::CreateAccelerationStructureKHR(device(), nullptr, nullptr, &as);
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
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
    as_create_info.buffer = buffer;
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer};
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);
    // invalid buffer;
    {
        vkt::Buffer invalid_buffer(*m_device, 4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.buffer = invalid_buffer;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid deviceAddress and flag;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.deviceAddress = device_address;
        m_errorMonitor->SetDesiredError("VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();

        invalid_as_create_info.deviceAddress = 0;
        invalid_as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-createFlags-parameter");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid size and offset;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.size = 4097;  // buffer size is 4096
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-offset-03616");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid sType;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-sType-sType");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid type;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-type-parameter");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_1);

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer;
    as_create_info.size = 4096;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCreateAccelerationStructureKHR-accelerationStructure-03611");
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-08934");
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHRReplayFeature) {
    TEST_DESCRIPTION("Validate acceleration structure creation replay feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    VkBufferDeviceAddressInfo device_address_info = vku::InitStructHelper();
    device_address_info.buffer = buffer;
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer;
    as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = device_address;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613");
    m_errorMonitor->SetDesiredError("VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488");
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, GetAccelerationStructureAddressBabBuffer) {
    TEST_DESCRIPTION(
        "Call vkGetAccelerationStructureDeviceAddressKHR on an acceleration structure whose buffer is missing usage "
        "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, and whose memory has been destroyed");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    blas->Create();

    blas->GetBuffer().Memory().Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureDeviceAddressKHR-pInfo-09541");
    m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureDeviceAddressKHR-pInfo-09542");
    (void)blas->GetAccelerationStructureDeviceAddress();
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
        shader_stages[0].module = chit_shader;
        shader_stages[0].pName = "main";

        shader_stages[1] = vku::InitStructHelper();
        shader_stages[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        shader_stages[1].module = rgen_shader;
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
        raytracing_pipeline_ci.layout = pipeline_layout;

        const VkResult result = vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                                 &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
        ASSERT_EQ(VK_SUCCESS, result);
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer, mem, 0);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    const VkDeviceAddress device_address = buffer.Address();

    const VkStridedDeviceAddressRegionKHR stridebufregion = {device_address, ray_tracing_properties.shaderGroupHandleAlignment,
                                                             ray_tracing_properties.shaderGroupHandleAlignment};

    m_command_buffer.Begin();

    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-stride-03694");
            vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100, 100,
                                1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-stride-03690");
            vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100, 100,
                                1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-stride-03686");
            vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100, 100,
                                1);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride = static_cast<VkDeviceSize>(ray_tracing_properties.maxShaderGroupStride) +
                                (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-stride-04041");
        vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100, 100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-stride-04035");
        vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100, 100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-stride-04029");
        vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    {
        buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        vkt::Buffer buffer_missing_flag(*m_device, buffer_ci, vkt::no_mem);
        vk::BindBufferMemory(device(), buffer_missing_flag, mem, 0);
        const VkDeviceAddress device_address_missing_flag = buffer_missing_flag.Address();

        // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
        if (device_address_missing_flag == device_address) {
            VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
            // This address is the same as the one from the first (valid) buffer, so no validation error
            invalid_stride.deviceAddress = device_address_missing_flag;
            vk::CmdTraceRaysKHR(m_command_buffer, &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100, 100,
                                1);
        }
    }

    // pRayGenShaderBindingTable address range and stride are invalid
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = 8128;
        invalid_stride.size = 8128;
        m_errorMonitor->SetDesiredError("VUID-VkStridedDeviceAddressRegionKHR-size-04631");
        m_errorMonitor->SetDesiredError("VUID-VkStridedDeviceAddressRegionKHR-size-04632");
        vk::CmdTraceRaysKHR(m_command_buffer, &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    // pMissShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }
    // pHitShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100, 100, 1);
    }
    // pCallableShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_command_buffer, &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100, 100, 1);
    }

    m_command_buffer.End();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirectKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    const VkDeviceAddress device_address = buffer.Address();

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_command_buffer.Begin();
    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        invalid_stride.size = invalid_stride.stride;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_command_buffer, &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                        device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_command_buffer, &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                        device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vk::CmdTraceRaysIndirectKHR(m_command_buffer, &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                        device_address);
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

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_command_buffer, &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                        device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_command_buffer, &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                        device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-04029");
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vk::CmdTraceRaysIndirectKHR(m_command_buffer, &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                        device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirect2KHRFeatureDisabled) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirect2KHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    const VkDeviceAddress device_address = buffer.Address();

    m_command_buffer.Begin();
    // No VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR was in the device create info pNext chain
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637");
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer, device_address);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirect2KHRAddress) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirect2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    const VkDeviceAddress device_address = buffer.Address();

    m_command_buffer.Begin();
    // indirectDeviceAddress is not a multiple of 4
    m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634");
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, device_address + 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirectDeviceAddressUsage) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    const VkDeviceAddress device_address = buffer.Address();

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();
    pipeline.Build();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03633");
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, device_address);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirectDeviceAddress) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();
    pipeline.Build();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    VkDeviceAddress invalid_buffer = CastToHandle<VkDeviceAddress, uintptr_t>(0xbad00000);
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, invalid_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureVersionInfoKHR-sType-sType");
        vk::GetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.pVersionData = NULL;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureVersionInfoKHR-pVersionData-parameter");
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
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetDstAS(vkt::as::blueprint::AccelStructNull(*m_device));
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800");
    blas.BuildCmdBufferIndirect(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, IndirectStridesMultiple) {
    TEST_DESCRIPTION("pIndirectStrides not a multiple of 4.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureIndirectBuild);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetIndirectStride(13);
    blas.SetIndirectDeviceAddress(13);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pIndirectStrides-03787");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pIndirectDeviceAddresses-03648");
    blas.BuildCmdBufferIndirect(m_command_buffer);
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
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    blas.SetDstAS(vkt::as::blueprint::AccelStructNull(*m_device));
    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
    blas.BuildHost();
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
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // dstAccelerationStructure == VK_NULL_HANDLE
    {  // Command buffer build
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetDstAS(vkt::as::blueprint::AccelStructNull(*m_device));

        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // Positive build tests
    {
        m_command_buffer.Begin();
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.BuildCmdBuffer(m_command_buffer);
        m_command_buffer.End();
    }

    {
        m_command_buffer.Begin();
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetupBuild(true);
        blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer, false);
        m_command_buffer.End();
    }

    m_command_buffer.Begin();

    // Invalid info count
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetInfoCount(0);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // Invalid pInfos
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetNullInfos(true);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-parameter");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // Invalid ppBuildRangeInfos
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetNullBuildRangeInfos(true);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-parameter");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // must be called outside renderpass
    {
        InitRenderTarget();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-renderpass");
        blas.BuildCmdBuffer(m_command_buffer);
        m_command_buffer.EndRenderPass();
        m_errorMonitor->VerifyFound();
    }
    // Invalid flags
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // Invalid dst buffer
    {
        // 8192 is large enough for all known driver
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        auto buffer_ci = vkt::Buffer::CreateInfo(blas.GetSizeInfo().accelerationStructureSize,
                                                 VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        vkt::Buffer invalid_buffer(*m_device, buffer_ci, vkt::no_mem);
        blas.GetDstAS()->SetDeviceBuffer(std::move(invalid_buffer));
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Invalid sType
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetInfo().sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Invalid Type
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetType(VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR);
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();

        blas.SetType(VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR);
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Total number of triangles in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795");
            blas.GetSizeInfo();
            m_errorMonitor->VerifyFound();
        }
    }
    // Total number of AABBs in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
            blas.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794");
            blas.GetSizeInfo();
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride in pGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        blas.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        blas.GetSizeInfo(false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        blas.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        blas.GetSizeInfo(true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in pGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        blas.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        blas.GetSizeInfo(false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        blas.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        blas.GetSizeInfo(true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex stride
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetGeometries()[0].SetStride(VkDeviceSize(vvl::kU32Max) + 1);
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index type
    if (index_type_uint8) {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetGeometries()[0].SetTrianglesIndexType(VK_INDEX_TYPE_UINT8);
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // ppGeometries and pGeometries both valid pointer
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
        std::vector<VkAccelerationStructureGeometryKHR> geometries;
        for (const auto &geometry : blas.GetGeometries()) {
            geometries.emplace_back(geometry.GetVkObj());
        }
        blas.GetInfo().pGeometries = geometries.data();  // .ppGeometries is set in .BuildCmdBuffer()
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
        // computed scratch buffer size will be 0 since vkGetAccelerationStructureBuildSizesKHR fails
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Buffer is missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR usage flag
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer bad_usage_buffer(*m_device, 1024,
                                     VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                     kHostVisibleMemProps, &alloc_flags);
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(bad_usage_buffer), 3);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag
    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        const VkDeviceSize scratch_size =
            blas.GetSizeInfo().buildScratchSize + acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        auto bad_scratch = std::make_shared<vkt::Buffer>(*m_device, scratch_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
        blas.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is 0
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        // no VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT => scratch address will be set to 0
        auto bad_scratch = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        blas.BuildCmdBuffer(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, AccelerationStructuresOverlappingMemory) {
    TEST_DESCRIPTION("Validate acceleration structure building when source/destination acceleration structures overlap.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    if (IsPlatformMockICD()) {
        GTEST_SKIP()
            << "Test needs acceleration structures addresses to be related to the buffer backing them, mock ICD does not do that";
    }

    constexpr size_t build_info_count = 3;

    // To get a valid VkAccelerationStructureBuildSizesInfoKHR
    auto dummy_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    dummy_blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    const VkAccelerationStructureBuildSizesInfoKHR dummy_size_info = dummy_blas.GetSizeInfo();

    VkBufferCreateInfo dst_blas_buffer_ci = vku::InitStructHelper();
    dst_blas_buffer_ci.size = dummy_size_info.accelerationStructureSize;
    dst_blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // All buffers used to back source/destination acceleration structures will be bound to this memory chunk
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = dummy_size_info.accelerationStructureSize;

    {
        vkt::Buffer dummy_blas_buffer(*m_device, dst_blas_buffer_ci, vkt::no_mem);
        alloc_info.allocationSize = std::max(alloc_info.allocationSize, dummy_blas_buffer.MemoryRequirements().size);
    }

    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test overlapping destination acceleration structures
    {
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (auto &dst_blas_buffer : dst_blas_buffers) {
            dst_blas_buffer.InitNoMemory(*m_device, dst_blas_buffer_ci);
            vk::BindBufferMemory(device(), dst_blas_buffer, buffer_memory, 0);

            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffer));
            build_infos.emplace_back(std::move(blas));
        }

        // Since all the destination acceleration structures are bound to the same memory, 03702 should be triggered for each pair
        // of elements in `build_infos`
        // => due to validation code optimisations, not *all* overlaps will be detected,
        // but if there is *at least one*, it will *always+ be detected.
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        m_command_buffer.Begin();
        vkt::as::BuildAccelerationStructuresKHR(m_command_buffer, build_infos);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }

    // Test overlapping source acceleration structure and destination acceleration structures
    {
        VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
        blas_buffer_ci.size = dummy_size_info.accelerationStructureSize;
        blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        std::vector<vkt::Buffer> src_blas_buffers(build_info_count);
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
        for (size_t i = 0; i < build_info_count; ++i) {
            src_blas_buffers[i].InitNoMemory(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(device(), src_blas_buffers[i], buffer_memory, 0);

            dst_blas_buffers[i].InitNoMemory(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(device(), dst_blas_buffers[i], buffer_memory, 0);

            // 1st step: build destination acceleration struct
            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.GetDstAS()->SetDeviceBuffer(std::move(src_blas_buffers[i]));
            blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
            m_command_buffer.Begin();
            blas.BuildCmdBuffer(m_command_buffer);
            m_command_buffer.End();

            // Possible 2nd step: insert memory barrier on acceleration structure buffer
            // => no need here since 2nd build call will not be executed by the driver

            // 3rd step: set destination acceleration struct as source, create new destination acceleration struct with its
            // underlying buffer bound to the same memory as the source acceleration struct, and build using update mode to trigger
            // 03701
            blas.SetSrcAS(blas.GetDstAS());
            blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
            blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
            blas.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffers[i]));
            blas_vec.emplace_back(std::move(blas));
        }

        // Since all the source and destination acceleration structures are bound to the same memory, 03701 and 03702 should be
        // triggered for each pair of elements in `build_infos`, and 03668 for each element
        // => due to validation code optimisations, not *all* overlaps described by 03701 and 03702 will be detected,
        // but if there is *at least one*, it will *always+ be detected.
        // Also, the emitted errors will depend on how std::lower_bound operates given an input device address range,
        // so it is unpredicatble: use SetAllowedFailureMsg to cope
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03668");

        m_command_buffer.Begin();
        vkt::as::BuildAccelerationStructuresKHR(m_command_buffer, blas_vec);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, AccelerationStructuresOverlappingMemory2) {
    TEST_DESCRIPTION("Validate acceleration structure building when scratch buffers overlap.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr size_t build_info_count = 3;

    // To get a valid VkAccelerationStructureBuildSizesInfoKHR
    auto dummy_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    const VkAccelerationStructureBuildSizesInfoKHR dummy_size_info = dummy_blas.GetSizeInfo();

    VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
    scratch_buffer_ci.size = dummy_size_info.buildScratchSize;
    scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // All buffers used to back scratch buffers will be bound to this memory chunk
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = std::max(dummy_size_info.accelerationStructureSize, dummy_size_info.buildScratchSize);

    {
        vkt::Buffer dummy_scratch_buffer(*m_device, scratch_buffer_ci, vkt::no_mem);
        alloc_info.allocationSize = std::max(alloc_info.allocationSize, dummy_scratch_buffer.MemoryRequirements().size);
    }

    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test overlapping scratch buffers
    {
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;

        VkDeviceAddress ref_address = 0;
        for (auto &scratch_buffer : scratch_buffers) {
            scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, scratch_buffer_ci, vkt::no_mem);
            vk::BindBufferMemory(device(), scratch_buffer->handle(), buffer_memory, 0);

            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.SetScratchBuffer(std::move(scratch_buffer));
            if (ref_address == 0) {
                ref_address = blas.GetScratchBuffer()->Address();
            } else {
                if (blas.GetScratchBuffer()->Address() != ref_address) {
                    GTEST_SKIP()
                        << "Bounding two buffers to the same memory location does not result in identical buffer device addresses";
                }
            }
            build_infos.emplace_back(std::move(blas));
        }

        // Since all the scratch buffers are bound to the same memory, 03704 will be triggered for each pair of elements in
        // `build_infos`
        // => due to validation code optimisations, not *all* overlaps will be detected,
        // but if there is *at least one*, it will *always+ be detected.
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704", 2);
        m_command_buffer.Begin();
        vkt::as::BuildAccelerationStructuresKHR(m_command_buffer, build_infos);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, AccelerationStructuresOverlappingMemory3) {
    TEST_DESCRIPTION(
        "Validate acceleration structure building when destination acceleration structures and scratch buffers overlap.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Buffer device addresses computed by the mock ICD are not aligned to the strictest alignment possible,
    // but this test has been written with this assumption in mind to make sure scratch buffers do overlap with each other and with
    // src acceleration structures
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr size_t build_info_count = 3;

    // To get a valid VkAccelerationStructureBuildSizesInfoKHR
    auto dummy_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    const VkAccelerationStructureBuildSizesInfoKHR dummy_size_info = dummy_blas.GetSizeInfo();

    VkBufferCreateInfo dst_blas_buffer_ci = vku::InitStructHelper();
    dst_blas_buffer_ci.size = dummy_size_info.accelerationStructureSize;  // large enough for all known driver
    dst_blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
    scratch_buffer_ci.size = dummy_size_info.buildScratchSize;
    scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // All buffers used to back destination acceleration struct and scratch will be bound to this memory chunk
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);

    alloc_info.allocationSize = std::max(dummy_size_info.accelerationStructureSize, dummy_size_info.buildScratchSize);

    {
        vkt::Buffer dummy_blas_buffer(*m_device, dst_blas_buffer_ci, vkt::no_mem);
        vkt::Buffer dummy_scratch_buffer(*m_device, scratch_buffer_ci, vkt::no_mem);
        alloc_info.allocationSize = std::max(alloc_info.allocationSize, dummy_blas_buffer.MemoryRequirements().size);
        alloc_info.allocationSize = std::max(alloc_info.allocationSize, dummy_scratch_buffer.MemoryRequirements().size);
    }

    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test overlapping destination acceleration structure and scratch buffer
    {
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
        for (size_t i = 0; i < build_info_count; ++i) {
            dst_blas_buffers[i].InitNoMemory(*m_device, dst_blas_buffer_ci);
            vk::BindBufferMemory(device(), dst_blas_buffers[i].handle(), buffer_memory, 0);
            scratch_buffers[i] = std::make_shared<vkt::Buffer>();
            scratch_buffers[i]->InitNoMemory(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(device(), scratch_buffers[i]->handle(), buffer_memory, 0);

            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffers[i]));
            blas.SetScratchBuffer(std::move(scratch_buffers[i]));
            blas_vec.emplace_back(std::move(blas));
        }

        // Since all the destination acceleration structures and scratch buffers are bound to the same memory, 03702, 03703 and
        // 03704 *should* be triggered for each pair of elements in `build_infos`. 03703 *should* also be triggered for individual
        // elements.
        // => due to validation code optimisations, not *all* overlaps will be detected,
        // but if there is *at least one*, it will *always+ be detected.
        // Also, the emitted errors will depend on how std::lower_bound operates given an input device address range,
        // so it is unpredicatble: use SetAllowedFailureMsg to cope
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");

        m_command_buffer.Begin();
        vkt::as::BuildAccelerationStructuresKHR(m_command_buffer, blas_vec);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, AccelerationStructuresOverlappingMemory4) {
    TEST_DESCRIPTION(
        "Validate acceleration structure building when source/destination acceleration structures and scratch buffers overlap.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Buffer device addresses computed by the mock ICD are not aligned to the strictest alignment possible,
    // but this test has been written with this assumption in mind to make sure scratch buffers do overlap with each other and with
    // src acceleration structures
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr size_t build_info_count = 3;

    // To get a valid VkAccelerationStructureBuildSizesInfoKHR
    auto dummy_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    dummy_blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    const VkAccelerationStructureBuildSizesInfoKHR dummy_size_info = dummy_blas.GetSizeInfo();

    VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
    blas_buffer_ci.size = dummy_size_info.accelerationStructureSize;
    blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                           VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
    scratch_buffer_ci.size = dummy_size_info.buildScratchSize;
    scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // All buffers used to back source/destination acceleration structures will be bound to this memory chunk
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = std::max(dummy_size_info.accelerationStructureSize, dummy_size_info.buildScratchSize);

    {
        vkt::Buffer dummy_blas_buffer(*m_device, blas_buffer_ci, vkt::no_mem);
        alloc_info.allocationSize = std::max(alloc_info.allocationSize, dummy_blas_buffer.MemoryRequirements().size);
        vkt::Buffer dummy_scratch_buffer(*m_device, scratch_buffer_ci, vkt::no_mem);
        alloc_info.allocationSize = std::max(alloc_info.allocationSize, dummy_scratch_buffer.MemoryRequirements().size);
    }

    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test overlapping source acceleration structure and scratch buffer
    {
        std::vector<vkt::Buffer> src_blas_buffers(build_info_count);
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;

        for (size_t i = 0; i < build_info_count; ++i) {
            src_blas_buffers[i].InitNoMemory(*m_device, blas_buffer_ci);
            src_blas_buffers[i].BindMemory(buffer_memory, 0);

            scratch_buffers[i] = std::make_shared<vkt::Buffer>();
            scratch_buffers[i]->InitNoMemory(*m_device, scratch_buffer_ci);
            scratch_buffers[i]->BindMemory(buffer_memory, 0);

            // 1st step: build destination acceleration struct
            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.GetDstAS()->SetDeviceBuffer(std::move(src_blas_buffers[i]));
            blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
            m_command_buffer.Begin();
            blas.BuildCmdBuffer(m_command_buffer);
            m_command_buffer.End();
            m_default_queue->Wait();

            // Possible 2nd step: insert memory barrier on acceleration structure buffer
            // => no need here since 2nd build call will not be executed by the driver

            // 3rd step: set destination acceleration struct as source, create new destination acceleration struct,
            // bound scratch buffer to the same memory as the source acceleration struct, and build using update mode to trigger
            // 03705
            blas.SetSrcAS(blas.GetDstAS());
            blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
            blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
            blas.SetScratchBuffer(std::move(scratch_buffers[i]));
            blas_vec.emplace_back(std::move(blas));
        }

        // Since all the source and destination acceleration structures are bound to the same memory, 03704 and 03705 *should* be
        // triggered for each pair of elements in `build_infos`. 03705 *should* also be triggered for individual elements.
        // => due to validation code optimisations, not *all* overlaps will be detected,
        // but if there is *at least one*, it will *always+ be detected.
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704", 2);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705", 3);

        m_command_buffer.Begin();
        vkt::as::BuildAccelerationStructuresKHR(m_command_buffer, blas_vec);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, ObjInUseCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    // This test used to destroy buffers used for building the acceleration structure,
    // to see if there life span was correctly tracked.
    // Following issue 6461, buffers associated to a device address are not tracked anymore, as it is impossible
    // to track the "correct" one: there is not a 1 to 1 mapping between a buffer and a device address.

    m_errorMonitor->SetDesiredError("VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    vk::DestroyAccelerationStructureKHR(*m_device, blas.GetDstAS()->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeRayTracing, CmdCopyAccelerationStructureToMemoryKHR) {
    TEST_DESCRIPTION(
        "vkCmdCopyAccelerationStructureToMemoryKHR with src acceleration structure not bound to memory and invalid destination "
        "address");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->SetDeviceBufferInitNoMem(true);
    blas->Create();

    VkDeviceOrHostAddressKHR output_data;
    output_data.deviceAddress = 256;
    VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas->handle();
    copy_info.dst = output_data;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-None-03559");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-04959");
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CopyAccelerationStructureToMemoryKHR) {
    TEST_DESCRIPTION("vkCopyAccelerationStructureToMemoryKHR with src acceleration structure not bound to host visible memory");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Init a non host visible buffer
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);
    VkMemoryRequirements memory_requirements = buffer.MemoryRequirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_alloc, 0,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(*m_device, buffer, device_memory, 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->SetDeviceBuffer(std::move(buffer));
    blas->Create();

    std::vector<uint8_t> data(4096, 0);
    VkDeviceOrHostAddressKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(Align<uintptr_t>(reinterpret_cast<uintptr_t>(data.data()), 16));
    VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas->handle();
    copy_info.dst = output_data;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCopyAccelerationStructureToMemoryKHR-buffer-03731");
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-04959");
    vk::CopyAccelerationStructureToMemoryKHR(*m_device, VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdCopyMemoryToAccelerationStructureKHRInvalidSrcBuffer) {
    TEST_DESCRIPTION("Validate vkCmdCopyMemoryToAccelerationStructureKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->Create();

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src.deviceAddress = 256;
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, UpdateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Test for updating an acceleration structure without a srcAccelerationStructure");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_command_buffer.Begin();

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    // Update acceleration structure, with .srcAccelerationStructure == VK_NULL_HANDLE
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
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
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers, with correct and incorrect usage
    constexpr size_t N = 3;
    std::array<std::unique_ptr<vkt::as::BuildGeometryInfoKHR>, N> blas_vec{};
    const VkBufferUsageFlags good_buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags bad_buffer_usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = good_buffer_usage;
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            buffer_ci.usage = bad_buffer_usage;
        }

        vkt::Buffer vbo(*m_device, buffer_ci, vkt::no_mem);
        vk::BindBufferMemory(device(), vbo, buffer_memory, 0);

        vkt::Buffer ibo(*m_device, buffer_ci, vkt::no_mem);
        vk::BindBufferMemory(device(), ibo, buffer_memory, 0);

        // Those calls to vkGetBufferDeviceAddressKHR will internally record vbo and ibo device addresses
        {
            const VkDeviceAddress vbo_address = vbo.Address();
            const VkDeviceAddress ibo_address = ibo.Address();
            if (vbo_address != ibo_address) {
                GTEST_SKIP()
                    << "Bounding two buffers to the same memory location does not result in identical buffer device addresses";
            }
        }
        blas_vec[i] = std::make_unique<vkt::as::BuildGeometryInfoKHR>(
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
        blas_vec[i]->GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(vbo), 2);
        blas_vec[i]->GetGeometries()[0].SetTrianglesDeviceIndexBuffer(std::move(ibo));
    }

    // The first series of calls to vkCmdBuildAccelerationStructuresKHR should succeed,
    // since the first vbo and ibo do have the VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR flag.
    // After deleting the valid vbo and ibo, calls are expected to fail.

    for (size_t i = 0; i < N; ++i) {
        m_command_buffer.Begin();
        blas_vec[i]->BuildCmdBuffer(m_command_buffer);
        m_command_buffer.End();
    }

    for (size_t i = 0; i < N; ++i) {
        m_command_buffer.Begin();
        if (i > 0) {
            // for vbo
            m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
            // for ibo
            m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        }
        blas_vec[i]->VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
        if (i > 0) {
            m_errorMonitor->VerifyFound();
        }
        m_command_buffer.End();

        blas_vec[i] = nullptr;
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
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_device->Wait();

    constexpr size_t stride = 1;
    constexpr size_t data_size = sizeof(uint32_t) * stride;
    uint8_t data[data_size];
    // Incorrect query type
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(), VK_QUERY_TYPE_OCCLUSION, data_size, data,
                                                 stride);
    m_errorMonitor->VerifyFound();

    // query types not known without extension
    if (IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME)) {
        // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but stride is not a multiple of the size of VkDeviceSize
        m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731");
        vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, data_size, data, stride);
        m_errorMonitor->VerifyFound();

        // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but stride is not a
        // multiple of the size of VkDeviceSize
        m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733");
        vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
                                                     data_size, data, stride);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesDevice) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_device->Wait();

    m_command_buffer.Begin();
    // Incorrect query type
    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer, 1, &blas.GetDstAS()->handle(), VK_QUERY_TYPE_OCCLUSION,
                                                    query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesAccelStructDestroyedMemory) {
    TEST_DESCRIPTION(
        "call CmdWriteAccelerationStructuresPropertiesKHR on an acceleration structure whose buffer memory has been destroyed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, 1);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_device->Wait();

    blas.GetDstAS()->GetBuffer().Memory().Destroy();

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("vkCmdWriteAccelerationStructuresPropertiesKHR-buffer-03736");
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer, 1, &blas.GetDstAS()->handle(),
                                                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesDeviceBlasNotBuilt) {
    TEST_DESCRIPTION("vkCmdWriteAccelerationStructuresPropertiesKHR with unbuilt blas");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    blas.GetDstAS()->Create();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, 1);

    m_command_buffer.Begin();
    // blas not built
    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-04964");
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer, 1, &blas.GetDstAS()->handle(),
                                                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
        vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        m_command_buffer.Begin();
        blas.BuildCmdBuffer(m_command_buffer);
        m_command_buffer.End();

        // Incorrect query type
        m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(), VK_QUERY_TYPE_OCCLUSION, data_size,
                                                     data, stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with missing BLAS flag
    {
        vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // missing flag
        blas.SetFlags(0);

        m_command_buffer.Begin();
        blas.BuildCmdBuffer(m_command_buffer);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431");
        vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with invalid stride
    {
        vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        m_command_buffer.Begin();
        blas.BuildCmdBuffer(m_command_buffer);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448");
        vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450");
        vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, data_size, data,
                                                     1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesMaintenance1Device) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // On device query with invalid query type
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_device->Wait();

    m_command_buffer.Begin();
    // Incorrect query type
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer, 1, &blas.GetDstAS()->handle(), VK_QUERY_TYPE_OCCLUSION,
                                                    query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesDataSizeTooSmall) {
    TEST_DESCRIPTION(
        "Call vkWriteAccelerationStructuresPropertiesKHR with a dataSize that is smaller that VkDeviceSize for relevant query "
        "types");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // On host query with invalid query type
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    blas.GetDstAS()->Create();

    constexpr size_t stride = 1;
    constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
    uint8_t data[data_size];

    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-04964");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                 sizeof(VkDeviceSize));
    m_errorMonitor->VerifyFound();

    blas.BuildHost();

    // Incorrect data size

    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448");
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03449");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, 1, data, stride);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450");
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03451");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, 1, data, stride);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731");
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06732");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, 1, data, stride);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733");
    m_errorMonitor->SetDesiredError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06734");
    vk::WriteAccelerationStructuresPropertiesKHR(*m_device, 1, &blas.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, 1,
                                                 data, stride);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresDeferredOperation) {
    TEST_DESCRIPTION("Call vkBuildAccelerationStructuresKHR with an invalid VkDeferredOperationKHR object");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-deferredOperation-parameter");
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetDeferredOp(CastFromUint64<VkDeferredOperationKHR>(0xdeadbeef));
    blas.BuildHost();
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

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetMode(static_cast<VkBuildAccelerationStructureModeKHR>(42));
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-mode-04628");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer);

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    const VkGeometryFlagsKHR geometry_flags = blas.GetGeometries()[0].GetFlags();
    blas.GetGeometries()[0] = vkt::as::blueprint::GeometrySimpleOnDeviceAABBInfo(*m_device);
    blas.GetGeometries()[0].SetFlags(geometry_flags);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03761");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer);

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    blas.GetGeometries()[0].SetFlags((VkGeometryFlagsKHR)0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03762");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructuresInvalidUpdatesToGeometryTrianglesFormat) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "vertex format for the triangles");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const VkFormat second_triangles_vertex_format = VK_FORMAT_R16G16B16_SFLOAT;

    // Invalid update to triangles vertex format
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer);

    m_command_buffer.End();
    m_device->Wait();
    m_command_buffer.Begin();

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetScratchBuffer(std::make_shared<vkt::Buffer>());
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    blas.GetGeometries()[0].SetTrianglesVertexFormat(second_triangles_vertex_format);

    if (!BufferFormatAndFeaturesSupported(Gpu(), second_triangles_vertex_format,
                                          VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-03797");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03763");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TrianglesFormatMissingFeature) {
    TEST_DESCRIPTION("Use a triangles vertex format that misses the acceleration structure vertex buffer format feature");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const VkFormat triangles_vertex_format = VK_FORMAT_R32_UINT;
    if (!BufferFormatAndFeaturesSupported(Gpu(), triangles_vertex_format,
                                          VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
        GTEST_SKIP()
            << "Hard coded vertex format has VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR, skipping test.";
    }

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesVertexFormat(triangles_vertex_format);

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-03797");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TrianglesMisalignedVertexStride) {
    TEST_DESCRIPTION(
        "Use a triangles vertex buffer stride that is not a multiple of the size in bytes of the smallest component of triangles "
        "vertex format");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetStride(1);

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03735");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TrianglesMisalignedVertexBufferAddress) {
    TEST_DESCRIPTION(
        "Use a triangles vertex buffer address that is not a multiple of the size in bytes of the smallest component of triangles "
        "vertex format");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesVertexBufferDeviceAddress(1);

    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");  // device address does not belong to a buffer
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03711");  // misaligned device address
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer);

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    blas.GetGeometries()[0].SetTrianglesMaxVertex(666);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03764");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, InvalidMaxVertex) {
    TEST_DESCRIPTION("Build a BLAS made out of triangles, without using indices, and using an invalid maxVertex");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Invalid update to triangles max vertex
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.GetGeometries()[0].SetTrianglesIndexType(VK_INDEX_TYPE_NONE_KHR);
    blas.GetGeometries()[0].SetTrianglesMaxVertex(1);

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildRangeInfoKHR-None-10775");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer);

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    blas.GetGeometries()[0].SetTrianglesIndexType(VkIndexType::VK_INDEX_TYPE_UINT16);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03765");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.GetGeometries()[0].SetTrianglesTransformatData(0);
    blas.BuildCmdBuffer(m_command_buffer);

    m_command_buffer.End();
    m_device->Wait();
    m_command_buffer.Begin();

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetScratchBuffer(std::make_shared<vkt::Buffer>());
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    blas.GetGeometries()[0].SetTrianglesTransformatData(666 * 16);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03766");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TrianglesVertexBufferNull) {
    TEST_DESCRIPTION(
        "Use a triangles vertex buffer specified referencing a null device address for an acceleration structure build operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesVertexBufferDeviceAddress(0);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03804");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TrianglesIndexBufferNull) {
    TEST_DESCRIPTION(
        "Use a triangles vertex buffer specified referencing a null device address for an acceleration structure build operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesIndexBufferDeviceAddress(0);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03806");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TrianglesIndexBufferInvalidAddress) {
    TEST_DESCRIPTION("Use a triangles vertex buffer specified referencing a device address that is referencing no existing buffer");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesIndexBufferDeviceAddress(32);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, AabbBufferInvalidAddress) {
    TEST_DESCRIPTION("Use a AABB buffer specified referencing a device address that referencing no existing buffer");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0] = vkt::as::blueprint::GeometrySimpleOnDeviceAABBInfo(*m_device);
    blas.GetGeometries()[0].SetAABBsDeviceAddress(32);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer transform_buffer(*m_device, 4096,
                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.GetGeometries()[0].SetTrianglesTransformatData(transform_buffer.Address());

    blas.BuildCmdBuffer(m_command_buffer);

    blas.SetSrcAS(blas.GetDstAS());
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    blas.GetGeometries()[0].SetTrianglesTransformatData(0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03767");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, BuildNullGeometries) {
    TEST_DESCRIPTION("Have a geometryCount > 0 but both pGeometries and ppGeometries be NULL");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();
    // Build Bottom Level Acceleration Structure
    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
    blas->SetNullGeometries(true);
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
    blas->GetSizeInfo();
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TransformBufferInvalidDeviceAddress) {
    TEST_DESCRIPTION(
        "Use a transform buffer specified with a device address that does not belong to an existing buffer for an acceleration "
        "structure build operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesTransformatData(16);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-size-11364");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TransformBufferInvalid) {
    TEST_DESCRIPTION("Use a transform buffer whose memory has been freed for an acceleration structure build operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer transform_buffer(*m_device, 4096,
                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetGeometries()[0].SetTrianglesTransformatData(transform_buffer.Address());

    m_command_buffer.Begin();
    blas.SetupBuild(*m_device, true);
    transform_buffer.Memory().Destroy();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    blas.SetMode(static_cast<VkBuildAccelerationStructureModeKHR>(0xbadbeef0));

    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-mode-04628");
    blas.BuildHost();
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

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    auto tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas.GetDstAS());

    m_command_buffer.Begin();
    tlas.SetupBuild(*m_device, true);

    tlas.GetGeometries()[0].SetInstancesDeviceAddress(0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03813");
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, InstanceBufferBadMemory) {
    TEST_DESCRIPTION("Use an instance buffer whose memory has been destroyed for an acceleration structure build operation.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    auto tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas.GetDstAS());

    m_command_buffer.Begin();
    tlas.SetupBuild(*m_device, true);

    tlas.GetGeometries()[0].GetInstance().buffer.Memory().Destroy();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
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
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, will fail alignment sometimes";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    pipeline.AddDynamicState(VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR);
    pipeline.Build();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-None-09458");
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, UpdatedFirstPrimitiveCount) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "VkAccelerationStructureBuildRangeInfoKHR::primitiveCount");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer);

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    m_command_buffer.Begin();

    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetSrcAS(blas.GetDstAS());

    // Create custom build ranges, with the default valid as a template, then somehow supply it?
    auto build_range_infos = blas.GetBuildRangeInfosFromGeometries();
    build_range_infos[0].primitiveCount = 0;
    blas.SetBuildRanges(build_range_infos);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-primitiveCount-03769");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, ScratchBufferBadAddressSpaceOpBuild) {
    TEST_DESCRIPTION("Use a scratch buffer that is too small for an acceleration structure build operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    auto size_info = blas.GetSizeInfo(*m_device);
    if (size_info.buildScratchSize <= 64) {
        GTEST_SKIP() << "Need a big scratch size, skipping test.";
    }

    // Allocate buffer memory separately so that it can be large enough. Scratch buffer size will be smaller.
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    VkBufferCreateInfo small_buffer_ci = vku::InitStructHelper();
    small_buffer_ci.size = 64;
    small_buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    auto scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, small_buffer_ci, vkt::no_mem);
    scratch_buffer->BindMemory(buffer_memory, 0);

    m_command_buffer.Begin();
    blas.SetScratchBuffer(scratch_buffer);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03671");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, ScratchBufferBadAddressSpaceOpUpdate) {
    TEST_DESCRIPTION("Use a scratch buffer that is too small for an acceleration structure update operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    auto size_info = blas.GetSizeInfo();
    if (size_info.updateScratchSize <= 64) {
        GTEST_SKIP() << "Update scratch size too small, skipping test.";
    }

    // Allocate buffer memory separately so that it can be large enough. Scratch buffer size will be smaller.
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    auto scratch_buffer =
        std::make_shared<vkt::Buffer>(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    m_command_buffer.Begin();
    blas.SetScratchBuffer(scratch_buffer);
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetSrcAS(blas.GetDstAS());
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03672");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, ScratchBufferBadMemory) {
    TEST_DESCRIPTION("Use a scratch buffer whose memory has been destroyed for an acceleration structure build operation");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
    scratch_buffer_ci.size = blas.GetSizeInfo().buildScratchSize;
    scratch_buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    auto scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, scratch_buffer_ci, vkt::no_mem);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = scratch_buffer->MemoryRequirements().size;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    scratch_buffer->BindMemory(buffer_memory, 0);

    m_command_buffer.Begin();
    blas.SetScratchBuffer(scratch_buffer);
    blas.SetupBuild(*m_device, true);

    buffer_memory.Destroy();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, DstAsTooSmall) {
    TEST_DESCRIPTION("Try to perform a build on an acceleration structure that was created too small.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetUpdateDstAccelStructSizeBeforeBuild(false);

    blas.GetDstAS()->SetSize(1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-10126");
    blas.BuildCmdBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, TooManyInstances) {
    TEST_DESCRIPTION(
        "Call vkGetAccelerationStructureBuildSizesKHR with pMaxPrimitiveCounts[0] > "
        "VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxInstanceCount");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    auto tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas.GetDstAS());
    tlas.GetGeometries()[0].SetPrimitiveCount(vvl::kU32Max);
    m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03785");
    tlas.GetSizeInfo();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, HostInstanceInvalid) {
    TEST_DESCRIPTION("build a TLAS with an invalid geometry.instances.data.hostAddress");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    // Build Bottom Level Acceleration Structure
    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device));
    blas->BuildHost();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostTopLevel(*m_device, blas);
    tlas.GetGeometries()[0].SetInstanceHostAccelStructRef(VK_NULL_HANDLE, 0);
    tlas.GetGeometries()[0].AddInstanceHostAccelStructRef(VK_NULL_HANDLE);
    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03779");
    m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03779");
    tlas.BuildHost();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, HostAccelerationStructureBuildNullPointers) {
    TEST_DESCRIPTION("Test host side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
        blas.SetEnableScratchBuild(false);
        blas.SetHostScratchBuffer(nullptr);
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03725");
        blas.BuildHost();
        m_errorMonitor->VerifyFound();
    }

    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
        blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
        blas.SetEnableScratchBuild(false);
        blas.SetHostScratchBuffer(nullptr);
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-04630");  // Null src accel struct
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03726");
        blas.BuildHost();
        m_errorMonitor->VerifyFound();
    }

    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
        blas.GetGeometries()[0].SetTrianglesHostVertexBuffer(nullptr, 1);
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03771");
        blas.BuildHost();
        m_errorMonitor->VerifyFound();
    }

    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
        blas.GetGeometries()[0].SetTrianglesHostIndexBuffer(nullptr);
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03772");
        blas.BuildHost();
        m_errorMonitor->VerifyFound();
    }

    {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        blas.GetGeometries()[0].SetAABBsHostBuffer(nullptr);
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03774");
        blas.BuildHost();
        m_errorMonitor->VerifyFound();
    }

    {
        auto blas = std::make_shared<vkt::as::BuildGeometryInfoKHR>(
            vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device));
        blas->BuildHost();

        vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostTopLevel(*m_device, blas);
        tlas.GetGeometries()[0].SetInstanceHostAddress(nullptr);
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03778");
        tlas.BuildHost();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, HostBuildOverlappingScratchBuffers) {
    TEST_DESCRIPTION("Attempt an host acceleration structure build with overlapping scratch buffers");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    constexpr size_t blas_count = 3;

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
    auto scratch_data = std::make_shared<std::vector<uint8_t>>(1u << 15u, uint8_t(0));
    for (size_t i = 0; i < blas_count; ++i) {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
        blas.SetHostScratchBuffer(scratch_data);
        blas.SetEnableScratchBuild(false);
        blas_vec.emplace_back(std::move(blas));
    }

    for (size_t i = 0; i < binom<size_t>(blas_count, 2); ++i) {
        m_errorMonitor->SetDesiredError("VUID-vkBuildAccelerationStructuresKHR-scratchData-03704");
    }
    vkt::as::BuildHostAccelerationStructuresKHR(*m_device, blas_vec);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, OpacityMicromapFeatureDisable) {
    TEST_DESCRIPTION("Micromap feature is disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCreateMicromapEXT-micromap-07430");

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);

    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT maCreateInfo = vku::InitStructHelper();

    maCreateInfo.createFlags = 0;
    maCreateInfo.buffer = buffer;
    maCreateInfo.offset = 0;
    maCreateInfo.size = 0;
    maCreateInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    maCreateInfo.deviceAddress = 0ull;

    vk::CreateMicromapEXT(device(), &maCreateInfo, nullptr, &micromap);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, OpacityMicromapCaptureReplayFeatureDisable) {
    TEST_DESCRIPTION("Micromap capture replay feature is disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::micromap);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCreateMicromapEXT-deviceAddress-07431");

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);

    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT maCreateInfo = vku::InitStructHelper();

    maCreateInfo.createFlags = 0;
    maCreateInfo.buffer = buffer;
    maCreateInfo.offset = 0;
    maCreateInfo.size = 0;
    maCreateInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    maCreateInfo.deviceAddress = 0x100000ull;

    vk::CreateMicromapEXT(device(), &maCreateInfo, nullptr, &micromap);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, AccelerationStructureGeometry) {
    TEST_DESCRIPTION("Test VkAccelerationStructureGeometryKHR parameters");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.flags =
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    uint32_t max_primitives_count = 0u;
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-03796");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
    m_errorMonitor->VerifyFound();

    build_info.flags = 0u;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03790");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
    m_errorMonitor->VerifyFound();

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometry.triangles = vku::InitStructHelper();

    build_info.geometryCount = 1u;
    build_info.pGeometries = &geometry;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
    m_errorMonitor->VerifyFound();

    geometry.geometry.instances = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
    m_errorMonitor->VerifyFound();

    VkAccelerationStructureGeometryKHR geometries[2];
    geometries[0] = vku::InitStructHelper();
    geometries[0].geometry.triangles = vku::InitStructHelper();
    geometries[0].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometries[1] = vku::InitStructHelper();
    geometries[1].geometry.aabbs = vku::InitStructHelper();
    geometries[1].geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
    build_info.geometryCount = 2u;
    build_info.pGeometries = geometries;
    uint32_t max_primitives_counts[2] = {0u, 0u};
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              max_primitives_counts, &build_sizes_info);
    m_errorMonitor->VerifyFound();

    build_info.geometryCount = 1u;
    build_info.pGeometries = &geometry;
    geometry.geometryType = VK_GEOMETRY_TYPE_MAX_ENUM_KHR;  // Invalid
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CopyAccelerationStructureMode) {
    TEST_DESCRIPTION("Test VkAccelerationStructureGeometryKHR parameters");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas1 = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    blas1.BuildHost();

    auto blas2 = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(*m_device, 4096);
    blas2->Create();

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    copy_info.src = blas1.GetDstAS()->handle();
    copy_info.dst = blas2->handle();

    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-mode-03410");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureInfoKHR-src-03411");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, StridedDeviceAddressRegion) {
    TEST_DESCRIPTION("Invalid ray gen SBT address");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipeline(*this, m_device);

    rt_pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);

    rt_pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    rt_pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    rt_pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipeline.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipeline.Build();

    vkt::rt::TraceRaysSbt sbt = rt_pipeline.GetTraceRaysSbt();

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline);

    m_errorMonitor->SetDesiredError("UNASSIGNED-TraceRays-InvalidRayGenSBTAddress");
    sbt.ray_gen_sbt.deviceAddress = 0;
    vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100, 100, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, InvalidAsCopy) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    m_command_buffer.Begin();

    blas.BuildCmdBuffer(m_command_buffer);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    vkt::Buffer serialized_accel_struct_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vkt::device_address);

    VkCopyAccelerationStructureToMemoryInfoKHR copy_accel_struct_to_memory_info = vku::InitStructHelper();
    copy_accel_struct_to_memory_info.src = blas.GetDstAS()->handle();
    copy_accel_struct_to_memory_info.dst.deviceAddress = Align<VkDeviceAddress>(serialized_accel_struct_buffer.Address(), 256u);
    copy_accel_struct_to_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412");
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_accel_struct_to_memory_info);
    m_errorMonitor->VerifyFound();

    copy_accel_struct_to_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    copy_accel_struct_to_memory_info.dst.deviceAddress += 128;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03740");
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_accel_struct_to_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdTraceRays) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipeline(*this, m_device);

    rt_pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);

    rt_pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    rt_pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    rt_pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipeline.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipeline.Build();

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
    vk::GetPhysicalDeviceProperties2(gpu_, &props2);

    const uint32_t handle_size_base_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupBaseAlignment);
    const uint32_t handle_size_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);

    const auto sbt = rt_pipeline.GetTraceRaysSbt();

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline);

    {
        VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
        sbt_buffer_info.size = handle_size_aligned;
        sbt_buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer invalid_buffer(*m_device, sbt_buffer_info, kHostVisibleMemProps, &alloc_flags);

        VkDeviceAddress invalid_buffer_address = invalid_buffer.Address();

        VkStridedDeviceAddressRegionKHR invalid_closest_hit_sbt{};
        invalid_buffer_address = Align<VkDeviceAddress>(invalid_buffer_address, rt_pipeline_props.shaderGroupBaseAlignment);
        invalid_closest_hit_sbt.deviceAddress = invalid_buffer_address;
        invalid_closest_hit_sbt.stride = handle_size_aligned;
        invalid_closest_hit_sbt.size = handle_size_aligned;

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03688");
        vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &sbt.miss_sbt, &invalid_closest_hit_sbt, &sbt.callable_sbt, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
        sbt_buffer_info.size = handle_size_base_aligned;
        sbt_buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer invalid_buffer(*m_device, sbt_buffer_info, kHostVisibleMemProps, &alloc_flags);

        VkDeviceAddress invalid_buffer_address = invalid_buffer.Address();

        VkStridedDeviceAddressRegionKHR invalid_ray_gen_sbt{};
        invalid_buffer_address = Align<VkDeviceAddress>(invalid_buffer_address, rt_pipeline_props.shaderGroupBaseAlignment);
        invalid_ray_gen_sbt.deviceAddress = invalid_buffer_address;
        invalid_ray_gen_sbt.stride = handle_size_base_aligned;
        invalid_ray_gen_sbt.size = handle_size_base_aligned;

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681");
        vk::CmdTraceRaysKHR(m_command_buffer, &invalid_ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
        sbt_buffer_info.size = handle_size_aligned;
        sbt_buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer invalid_buffer(*m_device, sbt_buffer_info, kHostVisibleMemProps, &alloc_flags);

        VkDeviceAddress invalid_buffer_address = invalid_buffer.Address();

        VkStridedDeviceAddressRegionKHR invalid_miss_sbt{};
        invalid_buffer_address = Align<VkDeviceAddress>(invalid_buffer_address, rt_pipeline_props.shaderGroupBaseAlignment);
        invalid_miss_sbt.deviceAddress = invalid_buffer_address;
        invalid_miss_sbt.stride = handle_size_aligned;
        invalid_miss_sbt.size = handle_size_aligned;

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03684");
        vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &invalid_miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
        sbt_buffer_info.size = handle_size_aligned;
        sbt_buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer invalid_buffer(*m_device, sbt_buffer_info, kHostVisibleMemProps, &alloc_flags);

        VkDeviceAddress invalid_buffer_address = invalid_buffer.Address();

        VkStridedDeviceAddressRegionKHR invalid_callable_sbt{};
        invalid_buffer_address = Align<VkDeviceAddress>(invalid_buffer_address, rt_pipeline_props.shaderGroupBaseAlignment);
        invalid_callable_sbt.deviceAddress = invalid_buffer_address;
        invalid_callable_sbt.stride = handle_size_aligned;
        invalid_callable_sbt.size = handle_size_aligned;

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03692");
        vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &invalid_callable_sbt, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
        sbt_buffer_info.size = handle_size_aligned;
        sbt_buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer invalid_buffer(*m_device, sbt_buffer_info, kHostVisibleMemProps, &alloc_flags);

        VkDeviceAddress invalid_buffer_address = invalid_buffer.Address();

        VkStridedDeviceAddressRegionKHR invalid_callable_sbt{};
        invalid_buffer_address = Align<VkDeviceAddress>(invalid_buffer_address, rt_pipeline_props.shaderGroupBaseAlignment) + 1;
        invalid_callable_sbt.deviceAddress = invalid_buffer_address;
        invalid_callable_sbt.stride = handle_size_aligned;
        invalid_callable_sbt.size = handle_size_aligned;

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03693");
        vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &invalid_callable_sbt, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
        sbt_buffer_info.size = handle_size_base_aligned;
        sbt_buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vkt::Buffer invalid_buffer(*m_device, sbt_buffer_info, kHostVisibleMemProps, &alloc_flags);

        VkDeviceAddress invalid_buffer_address = invalid_buffer.Address();

        VkStridedDeviceAddressRegionKHR invalid_ray_gen_sbt{};
        invalid_buffer_address = Align<VkDeviceAddress>(invalid_buffer_address, rt_pipeline_props.shaderGroupBaseAlignment);
        invalid_ray_gen_sbt.deviceAddress = invalid_buffer_address;
        invalid_ray_gen_sbt.stride = handle_size_base_aligned / 2u;
        invalid_ray_gen_sbt.size = handle_size_base_aligned;

        m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysKHR-size-04023");
        vk::CmdTraceRaysKHR(m_command_buffer, &invalid_ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100, 100, 1);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100, 100, 1);

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    m_device->Wait();
}

TEST_F(NegativeRayTracing, GetAccelerationStructureBuildSizesNullMaxPrimitiveCount) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureGeometryKHR geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles = vku::InitStructHelper();
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry.geometry.triangles.vertexStride = sizeof(float) * 3u;
    geometry.geometry.triangles.maxVertex = 3u;
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
    geometry.geometry.triangles.indexData.deviceAddress = 0;
    geometry.geometry.triangles.transformData.deviceAddress = 0;
    geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = VK_NULL_HANDLE;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    build_info.ppGeometries = NULL;
    build_info.scratchData.deviceAddress = 0;
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03619");
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              nullptr, &build_sizes_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildPartitionedAccelerationStructureInfo) {
    TEST_DESCRIPTION("Validate all the valid usages of VkBuildPartitionedAccelerationStructureInfo");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::partitionedAccelerationStructure);
    AddRequiredExtensions(VK_NV_PARTITIONED_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());
    // MockICD will cause the test to fail, skip it
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    int instance_count = 20;
    int partition_count = 5;

    VkPartitionedAccelerationStructureInstancesInputNV input_info = vku::InitStructHelper();
    input_info.instanceCount = instance_count;
    input_info.maxInstancePerPartitionCount = instance_count / partition_count;
    input_info.partitionCount = partition_count;
    input_info.maxInstanceInGlobalPartitionCount = instance_count / partition_count;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;

    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
    vk::GetPartitionedAccelerationStructuresBuildSizesNV(m_device->handle(), &input_info, &size_info);
    // memory dstAccelerationStructureData must be equal or larger than the
    // VkAccelerationStructureBuildSizesInfoKHR::accelerationStructureSize minus 1500 to cause memeory difference
    vkt::Buffer build_buffer(
        *m_device, size_info.accelerationStructureSize - 1500,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);
    VkDeviceAddress ptlas_buffer_address = build_buffer.Address();

    vkt::Buffer correct_build_buffer(*m_device, size_info.accelerationStructureSize,
                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                         VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                                     vkt::device_address);
    VkDeviceAddress correct_ptlas_buffer_address = correct_build_buffer.Address();
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t),
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                             vkt::device_address);

    int input = instance_count;
    auto *data = static_cast<uint32_t *>(count_buffer.Memory().Map());
    memcpy(data, &input, sizeof(input));
    VkDeviceAddress count_buffer_address = count_buffer.Address();

    // Create correct Buffer for scratchData
    vkt::Buffer scratch_buffer(*m_device, size_info.buildScratchSize,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);
    VkDeviceAddress scratch_buffer_address = scratch_buffer.Address();

    // VkAccelerationStructureBuildSizesInfoKHR::accelerationStructureSize minus 1500 to cause memeory difference
    vkt::Buffer wrong_scratch_buffer(
        *m_device, size_info.buildScratchSize - 1500,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);
    VkDeviceAddress wrong_scratch_buffer_address = wrong_scratch_buffer.Address();

    VkBuildPartitionedAccelerationStructureInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.srcAccelerationStructureData = 0;
    command_info.dstAccelerationStructureData = 0;
    command_info.scratchData = scratch_buffer_address;
    command_info.srcInfos = 0;
    command_info.srcInfosCount = count_buffer_address;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-dstAccelerationStructureData-10561");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstAccelerationStructureData = ptlas_buffer_address;
    m_command_buffer.End();

    // add 1 to cause aligned error
    command_info.srcInfosCount = count_buffer_address + 1;
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-srcInfosCount-10563");
    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-dstAccelerationStructureData-10562");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10552");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10543");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    command_info.srcInfosCount = count_buffer_address;
    command_info.dstAccelerationStructureData = correct_ptlas_buffer_address;
    command_info.scratchData = 0;
    m_command_buffer.Begin();
    // scratchData must not be NULL
    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-scratchData-10558");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    command_info.scratchData = wrong_scratch_buffer_address;
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-scratchData-10559");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10550");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10541");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    command_info.scratchData = scratch_buffer_address + 1;
    command_info.dstAccelerationStructureData = correct_ptlas_buffer_address + 1;
    command_info.srcAccelerationStructureData = correct_ptlas_buffer_address + 1;
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10542");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10544");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10545");
    // Add this VU to avoid dst and src addres overlap warning
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10549");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    command_info.scratchData = scratch_buffer_address;
    command_info.dstAccelerationStructureData = scratch_buffer_address;
    command_info.srcAccelerationStructureData = scratch_buffer_address;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10549");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10548");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildPartitionedAccelerationStructuresNV-pBuildInfo-10547");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, BuildPartitionedAccelerationStrutureInfoBadMemory) {
    TEST_DESCRIPTION(
        "Validate all buffer is a non-sparse buffer then it must be bound completely and contiguously to a single VkDeviceMemory "
        "object");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::partitionedAccelerationStructure);
    AddRequiredExtensions(VK_NV_PARTITIONED_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t instance_count = 20;
    uint32_t partition_count = 5;

    VkPartitionedAccelerationStructureFlagsNV ptlas_flags = vku::InitStructHelper();
    ptlas_flags.enablePartitionTranslation = true;
    VkPartitionedAccelerationStructureInstancesInputNV input_info = vku::InitStructHelper(&ptlas_flags);
    input_info.instanceCount = instance_count;
    input_info.maxInstancePerPartitionCount = instance_count / partition_count;
    input_info.partitionCount = partition_count;
    input_info.maxInstanceInGlobalPartitionCount = instance_count / partition_count;

    VkAccelerationStructureBuildSizesInfoKHR ptlas_size_info = vku::InitStructHelper();
    vk::GetPartitionedAccelerationStructuresBuildSizesNV(*m_device, &input_info, &ptlas_size_info);

    vkt::Buffer build_buffer(*m_device, ptlas_size_info.accelerationStructureSize,
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                             vkt::device_address);
    VkDeviceAddress ptlas_buffer_address = build_buffer.Address();

    vkt::Buffer dst_build_buffer(*m_device, ptlas_size_info.accelerationStructureSize,
                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                 vkt::device_address);
    VkDeviceAddress dst_ptlas_buffer_address = dst_build_buffer.Address();

    vkt::Buffer scratch_buffer(*m_device, ptlas_size_info.buildScratchSize,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);
    VkDeviceAddress scratch_buffer_address = scratch_buffer.Address();

    vkt::Buffer write_partition_buffer(
        *m_device, partition_count * sizeof(VkPartitionedAccelerationStructureWritePartitionTranslationDataNV),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);
    std::vector<VkPartitionedAccelerationStructureWritePartitionTranslationDataNV> writePartitionArgs;
    static uint32_t partitionArray[5] = {3, 0xFFFFFFFF, 0, 2, 1};
    static float partitionTranslationY[] = {0, 20, 40, 20, 0};
    for (uint32_t t = 0; t < partition_count; t++) {
        VkPartitionedAccelerationStructureWritePartitionTranslationDataNV writePartition{};
        writePartition.partitionIndex = partitionArray[t];
        writePartition.partitionTranslation[0] = 0;
        writePartition.partitionTranslation[1] = partitionTranslationY[t];
        writePartition.partitionTranslation[2] = 0;
        writePartitionArgs.push_back(writePartition);
    }

    auto *write_partition_data =
        static_cast<VkPartitionedAccelerationStructureWritePartitionTranslationDataNV *>(write_partition_buffer.Memory().Map());
    memcpy(write_partition_data, writePartitionArgs.data(),
           partition_count * sizeof(VkPartitionedAccelerationStructureWritePartitionTranslationDataNV));
    VkDeviceAddress write_partition_buffer_address = write_partition_buffer.Address();

    std::vector<VkBuildPartitionedAccelerationStructureIndirectCommandNV> ptlas_ops;
    VkBuildPartitionedAccelerationStructureIndirectCommandNV ptlas_op = {};
    ptlas_op.opType = VK_PARTITIONED_ACCELERATION_STRUCTURE_OP_TYPE_WRITE_PARTITION_TRANSLATION_NV;
    ptlas_op.argCount = partition_count;
    ptlas_op.argData.startAddress = write_partition_buffer_address;
    ptlas_op.argData.strideInBytes = sizeof(VkPartitionedAccelerationStructureWriteInstanceDataNV);
    ptlas_ops.push_back(ptlas_op);

    vkt::Buffer src_info_buffer(*m_device, partition_count * sizeof(VkBuildPartitionedAccelerationStructureIndirectCommandNV),
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                vkt::device_address);
    auto *src_info_data = static_cast<VkBuildPartitionedAccelerationStructureIndirectCommandNV *>(src_info_buffer.Memory().Map());
    memcpy(src_info_data, ptlas_ops.data(), sizeof(VkBuildPartitionedAccelerationStructureIndirectCommandNV));
    VkDeviceAddress src_info_buffer_address = src_info_buffer.Address();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t),
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                             vkt::device_address);
    VkDeviceAddress count_buffer_address = count_buffer.Address();

    // Manually destroy memory to test memory validation
    build_buffer.Memory().Destroy();
    dst_build_buffer.Memory().Destroy();
    scratch_buffer.Memory().Destroy();
    src_info_buffer.Memory().Destroy();
    count_buffer.Memory().Destroy();

    VkBuildPartitionedAccelerationStructureInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.srcAccelerationStructureData = ptlas_buffer_address;
    command_info.dstAccelerationStructureData = dst_ptlas_buffer_address;
    command_info.scratchData = scratch_buffer_address;
    command_info.srcInfos = src_info_buffer_address;
    command_info.srcInfosCount = count_buffer_address;

    m_command_buffer.Begin();
    // For checking BuildPartitionedAccelerationStructureInfoNV memory bound
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    // For checking CmdBuildPartitionedAccelerationStructuresNV memory bound
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, CmdBuildPartitionedAccelerationStructures) {
    TEST_DESCRIPTION("Validate vkCmdBuildPartitionedAccelerationStructuresNV need partitionedAccelerationStructure feature");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredExtensions(VK_NV_PARTITIONED_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    int instance_count = 20;
    int partition_count = 5;

    VkPartitionedAccelerationStructureInstancesInputNV input_info = vku::InitStructHelper();
    input_info.instanceCount = instance_count;
    input_info.maxInstancePerPartitionCount = instance_count / partition_count;
    input_info.partitionCount = partition_count;
    input_info.maxInstanceInGlobalPartitionCount = instance_count / partition_count;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;

    // constant size here is irrelevant, this test just check the feature enabled
    // Set a size smaller than VkAccelerationStructureBuildSizesInfoKHR::buildScratchSize to trigger the VU
    vkt::Buffer build_buffer(*m_device, 0x4,
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                             vkt::device_address);
    VkDeviceAddress ptlas_buffer_address = build_buffer.Address();

    vkt::Buffer count_buffer(
        *m_device, sizeof(uint32_t),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        vkt::device_address);
    // store the value into count buffer
    int input = instance_count;
    auto *data = static_cast<uint32_t *>(count_buffer.Memory().Map());
    memcpy(data, &input, sizeof(input));
    VkDeviceAddress count_buffer_address = count_buffer.Address();
    // constant size here is irrelevant, this test just check the feature enabled
    // Set a size smaller than VkAccelerationStructureBuildSizesInfoKHR:: to trigger the VU
    vkt::Buffer scratch_buffer(*m_device, 0x4,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);
    VkDeviceAddress scratch_buffer_address = scratch_buffer.Address();
    VkBuildPartitionedAccelerationStructureInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.srcAccelerationStructureData = 0;
    command_info.dstAccelerationStructureData = ptlas_buffer_address;
    command_info.scratchData = scratch_buffer_address;
    command_info.srcInfos = 0;
    command_info.srcInfosCount = count_buffer_address;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-srcAccelerationStructureData-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkBuildPartitionedAccelerationStructureInfoNV-srcInfos-parameter");
    vk::CmdBuildPartitionedAccelerationStructuresNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, PartitionedAccelerationStructuresBuildSizes) {
    TEST_DESCRIPTION(
        "Validate PartitionedAccelerationStructuresBuildSizes"
        "need to have partitionedAccelerationStructure feature enable "
        "and have valid VkPartitionedAccelerationStructureInstancesInputNV");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredExtensions(VK_NV_PARTITIONED_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    int instance_count = 20;
    int partition_count = 5;

    VkPartitionedAccelerationStructureInstancesInputNV input_info = vku::InitStructHelper();
    input_info.instanceCount = instance_count;
    input_info.maxInstancePerPartitionCount = instance_count / partition_count;
    input_info.partitionCount = partition_count;
    input_info.maxInstanceInGlobalPartitionCount = instance_count / partition_count;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    VkAccelerationStructureBuildSizesInfoKHR ptlas_size_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError(
        "VUID-vkGetPartitionedAccelerationStructuresBuildSizesNV-partitionedAccelerationStructure-10534");
    vk::GetPartitionedAccelerationStructuresBuildSizesNV(*m_device, &input_info, &ptlas_size_info);
    m_errorMonitor->VerifyFound();

    VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV partitioned_accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(partitioned_accel_struct_props);
    input_info.partitionCount = partitioned_accel_struct_props.maxPartitionCount;
    m_errorMonitor->SetDesiredError(
        "VUID-vkGetPartitionedAccelerationStructuresBuildSizesNV-partitionedAccelerationStructure-10534");
    m_errorMonitor->SetDesiredError("VUID-VkPartitionedAccelerationStructureInstancesInputNV-partitionCount-10535");
    vk::GetPartitionedAccelerationStructuresBuildSizesNV(*m_device, &input_info, &ptlas_size_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdBuildClusterAccelerationStructureIndirect) {
    TEST_DESCRIPTION("Validate vkCmdBuildClusterAccelerationStructureIndirect need clusterAccelerationStructures feature");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t total_triangles = 1;
    uint32_t total_vertices = 3 * total_triangles;
    VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    tri_cluster.maxGeometryIndexValue = total_triangles - 1;

    tri_cluster.maxClusterUniqueGeometryCount = 0;
    tri_cluster.maxClusterTriangleCount = total_triangles;
    tri_cluster.maxClusterVertexCount = total_vertices;

    tri_cluster.maxTotalTriangleCount = total_triangles;
    tri_cluster.maxTotalVertexCount = total_vertices;
    tri_cluster.minPositionTruncateBitCount = 0;

    VkClusterAccelerationStructureOpInputNV input = {};
    input.pTriangleClusters = &tri_cluster;
    VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
    input_info.maxAccelerationStructureCount = 1;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;

    // Create scratch_buffer constant size here is irrelevant, this test just checks the feature enabled
    vkt::Buffer scratch_buffer(*m_device, 0x1024,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);
    vkt::Buffer src_info_buffer(
        *m_device, sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    uint32_t input_num = 1;
    vkt::Buffer src_count_buffer(
        *m_device, sizeof(uint32_t),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);
    auto *count_data = static_cast<uint32_t *>(src_count_buffer.Memory().Map());
    memcpy(count_data, &input_num, sizeof(input_num));

    VkClusterAccelerationStructureCommandsInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.scratchData = scratch_buffer.Address();
    command_info.srcInfosArray.deviceAddress = src_info_buffer.Address();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
    command_info.srcInfosCount = src_count_buffer.Address();

    vkt::Buffer implicit_build_buffer(*m_device, 0x1024,
                                      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                          VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                                      vkt::device_address);

    vkt::Buffer dst_buffer(*m_device, 1 * sizeof(VkStridedDeviceAddressNV),
                           VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                           vkt::device_address);
    command_info.input.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV;
    command_info.dstImplicitData = implicit_build_buffer.Address();
    command_info.dstAddressesArray.deviceAddress = dst_buffer.Address();
    command_info.dstAddressesArray.stride = sizeof(VkDeviceAddress);
    command_info.dstSizesArray.stride = sizeof(uint32_t);
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-clusterAccelerationStructure-10443");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdBuildClusterAccelerationStructureIndirectBadMemory) {
    TEST_DESCRIPTION("Validate vkCmdBuildClusterAccelerationStructureIndirectNV can not build with bad memory buffer");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t total_triangles = 1;
    uint32_t totalVertices = 3 * total_triangles;

    VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    tri_cluster.maxGeometryIndexValue = total_triangles - 1;
    tri_cluster.maxClusterUniqueGeometryCount = 0;
    tri_cluster.maxClusterTriangleCount = 1;
    tri_cluster.maxClusterVertexCount = 3;
    tri_cluster.maxTotalTriangleCount = total_triangles;
    tri_cluster.maxTotalVertexCount = totalVertices;
    tri_cluster.minPositionTruncateBitCount = 0;

    VkClusterAccelerationStructureOpInputNV input = {};
    input.pTriangleClusters = &tri_cluster;
    VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
    input_info.maxAccelerationStructureCount = 1;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;

    VkAccelerationStructureBuildSizesInfoKHR clas_size_info = vku::InitStructHelper();
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);

    vkt::Buffer scratch_buffer(*m_device, clas_size_info.buildScratchSize,
                               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    uint32_t index_data_f[10];
    for (uint32_t i = 0; i < 10; i++) {
        index_data_f[i] = i;
    }
    vkt::Buffer index_buffer(
        *m_device, 10 * sizeof(uint32_t),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);
    auto *index_data = static_cast<uint32_t *>(index_buffer.Memory().Map());
    memcpy(index_data, index_data_f, sizeof(index_data_f));

    vkt::Buffer src_info_buffer(
        *m_device, sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    const uint32_t numVerticesInCluster = totalVertices;
    VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV triClusterTemplateArg = {};
    triClusterTemplateArg.clusterID = 0;
    triClusterTemplateArg.clusterFlags = VK_CLUSTER_ACCELERATION_STRUCTURE_CLUSTER_ALLOW_DISABLE_OPACITY_MICROMAPS_NV;
    triClusterTemplateArg.triangleCount = total_triangles;
    triClusterTemplateArg.vertexCount = numVerticesInCluster;
    triClusterTemplateArg.positionTruncateBitCount = 0;
    triClusterTemplateArg.indexType = VK_CLUSTER_ACCELERATION_STRUCTURE_INDEX_FORMAT_32BIT_NV;
    triClusterTemplateArg.opacityMicromapIndexType = VK_CLUSTER_ACCELERATION_STRUCTURE_INDEX_FORMAT_32BIT_NV;
    triClusterTemplateArg.baseGeometryIndexAndGeometryFlags.geometryFlags =
        VK_CLUSTER_ACCELERATION_STRUCTURE_GEOMETRY_OPAQUE_BIT_NV;
    triClusterTemplateArg.indexBufferStride = sizeof(uint32_t);
    triClusterTemplateArg.vertexBufferStride = 3 * sizeof(float);
    triClusterTemplateArg.geometryIndexAndFlagsBufferStride = 0;
    triClusterTemplateArg.opacityMicromapIndexBufferStride = 0;
    triClusterTemplateArg.indexBuffer = index_buffer.Address();
    triClusterTemplateArg.vertexBuffer = 0;
    triClusterTemplateArg.geometryIndexAndFlagsBuffer = 0;
    triClusterTemplateArg.opacityMicromapArray = 0;
    triClusterTemplateArg.opacityMicromapIndexBuffer = 0;
    triClusterTemplateArg.instantiationBoundingBoxLimit = 0;

    auto *src_info_data =
        static_cast<VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV *>(src_info_buffer.Memory().Map());
    memcpy(src_info_data, &triClusterTemplateArg, sizeof(triClusterTemplateArg));

    vkt::Buffer src_count_buffer(
        *m_device, sizeof(uint32_t),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);
    uint32_t input_num = 1;
    auto *count_data = static_cast<uint32_t *>(src_count_buffer.Memory().Map());
    memcpy(count_data, &input_num, sizeof(input_num));

    VkClusterAccelerationStructureCommandsInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.scratchData = scratch_buffer.Address();
    command_info.srcInfosArray.deviceAddress = src_info_buffer.Address();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
    command_info.srcInfosCount = src_count_buffer.Address();

    vkt::Buffer implicit_build_buffer(
        *m_device, clas_size_info.accelerationStructureSize,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);

    vkt::Buffer dstBuffer(*m_device, 1 * sizeof(VkStridedDeviceAddressNV),
                          VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                          vkt::device_address);

    command_info.input.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV;
    command_info.dstImplicitData = implicit_build_buffer.Address();
    command_info.dstAddressesArray.deviceAddress = dstBuffer.Address();
    command_info.dstAddressesArray.stride = sizeof(VkDeviceAddress);
    command_info.dstSizesArray.deviceAddress = dstBuffer.Address();
    command_info.dstSizesArray.stride = sizeof(uint32_t);

    scratch_buffer.Memory().Destroy();
    src_info_buffer.Memory().Destroy();
    src_count_buffer.Memory().Destroy();
    implicit_build_buffer.Memory().Destroy();
    dstBuffer.Memory().Destroy();

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, GetClusterAccelerationStructureBuildSizes) {
    TEST_DESCRIPTION("Validate all valid usages of vkGetClusterAccelerationStructureBuildSizesNV");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t total_triangles = 10;
    uint32_t max_triangle_per_cluster = 10;
    uint32_t total_vertices = 3 * total_triangles;

    VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    tri_cluster.maxGeometryIndexValue = total_triangles - 1;

    tri_cluster.maxClusterUniqueGeometryCount = max_triangle_per_cluster - 1;
    tri_cluster.maxClusterTriangleCount = max_triangle_per_cluster;
    tri_cluster.maxClusterVertexCount = max_triangle_per_cluster * 3;

    tri_cluster.maxTotalTriangleCount = total_triangles;
    tri_cluster.maxTotalVertexCount = total_vertices;
    tri_cluster.minPositionTruncateBitCount = 0;

    VkClusterAccelerationStructureOpInputNV input = {};
    input.pTriangleClusters = &tri_cluster;

    VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
    input_info.maxAccelerationStructureCount = 1;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;

    VkAccelerationStructureBuildSizesInfoKHR clas_size_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetClusterAccelerationStructureBuildSizesNV-clusterAccelerationStructure-10438");
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, ClusterAccelerationStructureCommandsInfo) {
    TEST_DESCRIPTION("Validate ClusterAccelerationStructureCommandsInfo all valid usages");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t total_triangles = 1;
    uint32_t total_vertices = 3 * total_triangles;

    VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    tri_cluster.maxGeometryIndexValue = total_triangles - 1;

    tri_cluster.maxClusterUniqueGeometryCount = 0;
    tri_cluster.maxClusterTriangleCount = 1;
    tri_cluster.maxClusterVertexCount = 3;
    tri_cluster.maxTotalTriangleCount = total_triangles;
    tri_cluster.maxTotalVertexCount = total_vertices;
    tri_cluster.minPositionTruncateBitCount = 0;

    VkClusterAccelerationStructureOpInputNV input = {};
    input.pTriangleClusters = &tri_cluster;
    VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
    input_info.maxAccelerationStructureCount = 1;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;

    VkAccelerationStructureBuildSizesInfoKHR clas_size_info = vku::InitStructHelper();
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);

    vkt::Buffer scratch_buffer(*m_device, clas_size_info.buildScratchSize,
                               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                               vkt::device_address);

    vkt::Buffer src_info_buffer(
        *m_device, sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    uint32_t input_num = 1;
    vkt::Buffer src_count_buffer(
        *m_device, sizeof(uint32_t),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);
    auto *count_data = static_cast<uint32_t *>(src_count_buffer.Memory().Map());
    memcpy(count_data, &input_num, sizeof(input_num));

    VkClusterAccelerationStructureCommandsInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.scratchData = scratch_buffer.Address();
    command_info.srcInfosArray.deviceAddress = src_info_buffer.Address();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
    command_info.srcInfosCount = src_count_buffer.Address();

    vkt::Buffer implicit_build_buffer(
        *m_device, clas_size_info.accelerationStructureSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    vkt::Buffer dst_buffer(*m_device, 1 * sizeof(VkStridedDeviceAddressNV),
                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                           vkt::device_address);

    vkt::Buffer countBuffer(
        *m_device, sizeof(uint32_t),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    command_info.input.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV;
    command_info.dstImplicitData = implicit_build_buffer.Address();
    command_info.dstAddressesArray.deviceAddress = dst_buffer.Address();
    command_info.dstAddressesArray.stride = sizeof(VkDeviceAddress);
    command_info.dstSizesArray.stride = sizeof(uint32_t);

    m_command_buffer.Begin();

    command_info.dstImplicitData = 0;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-dstImplicitData-parameter");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstImplicitData = implicit_build_buffer.Address();
    m_command_buffer.End();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-10470");
    command_info.input.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_COMPUTE_SIZES_NV;
    command_info.dstSizesArray.deviceAddress = 0;
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_command_buffer.Begin();
    command_info.input.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV;
    command_info.dstSizesArray.deviceAddress = countBuffer.Address();
    command_info.dstAddressesArray.stride = 7;
    command_info.dstSizesArray.stride = 3;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-dstAddressesArray-10474");
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-dstSizesArray-10475");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstAddressesArray.stride = sizeof(VkDeviceAddress);
    command_info.dstSizesArray.stride = 4;
    m_command_buffer.End();

    VkPhysicalDeviceClusterAccelerationStructurePropertiesNV accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);
    m_command_buffer.Begin();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV) - 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-srcInfosArray-10476");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
    m_command_buffer.End();

    m_command_buffer.Begin();
    command_info.dstImplicitData = implicit_build_buffer.Address() / accel_struct_props.clusterTemplateByteAlignment *
                                       accel_struct_props.clusterTemplateByteAlignment +
                                   1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-input-10478");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstImplicitData = implicit_build_buffer.Address();
    m_command_buffer.End();

    m_command_buffer.Begin();
    command_info.scratchData = (scratch_buffer.Address() / accel_struct_props.clusterScratchByteAlignment) *
                                   accel_struct_props.clusterScratchByteAlignment +
                               1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-scratchData-10480");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-scratchData-10447");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.scratchData = scratch_buffer.Address();
    m_command_buffer.End();

    m_command_buffer.Begin();
    command_info.dstImplicitData = scratch_buffer.Address();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-dstImplicitData-10456");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstImplicitData = implicit_build_buffer.Address();
    m_command_buffer.End();

    m_command_buffer.Begin();
    command_info.dstAddressesArray.deviceAddress = scratch_buffer.Address();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-dstAddressesArray-10455");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstAddressesArray.deviceAddress = dst_buffer.Address();
    m_command_buffer.End();

    m_command_buffer.Begin();
    command_info.srcInfosCount = (src_count_buffer.Address() / 4) * 4 + 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-srcInfosCount-10481");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.srcInfosCount = src_count_buffer.Address();
    m_command_buffer.End();

    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV;
    input_info.opInput = input;
    command_info.input = input_info;

    m_command_buffer.Begin();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterInfoNV) - 1;
    command_info.dstImplicitData =
        implicit_build_buffer.Address() / accel_struct_props.clusterByteAlignment * accel_struct_props.clusterByteAlignment + 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-srcInfosArray-10476");
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-input-10477");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    command_info.dstImplicitData = implicit_build_buffer.Address();
    m_command_buffer.End();

    m_command_buffer.Begin();
    input_info.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_EXPLICIT_DESTINATIONS_NV;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;
    command_info.input = input_info;
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
    command_info.dstImplicitData = implicit_build_buffer.Address();
    command_info.dstAddressesArray.deviceAddress = 0;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-10471");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, ClusterAccelerationStructureTriangleClusterInput) {
    TEST_DESCRIPTION("Validate all valid usages of VkClusterAccelerationStructureTriangleClusterInputNV");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t total_triangles = 10;
    uint32_t max_triangle_per_cluster = 10;
    uint32_t total_vertices = 3 * total_triangles;
    VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    tri_cluster.maxGeometryIndexValue = total_triangles - 1;

    tri_cluster.maxClusterUniqueGeometryCount = max_triangle_per_cluster - 1;
    tri_cluster.maxClusterTriangleCount = max_triangle_per_cluster;
    tri_cluster.maxClusterVertexCount = max_triangle_per_cluster * 3;

    tri_cluster.maxTotalTriangleCount = total_triangles;
    tri_cluster.maxTotalVertexCount = total_vertices;
    tri_cluster.minPositionTruncateBitCount = 0;

    VkClusterAccelerationStructureOpInputNV input = {};
    input.pTriangleClusters = &tri_cluster;

    VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
    input_info.maxAccelerationStructureCount = 1;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;

    tri_cluster.vertexFormat = VK_FORMAT_R32_SFLOAT;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-vertexFormat-10439");
    VkAccelerationStructureBuildSizesInfoKHR clas_size_info = vku::InitStructHelper();
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

    VkPhysicalDeviceClusterAccelerationStructurePropertiesNV accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);
    tri_cluster.maxClusterTriangleCount = accel_struct_props.maxTrianglesPerCluster + 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-maxClusterTriangleCount-10440");
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.maxClusterTriangleCount = max_triangle_per_cluster;

    tri_cluster.maxClusterVertexCount = accel_struct_props.maxVerticesPerCluster + 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-maxClusterVertexCount-10441");
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.maxClusterVertexCount = max_triangle_per_cluster * 3;

    tri_cluster.minPositionTruncateBitCount = 33;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-minPositionTruncateBitCount-10442");
    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.minPositionTruncateBitCount = 0;

    vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &clas_size_info);

    vkt::Buffer scratch_buffer(*m_device, clas_size_info.buildScratchSize,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    vkt::Buffer src_info_buffer(
        *m_device, sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    uint32_t input_num = 1;
    vkt::Buffer src_count_buffer(
        *m_device, sizeof(uint32_t),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);
    auto *count_data = static_cast<uint32_t *>(src_count_buffer.Memory().Map());
    memcpy(count_data, &input_num, sizeof(input_num));

    vkt::Buffer implicit_buffer(*m_device, clas_size_info.accelerationStructureSize + 256,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                                vkt::device_address);

    VkClusterAccelerationStructureCommandsInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.scratchData = scratch_buffer.Address();
    command_info.srcInfosArray.deviceAddress = src_info_buffer.Address();
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);
    command_info.srcInfosCount = src_count_buffer.Address();
    command_info.dstImplicitData = implicit_buffer.Address();

    m_command_buffer.Begin();

    tri_cluster.vertexFormat = VK_FORMAT_R32_SFLOAT;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-vertexFormat-10439");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

    tri_cluster.maxClusterTriangleCount = accel_struct_props.maxTrianglesPerCluster + 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-maxClusterTriangleCount-10440");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.maxClusterTriangleCount = max_triangle_per_cluster;

    tri_cluster.maxClusterVertexCount = accel_struct_props.maxVerticesPerCluster + 1;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-maxClusterVertexCount-10441");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();
    tri_cluster.maxClusterVertexCount = max_triangle_per_cluster * 3;
    tri_cluster.minPositionTruncateBitCount = 33;
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-minPositionTruncateBitCount-10442");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRayTracing, ClusterAccelerationStructureTriangleClusterStatelessValidation) {
    TEST_DESCRIPTION("Test stateless validation of VkClusterAccelerationStructureTriangleClusterInputNV sType and pNext");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    {
        VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
        tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        tri_cluster.maxClusterTriangleCount = 1;
        tri_cluster.maxClusterVertexCount = 3;

        VkDeviceCreateInfo invalid_pnext = vku::InitStructHelper();
        tri_cluster.pNext = &invalid_pnext;

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pTriangleClusters = &tri_cluster;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-pNext-pNext");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
        tri_cluster.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;  // Wrong sType
        tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        tri_cluster.maxClusterTriangleCount = 1;
        tri_cluster.maxClusterVertexCount = 3;

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pTriangleClusters = &tri_cluster;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-sType-sType");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferCreateInfo invalid_pnext = vku::InitStructHelper();
        VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper(&invalid_pnext);
        tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        tri_cluster.maxClusterTriangleCount = 1;
        tri_cluster.maxClusterVertexCount = 3;

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pTriangleClusters = &tri_cluster;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-pNext-pNext");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
    {
        VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
        // Invalid enum value
        tri_cluster.vertexFormat = static_cast<VkFormat>(999);
        tri_cluster.maxClusterTriangleCount = 1;
        tri_cluster.maxClusterVertexCount = 3;

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pTriangleClusters = &tri_cluster;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureTriangleClusterInputNV-vertexFormat-parameter");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, ClusterAccelerationStructureMoveObjectsStatelessValidation) {
    TEST_DESCRIPTION("Test stateless validation of VkClusterAccelerationStructureMoveObjectsInputNV sType, pNext, and type enum");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());
    {
        VkClusterAccelerationStructureMoveObjectsInputNV move_objects = vku::InitStructHelper();
        move_objects.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        move_objects.type = VK_CLUSTER_ACCELERATION_STRUCTURE_TYPE_TRIANGLE_CLUSTER_NV;

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pMoveObjects = &move_objects;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureMoveObjectsInputNV-sType-sType");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
    {
        VkBufferCreateInfo invalid_pnext = vku::InitStructHelper();
        VkClusterAccelerationStructureMoveObjectsInputNV move_objects = vku::InitStructHelper(&invalid_pnext);
        move_objects.type = VK_CLUSTER_ACCELERATION_STRUCTURE_TYPE_TRIANGLE_CLUSTER_NV;

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pMoveObjects = &move_objects;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureMoveObjectsInputNV-pNext-pNext");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
    {
        VkClusterAccelerationStructureMoveObjectsInputNV move_objects = vku::InitStructHelper();
        // Invalid enum value
        move_objects.type = static_cast<VkClusterAccelerationStructureTypeNV>(999);

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pMoveObjects = &move_objects;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureMoveObjectsInputNV-type-parameter");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, ClusterAccelerationStructureClustersBottomLevelStatelessValidation) {
    TEST_DESCRIPTION("Test stateless validation of VkClusterAccelerationStructureClustersBottomLevelInputNV sType and pNext");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    {
        VkClusterAccelerationStructureClustersBottomLevelInputNV bottom_level = vku::InitStructHelper();
        bottom_level.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;  // Wrong sType

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pClustersBottomLevel = &bottom_level;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureClustersBottomLevelInputNV-sType-sType");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
    {
        VkBufferCreateInfo invalid_pnext = vku::InitStructHelper();
        VkClusterAccelerationStructureClustersBottomLevelInputNV bottom_level = vku::InitStructHelper(&invalid_pnext);

        VkClusterAccelerationStructureOpInputNV input = {};
        input.pClustersBottomLevel = &bottom_level;

        VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
        input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV;
        input_info.opInput = input;

        VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
        m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureClustersBottomLevelInputNV-pNext-pNext");
        vk::GetClusterAccelerationStructureBuildSizesNV(*m_device, &input_info, &size_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, CmdBuildClusterAccelerationStructureIndirectValidation) {
    TEST_DESCRIPTION("Test various validation rules for vkCmdBuildClusterAccelerationStructureIndirectNV");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint32_t total_triangles = 1;
    uint32_t total_vertices = 3 * total_triangles;
    VkClusterAccelerationStructureTriangleClusterInputNV tri_cluster = vku::InitStructHelper();
    tri_cluster.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    tri_cluster.maxGeometryIndexValue = total_triangles - 1;
    tri_cluster.maxClusterUniqueGeometryCount = 0;
    tri_cluster.maxClusterTriangleCount = total_triangles;
    tri_cluster.maxClusterVertexCount = total_vertices;
    tri_cluster.maxTotalTriangleCount = total_triangles;
    tri_cluster.maxTotalVertexCount = total_vertices;
    tri_cluster.minPositionTruncateBitCount = 0;

    VkClusterAccelerationStructureOpInputNV input = {};
    input.pTriangleClusters = &tri_cluster;
    VkClusterAccelerationStructureInputInfoNV input_info = vku::InitStructHelper();
    input_info.maxAccelerationStructureCount = 1;
    input_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    input_info.opType = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV;
    input_info.opInput = input;

    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
    vk::GetClusterAccelerationStructureBuildSizesNV(m_device->handle(), &input_info, &size_info);

    vkt::Buffer scratch_buffer_wrong_usage(
        *m_device, size_info.buildScratchSize,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    vkt::Buffer scratch_buffer(*m_device, size_info.buildScratchSize,
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               vkt::device_address);

    vkt::Buffer small_scratch_buffer(*m_device, size_info.buildScratchSize - 100,
                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                     vkt::device_address);

    vkt::Buffer dst_buffer_wrong_usage(*m_device, size_info.accelerationStructureSize,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                       vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t),
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                             vkt::device_address);
    vkt::Buffer implicit_buffer(*m_device, size_info.accelerationStructureSize,
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                                vkt::device_address);

    auto *data = static_cast<uint32_t *>(count_buffer.Memory().Map());
    memcpy(data, &total_triangles, sizeof(total_triangles));
    VkDeviceAddress count_buffer_address = count_buffer.Address();

    VkClusterAccelerationStructureCommandsInfoNV command_info = vku::InitStructHelper();
    command_info.input = input_info;
    command_info.scratchData = scratch_buffer_wrong_usage.Address();
    command_info.input.opMode = VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV;
    command_info.dstImplicitData = implicit_buffer.Address();
    command_info.srcInfosCount = count_buffer_address;
    command_info.srcInfosArray.stride = sizeof(VkClusterAccelerationStructureBuildTriangleClusterTemplateInfoNV);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-10457");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();

    command_info.scratchData = scratch_buffer.Address();
    command_info.dstImplicitData = dst_buffer_wrong_usage.Address();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pCommandInfos-10459");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();

    vkt::Buffer dst_buffer_small(*m_device, size_info.accelerationStructureSize - 100,
                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                                 vkt::device_address);
    command_info.dstImplicitData = dst_buffer_small.Address();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkClusterAccelerationStructureCommandsInfoNV-opMode-10467");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
    command_info.dstImplicitData = implicit_buffer.Address();

    command_info.scratchData = small_scratch_buffer.Address();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-scratchData-10446");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
    command_info.scratchData = scratch_buffer.Address();

    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;
    {
        const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
        VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
        VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

        const vkt::PipelineLayout pipeline_layout(*m_device, {});

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
        shader_stages[0] = vku::InitStructHelper();
        shader_stages[0].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        shader_stages[0].module = chit_shader;
        shader_stages[0].pName = "main";

        shader_stages[1] = vku::InitStructHelper();
        shader_stages[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        shader_stages[1].module = rgen_shader;
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
        raytracing_pipeline_ci.layout = pipeline_layout;
        raytracing_pipeline_ci.maxPipelineRayRecursionDepth = 1;

        const VkResult result = vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                                 &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
        ASSERT_EQ(VK_SUCCESS, result);
    }

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-pNext-10444");
    vk::CmdBuildClusterAccelerationStructureIndirectNV(m_command_buffer.handle(), &command_info);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}
