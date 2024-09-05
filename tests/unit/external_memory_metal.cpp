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

#include "utils/vk_layer_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"

#ifdef VK_USE_PLATFORM_METAL_EXT

class NegativeExternalMemoryMetal : public VkLayerTest {};

TEST_F(NegativeExternalMemoryMetal, AllocateExportableImageWithoutDedicatedAllocationInPNext) {
    TEST_DESCRIPTION("Allocate exportable metal texture without dedicated allocation in pNext chain");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    VkExternalMemoryHandleTypeFlagBits metal_texture_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT;
    if ((exportable_types & metal_texture_type) == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal texture handle type";
    }
    external_image_info.handleTypes = metal_texture_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(nullptr);
    export_memory_info.handleTypes = metal_texture_type;

    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(),
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->SetDesiredError("VUID-UNASSIGNED-VK_EXT_external_memory-no-VkMemoryDedicatedAllocateInfo-export");
    image.memory().try_init(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemoryMetal, InvalidImportType) {
    TEST_DESCRIPTION("Import metal resource with invalid handle type");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);

    auto metal_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLBUFFER_BIT_EXT |
                                                                       VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT);
    auto valid_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(exportable_types & metal_types);
    if (valid_types == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(valid_types);
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

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type;

    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(),
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    VkResult result = buffer.memory().try_init(*m_device, alloc_info);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "vkAllocateMemory failed. Unable to continue test to check for incorrect import handle type.";
    }

    VkMemoryGetMetalHandleInfoEXT get_handle_info = vku::InitStructHelper();
    get_handle_info.handleType = handle_type;
    get_handle_info.memory = buffer.memory().handle();
    MTLResource_id import_handle = nullptr;
    vk::GetMemoryMetalHandleEXT(*m_device, &get_handle_info, &import_handle);

    VkImportMemoryMetalHandleInfoEXT metal_import_info = vku::InitStructHelper();
    metal_import_info.handleType = metal_types;
    metal_import_info.handle = import_handle;
    VkMemoryAllocateInfo import_allocate_info = vku::InitStructHelper(&metal_import_info);
    VkDeviceMemory device_memory = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VK_EXT_external_memory-incorrect-handle-type");
    // Since we are using two bits to import the memory which is not allowed, we will trigger this undefined VUID
    m_errorMonitor->SetAllowedFailureMsg("VUID_Undefined");
    vk::AllocateMemory(*m_device, &import_allocate_info, nullptr, &device_memory);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemoryMetal, AllocateImportableImageWithoutDedicatedAllocationInPNext) {
    TEST_DESCRIPTION("Import metal texture at allocation without dedicated allocation in pNext chain");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(
        gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);

    VkExternalMemoryHandleTypeFlagBits metal_texture_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT;
    if ((exportable_types & metal_texture_type) == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal texture handle type";
    }
    external_image_info.handleTypes = metal_texture_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image.handle();
    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&dedicated_info);
    export_memory_info.handleTypes = metal_texture_type;
    image.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);

    VkMemoryGetMetalHandleInfoEXT get_handle_info = vku::InitStructHelper();
    get_handle_info.handleType = metal_texture_type;
    get_handle_info.memory = image.memory().handle();
    MTLResource_id import_handle = nullptr;
    vk::GetMemoryMetalHandleEXT(*m_device, &get_handle_info, &import_handle);

    VkImportMemoryMetalHandleInfoEXT metal_import_info = vku::InitStructHelper();
    metal_import_info.handleType = metal_texture_type;
    metal_import_info.handle = import_handle;
    VkMemoryAllocateInfo import_allocate_info = vku::InitStructHelper(&metal_import_info);
    VkDeviceMemory device_memory = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-UNASSIGNED-VK_EXT_external_memory-no-VkMemoryDedicatedAllocateInfo-import");
    vk::AllocateMemory(*m_device, &import_allocate_info, nullptr, &device_memory);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemoryMetal, AllocateImportableImageWithoutDedicatedImageAllocation) {
    TEST_DESCRIPTION("Import invalid metal texture without dedicated image specified");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(
        gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);

    VkExternalMemoryHandleTypeFlagBits metal_texture_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT;
    if ((exportable_types & metal_texture_type) == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal texture handle type";
    }
    external_image_info.handleTypes = metal_texture_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image.handle();
    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&dedicated_info);
    export_memory_info.handleTypes = metal_texture_type;
    image.allocate_and_bind_memory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);

    VkMemoryGetMetalHandleInfoEXT get_handle_info = vku::InitStructHelper();
    get_handle_info.handleType = metal_texture_type;
    get_handle_info.memory = image.memory().handle();
    MTLResource_id import_handle = nullptr;
    vk::GetMemoryMetalHandleEXT(*m_device, &get_handle_info, &import_handle);

    dedicated_info.image = VK_NULL_HANDLE;  // This is what makes it ilegal
    VkImportMemoryMetalHandleInfoEXT metal_import_info = vku::InitStructHelper(&dedicated_info);
    metal_import_info.handleType = metal_texture_type;
    metal_import_info.handle = import_handle;
    VkMemoryAllocateInfo import_allocate_info = vku::InitStructHelper(&metal_import_info);
    VkDeviceMemory device_memory = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-UNASSIGNED-VK_EXT_external_memory-dedicated-null-image-import");
    vk::AllocateMemory(*m_device, &import_allocate_info, nullptr, &device_memory);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemoryMetal, GetResourceHandleWithoutExportStructAtCreation) {
    TEST_DESCRIPTION("Get resource handle that was created without the export struct at creation");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);

    auto metal_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLBUFFER_BIT_EXT |
                                                                       VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT);
    auto valid_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(exportable_types & metal_types);
    if (valid_types == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(valid_types);
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

    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(),
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dedicated_allocation ? &dedicated_info : nullptr);
    VkResult result = buffer.memory().try_init(*m_device, alloc_info);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "vkAllocateMemory failed. Unable to continue test to check for incorrect import handle type.";
    }

    MTLResource_id handle = nullptr;
    VkMemoryGetMetalHandleInfoEXT get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = buffer.memory().handle();
    get_handle_info.handleType = handle_type;
    m_errorMonitor->SetDesiredError("VUID-UNASSIGNED-VK_EXT_external_memory-memory-not-created-with-VkExportMemoryAllocateInfo");
    vk::GetMemoryMetalHandleEXT(*m_device, &get_handle_info, &handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemoryMetal, GetResourceHandleWithIncorrectHandleType) {
    TEST_DESCRIPTION("Get resource handle that was created with a different external handle type");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);

    auto metal_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLBUFFER_BIT_EXT |
                                                                       VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT);
    auto valid_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(exportable_types & metal_types);
    if (valid_types == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(valid_types);
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

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type;

    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(),
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    VkResult result = buffer.memory().try_init(*m_device, alloc_info);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "vkAllocateMemory failed. Unable to continue test to check for incorrect import handle type.";
    }

    MTLResource_id handle = nullptr;
    VkMemoryGetMetalHandleInfoEXT get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = buffer.memory().handle();
    // Get the other handle type that was not used to allocate the memory
    get_handle_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(handle_type ^ metal_types);
    m_errorMonitor->SetDesiredError("VUID-VK_EXT_external_memory-memory-allocation-missing-handle-type");
    vk::GetMemoryMetalHandleEXT(*m_device, &get_handle_info, &handle);
    m_errorMonitor->VerifyFound();
}


TEST_F(NegativeExternalMemoryMetal, GetResourceHandleWithNonMetalHandle) {
    TEST_DESCRIPTION("Get resource handle as non metal handle");

    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_METAL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);

    auto metal_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLBUFFER_BIT_EXT |
                                                                       VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT);
    auto valid_types = static_cast<VkExternalMemoryHandleTypeFlagBits>(exportable_types & metal_types);
    if (valid_types == 0u) {
        GTEST_SKIP() << "Unable to find exportable metal handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(valid_types);
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

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type;

    auto alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(),
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    VkResult result = buffer.memory().try_init(*m_device, alloc_info);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "vkAllocateMemory failed. Unable to continue test to check for incorrect import handle type.";
    }

    MTLResource_id handle = nullptr;
    VkMemoryGetMetalHandleInfoEXT get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = buffer.memory().handle();
    // Get the other handle type that was not used to allocate the memory
    get_handle_info.handleType = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(~metal_types);
    m_errorMonitor->SetDesiredError("VUID-VK_EXT_external_memory-get-handle-incorrect-handle-type");
    // We are trying to get the handle using a type that was not specified at creation, so we also get this one
    m_errorMonitor->SetAllowedFailureMsg("UID-VK_EXT_external_memory-memory-allocation-missing-handle-type");
    vk::GetMemoryMetalHandleEXT(*m_device, &get_handle_info, &handle);
    m_errorMonitor->VerifyFound();
}

#endif // VK_USE_PLATFORM_METAL_EXT
