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
#include "core_validation_error_enums.h"

TEST_F(VkLayerTest, InitSwapchainPotentiallyIncompatibleFlag) {
    TEST_DESCRIPTION("Initialize swapchain with potentially incompatible flags");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface, skipping test";
    }
    InitSwapchainInfo();

    auto swapchain_ci = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = m_surface;
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;

    // "protected" flag support is device defined
    {
        swapchain_ci.flags = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR;

        PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR =
            reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR>(
                vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceSurfaceCapabilities2KHR"));
        ASSERT_TRUE(vkGetPhysicalDeviceSurfaceCapabilities2KHR != nullptr);

        // Get surface protected capabilities
        auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
        surface_info.surface = swapchain_ci.surface;
        auto surface_protected_capabilities = LvlInitStruct<VkSurfaceProtectedCapabilitiesKHR>();
        auto surface_capabilities = LvlInitStruct<VkSurfaceCapabilities2KHR>();
        surface_capabilities.pNext = &surface_protected_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_capabilities);

        // Create swapchain, monitor potential validation error
        if (!surface_protected_capabilities.supportsProtected) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-flags-03187");
        }

        vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &m_swapchain);

        if (!surface_protected_capabilities.supportsProtected) {
            m_errorMonitor->VerifyFound();
            m_swapchain = VK_NULL_HANDLE;  // swapchain was not created, so prevent destroy
        } else {
            vk::DestroySwapchainKHR(device(), m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }

    // "split instance bind regions" not supported when there is only one device
    {
        swapchain_ci.flags = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-physicalDeviceCount-01429");
        vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
        m_swapchain = VK_NULL_HANDLE;  // swapchain was not created, so prevent destroy
    }
}

TEST_F(VkLayerTest, BindImageMemorySwapchain) {
    TEST_DESCRIPTION("Invalid bind image with a swapchain");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "This test appears to leave the image created a swapchain in a weird state that leads to 00378 when it "
                        "shouldn't. Requires further investigation.";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping BindSwapchainImageMemory test";
    }

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vk_testing::Image image_from_swapchain(*m_device, image_create_info, vk_testing::no_mem);

    VkMemoryRequirements mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image_from_swapchain.handle(), &mem_reqs);

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = mem_reqs.size;
    if (alloc_info.allocationSize == 0) {
        GTEST_SKIP() << "Driver seems to not be returning an valid allocation size and need to end test";
    }

    vk_testing::DeviceMemory mem;
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    // some devices don't give us good memory requirements for the swapchain image
    if (pass) {
        mem.init(*m_device, alloc_info);
        ASSERT_TRUE(mem.initialized());
    }

    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_info.image = image_from_swapchain.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-image-01630");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01632");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    auto bind_swapchain_info = LvlInitStruct<VkBindImageMemorySwapchainInfoKHR>();
    bind_swapchain_info.swapchain = VK_NULL_HANDLE;
    bind_swapchain_info.imageIndex = 0;
    bind_info.pNext = &bind_swapchain_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-RequiredParameter");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    bind_info.memory = mem.handle();
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = std::numeric_limits<uint32_t>::max();

    if (mem.initialized()) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01631");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    bind_info.memory = VK_NULL_HANDLE;
    bind_swapchain_info.imageIndex = 0;
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
}

TEST_F(VkLayerTest, ValidSwapchainImage) {
    TEST_DESCRIPTION("Swapchain images with invalid parameters");
    const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    VkImage image;
    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>(&image_swapchain_create_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImageCreateInfo good_create_info = image_create_info;

    // imageType
    image_create_info = good_create_info;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // mipLevels
    image_create_info = good_create_info;
    image_create_info.mipLevels = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // samples
    image_create_info = good_create_info;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // tiling
    image_create_info = good_create_info;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // initialLayout
    image_create_info = good_create_info;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // flags
    if (m_device->phy().features().sparseBinding) {
        image_create_info = good_create_info;
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vk::CreateImage(device(), &image_create_info, NULL, &image);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, TransferImageToSwapchainWithInvalidLayoutDeviceGroup) {
    TEST_DESCRIPTION("Transfer an image to a swapchain's image with a invalid layout between device group");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GTEST_SKIP() << "According to valid usage, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is "
                    "NULL, skipping test";
#endif

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        // Seeing the same crash as the Android comment above
        GTEST_SKIP() << "This test should not be run on the RADV driver";
    }

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0, skipping test";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    auto create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj src_Image(m_device);
    src_Image.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vk_testing::Image peer_image(*m_device, image_create_info, vk_testing::no_mem);

    auto bind_devicegroup_info = LvlInitStruct<VkBindImageMemoryDeviceGroupInfo>();
    bind_devicegroup_info.deviceIndexCount = 1;
    std::array<uint32_t, 1> deviceIndices = {{0}};
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    auto bind_swapchain_info = LvlInitStruct<VkBindImageMemorySwapchainInfoKHR>(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>(&bind_swapchain_info);
    bind_info.image = peer_image.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    m_commandBuffer->begin();

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {10, 10, 1};
    vk::CmdCopyImage(m_commandBuffer->handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_commandBuffer->end();

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Even though both peer_image and swapchain_images[0] use the same memory and are in an invalid layout,
    // only peer_image is referenced by the command buffer so there should only be one error reported.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // peer_image is a presentable image and controlled by the implementation
}

TEST_F(VkLayerTest, ValidSwapchainImageParams) {
    TEST_DESCRIPTION("Swapchain with invalid implied image creation parameters");
    const char *vuid = "VUID-VkSwapchainCreateInfoKHR-imageFormat-01778";

    AddSurfaceExtension();

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto device_group_ci = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    device_group_ci.physicalDeviceCount = 1;
    VkPhysicalDevice pdev = gpu();
    device_group_ci.pPhysicalDevices = &pdev;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &device_group_ci));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface, skipping test";
    }
    InitSwapchainInfo();

    auto good_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    good_create_info.surface = m_surface;
    good_create_info.minImageCount = m_surface_capabilities.minImageCount;
    good_create_info.imageFormat = m_surface_formats[0].format;
    good_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    good_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    good_create_info.imageArrayLayers = 1;
    good_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    good_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    good_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    good_create_info.compositeAlpha = m_surface_composite_alpha;
    good_create_info.presentMode = m_surface_non_shared_present_mode;
    good_create_info.clipped = VK_FALSE;
    good_create_info.oldSwapchain = 0;

    VkSwapchainCreateInfoKHR create_info_bad_usage = good_create_info;
    bool found_bad_usage = false;
    // Trying to find format+usage combination supported by surface, but not supported by image.
    const std::array<VkImageUsageFlags, 5> kImageUsageFlags = {{
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
    }};

    for (uint32_t i = 0; i < kImageUsageFlags.size() && !found_bad_usage; ++i) {
        if ((m_surface_capabilities.supportedUsageFlags & kImageUsageFlags[i]) != 0) {
            for (size_t j = 0; j < m_surface_formats.size(); ++j) {
                VkImageFormatProperties image_format_properties = {};
                VkResult image_format_properties_result = vk::GetPhysicalDeviceImageFormatProperties(
                    m_device->phy().handle(), m_surface_formats[j].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                    kImageUsageFlags[i], 0, &image_format_properties);

                if (image_format_properties_result != VK_SUCCESS) {
                    create_info_bad_usage.imageFormat = m_surface_formats[j].format;
                    create_info_bad_usage.imageUsage = kImageUsageFlags[i];
                    found_bad_usage = true;
                    break;
                }
            }
        }
    }
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    if (found_bad_usage) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vk::CreateSwapchainKHR(device(), &create_info_bad_usage, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
    } else {
        printf(
            "could not find imageFormat and imageUsage values, supported by "
            "surface but unsupported by image, skipping test\n");
    }
    vk::DestroySwapchainKHR(device(), m_swapchain, nullptr);

    VkImageFormatProperties props;
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(gpu(), good_create_info.imageFormat, VK_IMAGE_TYPE_2D,
                                                              VK_IMAGE_TILING_OPTIMAL, good_create_info.imageUsage,
                                                              VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, &props);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Swapchain image format does not support SPLIT_INSTANCE_BIND_REGIONS";
    }

    VkSwapchainCreateInfoKHR create_info_bad_flags = good_create_info;
    create_info_bad_flags.flags = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-physicalDeviceCount-01429");
    vk::CreateSwapchainKHR(device(), &create_info_bad_flags, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    // No valid swapchain
    m_swapchain = VK_NULL_HANDLE;
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoSync) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with VK_NULL_HANDLE semaphore and fence");

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-semaphore-01780");
        uint32_t dummy;
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, VK_NULL_HANDLE, &dummy);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoSync2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with VK_NULL_HANDLE semaphore and fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-semaphore-01782");
        auto acquire_info = LvlInitStruct<VkAcquireNextImageInfoKHR>();
        acquire_info.swapchain = m_swapchain;
        acquire_info.timeout = kWaitTimeout;
        acquire_info.semaphore = VK_NULL_HANDLE;
        acquire_info.fence = VK_NULL_HANDLE;
        acquire_info.deviceMask = 0x1;

        uint32_t dummy;
        vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoBinarySemaphore) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with non-binary semaphore");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore feature not supported.";
    }

    auto timeline_semaphore_props = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreProperties>();
    GetPhysicalDeviceProperties2(timeline_semaphore_props);
    if (timeline_semaphore_props.maxTimelineSemaphoreValueDifference == 0) {
        // If using MockICD and profiles the value might be zero'ed and cause false errors
        GTEST_SKIP() << "maxTimelineSemaphoreValueDifference is 0";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_TRUE(InitSwapchain());

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-semaphore-03265");
    uint32_t image_i;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore.handle(), VK_NULL_HANDLE, &image_i);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoBinarySemaphore2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with non-binary semaphore");

    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with VK_NULL_HANDLE semaphore and fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    if (!timeline_semaphore_features.timelineSemaphore) {
        GTEST_SKIP() << "timelineSemaphore not supported.";
    }

    auto timeline_semaphore_props = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreProperties>();
    GetPhysicalDeviceProperties2(timeline_semaphore_props);
    if (timeline_semaphore_props.maxTimelineSemaphoreValueDifference == 0) {
        // If using MockICD and profiles the value might be zero'ed and cause false errors
        GTEST_SKIP() << "maxTimelineSemaphoreValueDifference is 0";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_TRUE(InitSwapchain());

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    auto acquire_info = LvlInitStruct<VkAcquireNextImageInfoKHR>();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = kWaitTimeout;
    acquire_info.semaphore = semaphore.handle();
    acquire_info.deviceMask = 0x1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-semaphore-03266");
    uint32_t image_i;
    vk::AcquireNextImage2KHR(device(), &acquire_info, &image_i);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SwapchainAcquireTooManyImages) {
    TEST_DESCRIPTION("Acquiring invalid amount of images from the swapchain.");

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, will throw a std::bad_alloc sometimes";
    }
    ASSERT_TRUE(InitSwapchain());
    uint32_t image_count;
    ASSERT_VK_SUCCESS(vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    VkSurfaceCapabilitiesKHR caps;
    ASSERT_VK_SUCCESS(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &caps));

    const uint32_t acquirable_count = image_count - caps.minImageCount + 1;
    std::vector<VkFenceObj> fences(acquirable_count);
    for (uint32_t i = 0; i < acquirable_count; ++i) {
        fences[i].init(*m_device, VkFenceObj::create_info());
        uint32_t image_i;
        const auto res = vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fences[i].handle(), &image_i);
        ASSERT_TRUE(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
    }
    VkFenceObj error_fence;
    error_fence.init(*m_device, VkFenceObj::create_info());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-surface-07783");
    uint32_t image_i;
    // NOTE: timeout MUST be UINT64_MAX to trigger the VUID
    vk::AcquireNextImageKHR(device(), m_swapchain, UINT64_MAX, VK_NULL_HANDLE, error_fence.handle(), &image_i);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::WaitForFences(device(), fences.size(), MakeVkHandles<VkFence>(fences).data(), VK_TRUE, kWaitTimeout);
}

TEST_F(VkLayerTest, GetSwapchainImageAndTryDestroy) {
    TEST_DESCRIPTION("Try destroying a swapchain presentable image with vkDestroyImage");

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());
    uint32_t image_count;
    std::vector<VkImage> images;
    ASSERT_VK_SUCCESS(vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    images.resize(image_count, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data()));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyImage-image-04882");
    vk::DestroyImage(device(), images.at(0), nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SwapchainNotSupported) {
    TEST_DESCRIPTION("Test creating a swapchain when GetPhysicalDeviceSurfaceSupportKHR returns VK_FALSE");

    AddSurfaceExtension();

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // in "issue" section of VK_KHR_android_surface it talks how querying support is not needed on Android
    // The validation layers currently don't validate this VUID for Android surfaces
    if (std::find(instance_extensions_.begin(), instance_extensions_.end(), VK_KHR_ANDROID_SURFACE_EXTENSION_NAME) !=
        instance_extensions_.end()) {
        GTEST_SKIP() << "Test does not run on Android Surface";
    }
#endif

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    std::vector<VkQueueFamilyProperties> queue_families;
    uint32_t count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &count, nullptr);
    queue_families.resize(count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &count, queue_families.data());

    bool found = false;
    uint32_t qfi = 0;
    for (uint32_t i = 0; i < queue_families.size(); i++) {
        VkBool32 supported = VK_FALSE;
        vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), i, m_surface, &supported);
        if (!supported) {
            found = true;
            qfi = i;
            break;
        }
    }

    if (!found) {
        GTEST_SKIP() << "All queues support surface present";
    }
    float queue_priority = 1.0f;
    auto queue_create_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_create_info.queueFamilyIndex = qfi;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    auto device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    vk_testing::Device test_device(gpu());
    test_device.init(device_create_info);

    // try creating a swapchain, using surface info queried from the default device
    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-surface-01270");
    vk::CreateSwapchainKHR(test_device.handle(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SwapchainAcquireTooManyImages2KHR) {
    TEST_DESCRIPTION("Acquiring invalid amount of images from the swapchain via vkAcquireNextImage2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, will throw a std::bad_alloc sometimes";
    }
    ASSERT_TRUE(InitSwapchain());
    uint32_t image_count;
    ASSERT_VK_SUCCESS(vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    VkSurfaceCapabilitiesKHR caps;
    ASSERT_VK_SUCCESS(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &caps));

    const uint32_t acquirable_count = image_count - caps.minImageCount + 1;
    std::vector<VkFenceObj> fences(acquirable_count);
    for (uint32_t i = 0; i < acquirable_count; ++i) {
        fences[i].init(*m_device, VkFenceObj::create_info());
        uint32_t image_i;
        const auto res = vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fences[i].handle(), &image_i);
        ASSERT_TRUE(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
    }
    VkFenceObj error_fence;
    error_fence.init(*m_device, VkFenceObj::create_info());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImage2KHR-surface-07784");
    auto acquire_info = LvlInitStruct<VkAcquireNextImageInfoKHR>();

    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = UINT64_MAX; // NOTE: timeout MUST be UINT64_MAX to trigger the VUID
    acquire_info.fence = error_fence.handle();
    acquire_info.deviceMask = 0x1;

    uint32_t image_i;
    vk::AcquireNextImage2KHR(device(), &acquire_info, &image_i);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::WaitForFences(device(), fences.size(), MakeVkHandles<VkFence>(fences).data(), VK_TRUE, kWaitTimeout);
}

TEST_F(VkLayerTest, InvalidSwapchainImageFormatList) {
    TEST_DESCRIPTION("Test VK_KHR_image_format_list and VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR with swapchains");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    // To make test use, assume a common surface format
    VkSurfaceFormatKHR valid_surface_format{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR};
    VkSurfaceFormatKHR other_surface_format{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR};
    for (VkSurfaceFormatKHR surface_format : m_surface_formats) {
        if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM) {
            valid_surface_format = surface_format;
            break;
        } else {
            other_surface_format = surface_format;
        }
    }
    if (valid_surface_format.format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Test requires VK_FORMAT_B8G8R8A8_UNORM as a supported surface format";
    }

    // Use sampled formats that will always be supported
    // Last format is not compatible with the rest
    const VkFormat formats[4] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8_UNORM};
    auto format_list = LvlInitStruct<VkImageFormatListCreateInfo>();
    format_list.viewFormatCount = 3;  // first 3 are compatible
    format_list.pViewFormats = formats;

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>(&format_list);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = valid_surface_format.format;
    swapchain_create_info.imageColorSpace = valid_surface_format.colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    // No mutable flag
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-flags-04100");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;

    // Last format is not compatible
    format_list.viewFormatCount = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-pNext-04099");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    // viewFormatCount of 0
    format_list.viewFormatCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-flags-03168");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
    format_list.viewFormatCount = 3;  // valid

    // missing pNext
    swapchain_create_info.pNext = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-flags-03168");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
    swapchain_create_info.pNext = &format_list;

    // Another surface format is available and is not in list of viewFormats
    if (other_surface_format.format != VK_FORMAT_UNDEFINED) {
        swapchain_create_info.imageFormat = other_surface_format.format;
        swapchain_create_info.imageColorSpace = other_surface_format.colorSpace;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-flags-03168");
        vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
        swapchain_create_info.imageFormat = valid_surface_format.format;
        swapchain_create_info.imageColorSpace = valid_surface_format.colorSpace;
    }

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkLayerTest, SwapchainMinImageCountNonShared) {
    TEST_DESCRIPTION("Use invalid minImageCount for non shared swapchain creation");
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();
    if (m_surface_capabilities.minImageCount <= 1) {
        GTEST_SKIP() << "minImageCount is not at least 2";
    }

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 1;  // invalid
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-minImageCount-01271");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    // Sanity check
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkLayerTest, SwapchainMinImageCountShared) {
    TEST_DESCRIPTION("Use invalid minImageCount for shared swapchain creation");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkPresentModeKHR shared_present_mode = m_surface_non_shared_present_mode;
    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        const VkPresentModeKHR present_mode = m_surface_present_modes[i];
        if ((present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) ||
            (present_mode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
            shared_present_mode = present_mode;
            break;
        }
    }
    if (shared_present_mode == m_surface_non_shared_present_mode) {
        GTEST_SKIP() << "Cannot find supported shared present mode";
    }

    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vk::GetInstanceProcAddr(instance(),
                                                                                "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceSurfaceCapabilities2KHR != nullptr);

    auto shared_present_capabilities = LvlInitStruct<VkSharedPresentSurfaceCapabilitiesKHR>();
    auto capabilities = LvlInitStruct<VkSurfaceCapabilities2KHR>(&shared_present_capabilities);
    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
    surface_info.surface = m_surface;
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &capabilities);

    // This was recently added to CTS, but some drivers might not correctly advertise the flag
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Driver was suppose to support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    }

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 2;  // invalid
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-minImageCount-01383");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    // Sanity check
    swapchain_create_info.minImageCount = 1;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
}

TEST_F(VkLayerTest, SwapchainInvalidUsageNonShared) {
    TEST_DESCRIPTION("Use invalid imageUsage for non-shared swapchain creation");
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    // No implementation should support depth/stencil for swapchain
    if ((m_surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
        GTEST_SKIP() << "Test has supported usage already the test is using";
    }

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-presentMode-01427");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SwapchainInvalidUsageShared) {
    TEST_DESCRIPTION("Use invalid imageUsage for shared swapchain creation");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkPresentModeKHR shared_present_mode = m_surface_non_shared_present_mode;
    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        const VkPresentModeKHR present_mode = m_surface_present_modes[i];
        if ((present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) ||
            (present_mode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
            shared_present_mode = present_mode;
            break;
        }
    }
    if (shared_present_mode == m_surface_non_shared_present_mode) {
        GTEST_SKIP() << "Cannot find supported shared present mode";
    }

    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vk::GetInstanceProcAddr(instance(),
                                                                                "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceSurfaceCapabilities2KHR != nullptr);

    auto shared_present_capabilities = LvlInitStruct<VkSharedPresentSurfaceCapabilitiesKHR>();
    auto capabilities = LvlInitStruct<VkSurfaceCapabilities2KHR>(&shared_present_capabilities);
    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
    surface_info.surface = m_surface;
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &capabilities);

    // No implementation should support depth/stencil for swapchain
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
        GTEST_SKIP() << "Test has supported usage already the test is using";
    }

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = 1;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-imageUsage-01384");
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDeviceMask) {
    TEST_DESCRIPTION("Invalid deviceMask.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    auto create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    // Test VkMemoryAllocateFlagsInfo
    auto alloc_flags_info = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
    alloc_flags_info.deviceMask = 0xFFFFFFFF;
    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags_info);
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = 1024;

    VkDeviceMemory mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
    vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    alloc_flags_info.deviceMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    uint32_t pdev_group_count = 0;
    VkResult err = vk::EnumeratePhysicalDeviceGroups(instance(), &pdev_group_count, nullptr);
    // TODO: initialization can be removed once https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4138 merges
    std::vector<VkPhysicalDeviceGroupProperties> group_props(pdev_group_count,
                                                             {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    err = vk::EnumeratePhysicalDeviceGroups(instance(), &pdev_group_count, &group_props[0]);

    auto tgt = gpu();
    bool test_run = false;
    for (uint32_t i = 0; i < pdev_group_count; i++) {
        if ((group_props[i].physicalDeviceCount > 1) && !test_run) {
            for (uint32_t j = 0; j < group_props[i].physicalDeviceCount; j++) {
                if (tgt == group_props[i].physicalDevices[j]) {
                    void *data;
                    VkDeviceMemory mi_mem;
                    alloc_flags_info.deviceMask = 3;
                    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mi_mem);
                    if (VK_SUCCESS == err) {
                        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-memory-00683");
                        vk::MapMemory(m_device->device(), mi_mem, 0, 1024, 0, &data);
                        m_errorMonitor->VerifyFound();
                        vk::FreeMemory(m_device->device(), mi_mem, nullptr);
                    }
                    test_run = true;
                    break;
                }
            }
        }
    }

    // Test VkDeviceGroupCommandBufferBeginInfo
    auto dev_grp_cmd_buf_info = LvlInitStruct<VkDeviceGroupCommandBufferBeginInfo>();
    dev_grp_cmd_buf_info.deviceMask = 0xFFFFFFFF;
    auto cmd_buf_info = LvlInitStruct<VkCommandBufferBeginInfo>(&dev_grp_cmd_buf_info);

    m_commandBuffer->reset();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    dev_grp_cmd_buf_info.deviceMask = 0;
    m_commandBuffer->reset();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupRenderPassBeginInfo
    dev_grp_cmd_buf_info.deviceMask = 0x00000001;
    m_commandBuffer->reset();
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);

    auto dev_grp_rp_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    dev_grp_rp_info.deviceMask = 0xFFFFFFFF;
    m_renderPassBeginInfo.pNext = &dev_grp_rp_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00906");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0x00000001;
    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group[0].physicalDeviceCount + 1;
    std::vector<VkRect2D> device_render_areas(dev_grp_rp_info.deviceRenderAreaCount, m_renderPassBeginInfo.renderArea);
    dev_grp_rp_info.pDeviceRenderAreas = device_render_areas.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceRenderAreaCount-00908");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    // Test vk::CmdSetDeviceMask()
    vk::CmdSetDeviceMask(m_commandBuffer->handle(), 0x00000001);

    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group[0].physicalDeviceCount;
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    vk::CmdSetDeviceMask(m_commandBuffer->handle(), 0xFFFFFFFF);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    vk::CmdSetDeviceMask(m_commandBuffer->handle(), 0);
    m_errorMonitor->VerifyFound();

    vk_testing::Semaphore semaphore(*m_device), semaphore2(*m_device);
    vk_testing::Fence fence(*m_device);

    // Test VkAcquireNextImageInfoKHR
    uint32_t imageIndex;
    auto acquire_next_image_info = LvlInitStruct<VkAcquireNextImageInfoKHR>();
    acquire_next_image_info.semaphore = semaphore.handle();
    acquire_next_image_info.swapchain = m_swapchain;
    acquire_next_image_info.fence = fence.handle();
    acquire_next_image_info.deviceMask = 0xFFFFFFFF;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-deviceMask-01290");
    vk::AcquireNextImage2KHR(m_device->device(), &acquire_next_image_info, &imageIndex);
    m_errorMonitor->VerifyFound();

    //NOTE: We cannot wait on fence in this test because all of the acquire calls fail.

    acquire_next_image_info.semaphore = semaphore2.handle();
    acquire_next_image_info.deviceMask = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-deviceMask-01291");
    vk::AcquireNextImage2KHR(m_device->device(), &acquire_next_image_info, &imageIndex);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupSubmitInfo
    auto device_group_submit_info = LvlInitStruct<VkDeviceGroupSubmitInfo>();
    device_group_submit_info.commandBufferCount = 1;
    std::array<uint32_t, 1> command_buffer_device_masks = {{0xFFFFFFFF}};
    device_group_submit_info.pCommandBufferDeviceMasks = command_buffer_device_masks.data();

    auto submit_info = LvlInitStruct<VkSubmitInfo>(&device_group_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->reset();
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, DisplayPlaneSurface) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects to test VkDisplaySurfaceCreateInfoKHR.");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (!InitSurface()) {
        GTEST_SKIP() << "Failed to create surface.  Skipping.";
    }

    // Load all VK_KHR_display functions
    PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR =
        (PFN_vkCreateDisplayModeKHR)vk::GetInstanceProcAddr(instance(), "vkCreateDisplayModeKHR");
    PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR =
        (PFN_vkCreateDisplayPlaneSurfaceKHR)vk::GetInstanceProcAddr(instance(), "vkCreateDisplayPlaneSurfaceKHR");
    PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR =
        (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vk::GetInstanceProcAddr(instance(), "vkGetDisplayPlaneSupportedDisplaysKHR");
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vk::GetInstanceProcAddr(instance(),
                                                                                  "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR =
        (PFN_vkGetDisplayModePropertiesKHR)vk::GetInstanceProcAddr(instance(), "vkGetDisplayModePropertiesKHR");
    PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR =
        (PFN_vkGetDisplayPlaneCapabilitiesKHR)vk::GetInstanceProcAddr(instance(), "vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_TRUE(vkCreateDisplayModeKHR != nullptr);
    ASSERT_TRUE(vkCreateDisplayPlaneSurfaceKHR != nullptr);
    ASSERT_TRUE(vkGetDisplayPlaneSupportedDisplaysKHR != nullptr);
    ASSERT_TRUE(vkGetPhysicalDeviceDisplayPlanePropertiesKHR != nullptr);
    ASSERT_TRUE(vkGetDisplayModePropertiesKHR != nullptr);
    ASSERT_TRUE(vkGetDisplayPlaneCapabilitiesKHR != nullptr);

    uint32_t plane_prop_count = 0;
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &plane_prop_count, nullptr);
    if (plane_prop_count == 0) {
        GTEST_SKIP() << "Test requires at least 1 supported display plane property";
    }
    std::vector<VkDisplayPlanePropertiesKHR> display_plane_props(plane_prop_count);
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(gpu(), &plane_prop_count, display_plane_props.data());
    // using plane 0 for rest of test
    VkDisplayKHR current_display = display_plane_props[0].currentDisplay;
    if (current_display == VK_NULL_HANDLE) {
        GTEST_SKIP() << "VkDisplayPlanePropertiesKHR[0].currentDisplay is not attached to device";
    }

    uint32_t mode_prop_count = 0;
    vkGetDisplayModePropertiesKHR(gpu(), current_display, &mode_prop_count, nullptr);
    if (plane_prop_count == 0) {
        GTEST_SKIP() << "test requires at least 1 supported display mode property";
    }
    std::vector<VkDisplayModePropertiesKHR> display_mode_props(mode_prop_count);
    vkGetDisplayModePropertiesKHR(gpu(), current_display, &mode_prop_count, display_mode_props.data());

    uint32_t plane_count;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDisplayPlaneSupportedDisplaysKHR-planeIndex-01249");
    vkGetDisplayPlaneSupportedDisplaysKHR(gpu(), plane_prop_count, &plane_count, nullptr);
    m_errorMonitor->VerifyFound();
    ASSERT_VK_SUCCESS(vkGetDisplayPlaneSupportedDisplaysKHR(gpu(), 0, &plane_count, nullptr));
    if (plane_count == 0) {
        GTEST_SKIP() << "test requires at least 1 supported display plane";
    }
    std::vector<VkDisplayKHR> supported_displays(plane_count);
    plane_count = 1;
    ASSERT_VK_SUCCESS(vkGetDisplayPlaneSupportedDisplaysKHR(gpu(), 0, &plane_count, supported_displays.data()));
    if (supported_displays[0] != current_display) {
        GTEST_SKIP() << "Current VkDisplayKHR used is not supported";
    }

    VkDisplayModeKHR display_mode;
    VkDisplayModeParametersKHR display_mode_parameters = {{0, 0}, 0};
    VkDisplayModeCreateInfoKHR display_mode_info = {VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR, nullptr, 0,
                                                    display_mode_parameters};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplayModeParametersKHR-width-01990");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplayModeParametersKHR-height-01991");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplayModeParametersKHR-refreshRate-01992");
    vkCreateDisplayModeKHR(gpu(), current_display, &display_mode_info, nullptr, &display_mode);
    m_errorMonitor->VerifyFound();
    // Use the first good parameter queried
    display_mode_info.parameters = display_mode_props[0].parameters;
    VkResult result = vkCreateDisplayModeKHR(gpu(), current_display, &display_mode_info, nullptr, &display_mode);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "test failed to create a display mode with vkCreateDisplayModeKHR";
    }

    VkDisplayPlaneCapabilitiesKHR plane_capabilities;
    ASSERT_VK_SUCCESS(vkGetDisplayPlaneCapabilitiesKHR(gpu(), display_mode, 0, &plane_capabilities));

    VkSurfaceKHR surface;
    auto display_surface_info = LvlInitStruct<VkDisplaySurfaceCreateInfoKHR>();
    display_surface_info.flags = 0;
    display_surface_info.displayMode = display_mode;
    display_surface_info.planeIndex = 0;
    display_surface_info.planeStackIndex = 0;
    display_surface_info.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    display_surface_info.imageExtent = {8, 8};
    display_surface_info.globalAlpha = 1.0f;

    // Test if the device doesn't support the bits
    if ((plane_capabilities.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR) == 0) {
        display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255");
        vkCreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
        m_errorMonitor->VerifyFound();
    }
    if ((plane_capabilities.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR) == 0) {
        display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255");
        vkCreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
        m_errorMonitor->VerifyFound();
    }

    display_surface_info.globalAlpha = 2.0f;
    display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR;
    if ((plane_capabilities.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR) == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01254");
    vkCreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();

    display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
    display_surface_info.planeIndex = plane_prop_count;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-planeIndex-01252");
    vkCreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();
    display_surface_info.planeIndex = 0;  // restore to good value

    uint32_t bad_size = m_device->phy().properties().limits.maxImageDimension2D + 1;
    display_surface_info.imageExtent = {bad_size, bad_size};
    // one for height and width
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-width-01256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplaySurfaceCreateInfoKHR-width-01256");
    vkCreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, WarningSwapchainCreateInfoPreTransform) {
    TEST_DESCRIPTION("Print warning when preTransform doesn't match curretTransform");

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-CoreValidation-SwapchainPreTransform");
    m_errorMonitor->SetUnexpectedError("VUID-VkSwapchainCreateInfoKHR-preTransform-01279");
    InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceGroupSubmitInfoSemaphoreCount) {
    TEST_DESCRIPTION("Test semaphoreCounts in DeviceGroupSubmitInfo");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Vulkan >= 1.1 required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0, skipping test";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    auto create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    auto dev_grp_cmd_buf_info = LvlInitStruct<VkDeviceGroupCommandBufferBeginInfo>();
    dev_grp_cmd_buf_info.deviceMask = 0x1;
    auto cmd_buf_info = LvlInitStruct<VkCommandBufferBeginInfo>(&dev_grp_cmd_buf_info);

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);
    ASSERT_TRUE(semaphore.initialized());

    auto device_group_submit_info = LvlInitStruct<VkDeviceGroupSubmitInfo>();
    device_group_submit_info.commandBufferCount = 1;
    uint32_t command_buffer_device_masks = 0;
    device_group_submit_info.pCommandBufferDeviceMasks = &command_buffer_device_masks;

    auto submit_info = LvlInitStruct<VkSubmitInfo>(&device_group_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();

    m_commandBuffer->reset();
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-signalSemaphoreCount-00084");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    auto signal_submit_info = LvlInitStruct<VkSubmitInfo>();
    signal_submit_info.signalSemaphoreCount = 1;
    signal_submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &signal_submit_info, VK_NULL_HANDLE);

    VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submit_info.pWaitDstStageMask = &waitMask;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-waitSemaphoreCount-00082");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info.waitSemaphoreCount = 0;
    submit_info.commandBufferCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-commandBufferCount-00083");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Need to wait for semaphore to not be in use before destroying it
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, SwapchainAcquireImageWithSignaledSemaphore) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with signaled semaphore");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    vk_testing::Semaphore semaphore(*m_device);

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    auto acquire_info = LvlInitStruct<VkAcquireNextImageInfoKHR>();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = kWaitTimeout;
    acquire_info.semaphore = semaphore.handle();
    acquire_info.fence = VK_NULL_HANDLE;
    acquire_info.deviceMask = 0x1;

    uint32_t dummy;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-semaphore-01286");
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore.handle(), VK_NULL_HANDLE, &dummy);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-semaphore-01288");
    vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DisplayPresentInfoSrcRect) {
    TEST_DESCRIPTION("Test layout tracking on imageless framebuffers");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping test";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t current_buffer;
    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore image_acquired(*m_device, semaphore_create_info);
    ASSERT_TRUE(image_acquired.initialized());
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, image_acquired.handle(), VK_NULL_HANDLE, &current_buffer);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    uint32_t swapchain_width = m_surface_capabilities.minImageExtent.width;
    uint32_t swapchain_height = m_surface_capabilities.minImageExtent.height;

    auto display_present_info = LvlInitStruct<VkDisplayPresentInfoKHR>();
    display_present_info.srcRect.extent.width = swapchain_width + 1;  // Invalid
    display_present_info.srcRect.extent.height = swapchain_height;
    display_present_info.dstRect.extent.width = swapchain_width;
    display_present_info.dstRect.extent.height = swapchain_height;

    auto present = LvlInitStruct<VkPresentInfoKHR>(&display_present_info);
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &image_acquired.handle();
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &current_buffer;
    present.swapchainCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDisplayPresentInfoKHR-srcRect-01257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentInfoKHR-pImageIndices-01296");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, LeakASwapchain) {
    TEST_DESCRIPTION("Leak a VkSwapchainKHR.");

    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!IsPlatform(kMockICD)) {
        // This test leaks a swapchain (on purpose) and should not be run on a real driver
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    ASSERT_TRUE(InitSwapchain());

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    ASSERT_TRUE(InitSurface(surface));
    ASSERT_TRUE(InitSwapchain(surface, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, swapchain));

    // Warn about the surface/swapchain not being destroyed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyInstance-instance-00629");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyDevice-device-00378");
    ShutdownFramework();  // Destroy Instance/Device
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PresentIdWait) {
    TEST_DESCRIPTION("Test present wait extension");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto present_id_features = LvlInitStruct<VkPhysicalDevicePresentIdFeaturesKHR>();
    auto present_wait_features = LvlInitStruct<VkPhysicalDevicePresentWaitFeaturesKHR>(&present_id_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&present_wait_features);
    AddRequiredExtensions(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_ID_EXTENSION_NAME);
    AddSurfaceExtension();
    const bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        GTEST_SKIP() << "Error initializing extensions or retrieving features, skipping test.";
    }
    if (!present_id_features.presentId || !present_wait_features.presentWait) {
        GTEST_SKIP() << "presentWait feature is not available, skipping test.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required, skipping test.";
    }

    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create swapchain, skipping test";
    }

    VkSurfaceKHR surface2;
    VkSwapchainKHR swapchain2;
    InitSurface(surface2);
    InitSwapchain(surface2, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, swapchain2);

    auto vkWaitForPresentKHR = (PFN_vkWaitForPresentKHR)vk::GetDeviceProcAddr(m_device->device(), "vkWaitForPresentKHR");
    ASSERT_TRUE(vkWaitForPresentKHR != nullptr);

    uint32_t image_count, image_count2;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr);
    vk::GetSwapchainImagesKHR(device(), swapchain2, &image_count2, nullptr);
    std::vector<VkImage> images(image_count);
    std::vector<VkImage> images2(image_count2);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data());
    vk::GetSwapchainImagesKHR(device(), swapchain2, &image_count2, images2.data());

    uint32_t image_indices[2];
    VkFenceObj fence, fence2;
    fence.init(*m_device, VkFenceObj::create_info());
    fence2.init(*m_device, VkFenceObj::create_info());
    VkFence fence_handles[2];
    fence_handles[0] = fence.handle();
    fence_handles[1] = fence2.handle();

    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence_handles[0], &image_indices[0]);
    vk::AcquireNextImageKHR(device(), swapchain2, kWaitTimeout, VK_NULL_HANDLE, fence_handles[1], &image_indices[1]);
    vk::WaitForFences(device(), 2, fence_handles, true, kWaitTimeout);
    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_indices[0]], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images2[image_indices[1]], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkSwapchainKHR swap_chains[2] = {m_swapchain, swapchain2};
    uint64_t present_ids[2] = {};
    present_ids[0] = 4;  // Try setting 3 later
    auto present_id = LvlInitStruct<VkPresentIdKHR>();
    present_id.swapchainCount = 2;
    present_id.pPresentIds = present_ids;
    auto present = LvlInitStruct<VkPresentInfoKHR>(&present_id);
    present.pSwapchains = swap_chains;
    present.pImageIndices = image_indices;
    present.swapchainCount = 2;

    // Submit a clean present to establish presentIds
    vk::QueuePresentKHR(m_device->m_queue, &present);

    vk::ResetFences(device(), 2, fence_handles);
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence_handles[0], &image_indices[0]);
    vk::AcquireNextImageKHR(device(), swapchain2, kWaitTimeout, VK_NULL_HANDLE, fence_handles[1], &image_indices[1]);
    vk::WaitForFences(device(), 2, fence_handles, true, kWaitTimeout);
    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_indices[0]], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images2[image_indices[1]], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // presentIds[0] = 3 (smaller than 4), presentIds[1] = 5 (wait for this after swapchain 2 is retired)
    present_ids[0] = 3;
    present_ids[1] = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentIdKHR-presentIds-04999");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    // Errors should prevent previous and future vkQueuePresents from actually happening so ok to re-use images
    present_id.swapchainCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentIdKHR-swapchainCount-arraylength");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    present_id.swapchainCount = 1;
    present_ids[0] = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentIdKHR-swapchainCount-04998");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    VkSwapchainKHR swapchain3;
    // Retire swapchain2
    InitSwapchain(surface2, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, swapchain3, swapchain2);
    present_id.swapchainCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWaitForPresentKHR-swapchain-04997");
    vkWaitForPresentKHR(device(), swapchain2, 5, kWaitTimeout);
    m_errorMonitor->VerifyFound();

    vk::DestroySwapchainKHR(m_device->device(), swapchain2, nullptr);
    vk::DestroySwapchainKHR(m_device->device(), swapchain3, nullptr);
    vk::DestroySurfaceKHR(instance(), surface2, nullptr);
}

TEST_F(VkLayerTest, PresentIdWaitFeatures) {
    TEST_DESCRIPTION("Test present wait extension");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_ID_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>();
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        GTEST_SKIP() << "Error initializing extensions or retrieving features";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required, skipping test.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create swapchain, skipping test";
    }

    auto vkWaitForPresentKHR = (PFN_vkWaitForPresentKHR)vk::GetDeviceProcAddr(m_device->device(), "vkWaitForPresentKHR");
    assert(vkWaitForPresentKHR != nullptr);

    uint32_t image_count;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data());

    uint32_t image_index;
    VkFenceObj fence;
    fence.init(*m_device, VkFenceObj::create_info());
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, VK_NULL_HANDLE, fence.handle(), &image_index);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    uint64_t present_id_index = 1;
    auto present_id = LvlInitStruct<VkPresentIdKHR>();
    present_id.swapchainCount = 1;
    present_id.pPresentIds = &present_id_index;

    auto present = LvlInitStruct<VkPresentInfoKHR>(&present_id);
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    present.swapchainCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentInfoKHR-pNext-06235");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWaitForPresentKHR-presentWait-06234");
    vkWaitForPresentKHR(device(), m_swapchain, 1, kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GetSwapchainImagesCountButNotImages) {
    TEST_DESCRIPTION("Test for getting swapchain images count and presenting before getting swapchain images.");
    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSurface());

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present, skipping test";
    }
    InitSwapchainInfo();

    VkImageFormatProperties img_format_props;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), m_surface_formats[0].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &img_format_props);
    VkExtent2D img_ext = {std::min(m_surface_capabilities.maxImageExtent.width, img_format_props.maxExtent.width),
                          std::min(m_surface_capabilities.maxImageExtent.height, img_format_props.maxExtent.height)};

    auto swapchain_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_info.surface = m_surface;
    swapchain_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_info.imageFormat = m_surface_formats[0].format;
    swapchain_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_info.imageExtent = img_ext;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount = 0;
    swapchain_info.pQueueFamilyIndices = nullptr;
    swapchain_info.preTransform = m_surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_info.presentMode = m_surface_present_modes[0];
    swapchain_info.clipped = VK_FALSE;

    vk::CreateSwapchainKHR(device(), &swapchain_info, nullptr, &m_swapchain);

    uint32_t imageCount;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &imageCount, nullptr);

    const uint32_t image_index = 0;
    auto present_info = LvlInitStruct<VkPresentInfoKHR>();
    present_info.pImageIndices = &image_index;
    present_info.pSwapchains = &m_swapchain;
    present_info.swapchainCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentInfoKHR-pImageIndices-01296");
    vk::QueuePresentKHR(m_device->m_queue, &present_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestSurfaceSupportByPhysicalDevice) {
    TEST_DESCRIPTION("Test if physical device supports surface.");
    AddOptionalExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    AddOptionalExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
#endif
    AddOptionalExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    const bool swapchain = IsExtensionsEnabled(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    const bool get_surface_capabilities2 = IsExtensionsEnabled(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    const bool display_surface_counter = IsExtensionsEnabled(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const bool full_screen_exclusive = IsExtensionsEnabled(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
#endif
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }

    uint32_t queueFamilyPropertyCount;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queueFamilyPropertyCount, nullptr);

    VkBool32 supported = VK_FALSE;
    for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i) {
        vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), i, m_surface, &supported);
        if (supported) {
            break;
        }
    }
    if (supported) {
        GTEST_SKIP() << "Physical device supports present";
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (full_screen_exclusive) {
        auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
        surface_info.surface = m_surface;
        VkDeviceGroupPresentModeFlagsKHR flags = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;

        PFN_vkGetDeviceGroupSurfacePresentModes2EXT vkGetDeviceGroupSurfacePresentModes2EXT =
            reinterpret_cast<PFN_vkGetDeviceGroupSurfacePresentModes2EXT>(
                vk::GetInstanceProcAddr(instance(), "vkGetDeviceGroupSurfacePresentModes2EXT"));

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-06213");
        vkGetDeviceGroupSurfacePresentModes2EXT(device(), &surface_info, &flags);
        m_errorMonitor->VerifyFound();

        PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT vkGetPhysicalDeviceSurfacePresentModes2EXT =
            (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)vk::GetInstanceProcAddr(instance(),
                                                                                    "vkGetPhysicalDeviceSurfacePresentModes2EXT");

        uint32_t count;
        vkGetPhysicalDeviceSurfacePresentModes2EXT(gpu(), &surface_info, &count, nullptr);
    }
#endif

    if (swapchain) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceGroupSurfacePresentModesKHR-surface-06212");
        VkDeviceGroupPresentModeFlagsKHR flags = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;
        vk::GetDeviceGroupSurfacePresentModesKHR(device(), m_surface, &flags);
        m_errorMonitor->VerifyFound();

        uint32_t count;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDevicePresentRectanglesKHR-surface-06211");
        vk::GetPhysicalDevicePresentRectanglesKHR(gpu(), m_surface, &count, nullptr);
        m_errorMonitor->VerifyFound();
    }

    if (display_surface_counter) {
        PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT vkGetPhysicalDeviceSurfaceCapabilities2EXT =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vk::GetInstanceProcAddr(instance(),
                                                                                    "vkGetPhysicalDeviceSurfaceCapabilities2EXT");

        auto capabilities = LvlInitStruct<VkSurfaceCapabilities2EXT>();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-surface-06211");
        vkGetPhysicalDeviceSurfaceCapabilities2EXT(gpu(), m_surface, &capabilities);
        m_errorMonitor->VerifyFound();
    }

    if (get_surface_capabilities2) {
        PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vk::GetInstanceProcAddr(instance(),
                                                                                    "vkGetPhysicalDeviceSurfaceCapabilities2KHR");

        auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
        surface_info.surface = m_surface;
        auto capabilities = LvlInitStruct<VkSurfaceCapabilities2KHR>();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06210");
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &capabilities);
        m_errorMonitor->VerifyFound();
    }

    {
        VkSurfaceCapabilitiesKHR capabilities;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-surface-06211");
        vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &capabilities);
        m_errorMonitor->VerifyFound();
    }

    if (get_surface_capabilities2) {
        PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR =
            (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceSurfaceFormats2KHR");

        auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
        surface_info.surface = m_surface;
        uint32_t count;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06210");
        vkGetPhysicalDeviceSurfaceFormats2KHR(gpu(), &surface_info, &count, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        uint32_t count;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06211");
        vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu(), m_surface, &count, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        uint32_t count;
        vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), m_surface, &count, nullptr);
    }
}

TEST_F(VkLayerTest, SwapchainMaintenance1ExtensionTestsAcquire) {
    TEST_DESCRIPTION("Test swapchain Maintenance1 extensions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
    InitSwapchainInfo();

    uint32_t count;

    VkSurfaceKHR surface = m_surface;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    // Query present mode data
    auto vkGetPhysicalDeviceSurfaceCapabilities2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceSurfaceCapabilities2KHR"));
    const std::array defined_present_modes{
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
        VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
    };

    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), m_surface, &count, nullptr);
    std::vector<VkPresentModeKHR> pdev_surface_present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), m_surface, &count, pdev_surface_present_modes.data());

    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
    auto surface_caps = LvlInitStruct<VkSurfaceCapabilities2KHR>();
    surface_info.surface = m_surface;

    // Set a present_mode in VkSurfacePresentModeEXT that's NOT returned by GetPhsyicalDeviceSurfaceCapabilities2KHR
    VkPresentModeKHR mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (auto item : defined_present_modes) {
        if (std::find(pdev_surface_present_modes.begin(), pdev_surface_present_modes.end(), item) ==
            pdev_surface_present_modes.end()) {
            mismatched_present_mode = item;
            break;
        }
    }

    auto present_mode = LvlInitStruct<VkSurfacePresentModeEXT>();
    present_mode.presentMode = mismatched_present_mode;

    surface_info.pNext = &present_mode;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSurfacePresentModeEXT-presentMode-07780");
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();

    auto present_mode_compatibility = LvlInitStruct<VkSurfacePresentModeCompatibilityEXT>();
    present_mode.presentMode = pdev_surface_present_modes[0];
    surface_caps.pNext = &present_mode_compatibility;
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);

    std::vector<VkPresentModeKHR> compatible_present_modes(present_mode_compatibility.presentModeCount);
    present_mode_compatibility.pPresentModes = compatible_present_modes.data();
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);

    auto scaling_capabilities = LvlInitStruct<VkSurfacePresentScalingCapabilitiesEXT>();
    surface_caps.pNext = &scaling_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);

    mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;

    auto present_modes_ci = LvlInitStruct<VkSwapchainPresentModesCreateInfoEXT>();
    swapchain_create_info.pNext = &present_modes_ci;
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &mismatched_present_mode;

    // Pick a presentmode that's not in gpspmkhr
    for (auto item : defined_present_modes) {
        if (std::find(pdev_surface_present_modes.begin(), pdev_surface_present_modes.end(), item) ==
            pdev_surface_present_modes.end()) {
            mismatched_present_mode = item;
            break;
        }
    }
    if (mismatched_present_mode != VK_PRESENT_MODE_MAX_ENUM_KHR) {
        // Each entry in QueuePresent->vkPresentInfoKHR->pNext->SwapchainPresentModesCreateInfo->pPresentModes must be one of the
        // VkPresentModeKHR values returned by vkGetPhysicalDeviceSurfacePresentModesKHR for the surface
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentModesCreateInfoEXT-None-07762");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-pPresentModes-07763");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764");
        vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
    }

    // The entries in pPresentModes must be a subset of the present modes returned in
    // VkSurfacePresentModeCompatibilityEXT::pPresentModes, given vkSwapchainCreateInfoKHR::presentMode in VkSurfacePresentModeEXT
    mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (auto item : defined_present_modes) {
        if (std::find(compatible_present_modes.begin(), compatible_present_modes.end(), item) == compatible_present_modes.end()) {
            mismatched_present_mode = item;
            break;
        }
    }
    if (mismatched_present_mode != VK_PRESENT_MODE_MAX_ENUM_KHR) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentModesCreateInfoEXT-pPresentModes-07763");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-None-07762");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764");
        present_modes_ci.pPresentModes = &mismatched_present_mode;
        vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
    }

    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = present_mode_compatibility.pPresentModes;
    // SwapchainCreateInfo->presentMode has to be in VkSurfacePresentModeCompatibilityEXT->pPresentModes
    if (compatible_present_modes.size() > 1) {
        swapchain_create_info.presentMode = compatible_present_modes[1];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764");
        vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
    }

    swapchain_create_info.presentMode = compatible_present_modes[0];
    auto present_scaling_info = LvlInitStruct<VkSwapchainPresentScalingCreateInfoEXT>();
    present_scaling_info.pNext = swapchain_create_info.pNext;
    swapchain_create_info.pNext = &present_scaling_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07765");
    present_scaling_info.presentGravityY = 1;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07766");
    present_scaling_info.presentGravityX = 1;
    present_scaling_info.presentGravityY = 0;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07767");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-pNext-07782");
    present_scaling_info.presentGravityX = 0;
    present_scaling_info.scalingBehavior = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT | VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07768");
    present_scaling_info.presentGravityX = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT | VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    present_scaling_info.presentGravityY = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT;
    present_scaling_info.scalingBehavior = 0;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07769");
    present_scaling_info.presentGravityX = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT;
    present_scaling_info.presentGravityY = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT | VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();

    present_scaling_info.presentGravityX = 0;
    present_scaling_info.presentGravityY = 0;
    // Find scaling cap not in scaling_capabilities.supportedPresentScaling and create a swapchain using that
    if (scaling_capabilities.supportedPresentScaling != 0) {
        const std::array defined_scaling_flag_bits = {VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
                                                      VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT,
                                                      VK_PRESENT_SCALING_STRETCH_BIT_EXT};
        for (auto scaling_flag : defined_scaling_flag_bits) {
            if ((scaling_capabilities.supportedPresentScaling & scaling_flag) == 0) {
                present_scaling_info.scalingBehavior = scaling_flag;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                     "VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07770");
                vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    const std::array defined_gravity_flag_bits = {VK_PRESENT_GRAVITY_MIN_BIT_EXT, VK_PRESENT_GRAVITY_MAX_BIT_EXT,
                                                  VK_PRESENT_GRAVITY_CENTERED_BIT_EXT};
    if (scaling_capabilities.supportedPresentGravityX != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityX & gravity_flag) == 0) {
                present_scaling_info.presentGravityX = gravity_flag;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                     "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07772");
                vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }
    if (scaling_capabilities.supportedPresentGravityY != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityY & gravity_flag) == 0) {
                present_scaling_info.presentGravityY = gravity_flag;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                     "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07774");
                vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // If the swapchain is created with VkSwapchainPresentModesCreateInfoEXT,
    present_mode.presentMode = present_modes_ci.pPresentModes[0];
    surface_caps.pNext = &scaling_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);

    // presentScaling must be a valid scaling method for the surface
    // as returned in VkSurfacePresentScalingCapabilitiesEXT::supportedPresentScaling,
    // given each present mode in VkSwapchainPresentModesCreateInfoEXT::pPresentModes in VkSurfacePresentModeEXT
    if (scaling_capabilities.supportedPresentScaling != 0) {
        const std::array defined_scaling_flag_bits = {VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
                                                      VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT,
                                                      VK_PRESENT_SCALING_STRETCH_BIT_EXT};
        for (auto scaling_flag : defined_scaling_flag_bits) {
            if ((scaling_capabilities.supportedPresentScaling & scaling_flag) == 0) {
                present_scaling_info.scalingBehavior = scaling_flag;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                     "VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07771");
                vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // presentGravityX must be a valid x-axis present gravity for the surface
    // as returned in VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityX,
    // given each present mode in VkSwapchainPresentModesCreateInfoEXT::pPresentModes in VkSurfacePresentModeEXT
    if (scaling_capabilities.supportedPresentGravityX != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityX & gravity_flag) == 0) {
                present_scaling_info.presentGravityX = gravity_flag;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                     "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07773");
                vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // presentGravityY must be a valid y-axis present gravity for the surface
    // as returned in VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityY,
    // given each present mode in VkSwapchainPresentModesCreateInfoEXT::pPresentModes in VkSurfacePresentModeEXT
    if (scaling_capabilities.supportedPresentGravityY != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityY & gravity_flag) == 0) {
                present_scaling_info.presentGravityY = gravity_flag;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                     "VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07775");
                vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // Create swapchain
    VkPresentModeKHR good_present_mode = m_surface_non_shared_present_mode;
    present_modes_ci.pPresentModes = &good_present_mode;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore acquire_semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &acquire_semaphore));

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

    vk::QueueWaitIdle(m_device->m_queue);

    auto vkReleaseSwapchainImagesEXT =
        reinterpret_cast<PFN_vkReleaseSwapchainImagesEXT>(vk::GetInstanceProcAddr(instance(), "vkReleaseSwapchainImagesEXT"));

    uint32_t release_index = swapchain_images_count + 2;
    auto release_info = LvlInitStruct<VkReleaseSwapchainImagesInfoEXT>();
    release_info.swapchain = m_swapchain;
    release_info.imageIndexCount = swapchain_images_count;
    release_info.pImageIndices = &release_index;
    if (vkReleaseSwapchainImagesEXT) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07785");
        vkReleaseSwapchainImagesEXT(m_device->device(), &release_info);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroySemaphore(m_device->device(), acquire_semaphore, nullptr);
}

TEST_F(VkLayerTest, SwapchainMaintenance1ExtensionTestsCaps) {
    TEST_DESCRIPTION("Test swapchain and surface Maintenance1 extensions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    // Add this after check, surfacless checks are done conditionally
    AddOptionalExtensions(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME);

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
    InitSwapchainInfo();

    uint32_t count;

    // Call CreateSwapChain with a VkSwapchainPresentModesCreateInfoEXT struct W/O calling getcompatibleModes/getScalingCaps
    VkSurfaceKHR surface = m_surface;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT;

    VkPresentModeKHR old_present_mode = m_surface_non_shared_present_mode;
    auto present_modes_ci = LvlInitStruct<VkSwapchainPresentModesCreateInfoEXT>();
    swapchain_create_info.pNext = &present_modes_ci;
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &old_present_mode;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vk_testing::Image image_from_swapchain(*m_device, image_create_info, vk_testing::no_mem);

    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_info.image = image_from_swapchain.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    auto bind_swapchain_info = LvlInitStruct<VkBindImageMemorySwapchainInfoKHR>();
    bind_swapchain_info.imageIndex = 0;
    bind_info.pNext = &bind_swapchain_info;
    bind_swapchain_info.swapchain = m_swapchain;

    // SwapchainMaint1 enabled + deferred_memory_alloc but image not acquired:
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemorySwapchainInfoKHR-swapchain-07756");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), m_surface, &count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu(), m_surface, &count, present_modes.data());

    auto vkGetPhysicalDeviceSurfaceCapabilities2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceSurfaceCapabilities2KHR"));

    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
    auto surface_caps = LvlInitStruct<VkSurfaceCapabilities2KHR>();
    surface_info.surface = m_surface;

    auto present_mode = LvlInitStruct<VkSurfacePresentModeEXT>();
    auto present_mode_compatibility = LvlInitStruct<VkSurfacePresentModeCompatibilityEXT>();
    present_mode.presentMode = present_modes[0];
    surface_caps.pNext = &present_mode_compatibility;

    // Leave VkSurfacePresentMode off of the pNext chain
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07776");
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();

    surface_info.pNext = &present_mode;
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);

    std::vector<VkPresentModeKHR> compatible_present_modes(present_mode_compatibility.presentModeCount);
    present_mode_compatibility.pPresentModes = compatible_present_modes.data();
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);

    auto scaling_capabilities = LvlInitStruct<VkSurfacePresentScalingCapabilitiesEXT>();
    surface_caps.pNext = &scaling_capabilities;
    surface_info.pNext = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07777");
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();

    if (IsExtensionsEnabled(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME)) {
        surface_info.pNext = &present_mode;
        surface_info.surface = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07778");
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);
        m_errorMonitor->VerifyFound();

        surface_caps.pNext = &present_mode_compatibility;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07779");
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_caps);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, SwapchainMaintenance1ExtensionTestsRelease) {
    TEST_DESCRIPTION("Test acquiring swapchain images with Maint1 features.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();

    VkSurfaceKHR surface = m_surface;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT;

    VkPresentModeKHR old_present_mode = m_surface_non_shared_present_mode;
    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = LvlInitStruct<VkSwapchainPresentModesCreateInfoEXT>();
    swapchain_create_info.pNext = &present_modes_ci;
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &old_present_mode;

    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore acquire_semaphore;
    VkSemaphore submit_semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &acquire_semaphore));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &submit_semaphore));

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index);

    m_commandBuffer->begin();

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->end();

    VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &acquire_semaphore;
    submit_info.pWaitDstStageMask = &stage_mask;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &submit_semaphore;

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    VkPresentInfoKHR present = LvlInitStruct<VkPresentInfoKHR>();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &submit_semaphore;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    VkFenceObj present_fence;
    present_fence.init(*m_device, VkFenceObj::create_info());

    // PresentFenceInfo swapchaincount not equal to PresentInfo swapchaincount
    VkSwapchainPresentFenceInfoEXT fence_info = LvlInitStruct<VkSwapchainPresentFenceInfoEXT>();
    fence_info.swapchainCount = present.swapchainCount + 1;
    fence_info.pFences = &present_fence.handle();
    present.pNext = &fence_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentFenceInfoEXT-swapchainCount-07757");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    const std::vector<VkPresentModeKHR> defined_present_modes{
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
        VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
    };

    VkPresentModeKHR mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (auto item : defined_present_modes) {
        if (item != old_present_mode) {
            mismatched_present_mode = item;
            break;
        }
    }

    // Each entry in pPresentModes must be a presentation mode specified in VkSwapchainPresentModesCreateInfoEXT::pPresentModes
    // when creating the entry's corresponding swapchain
    VkSwapchainPresentModeInfoEXT present_mode_info = LvlInitStruct<VkSwapchainPresentModeInfoEXT>();
    present_mode_info.swapchainCount = 1;
    present_mode_info.pPresentModes = &mismatched_present_mode;
    present.pNext = &present_mode_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainPresentModeInfoEXT-pPresentModes-07761");
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    // QueuePresent resets image[index].acquired to false
    VkPresentModeKHR good_present_mode = m_surface_non_shared_present_mode;
    present_mode_info.pPresentModes = &good_present_mode;
    vk::QueuePresentKHR(m_device->m_queue, &present);

    auto vkReleaseSwapchainImagesEXT =
        reinterpret_cast<PFN_vkReleaseSwapchainImagesEXT>(vk::GetInstanceProcAddr(instance(), "vkReleaseSwapchainImagesEXT"));

    uint32_t release_index = 0;
    VkReleaseSwapchainImagesInfoEXT release_info = LvlInitStruct<VkReleaseSwapchainImagesInfoEXT>();
    release_info.swapchain = m_swapchain;
    release_info.imageIndexCount = swapchain_images_count;
    release_info.pImageIndices = &release_index;
    if (vkReleaseSwapchainImagesEXT) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07785");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07786");
        vkReleaseSwapchainImagesEXT(m_device->device(), &release_info);
        m_errorMonitor->VerifyFound();
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroySemaphore(device(), submit_semaphore, nullptr);
    vk::DestroySemaphore(device(), acquire_semaphore, nullptr);
}

#ifdef VVL_TESTS_ENABLE_EXCLUSIVE_FULLSCREEN
TEST_F(VkLayerTest, TestvkAcquireFullScreenExclusiveModeEXT) {
    TEST_DESCRIPTION("Test vkAcquireFullScreenExclusiveModeEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
#ifndef VK_USE_PLATFORM_WIN32_KHR
    GTEST_SKIP() << "Test not supported on platform, skipping".
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    InitSwapchainInfo();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    auto vkAcquireFullScreenExclusiveModeEXT = reinterpret_cast<PFN_vkAcquireFullScreenExclusiveModeEXT>(
        vk::GetInstanceProcAddr(instance(), "vkAcquireFullScreenExclusiveModeEXT"));
    auto vkReleaseFullScreenExclusiveModeEXT = reinterpret_cast<PFN_vkReleaseFullScreenExclusiveModeEXT>(
        vk::GetInstanceProcAddr(instance(), "vkReleaseFullScreenExclusiveModeEXT"));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02675");
    vkAcquireFullScreenExclusiveModeEXT(device(), m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkReleaseFullScreenExclusiveModeEXT-swapchain-02678");
    vkReleaseFullScreenExclusiveModeEXT(device(), m_swapchain);
    m_errorMonitor->VerifyFound();

    const POINT pt_zero = {0, 0};

    auto surface_full_screen_exlusive_info_win32 = LvlInitStruct<VkSurfaceFullScreenExclusiveWin32InfoEXT>();
    surface_full_screen_exlusive_info_win32.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);

    auto surface_full_screen_exlusive_info =
        LvlInitStruct<VkSurfaceFullScreenExclusiveInfoEXT>(&surface_full_screen_exlusive_info_win32);
    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    auto swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = m_swapchain;

    VkSwapchainKHR swapchain_one, swapchain_two;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain_one);
    swapchain_create_info.oldSwapchain = swapchain_one;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain_two);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02674");
    vkAcquireFullScreenExclusiveModeEXT(device(), swapchain_one);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkReleaseFullScreenExclusiveModeEXT-swapchain-02677");
    vkReleaseFullScreenExclusiveModeEXT(device(), swapchain_one);
    m_errorMonitor->VerifyFound();

    VkResult res = vkAcquireFullScreenExclusiveModeEXT(device(), swapchain_two);
    if (res == VK_SUCCESS) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02676");
        vkAcquireFullScreenExclusiveModeEXT(device(), swapchain_two);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroySwapchainKHR(device(), swapchain_one, nullptr);
    vk::DestroySwapchainKHR(device(), swapchain_two, nullptr);
#endif
}
#endif

TEST_F(VkLayerTest, TestCreatingWin32Surface) {
    TEST_DESCRIPTION("Test creating win32 surface with invalid hwnd");

#ifndef VK_USE_PLATFORM_WIN32_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    ASSERT_NO_FATAL_FAILURE(Init());

    auto surface_create_info = LvlInitStruct<VkWin32SurfaceCreateInfoKHR>();
    surface_create_info.hinstance = GetModuleHandle(0);
    surface_create_info.hwnd = NULL; // Invalid

    VkSurfaceKHR surface;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308");
    vk::CreateWin32SurfaceKHR(instance(), &surface_create_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();
#endif
}

TEST_F(VkLayerTest, UseSwapchainImageBeforeWait) {
    TEST_DESCRIPTION("Test using a swapchain image that was acquired but not waited on.");

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPresentInfoKHR-pImageIndices-01296");

    vk_testing::Semaphore acquire_semaphore(*m_device);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore.handle(), VK_NULL_HANDLE, &image_index);

    auto present = LvlInitStruct<VkPresentInfoKHR>();
    present.waitSemaphoreCount = 0; // Invalid, acquire_semaphore should be waited on
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    vk::QueuePresentKHR(m_device->m_queue, &present);

    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestCreatingSwapchainWithInvalidExtent) {
    TEST_DESCRIPTION("Create swapchain with extent greater than maxImageExtent of SurfaceCapabilities");

    AddSurfaceExtension();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    InitSwapchainInfo();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-imageExtent-01274");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778");

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &surface_capabilities);

    auto swapchain_ci = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = m_surface;
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent.width = surface_capabilities.maxImageExtent.width + 1;
    swapchain_ci.imageExtent.height = surface_capabilities.maxImageExtent.height;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;

    VkSwapchainKHR swapchain;
    vk::CreateSwapchainKHR(device(), &swapchain_ci, nullptr, &swapchain);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestSurfaceQueryImageCompressionControlWithoutExtension) {
    TEST_DESCRIPTION("Test querying surface image compression control without extension.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();

    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);

    auto image_compression_control = LvlInitStruct<VkPhysicalDeviceImageCompressionControlFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&image_compression_control);

    ASSERT_NO_FATAL_FAILURE(InitFrameworkAndRetrieveFeatures(features2));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (image_compression_control.imageCompressionControl) {
        // disable imageCompressionControl feature;
        image_compression_control.imageCompressionControl = VK_FALSE;
        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    } else {
        ASSERT_NO_FATAL_FAILURE(InitState());
    }

    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface";
    }

    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR =
        reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormats2KHR>(vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceSurfaceFormats2KHR"));

    auto compression_properties = LvlInitStruct<VkImageCompressionPropertiesEXT>();
    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>(&compression_properties);
    surface_info.surface = m_surface;
    uint32_t count;

    // get compression control properties even of VK_EXT_image_compression_control extension is disabled(or is not supported).
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext");
    vkGetPhysicalDeviceSurfaceFormats2KHR(gpu(), &surface_info, &count, nullptr);
    m_errorMonitor->VerifyFound();
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(VkLayerTest, PhysicalDeviceSurfaceCapabilities) {
    TEST_DESCRIPTION("Test pNext in GetPhysicalDeviceSurfaceCapabilities2KHR");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vk::GetInstanceProcAddr(instance(),
                                                                                "vkGetPhysicalDeviceSurfaceCapabilities2KHR");

    auto surface_info = LvlInitStruct<VkPhysicalDeviceSurfaceInfo2KHR>();
    surface_info.surface = m_surface;

    auto capabilities_full_screen_exclusive = LvlInitStruct<VkSurfaceCapabilitiesFullScreenExclusiveEXT>();

    auto surface_capabilities = LvlInitStruct<VkSurfaceCapabilities2KHR>(&capabilities_full_screen_exclusive);
    surface_capabilities.surfaceCapabilities = m_surface_capabilities;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-02671");
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu(), &surface_info, &surface_capabilities);
    m_errorMonitor->VerifyFound();
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
        //
TEST_F(VkLayerTest, QueuePresentWaitingSameSemaphore) {
    TEST_DESCRIPTION("Submit to queue with waitSemaphore that another queue is already waiting on.");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping test";
    }

    if (m_device->graphics_queues().size() < 2) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    uint32_t image_index{0};
    uint32_t image_count{0};
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data());

    vk_testing::Fence fence(*m_device);
    vk_testing::Semaphore semaphore(*m_device);

    auto err = vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore.handle(), fence.handle(), &image_index);
    ASSERT_VK_SUCCESS(err);

    err = fence.wait(kWaitTimeout);
    ASSERT_VK_SUCCESS(err);
    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkQueue other = m_device->graphics_queues()[1]->handle();

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    auto wait_submit = LvlInitStruct<VkSubmitInfo>();
    wait_submit.waitSemaphoreCount = 1;
    wait_submit.pWaitSemaphores = &semaphore.handle();
    wait_submit.pWaitDstStageMask = &stage_flags;

    vk::QueueSubmit(m_device->m_queue, 1, &wait_submit, VK_NULL_HANDLE);

    auto present = LvlInitStruct<VkPresentInfoKHR>();
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    present.swapchainCount = 1;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &semaphore.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueuePresentKHR-pWaitSemaphores-01294");
    vk::QueuePresentKHR(other, &present);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    vk::QueueWaitIdle(other);
}

TEST_F(VkLayerTest, QueuePresentBinarySemaphoreNotSignaled) {
    TEST_DESCRIPTION("Submit a present operation with a waiting binary semaphore not previously signaled.");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
    // timeline semaphore determines which VUID used, even though it isn't needed for the test
    AddOptionalExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto timeline_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(timeline_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    if (!InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "Cannot create surface or swapchain, skipping test";
    }
    uint32_t image_index{0};
    uint32_t image_count{0};
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data());

    vk_testing::Fence fence(*m_device);
    vk_testing::Semaphore semaphore(*m_device);

    auto err = vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore.handle(), fence.handle(), &image_index);
    ASSERT_VK_SUCCESS(err);

    err = fence.wait(kWaitTimeout);
    ASSERT_VK_SUCCESS(err);
    SetImageLayout(m_device, VK_IMAGE_ASPECT_COLOR_BIT, images[image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stage_flags;
    err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    auto present = LvlInitStruct<VkPresentInfoKHR>();
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;
    present.swapchainCount = 1;
    present.waitSemaphoreCount = 1;
    // the semaphore has already been waited on
    present.pWaitSemaphores = &semaphore.handle();

    // VUIDs reported change if the extension is enabled, even if the timelineSemaphore feature isn't supported.
    const bool has_timeline_sem_ext = DeviceExtensionEnabled(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    const char *expected_vuid =
        has_timeline_sem_ext ? "VUID-vkQueuePresentKHR-pWaitSemaphores-03268" : "VUID-vkQueuePresentKHR-pWaitSemaphores-01295";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
    vk::QueuePresentKHR(m_device->m_queue, &present);
    m_errorMonitor->VerifyFound();

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
}
