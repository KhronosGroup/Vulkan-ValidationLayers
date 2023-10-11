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

bool HostImageCopyTest::CopyLayoutSupported(const std::vector<VkImageLayout> &src_layouts,
                                            const std::vector<VkImageLayout> &dst_layouts, VkImageLayout layout) {
    return ((std::find(src_layouts.begin(), src_layouts.end(), layout) != src_layouts.end()) &&
            (std::find(dst_layouts.begin(), dst_layouts.end(), layout) != dst_layouts.end()));
}

void HostImageCopyTest::InitHostImageCopyTest(const VkImageCreateInfo &image_ci) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    // Assumes VK_KHR_sampler_ycbcr_conversion and VK_EXT_separate_stencil_usage,

    VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR separate_depth_stencil_layouts_features = vku::InitStructHelper();
    VkPhysicalDeviceHostImageCopyFeaturesEXT host_copy_features = vku::InitStructHelper(&separate_depth_stencil_layouts_features);
    GetPhysicalDeviceFeatures2(host_copy_features);
    if (!host_copy_features.hostImageCopy) {
        GTEST_SKIP() << "Test requires (unsupported) hostImageCopy";
    }
    separate_depth_stencil = separate_depth_stencil_layouts_features.separateDepthStencilLayouts;
    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    compressed_format = VK_FORMAT_UNDEFINED;
    if (device_features.textureCompressionBC) {
        compressed_format = VK_FORMAT_BC3_SRGB_BLOCK;
    } else if (device_features.textureCompressionETC2) {
        compressed_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    } else if (device_features.textureCompressionASTC_LDR) {
        compressed_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    }

    RETURN_IF_SKIP(InitState(nullptr, &host_copy_features));
    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), image_ci.format, image_ci.imageType,
                                                                 image_ci.tiling, image_ci.usage, image_ci.flags, &img_prop)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkPhysicalDeviceHostImageCopyPropertiesEXT host_image_copy_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(host_image_copy_props);
    copy_src_layouts.resize(host_image_copy_props.copySrcLayoutCount);
    copy_dst_layouts.resize(host_image_copy_props.copyDstLayoutCount);
    host_image_copy_props.pCopySrcLayouts = copy_src_layouts.data();
    host_image_copy_props.pCopyDstLayouts = copy_dst_layouts.data();
    GetPhysicalDeviceProperties2(host_image_copy_props);
    if (!CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ||
        !CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_GENERAL)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }
}


TEST_F(PositiveHostImageCopy, BasicUsage) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy to and from host memory");

    uint32_t width = 32;
    uint32_t height = 32;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = VkImageObj::ImageCreateInfo2D(
        width, height, 1, 1, format,
        VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_TILING_OPTIMAL);
    RETURN_IF_SKIP(InitHostImageCopyTest(image_ci))

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Positive host image copy test requires a driver that can copy.";
    }
    if (!CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_GENERAL)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;
    VkImageObj image(m_device);
    image.Init(image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);
    // Fill image with random values
    for (auto &channel : pixels) {
        const uint32_t r = static_cast<uint32_t>(std::rand());
        channel = static_cast<uint8_t>((r & 0xffu) | ((r >> 8) & 0xff) | ((r >> 16) & 0xff) | (r >> 24));
    }

    VkMemoryToImageCopyEXT region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_to_image.imageSubresource.layerCount = 1;
    region_to_image.imageExtent.width = width;
    region_to_image.imageExtent.height = height;
    region_to_image.imageExtent.depth = 1;

    VkCopyMemoryToImageInfoEXT copy_to_image = vku::InitStructHelper();
    copy_to_image.flags = 0;
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkResult result = vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    ASSERT_EQ(VK_SUCCESS, result);

    // Copy back to host memory
    std::vector<uint8_t> welcome_back(width * height * 4);

    VkImageToMemoryCopyEXT region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = welcome_back.data();
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_from_image.imageSubresource.layerCount = 1;
    region_from_image.imageExtent.width = width;
    region_from_image.imageExtent.height = height;
    region_from_image.imageExtent.depth = 1;

    VkCopyImageToMemoryInfoEXT copy_from_image = vku::InitStructHelper();
    copy_from_image.flags = 0;
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    result = vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    ASSERT_EQ(VK_SUCCESS, result);
    ASSERT_EQ(pixels, welcome_back);

    // Copy from one image to another
    VkImageObj image2(m_device);
    image2.Init(image_ci);
    image2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfoEXT copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = layout;
    copy_image_to_image.dstImageLayout = layout;
    copy_image_to_image.srcImage = image;
    copy_image_to_image.dstImage = image2;

    result = vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    ASSERT_EQ(VK_SUCCESS, result);

    // Copy back from destination image to memory
    std::vector<uint8_t> after_image_copy(width * height * 4);

    copy_from_image.srcImage = image2;
    region_from_image.pHostPointer = after_image_copy.data();
    result = vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    ASSERT_EQ(VK_SUCCESS, result);
    ASSERT_EQ(pixels, after_image_copy);

    // Do a layout transition, then use the image in new layout
    VkHostImageLayoutTransitionInfoEXT transition_info = vku::InitStructHelper();
    transition_info.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    transition_info.image = image2;
    result = vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    ASSERT_EQ(VK_SUCCESS, result);
    VkImageSubresource image_sub = VkImageObj::subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
    VkImageSubresourceRange image_sub_range = VkImageObj::subresource_range(image_sub);
    VkImageMemoryBarrier image_barrier =
        image2.image_memory_barrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, image_sub_range);

    image_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &image_barrier);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(true);

    // Get memory size of tiled image
    VkImageSubresource2KHR subresource = vku::InitStructHelper();
    subresource.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceHostMemcpySizeEXT host_copy_size = vku::InitStructHelper();
    VkSubresourceLayout2KHR subresource_layout = vku::InitStructHelper(&host_copy_size);
    vk::GetImageSubresourceLayout2EXT(*m_device, image, &subresource, &subresource_layout);
    ASSERT_NE(host_copy_size.size, 0);

    VkHostImageCopyDevicePerformanceQueryEXT perf_data = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&perf_data);
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.usage = VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
    image_format_info.flags = 0;
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
}

TEST_F(PositiveHostImageCopy, CopyImageToMemoryMipLevel) {
    TEST_DESCRIPTION("Use only selected image mip level to memory");

    constexpr uint32_t width = 32;
    constexpr uint32_t height = 32;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = VkImageObj::ImageCreateInfo2D(
        width, height, 4, 1, format,
        VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_TILING_OPTIMAL);
    RETURN_IF_SKIP(InitHostImageCopyTest(image_ci))

    VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;
    VkImageObj image(m_device);
    image.Init(image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    const uint32_t bufferSize = width * height * 4u;
    std::vector<uint8_t> data(bufferSize);

    VkImageSubresourceLayers imageSubresource;
    imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresource.mipLevel = 3u;
    imageSubresource.baseArrayLayer = 0u;
    imageSubresource.layerCount = 1u;

    VkImageToMemoryCopyEXT region = vku::InitStructHelper();
    region.pHostPointer = data.data();
    region.memoryRowLength = 0u;
    region.memoryImageHeight = 0u;
    region.imageSubresource = imageSubresource;
    region.imageOffset = {0u, 0u, 0u};
    region.imageExtent = {4u, 4u, 1u};

    VkCopyImageToMemoryInfoEXT copyImageToMemory = vku::InitStructHelper();
    copyImageToMemory.flags = VK_HOST_IMAGE_COPY_MEMCPY_EXT;
    copyImageToMemory.srcImage = image.handle();
    copyImageToMemory.srcImageLayout = layout;
    copyImageToMemory.regionCount = 1u;
    copyImageToMemory.pRegions = &region;

    vk::CopyImageToMemoryEXT(*m_device, &copyImageToMemory);
}
