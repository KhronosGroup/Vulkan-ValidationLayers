/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"

class PositiveCopyBufferImage : public VkLayerTest {};

constexpr VkImageUsageFlags kSrcDstUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
constexpr VkFormatFeatureFlags kSrcDstFeature = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;

TEST_F(PositiveCopyBufferImage, ImageRemainingLayersMaintenance5) {
    TEST_DESCRIPTION(
        "Test copying an image with VkImageSubresourceLayers.layerCount = VK_REMAINING_ARRAY_LAYERS using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 8, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);

    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);

    m_command_buffer.Begin();
    image_a.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region{};
    copy_region.extent = image_ci.extent;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 2;
    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copy_region.dstSubresource = copy_region.srcSubresource;

    vk::CmdCopyImage(m_command_buffer, image_a, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &copy_region);
    m_command_buffer.FullMemoryBarrier();
    // layerCount can explicitly list value
    copy_region.dstSubresource.layerCount = 6;
    vk::CmdCopyImage(m_command_buffer, image_a, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, Image3DRemainingLayersMaintenance5) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, 2048, transfer_usage);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {4, 4, 1};
    region.bufferOffset = 0;

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageTypeExtentMismatchMaintenance5) {
    TEST_DESCRIPTION("Test copying an image with extent mismatch using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    vkt::Image image_2D(*m_device, image_ci, vkt::set_layout);

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent.height = 1;
    vkt::Image image_1D(*m_device, image_ci, vkt::set_layout);

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, image_1D, VK_IMAGE_LAYOUT_GENERAL, image_2D, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageLayerCount) {
    TEST_DESCRIPTION("Check layerCount in vkCmdCopyImage");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    m_command_buffer.Begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    copyRegion.dstOffset = {32, 32, 0};
    copyRegion.extent = {16, 16, 1};
    vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BufferToRemaingImageLayers) {
    TEST_DESCRIPTION("Test vkCmdCopyBufferToImage2 with VK_REMAINING_ARRAY_LAYERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy2 region = vku::InitStructHelper();
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyBufferToImageInfo2 copy_buffer_to_image = vku::InitStructHelper();
    copy_buffer_to_image.srcBuffer = buffer;
    copy_buffer_to_image.dstImage = image;
    copy_buffer_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_buffer_to_image.regionCount = 1u;
    copy_buffer_to_image.pRegions = &region;

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage2KHR(m_command_buffer, &copy_buffer_to_image);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageOverlappingMemory) {
    TEST_DESCRIPTION("Validate Copy Image from/to Buffer with overlapping memory");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    auto image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

    VkDeviceSize buff_size = 32 * 32 * 4;
    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(buff_size, kSrcDstUsage), vkt::no_mem);
    auto buffer_memory_requirements = buffer.MemoryRequirements();

    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    auto image_memory_requirements = image.MemoryRequirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_memory_requirements.size + image_memory_requirements.size;
    bool has_memtype = m_device->Physical().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }
    mem.init(*m_device, alloc_info);

    buffer.BindMemory(mem, 0);
    image.BindMemory(mem, buffer_memory_requirements.size);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    region.imageExtent = {32, 32, 1};
    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageOverlappingMemoryCompressed) {
    TEST_DESCRIPTION("Validate Copy Image from/to Buffer with overlapping memory");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), VK_FORMAT_BC3_UNORM_BLOCK, &format_properties);
    if (((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0) ||
        ((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0)) {
        GTEST_SKIP() << "VK_FORMAT_BC3_UNORM_BLOCK with linear tiling not supported";
    }

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;
    region.imageExtent = {32, 32, 1};

    auto image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_BC3_UNORM_BLOCK,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

    // 1 byte per texel
    VkDeviceSize buff_size = 32 * 32 * 1;
    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(buff_size, kSrcDstUsage), vkt::no_mem);
    auto buffer_memory_requirements = buffer.MemoryRequirements();

    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    auto image_memory_requirements = image.MemoryRequirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_memory_requirements.size + image_memory_requirements.size;
    bool has_memtype = m_device->Physical().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }

    mem.init(*m_device, alloc_info);

    buffer.BindMemory(mem, 0);
    image.BindMemory(mem, buffer_memory_requirements.size);

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedImage) {
    TEST_DESCRIPTION("Image copies between compressed and uncompressed images");
    RETURN_IF_SKIP(Init());

    // Size-compatible (64-bit) formats. Uncompressed is 64 bits per texel, compressed is 64 bits per 4x4 block (or 4bpt).
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Size = 10 * 10 * 64 = 6400
    vkt::Image uncomp_image(*m_device, 10, 10, VK_FORMAT_R16G16B16A16_UINT, kSrcDstUsage);
    // Size = 40 * 40 * 4  = 6400
    vkt::Image comp_image(*m_device, 40, 40, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, kSrcDstUsage);

    if (!uncomp_image.initialized() || !comp_image.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    // Both copies represent the same number of bytes. Bytes Per Texel = 1 for bc6, 16 for uncompressed
    // Copy compressed to uncompressed
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();

    // Copy from uncompressed to compressed
    copy_region.extent = {10, 10, 1};  // Dimensions in (uncompressed) texels
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();
    // And from compressed to uncompressed
    copy_region.extent = {40, 40, 1};  // Dimensions in (compressed) texels
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedDstOffset) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Image uncomp_image(*m_device, 8, 8, VK_FORMAT_R16G16B16A16_UINT, kSrcDstUsage);
    // create a texel block of {5,1,1}
    vkt::Image comp_image(*m_device, 20, 4, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, kSrcDstUsage);

    VkImageCopy copy_region = {};
    copy_region.extent = {1, 1, 1};  // copy single texel block
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();
    copy_region.dstOffset.x = 4;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset.x = 8;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset.x = 12;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset.x = 16;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.extent.width = 2;
    copy_region.dstOffset.x = 4;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.extent.width = 3;
    copy_region.dstOffset.x = 8;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CompressedToUncompressedDstOffset) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Image uncomp_image(*m_device, 5, 1, VK_FORMAT_R16G16B16A16_UINT, kSrcDstUsage);
    // create a {8,8,1} texel block
    vkt::Image comp_image(*m_device, 32, 32, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, kSrcDstUsage);

    VkImageCopy copy_region = {};
    copy_region.extent = {4, 4, 1};  // copy single texel block
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();
    copy_region.dstOffset.x = 1;
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset.x = 2;
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset.x = 3;
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset.x = 4;
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.extent.width = 8;
    copy_region.dstOffset.x = 1;
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.extent.width = 12;
    copy_region.dstOffset.x = 2;
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedImage2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-Docs/issues/593");
    RETURN_IF_SKIP(Init());

    // Size-compatible (64-bit) formats. Uncompressed is 64 bits per texel, compressed is 64 bits per 4x4 block (or 4bpt).
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Size = 3 * 3 * 64 = 576
    vkt::Image uncomp_image(*m_device, 3, 3, VK_FORMAT_R16G16B16A16_UINT, kSrcDstUsage);
    // Size = 12 * 12 * 4  = 576
    vkt::Image comp_image(*m_device, 12, 12, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, kSrcDstUsage);

    if (!uncomp_image.initialized() || !comp_image.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {3, 3, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedNonPowerOfTwo) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Image uncomp_image(*m_device, 4, 4, VK_FORMAT_R16G16B16A16_UINT, kSrcDstUsage);
    // create a texel block of {2,1,1}
    vkt::Image comp_image(*m_device, 6, 4, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, kSrcDstUsage);

    VkImageCopy copy_region = {};
    copy_region.extent = {1, 1, 1};  // copy single texel block
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {4, 0, 0};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.dstOffset.x = 0;
    copy_region.extent.width = 2;
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CompressedNonPowerOfTwo) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC2_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Image image_bc2(*m_device, 60, 60, VK_FORMAT_BC2_UNORM_BLOCK, kSrcDstUsage);
    vkt::Image image_bc3(*m_device, 60, 60, VK_FORMAT_BC3_UNORM_BLOCK, kSrcDstUsage);

    if (!image_bc2.initialized() || !image_bc3.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {16, 16, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, image_bc2, VK_IMAGE_LAYOUT_GENERAL, image_bc3, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CompressedNonPowerOfTwo2) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_BC3_SRGB_BLOCK, kSrcDstUsage);
    vkt::Image image_1(*m_device, image_ci, vkt::set_layout);
    image_ci.extent = {62, 62, 1};  // slightly smaller and not divisible by block size
    vkt::Image image_2(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {62, 60, 1};  // dst width

    vk::CmdCopyImage(m_command_buffer, image_2, VK_IMAGE_LAYOUT_GENERAL, image_1, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.extent = {60, 62, 1};  // dst height
    vk::CmdCopyImage(m_command_buffer, image_2, VK_IMAGE_LAYOUT_GENERAL, image_1, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.srcOffset = {8, 8, 0};
    copy_region.dstOffset = {8, 8, 0};
    copy_region.extent = {54, 52, 1};  // dst width
    vk::CmdCopyImage(m_command_buffer, image_2, VK_IMAGE_LAYOUT_GENERAL, image_1, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
}

TEST_F(PositiveCopyBufferImage, BufferCopyNonPowerOfTwo) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Mips are    0       1      2      3     4
    //          [15,16], [7,8], [3,4], [1,2], [1,1]
    auto image_ci = vkt::Image::ImageCreateInfo2D(15, 16, 5, 1, VK_FORMAT_BC3_SRGB_BLOCK, kSrcDstUsage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};

    m_command_buffer.Begin();
    // Offset + extent width = mip width - should succeed
    region.imageOffset = {8, 8, 0};
    region.imageExtent = {7, 8, 1};
    region.imageSubresource.mipLevel = 0;

    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    region.imageOffset = {4, 4, 0};
    region.imageExtent = {3, 4, 1};
    region.imageSubresource.mipLevel = 1;

    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {3, 4, 1};
    region.imageSubresource.mipLevel = 2;

    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {1, 2, 1};
    region.imageSubresource.mipLevel = 3;

    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10125
// Need to rewrite test to work everywhere
TEST_F(PositiveCopyBufferImage, DISABLED_OverlappingRegion) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9537");
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(4, 4, 1, 1, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, kSrcDstUsage);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 8;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(*m_device, image, &image_mem_reqs);

    VkMemoryAllocateInfo mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mem_alloc.allocationSize = 1 << 16;
    vkt::DeviceMemory memory(*m_device, mem_alloc);

    buffer.BindMemory(memory, 320);
    image.BindMemory(memory, 256);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 128;
    region.bufferImageHeight = 4;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {4, 4, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10125
// Need to rewrite test to work everywhere
TEST_F(PositiveCopyBufferImage, DISABLED_OverlappingRegion2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9276");
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(2, 2, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 512;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(*m_device, image, &image_mem_reqs);

    VkMemoryAllocateInfo mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mem_alloc.allocationSize = 1 << 14;
    vkt::DeviceMemory memory(*m_device, mem_alloc);

    buffer.BindMemory(memory, 800);
    image.BindMemory(memory, 544);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 64;
    region.bufferImageHeight = 2;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {2, 2, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BufferToCompressedNonPowerOfTwo) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC2_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }
    vkt::Image image(*m_device, 7, 7, VK_FORMAT_BC2_UNORM_BLOCK, kSrcDstUsage);
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copy = {};
    copy.imageSubresource = {1u, 0u, 0u, 1u};
    copy.imageOffset = {0, 0, 0};
    copy.imageExtent = {7u, 7u, 1u};

    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageSubresource) {
    RETURN_IF_SKIP(Init());

    VkImageUsageFlags usage = kSrcDstUsage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 2, 5, format, usage);
    vkt::Image image(*m_device, image_ci);

    VkImageSubresourceLayers src_layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers dst_layer{VK_IMAGE_ASPECT_COLOR_BIT, 1, 3, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128 / 2, 128 / 2, 1};  // <-- image type is 2D
    VkImageCopy region = {src_layer, zero_offset, dst_layer, zero_offset, full_extent};
    auto init_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    auto src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    auto dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    auto final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_command_buffer.Begin();
    auto cb = m_command_buffer.handle();

    VkImageSubresourceRange src_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageMemoryBarrier image_barriers[2];
    const VkImageAspectFlags full_transfer = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

    image_barriers[0] = vku::InitStructHelper();
    image_barriers[0].srcAccessMask = 0;
    image_barriers[0].dstAccessMask = full_transfer;
    image_barriers[0].image = image;
    image_barriers[0].subresourceRange = src_range;
    image_barriers[0].oldLayout = init_layout;
    image_barriers[0].newLayout = dst_layout;

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           image_barriers);
    VkClearColorValue clear_color{};
    vk::CmdClearColorImage(cb, image, dst_layout, &clear_color, 1, &src_range);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    m_command_buffer.Begin();

    image_barriers[0].oldLayout = dst_layout;
    image_barriers[0].newLayout = src_layout;

    VkImageSubresourceRange dst_range{VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 3, 1};
    image_barriers[1] = vku::InitStructHelper();
    image_barriers[1].srcAccessMask = 0;
    image_barriers[1].dstAccessMask = full_transfer;
    image_barriers[1].image = image;
    image_barriers[1].subresourceRange = dst_range;
    image_barriers[1].oldLayout = init_layout;
    image_barriers[1].newLayout = dst_layout;

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2,
                           image_barriers);

    vk::CmdCopyImage(cb, image, src_layout, image, dst_layout, 1, &region);

    image_barriers[0].oldLayout = src_layout;
    image_barriers[0].newLayout = final_layout;
    image_barriers[0].srcAccessMask = full_transfer;
    image_barriers[0].dstAccessMask = 0;
    image_barriers[1].oldLayout = dst_layout;
    image_barriers[1].newLayout = final_layout;
    image_barriers[1].srcAccessMask = full_transfer;
    image_barriers[1].dstAccessMask = 0;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 2,
                           image_barriers);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveCopyBufferImage, CopyCompressed1DImage) {
    TEST_DESCRIPTION("Copy a 1D image with compressed format");
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.format = VK_FORMAT_R16G16B16A16_UNORM;
    image_ci.extent = {256u, 1u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = kSrcDstUsage;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image uncomp_image(*m_device, image_ci);

    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.usage = kSrcDstUsage;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image comp_image(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {4u, 1u, 1u};

    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1u, &image_copy);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1u, &image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, Compressed2DToUncompressed1D) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, kSrcDstUsage);
    vkt::Image comp_image(*m_device, image_ci, vkt::set_layout);

    image_ci.format = VK_FORMAT_R16G16B16A16_UNORM;
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent = {256, 1, 1};
    vkt::Image uncomp_image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};

    // 2D to 1D
    image_copy.extent = {16u, 4u, 1u};
    vk::CmdCopyImage(m_command_buffer, comp_image, VK_IMAGE_LAYOUT_GENERAL, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, 1u, &image_copy);

    m_command_buffer.FullMemoryBarrier();

    // 1D to 2D
    image_copy.extent = {16u, 1u, 1u};
    vk::CmdCopyImage(m_command_buffer, uncomp_image, VK_IMAGE_LAYOUT_GENERAL, comp_image, VK_IMAGE_LAYOUT_GENERAL, 1u, &image_copy);
    m_command_buffer.End();
}

// TODO - Need WG agreement how this works still
TEST_F(PositiveCopyBufferImage, DISABLED_CopyCompress1DTo2D) {
    TEST_DESCRIPTION("Copy a 1D compressed format to a 2D compressed format");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent = {256u, 1u, 1u};
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci);

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {32u, 1u, 1u};

    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyBufferTo1DCompressedImage) {
    TEST_DESCRIPTION("Copy a buffer to 1D image with compressed format");
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.extent = {64u, 1u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci);

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0u;
    buffer_image_copy.bufferRowLength = 64u;
    buffer_image_copy.bufferImageHeight = 4u;
    buffer_image_copy.imageSubresource = {1u, 0u, 0u, 1u};
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {4u, 1u, 1u};

    m_command_buffer.Begin();
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &buffer_image_copy);

    buffer_image_copy.bufferRowLength = 4;
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &buffer_image_copy);

    m_command_buffer.End();
}

// TODO - Need WG agreement how this works still
TEST_F(PositiveCopyBufferImage, DISABLED_CopyCompress2DTo1D) {
    TEST_DESCRIPTION("Copy a compressed 2D image to a compressed 1D image");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0u;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.extent = {64u, 64u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci);

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent = {1024u, 1u, 1u};
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {64u, 4u, 1u};

    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyCompressLowestMip) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1946");
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0u;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R16G16B16A16_UINT;
    image_ci.extent = {8u, 8u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci);

    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.mipLevels = 4u;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 2u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {1u, 1u, 1u};  // copy single texel

    // 1 texel block
    image_copy.dstSubresource.mipLevel = 1u;
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);

    // sub texel block
    image_copy.dstSubresource.mipLevel = 2u;
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);

    image_copy.dstSubresource.mipLevel = 3u;
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CompressedImageMip) {
    TEST_DESCRIPTION("Image/Buffer copies for higher mip levels");
    RETURN_IF_SKIP(Init());
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 6, 1, VK_FORMAT_BC3_SRGB_BLOCK, kSrcDstUsage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_1024(*m_device, 1024, transfer_usage);
    vkt::Buffer buffer_64(*m_device, 64, transfer_usage);
    vkt::Buffer buffer_16(*m_device, 16, transfer_usage);
    vkt::Buffer buffer_8(*m_device, 8, transfer_usage);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    m_command_buffer.Begin();
    // Mip level copies that work - 5 levels

    // Mip 0 should fit in 1k buffer - 1k texels @ 1b each
    region.imageExtent = {32, 32, 1};
    region.imageSubresource.mipLevel = 0;
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer_1024, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_1024, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 2 should fit in 64b buffer - 64 texels @ 1b each
    region.imageExtent = {8, 8, 1};
    region.imageSubresource.mipLevel = 2;
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer_64, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_64, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 3 should fit in 16b buffer - 16 texels @ 1b each
    region.imageExtent = {4, 4, 1};
    region.imageSubresource.mipLevel = 3;
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer_16, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_16, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 4&5 should fit in 16b buffer with no complaint - 4 & 1 texels @ 1b each
    // When dealing with mips smaller than the block texel size, need to be full size
    region.imageExtent = {2, 2, 1};
    region.imageSubresource.mipLevel = 4;
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer_16, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_16, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    region.imageExtent = {1, 1, 1};
    region.imageSubresource.mipLevel = 5;
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer_16, 1, &region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_16, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
}

// TODO - Need WG agreement how this works still
TEST_F(PositiveCopyBufferImage, DISABLED_CopyCompressToCompress) {
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.mipLevels = 4u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.extent = {16, 16u, 1u};
    image_ci.usage = kSrcDstUsage;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, kSrcDstFeature)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci);
    vkt::Image dst_image(*m_device, image_ci);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0u, 0u, 0u};
    image_copy.dstSubresource = {1u, 3u, 0u, 1u};  // last mip
    image_copy.dstOffset = {0u, 0u, 0u};
    image_copy.extent = {2u, 2u, 1u};  // single texel

    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyBufferToCompressedNonFullTexelBlock) {
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.extent = {22u, 22u, 1u};
    image_ci.mipLevels = 3u;
    image_ci.arrayLayers = 3u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci);
    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0u;
    buffer_image_copy.bufferRowLength = 24u;
    buffer_image_copy.bufferImageHeight = 24u;
    buffer_image_copy.imageSubresource = {1u, 0u, 0u, 1u};
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {22u, 22u, 1u};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, dst_image, VK_IMAGE_LAYOUT_GENERAL, 1u, &buffer_image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyBufferToCompressedMipLevels) {
    RETURN_IF_SKIP(Init());

    // 1 texel block is 8 bytes
    vkt::Buffer buffer(*m_device, 8u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.extent = {8u, 8u, 1u};
    image_ci.mipLevels = 4u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci);

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0u;
    buffer_image_copy.bufferRowLength = 4u;
    buffer_image_copy.bufferImageHeight = 4u;
    buffer_image_copy.imageSubresource = {1u, 2u, 0u, 1u};
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {2u, 2u, 1u};

    m_command_buffer.Begin();
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &buffer_image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyBufferSizePlanar) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = kSrcDstUsage | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_ci.extent = {4, 4, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;

    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, kSrcDstFeature)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }
    vkt::Image image(*m_device, image_ci);
    vkt::Buffer buffer_0(*m_device, 48, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_1(*m_device, 12, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copy = {};
    copy.imageSubresource.layerCount = 1;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy.imageExtent = {4, 4, 1};
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_0, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy.imageExtent = {2, 2, 1};
    vk::CmdCopyBufferToImage(m_command_buffer, buffer_1, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyImagePlanar) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = kSrcDstUsage | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_ci.extent = {4, 4, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, kSrcDstFeature)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }
    vkt::Image image(*m_device, image_ci);

    image_ci.usage = kSrcDstUsage;
    image_ci.format = VK_FORMAT_R8_UNORM;
    vkt::Image image_p0(*m_device, image_ci);
    image_ci.format = VK_FORMAT_R8G8_UNORM;
    vkt::Image image_p1(*m_device, image_ci);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {4, 4, 1};

    m_command_buffer.Begin();
    // copy into multi-planar
    {
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        vk::CmdCopyImage(m_command_buffer, image_p0, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        copy_region.extent = {2, 2, 1};
        vk::CmdCopyImage(m_command_buffer, image_p1, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        m_command_buffer.FullMemoryBarrier();
    }
    // offset only 1 texel
    {
        copy_region.srcOffset = {1, 1, 0};
        copy_region.dstOffset = {3, 3, 0};
        copy_region.extent = {1, 1, 1};
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        vk::CmdCopyImage(m_command_buffer, image_p0, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        copy_region.dstOffset = {1, 1, 0};
        vk::CmdCopyImage(m_command_buffer, image_p1, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        m_command_buffer.FullMemoryBarrier();
    }

    // copy from multi-planar
    {
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.srcOffset = {0, 0, 0};
        copy_region.dstOffset = {0, 0, 0};
        copy_region.extent = {4, 4, 1};

        copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, image_p0, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

        copy_region.extent = {2, 2, 1};
        copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, image_p1, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    }
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BufferCopiesStressTest) {
    TEST_DESCRIPTION("Do many buffer copies, make sure perf is good");

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 1024;
    vkt::Buffer src_buffer(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer dst_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), src_buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    buffer_mem_alloc.allocationSize *= 2;

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);
    src_buffer.BindMemory(buffer_mem, 0);
    dst_buffer.BindMemory(buffer_mem, 1024);

    constexpr VkDeviceSize copy_size = 1024 / 4;
    std::vector<VkBufferCopy> copy_info_list(4);
    copy_info_list[3].srcOffset = 0;
    copy_info_list[3].dstOffset = 0;
    copy_info_list[3].size = copy_size;

    copy_info_list[2].srcOffset = copy_size;
    copy_info_list[2].dstOffset = copy_size;
    copy_info_list[2].size = copy_size;

    copy_info_list[1].srcOffset = 2 * copy_size;
    copy_info_list[1].dstOffset = 2 * copy_size;
    copy_info_list[1].size = copy_size;

    copy_info_list[0].srcOffset = 3 * copy_size;
    copy_info_list[0].dstOffset = 3 * copy_size;
    copy_info_list[0].size = copy_size;

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + 4] = copy_info_list[i % 4];
    }

    m_command_buffer.Begin();

    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, size32(copy_info_list), copy_info_list.data());

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyColorToDepthMaintenanc8) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required VK_FORMAT_R32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT features not supported";
    }

    vkt::Image color_image(*m_device, 128, 128, VK_FORMAT_R32_SFLOAT, kSrcDstUsage);
    vkt::Image depth_image(*m_device, 128, 128, VK_FORMAT_D32_SFLOAT, kSrcDstUsage);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {64, 64, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, color_image, VK_IMAGE_LAYOUT_GENERAL, depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyColorToStencilMaintenanc8Compatible) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9588");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UINT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, kSrcDstFeature)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT_S8_UINT features not supported";
    }

    vkt::Image color_image(*m_device, 128, 128, VK_FORMAT_R8_UINT, kSrcDstUsage);
    vkt::Image ds_image(*m_device, 128, 128, VK_FORMAT_D32_SFLOAT_S8_UINT, kSrcDstUsage);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {64, 64, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, color_image, VK_IMAGE_LAYOUT_GENERAL, ds_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdCopyImage(m_command_buffer, ds_image, VK_IMAGE_LAYOUT_GENERAL, color_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageBufferCopyDepthStencil) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Verify all needed Depth/Stencil formats are supported
    bool missing_ds_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), VK_FORMAT_D32_SFLOAT_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), VK_FORMAT_D24_UNORM_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), VK_FORMAT_D16_UNORM, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), VK_FORMAT_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (missing_ds_support) {
        GTEST_SKIP() << "Depth / Stencil formats unsupported";
    }

    // 256^2 texels, 512kb (256k depth, 64k stencil, 192k pack)
    vkt::Image ds_image_4D_1S(*m_device, 256, 256, VK_FORMAT_D32_SFLOAT_S8_UINT,
                              kSrcDstUsage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // 256^2 texels, 256kb (192k depth, 64k stencil)
    vkt::Image ds_image_3D_1S(*m_device, 256, 256, VK_FORMAT_D24_UNORM_S8_UINT,
                              kSrcDstUsage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // 256^2 texels, 128k (128k depth)
    vkt::Image ds_image_2D(*m_device, 256, 256, VK_FORMAT_D16_UNORM, kSrcDstUsage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // 256^2 texels, 64k (64k stencil)
    vkt::Image ds_image_1S(*m_device, 256, 256, VK_FORMAT_S8_UINT, kSrcDstUsage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_256k(*m_device, 262144, transfer_usage);  // 256k
    vkt::Buffer buffer_128k(*m_device, 131072, transfer_usage);  // 128k
    vkt::Buffer buffer_64k(*m_device, 65536, transfer_usage);    // 64k

    VkBufferImageCopy ds_region = {};
    ds_region.bufferOffset = 0;
    ds_region.bufferRowLength = 0;
    ds_region.bufferImageHeight = 0;
    ds_region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    ds_region.imageOffset = {0, 0, 0};
    ds_region.imageExtent = {256, 256, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, ds_image_4D_1S, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer_256k, 1, &ds_region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, ds_image_3D_1S, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer_256k, 1, &ds_region);

    vk::CmdCopyImageToBuffer(m_command_buffer, ds_image_2D, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer_128k, 1, &ds_region);

    // Stencil
    ds_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, ds_image_4D_1S, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer_64k, 1, &ds_region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, ds_image_3D_1S, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer_64k, 1, &ds_region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyImageToBuffer(m_command_buffer, ds_image_1S, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer_64k, 1, &ds_region);
}

TEST_F(PositiveCopyBufferImage, MissingQueueGraphicsSupport) {
    TEST_DESCRIPTION("Copy from image with depth aspect when queue does not support graphics");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);

    vkt::Image src_image(*m_device, 32u, 32u, VK_FORMAT_D16_UNORM,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::Image dst_image(*m_device, 32u, 32u, VK_FORMAT_D16_UNORM,
                         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    command_buffer.Begin();

    VkImageCopy image_copy;
    image_copy.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {32u, 32u, 1u};

    vk::CmdCopyImage(command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);

    command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BlitDepth) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 2u, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    vkt::Image src_image(*m_device, image_ci);
    vkt::Image dst_image(*m_device, image_ci);

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.arrayLayers = 1u;
    image_ci.extent.depth = 2u;
    vkt::Image src_image_3d(*m_device, image_ci);
    vkt::Image dst_image_3d(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageBlit region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, VK_REMAINING_ARRAY_LAYERS};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffsets[0] = {0, 0, 0};
    region.srcOffsets[1] = {32, 32, 1};
    region.dstOffsets[0] = {0, 0, 0};
    region.dstOffsets[1] = {32, 32, 1};

    vk::CmdBlitImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image_3d,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region, VK_FILTER_NEAREST);

    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, VK_REMAINING_ARRAY_LAYERS};
    region.srcOffsets[0] = {0, 0, 0};
    region.srcOffsets[1] = {32, 32, 1};
    region.dstOffsets[0] = {0, 0, 0};
    region.dstOffsets[1] = {32, 32, 1};
    vk::CmdBlitImage(m_command_buffer, src_image_3d, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region, VK_FILTER_NEAREST);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BlitDepth2DArray) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7261");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 4u, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    vkt::Image src_image(*m_device, image_ci);
    vkt::Image dst_image(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageBlit region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.srcOffsets[0] = {0, 0, 0};
    region.srcOffsets[1] = {32, 32, 1};
    region.dstOffsets[0] = {0, 0, 0};
    region.dstOffsets[1] = {32, 32, 1};

    vk::CmdBlitImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region, VK_FILTER_NEAREST);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, Image2DArrayTo3D) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 4, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    vkt::Image image_2D(*m_device, image_ci, vkt::set_layout);

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.arrayLayers = 1;
    image_ci.extent.depth = 4;
    vkt::Image image_3D(*m_device, image_ci, vkt::set_layout);

    VkImageCopy copy_region;
    copy_region.extent = {4, 4, 4};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 4};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, image_2D, VK_IMAGE_LAYOUT_GENERAL, image_3D, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 4};
    vk::CmdCopyImage(m_command_buffer, image_3D, VK_IMAGE_LAYOUT_GENERAL, image_2D, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copy_region.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    vk::CmdCopyImage(m_command_buffer, image_3D, VK_IMAGE_LAYOUT_GENERAL, image_2D, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, Image2DArrayTo3DMiplevel) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    // mips from 8x8 to 1x1
    auto image_ci = vkt::Image::ImageCreateInfo2D(8, 8, 4, 4, VK_FORMAT_R8G8B8A8_UNORM, kSrcDstUsage);
    vkt::Image image_2D(*m_device, image_ci, vkt::set_layout);

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.arrayLayers = 1;
    image_ci.extent.depth = 8;
    vkt::Image image_3D(*m_device, image_ci, vkt::set_layout);

    VkImageCopy copy_region;
    copy_region.extent = {2, 2, 2};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 2, 2, 2};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 2, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, image_2D, VK_IMAGE_LAYOUT_GENERAL, image_3D, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();

    copy_region.extent = {1, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 2, 3, 1};
    vk::CmdCopyImage(m_command_buffer, image_3D, VK_IMAGE_LAYOUT_GENERAL, image_2D, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, Transition3dImage) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {16u, 16u, 4u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = kSrcDstUsage;
    vkt::Image image(*m_device, image_ci);
    vkt::Buffer buffer(*m_device, 4096u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {16u, 16u, 4u};

    VkImageMemoryBarrier image_memory_barrier = vku::InitStructHelper();
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0u;
    image_memory_barrier.subresourceRange.levelCount = 1u;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0u;
    image_memory_barrier.subresourceRange.layerCount = 4u;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &image_memory_barrier);
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveCopyBufferImage, Transition3dImageSlices) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {16u, 16u, 4u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = kSrcDstUsage;
    vkt::Image image(*m_device, image_ci);
    vkt::Buffer buffer(*m_device, 4096u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 2};
    region.imageExtent = {16u, 16u, 2u};

    m_command_buffer.Begin();

    VkImageMemoryBarrier image_memory_barrier = vku::InitStructHelper();
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0u;
    image_memory_barrier.subresourceRange.levelCount = 1u;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0u;
    image_memory_barrier.subresourceRange.layerCount = 4u;

    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &image_memory_barrier);

    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_memory_barrier.subresourceRange.baseArrayLayer = 2u;
    image_memory_barrier.subresourceRange.layerCount = 2u;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &image_memory_barrier);

    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1u, &region);

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}
