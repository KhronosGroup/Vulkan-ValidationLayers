/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "utils/vk_layer_utils.h"
#include "generated/enum_flag_bits.h"

bool copy_layout_supported(std::vector<VkImageLayout> &copy_src_layouts, std::vector<VkImageLayout> &copy_dst_layouts,
                           VkImageLayout layout) {
    return ((std::find(copy_src_layouts.begin(), copy_src_layouts.end(), layout) != copy_src_layouts.end()) &&
            (std::find(copy_dst_layouts.begin(), copy_dst_layouts.end(), layout) != copy_dst_layouts.end()));
}

void NegativeHostImageCopy::InitHostImageCopyTest(VkFormat &compressed_format) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    // Assumes VK_KHR_sampler_ycbcr_conversion and VK_EXT_separate_stencil_usage,
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "Need 1.2 api version";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto host_copy_features = LvlInitStruct<VkPhysicalDeviceHostImageCopyFeaturesEXT>();
    GetPhysicalDeviceFeatures2(host_copy_features);
    if (!host_copy_features.hostImageCopy) {
        GTEST_SKIP() << "Test requires (unsupported) multiDraw";
    }

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    compressed_format = VK_FORMAT_UNDEFINED;
    if (device_features.textureCompressionBC) {
        compressed_format = VK_FORMAT_BC3_SRGB_BLOCK;
    } else if (device_features.textureCompressionETC2) {
        compressed_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    } else if (device_features.textureCompressionASTC_LDR) {
        compressed_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &host_copy_features));
}

TEST_F(NegativeHostImageCopy, HostCopyImageToFromMemory) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from images to memory and vice versa");
    VkFormat compressed_format = VK_FORMAT_UNDEFINED;
    InitHostImageCopyTest(compressed_format);
    if (::testing::Test::IsSkipped()) return;

    uint32_t width = 32;
    uint32_t height = 32;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto image_ci = VkImageObj::ImageCreateInfo2D(
        width, height, 1, 1, format,
        VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_TILING_OPTIMAL);
    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), image_ci.format, image_ci.imageType,
                                                                 image_ci.tiling, image_ci.usage, image_ci.flags, &img_prop)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto host_image_copy_props = LvlInitStruct<VkPhysicalDeviceHostImageCopyPropertiesEXT>();
    GetPhysicalDeviceProperties2(host_image_copy_props);
    std::vector<VkImageLayout> copy_src_layouts;
    std::vector<VkImageLayout> copy_dst_layouts;
    copy_src_layouts.resize(host_image_copy_props.copySrcLayoutCount);
    copy_dst_layouts.resize(host_image_copy_props.copyDstLayoutCount);
    host_image_copy_props.pCopySrcLayouts = copy_src_layouts.data();
    host_image_copy_props.pCopyDstLayouts = copy_dst_layouts.data();
    GetPhysicalDeviceProperties2(host_image_copy_props);
    if (!copy_layout_supported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ||
        !copy_layout_supported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_GENERAL)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageObj image(m_device);
    image.Init(image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    auto region_to_image = LvlInitStruct<VkMemoryToImageCopyEXT>();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_to_image.imageSubresource.layerCount = 1;
    region_to_image.imageExtent.width = width;
    region_to_image.imageExtent.height = height;
    region_to_image.imageExtent.depth = 1;

    auto copy_to_image = LvlInitStruct<VkCopyMemoryToImageInfoEXT>();
    copy_to_image.flags = 0;
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    auto region_from_image = LvlInitStruct<VkImageToMemoryCopyEXT>();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_from_image.imageSubresource.layerCount = 1;
    region_from_image.imageExtent.width = width;
    region_from_image.imageExtent.height = height;
    region_from_image.imageExtent.depth = 1;

    auto copy_from_image = LvlInitStruct<VkCopyImageToMemoryInfoEXT>();
    copy_from_image.flags = 0;
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Bad image layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImageLayout-09059");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImageLayout-09064");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    copy_to_image.dstImageLayout = layout;
    copy_from_image.srcImageLayout = layout;

    {
        auto image_ci_no_transfer = VkImageObj::ImageCreateInfo2D(width, height, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                  VK_IMAGE_TILING_OPTIMAL);
        // Missing transfer usage
        VkImageObj image_no_transfer(m_device);
        image_no_transfer.Init(image_ci_no_transfer);
        image_no_transfer.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        copy_to_image.dstImage = image_no_transfer;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09113");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();

        copy_from_image.srcImage = image_no_transfer;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09113");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
    }
    // Memcpy with imageOffset x, y, or z == 0
    copy_to_image.flags = VK_HOST_IMAGE_COPY_MEMCPY_EXT;
    region_to_image.imageOffset.x = 1;
    // If ImageExtent.width is left at width, offset will exceed width of image (07971). Setting it to width-1 will not match
    // image dimensions (09115). Pick the one with MEMCPY flag set and test for both here.
    region_to_image.imageExtent.width = width - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09114");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageExtent-09115");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    copy_from_image.flags = VK_HOST_IMAGE_COPY_MEMCPY_EXT;
    region_from_image.imageOffset.x = 1;
    region_from_image.imageExtent.width = width - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09114");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageExtent-09115");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    // Reset to baseline test configuration
    copy_to_image.flags = 0;
    region_to_image.imageOffset.x = 0;
    region_to_image.imageExtent.width = width;
    copy_from_image.flags = 0;
    region_from_image.imageOffset.x = 0;
    region_from_image.imageExtent.width = width;

    {
        // No image memory
        VkImageObj image_no_mem(m_device);
        image_no_mem.init_no_mem(*m_device, image_ci);
        copy_to_image.dstImage = image_no_mem;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07966");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();

        copy_from_image.srcImage = image_no_mem;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07966");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
    }

    // Bad mipLevel - also throws off multiple size calculations, causing other errors
    // Also get 07970 - pRegions must be contained within the specified dstSubresource of dstImage
    // Also get 07971 - imageOffset.x and (imageExtent.width + imageOffset.x) both >= 0 and <= imageSubresource width
    // Also get 07972 - imageOffset.y and (imageExtent.height + imageOffset.y) both >= 0 and <= imageSubresource height
    // Also get 09104 - imageOffset.z and (imageExtent.depth + imageOffset.z) both >= 0 and <= imageSubresource depth
    region_to_image.imageSubresource.mipLevel = image_ci.mipLevels + 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09104");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07967");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    region_from_image.imageSubresource.mipLevel = image_ci.mipLevels + 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09104");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07967");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageSubresource.mipLevel = 0;
    region_from_image.imageSubresource.mipLevel = 0;

    // baseArrayLayer + layerCount > arrayLayers
    region_to_image.imageSubresource.baseArrayLayer = image_ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-08790");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.baseArrayLayer = image_ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-08790");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageSubresource.baseArrayLayer = 0;
    region_from_image.imageSubresource.baseArrayLayer = 0;

    {
        // Can't use subsampled image
        VkImageObj image_subsampled(m_device);
        image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
        image_subsampled.Init(image_ci);
        image_subsampled.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        copy_to_image.dstImage = image_subsampled;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07969");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();

        copy_from_image.srcImage = image_subsampled;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07969");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
        image_ci.flags = 0;
    }

    // Extent bigger than image - Can't get 07970 without getting 07971, 07972 or 09104, so test both 07971 and 07970 here
    region_to_image.imageExtent.width = width + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07971");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.width = width + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07971");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageExtent.width = width;
    region_from_image.imageExtent.width = width;

    // imageOffset.y and (imageExtent.height + imageOffset.y) both >= 0 and <= imageSubresource height
    region_to_image.imageOffset.y = -1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageOffset.y = -1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageOffset.y = 0;
    region_from_image.imageOffset.y = 0;

    {
        // Use 3D image to avoid 07980
        VkImageObj image_3d(m_device);
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_3d.Init(image_ci);
        image_3d.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // Image must have offset.z of 0 and extent.depth of 1
        copy_to_image.dstImage = image_3d;
        region_to_image.imageOffset.z = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09104");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageOffset.z = 1;
        copy_from_image.srcImage = image_3d;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09104");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        region_to_image.imageOffset.z = 0;
        region_from_image.imageOffset.z = 0;

        // imageSubresource.baseArrayLayer must be 0 and imageSubresource.layerCount must be 1
        region_to_image.imageSubresource.baseArrayLayer = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07983");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-08790");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.baseArrayLayer = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07983");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-08790");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        region_to_image.imageSubresource.baseArrayLayer = 0;
        region_from_image.imageSubresource.baseArrayLayer = 0;

        // imageExtent.depth must not be 0
        region_to_image.imageExtent.depth = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryToImageCopyEXT-imageExtent-06661");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageExtent.depth = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageToMemoryCopyEXT-imageExtent-06661");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        region_to_image.imageExtent.depth = 1;
        region_from_image.imageExtent.depth = 1;

        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
    }

    {
        // Can't use sampled image
        VkImageObj image_samplecount(m_device);
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        image_samplecount.Init(image_ci);
        image_samplecount.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        copy_to_image.dstImage = image_samplecount;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07973");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();

        copy_from_image.srcImage = image_samplecount;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07973");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    }

    {
        // Image type 1D
        // imageOffset.y must be 0 and imageExtent.height must be 1
        VkImageObj image_1d(m_device);
        image_ci.imageType = VK_IMAGE_TYPE_1D;
        image_ci.extent.height = 1;
        image_1d.Init(image_ci);
        image_1d.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.extent.height = height;
        copy_to_image.dstImage = image_1d;
        region_to_image.imageOffset.y = 1;
        region_to_image.imageExtent.height = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07979");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07972");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        copy_from_image.srcImage = image_1d;
        region_from_image.imageOffset.y = 1;
        region_from_image.imageExtent.height = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07979");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07972");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        region_to_image.imageOffset.y = 0;
        region_from_image.imageOffset.y = 0;

        // imageOffset.z must be 0 and imageExtent.depth must be 1
        region_to_image.imageOffset.z = 1;
        region_to_image.imageExtent.depth = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07980");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09104");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageOffset.z = 1;
        region_from_image.imageExtent.depth = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07980");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09104");
        m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();

        region_to_image.imageOffset.z = 0;
        region_from_image.imageOffset.z = 0;
        region_from_image.imageExtent.height = height;
        region_to_image.imageExtent.height = height;
        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
    }

    if (compressed_format != VK_FORMAT_UNDEFINED) {
        if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), compressed_format,
                                                                     image_ci.imageType, image_ci.tiling, image_ci.usage,
                                                                     image_ci.flags, &img_prop)) {
            VkImageObj image_compressed(m_device);
            image_ci.format = compressed_format;
            image_ci.mipLevels = 6;
            image_compressed.Init(image_ci);
            image_compressed.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            // imageOffset not a multiple of block size
            region_to_image.imageOffset = {1, 1, 0};
            region_to_image.imageExtent = {1, 1, 1};
            region_to_image.imageSubresource.mipLevel = 4;
            copy_to_image.dstImage = image_compressed;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-pRegions-07274");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-pRegions-07275");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            region_from_image.imageOffset = {1, 1, 0};
            region_from_image.imageExtent = {1, 1, 1};
            region_from_image.imageSubresource.mipLevel = 4;
            copy_from_image.srcImage = image_compressed;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-pRegions-07274");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-pRegions-07275");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();
            region_to_image.imageOffset = {0, 0, 0};
            region_from_image.imageOffset = {0, 0, 0};

            // width not a multiple of compressed block width
            region_to_image.imageExtent = {1, 2, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageExtent-00207");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            region_from_image.imageExtent = {1, 2, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageExtent-00207");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();

            // Copy height < compressed block size but not the full mip height
            region_to_image.imageExtent = {2, 1, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageExtent-00208");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            region_from_image.imageExtent = {2, 1, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageExtent-00208");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();

            region_to_image.imageSubresource.mipLevel = 0;
            region_from_image.imageSubresource.mipLevel = 0;
            region_to_image.imageExtent = {width, height, 1};
            region_from_image.imageExtent = {width, height, 1};

            // memoryRowLength not a multiple of block width (4)
            region_to_image.memoryRowLength = 130;
            region_to_image.memoryImageHeight = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-memoryRowLength-09106");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            region_from_image.memoryRowLength = 130;
            region_from_image.memoryImageHeight = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-memoryRowLength-09106");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();

            // memoryImageHeight not a multiple of block height (4)
            region_to_image.memoryRowLength = 0;
            region_to_image.memoryImageHeight = 130;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-memoryImageHeight-09107");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            region_from_image.memoryRowLength = 0;
            region_from_image.memoryImageHeight = 130;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-memoryImageHeight-09107");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();

            region_to_image.memoryImageHeight = 0;
            region_from_image.memoryImageHeight = 0;

            // memoryRowLength divided by the texel block extent width and then multiplied by the texel block size of the image must
            // be less than or equal to 2^31-1
            region_to_image.memoryRowLength = 0x20000000;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-memoryRowLength-09108");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            region_from_image.memoryRowLength = 0x20000000;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-memoryRowLength-09108");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();

            region_to_image.memoryRowLength = 0;
            region_from_image.memoryRowLength = 0;
            copy_to_image.dstImage = image;
            copy_from_image.srcImage = image;
            image_ci.format = format;
            image_ci.mipLevels = 1;
        }
    }

    // Bad aspectMask
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-09105");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-09105");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (copy_layout_supported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)) {
        auto stencil_format = FindSupportedDepthStencilFormat(gpu());
        VkImageObj image_stencil(m_device);
        image_ci.format = stencil_format;
        image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_stencil.Init(image_ci);
        image_stencil.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

        // Stencil, no VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        copy_to_image.dstImage = image_stencil;
        copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09111");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        copy_from_image.srcImage = image_stencil;
        copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09111");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();

        auto stencil_usage_ci = LvlInitStruct<VkImageStencilUsageCreateInfo>();
        stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_ci.pNext = &stencil_usage_ci;
        VkImageObj image_separate_stencil(m_device);
        image_separate_stencil.Init(image_ci);
        image_separate_stencil.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

        // Seperate stencil, no VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        copy_to_image.dstImage = image_separate_stencil;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09112");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        copy_from_image.srcImage = image_separate_stencil;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09112");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();

        if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                              m_device->phy().handle(), stencil_format, image_ci.imageType, image_ci.tiling,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT, image_ci.flags,
                              &img_prop)) {
            // The aspectMask member of imageSubresource must only have a single bit set
            VkImageObj image_stencil2(m_device);
            image_ci.format = stencil_format;
            image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
            image_ci.pNext = nullptr;
            image_stencil2.Init(image_ci);
            image_stencil2.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            copy_to_image.dstImage = image_stencil2;
            region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryToImageCopyEXT-aspectMask-09103");
            vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
            m_errorMonitor->VerifyFound();
            copy_from_image.srcImage = image_stencil2;
            region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageToMemoryCopyEXT-aspectMask-09103");
            vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
            m_errorMonitor->VerifyFound();
        }

        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_to_image.dstImage = image;
        copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_from_image.srcImage = image;
        copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        image_ci.format = format;
        image_ci.usage = VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.pNext = nullptr;
    }

    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &img_prop)) {
        // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT or VK_IMAGE_ASPECT_PLANE_1_BIT
        VkImageObj image_multi_planar2(m_device);
        image_multi_planar2.Init(128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                 VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_IMAGE_TILING_OPTIMAL, 0);
        image_multi_planar2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
        copy_to_image.dstImage = image_multi_planar2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07981");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
        copy_from_image.srcImage = image_multi_planar2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07981");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();

        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_to_image.dstImage = image;
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_from_image.srcImage = image;
    }

    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->phy().handle(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &img_prop)) {
        // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or
        // VK_IMAGE_ASPECT_PLANE_2_BIT
        VkImageObj image_multi_planar3(m_device);
        image_multi_planar3.Init(128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
                                 VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_IMAGE_TILING_OPTIMAL, 0);
        image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_to_image.dstImage = image_multi_planar3;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07982");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_from_image.srcImage = image_multi_planar3;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07982");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();

        copy_to_image.dstImage = image;
        copy_from_image.srcImage = image;
    }

    if (!copy_layout_supported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)) {
        // layout must be one of the image layouts returned in VkPhysicalDeviceHostImageCopyPropertiesEXT::pCopySrcLayouts
        image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyMemoryToImageInfoEXT-dstImageLayout-09060");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageToMemoryInfoEXT-srcImageLayout-09065");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
        copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    // memoryRowLength must be 0, or greater than or equal to the width member of imageExtent
    region_to_image.memoryRowLength = region_to_image.imageExtent.width - 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryToImageCopyEXT-memoryRowLength-09101");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryRowLength = region_from_image.imageExtent.width - 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageToMemoryCopyEXT-memoryRowLength-09101");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.memoryRowLength = 0;
    region_from_image.memoryRowLength = 0;

    // memoryImageHeight must be 0, or greater than or equal to the height member of imageExtent
    region_to_image.memoryImageHeight = region_to_image.imageExtent.height - 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryToImageCopyEXT-memoryImageHeight-09102");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryImageHeight = region_from_image.imageExtent.height - 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageToMemoryCopyEXT-memoryImageHeight-09102");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.memoryImageHeight = 0;
    region_from_image.memoryImageHeight = 0;

    // imageExtent.width must not be 0
    region_to_image.imageExtent.width = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryToImageCopyEXT-imageExtent-06659");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.width = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageToMemoryCopyEXT-imageExtent-06659");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageExtent.width = width;
    region_from_image.imageExtent.width = width;

    // imageExtent.height must not be 0
    region_to_image.imageExtent.height = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryToImageCopyEXT-imageExtent-06660");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.height = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageToMemoryCopyEXT-imageExtent-06660");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageExtent.height = height;
    region_from_image.imageExtent.height = height;
}
