/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Rodrigo Locatti <rlocatti@nvidia.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

// Tests for NVIDIA-specific best practices
const char *kEnableNVIDIAValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA";

static constexpr float defaultQueuePriority = 0.0f;

TEST_F(VkNvidiaBestPracticesLayerTest, PageableDeviceLocalMemory) {
    AddRequiredExtensions(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    InitBestPracticesFramework(kEnableNVIDIAValidation);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkDeviceQueueCreateInfo queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageable = LvlInitStruct<VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT>();
    pageable.pageableDeviceLocalMemory = VK_TRUE;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateDevice-PageableDeviceLocalMemory");
        VkDevice test_device = VK_NULL_HANDLE;
        VkResult err = vk::CreateDevice(gpu(), &device_ci, nullptr, &test_device);
        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vk::DestroyDevice(test_device, nullptr);
        }
    }

    // Now enable the expected features
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
    device_ci.pNext = &pageable;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateDevice-PageableDeviceLocalMemory");
        vk_testing::Device test_device(gpu());
        test_device.init(device_ci);
        m_errorMonitor->Finish();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, TilingLinear) {
    InitBestPracticesFramework(kEnableNVIDIAValidation);
    InitState();

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    image_ci.extent = { 512, 512, 1 };
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateImage-TilingLinear");
        vk_testing::Image image(*m_device, image_ci, vk_testing::no_mem);
        m_errorMonitor->Finish();
    }

    image_ci.tiling = VK_IMAGE_TILING_LINEAR;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateImage-TilingLinear");
        vk_testing::Image image(*m_device, image_ci, vk_testing::no_mem);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, Depth32Format) {
    InitBestPracticesFramework(kEnableNVIDIAValidation);
    InitState();

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    // This should be VK_FORMAT_D24_UNORM_S8_UINT, but that's not a required format.
    image_ci.format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    image_ci.extent = { 512, 512, 1 };
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateImage-Depth32Format");
        vk_testing::Image image(*m_device, image_ci, vk_testing::no_mem);
        m_errorMonitor->Finish();
    }

    VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT };

    for (VkFormat format : formats) {
        image_ci.format = format;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateImage-Depth32Format");
        vk_testing::Image image(*m_device, image_ci, vk_testing::no_mem);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, QueueBindSparse_NotAsync) {
    InitBestPracticesFramework(kEnableNVIDIAValidation);
    InitState();

    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Test requires sparseBinding";
    }

    VkDeviceQueueCreateInfo general_queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    general_queue_ci.queueFamilyIndex = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_SPARSE_BINDING_BIT, 0);
    general_queue_ci.queueCount = 1;
    general_queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkDeviceQueueCreateInfo transfer_queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    transfer_queue_ci.queueFamilyIndex = m_device->QueueFamilyMatching(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT,
                                                                       VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT);
    transfer_queue_ci.queueCount = 1;
    transfer_queue_ci.pQueuePriorities = &defaultQueuePriority;

    if (general_queue_ci.queueFamilyIndex == UINT32_MAX || transfer_queue_ci.queueFamilyIndex == UINT32_MAX) {
        GTEST_SKIP() << "Test requires a general and a transfer queue";
    }

    VkDeviceQueueCreateInfo queue_cis[2] = {
        general_queue_ci,
        transfer_queue_ci,
    };
    uint32_t queue_family_indices[] = {
        general_queue_ci.queueFamilyIndex,
        transfer_queue_ci.queueFamilyIndex,
    };

    VkPhysicalDeviceFeatures features = {};
    features.sparseBinding = VK_TRUE;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 2;
    device_ci.pQueueCreateInfos = queue_cis;
    device_ci.pEnabledFeatures = &features;

    vk_testing::Device test_device(gpu());
    test_device.init(device_ci);

    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue transfer_queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(test_device.handle(), general_queue_ci.queueFamilyIndex, 0, &graphics_queue);
    vk::GetDeviceQueue(test_device.handle(), transfer_queue_ci.queueFamilyIndex, 0, &transfer_queue);

    VkBufferCreateInfo sparse_buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    sparse_buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    sparse_buffer_ci.size = 0x10000;
    sparse_buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    sparse_buffer_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
    sparse_buffer_ci.queueFamilyIndexCount = 2;
    sparse_buffer_ci.pQueueFamilyIndices = queue_family_indices;

    vk_testing::Buffer sparse_buffer(test_device, sparse_buffer_ci, vk_testing::no_mem);

    const VkMemoryRequirements memory_requirements = sparse_buffer.memory_requirements();
    ASSERT_NE(memory_requirements.memoryTypeBits, 0);

    // Find first valid bit, whatever it is
    uint32_t memory_type_index = 0;
    while (((memory_requirements.memoryTypeBits >> memory_type_index) & 1) == 0) {
        ++memory_type_index;
    }

    VkMemoryAllocateInfo memory_ai = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_ai.allocationSize = memory_requirements.size;
    memory_ai.memoryTypeIndex = memory_type_index;

    vk_testing::DeviceMemory sparse_memory(test_device, memory_ai);

    VkSparseMemoryBind bind = {};
    bind.resourceOffset = 0;
    bind.size = sparse_buffer_ci.size;
    bind.memory = sparse_memory.handle();
    bind.memoryOffset = 0;
    bind.flags = 0;

    VkSparseBufferMemoryBindInfo sparse_buffer_bind = {};
    sparse_buffer_bind.buffer = sparse_buffer.handle();
    sparse_buffer_bind.bindCount = 1;
    sparse_buffer_bind.pBinds = &bind;

    VkBindSparseInfo bind_info = LvlInitStruct<VkBindSparseInfo>();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &sparse_buffer_bind;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-QueueBindSparse-NotAsync");
        vk::QueueBindSparse(transfer_queue, 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->Finish();
    }

    test_device.wait();

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-QueueBindSparse-NotAsync");
        vk::QueueBindSparse(graphics_queue, 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, AccelerationStructure_NotAsync) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    InitBestPracticesFramework(kEnableNVIDIAValidation);

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Vulkan >= 1.1 required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    InitState();

    VkDeviceQueueCreateInfo general_queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    general_queue_ci.queueFamilyIndex = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, 0);
    general_queue_ci.queueCount = 1;
    general_queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkDeviceQueueCreateInfo compute_queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    compute_queue_ci.queueFamilyIndex = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    compute_queue_ci.queueCount = 1;
    compute_queue_ci.pQueuePriorities = &defaultQueuePriority;

    if (general_queue_ci.queueFamilyIndex == UINT32_MAX || compute_queue_ci.queueFamilyIndex == UINT32_MAX) {
        // There's no asynchronous compute queue, skip.
        return;
    }

    VkDeviceQueueCreateInfo queue_cis[2] = {
        general_queue_ci,
        compute_queue_ci,
    };
    uint32_t queue_family_indices[] = {
        general_queue_ci.queueFamilyIndex,
        compute_queue_ci.queueFamilyIndex,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structures_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    acceleration_structures_features.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT>();
    buffer_device_address_features.pNext = &acceleration_structures_features;
    buffer_device_address_features.bufferDeviceAddress = VK_TRUE;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.pNext = &buffer_device_address_features;
    device_ci.queueCreateInfoCount = 2;
    device_ci.pQueueCreateInfos = queue_cis;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    vk_testing::Device test_device(gpu());
    test_device.init(device_ci);

    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue transfer_queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(test_device.handle(), general_queue_ci.queueFamilyIndex, 0, &graphics_queue);
    vk::GetDeviceQueue(test_device.handle(), compute_queue_ci.queueFamilyIndex, 0, &transfer_queue);

    VkBufferCreateInfo acceleration_structure_buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    acceleration_structure_buffer_ci.size = 0x10000;
    acceleration_structure_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

    VkBufferCreateInfo scratch_buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    scratch_buffer_ci.size = 1ULL << 20;
    scratch_buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    vk_testing::Buffer acceleration_structure_buffer(test_device, acceleration_structure_buffer_ci);
    vk_testing::Buffer scratch_buffer(test_device, scratch_buffer_ci);

    auto vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddress)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferDeviceAddressKHR");
    ASSERT_TRUE(vkGetBufferDeviceAddressKHR != nullptr);

    VkBufferDeviceAddressInfo scratch_buffer_device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    scratch_buffer_device_address_info.buffer = scratch_buffer.handle();
    VkDeviceAddress scratch_address = vkGetBufferDeviceAddressKHR(test_device.handle(), &scratch_buffer_device_address_info);

    VkAccelerationStructureCreateInfoKHR acceleration_structure_ci = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    acceleration_structure_ci.buffer = acceleration_structure_buffer.handle();
    acceleration_structure_ci.offset = 0;
    acceleration_structure_ci.size = acceleration_structure_buffer_ci.size;
    acceleration_structure_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    acceleration_structure_ci.deviceAddress = 0;

    vk_testing::AccelerationStructureKHR acceleration_structure(test_device, acceleration_structure_ci);

    VkAccelerationStructureGeometryTrianglesDataKHR triangle_data = LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
    triangle_data.vertexFormat = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    triangle_data.vertexData.deviceAddress = 0;
    triangle_data.vertexStride = 0;
    triangle_data.maxVertex = 3;
    triangle_data.indexType = VK_INDEX_TYPE_UINT16;
    triangle_data.indexData.deviceAddress = 0;
    triangle_data.transformData.deviceAddress = 0;

    VkAccelerationStructureGeometryKHR geometry = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles = triangle_data;
    geometry.flags = 0;

    VkAccelerationStructureBuildGeometryInfoKHR geometry_info = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    geometry_info.flags = 0;
    geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
    geometry_info.dstAccelerationStructure = acceleration_structure.handle();
    geometry_info.geometryCount = 1;
    geometry_info.pGeometries = &geometry;
    geometry_info.ppGeometries = nullptr;
    geometry_info.scratchData.deviceAddress = scratch_address;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info = {};
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 0;
    build_range_info.firstVertex = 0;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *p_build_range_info = &build_range_info;

    auto vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
        vk::GetDeviceProcAddr(test_device.handle(), "vkCmdBuildAccelerationStructuresKHR"));
    ASSERT_NE(vkCmdBuildAccelerationStructuresKHR, nullptr);

    for (uint32_t queue_family_index : queue_family_indices) {
        VkCommandPoolCreateInfo command_pool_ci = LvlInitStruct<VkCommandPoolCreateInfo>();
        command_pool_ci.queueFamilyIndex = queue_family_index;
        vk_testing::CommandPool command_pool(test_device, command_pool_ci);

        VkCommandBufferAllocateInfo command_buffer_ai = LvlInitStruct<VkCommandBufferAllocateInfo>();
        command_buffer_ai.commandPool = command_pool.handle();
        command_buffer_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_ai.commandBufferCount = 1;

        vk_testing::CommandBuffer command_buffer(test_device, command_buffer_ai);
        VkCommandBufferBeginInfo command_buffer_bi = LvlInitStruct<VkCommandBufferBeginInfo>();
        command_buffer_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        command_buffer.begin(&command_buffer_bi);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-AccelerationStructure-NotAsync");
        vkCmdBuildAccelerationStructuresKHR(command_buffer.handle(), 1, &geometry_info, &p_build_range_info);

        if (queue_family_index == compute_queue_ci.queueFamilyIndex) {
            m_errorMonitor->Finish();
        } else {
            m_errorMonitor->VerifyFound();
        }

        command_buffer.end();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, AllocateMemory_SetPriority) {
    InitBestPracticesFramework(kEnableNVIDIAValidation);
    InitState();

    VkMemoryAllocateInfo memory_ai = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_ai.allocationSize = 0x100000;
    memory_ai.memoryTypeIndex = 0;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-AllocateMemory-SetPriority");
        vk_testing::DeviceMemory memory(*m_device, memory_ai);
        m_errorMonitor->VerifyFound();
    }

    VkMemoryPriorityAllocateInfoEXT priority = LvlInitStruct<VkMemoryPriorityAllocateInfoEXT>();
    priority.priority = 0.5f;
    memory_ai.pNext = &priority;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-AllocateMemory-SetPriority");
        vk_testing::DeviceMemory memory(*m_device, memory_ai);
        m_errorMonitor->Finish();
    }
}
