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
