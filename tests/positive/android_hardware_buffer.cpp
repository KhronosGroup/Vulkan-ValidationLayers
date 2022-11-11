/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
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
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include "android_ndk_types.h"
#ifdef AHB_VALIDATION_SUPPORT
TEST_F(VkPositiveLayerTest, AndroidHardwareBufferMemoryRequirements) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer doesn't conflict with memory requirements.");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // Allocate an AHardwareBuffer
    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>(&ext_buf_info);
    buffer_create_info.size = 512;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    // Should be able to bind memory with no error
    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, nullptr, &memory);
    vk::BindBufferMemory(m_device->device(), buffer.handle(), memory, 0);

    vk::FreeMemory(m_device->device(), memory, nullptr);
    AHardwareBuffer_release(ahb);
}

TEST_F(VkPositiveLayerTest, AndroidHardwareBufferDepthStencil) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer can import Depth/Stencil");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // Allocate an AHardwareBuffer
    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_D16_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Incase it hits the below driver bug, catch the false VUID error thrown from driver not creating valid AHB
    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkExternalMemoryImageCreateInfo ext_image_info = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    if (ahb_fmt_props.format != VK_FORMAT_D16_UNORM) {
        GTEST_SKIP() << "Driver bug: Didn't turn AHB format into VK_FORMAT_D16_UNORM";
    }

    // Create a Depth/Stencil image
    VkImageObj ds_image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = ahb_fmt_props.format;
    image_create_info.extent = {64, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ds_image.init_no_mem(*m_device, image_create_info);
    if (ds_image.handle() == VK_NULL_HANDLE) {
        GTEST_SKIP() << "Was not able to create a D16 AHB framebuffer";
    }

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    memory_dedicated_info.image = ds_image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, nullptr, &memory);
    vk::BindImageMemory(m_device->device(), ds_image.handle(), memory, 0);

    vk::FreeMemory(m_device->device(), memory, nullptr);
    AHardwareBuffer_release(ahb);
}

TEST_F(VkPositiveLayerTest, AndroidHardwareBufferBindBufferMemory) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer Buffers can be queried for mem requirements while unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>(&ext_buf_info);
    buffer_create_info.size = 8192;  // greater than the 4k AHB usually are
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);

    // Try to get memory requirements prior to binding memory
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);

    // Test bind memory 2 extension
    VkBufferMemoryRequirementsInfo2 buffer_mem_reqs2 = LvlInitStruct<VkBufferMemoryRequirementsInfo2>();
    buffer_mem_reqs2.buffer = buffer.handle();
    VkMemoryRequirements2 mem_reqs2 = LvlInitStruct<VkMemoryRequirements2>();
    vk::GetBufferMemoryRequirements2(m_device->device(), &buffer_mem_reqs2, &mem_reqs2);

    // Allocate an AHardwareBuffer to match the size
    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = mem_reqs.size;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Get real values
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &memory);
    vk::BindBufferMemory(m_device->device(), buffer.handle(), memory, 0);

    vk::FreeMemory(m_device->device(), memory, nullptr);
    AHardwareBuffer_release(ahb);
}

TEST_F(VkPositiveLayerTest, AndroidHardwareBufferExportBuffer) {
    TEST_DESCRIPTION("Verify VkBuffers can export to an AHB.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetMemoryAndroidHardwareBufferANDROID vkGetMemoryAndroidHardwareBufferANDROID =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vk::GetDeviceProcAddr(device(), "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(vkGetMemoryAndroidHardwareBufferANDROID != nullptr);

    // Create VkBuffer to be exported to an AHB
    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>(&ext_buf_info);
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkExportMemoryAllocateInfo export_memory_info = LvlInitStruct<VkExportMemoryAllocateInfo>();
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>(&export_memory_info);
    memory_info.allocationSize = mem_reqs.size;

    bool has_memtype = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &memory_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &memory_info, NULL, &memory);
    vk::BindBufferMemory(device(), buffer.handle(), memory, 0);

    // Export memory to AHB
    AHardwareBuffer *ahb = nullptr;

    VkMemoryGetAndroidHardwareBufferInfoANDROID get_ahb_info = LvlInitStruct<VkMemoryGetAndroidHardwareBufferInfoANDROID>();
    get_ahb_info.memory = memory;
    vkGetMemoryAndroidHardwareBufferANDROID(device(), &get_ahb_info, &ahb);

    // App in charge of releasing after exporting
    AHardwareBuffer_release(ahb);
    vk::FreeMemory(device(), memory, NULL);
}

TEST_F(VkPositiveLayerTest, AndroidHardwareBufferExportImage) {
    TEST_DESCRIPTION("Verify VkImages can export to an AHB.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetMemoryAndroidHardwareBufferANDROID vkGetMemoryAndroidHardwareBufferANDROID =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vk::GetDeviceProcAddr(device(), "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(vkGetMemoryAndroidHardwareBufferANDROID != nullptr);

    // Create VkImage to be exported to an AHB
    VkExternalMemoryImageCreateInfo ext_image_info = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    memory_dedicated_info.image = image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkExportMemoryAllocateInfo export_memory_info = LvlInitStruct<VkExportMemoryAllocateInfo>(&memory_dedicated_info);
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>(&export_memory_info);

    // "When allocating new memory for an image that can be exported to an Android hardware buffer, the memory’s allocationSize must
    // be zero":
    memory_info.allocationSize = 0;

    // Use any DEVICE_LOCAL memory found
    bool has_memtype = m_device->phy().set_memory_type(0xFFFFFFFF, &memory_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &memory_info, NULL, &memory);
    vk::BindImageMemory(device(), image.handle(), memory, 0);

    // Export memory to AHB
    AHardwareBuffer *ahb = nullptr;

    VkMemoryGetAndroidHardwareBufferInfoANDROID get_ahb_info = LvlInitStruct<VkMemoryGetAndroidHardwareBufferInfoANDROID>();
    get_ahb_info.memory = memory;
    vkGetMemoryAndroidHardwareBufferANDROID(device(), &get_ahb_info, &ahb);

    // App in charge of releasing after exporting
    AHardwareBuffer_release(ahb);
    vk::FreeMemory(device(), memory, NULL);
}

TEST_F(VkPositiveLayerTest, AndroidHardwareBufferExternalImage) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer can import AHB with external format");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // FORMAT_Y8Cb8Cr8_420 is a known/public valid AHB Format but does not have a Vulkan mapping to it
    // Will use the external image feature to get access to it
    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    int result = AHardwareBuffer_allocate(&ahb_desc, &ahb);
    if (result != 0) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    // The spec says the driver must not return zero, even if a VkFormat is returned with it, some older drivers do as a driver bug
    if (ahb_fmt_props.externalFormat == 0) {
        GTEST_SKIP() << "externalFormat was zero which is not valid";
    }

    // Create an image w/ external format
    VkExternalFormatANDROID ext_format = LvlInitStruct<VkExternalFormatANDROID>();
    ext_format.externalFormat = ahb_fmt_props.externalFormat;

    VkExternalMemoryImageCreateInfo ext_image_info = LvlInitStruct<VkExternalMemoryImageCreateInfo>(&ext_format);
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_UNDEFINED;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image.init_no_mem(*m_device, image_create_info);
    if (image.handle() == VK_NULL_HANDLE) {
        GTEST_SKIP() << "could not create image with external format";
    }

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    memory_dedicated_info.image = image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, nullptr, &memory);
    vk::BindImageMemory(m_device->device(), image.handle(), memory, 0);

    vk::FreeMemory(m_device->device(), memory, nullptr);
    AHardwareBuffer_release(ahb);
}

TEST_F(VkPositiveLayerTest, AndroidHardwareBufferExternalCameraFormat) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer can import AHB with external format");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // Simulate camera usage of AHB
    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED;
    ahb_desc.usage =
        AHARDWAREBUFFER_USAGE_CAMERA_WRITE | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    int result = AHardwareBuffer_allocate(&ahb_desc, &ahb);
    if (result != 0) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    // The spec says the driver must not return zero, even if a VkFormat is returned with it, some older drivers do as a driver bug
    if (ahb_fmt_props.externalFormat == 0) {
        GTEST_SKIP() << "externalFormat was zero which is not valid";
    }

    // Create an image w/ external format
    VkExternalFormatANDROID ext_format = LvlInitStruct<VkExternalFormatANDROID>();
    ext_format.externalFormat = ahb_fmt_props.externalFormat;

    VkExternalMemoryImageCreateInfo ext_image_info = LvlInitStruct<VkExternalMemoryImageCreateInfo>(&ext_format);
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_UNDEFINED;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image.init_no_mem(*m_device, image_create_info);
    if (image.handle() == VK_NULL_HANDLE) {
        GTEST_SKIP() << "could not create image with external format";
    }

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    memory_dedicated_info.image = image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, nullptr, &memory);
    vk::BindImageMemory(m_device->device(), image.handle(), memory, 0);

    vk::FreeMemory(m_device->device(), memory, nullptr);
    AHardwareBuffer_release(ahb);
}

#endif  // AHB_VALIDATION_SUPPORT
