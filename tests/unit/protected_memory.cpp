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

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(NegativeProtectedMemory, Queue) {
    TEST_DESCRIPTION("Try creating queue without VK_QUEUE_PROTECTED_BIT capability");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceProtectedMemoryFeatures protected_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_features);

    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool unprotected_queue = false;
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_families.data());

    // need to find a queue without protected support
    for (size_t i = 0; i < queue_families.size(); i++) {
        if ((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) == 0) {
            unprotected_queue = true;
            queue_family_index = i;
            break;
        }
    }

    if (unprotected_queue == false) {
        GTEST_SKIP() << "test requires queue without VK_QUEUE_PROTECTED_BIT";
    }

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = vku::InitStructHelper();
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueCreateInfo-flags-06449");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, Submit) {
    TEST_DESCRIPTION("Setting protectedSubmit with a queue not created with VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // creates a queue without VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT
    RETURN_IF_SKIP(InitState())

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandPoolCreateInfo-flags-02860");
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-01887");

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-01890");

    // Try to find memory with protected bit in it at all
    VkDeviceMemory memory_protected = VK_NULL_HANDLE;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 4096;

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
    alloc_info.memoryTypeIndex = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        // Check just protected bit is in type at all
        if ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0) {
            alloc_info.memoryTypeIndex = i;
            break;
        }
    }
    if (alloc_info.memoryTypeIndex < phys_mem_props.memoryTypeCount) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-01872");
        vk::AllocateMemory(device(), &alloc_info, NULL, &memory_protected);
        m_errorMonitor->VerifyFound();
    }

    VkProtectedSubmitInfo protected_submit_info = vku::InitStructHelper();
    protected_submit_info.protectedSubmit = VK_TRUE;

    VkSubmitInfo submit_info = vku::InitStructHelper(&protected_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->begin();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-queue-06448");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubmitInfo-pNext-04148");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, Memory) {
    TEST_DESCRIPTION("Validate cases where protectedMemory feature is enabled and usages are invalid");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(protected_memory_features);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported";
    };

    // Turns m_commandBuffer into a protected command buffer
    RETURN_IF_SKIP(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    bool sparse_support = (m_device->phy().features().sparseBinding == VK_TRUE);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (sparse_support == true) {
        CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-None-01888");
    }

    // Create actual protected and unprotected buffers
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vkt::Buffer buffer_protected;
    buffer_protected.init_no_mem(*m_device, buffer_create_info);

    buffer_create_info.flags = 0;
    vkt::Buffer buffer_unprotected;
    buffer_unprotected.init_no_mem(*m_device, buffer_create_info);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.extent = {8, 8, 1};
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;

    if (sparse_support == true) {
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-None-01891");
    }

    // Create actual protected and unprotected images
    VkImageObj image_protected(m_device);
    VkImageObj image_unprotected(m_device);

    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    image_protected.init_no_mem(*m_device, image_create_info);
    image_create_info.flags = 0;
    image_unprotected.init_no_mem(*m_device, image_create_info);

    // Create protected and unproteced memory
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 0;

    // set allocationSize to buffer as it will be larger than the image, but query image to avoid BP warning
    VkMemoryRequirements mem_reqs_protected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_protected);
    vk::GetBufferMemoryRequirements(device(), buffer_protected.handle(), &mem_reqs_protected);
    VkMemoryRequirements mem_reqs_unprotected;
    vk::GetImageMemoryRequirements(device(), image_unprotected.handle(), &mem_reqs_unprotected);
    vk::GetBufferMemoryRequirements(device(), buffer_unprotected.handle(), &mem_reqs_unprotected);

    // Get memory index for a protected and unprotected memory
    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
    uint32_t memory_type_protected = phys_mem_props.memoryTypeCount + 1;
    uint32_t memory_type_unprotected = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        if ((mem_reqs_unprotected.memoryTypeBits & (1 << i)) &&
            ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memory_type_unprotected = i;
        }
        // Check just protected bit is in type at all
        if ((mem_reqs_protected.memoryTypeBits & (1 << i)) &&
            ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0)) {
            memory_type_protected = i;
        }
    }
    if ((memory_type_protected >= phys_mem_props.memoryTypeCount) || (memory_type_unprotected >= phys_mem_props.memoryTypeCount)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    alloc_info.memoryTypeIndex = memory_type_protected;
    alloc_info.allocationSize = mem_reqs_protected.size;
    vkt::DeviceMemory memory_protected(*m_device, alloc_info);

    alloc_info.allocationSize = mem_reqs_unprotected.size;
    alloc_info.memoryTypeIndex = memory_type_unprotected;
    vkt::DeviceMemory memory_unprotected(*m_device, alloc_info);

    // Bind protected buffer with unprotected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-None-01898");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer_protected.handle(), memory_unprotected.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind unprotected buffer with protected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-None-01899");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer_unprotected.handle(), memory_protected.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind protected image with unprotected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-None-01901");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    vk::BindImageMemory(device(), image_protected.handle(), memory_unprotected.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind unprotected image with protected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-None-01902");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    vk::BindImageMemory(device(), image_unprotected.handle(), memory_protected.handle(), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, UniqueQueueDeviceCreationBothProtected) {
    TEST_DESCRIPTION("Vulkan 1.1 unique queue detection where both are protected and same queue family");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Needed for both protected memory and vkGetDeviceQueue2
    VkPhysicalDeviceProtectedMemoryFeatures protected_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_features);

    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool protected_queue = false;
    VkQueueFamilyProperties queue_properties;  // selected queue family used
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        // need to have at least 2 queues to use
        if (((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) != 0) && (queue_families[i].queueCount > 1)) {
            protected_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (protected_queue == false) {
        GTEST_SKIP() << "test requires queue with VK_QUEUE_PROTECTED_BIT with 2 queues, not available";
    }

    float queue_priority = 1.0;

    VkDeviceQueueCreateInfo queue_create_info[2];
    queue_create_info[0] = vku::InitStructHelper();
    queue_create_info[0].flags = 0;
    queue_create_info[0].queueFamilyIndex = queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    // queueFamilyIndex is the same and both are empty flags
    queue_create_info[1] = queue_create_info[0];

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.queueCreateInfoCount = 2;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();

    // both protected
    queue_create_info[0].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info[1].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, GetDeviceQueue) {
    TEST_DESCRIPTION("General testing of vkGetDeviceQueue and general Device creation cases");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Needed for both protected memory and vkGetDeviceQueue2

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        GTEST_SKIP() << "This test should not be run on the RADV driver.";
    }

    VkDeviceQueueInfo2 queue_info_2 = vku::InitStructHelper();
    VkDevice test_device = VK_NULL_HANDLE;
    VkQueue test_queue = VK_NULL_HANDLE;
    VkResult result;

    // Use the first Physical device and queue family
    // Makes test more portable as every driver has at least 1 queue with a queueCount of 1
    uint32_t queue_family_count = 1;
    uint32_t queue_family_index = 0;
    VkQueueFamilyProperties queue_properties;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, &queue_properties);

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = vku::InitStructHelper();
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceProtectedMemoryFeatures protect_features = vku::InitStructHelper();
    protect_features.protectedMemory = VK_FALSE;  // Starting with it off

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protect_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    // Protect feature not set
    m_errorMonitor->SetUnexpectedError("VUID-VkDeviceQueueCreateInfo-flags-06449");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueCreateInfo-flags-02861");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();

    GetPhysicalDeviceFeatures2(protect_features);

    if (protect_features.protectedMemory == VK_TRUE) {
        // Might not have protected queue support
        m_errorMonitor->SetUnexpectedError("VUID-VkDeviceQueueCreateInfo-flags-06449");
        result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
        if (result != VK_SUCCESS) {
            GTEST_SKIP() << "CreateDevice returned back not VK_SUCCESS";
        }

        // Try using GetDeviceQueue with a queue that has as flag
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-flags-01841");
        vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
        m_errorMonitor->VerifyFound();

        // Test device created with flag and trying to query with no flag
        queue_info_2.flags = 0;
        queue_info_2.queueFamilyIndex = queue_family_index;
        queue_info_2.queueIndex = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-flags-06225");
        vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
        m_errorMonitor->VerifyFound();

        vk::DestroyDevice(test_device, nullptr);
        test_device = VK_NULL_HANDLE;
    }

    // Create device without protected queue
    protect_features.protectedMemory = VK_FALSE;
    queue_create_info.flags = 0;
    result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "CreateDevice returned back not VK_SUCCESS";
    }

    if (queue_properties.queueCount > 1) {
        // Set queueIndex 1 over size of queueCount used to create device
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueIndex-00385");
        vk::GetDeviceQueue(test_device, queue_family_index, 1, &test_queue);
        m_errorMonitor->VerifyFound();
    }

    // Use an unknown queue family index
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");
    vk::GetDeviceQueue(test_device, queue_family_index + 1, 0, &test_queue);
    m_errorMonitor->VerifyFound();

    queue_info_2.flags = 0;  // same as device creation
    queue_info_2.queueFamilyIndex = queue_family_index;
    queue_info_2.queueIndex = 0;

    if (queue_properties.queueCount > 1) {
        // Set queueIndex 1 over size of queueCount used to create device
        queue_info_2.queueIndex = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-queueIndex-01843");
        vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
        m_errorMonitor->VerifyFound();
        queue_info_2.queueIndex = 0;  // reset
    }

    // Use an unknown queue family index
    queue_info_2.queueFamilyIndex = queue_family_index + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-queueFamilyIndex-01842");
    vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
    m_errorMonitor->VerifyFound();
    queue_info_2.queueFamilyIndex = queue_family_index;  // reset

    // Test device created with no flags and trying to query with flag
    queue_info_2.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-flags-06225");
    vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
    m_errorMonitor->VerifyFound();
    queue_info_2.flags = 0;  // reset

    // Sanity check can still get the queue
    vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);

    vk::DestroyDevice(test_device, nullptr);
}

TEST_F(NegativeProtectedMemory, PipelineProtectedAccess) {
    TEST_DESCRIPTION("Test VUIDs from VK_EXT_pipeline_protected_access");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool pipeline_libraries = IsExtensionsEnabled(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper();
    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    if (pipeline_libraries) protected_memory_features.pNext = &gpl_features;
    VkPhysicalDevicePipelineProtectedAccessFeaturesEXT pipeline_protected_access_features =
        vku::InitStructHelper(&protected_memory_features);
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_protected_access_features);
    pipeline_protected_access_features.pipelineProtectedAccess = VK_TRUE;

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported";
    };

    RETURN_IF_SKIP(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo()};
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-07369");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineProtectedAccess-07408");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->VerifyFound();

    CreatePipelineHelper protected_pipe(*this);
    protected_pipe.InitState();
    protected_pipe.shader_stages_ = {protected_pipe.vs_->GetStageCreateInfo()};
    protected_pipe.gp_ci_.flags = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
    protected_pipe.CreateGraphicsPipeline();

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer unprotected_cmdbuf(m_device, &command_pool);
    unprotected_cmdbuf.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineProtectedAccess-07409");
    vk::CmdBindPipeline(unprotected_cmdbuf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, protected_pipe.Handle());
    m_errorMonitor->VerifyFound();

    if (pipeline_libraries) {
        CreatePipelineHelper pre_raster_lib(*this);
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        VkShaderModuleCreateInfo vs_ci = vku::InitStructHelper();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(&stage_ci);
        pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        pre_raster_lib.InitState();
        ASSERT_EQ(VK_SUCCESS, pre_raster_lib.CreateGraphicsPipeline());

        VkPipeline libraries[1] = {
            pre_raster_lib.pipeline_,
        };
        VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
        link_info.libraryCount = size(libraries);
        link_info.pLibraries = libraries;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07404");
        VkGraphicsPipelineCreateInfo lib_ci = vku::InitStructHelper(&link_info);
        lib_ci.flags = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
        vkt::Pipeline lib(*m_device, lib_ci);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07406");
        lib_ci.flags = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
        vkt::Pipeline lib2(*m_device, lib_ci);
        m_errorMonitor->VerifyFound();

        CreatePipelineHelper protected_pre_raster_lib(*this);
        protected_pre_raster_lib.InitPreRasterLibInfo(&stage_ci);
        protected_pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        protected_pre_raster_lib.InitState();
        protected_pre_raster_lib.gp_ci_.flags = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
        ASSERT_EQ(VK_SUCCESS, protected_pre_raster_lib.CreateGraphicsPipeline());
        libraries[0] = protected_pre_raster_lib.pipeline_;
        VkGraphicsPipelineCreateInfo protected_lib_ci = vku::InitStructHelper(&link_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07407");
        lib_ci.flags = 0;
        vkt::Pipeline lib3(*m_device, protected_lib_ci);
        m_errorMonitor->VerifyFound();

        CreatePipelineHelper unprotected_pre_raster_lib(*this);
        unprotected_pre_raster_lib.InitPreRasterLibInfo(&stage_ci);
        unprotected_pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        unprotected_pre_raster_lib.InitState();
        unprotected_pre_raster_lib.gp_ci_.flags = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
        ASSERT_EQ(VK_SUCCESS, unprotected_pre_raster_lib.CreateGraphicsPipeline());
        libraries[0] = unprotected_pre_raster_lib.pipeline_;
        VkGraphicsPipelineCreateInfo unprotected_lib_ci = vku::InitStructHelper(&link_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07405");
        vkt::Pipeline lib4(*m_device, unprotected_lib_ci);
        m_errorMonitor->VerifyFound();
    }

    // Create device without protected access features
    vkt::Device test_device(gpu());
    VkShaderObj vs2(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs2.InitFromGLSLTry(false, &test_device);

    const vkt::PipelineLayout test_pipeline_layout(test_device);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pipelineProtectedAccess-07368");
    CreatePipelineHelper featureless_pipe(*this);
    featureless_pipe.device_ = &test_device;
    featureless_pipe.InitState();
    featureless_pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    featureless_pipe.shader_stages_ = {vs2.GetStageCreateInfo()};
    featureless_pipe.gp_ci_.flags = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
    featureless_pipe.gp_ci_.layout = test_pipeline_layout.handle();
    featureless_pipe.gp_ci_.renderPass = render_pass.handle();
    featureless_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, UnprotectedCommands) {
    TEST_DESCRIPTION("Test making commands in unprotected command buffers that can't be used");

    // protect memory added in VK 1.1
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(protected_memory_features);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported";
    };

    // Turns m_commandBuffer into a protected command buffer
    RETURN_IF_SKIP(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    InitRenderTarget();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-commandBuffer-02711");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, 1,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();

    // Query should be outside renderpass
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-commandBuffer-01885");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-commandBuffer-01886");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndQuery-None-01923");
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeProtectedMemory, MixingProtectedResources) {
    TEST_DESCRIPTION("Test where there is mixing of protectedMemory backed resource in command buffers");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(protected_memory_features);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported";
    };

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(protected_memory_properties);

    // Turns m_commandBuffer into a unprotected command buffer without passing in a VkCommandPoolCreateFlags
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    vkt::CommandPool protectedCommandPool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_PROTECTED_BIT);
    vkt::CommandBuffer protectedCommandBuffer(m_device, &protectedCommandPool);

    // Create actual protected and unprotected buffers
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vkt::Buffer buffer_protected;
    buffer_protected.init_no_mem(*m_device, buffer_create_info);

    buffer_create_info.flags = 0;
    vkt::Buffer buffer_unprotected;
    buffer_unprotected.init_no_mem(*m_device, buffer_create_info);

    // Create actual protected and unprotected images
    const VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_protected(m_device);
    VkImageObj image_unprotected(m_device);
    VkImageObj image_protected_descriptor(m_device);
    VkImageObj image_unprotected_descriptor(m_device);
    VkImageView image_views[2];
    VkImageView image_views_descriptor[2];
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = image_format;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    image_protected.init_no_mem(*m_device, image_create_info);
    image_protected_descriptor.init_no_mem(*m_device, image_create_info);

    image_create_info.flags = 0;
    image_unprotected.init_no_mem(*m_device, image_create_info);
    image_unprotected_descriptor.init_no_mem(*m_device, image_create_info);

    // Create protected and unproteced memory
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 0;

    VkMemoryRequirements mem_reqs_buffer_protected;
    VkMemoryRequirements mem_reqs_buffer_unprotected;
    vk::GetBufferMemoryRequirements(device(), buffer_protected.handle(), &mem_reqs_buffer_protected);
    vk::GetBufferMemoryRequirements(device(), buffer_unprotected.handle(), &mem_reqs_buffer_unprotected);

    VkMemoryRequirements mem_reqs_image_protected;
    VkMemoryRequirements mem_reqs_image_unprotected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_image_protected);
    vk::GetImageMemoryRequirements(device(), image_unprotected.handle(), &mem_reqs_image_unprotected);

    m_device->phy().set_memory_type(mem_reqs_buffer_protected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    alloc_info.allocationSize = mem_reqs_buffer_protected.size;
    vkt::DeviceMemory memory_buffer_protected(*m_device, alloc_info);

    m_device->phy().set_memory_type(mem_reqs_buffer_unprotected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    VK_MEMORY_PROPERTY_PROTECTED_BIT);
    alloc_info.allocationSize = mem_reqs_buffer_unprotected.size;
    vkt::DeviceMemory memory_buffer_unprotected(*m_device, alloc_info);

    alloc_info.allocationSize = mem_reqs_image_protected.size;
    m_device->phy().set_memory_type(mem_reqs_image_protected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    vkt::DeviceMemory memory_image_protected(*m_device, alloc_info);

    m_device->phy().set_memory_type(mem_reqs_image_unprotected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    VK_MEMORY_PROPERTY_PROTECTED_BIT);
    alloc_info.allocationSize = mem_reqs_image_unprotected.size;
    vkt::DeviceMemory memory_image_unprotected(*m_device, alloc_info);

    vk::BindBufferMemory(device(), buffer_protected.handle(), memory_buffer_protected.handle(), 0);
    vk::BindBufferMemory(device(), buffer_unprotected.handle(), memory_buffer_unprotected.handle(), 0);
    vk::BindImageMemory(device(), image_protected.handle(), memory_image_protected.handle(), 0);
    vk::BindImageMemory(device(), image_unprotected.handle(), memory_image_unprotected.handle(), 0);
    vk::BindImageMemory(device(), image_protected_descriptor.handle(), memory_image_protected.handle(), 0);
    vk::BindImageMemory(device(), image_unprotected_descriptor.handle(), memory_image_unprotected.handle(), 0);

    // Change layout once memory is bound
    image_protected.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_protected_descriptor.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_unprotected.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_unprotected_descriptor.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    // need memory bound at image view creation time
    image_views[0] = image_protected.targetView(image_format);
    image_views[1] = image_unprotected.targetView(image_format);
    image_views_descriptor[0] = image_protected_descriptor.targetView(image_format);
    image_views_descriptor[1] = image_unprotected_descriptor.targetView(image_format);

    // A renderpass and framebuffer that contains a protected and unprotected image view
    VkAttachmentDescription attachments[2] = {
        {0, image_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, image_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference references[2] = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                           {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 2, references, nullptr, nullptr, 0, nullptr};
    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attachments, 1, &subpass, 1, &dependency};
    vkt::RenderPass render_pass(*m_device, render_pass_create_info);
    VkFramebufferCreateInfo framebuffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, render_pass, 2, image_views, 8, 8, 1};
    vkt::Framebuffer framebuffer(*m_device, framebuffer_create_info);

    // Various structs used for commands
    VkImageSubresourceLayers image_subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageBlit blit_region = {};
    blit_region.srcSubresource = image_subresource;
    blit_region.dstSubresource = image_subresource;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {8, 8, 1};
    blit_region.dstOffsets[0] = {0, 8, 0};
    blit_region.dstOffsets[1] = {8, 8, 1};
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkBufferCopy buffer_copy = {0, 0, 64};
    VkBufferImageCopy buffer_image_copy = {};
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = image_subresource;
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {1, 1, 1};
    buffer_image_copy.bufferOffset = 0;
    VkImageCopy image_copy = {};
    image_copy.srcSubresource = image_subresource;
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = image_subresource;
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {1, 1, 1};
    uint32_t update_data[4] = {0, 0, 0, 0};
    VkRect2D render_area = {{0, 0}, {8, 8}};
    VkRenderPassBeginInfo render_pass_begin =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, render_pass.handle(), framebuffer.handle(), render_area, 0u, nullptr);
    VkClearAttachment clear_attachments[2] = {{VK_IMAGE_ASPECT_COLOR_BIT, 0, {m_clear_color}},
                                              {VK_IMAGE_ASPECT_COLOR_BIT, 1, {m_clear_color}}};
    VkClearRect clear_rect[2] = {{render_area, 0, 1}, {render_area, 0, 1}};

    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { int x; int y; } bar;
        layout(set=0, binding=1, rgba8) uniform image2D si1;
        layout(location=0) out vec4 x;
        void main(){
           x = vec4(bar.y);
           imageStore(si1, ivec2(0), vec4(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this, 2u);
    g_pipe.gp_ci_.renderPass = render_pass.handle();
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    // Use protected resources in unprotected command buffer
    g_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_protected.handle(), 0, 1024);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_views_descriptor[0], sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    // will get undefined values, but not invalid if protectedNoFault is supported
    // Will still create an empty command buffer to test submit VUs if protectedNoFault is supported
    if (!protected_memory_properties.protectedNoFault) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-commandBuffer-01834");
        vk::CmdBlitImage(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_unprotected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-commandBuffer-01835");
        vk::CmdBlitImage(m_commandBuffer->handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-commandBuffer-01805");
        vk::CmdClearColorImage(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1,
                               &subresource_range);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-commandBuffer-01822");
        vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_protected.handle(), buffer_unprotected.handle(), 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-commandBuffer-01823");
        vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_unprotected.handle(), buffer_protected.handle(), 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-01828");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_protected.handle(), image_unprotected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-01829");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer_unprotected.handle(), image_protected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-01825");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_unprotected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-01826");
        vk::CmdCopyImage(m_commandBuffer->handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_unprotected.handle(), 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_protected.handle(), 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commandBuffer-01811");
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer_protected.handle(), 0, 4, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-commandBuffer-01813");
        vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer_protected.handle(), 0, 4, (void *)update_data);
        m_errorMonitor->VerifyFound();

        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-commandBuffer-02504");
        vk::CmdClearAttachments(m_commandBuffer->handle(), 2, clear_attachments, 2, clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &g_pipe.descriptor_set_->set_, 0, nullptr);
        VkDeviceSize offset = 0;
        vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer_protected.handle(), &offset);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer_protected.handle(), 0, VK_INDEX_TYPE_UINT16);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // color attachment
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // buffer descriptorSet
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // image descriptorSet
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // vertex
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02707");  // index

        vk::CmdDrawIndexed(m_commandBuffer->handle(), 1, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
    }
    m_commandBuffer->end();

    // Use unprotected resources in protected command buffer
    g_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_unprotected.handle(), 0, 1024);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_views_descriptor[1], sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    protectedCommandBuffer.begin();
    if (!protected_memory_properties.protectedNoFault) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-commandBuffer-01836");
        vk::CmdBlitImage(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-commandBuffer-01806");
        vk::CmdClearColorImage(protectedCommandBuffer.handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color,
                               1, &subresource_range);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-commandBuffer-01824");
        vk::CmdCopyBuffer(protectedCommandBuffer.handle(), buffer_protected.handle(), buffer_unprotected.handle(), 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-commandBuffer-01830");
        vk::CmdCopyBufferToImage(protectedCommandBuffer.handle(), buffer_protected.handle(), image_unprotected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-commandBuffer-01827");
        vk::CmdCopyImage(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833");
        vk::CmdCopyImageToBuffer(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_unprotected.handle(), 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commandBuffer-01812");
        vk::CmdFillBuffer(protectedCommandBuffer.handle(), buffer_unprotected.handle(), 0, 4, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-commandBuffer-01814");
        vk::CmdUpdateBuffer(protectedCommandBuffer.handle(), buffer_unprotected.handle(), 0, 4, (void *)update_data);
        m_errorMonitor->VerifyFound();

        vk::CmdBeginRenderPass(protectedCommandBuffer.handle(), &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-commandBuffer-02505");
        vk::CmdClearAttachments(protectedCommandBuffer.handle(), 2, clear_attachments, 2, clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdBindPipeline(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
        vk::CmdBindDescriptorSets(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  g_pipe.pipeline_layout_.handle(), 0, 1, &g_pipe.descriptor_set_->set_, 0, nullptr);
        VkDeviceSize offset = 0;
        vk::CmdBindVertexBuffers(protectedCommandBuffer.handle(), 0, 1, &buffer_unprotected.handle(), &offset);
        vk::CmdBindIndexBuffer(protectedCommandBuffer.handle(), buffer_unprotected.handle(), 0, VK_INDEX_TYPE_UINT16);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02712");  // color attachment
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-commandBuffer-02712");  // descriptorSet
        vk::CmdDrawIndexed(protectedCommandBuffer.handle(), 1, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRenderPass(protectedCommandBuffer.handle());
    }
    protectedCommandBuffer.end();

    // Try submitting together to test only 1 error occurs for the corresponding command buffer
    VkCommandBuffer comman_buffers[2] = {m_commandBuffer->handle(), protectedCommandBuffer.handle()};

    VkProtectedSubmitInfo protected_submit_info = vku::InitStructHelper();
    VkSubmitInfo submit_info = vku::InitStructHelper(&protected_submit_info);
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = comman_buffers;

    protected_submit_info.protectedSubmit = VK_TRUE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-queue-06448");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-04148");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    protected_submit_info.protectedSubmit = VK_FALSE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-04120");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // If the VkSubmitInfo::pNext chain does not include this structure, the batch is unprotected.
    submit_info.pNext = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-04120");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}
