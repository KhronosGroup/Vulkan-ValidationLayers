/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"
#include "utils/vk_layer_utils.h"
#include "generated/enum_flag_bits.h"

TEST_F(PositiveExternalMemorySync, GetMemoryFdHandle) {
    TEST_DESCRIPTION("Get POXIS handle for memory allocation");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
    alloc_info.allocationSize = 1024;
    alloc_info.memoryTypeIndex = 0;

    vkt::DeviceMemory memory;
    memory.init(*m_device, alloc_info);
    VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = memory;
    get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    int fd = -1;
    vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
}

TEST_F(PositiveExternalMemorySync, ExternalMemory) {
    TEST_DESCRIPTION("Perform a copy through a pair of buffers linked by external memory");

#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(ext_mem_extension_name);
    RETURN_IF_SKIP(InitFramework())
    // Check for import/export capability
    VkPhysicalDeviceExternalBufferInfoKHR ebi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR, nullptr, 0,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, handle_type};
    VkExternalBufferPropertiesKHR ebp = {VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR, nullptr, {0, 0, 0}};
    vk::GetPhysicalDeviceExternalBufferPropertiesKHR(gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External buffer does not support importing and exporting";
    }

    // Check if dedicated allocation is required
    bool dedicated_allocation =
        ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_KHR;
    if (dedicated_allocation && !IsExtensionsEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        GTEST_SKIP() << "Dedicated allocation extension not supported";
    }

    RETURN_IF_SKIP(InitState())

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    const VkExternalMemoryBufferCreateInfoKHR external_buffer_info = {VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR,
                                                                      nullptr, handle_type};
    auto buffer_info = vkt::Buffer::create_info(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.pNext = &external_buffer_info;
    vkt::Buffer buffer_export;
    buffer_export.init_no_mem(*m_device, buffer_info);
    vkt::Buffer buffer_import;
    buffer_import.init_no_mem(*m_device, buffer_info);

    // Allocation info
    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_export.memory_requirements(), mem_flags);

    // Add export allocation info to pNext chain
    VkExportMemoryAllocateInfoKHR export_info = {VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, nullptr, handle_type};
    alloc_info.pNext = &export_info;

    // Add dedicated allocation info to pNext chain if required
    VkMemoryDedicatedAllocateInfoKHR dedicated_info = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR, nullptr,
                                                       VK_NULL_HANDLE, buffer_export.handle()};
    if (dedicated_allocation) {
        export_info.pNext = &dedicated_info;
    }

    // Allocate memory to be exported
    vkt::DeviceMemory memory_export;
    memory_export.init(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.bind_memory(memory_export, 0);

#ifdef _WIN32
    // Export memory to handle
    VkMemoryGetWin32HandleInfoKHR mghi = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr, memory_export.handle(),
                                          handle_type};
    HANDLE handle;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(m_device->device(), &mghi, &handle));

    VkImportMemoryWin32HandleInfoKHR import_info = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr, handle_type,
                                                    handle};
#else
    // Export memory to fd
    VkMemoryGetFdInfoKHR mgfi = {VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR, nullptr, memory_export.handle(), handle_type};
    int fd;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(m_device->device(), &mgfi, &fd));

    VkImportMemoryFdInfoKHR import_info = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR, nullptr, handle_type, fd};
#endif

    // Import memory
    alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_import.memory_requirements(), mem_flags);
    alloc_info.pNext = &import_info;
    vkt::DeviceMemory memory_import;
    memory_import.init(*m_device, alloc_info);

    // Bind imported memory
    buffer_import.bind_memory(memory_import, 0);

    // Create test buffers and fill input buffer
    vkt::Buffer buffer_input(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto input_mem = (uint8_t *)buffer_input.memory().map();
    for (uint32_t i = 0; i < buffer_size; i++) {
        input_mem[i] = (i & 0xFF);
    }
    buffer_input.memory().unmap();
    vkt::Buffer buffer_output(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy from input buffer to output buffer through the exported/imported memory
    m_commandBuffer->begin();
    VkBufferCopy copy_info = {0, 0, buffer_size};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_input.handle(), buffer_export.handle(), 1, &copy_info);
    // Insert memory barrier to guarantee copy order
    VkMemoryBarrier mem_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
                                   VK_ACCESS_TRANSFER_READ_BIT};
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_import.handle(), buffer_output.handle(), 1, &copy_info);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveExternalMemorySync, BufferDedicatedAllocation) {
    TEST_DESCRIPTION("Create external buffer that requires dedicated allocation.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }

    auto exportable_dedicated_types = FindSupportedExternalMemoryHandleTypes(
        gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
    if (!exportable_dedicated_types) {
        GTEST_SKIP() << "Unable to find exportable handle type that requires dedicated allocation";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_dedicated_types);

    external_buffer_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&dedicated_info);
    export_memory_info.handleTypes = handle_type;

    buffer.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
}
