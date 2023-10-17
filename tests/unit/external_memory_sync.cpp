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
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"
#include "utils/vk_layer_utils.h"

TEST_F(NegativeExternalMemorySync, CreateBufferIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Creating buffer with incompatible external memory handle types");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Try all flags first. It's unlikely all of them are compatible.
    VkExternalMemoryBufferCreateInfo external_memory_info = vku::InitStructHelper();
    external_memory_info.handleTypes = AllVkExternalMemoryHandleTypeFlagBits;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&external_memory_info);
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-pNext-00920");

    // Get all exportable handle types supported by the platform.
    VkExternalMemoryHandleTypeFlags supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlags any_compatible_group = 0;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(
        AllVkExternalMemoryHandleTypeFlagBits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalBufferInfo external_buffer_info = vku::InitStructHelper();
            external_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            external_buffer_info.handleType = flag;
            VkExternalBufferProperties external_buffer_properties = vku::InitStructHelper();
            vk::GetPhysicalDeviceExternalBufferProperties(gpu(), &external_buffer_info, &external_buffer_properties);
            const auto external_features = external_buffer_properties.externalMemoryProperties.externalMemoryFeatures;
            if (external_features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) {
                supported_handle_types |= external_buffer_info.handleType;
                any_compatible_group = external_buffer_properties.externalMemoryProperties.compatibleHandleTypes;
            }
        });

    // Main test case. Handle types are supported but not compatible with each other
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        external_memory_info.handleTypes = supported_handle_types;
        CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-pNext-00920");
    }
}

TEST_F(NegativeExternalMemorySync, CreateImageIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Creating image with incompatible external memory handle types");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Try all flags first. It's unlikely all of them are compatible.
    VkExternalMemoryImageCreateInfo external_memory_info = vku::InitStructHelper();
    external_memory_info.handleTypes = AllVkExternalMemoryHandleTypeFlagBits;
    VkImageCreateInfo image_create_info = vku::InitStructHelper(&external_memory_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-00990");

    // Get all exportable handle types supported by the platform.
    VkExternalMemoryHandleTypeFlags supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlags any_compatible_group = 0;

    VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
    image_info.format = image_create_info.format;
    image_info.type = image_create_info.imageType;
    image_info.tiling = image_create_info.tiling;
    image_info.usage = image_create_info.usage;
    image_info.flags = image_create_info.flags;

    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(
        AllVkExternalMemoryHandleTypeFlagBits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
            external_image_info.handleType = flag;
            VkExternalImageFormatProperties external_image_properties = vku::InitStructHelper();
            VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_image_properties);
            VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
            const auto external_features = external_image_properties.externalMemoryProperties.externalMemoryFeatures;
            if (result == VK_SUCCESS && (external_features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT)) {
                supported_handle_types |= external_image_info.handleType;
                any_compatible_group = external_image_properties.externalMemoryProperties.compatibleHandleTypes;
            }
        });

    // Main test case. Handle types are supported but not compatible with each other
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        external_memory_info.handleTypes = supported_handle_types;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-00990");
    }
}

TEST_F(NegativeExternalMemorySync, CreateImageIncompatibleHandleTypesNV) {
    TEST_DESCRIPTION("Creating image with incompatible external memory handle types from NVIDIA extension");

    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkExternalMemoryImageCreateInfoNV external_memory_info = vku::InitStructHelper();
    VkImageCreateInfo image_create_info = vku::InitStructHelper(&external_memory_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Get all exportable handle types supported by the platform.
    VkExternalMemoryHandleTypeFlagsNV supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlagsNV any_compatible_group = 0;

    IterateFlags<VkExternalMemoryHandleTypeFlagBitsNV>(
        AllVkExternalMemoryHandleTypeFlagBitsNV, [&](VkExternalMemoryHandleTypeFlagBitsNV flag) {
            VkExternalImageFormatPropertiesNV external_image_properties = {};
            VkResult result = vk::GetPhysicalDeviceExternalImageFormatPropertiesNV(
                gpu(), image_create_info.format, image_create_info.imageType, image_create_info.tiling, image_create_info.usage,
                image_create_info.flags, flag, &external_image_properties);
            if (result == VK_SUCCESS &&
                (external_image_properties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV)) {
                supported_handle_types |= flag;
                any_compatible_group = external_image_properties.compatibleHandleTypes;
            }
        });

    // Main test case. Handle types are supported but not compatible with each other
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        external_memory_info.handleTypes = supported_handle_types;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-00991");
    }
}

TEST_F(NegativeExternalMemorySync, ExportImageHandleType) {
    TEST_DESCRIPTION("Test exporting memory with mismatching handleTypes.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Create export image
    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (GetBitSetCount(exportable_types) < 2) {
        GTEST_SKIP() << "Cannot find two distinct exportable handle types, skipping test";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    exportable_types &= ~handle_type;
    const auto handle_type2 = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    assert(handle_type != handle_type2);

    // Create an image with one of the handle types
    external_image_info.handleTypes = handle_type;
    vkt::Image image(*m_device, image_info, vkt::NoMemT{});

    // Create export memory with a different handle type
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;
    const bool dedicated_allocation = HandleTypeNeedsDedicatedAllocation(gpu(), image_info, handle_type2);
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type2;

    // vkBindImageMemory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02728");
    image.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();

    // vkBindImageMemory2
    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = image.memory();  // re-use memory object from the previous check
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02728");
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, BufferMemoryWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Bind buffer memory with unsupported external memory handle type.");
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
    if (exportable_types == AllVkExternalMemoryHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    external_buffer_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    // Check if dedicated allocation is required
    bool dedicated_allocation = false;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(exportable_types, [&](VkExternalMemoryHandleTypeFlagBits handle_type) {
        if (HandleTypeNeedsDedicatedAllocation(gpu(), buffer_info, handle_type)) {
            dedicated_allocation = true;
        }
    });
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    // Create memory object with unsupported handle type
    const auto not_supported_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(~exportable_types);
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type | not_supported_type;

    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(),
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    VkResult result = buffer.memory().try_init(*m_device, alloc_info);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "vkAllocateMemory failed (probably due to unsupported handle type). Unable to reach vkBindBufferMemory to "
                        "run valdiation";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    buffer.bind_memory(buffer.memory(), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, BufferMemoryWithIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Bind buffer memory with incompatible external memory handle types.");
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
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(gpu(), buffer_info, handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }
    external_buffer_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    // Check if dedicated allocation is required
    bool dedicated_allocation = false;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(exportable_types, [&](VkExternalMemoryHandleTypeFlagBits handle_type) {
        if (HandleTypeNeedsDedicatedAllocation(gpu(), buffer_info, handle_type)) {
            dedicated_allocation = true;
        }
    });
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    // Create memory object with incompatible handle types
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = exportable_types;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    buffer.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImageMemoryWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Bind image memory with unsupported external memory handle type.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    // This test does not support the AHB handle type, which does not
    // allow to query memory requirements before memory is bound
    exportable_types &= ~VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalMemoryHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);

    // Create an image with supported handle type
    external_image_info.handleTypes = handle_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    // Create memory object which additionally includes unsupported handle type
    const auto not_supported_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(~exportable_types);
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;
    const bool dedicated_allocation = HandleTypeNeedsDedicatedAllocation(gpu(), image_info, handle_type);
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type | not_supported_type;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    image.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImageMemoryWithIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Bind image memory with incompatible external memory handle types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Create export image
    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    // This test does not support the AHB handle type, which does not
    // allow to query memory requirements before memory is bound
    exportable_types &= ~VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(gpu(), image_info, handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    external_image_info.handleTypes = handle_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    bool dedicated_allocation = false;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(exportable_types, [&](VkExternalMemoryHandleTypeFlagBits handle_type) {
        if (HandleTypeNeedsDedicatedAllocation(gpu(), image_info, handle_type)) {
            dedicated_allocation = true;
        }
    });
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;

    // Create memory object with incompatible handle types
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = exportable_types;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    image.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ExportBufferHandleType) {
    TEST_DESCRIPTION("Test exporting memory with mismatching handleTypes.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Create export buffer
    VkExternalMemoryBufferCreateInfo external_info = vku::InitStructHelper();
    VkBufferCreateInfo buffer_info = vku::InitStructHelper(&external_info);
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.size = 4096;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (GetBitSetCount(exportable_types) < 2) {
        GTEST_SKIP() << "Cannot find two distinct exportable handle types, skipping test";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    exportable_types &= ~handle_type;
    const auto handle_type2 = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    assert(handle_type != handle_type2);

    // Create a buffer with one of the handle types
    external_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::NoMemT{});

    // Check if dedicated allocation is required
    const bool dedicated_allocation = HandleTypeNeedsDedicatedAllocation(gpu(), buffer_info, handle_type2);
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    // Create export memory with a different handle type
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type2;
    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    const auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs,
                                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    const auto memory = vkt::DeviceMemory(*m_device, alloc_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02726");
    vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-memory-02726");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, TimelineSemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const char *no_tempory_tl_vuid = "VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322";
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const char *no_tempory_tl_vuid = "VUID-VkImportSemaphoreFdInfoKHR-flags-03323";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    // Check for external semaphore import and export capability
    {
        VkSemaphoreTypeCreateInfo sti = vku::InitStructHelper();
        sti.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        VkPhysicalDeviceExternalSemaphoreInfoKHR esi = vku::InitStructHelper(&sti);
        esi.handleType = handle_type;

        VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

        vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

        if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
            !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
            GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
        }
    }

    VkResult err;

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfoKHR esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreTypeCreateInfoKHR stci = vku::InitStructHelper(&esci);
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&stci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    stci.pNext = nullptr;
    vkt::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    err = export_semaphore.export_handle(ext_handle, handle_type);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, no_tempory_tl_vuid);
    err = import_semaphore.import_handle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR);
    m_errorMonitor->VerifyFound();

    err = import_semaphore.import_handle(ext_handle, handle_type);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(NegativeExternalMemorySync, SyncFdSemaphore) {
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = vku::InitStructHelper();
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }

    if (!(esp.compatibleHandleTypes & handle_type)) {
        GTEST_SKIP() << "External semaphore does not support VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT";
    }

    // create a timeline semaphore.
    // Note that adding a sync fd VkExportSemaphoreCreateInfo will cause creation to fail.
    VkSemaphoreTypeCreateInfoKHR stci = vku::InitStructHelper();
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&stci);

    vkt::Semaphore timeline_sem(*m_device, sci);

    // binary semaphore works fine.
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    stci.pNext = &esci;

    stci.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    vkt::Semaphore binary_sem(*m_device, sci);

    // Create a semaphore to import payload into
    vkt::Semaphore import_semaphore(*m_device);

    int fd_handle = -1;

    // timeline not allowed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-01132");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-03253");
    timeline_sem.export_handle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    // must have pending signal
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    binary_sem.export_handle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    VkSubmitInfo si = vku::InitStructHelper();
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &binary_sem.handle();

    vk::QueueSubmit(m_default_queue, 1, &si, VK_NULL_HANDLE);

    binary_sem.export_handle(fd_handle, handle_type);

    // must be temporary
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportSemaphoreFdInfoKHR-handleType-07307");
    import_semaphore.import_handle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    import_semaphore.import_handle(fd_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeExternalMemorySync, TemporaryFence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfoKHR efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfoKHR efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    // Export fence payload to an opaque handle
    ExternalHandle ext_fence{};
    export_fence.export_handle(ext_fence, handle_type);
    import_fence.import_handle(ext_fence, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT_KHR);

    // Undo the temporary import
    vk::ResetFences(m_device->device(), 1, &import_fence.handle());

    // Signal the previously imported fence twice, the second signal should produce a validation error
    vk::QueueSubmit(m_default_queue, 0, nullptr, import_fence.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-fence-00064");
    vk::QueueSubmit(m_default_queue, 0, nullptr, import_fence.handle());
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);

    // Signal without reseting
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-fence-00063");
    vk::QueueSubmit(m_default_queue, 0, nullptr, import_fence.handle());
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeExternalMemorySync, Fence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    const auto other_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const auto bad_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01452";
    const char *other_export_type_vuid = "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01448";
    const char *bad_import_type_vuid = "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01457";
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const auto other_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    const auto bad_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkFenceGetFdInfoKHR-handleType-01456";
    const char *other_export_type_vuid = "VUID-VkFenceGetFdInfoKHR-handleType-01453";
    const char *bad_import_type_vuid = "VUID-VkImportFenceFdInfoKHR-handleType-01464";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfoKHR efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfoKHR efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    ExternalHandle ext_handle{};

    // windows vs unix mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_export_type_vuid);
    export_fence.export_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();

    // a valid type for the platform which we didn't ask for during create
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, other_export_type_vuid);
    if constexpr (other_type == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
        // SYNC_FD is a special snowflake
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkFenceGetFdInfoKHR-handleType-01454");
    }
    export_fence.export_handle(ext_handle, other_type);
    m_errorMonitor->VerifyFound();

    export_fence.export_handle(ext_handle, handle_type);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_import_type_vuid);
    import_fence.import_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkImportFenceWin32HandleInfoKHR ifi = vku::InitStructHelper();
    ifi.fence = import_fence.handle();
    ifi.handleType = handle_type;
    ifi.handle = ext_handle;
    ifi.flags = 0;
    ifi.name = L"something";

    // If handleType is not VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT, name must be NULL
    // However, it looks like at least some windows drivers don't support exporting KMT handles for fences
    if constexpr (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01459");
    }
    // If handle is not NULL, name must be NULL
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportFenceWin32HandleInfoKHR-handle-01462");
    vk::ImportFenceWin32HandleKHR(m_device->device(), &ifi);
    m_errorMonitor->VerifyFound();
#endif
    auto err = import_fence.import_handle(ext_handle, handle_type);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(NegativeExternalMemorySync, SyncFdFence) {
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;

    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfoKHR efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfoKHR efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    int fd_handle = -1;

    // SYNC_FD must have a pending signal for export
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFenceGetFdInfoKHR-handleType-01454");
    export_fence.export_handle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    vk::QueueSubmit(m_default_queue, 0, nullptr, export_fence.handle());

    export_fence.export_handle(fd_handle, handle_type);

    // must be temporary
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportFenceFdInfoKHR-handleType-07306");
    import_fence.import_handle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    import_fence.import_handle(fd_handle, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT);

    import_fence.wait(1000000000);
}

TEST_F(NegativeExternalMemorySync, TemporarySemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = vku::InitStructHelper();
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfoKHR esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&esci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    vkt::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    export_semaphore.export_handle(ext_handle, handle_type);
    import_semaphore.import_handle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR);

    // Wait on the imported semaphore twice in vk::QueueSubmit, the second wait should be an error
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkSubmitInfo> si(4, vku::InitStruct<VkSubmitInfo>());
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();

    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;

    si[2] = si[0];
    si[3] = si[1];

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress");
    vk::QueueSubmit(m_default_queue, si.size(), si.data(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    auto index = m_device->graphics_queue_node_index_;
    if (m_device->phy().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        // Wait on the imported semaphore twice in vk::QueueBindSparse, the second wait should be an error
        std::vector<VkBindSparseInfo> bi(4, vku::InitStruct<VkBindSparseInfo>());
        bi[0].signalSemaphoreCount = 1;
        bi[0].pSignalSemaphores = &export_semaphore.handle();

        bi[1].waitSemaphoreCount = 1;
        bi[1].pWaitSemaphores = &import_semaphore.handle();

        bi[2] = bi[0];
        bi[3] = bi[1];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress");
        vk::QueueBindSparse(m_default_queue, bi.size(), bi.data(), VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Cleanup
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeExternalMemorySync, Semaphore) {
    TEST_DESCRIPTION("Import and export invalid external semaphores, no queue sumbits involved.");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const auto bad_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const auto other_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01131";
    const char *other_export_type_vuid = "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01126";
    const char *bad_import_type_vuid = "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01140";
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const auto bad_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const auto other_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkSemaphoreGetFdInfoKHR-handleType-01136";
    const char *other_export_type_vuid = "VUID-VkSemaphoreGetFdInfoKHR-handleType-01132";
    const char *bad_import_type_vuid = "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = vku::InitStructHelper();
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }
    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfoKHR esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&esci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore for importing
    vkt::Semaphore import_semaphore(*m_device);

    ExternalHandle ext_handle{};

    // windows vs unix mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_export_type_vuid);
    export_semaphore.export_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();

    // not specified during create
    if constexpr (other_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
        // SYNC_FD must have pending signal
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, other_export_type_vuid);
    export_semaphore.export_handle(ext_handle, other_type);
    m_errorMonitor->VerifyFound();

    export_semaphore.export_handle(ext_handle, handle_type);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_import_type_vuid);
    export_semaphore.import_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryHandleType) {
    TEST_DESCRIPTION("Validate import memory handleType for buffers and images");
    SetTargetApiVersion(VK_API_VERSION_1_1);
#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(ext_mem_extension_name);
    RETURN_IF_SKIP(InitFramework())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "External tests are not supported by MockICD, skipping tests";
    }
    RETURN_IF_SKIP(InitState())

    // Check for import/export capability
    // export used to feed memory to test import
    VkPhysicalDeviceExternalBufferInfo ebi = vku::InitStructHelper();
    ebi.handleType = handle_type;
    ebi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkExternalBufferPropertiesKHR ebp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalBufferProperties(gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External buffer does not support importing and exporting, skipping test";
    }

    // Check if dedicated allocation is required
    const bool buffer_dedicated_allocation =
        ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT;

    constexpr VkMemoryPropertyFlags mem_flags = 0;
    constexpr VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    external_buffer_info.handleTypes = handle_type;

    auto buffer_info = vkt::Buffer::create_info(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.pNext = &external_buffer_info;
    vkt::Buffer buffer_export;
    buffer_export.init_no_mem(*m_device, buffer_info);
    const VkMemoryRequirements buffer_export_reqs = buffer_export.memory_requirements();

    auto importable_buffer_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    importable_buffer_types &= ~handle_type;  // we need to find a flag that is different from handle_type
    if (importable_buffer_types == 0) GTEST_SKIP() << "Cannot find two different buffer handle types, skipping test";
    auto wrong_buffer_handle_type =
        static_cast<VkExternalMemoryHandleTypeFlagBits>(1 << MostSignificantBit(importable_buffer_types));
    external_buffer_info.handleTypes = wrong_buffer_handle_type;

    vkt::Buffer buffer_import;
    buffer_import.init_no_mem(*m_device, buffer_info);
    const VkMemoryRequirements buffer_import_reqs = buffer_import.memory_requirements();
    assert(buffer_import_reqs.memoryTypeBits != 0);  // according to spec at least one bit is set
    if ((buffer_import_reqs.memoryTypeBits & buffer_export_reqs.memoryTypeBits) == 0) {
        // required by VU 01743
        GTEST_SKIP() << "Cannot find memory type that supports both export and import";
    }

    // Allocation info
    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_export_reqs, mem_flags);

    // Add export allocation info to pNext chain
    VkExportMemoryAllocateInfoKHR export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;

    alloc_info.pNext = &export_info;

    // Add dedicated allocation info to pNext chain if required
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer_export.handle();

    if (buffer_dedicated_allocation) {
        export_info.pNext = &dedicated_info;
    }

    // Allocate memory to be exported
    vkt::DeviceMemory memory_buffer_export;
    memory_buffer_export.init(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.bind_memory(memory_buffer_export, 0);

    VkExternalMemoryImageCreateInfoKHR external_image_info = vku::InitStructHelper();
    external_image_info.handleTypes = handle_type;

    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    vkt::Image image_export(*m_device, image_info, vkt::no_mem);

    const bool image_dedicated_allocation = HandleTypeNeedsDedicatedAllocation(gpu(), image_info, handle_type);
    VkMemoryDedicatedAllocateInfo image_dedicated_info = vku::InitStructHelper();
    image_dedicated_info.image = image_export;

    auto export_memory_info =
        vku::InitStruct<VkExportMemoryAllocateInfo>(image_dedicated_allocation ? &image_dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type;
    image_export.allocate_and_bind_memory(*m_device, mem_flags, &export_memory_info);

    auto importable_image_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    importable_image_types &= ~handle_type;  // we need to find a flag that is different from handle_type
    if (importable_image_types == 0) {
        GTEST_SKIP() << "Cannot find two different image handle types";
    }
    auto wrong_image_handle_type = static_cast<VkExternalMemoryHandleTypeFlagBits>(1 << MostSignificantBit(importable_image_types));
    if (wrong_image_handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) {
        GTEST_SKIP() << "Don't want to use AHB as it has extra restrictions";
    }
    external_image_info.handleTypes = wrong_image_handle_type;

    VkImageObj image_import(m_device);
    image_import.init_no_mem(*m_device, image_info);

#ifdef _WIN32
    // Export memory to handle
    VkMemoryGetWin32HandleInfoKHR mghi_buffer = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr,
                                                 memory_buffer_export.handle(), handle_type};
    VkMemoryGetWin32HandleInfoKHR mghi_image = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr,
                                                image_export.memory().handle(), handle_type};
    HANDLE handle_buffer;
    HANDLE handle_image;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(m_device->device(), &mghi_buffer, &handle_buffer));
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(m_device->device(), &mghi_image, &handle_image));

    VkImportMemoryWin32HandleInfoKHR import_info_buffer = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr,
                                                           handle_type, handle_buffer};
    VkImportMemoryWin32HandleInfoKHR import_info_image = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr,
                                                          handle_type, handle_image};
#else
    // Export memory to fd
    VkMemoryGetFdInfoKHR mgfi_buffer = vku::InitStructHelper();
    mgfi_buffer.handleType = handle_type;
    mgfi_buffer.memory = memory_buffer_export.handle();

    VkMemoryGetFdInfoKHR mgfi_image = vku::InitStructHelper();
    mgfi_image.handleType = handle_type;
    mgfi_image.memory = image_export.memory().handle();

    int fd_buffer;
    int fd_image;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(m_device->device(), &mgfi_buffer, &fd_buffer));
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(m_device->device(), &mgfi_image, &fd_image));

    VkImportMemoryFdInfoKHR import_info_buffer = vku::InitStructHelper();
    import_info_buffer.handleType = handle_type;
    import_info_buffer.fd = fd_buffer;

    VkImportMemoryFdInfoKHR import_info_image = vku::InitStructHelper();
    import_info_image.handleType = handle_type;
    import_info_image.fd = fd_image;
#endif

    // Import memory
    alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_buffer;
    if constexpr (handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR) {
        alloc_info.allocationSize = buffer_export_reqs.size;
    }
    vkt::DeviceMemory memory_buffer_import;
    memory_buffer_import.init(*m_device, alloc_info);
    ASSERT_TRUE(memory_buffer_import.initialized());

    VkMemoryRequirements image_import_reqs = image_import.memory_requirements();
    if (image_import_reqs.memoryTypeBits == 0) {
        GTEST_SKIP() << "no suitable memory found, skipping test";
    }
    alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, image_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_image;
    if constexpr (handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR) {
        alloc_info.allocationSize = image_export.memory_requirements().size;
    }
    vkt::DeviceMemory memory_image_import;
    memory_image_import.init(*m_device, alloc_info);

    // Bind imported memory with different handleType
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02985");
    vk::BindBufferMemory(device(), buffer_import.handle(), memory_buffer_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer_import.handle();
    bind_buffer_info.memory = memory_buffer_import.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-memory-02985");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02989");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    vk::BindImageMemory(device(), image_import.handle(), memory_image_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image_import.handle();
    bind_image_info.memory = memory_image_import.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02989");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, FenceExportWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Create fence with unsupported external handle type in VkExportFenceCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto exportable_types = FindSupportedExternalFenceHandleTypes(gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalFenceHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    // Fence export with unsupported handle type
    const auto unsupported_type = LeastSignificantFlag<VkExternalFenceHandleTypeFlagBits>(~exportable_types);
    VkExportFenceCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = unsupported_type;

    const VkFenceCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkFence fence = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportFenceCreateInfo-handleTypes-01446");
    vk::CreateFence(m_device->device(), &create_info, nullptr, &fence);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, FenceExportWithIncompatibleHandleType) {
    TEST_DESCRIPTION("Create fence with incompatible external handle types in VkExportFenceCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto exportable_types = FindSupportedExternalFenceHandleTypes(gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalFenceHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(gpu(), handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    // Fence export with incompatible handle types
    VkExportFenceCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = exportable_types;

    const VkFenceCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkFence fence = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportFenceCreateInfo-handleTypes-01446");
    vk::CreateFence(m_device->device(), &create_info, nullptr, &fence);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, SemaphoreExportWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Create semaphore with unsupported external handle type in VkExportSemaphoreCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto exportable_types = FindSupportedExternalSemaphoreHandleTypes(gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalSemaphoreHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    // Semaphore export with unsupported handle type
    const auto unsupported_type = LeastSignificantFlag<VkExternalSemaphoreHandleTypeFlagBits>(~exportable_types);
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = unsupported_type;

    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkSemaphore semaphore = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportSemaphoreCreateInfo-handleTypes-01124");
    vk::CreateSemaphore(m_device->device(), &create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, SemaphoreExportWithIncompatibleHandleType) {
    TEST_DESCRIPTION("Create semaphore with incompatible external handle types in VkExportSemaphoreCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const auto exportable_types = FindSupportedExternalSemaphoreHandleTypes(gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalSemaphoreHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(gpu(), handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    // Semaphore export with incompatible handle types
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = exportable_types;

    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkSemaphore semaphore = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportSemaphoreCreateInfo-handleTypes-01124");
    vk::CreateSemaphore(m_device->device(), &create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, MemoryAndMemoryNV) {
    TEST_DESCRIPTION("Test for both external memory and external memory NV in image create pNext chain.");

    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    VkExternalMemoryImageCreateInfoNV external_mem_nv = vku::InitStructHelper();
    VkExternalMemoryImageCreateInfo external_mem = vku::InitStructHelper(&external_mem_nv);
    VkImageCreateInfo ici = vku::InitStructHelper(&external_mem);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    const auto supported_types_nv =
        FindSupportedExternalMemoryHandleTypesNV(gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV);
    const auto supported_types = FindSupportedExternalMemoryHandleTypes(gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!supported_types_nv || !supported_types) {
        GTEST_SKIP() << "Cannot find one regular handle type and one nvidia extension's handle type";
    }
    external_mem_nv.handleTypes = LeastSignificantFlag<VkExternalMemoryFeatureFlagBitsNV>(supported_types_nv);
    external_mem.handleTypes = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(supported_types);
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-00988");
}

TEST_F(NegativeExternalMemorySync, MemoryImageLayout) {
    TEST_DESCRIPTION("Validate layout of image with external memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkExternalMemoryImageCreateInfo external_mem = vku::InitStructHelper();
    VkImageCreateInfo ici = vku::InitStructHelper(&external_mem);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {32, 32, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    const auto supported_types = FindSupportedExternalMemoryHandleTypes(gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (supported_types) {
        external_mem.handleTypes = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(supported_types);
        CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-01443");
    }
    if (IsExtensionsEnabled(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME)) {
        VkExternalMemoryImageCreateInfoNV external_mem_nv = vku::InitStructHelper();
        const auto supported_types_nv =
            FindSupportedExternalMemoryHandleTypesNV(gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV);
        if (supported_types_nv) {
            external_mem_nv.handleTypes = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBitsNV>(supported_types_nv);
            ici.pNext = &external_mem_nv;
            CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-01443");
        }
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeExternalMemorySync, D3D12FenceSubmitInfo) {
    TEST_DESCRIPTION("Test invalid D3D12FenceSubmitInfo");
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    vkt::Semaphore semaphore(*m_device);

    // VkD3D12FenceSubmitInfoKHR::waitSemaphoreValuesCount == 1 is different from VkSubmitInfo::waitSemaphoreCount == 0
    {
        const uint64_t waitSemaphoreValues = 0;
        VkD3D12FenceSubmitInfoKHR d3d12_fence_submit_info = vku::InitStructHelper();
        d3d12_fence_submit_info.waitSemaphoreValuesCount = 1;
        d3d12_fence_submit_info.pWaitSemaphoreValues = &waitSemaphoreValues;
        const VkSubmitInfo submit_info = vku::InitStructHelper(&d3d12_fence_submit_info);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkD3D12FenceSubmitInfoKHR-waitSemaphoreValuesCount-00079");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
    // VkD3D12FenceSubmitInfoKHR::signalSemaphoreCount == 0 is different from VkSubmitInfo::signalSemaphoreCount == 1
    {
        VkD3D12FenceSubmitInfoKHR d3d12_fence_submit_info = vku::InitStructHelper();
        VkSubmitInfo submit_info = vku::InitStructHelper(&d3d12_fence_submit_info);
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkD3D12FenceSubmitInfoKHR-signalSemaphoreValuesCount-00080");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(NegativeExternalMemorySync, GetMemoryFdHandle) {
    TEST_DESCRIPTION("Validate VkMemoryGetFdInfoKHR passed to vkGetMemoryFdKHR");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    int fd = -1;

    // Allocate memory without VkExportMemoryAllocateInfo in the pNext chain
    {
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 32;
        alloc_info.memoryTypeIndex = 0;
        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);

        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetFdInfoKHR-handleType-00671");
        vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
        m_errorMonitor->VerifyFound();
    }
    // VkExportMemoryAllocateInfo::handleTypes does not include requested handle type
    {
        VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
        export_info.handleTypes = 0;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
        alloc_info.allocationSize = 1024;
        alloc_info.memoryTypeIndex = 0;
        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);

        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetFdInfoKHR-handleType-00671");
        vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
        m_errorMonitor->VerifyFound();
    }
    // Request handle of the wrong type
    {
        VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
        export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
        alloc_info.allocationSize = 1024;
        alloc_info.memoryTypeIndex = 0;

        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);
        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetFdInfoKHR-handleType-00672");
        vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFromFdHandle) {
    TEST_DESCRIPTION("POSIX fd handle memory import. Import parameters do not match payload's parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkExternalMemoryFeatureFlags external_features = 0;
    {
        constexpr auto required_features =
            VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR;
        VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
        external_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        external_info.handleType = handle_type;
        VkExternalBufferProperties external_properties = vku::InitStructHelper();
        vk::GetPhysicalDeviceExternalBufferProperties(gpu(), &external_info, &external_properties);
        external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
        if ((external_features & required_features) != required_features) {
            GTEST_SKIP() << "External buffer does not support both export and import, skipping test";
        }
    }

    vkt::Buffer buffer;
    {
        VkExternalMemoryBufferCreateInfo external_info = vku::InitStructHelper();
        external_info.handleTypes = handle_type;
        auto create_info = vkt::Buffer::create_info(1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        create_info.pNext = &external_info;
        buffer.init_no_mem(*m_device, create_info);
    }

    vkt::DeviceMemory memory;
    VkDeviceSize payload_size = 0;
    uint32_t payload_memory_type = 0;
    {
        const bool dedicated_allocation = (external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
        VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
        dedicated_info.buffer = buffer;
        auto export_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
        export_info.handleTypes = handle_type;
        auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(), 0, &export_info);
        memory.init(*m_device, alloc_info);
        buffer.bind_memory(memory, 0);
        payload_size = alloc_info.allocationSize;
        payload_memory_type = alloc_info.memoryTypeIndex;
    }

    int fd = -1;
    {
        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = handle_type;
        ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd));
    }
    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = handle_type;
    import_info.fd = fd;
    VkMemoryAllocateInfo alloc_info_with_import = vku::InitStructHelper(&import_info);
    VkDeviceMemory imported_memory = VK_NULL_HANDLE;

    // allocationSize != payload's allocationSize
    {
        alloc_info_with_import.allocationSize = payload_size * 2;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-01742");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // memoryTypeIndex != payload's memoryTypeIndex
    {
        alloc_info_with_import.allocationSize = payload_size;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-01742");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // Finish this test with a successful import operation in order to release the ownership of the file descriptor.
    // The alternative is to use 'close' system call.
    {
        alloc_info_with_import.allocationSize = payload_size;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type;
        vkt::DeviceMemory successfully_imported_memory;
        successfully_imported_memory.init(*m_device, alloc_info_with_import);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeExternalMemorySync, ImportMemoryFromWin32Handle) {
    TEST_DESCRIPTION("Win32 handle memory import. Import parameters do not match payload's parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkExternalMemoryFeatureFlags external_features = 0;
    {
        constexpr auto required_features =
            VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR;
        VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
        external_info.handleType = handle_type;
        VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_info);
        image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_info.type = VK_IMAGE_TYPE_2D;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
        VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_properties);
        const VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
        external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
        if (result != VK_SUCCESS || (external_features & required_features) != required_features) {
            GTEST_SKIP() << "External image does not support both export and import, skipping test";
        }
    }

    VkImageObj image(m_device);
    {
        VkExternalMemoryImageCreateInfo external_info = vku::InitStructHelper();
        external_info.handleTypes = handle_type;
        auto create_info = VkImageObj::create_info();
        create_info.pNext = &external_info;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image.init_no_mem(*m_device, create_info);
    }

    vkt::DeviceMemory memory;
    VkDeviceSize payload_size = 0;
    uint32_t payload_memory_type = 0;
    {
        const bool dedicated_allocation = (external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
        VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
        dedicated_info.image = image;
        auto export_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
        export_info.handleTypes = handle_type;
        auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), 0, &export_info);
        memory.init(*m_device, alloc_info);
        image.bind_memory(memory, 0);
        payload_size = alloc_info.allocationSize;
        payload_memory_type = alloc_info.memoryTypeIndex;
    }

    HANDLE handle = NULL;
    {
        VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = handle_type;
        ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(*m_device, &get_handle_info, &handle));
    }
    VkImportMemoryWin32HandleInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = handle_type;
    import_info.handle = handle;
    VkMemoryAllocateInfo alloc_info_with_import = vku::InitStructHelper(&import_info);
    VkDeviceMemory imported_memory = VK_NULL_HANDLE;

    // allocationSize != payload's allocationSize
    {
        alloc_info_with_import.allocationSize = payload_size * 2;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-01743");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // memoryTypeIndex != payload's memoryTypeIndex
    {
        alloc_info_with_import.allocationSize = payload_size;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-01743");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // Importing memory object payloads from Windows handles does not transfer ownership of the handle to the driver.
    // For NT handle types, the application must release handle ownership using the CloseHandle system call.
    // That's in contrast with the POSIX file descriptor handles, where memory import operation transfers the ownership,
    // so the application does not need to call 'close' system call.
    ::CloseHandle(handle);
}
#endif

TEST_F(NegativeExternalMemorySync, BufferDedicatedAllocation) {
    TEST_DESCRIPTION("Bind external buffer that requires dedicated allocation to non-dedicated memory.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, &external_buffer_info);
    const auto exportable_dedicated_types = FindSupportedExternalMemoryHandleTypes(
        gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
    if (!exportable_dedicated_types) {
        GTEST_SKIP() << "Unable to find exportable handle type that requires dedicated allocation";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_dedicated_types);

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper();
    export_memory_info.handleTypes = handle_type;
    external_buffer_info.handleTypes = handle_type;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00639");
    // pNext chain contains VkExportMemoryAllocateInfo but not VkMemoryDedicatedAllocateInfo
    vkt::Buffer buffer(*m_device, buffer_info, 0, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImageDedicatedAllocation) {
    TEST_DESCRIPTION("Bind external image that requires dedicated allocation to non-dedicated memory.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_dedicated_types = FindSupportedExternalMemoryHandleTypes(
        gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
    // This test does not support the AHB handle type, which does not
    // allow to query memory requirements before memory is bound
    exportable_dedicated_types &= ~VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    if (!exportable_dedicated_types) {
        GTEST_SKIP() << "Unable to find exportable handle type that requires dedicated allocation";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_dedicated_types);

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper();
    export_memory_info.handleTypes = handle_type;
    external_image_info.handleTypes = handle_type;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00639");
    // pNext chain contains VkExportMemoryAllocateInfo but not VkMemoryDedicatedAllocateInfo
    vkt::Image image(*m_device, image_info, 0, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeExternalMemorySync, Win32MemoryHandleProperties) {
    TEST_DESCRIPTION("Call vkGetMemoryWin32HandlePropertiesKHR with invalid Win32 handle or with opaque handle type");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    constexpr auto opaque_handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    // Generally, which value is considered invalid depends on the specific Win32 function.
    // VVL assumes that both these values do not represent a valid handle.
    constexpr HANDLE invalid_win32_handle = NULL;
    const HANDLE less_common_invalid_win32_handle = INVALID_HANDLE_VALUE;

    const HANDLE handle_that_passes_validation = (HANDLE)0x12345678;

    VkMemoryWin32HandlePropertiesKHR properties = vku::InitStructHelper();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetMemoryWin32HandlePropertiesKHR-handle-00665");
    vk::GetMemoryWin32HandlePropertiesKHR(*m_device, handle_type, invalid_win32_handle, &properties);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetMemoryWin32HandlePropertiesKHR-handle-00665");
    vk::GetMemoryWin32HandlePropertiesKHR(*m_device, handle_type, less_common_invalid_win32_handle, &properties);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetMemoryWin32HandlePropertiesKHR-handleType-00666");
    vk::GetMemoryWin32HandlePropertiesKHR(*m_device, opaque_handle_type, handle_that_passes_validation, &properties);
    m_errorMonitor->VerifyFound();
}
#endif

TEST_F(NegativeExternalMemorySync, FdMemoryHandleProperties) {
    TEST_DESCRIPTION("Call vkGetMemoryFdPropertiesKHR with invalid fd handle or with opaque handle type");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    constexpr auto opaque_handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    constexpr int invalid_fd_handle = -1;
    constexpr int valid_fd_handle = 0;

    VkMemoryFdPropertiesKHR properties = vku::InitStructHelper();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetMemoryFdPropertiesKHR-fd-00673");
    vk::GetMemoryFdPropertiesKHR(*m_device, handle_type, invalid_fd_handle, &properties);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetMemoryFdPropertiesKHR-handleType-00674");
    vk::GetMemoryFdPropertiesKHR(*m_device, opaque_handle_type, valid_fd_handle, &properties);
    m_errorMonitor->VerifyFound();
}
