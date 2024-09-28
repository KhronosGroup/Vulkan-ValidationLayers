/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveCopyBufferImage : public VkLayerTest {};

TEST_F(PositiveCopyBufferImage, ImageRemainingLayersMaintenance5) {
    TEST_DESCRIPTION(
        "Test copying an image with VkImageSubresourceLayers.layerCount = VK_REMAINING_ARRAY_LAYERS using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 8;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Copy from a to b
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image_a(*m_device, ci, vkt::set_layout);

    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image_b(*m_device, ci, vkt::set_layout);

    m_command_buffer.begin();
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region{};
    copy_region.extent = ci.extent;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 2;
    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copy_region.dstSubresource = copy_region.srcSubresource;

    vk::CmdCopyImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    // layerCount can explicitly list value
    copy_region.dstSubresource.layerCount = 6;
    vk::CmdCopyImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    m_command_buffer.end();
}

TEST_F(PositiveCopyBufferImage, ImageTypeExtentMismatchMaintenance5) {
    TEST_DESCRIPTION("Test copying an image with extent mismatch using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 1, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // Create 1D image
    vkt::Image image_1D(*m_device, ci, vkt::set_layout);

    // 2D image
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    vkt::Image image_2D(*m_device, ci, vkt::set_layout);

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.begin();
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.end();
}

TEST_F(PositiveCopyBufferImage, ImageLayerCount) {
    TEST_DESCRIPTION("Check layerCount in vkCmdCopyImage");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    copyRegion.dstOffset = {32, 32, 0};
    copyRegion.extent = {16, 16, 1};
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copyRegion);
    m_command_buffer.end();
}

TEST_F(PositiveCopyBufferImage, BufferToRemaingImageLayers) {
    TEST_DESCRIPTION("Test vkCmdCopyBufferToImage2 with VK_REMAINING_ARRAY_LAYERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkBufferImageCopy2 region = vku::InitStructHelper();
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyBufferToImageInfo2 copy_buffer_to_image = vku::InitStructHelper();
    copy_buffer_to_image.srcBuffer = buffer.handle();
    copy_buffer_to_image.dstImage = image.handle();
    copy_buffer_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_buffer_to_image.regionCount = 1u;
    copy_buffer_to_image.pRegions = &region;

    m_command_buffer.begin();
    vk::CmdCopyBufferToImage2KHR(m_command_buffer.handle(), &copy_buffer_to_image);
    m_command_buffer.end();
}

TEST_F(PositiveCopyBufferImage, ImageOverlappingMemory) {
    TEST_DESCRIPTION("Validate Copy Image from/to Buffer with overlapping memory");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    auto image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

    VkDeviceSize buff_size = 32 * 32 * 4;
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(buff_size, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                       vkt::no_mem);
    auto buffer_memory_requirements = buffer.memory_requirements();

    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    auto image_memory_requirements = image.memory_requirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_memory_requirements.size + image_memory_requirements.size;
    bool has_memtype = m_device->phy().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }
    mem.init(*m_device, alloc_info);

    buffer.bind_memory(mem, 0);
    image.bind_memory(mem, buffer_memory_requirements.size);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    region.imageExtent = {32, 32, 1};
    m_command_buffer.begin();
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1, &region);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.end();

    auto compressed_image2_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_BC7_UNORM_BLOCK,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_BC7_UNORM_BLOCK, &format_properties);
    if (((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0) ||
        ((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0)) {
        printf("VK_FORMAT_BC7_UNORM_BLOCK with linear tiling not supported - skipping compressed format test.\n");
        return;
    }
    // 1 byte per texel
    buff_size = 32 * 32 * 1;
    vkt::Buffer buffer2(*m_device,
                        vkt::Buffer::CreateInfo(buff_size, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                        vkt::no_mem);
    buffer_memory_requirements = buffer.memory_requirements();

    vkt::Image image2(*m_device, compressed_image2_ci, vkt::no_mem);
    image_memory_requirements = image2.memory_requirements();

    vkt::DeviceMemory mem2;
    alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_memory_requirements.size + image_memory_requirements.size;
    has_memtype = m_device->phy().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }

    mem2.init(*m_device, alloc_info);

    buffer2.bind_memory(mem2, 0);
    image2.bind_memory(mem2, buffer_memory_requirements.size);

    m_command_buffer.begin();
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image2.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer2.handle(), 1, &region);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer2.handle(), image2.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.end();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedImage) {
    TEST_DESCRIPTION("Image copies between compressed and uncompressed images");
    RETURN_IF_SKIP(Init());

    // Verify format support
    // Size-compatible (64-bit) formats. Uncompressed is 64 bits per texel, compressed is 64 bits per 4x4 block (or 4bpt).
    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR) ||
        !FormatFeaturesAreSupported(gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR)) {
        GTEST_SKIP() << "Required formats/features not supported - UncompressedToCompressedImageCopy";
    }

    // Size = 10 * 10 * 64 = 6400
    vkt::Image uncomp_10x10t_image(*m_device, 10, 10, 1, VK_FORMAT_R16G16B16A16_UINT,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    // Size = 40 * 40 * 4  = 6400
    vkt::Image comp_10x10b_40x40t_image(*m_device, 40, 40, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    if (!uncomp_10x10t_image.initialized() || !comp_10x10b_40x40t_image.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces - UncompressedToCompressedImageCopy";
    }

    // Both copies represent the same number of bytes. Bytes Per Texel = 1 for bc6, 16 for uncompressed
    // Copy compressed to uncompressed
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.begin();

    // Copy from uncompressed to compressed
    copy_region.extent = {10, 10, 1};  // Dimensions in (uncompressed) texels
    vk::CmdCopyImage(m_command_buffer.handle(), uncomp_10x10t_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     comp_10x10b_40x40t_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    // The next copy swaps source and dest s.t. we need an execution barrier on for the prior source and an access barrier for
    // prior dest
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = comp_10x10b_40x40t_image.handle();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &image_barrier);

    // And from compressed to uncompressed
    copy_region.extent = {40, 40, 1};  // Dimensions in (compressed) texels
    vk::CmdCopyImage(m_command_buffer.handle(), comp_10x10b_40x40t_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     uncomp_10x10t_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.end();
}

TEST_F(PositiveCopyBufferImage, ImageSubresource) {
    RETURN_IF_SKIP(Init());

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

    m_command_buffer.begin();

    auto cb = m_command_buffer.handle();

    VkImageSubresourceRange src_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageMemoryBarrier image_barriers[2];

    image_barriers[0] = vku::InitStructHelper();
    image_barriers[0].srcAccessMask = 0;
    image_barriers[0].dstAccessMask = 0;
    image_barriers[0].image = image.handle();
    image_barriers[0].subresourceRange = src_range;
    image_barriers[0].oldLayout = init_layout;
    image_barriers[0].newLayout = dst_layout;

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           image_barriers);
    VkClearColorValue clear_color{};
    vk::CmdClearColorImage(cb, image.handle(), dst_layout, &clear_color, 1, &src_range);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    m_command_buffer.begin();

    image_barriers[0].oldLayout = dst_layout;
    image_barriers[0].newLayout = src_layout;

    VkImageSubresourceRange dst_range{VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 3, 1};
    image_barriers[1] = vku::InitStructHelper();
    image_barriers[1].srcAccessMask = 0;
    image_barriers[1].dstAccessMask = 0;
    image_barriers[1].image = image.handle();
    image_barriers[1].subresourceRange = dst_range;
    image_barriers[1].oldLayout = init_layout;
    image_barriers[1].newLayout = dst_layout;

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2,
                           image_barriers);

    vk::CmdCopyImage(cb, image.handle(), src_layout, image.handle(), dst_layout, 1, &region);

    image_barriers[0].oldLayout = src_layout;
    image_barriers[0].newLayout = final_layout;
    image_barriers[1].oldLayout = dst_layout;
    image_barriers[1].newLayout = final_layout;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 2,
                           image_barriers);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
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
    vk::GetBufferMemoryRequirements(device(), src_buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    buffer_mem_alloc.allocationSize *= 2;

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);
    src_buffer.bind_memory(buffer_mem, 0);
    dst_buffer.bind_memory(buffer_mem, 1024);

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

    m_command_buffer.begin();

    vk::CmdCopyBuffer(m_command_buffer, src_buffer.handle(), dst_buffer.handle(), size32(copy_info_list), copy_info_list.data());

    m_command_buffer.end();
}
