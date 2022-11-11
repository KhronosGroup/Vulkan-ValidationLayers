/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
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

#include "android_ndk_types.h"
#ifdef AHB_VALIDATION_SUPPORT

TEST_F(VkLayerTest, AndroidHardwareBufferImageCreate) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image create info.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkImage img = VK_NULL_HANDLE;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // undefined format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01975");
    // Various extra errors for having VK_FORMAT_UNDEFINED without VkExternalFormatANDROID
    m_errorMonitor->SetUnexpectedError("VUID_Undefined");
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    // also undefined format
    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>();
    efa.externalFormat = 0;
    ici.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01975");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    // undefined format with an unknown external format
    efa.externalFormat = 0xBADC0DE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExternalFormatANDROID-externalFormat-01894");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    // Allocate an AHardwareBuffer
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve it's properties to make it's external format 'known' (AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM)
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(device(), ahb, &ahb_props);

    // a defined image format with a non-zero external format
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    efa.externalFormat = ahb_fmt_props.externalFormat;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01974");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.format = VK_FORMAT_UNDEFINED;

    // external format while MUTABLE
    ici.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02396");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.flags = 0;

    // external format while usage other than SAMPLED
    ici.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02397");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // external format while tiline other than OPTIMAL
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02398");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;

    // imageType
    VkExternalMemoryImageCreateInfo emici = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    ici.pNext = &emici;  // remove efa from chain, insert emici
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.imageType = VK_IMAGE_TYPE_3D;
    ici.extent = {64, 64, 64};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02393");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    // wrong mipLevels
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.extent = {64, 64, 1};
    ici.mipLevels = 6;  // should be 7
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02394");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferFetchUnboundImageInfo) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer retreive image properties while memory unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkExternalMemoryImageCreateInfo emici = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&emici);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    // attempt to fetch layout from unbound image
    VkImageSubresource sub_rsrc = {};
    sub_rsrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout sub_layout = {};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-image-01895");
    vk::GetImageSubresourceLayout(device(), image.handle(), &sub_rsrc, &sub_layout);
    m_errorMonitor->VerifyFound();

    // attempt to get memory reqs from unbound image
    VkImageMemoryRequirementsInfo2 imri = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
    imri.image = image.handle();
    VkMemoryRequirements2 mem_reqs = LvlInitStruct<VkMemoryRequirements2>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01897");
    vk::GetImageMemoryRequirements2(device(), &imri, &mem_reqs);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AndroidHardwareBufferGpuDataBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer missing USAGE_GPU_DATA_BUFFER.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    // Import requires format AHB_FMT_BLOB and usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02384");
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferAllocationSize) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct allocationSize is used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    // Allocation size mismatch
    memory_allocate_info.allocationSize = ahb_props.allocationSize + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-02383");
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory);
    m_errorMonitor->VerifyFound();

    // memoryTypeIndex mismatch
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    memory_allocate_info.memoryTypeIndex++;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferDedicatedUsageColor) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct usage for dedicated allocated color image.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkExternalFormatANDROID external_format = LvlInitStruct<VkExternalFormatANDROID>();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02390");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferDedicatedUsageDS) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct usage for dedicated allocated depth/stencil image.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_S8_UINT;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Incase it hits the below driver bug, catch the false VUID error thrown from driver not creating valid AHB
    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    if (ahb_fmt_props.format != VK_FORMAT_S8_UINT) {
        GTEST_SKIP() << "Driver bug: Didn't turn AHB format into VK_FORMAT_S8_UINT";
    }

    VkExternalFormatANDROID external_format = LvlInitStruct<VkExternalFormatANDROID>();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02390");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferMipmapChain) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct mipmap chain.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);
    if (!ahb) {
        // ERROR: AHardwareBuffer_allocate() with MIPMAP_COMPLETE fails. It returns -12, NO_MEMORY.
        // The problem seems to happen in Pixel 2, not Pixel 3.
        GTEST_SKIP() << "AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE not supported";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 2;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02389");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferImageDimensions) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer dimension and VkImage match.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 128;
    ahb_desc.height = 32;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkExternalFormatANDROID external_format = LvlInitStruct<VkExternalFormatANDROID>();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02388");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferUnknownFormat) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer uses VK_FORMAT_UNDEFINED for external.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02387");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferGpuUsage) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer has a GPU usage flag.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);
    if (!ahb) {
        GTEST_SKIP() << "Was unable to allocate an AHB";
    }

    // Everything from ahb_props is garbage and not usable
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);
    m_errorMonitor->VerifyFound();

    // Since we are creating a invalid AHB for the safe of getting the below AHB, there is a chance the driver will not be forgiving
    // and still give an usable AHB
    {
        AHardwareBuffer_Desc ahb_desc_check = {};
        AHardwareBuffer_describe(ahb, &ahb_desc_check);
        if (ahb_desc_check.usage == 0) {
            GTEST_SKIP() << "Was unable to create a valid AHB to be used";
        }
    }

    VkExternalFormatANDROID external_format = LvlInitStruct<VkExternalFormatANDROID>();
    external_format.externalFormat = 0;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);

    VkDeviceMemory memory = VK_NULL_HANDLE;

    // Dedicated allocation with missing usage bits
    // Setting up this test also triggers a slew of others
    memory_allocate_info.allocationSize = 4096;
    memory_allocate_info.memoryTypeIndex = 0;
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-pNext-02390");
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-allocationSize-02383");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02386");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferExportMemoryAllocate) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer VkExportMemoryAllocateInfo instead of import.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10) || IsPlatform(kShieldTV) || IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on this device";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkExternalFormatANDROID external_format = LvlInitStruct<VkExternalFormatANDROID>();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, ici);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    // Non-import allocation - replace import struct in chain with export struct
    VkExportMemoryAllocateInfo export_memory = LvlInitStruct<VkExportMemoryAllocateInfo>(&dedicated_allocation_info);
    export_memory.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&export_memory);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-01874");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferCreateYCbCrSampler) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer YCbCr sampler creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ycbcr_features));

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>();
    efa.externalFormat = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    sycci.format = VK_FORMAT_R8G8B8A8_UNORM;
    sycci.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    efa.externalFormat = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    // Spec says if we use VkExternalFormatANDROID value of components is ignored.
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferPhysDevImageFormatProp2) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer GetPhysicalDeviceImageFormatProperties.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageFormatProperties2 ifp = LvlInitStruct<VkImageFormatProperties2>();
    VkPhysicalDeviceImageFormatInfo2 pdifi = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
    pdifi.format = VK_FORMAT_R8G8B8A8_UNORM;
    pdifi.tiling = VK_IMAGE_TILING_OPTIMAL;
    pdifi.type = VK_IMAGE_TYPE_2D;
    pdifi.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkAndroidHardwareBufferUsageANDROID ahbu = LvlInitStruct<VkAndroidHardwareBufferUsageANDROID>();
    ahbu.androidHardwareBufferUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ifp.pNext = &ahbu;

    // AHB_usage chained to input without a matching external image format struc chained to output
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();

    // output struct chained, but does not include VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID usage
    VkPhysicalDeviceExternalImageFormatInfo pdeifi = LvlInitStruct<VkPhysicalDeviceExternalImageFormatInfo>();
    pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    pdifi.pNext = &pdeifi;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();
}

#if DISABLEUNTILAHBWORKS
TEST_F(VkLayerTest, AndroidHardwareBufferCreateImageView) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image view creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Allocate an AHB and fetch its properties
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve AHB properties to make it's external format 'known'
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(device(), ahb, &ahb_props);
    AHardwareBuffer_release(ahb);

    VkExternalMemoryImageCreateInfo emici = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    // Give image an external format
    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>(&emici);
    efa.externalFormat = ahb_fmt_props.externalFormat;

    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create another VkExternalFormatANDROID for test VUID-VkImageViewCreateInfo-image-02400
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props_Ycbcr =
        LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props_Ycbcr =
        LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props_Ycbcr);
    pfn_GetAHBProps(device(), ahb, &ahb_props_Ycbcr);
    AHardwareBuffer_release(ahb);

    VkExternalFormatANDROID efa_Ycbcr = LvlInitStruct<VkExternalFormatANDROID>();
    efa_Ycbcr.externalFormat = ahb_fmt_props_Ycbcr.externalFormat;

    // Need to make sure format has sample bit needed for image usage
    if ((ahb_fmt_props_Ycbcr.formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT feature bit not supported";
    }

    // Create the image
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&efa);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateImage(device(), &ici, NULL, &img);

    // Set up memory allocation
    VkDeviceMemory img_mem = VK_NULL_HANDLE;
    VkMemoryAllocateInfo mai = LvlInitStruct<VkMemoryAllocateInfo>();
    mai.allocationSize = 64 * 64 * 4;
    mai.memoryTypeIndex = 0;
    vk::AllocateMemory(device(), &mai, NULL, &img_mem);

    // It shouldn't use vk::GetImageMemoryRequirements for imported AndroidHardwareBuffer when memory isn't bound yet
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-04004");
    VkMemoryRequirements img_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), img, &img_mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::BindImageMemory(device(), img, img_mem, 0);

    // Bind image to memory
    vk::DestroyImage(device(), img, NULL);
    vk::FreeMemory(device(), img_mem, NULL);
    vk::CreateImage(device(), &ici, NULL, &img);
    vk::AllocateMemory(device(), &mai, NULL, &img_mem);
    vk::BindImageMemory(device(), img, img_mem, 0);

    // Create a YCbCr conversion, with different external format, chain to view
    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>(&efa_Ycbcr);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    VkSamplerYcbcrConversionInfo syci = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    syci.conversion = ycbcr_conv;

    // Create a view
    VkImageView image_view = VK_NULL_HANDLE;
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>(&syci);
    ivci.image = img;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto reset_view = [&image_view, dev]() {
        if (VK_NULL_HANDLE != image_view) vk::DestroyImageView(dev, image_view, NULL);
        image_view = VK_NULL_HANDLE;
    };

    // Up to this point, no errors expected

    // Chained ycbcr conversion has different (external) format than image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02400");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-None-02273");
    vk::CreateImageView(device(), &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, NULL);
    sycci.pNext = &efa;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    syci.conversion = ycbcr_conv;

    // View component swizzle not IDENTITY
    ivci.components.r = VK_COMPONENT_SWIZZLE_B;
    ivci.components.b = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02401");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-None-02273");
    vk::CreateImageView(device(), &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

    // View with external format, when format is not UNDEFINED
    ivci.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02399");
    // Also causes "view format different from image format"
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-01762");
    vk::CreateImageView(device(), &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, NULL);
    vk::DestroyImageView(device(), image_view, NULL);
    vk::DestroyImage(device(), img, NULL);
    vk::FreeMemory(device(), img_mem, NULL);
}
#endif  // DISABLEUNTILAHBWORKS

TEST_F(VkLayerTest, AndroidHardwareBufferImportBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer import as buffer.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDeviceMemory mem_handle = VK_NULL_HANDLE;

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // Allocate an AHardwareBuffer
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA;  // non USAGE_GPU_*
    ahb_desc.width = 512;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(device(), ahb, &ahb_props);

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    // Import as buffer requires usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881");
    // Also causes "non-dedicated allocation format/usage" error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02384");
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &mem_handle);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
    vk::FreeMemory(device(), mem_handle, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferExportBufferHandleType) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer export memory as AHB has a valid handleType.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDeviceMemory memory = VK_NULL_HANDLE;

    // Allocate device memory, no linked export struct indicating AHB handle type
    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_allocate_info.allocationSize = 65536;
    memory_allocate_info.memoryTypeIndex = 0;
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory);

    PFN_vkGetMemoryAndroidHardwareBufferANDROID pfn_GetMemAHB =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vk::GetDeviceProcAddr(device(), "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(pfn_GetMemAHB != nullptr);

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = LvlInitStruct<VkMemoryGetAndroidHardwareBufferInfoANDROID>();
    mgahbi.memory = memory;
    AHardwareBuffer *ahb = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882");
    pfn_GetMemAHB(device(), &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), memory, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferExportImageNonBound) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer export memory as AHB has image bound already.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetMemoryAndroidHardwareBufferANDROID pfn_GetMemAHB =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vk::GetDeviceProcAddr(device(), "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(pfn_GetMemAHB != nullptr);

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

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = LvlInitStruct<VkMemoryGetAndroidHardwareBufferInfoANDROID>();
    mgahbi.memory = memory;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883");
    AHardwareBuffer *ahb = nullptr;
    pfn_GetMemAHB(device(), &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), memory, NULL);
}

TEST_F(VkLayerTest, AndroidHardwareBufferInvalidBindBufferMemory) {
    TEST_DESCRIPTION("Validate binding AndroidHardwareBuffer VkBuffer act same as non-AHB buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Allocate an AHardwareBuffer
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(device(), ahb, &ahb_props);

    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>(&ext_buf_info);
    buffer_create_info.size = ahb_props.allocationSize;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    vk_testing::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);

    // Try to get memory requirements prior to binding memory
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    vk_testing::DeviceMemory memory(*m_device, memory_allocate_info);
    if (memory.handle() == VK_NULL_HANDLE) {
        AHardwareBuffer_release(ahb);
        GTEST_SKIP() << "This test failed to allocate memory for importing";
    }

    if (mem_reqs.alignment > 1) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-size-01037");
        vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), 1);
        m_errorMonitor->VerifyFound();
    }

    VkDeviceSize buffer_offset = (mem_reqs.size - 1) & ~(mem_reqs.alignment - 1);
    if (buffer_offset > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-size-01037");
        vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), buffer_offset);
        m_errorMonitor->VerifyFound();
    }

    AHardwareBuffer_release(ahb);
}

TEST_F(VkLayerTest, AndroidHardwareBufferImportBufferHandleType) {
    TEST_DESCRIPTION("Don't use proper resource handleType for import buffer");

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
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR =
        (PFN_vkBindBufferMemory2KHR)vk::GetDeviceProcAddr(device(), "vkBindBufferMemory2KHR");

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create buffer without VkExternalMemoryBufferCreateInfo
    VkBufferObj buffer;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 512;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer.init_no_mem(*m_device, buffer_create_info);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    // driver won't expose correct memoryType since resource was not created as an import operation
    // so just need any valid memory type returned from GetAHBInfo
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02986");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer.handle(), memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = LvlInitStruct<VkBindBufferMemoryInfo>();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory;
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-memory-02986");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindBufferMemoryInfo-memory-01035");
    vkBindBufferMemory2KHR(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), memory, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferImportImageHandleType) {
    TEST_DESCRIPTION("Don't use proper resource handleType for import image");

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
    PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR =
        (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");

    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create buffer without VkExternalMemoryImageCreateInfo
    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    memory_dedicated_info.image = image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    // driver won't expose correct memoryType since resource was not created as an import operation
    // so just need any valid memory type returned from GetAHBInfo
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02990");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-size-01049");
    vk::BindImageMemory(m_device->device(), image.handle(), memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_image_info.image = image.handle();
    bind_image_info.memory = memory;
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02990");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    vkBindImageMemory2KHR(m_device->device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), memory, nullptr);
}

#endif  // AHB_VALIDATION_SUPPORT