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
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "layer_validation_tests.h"

TEST_F(VkLayerTest, RayTracingTestBarrierAccessAccelerationStructure) {
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

    VkMemoryBarrier2 mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = LvlInitStruct<VkDependencyInfo>();
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

TEST_F(VkLayerTest, RayTracingValidateDescriptorBindingUpdateAfterBindWithAccelerationStructure) {
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

TEST_F(VkLayerTest, RayTracingAccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto acc_structure_props = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    GetPhysicalDeviceProperties2(acc_structure_props);

    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t maxBlocks = acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of per stage descriptor update after bind for acceleration structures, "
                        "skipping tests";
    }
    if (maxBlocks < 1) {
        GTEST_SKIP() << "Test requires maxPerStageDescriptorUpdateAfterBindAccelerationStructures >= 1";
    }

    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    dslb_vec.reserve(maxBlocks);
    VkDescriptorSetLayoutBinding dslb = {};
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    for (uint32_t i = 0; i < maxBlocks; ++i) {
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingValidateBeginQueryQueryPoolType) {
    TEST_DESCRIPTION("Test CmdBeginQuery with invalid queryPool queryType");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }

    bool khr_acceleration_structure = IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    bool nv_ray_tracing = IsExtensionsEnabled(VK_NV_RAY_TRACING_EXTENSION_NAME);
    bool ext_transform_feedback = IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);

    if (!khr_acceleration_structure && !nv_ray_tracing) {
        GTEST_SKIP() << "Extensions " << VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME << " and " << VK_NV_RAY_TRACING_EXTENSION_NAME
                     << " are not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    const auto vkCmdBeginQueryIndexedEXT = GetDeviceProcAddr<PFN_vkCmdBeginQueryIndexedEXT, false>("vkCmdBeginQueryIndexedEXT");

    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryCount = 1;

    if (khr_acceleration_structure) {
        {
            query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
            vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
            ASSERT_TRUE(query_pool.initialized());

            m_commandBuffer->begin();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04728");
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");
                vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        }

        {
            query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
            vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
            ASSERT_TRUE(query_pool.initialized());

            m_commandBuffer->begin();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04728");
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");
                vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        }
    }
    if (nv_ray_tracing) {
        query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV;
        vk_testing::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04729");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        if (ext_transform_feedback) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04729");
            vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();
    }
}

TEST_F(VkLayerTest, RayTracingCopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyQueryPoolResults with unsupported query type");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto as_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(as_features);

    if (as_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdCopyAccelerationStructureKHR = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkCmdCopyAccelerationStructureKHR"));
    assert(vkCmdCopyAccelerationStructureKHR != nullptr);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBufferObj buffer_no_mem;
    buffer_no_mem.init_no_mem(*m_device, buffer_ci);
    ASSERT_TRUE(buffer_no_mem.initialized());

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    auto as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer_no_mem.handle();
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vk_testing::AccelerationStructureKHR invalid_as(*m_device, as_create_info);

    as_create_info.buffer = buffer.handle();
    vk_testing::AccelerationStructureKHR valid_as(*m_device, as_create_info);

    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureInfoKHR>();
    copy_info.src = invalid_as.handle();
    copy_info.dst = valid_as.handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = valid_as.handle();
    copy_info.dst = invalid_as.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RayTracingCmdCopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffers not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(accel_features);

    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdCopyAccelerationStructureKHR = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkCmdCopyAccelerationStructureKHR"));
    assert(vkCmdCopyAccelerationStructureKHR != nullptr);
    auto vkCopyAccelerationStructureKHR =
        reinterpret_cast<PFN_vkCopyAccelerationStructureKHR>(vk::GetDeviceProcAddr(device(), "vkCopyAccelerationStructureKHR"));
    assert(vkCopyAccelerationStructureKHR != nullptr);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBufferObj buffer_no_mem;
    buffer_no_mem.init_no_mem(*m_device, buffer_ci);

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    VkBufferObj host_visible_buffer;
    host_visible_buffer.init(*m_device, buffer_ci.size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer_ci.usage);

    auto as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer_no_mem.handle();
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vk_testing::AccelerationStructureKHR as_no_mem(*m_device, as_create_info);
    as_create_info.buffer = buffer.handle();
    vk_testing::AccelerationStructureKHR as_mem(*m_device, as_create_info);
    as_create_info.buffer = host_visible_buffer.handle();
    vk_testing::AccelerationStructureKHR as_host_mem(*m_device, as_create_info);

    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureInfoKHR>();
    copy_info.src = as_no_mem.handle();
    copy_info.dst = as_mem.handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = as_mem.handle();
    copy_info.dst = as_no_mem.handle();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = as_mem.handle();
    copy_info.dst = as_host_mem.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03727");
    vkCopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = as_host_mem.handle();
    copy_info.dst = as_mem.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03728");
    vkCopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingTestCmdCopyMemoryToAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate CmdCopyMemoryToAccelerationStructureKHR with dst buffer not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCmdCopyMemoryToAccelerationStructureKHR =
        GetInstanceProcAddr<PFN_vkCmdCopyMemoryToAccelerationStructureKHR>("vkCmdCopyMemoryToAccelerationStructureKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");
    const auto vkCreateAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR");
    const auto vkDestroyAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR");

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

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = dst_buffer.handle();
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    VkAccelerationStructureKHR as;
    vkCreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, src_buffer.handle()};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(device(), &device_address_info);

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = LvlInitStruct<VkCopyMemoryToAccelerationStructureInfoKHR>();
    copy_info.src.deviceAddress = device_address;
    copy_info.dst = as;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    m_commandBuffer->begin();
    vkCmdCopyMemoryToAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
    vkDestroyAccelerationStructureKHR(device(), as, nullptr);
}

TEST_F(VkLayerTest, RayTracingBuildAccelerationStructureKHR) {
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

    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(acc_structure_features);
    if (acc_structure_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkBuildAccelerationStructuresKHR =
        GetInstanceProcAddr<PFN_vkBuildAccelerationStructuresKHR>("vkBuildAccelerationStructuresKHR");

    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkBufferObj host_buffer;
    host_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);
    as_create_info.buffer = host_buffer.handle();
    VkAccelerationStructurekhrObj host_bot_level_as(*m_device, as_create_info);

    auto build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = bot_level_as.handle();
    build_info_khr.geometryCount = 0;
    build_info_khr.pGeometries = nullptr;
    build_info_khr.ppGeometries = nullptr;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *p_build_range_info = &build_range_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_khr, &p_build_range_info);
    m_errorMonitor->VerifyFound();

    build_info_khr.srcAccelerationStructure = bot_level_as.handle();
    build_info_khr.dstAccelerationStructure = host_bot_level_as.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_khr, &p_build_range_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingTestWriteAccelerationStructureMemory) {
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

    auto as_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(as_features);

    if (as_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature is not supported";
    }
    if (as_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkWriteAccelerationStructuresPropertiesKHR =
        GetInstanceProcAddr<PFN_vkWriteAccelerationStructuresPropertiesKHR>("vkWriteAccelerationStructuresPropertiesKHR");
    const auto vkBuildAccelerationStructuresKHR =
        GetInstanceProcAddr<PFN_vkBuildAccelerationStructuresKHR>("vkBuildAccelerationStructuresKHR");

    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    vk_testing::AccelerationStructureKHR acceleration_structure(*m_device, as_create_info);

    auto build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = acceleration_structure.handle();
    build_info_khr.geometryCount = 0;
    build_info_khr.pGeometries = nullptr;
    build_info_khr.ppGeometries = nullptr;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &build_range_info;
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_khr, &pBuildRangeInfos);
    m_errorMonitor->VerifyFound();

    std::vector<uint32_t> data(4096);

    VkAccelerationStructureKHR acceleration_structure_handle = acceleration_structure.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");
    vkWriteAccelerationStructuresPropertiesKHR(device(), 1, &acceleration_structure_handle,
                                               VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, 4096, data.data(), 4096);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingTestCopyMemoryToAsBuffer) {
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

    const auto vkCopyMemoryToAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkCopyMemoryToAccelerationStructureKHR>("vkCopyMemoryToAccelerationStructureKHR");

    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);

    uint8_t output[4096];
    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(output);

    auto info = LvlInitStruct<VkCopyMemoryToAccelerationStructureInfoKHR>();
    info.dst = bot_level_as.handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    vkCopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingValidationArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "Core validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders.");

    OOBRayTracingShadersTestBody(false);
}

TEST_F(VkLayerTest, RayTracingValidateCreateAccelerationStructureKHR) {
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

    const auto vkCreateAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");

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
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(device(), &device_address_info);
    // invalid buffer;
    {
        VkBufferObj invalid_buffer;
        invalid_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.buffer = invalid_buffer.handle();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614");
        vkCreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid deviceAddress and flag;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.deviceAddress = device_address;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612");
        vkCreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();

        invalid_as_create_info.deviceAddress = 0;
        invalid_as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-parameter");
        vkCreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid size and offset;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.size = 4097;  // buffer size is 4096
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-offset-03616");
        vkCreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid sType;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-sType-sType");
        vkCreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid type;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-type-parameter");
        vkCreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingValidateCreateAccelerationStructureKHRReplayFeature) {
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

    const auto vkCreateAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);

    VkBufferDeviceAddressInfo device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    device_address_info.buffer = buffer.handle();
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(device(), &device_address_info);

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
    vkCreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingValidateCmdTraceRaysKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    bda_features.bufferDeviceAddress = VK_TRUE;
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&bda_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
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

        const char *empty_shader = R"glsl(
            #version 460
            #extension GL_EXT_ray_tracing : require
            void main() {}
        )glsl";

        VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
        VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

        auto vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
        ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);

        const VkPipelineLayoutObj pipeline_layout(m_device, {});

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";
        shader_stages.emplace_back(stage_create_info);

        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_info.module = rgen_shader.handle();
        shader_stages.emplace_back(stage_create_info);

        VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        raytracing_pipeline_ci.flags = 0;
        raytracing_pipeline_ci.stageCount = static_cast<uint32_t>(shader_stages.size());
        raytracing_pipeline_ci.pStages = shader_stages.data();
        raytracing_pipeline_ci.layout = pipeline_layout.handle();

        const VkResult result = vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
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

    const auto vkGetPhysicalDeviceProperties2KHR =
        GetInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties2KHR>("vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    const auto vkCmdTraceRaysKHR = GetInstanceProcAddr<PFN_vkCmdTraceRaysKHR>("vkCmdTraceRaysKHR");
    ASSERT_TRUE(vkCmdTraceRaysKHR != nullptr);

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
            vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                              100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03690");
            vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                              100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03686");
            vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                              100, 1);
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
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04035");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04029");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
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

    // pRayGenShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                          100, 1);
    }

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        // This address is the same as the one from the first (valid) buffer, so no validation error
        invalid_stride.deviceAddress = device_address_missing_flag;
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
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
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();
    }
    // pMissShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();
    }
    // pHitShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                          100, 1);
    }
    // pCallableShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                          100, 1);
    }

    m_commandBuffer->end();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

TEST_F(VkLayerTest, RayTracingValidateCmdTraceRaysIndirectKHR) {
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
    VkBuffer buffer;
    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 4096;
    VkDeviceMemory mem;
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(device(), buffer, mem, 0);

    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    GetPhysicalDeviceProperties2(properties2);

    const auto vkCmdTraceRaysIndirectKHR = GetInstanceProcAddr<PFN_vkCmdTraceRaysIndirectKHR>("vkCmdTraceRaysIndirectKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(device(), &device_address_info);

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
            vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                      &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                      &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion,
                                      &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride =
            ray_tracing_properties.maxShaderGroupStride + (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                  device_address);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                  device_address);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                  device_address);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
    vk::DestroyBuffer(device(), buffer, nullptr);
    vk::FreeMemory(device(), mem, nullptr);
}

TEST_F(VkLayerTest, RayTracingValidateVkAccelerationStructureVersionInfoKHR) {
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

    const auto vkGetDeviceAccelerationStructureCompatibilityKHR =
        GetInstanceProcAddr<PFN_vkGetDeviceAccelerationStructureCompatibilityKHR>(
            "vkGetDeviceAccelerationStructureCompatibilityKHR");

    VkAccelerationStructureVersionInfoKHR valid_version = LvlInitStruct<VkAccelerationStructureVersionInfoKHR>();
    VkAccelerationStructureCompatibilityKHR compatablity;
    uint8_t mode[] = {VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR};
    valid_version.pVersionData = mode;
    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-sType-sType");
        vkGetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.pVersionData = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-pVersionData-parameter");
        vkGetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingValidateCmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

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
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCmdBuildAccelerationStructuresKHR =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructuresKHR>("vkCmdBuildAccelerationStructuresKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");
    const auto vkCmdBuildAccelerationStructuresIndirectKHR =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructuresIndirectKHR>("vkCmdBuildAccelerationStructuresIndirectKHR");
    auto vkBuildAccelerationStructuresKHR =
        GetDeviceProcAddr<PFN_vkBuildAccelerationStructuresKHR>("vkBuildAccelerationStructuresKHR");
    auto vkGetAccelerationStructureBuildSizesKHR =
        GetInstanceProcAddr<PFN_vkGetAccelerationStructureBuildSizesKHR>("vkGetAccelerationStructureBuildSizesKHR");

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometryNV;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometryNV, 0, true);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);

    VkBufferDeviceAddressInfo vertexAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                   geometryNV.geometry.triangles.vertexData};
    VkDeviceAddress vertexAddress = vkGetBufferDeviceAddressKHR(device(), &vertexAddressInfo);

    VkBufferDeviceAddressInfo indexAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                  geometryNV.geometry.triangles.indexData};
    VkDeviceAddress indexAddress = vkGetBufferDeviceAddressKHR(device(), &indexAddressInfo);
    VkAccelerationStructureGeometryKHR valid_geometry_triangles = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    valid_geometry_triangles.geometryType = geometryNV.geometryType;
    valid_geometry_triangles.geometry.triangles = LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
    valid_geometry_triangles.geometry.triangles.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    valid_geometry_triangles.geometry.triangles.vertexData.deviceAddress = vertexAddress;
    valid_geometry_triangles.geometry.triangles.vertexStride = 8;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.indexData.deviceAddress = indexAddress;
    valid_geometry_triangles.geometry.triangles.transformData.deviceAddress = 0;
    valid_geometry_triangles.geometry.triangles.maxVertex = 1;
    valid_geometry_triangles.flags = 0;
    VkAccelerationStructureGeometryKHR *pGeometry = &valid_geometry_triangles;

    const auto vkGetPhysicalDeviceProperties2KHR =
        GetInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties2KHR>("vkGetPhysicalDeviceProperties2KHR");

    auto acc_struct_properties = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&acc_struct_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    create_info.size = acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
    const VkBufferObj bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device, &create_info, true);
    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                     bot_level_as_scratch.handle()};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(device(), &device_address_info);
    VkAccelerationStructureBuildGeometryInfoKHR build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info_khr.flags = 0;
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = bot_level_as.handle();
    build_info_khr.geometryCount = 1;
    build_info_khr.pGeometries = pGeometry;
    build_info_khr.ppGeometries = NULL;
    build_info_khr.scratchData.deviceAddress = device_address;

    VkAccelerationStructureBuildGeometryInfoKHR build_info_ppGeometries_khr = build_info_khr;
    build_info_ppGeometries_khr.pGeometries = NULL;
    build_info_ppGeometries_khr.ppGeometries = &pGeometry;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;
    VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &build_range_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording");
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
    m_errorMonitor->VerifyFound();

    {  // dstAccelerationStructure == VK_NULL_HANDLE
        auto build_info_null_dst_handle = build_info_khr;
        build_info_null_dst_handle.dstAccelerationStructure = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_null_dst_handle, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();

        if (accel_features.accelerationStructureIndirectBuild == VK_TRUE) {
            VkDeviceAddress range_ptrs{};
            uint32_t indirect_strides = sizeof(VkAccelerationStructureBuildRangeInfoKHR);
            uint32_t max_prim_counts[1] = {1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800");
            vkCmdBuildAccelerationStructuresIndirectKHR(m_commandBuffer->handle(), 1, &build_info_null_dst_handle, &range_ptrs,
                                                        &indirect_strides, reinterpret_cast<uint32_t **>(&max_prim_counts));
            m_errorMonitor->VerifyFound();
        }

        if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
            vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_null_dst_handle, &pBuildRangeInfos);
            m_errorMonitor->VerifyFound();
        }
    }

    m_commandBuffer->begin();

    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_ppGeometries_khr, &pBuildRangeInfos);
    // Invalid info count
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 0, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Invalid pInfos
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, NULL, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Invalid ppBuildRangeInfos
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, NULL);
        m_errorMonitor->VerifyFound();
    }

    // must be called outside renderpass
    {
        ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-renderpass");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
    }
    // Invalid flags
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.flags = VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Invalid dst buffer
    {
        auto buffer_ci =
            VkBufferObj::create_info(4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        VkBufferObj invalid_buffer;
        invalid_buffer.init_no_mem(*m_device, buffer_ci);
        as_create_info.buffer = invalid_buffer.handle();
        as_create_info.createFlags = 0;
        as_create_info.offset = 0;
        as_create_info.size = 0;
        as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        as_create_info.deviceAddress = 0;
        VkAccelerationStructurekhrObj invalid_bot_level_as(*m_device, as_create_info);

        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.dstAccelerationStructure = invalid_bot_level_as.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Invalid sType
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Invalid Type
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();

        invalid_build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Total number of triangles in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = std::numeric_limits<uint32_t>::max();
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795");
            vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info_khr,
                                                    &primitive_count, &build_size_info);
            m_errorMonitor->VerifyFound();
        }
    }
    // Total number of AABBs in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = std::numeric_limits<uint32_t>::max();
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
            auto [aabb_buffer, aabb_geometry] = GetSimpleAABB(*m_device, VK_API_VERSION_1_1);
            auto aabb_build_info_khr = build_info_khr;
            aabb_build_info_khr.pGeometries = &aabb_geometry;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794");
            vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &aabb_build_info_khr,
                                                    &primitive_count, &build_size_info);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride in pGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
        auto [aabb_buffer, as_aabb_geometry] = GetSimpleAABB(*m_device, VK_API_VERSION_1_1);
        as_aabb_geometry.geometry.aabbs.stride = 1;
        auto invalid_aabb_build_info_khr = build_info_khr;
        invalid_aabb_build_info_khr.pGeometries = &as_aabb_geometry;
        constexpr uint32_t primitive_count = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                &invalid_aabb_build_info_khr, &primitive_count, &build_size_info);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
        auto [aabb_buffer, as_aabb_geometry] = GetSimpleAABB(*m_device, VK_API_VERSION_1_1);
        as_aabb_geometry.geometry.aabbs.stride = 1;
        auto invalid_aabb_build_info_khr = build_info_khr;
        invalid_aabb_build_info_khr.pGeometries = nullptr;
        const auto ppGeometry = &as_aabb_geometry;
        invalid_aabb_build_info_khr.ppGeometries = &ppGeometry;
        constexpr uint32_t primitive_count = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                &invalid_aabb_build_info_khr, &primitive_count, &build_size_info);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in pGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
        auto [aabb_buffer, as_aabb_geometry] = GetSimpleAABB(*m_device, VK_API_VERSION_1_1);
        as_aabb_geometry.geometry.aabbs.stride = 8ull * std::numeric_limits<uint32_t>::max();
        auto invalid_aabb_build_info_khr = build_info_khr;
        invalid_aabb_build_info_khr.pGeometries = &as_aabb_geometry;
        constexpr uint32_t primitive_count = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                &invalid_aabb_build_info_khr, &primitive_count, &build_size_info);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
        auto [aabb_buffer, as_aabb_geometry] = GetSimpleAABB(*m_device, VK_API_VERSION_1_1);
        as_aabb_geometry.geometry.aabbs.stride = 8ull * std::numeric_limits<uint32_t>::max();
        auto invalid_aabb_build_info_khr = build_info_khr;
        invalid_aabb_build_info_khr.pGeometries = nullptr;
        const auto ppGeometry = &as_aabb_geometry;
        invalid_aabb_build_info_khr.ppGeometries = &ppGeometry;
        constexpr uint32_t primitive_count = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        vkGetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                &invalid_aabb_build_info_khr, &primitive_count, &build_size_info);
        m_errorMonitor->VerifyFound();
    }
    // Invalid VetexStride and Indexdata
    {
        VkAccelerationStructureGeometryKHR invalid_geometry_triangles = valid_geometry_triangles;
        invalid_geometry_triangles.geometry.triangles.vertexStride = UINT32_MAX + 1ull;
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.pGeometries = &invalid_geometry_triangles;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();

        invalid_geometry_triangles = valid_geometry_triangles;
        invalid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT8_EXT;
        invalid_build_info_khr.pGeometries = &invalid_geometry_triangles;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // ppGeometries and pGeometries both valid pointer
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.ppGeometries = &invalid_build_info_khr.pGeometries;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Buffer is missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR usage flag
    {
        VkBufferObj bad_usage_buffer;
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        bad_usage_buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);
        VkBufferDeviceAddressInfo addr_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, bad_usage_buffer.handle()};
        auto addr = vkGetBufferDeviceAddressKHR(device(), &addr_info);
        pGeometry[0].geometry.triangles.vertexData.deviceAddress = addr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
        pGeometry[0] = valid_geometry_triangles;  // reset to valid
    }

    // Scratch data buffer is missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag
    {
        VkBufferCreateInfo bad_create_info = LvlInitStruct<VkBufferCreateInfo>();
        bad_create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        bad_create_info.size = acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
        const VkBufferObj bad_scratch = bot_level_as.create_scratch_buffer(*m_device, &bad_create_info, true);
        VkBufferDeviceAddressInfo bad_device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
        bad_device_address_info.buffer = bad_scratch.handle();
        VkDeviceAddress bad_device_address = vkGetBufferDeviceAddressKHR(device(), &bad_device_address_info);

        build_info_khr.scratchData.deviceAddress = bad_device_address;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is 0
    {
        build_info_khr.scratchData.deviceAddress = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingObjInUseCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    accel_features.accelerationStructureIndirectBuild = VK_TRUE;
    accel_features.accelerationStructureHostCommands = VK_TRUE;
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCmdBuildAccelerationStructuresKHR =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructuresKHR>("vkCmdBuildAccelerationStructuresKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");
    const auto vkDestroyAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR");

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometryNV;
    const VkDeviceSize kBufferOffset = 256;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometryNV, kBufferOffset, true);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);

    auto address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    address_info.buffer = geometryNV.geometry.triangles.vertexData;
    VkDeviceAddress vertexAddress = vkGetBufferDeviceAddressKHR(device(), &address_info);

    address_info.buffer = geometryNV.geometry.triangles.indexData;
    VkDeviceAddress indexAddress = vkGetBufferDeviceAddressKHR(device(), &address_info);

    auto valid_geometry_triangles = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    valid_geometry_triangles.geometryType = geometryNV.geometryType;
    valid_geometry_triangles.geometry.triangles = LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
    valid_geometry_triangles.geometry.triangles.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    valid_geometry_triangles.geometry.triangles.vertexData.deviceAddress = vertexAddress + kBufferOffset;
    valid_geometry_triangles.geometry.triangles.vertexStride = 8;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.indexData.deviceAddress = indexAddress + kBufferOffset;
    valid_geometry_triangles.geometry.triangles.transformData.deviceAddress = 0;
    valid_geometry_triangles.geometry.triangles.maxVertex = 1;
    valid_geometry_triangles.flags = 0;

    auto acc_struct_properties = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&acc_struct_properties);
    GetPhysicalDeviceProperties2(properties2);

    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    create_info.size =
        std::max<VkDeviceSize>(acc_struct_properties.minAccelerationStructureScratchOffsetAlignment, kBufferOffset) * 2;
    const VkBufferObj bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device, &create_info, true);

    address_info.buffer = bot_level_as_scratch.handle();
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(device(), &address_info);

    auto build_info = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info.flags = 0;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = bot_level_as.handle();
    build_info.geometryCount = 1;
    build_info.pGeometries = &valid_geometry_triangles;
    build_info.ppGeometries = NULL;
    build_info.scratchData.deviceAddress = device_address + kBufferOffset;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info{};
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *range_infos[1];
    range_infos[0] = &build_range_info;

    m_commandBuffer->begin();
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info, range_infos);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    vkDestroyAccelerationStructureKHR(device(), bot_level_as.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, RayTracingCmdCopyAccelerationStructureToMemoryKHR) {
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

    const auto vkCreateAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR");
    const auto vkDestroyAccelerationStructureKHR =
        GetDeviceProcAddr<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR");
    const auto vkCmdCopyAccelerationStructureToMemoryKHR =
        GetDeviceProcAddr<PFN_vkCmdCopyAccelerationStructureToMemoryKHR>("vkCmdCopyAccelerationStructureToMemoryKHR");

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
    vkCreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);

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
    vkCmdCopyAccelerationStructureToMemoryKHR(cb.handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    cb.end();

    vkDestroyAccelerationStructureKHR(device(), as, nullptr);
}

TEST_F(VkLayerTest, RayTracingUpdateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Test for updating an acceleration structure without a srcAccelerationStructure");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto buffer_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&buffer_address_features);
    auto features2 = GetPhysicalDeviceFeatures2(acc_structure_features);
    if (acc_structure_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature not supported";
    }
    if (buffer_address_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdBuildAccelerationStructuresKHR =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructuresKHR>("vkCmdBuildAccelerationStructuresKHR");
    auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddress>("vkGetBufferDeviceAddressKHR");

    // Create bottom level acceleration structure buffer
    VkBufferObj as_buffer;
    as_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = as_buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);

    // Create scratch buffer
    VkBufferCreateInfo scratch_buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    scratch_buffer_ci.size = 4096;
    scratch_buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const VkBufferObj scratch_buffer = bot_level_as.create_scratch_buffer(*m_device, &scratch_buffer_ci, true);

    VkBufferDeviceAddressInfo scratch_buffer_device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    scratch_buffer_device_address_info.buffer = scratch_buffer.handle();
    VkDeviceAddress scratch_address = vkGetBufferDeviceAddressKHR(device(), &scratch_buffer_device_address_info);
    ASSERT_TRUE(scratch_address != 0);

    // Build acceleration structure
    auto build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = bot_level_as.handle();
    build_info_khr.geometryCount = 0;
    build_info_khr.pGeometries = nullptr;
    build_info_khr.ppGeometries = nullptr;
    build_info_khr.scratchData.deviceAddress = scratch_address;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *p_build_range_info = &build_range_info;

    m_commandBuffer->begin();
    // Build acceleration structure
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &p_build_range_info);

    // Update acceleration structure, .srcAccelerationStructure == VK_NULL_HANDLE
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630");
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &p_build_range_info);
    m_errorMonitor->VerifyFound();

    // Update acceleration structure, .srcAccelerationStructure is a valid handle
    build_info_khr.srcAccelerationStructure = bot_level_as.handle();
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &p_build_range_info);

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RayTracingBuffersAndBufferDeviceAddressesMapping) {
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
    auto buffer_addr_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    buffer_addr_features.bufferDeviceAddress = VK_TRUE;
    accel_features.accelerationStructureIndirectBuild = VK_TRUE;
    accel_features.accelerationStructureHostCommands = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&buffer_addr_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCmdBuildAccelerationStructuresKHR =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructuresKHR>("vkCmdBuildAccelerationStructuresKHR");
    const auto vkGetBufferDeviceAddressKHR = GetDeviceProcAddr<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");

    // Allocate common buffer memory
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vk_testing::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers, with correct and incorrect usage
    constexpr size_t N = 3;
    std::array<std::unique_ptr<VkBufferObj>, N> vbos{};
    std::array<std::unique_ptr<VkBufferObj>, N> ibos{};
    std::array<VkGeometryNV, N> geometriesNV{};
    const VkBufferUsageFlags good_buffer_usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV |
                                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags bad_buffer_usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = good_buffer_usage;
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            buffer_ci.usage = bad_buffer_usage;
        }
        vbos[i] = std::unique_ptr<VkBufferObj>(new VkBufferObj);
        vbos[i]->init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), vbos[i]->handle(), buffer_memory.handle(), 0);

        ibos[i] = std::unique_ptr<VkBufferObj>(new VkBufferObj);
        ibos[i]->init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), ibos[i]->handle(), buffer_memory.handle(), 0);

        // Those calls to vkGetBufferDeviceAddressKHR will internally record vbo and ibo device addresses
        {
            VkBufferDeviceAddressInfo addr_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, vbos[i]->handle()};
            const VkDeviceAddress vbo_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &addr_info);
            addr_info.buffer = ibos[i]->handle();
            const VkDeviceAddress ibo_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &addr_info);
            if (vbo_address != ibo_address) {
                GTEST_SKIP()
                    << "Bounding two buffers to the same memory location does not result in identical buffer device addresses";
            }
        }

        geometriesNV[i] = LvlInitStruct<VkGeometryNV>();
        geometriesNV[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        geometriesNV[i].geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        geometriesNV[i].geometry.triangles.vertexData = vbos[i]->handle();
        geometriesNV[i].geometry.triangles.vertexOffset = 0;
        geometriesNV[i].geometry.triangles.vertexCount = 3;
        geometriesNV[i].geometry.triangles.vertexStride = 12;
        geometriesNV[i].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        geometriesNV[i].geometry.triangles.indexData = ibos[i]->handle();
        geometriesNV[i].geometry.triangles.indexOffset = 0;
        geometriesNV[i].geometry.triangles.indexCount = 3;
        geometriesNV[i].geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        geometriesNV[i].geometry.triangles.transformData = VK_NULL_HANDLE;
        geometriesNV[i].geometry.triangles.transformOffset = 0;
        geometriesNV[i].geometry.aabbs = {};
        geometriesNV[i].geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    }

    struct AccelerationStructureData {
        VkAccelerationStructureGeometryKHR valid_geometry_triangles;
        VkBufferObj bot_level_accel_struct_scratch_buffer;
        VkBufferObj accel_struct_buffer;
        VkAccelerationStructurekhrObj bot_level_accel_struct;
        VkAccelerationStructureBuildRangeInfoKHR build_range_info;
        VkAccelerationStructureBuildGeometryInfoKHR accel_struct_build_geom_info;
    };

    // Helper lambda to fill the necessary acceleration structure data
    auto fill_accel_struct_data = [this, vkGetBufferDeviceAddressKHR](const VkGeometryNV &geometryNV,
                                                                      AccelerationStructureData &accel_struct_data) -> void {
        // Create acceleration structure geometry object
        const VkBufferDeviceAddressInfo vertex_buffer_addr_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                                   geometryNV.geometry.triangles.vertexData};
        const VkDeviceAddress vertex_buffer_addr = vkGetBufferDeviceAddressKHR(m_device->handle(), &vertex_buffer_addr_info);
        const VkBufferDeviceAddressInfo index_buffer_addr_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                                  geometryNV.geometry.triangles.indexData};
        const VkDeviceAddress index_buffer_addr = vkGetBufferDeviceAddressKHR(m_device->handle(), &index_buffer_addr_info);
        accel_struct_data.valid_geometry_triangles = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
        accel_struct_data.valid_geometry_triangles.geometryType = geometryNV.geometryType;
        accel_struct_data.valid_geometry_triangles.geometry.triangles =
            LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
        accel_struct_data.valid_geometry_triangles.geometry.triangles.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
        accel_struct_data.valid_geometry_triangles.geometry.triangles.vertexData.deviceAddress = vertex_buffer_addr;
        accel_struct_data.valid_geometry_triangles.geometry.triangles.vertexStride = 8;
        accel_struct_data.valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accel_struct_data.valid_geometry_triangles.geometry.triangles.indexData.deviceAddress = index_buffer_addr;
        accel_struct_data.valid_geometry_triangles.geometry.triangles.transformData.deviceAddress = 0;
        accel_struct_data.valid_geometry_triangles.geometry.triangles.maxVertex = 1;
        accel_struct_data.valid_geometry_triangles.flags = 0;
        VkAccelerationStructureGeometryKHR *pGeometry = &accel_struct_data.valid_geometry_triangles;

        // Create bottom level acceleration structure object and scratch buffer
        auto acc_struct_properties = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
        auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&acc_struct_properties);
        GetPhysicalDeviceProperties2(properties2);

        VkBufferCreateInfo scatch_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
        scatch_buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scatch_buffer_create_info.size = acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
        auto bot_level_accel_struct_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();

        accel_struct_data.accel_struct_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        bot_level_accel_struct_create_info.buffer = accel_struct_data.accel_struct_buffer.handle();
        bot_level_accel_struct_create_info.createFlags = 0;
        bot_level_accel_struct_create_info.offset = 0;
        bot_level_accel_struct_create_info.size = 0;
        bot_level_accel_struct_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        bot_level_accel_struct_create_info.deviceAddress = 0;
        accel_struct_data.bot_level_accel_struct.init(*m_device, bot_level_accel_struct_create_info);
        accel_struct_data.bot_level_accel_struct_scratch_buffer =
            accel_struct_data.bot_level_accel_struct.create_scratch_buffer(*m_device, &scatch_buffer_create_info, true);
        const VkBufferDeviceAddressInfo scratch_buffer_addr_info = {
            VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, accel_struct_data.bot_level_accel_struct_scratch_buffer.handle()};
        VkDeviceAddress scratch_buffer_device_addr = vkGetBufferDeviceAddressKHR(m_device->handle(), &scratch_buffer_addr_info);

        // Finally, create acceleration structure object
        accel_struct_data.accel_struct_build_geom_info = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
        accel_struct_data.accel_struct_build_geom_info.flags = 0;
        accel_struct_data.accel_struct_build_geom_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accel_struct_data.accel_struct_build_geom_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accel_struct_data.accel_struct_build_geom_info.srcAccelerationStructure = VK_NULL_HANDLE;
        accel_struct_data.accel_struct_build_geom_info.dstAccelerationStructure = accel_struct_data.bot_level_accel_struct.handle();
        accel_struct_data.accel_struct_build_geom_info.geometryCount = 1;
        accel_struct_data.accel_struct_build_geom_info.pGeometries = pGeometry;
        accel_struct_data.accel_struct_build_geom_info.ppGeometries = NULL;
        accel_struct_data.accel_struct_build_geom_info.scratchData.deviceAddress = scratch_buffer_device_addr;

        accel_struct_data.build_range_info.firstVertex = 0;
        accel_struct_data.build_range_info.primitiveCount = 1;
        accel_struct_data.build_range_info.primitiveOffset = 3;
        accel_struct_data.build_range_info.transformOffset = 0;
    };

    // The first series of calls to vkCmdBuildAccelerationStructuresKHR should succeed,
    // since the first vbo and ibo do have the VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR flag.
    // After deleting the valid vbo and ibo, calls are expected to fail.
    for (size_t i = 0; i < N; ++i) {
        auto cmd_buffer = std::unique_ptr<VkCommandBufferObj>(new VkCommandBufferObj(m_device, m_commandPool));
        cmd_buffer->begin();

        AccelerationStructureData accel_struct_data;
        fill_accel_struct_data(geometriesNV[i], accel_struct_data);
        VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &accel_struct_data.build_range_info;
        vkCmdBuildAccelerationStructuresKHR(cmd_buffer->handle(), 1, &accel_struct_data.accel_struct_build_geom_info,
                                            &pBuildRangeInfos);
        cmd_buffer->end();
    }

    for (size_t i = 0; i < N; ++i) {
        auto cmd_buffer = std::unique_ptr<VkCommandBufferObj>(new VkCommandBufferObj(m_device, m_commandPool));
        cmd_buffer->begin();

        AccelerationStructureData accel_struct_data;
        fill_accel_struct_data(geometriesNV[i], accel_struct_data);
        VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &accel_struct_data.build_range_info;
        if (i > 0) {
            // for vbo
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
            // for ibo
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        }
        vkCmdBuildAccelerationStructuresKHR(cmd_buffer->handle(), 1, &accel_struct_data.accel_struct_build_geom_info,
                                            &pBuildRangeInfos);
        if (i > 0) {
            m_errorMonitor->VerifyFound();
        }

        cmd_buffer->end();
        vbos[i] = nullptr;
        ibos[i] = nullptr;
    }
}

TEST_F(VkLayerTest, NVRayTracingAccelerationStructureBindings) {
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

TEST_F(VkLayerTest, NVRayTracingValidateGeometry) {
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

    const std::vector<float> vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    const std::vector<uint32_t> indicies = {0, 1, 2};
    const std::vector<float> aabbs = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    const std::vector<float> transforms = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

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

    const auto vkCreateAccelerationStructureNV =
        GetDeviceProcAddr<PFN_vkCreateAccelerationStructureNV>("vkCreateAccelerationStructureNV");

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
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - not multiple of component size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02429");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 12 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex buffer - no such buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = VkBuffer(123456789);

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexData-parameter");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#endif

    // Invalid index offset - not multiple of index size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02432");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02431");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid transform offset - not multiple of 16.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02438");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid transform offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02437");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid aabb offset - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02440");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 8 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02439");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb stride - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-stride-02441");
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, NVRayTracingValidateCreateAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkCreateAccelerationStructureNV =
        GetDeviceProcAddr<PFN_vkCreateAccelerationStructureNV>("vkCreateAccelerationStructureNV");

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
        vkCreateAccelerationStructureNV(device(), &bad_top_level_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &bad_flags_level_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &bad_compacting_as_create_info, nullptr, &as);
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
        vkCreateAccelerationStructureNV(device(), &mix_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, NVRayTracingValidateBindAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkBindAccelerationStructureMemoryNV =
        GetDeviceProcAddr<PFN_vkBindAccelerationStructureMemoryNV>("vkBindAccelerationStructureMemoryNV");

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
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_freed);
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
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_alignment);
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
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
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
            (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
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
            (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_type);
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

        ASSERT_VK_SUCCESS(vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_1));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620");
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(VkLayerTest, NVRayTracingValidateWriteDescriptorSetAccelerationStructure) {
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

TEST_F(VkLayerTest, NVRayTracingValidateCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkCmdBuildAccelerationStructureNV =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructureNV>("vkCmdBuildAccelerationStructureNV");

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
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
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
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_type, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible flags
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_flags = bot_level_as_create_info.info;
    as_build_info_with_incompatible_flags.flags = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_flags, VK_NULL_HANDLE, 0,
                                      VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible build size
    VkGeometryNV geometry_with_more_vertices = geometry;
    geometry_with_more_vertices.geometry.triangles.vertexCount += 1;

    VkAccelerationStructureInfoNV as_build_info_with_incompatible_geometry = bot_level_as_create_info.info;
    as_build_info_with_incompatible_geometry.pGeometries = &geometry_with_more_vertices;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_geometry, VK_NULL_HANDLE, 0,
                                      VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer too small
    VkBufferCreateInfo too_small_scratch_buffer_info = LvlInitStruct<VkBufferCreateInfo>();
    too_small_scratch_buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    too_small_scratch_buffer_info.size = 1;
    VkBufferObj too_small_scratch_buffer(*m_device, too_small_scratch_buffer_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, too_small_scratch_buffer.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer with offset too small
    VkDeviceSize scratch_buffer_offset = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), scratch_buffer_offset);
    m_errorMonitor->VerifyFound();

    // Src must have been built before
    VkAccelerationStructureObj bot_level_as_updated(*m_device, bot_level_as_create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02489");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                      bot_level_as_updated.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Src must have been built before with the VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV flag
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02490");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                      bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid scratch buffer (invalid usage)
    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    const VkBufferObj bot_level_as_invalid_scratch = bot_level_as.create_scratch_buffer(*m_device, &create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-scratch-02781");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_invalid_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid instance data.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-instanceData-02782");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info,
                                      bot_level_as_invalid_scratch.handle(), 0, VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE,
                                      bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // must be called outside renderpass
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-renderpass");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NVRayTracingObjInUseCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkCmdBuildAccelerationStructureNV =
        GetDeviceProcAddr<PFN_vkCmdBuildAccelerationStructureNV>("vkCmdBuildAccelerationStructureNV");
    const auto vkDestroyAccelerationStructureNV =
        GetDeviceProcAddr<PFN_vkDestroyAccelerationStructureNV>("vkDestroyAccelerationStructureNV");

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
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
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
    vkDestroyAccelerationStructureNV(device(), bot_level_as.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, NVRayTracingValidateGetAccelerationStructureHandle) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkGetAccelerationStructureHandleNV =
        GetDeviceProcAddr<PFN_vkGetAccelerationStructureHandleNV>("vkGetAccelerationStructureHandleNV");

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
        vkGetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint8_t), &handle);
        m_errorMonitor->VerifyFound();
    }

    // No memory bound to acceleration structure
    {
        VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info, /*init_memory=*/false);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
        vkGetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint64_t), &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, NVRayTracingValidateCmdCopyAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure copying.");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkCmdCopyAccelerationStructureNV =
        GetDeviceProcAddr<PFN_vkCmdCopyAccelerationStructureNV>("vkCmdCopyAccelerationStructureNV");

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
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Src must have been created with allow compaction flag
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-src-03411");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV);
    m_errorMonitor->VerifyFound();

    // Dst must have been bound with memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-dst-07792");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as_without_mem.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);

    m_errorMonitor->VerifyFound();

    // mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-03410");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR);
    m_errorMonitor->VerifyFound();

    // mode must be a valid VkCopyAccelerationStructureModeKHR value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-parameter");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR);
    m_errorMonitor->VerifyFound();

    // This command must only be called outside of a render pass instance
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-renderpass");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}
