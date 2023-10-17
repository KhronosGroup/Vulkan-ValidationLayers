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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "utils/vk_layer_utils.h"
#include "error_message/validation_error_enums.h"

TEST_F(NegativeImage, UsageBits) {
    TEST_DESCRIPTION(
        "Specify wrong usage for image then create conflicting view of image Initialize buffer with wrong usage then perform copy "
        "expecting errors from both the image and the buffer (2 calls)");

    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    auto format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj image(m_device);
    // Initialize image with transfer source usage
    image.Init(128, 128, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo dsvci = vku::InitStructHelper();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    dsvci.format = format;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    // Create a view with depth / stencil aspect for image with different usage
    CreateImageViewTest(*this, &dsvci, "VUID-VkImageViewCreateInfo-image-04441");

    // Initialize buffer with TRANSFER_DST usage
    vkt::Buffer buffer(*m_device, 128 * 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 128;
    region.bufferImageHeight = 128;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.height = 16;
    region.imageExtent.width = 16;
    region.imageExtent.depth = 1;

    // Buffer usage not set to TRANSFER_SRC and image usage not set to TRANSFER_DST
    m_commandBuffer->begin();

    // two separate errors from this call:
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-00177");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-srcBuffer-00174");

    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // equvalent test using using KHR_copy_commands2
    if (copy_commands2) {
        const VkBufferImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR,
                                               NULL,
                                               region.bufferRowLength,
                                               region.bufferImageHeight,
                                               region.bufferImageHeight,
                                               region.imageSubresource,
                                               region.imageOffset,
                                               region.imageExtent};
        VkCopyBufferToImageInfo2KHR buffer_to_image_info2 = {VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR,
                                                             NULL,
                                                             buffer.handle(),
                                                             image.handle(),
                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                             1,
                                                             &region2};
        // two separate errors from this call:
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-dstImage-00177");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-srcBuffer-00174");
        vk::CmdCopyBufferToImage2KHR(m_commandBuffer->handle(), &buffer_to_image_info2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImage, CopyBufferToCompressedImage) {
    TEST_DESCRIPTION("Copy buffer to compressed image when buffer is larger than image.");
    RETURN_IF_SKIP(Init())

    // Verify format support
    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageObj width_image(m_device);
    VkImageObj height_image(m_device);
    vkt::Buffer buffer(*m_device, 8 * 4 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = 8;
    region.imageExtent.height = 4;
    region.imageExtent.depth = 1;

    width_image.Init(5, 4, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    height_image.Init(8, 3, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    if (!width_image.initialized() || (!height_image.initialized())) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }
    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), width_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07970");

    VkResult err;
    VkImageCreateInfo depth_image_create_info = vku::InitStructHelper();
    depth_image_create_info.imageType = VK_IMAGE_TYPE_3D;
    depth_image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    depth_image_create_info.extent.width = 8;
    depth_image_create_info.extent.height = 4;
    depth_image_create_info.extent.depth = 1;
    depth_image_create_info.mipLevels = 1;
    depth_image_create_info.arrayLayers = 1;
    depth_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    depth_image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    depth_image_create_info.queueFamilyIndexCount = 0;
    depth_image_create_info.pQueueFamilyIndices = NULL;

    VkImage depth_image = VK_NULL_HANDLE;
    err = vk::CreateImage(m_device->handle(), &depth_image_create_info, NULL, &depth_image);
    ASSERT_EQ(VK_SUCCESS, err);

    VkDeviceMemory mem1;
    VkMemoryRequirements mem_reqs;
    mem_reqs.memoryTypeBits = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.memoryTypeIndex = 1;
    vk::GetImageMemoryRequirements(m_device->device(), depth_image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem1);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::BindImageMemory(m_device->device(), depth_image, mem1, 0);

    region.imageExtent.depth = 2;
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), depth_image, NULL);
    vk::FreeMemory(m_device->device(), mem1, NULL);
    m_commandBuffer->end();
}

TEST_F(NegativeImage, UnknownObject) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-parameter");

    TEST_DESCRIPTION("Pass an invalid image object handle into a Vulkan API call.");

    RETURN_IF_SKIP(Init())

    // Pass bogus handle into GetImageMemoryRequirements
    VkMemoryRequirements mem_reqs;
    constexpr uint64_t fakeImageHandle = 0xCADECADE;
    VkImage fauxImage = CastFromUint64<VkImage>(fakeImageHandle);

    vk::GetImageMemoryRequirements(m_device->device(), fauxImage, &mem_reqs);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, SampleCounts) {
    TEST_DESCRIPTION("Use bad sample counts in image transfer calls to trigger validation errors.");
    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.flags = 0;

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {256, 256, 1};
    blit_region.dstOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[1] = {128, 128, 1};

    // Create two images, the source with sampleCount = 4, and attempt to blit
    // between them
    {
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkImageObj src_image(m_device);
        src_image.init(&image_create_info);
        src_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageObj dst_image(m_device);
        dst_image.init(&image_create_info);
        dst_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00233");
        vk::CmdBlitImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }

    // Create two images, the dest with sampleCount = 4, and attempt to blit
    // between them
    {
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkImageObj src_image(m_device);
        src_image.init(&image_create_info);
        src_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageObj dst_image(m_device);
        dst_image.init(&image_create_info);
        dst_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00234");
        vk::CmdBlitImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }

    VkBufferImageCopy copy_region = {};
    copy_region.bufferRowLength = 128;
    copy_region.bufferImageHeight = 128;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.height = 64;
    copy_region.imageExtent.width = 64;
    copy_region.imageExtent.depth = 1;

    // Create src buffer and dst image with sampleCount = 4 and attempt to copy
    // buffer to image
    {
        vkt::Buffer src_buffer(*m_device, 128 * 128 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageObj dst_image(m_device);
        dst_image.init(&image_create_info);
        dst_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "was created with a sample count of VK_SAMPLE_COUNT_4_BIT but must be VK_SAMPLE_COUNT_1_BIT");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), src_buffer.handle(), dst_image.handle(),
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }

    // Create dst buffer and src image with sampleCount = 4 and attempt to copy
    // image to buffer
    {
        vkt::Buffer dst_buffer(*m_device, 128 * 128 * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        vkt::Image src_image;
        src_image.init(*m_device, (const VkImageCreateInfo &)image_create_info, 0);
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "was created with a sample count of VK_SAMPLE_COUNT_4_BIT but must be VK_SAMPLE_COUNT_1_BIT");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 dst_buffer.handle(), 1, &copy_region);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeImage, BlitFormatTypes) {
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    VkFormat f_unsigned = VK_FORMAT_R8G8B8A8_UINT;
    VkFormat f_signed = VK_FORMAT_R8G8B8A8_SINT;
    VkFormat f_float = VK_FORMAT_R32_SFLOAT;
    VkFormat f_depth = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat f_depth2 = VK_FORMAT_D32_SFLOAT;
    VkFormat f_ycbcr = VK_FORMAT_B16G16R16G16_422_UNORM;

    if (!FormatIsSupported(gpu(), f_unsigned, VK_IMAGE_TILING_OPTIMAL) ||
        !FormatIsSupported(gpu(), f_signed, VK_IMAGE_TILING_OPTIMAL) ||
        !FormatIsSupported(gpu(), f_float, VK_IMAGE_TILING_OPTIMAL) ||
        !FormatIsSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL) ||
        !FormatIsSupported(gpu(), f_depth2, VK_IMAGE_TILING_OPTIMAL)) {
        GTEST_SKIP() << "Requested formats not supported";
    }

    // Note any missing feature bits
    bool usrc = !FormatFeaturesAreSupported(gpu(), f_unsigned, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    bool udst = !FormatFeaturesAreSupported(gpu(), f_unsigned, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool ssrc = !FormatFeaturesAreSupported(gpu(), f_signed, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    bool sdst = !FormatFeaturesAreSupported(gpu(), f_signed, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool fsrc = !FormatFeaturesAreSupported(gpu(), f_float, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    bool fdst = !FormatFeaturesAreSupported(gpu(), f_float, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool d1dst = !FormatFeaturesAreSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool d2src = !FormatFeaturesAreSupported(gpu(), f_depth2, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);

    VkImageObj unsigned_image(m_device);
    unsigned_image.Init(64, 64, 1, f_unsigned, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                        VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(unsigned_image.initialized());
    unsigned_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj signed_image(m_device);
    signed_image.Init(64, 64, 1, f_signed, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(signed_image.initialized());
    signed_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj float_image(m_device);
    float_image.Init(64, 64, 1, f_float, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL,
                     0);
    ASSERT_TRUE(float_image.initialized());
    float_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj depth_image(m_device);
    depth_image.Init(64, 64, 1, f_depth, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL,
                     0);
    ASSERT_TRUE(depth_image.initialized());
    depth_image.SetLayout(VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj depth_image2(m_device);
    depth_image2.Init(64, 64, 1, f_depth2, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(depth_image2.initialized());
    depth_image2.SetLayout(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {64, 64, 1};
    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {32, 32, 1};

    m_commandBuffer->begin();

    // Unsigned int vs not an int
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (usrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), unsigned_image.image(), unsigned_image.Layout(), float_image.image(),
                     float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageBlit2KHR blitRegion2 = {
            VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR, NULL,
            blitRegion.srcSubresource,          {blitRegion.srcOffsets[0], blitRegion.srcOffsets[1]},
            blitRegion.dstSubresource,          {blitRegion.dstOffsets[0], blitRegion.dstOffsets[1]}};
        const VkBlitImageInfo2KHR blit_image_info2 = {VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      unsigned_image.image(),
                                                      unsigned_image.Layout(),
                                                      float_image.image(),
                                                      float_image.Layout(),
                                                      1,
                                                      &blitRegion2,
                                                      VK_FILTER_NEAREST};
        // Unsigned int vs not an int
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-srcImage-00230");
        if (usrc) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-srcImage-01999");
        if (fdst) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-dstImage-02000");
        vk::CmdBlitImage2KHR(m_commandBuffer->handle(), &blit_image_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), unsigned_image.image(),
                     unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageBlit2KHR blitRegion2 = {
            VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR, NULL,
            blitRegion.srcSubresource,          {blitRegion.srcOffsets[0], blitRegion.srcOffsets[1]},
            blitRegion.dstSubresource,          {blitRegion.dstOffsets[0], blitRegion.dstOffsets[1]}};
        const VkBlitImageInfo2KHR blit_image_info2 = {VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      float_image.image(),
                                                      float_image.Layout(),
                                                      unsigned_image.image(),
                                                      unsigned_image.Layout(),
                                                      1,
                                                      &blitRegion2,
                                                      VK_FILTER_NEAREST};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-srcImage-00230");
        if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-srcImage-01999");
        if (udst) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-dstImage-02000");
        vk::CmdBlitImage2KHR(m_commandBuffer->handle(), &blit_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Signed int vs not an int,
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), float_image.image(),
                     float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageBlit2KHR blitRegion2 = {
            VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR, NULL,
            blitRegion.srcSubresource,          {blitRegion.srcOffsets[0], blitRegion.srcOffsets[1]},
            blitRegion.dstSubresource,          {blitRegion.dstOffsets[0], blitRegion.dstOffsets[1]}};
        const VkBlitImageInfo2KHR blit_image_info2 = {VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      signed_image.image(),
                                                      signed_image.Layout(),
                                                      float_image.image(),
                                                      float_image.Layout(),
                                                      1,
                                                      &blitRegion2,
                                                      VK_FILTER_NEAREST};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-srcImage-00229");
        if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-srcImage-01999");
        if (fdst) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-dstImage-02000");
        vk::CmdBlitImage2KHR(m_commandBuffer->handle(), &blit_image_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (sdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), signed_image.image(),
                     signed_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Signed vs Unsigned int - generates both VUs
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), unsigned_image.image(),
                     unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (usrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (sdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), unsigned_image.image(), unsigned_image.Layout(), signed_image.image(),
                     signed_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    if (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) &&
        FormatIsSupported(gpu(), f_ycbcr, VK_IMAGE_TILING_OPTIMAL)) {
        bool ycbcrsrc = !FormatFeaturesAreSupported(gpu(), f_ycbcr, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
        bool ycbcrdst = !FormatFeaturesAreSupported(gpu(), f_ycbcr, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);

        VkImageObj ycbcr_image(m_device);
        ycbcr_image.Init(64, 64, 1, f_ycbcr, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ycbcr_image.initialized());
        ycbcr_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

        VkImageObj ycbcr_image_2(m_device);
        ycbcr_image_2.Init(64, 64, 1, f_ycbcr, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ycbcr_image_2.initialized());
        ycbcr_image_2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

        // Src, dst is ycbcr format
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-06421");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-06422");
        if (ycbcrsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
        if (ycbcrdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
        vk::CmdBlitImage(m_commandBuffer->handle(), ycbcr_image.image(), ycbcr_image.Layout(), ycbcr_image_2.image(),
                         ycbcr_image_2.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();
    } else {
        printf("Requested ycbcr format not supported - skipping test case.\n");
    }

    // Depth vs any non-identical depth format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00231");
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (d2src) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (d1dst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), depth_image2.image(), depth_image2.Layout(), depth_image.image(),
                     depth_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, BlitFilters) {
    AddOptionalExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool cubic_support = IsExtensionsEnabled(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);

    VkFormat fmt = VK_FORMAT_R8_UINT;
    if (!FormatIsSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL)) {
        GTEST_SKIP() << "No R8_UINT format support";
    }

    // Create 2D images
    VkImageObj src2D(m_device);
    VkImageObj dst2D(m_device);
    src2D.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    dst2D.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(src2D.initialized());
    ASSERT_TRUE(dst2D.initialized());
    src2D.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    dst2D.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    // Create 3D image
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = fmt;
    ci.extent = {64, 64, 4};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj src3D(m_device);
    src3D.init(&ci);
    ASSERT_TRUE(src3D.initialized());

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {48, 48, 1};
    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // UINT format should not support linear filtering, but check to be sure
    if (!FormatFeaturesAreSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02001");
        vk::CmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                         VK_FILTER_LINEAR);
        m_errorMonitor->VerifyFound();
    }

    if (cubic_support &&
        !FormatFeaturesAreSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG)) {
        // Invalid filter CUBIC_IMG
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02002");
        vk::CmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                         VK_FILTER_CUBIC_IMG);
        m_errorMonitor->VerifyFound();

        // Invalid filter CUBIC_IMG + invalid 2D source image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02002");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-00237");
        vk::CmdBlitImage(m_commandBuffer->handle(), src3D.image(), src3D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                         VK_FILTER_CUBIC_IMG);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeImage, BlitLayout) {
    TEST_DESCRIPTION("Incorrect vkCmdBlitImage layouts");
    RETURN_IF_SKIP(Init())

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Create images
    VkImageObj img_src_transfer(m_device);
    VkImageObj img_dst_transfer(m_device);
    VkImageObj img_general(m_device);
    VkImageObj img_color(m_device);

    img_src_transfer.InitNoLayout(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                  VK_IMAGE_TILING_OPTIMAL, 0);
    img_dst_transfer.InitNoLayout(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                  VK_IMAGE_TILING_OPTIMAL, 0);
    img_general.InitNoLayout(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                             VK_IMAGE_TILING_OPTIMAL, 0);
    img_color.InitNoLayout(64, 64, 1, fmt,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                           VK_IMAGE_TILING_OPTIMAL, 0);

    ASSERT_TRUE(img_src_transfer.initialized());
    ASSERT_TRUE(img_dst_transfer.initialized());
    ASSERT_TRUE(img_general.initialized());
    ASSERT_TRUE(img_color.initialized());

    img_src_transfer.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    img_dst_transfer.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    img_general.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    img_color.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {32, 32, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    vk::CmdBlitImage(m_commandBuffer->handle(), img_general.image(), img_general.Layout(), img_general.image(),
                     img_general.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    // Illegal srcImageLayout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImageLayout-01398");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     img_dst_transfer.image(), img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();

    // Illegal destImageLayout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImageLayout-01399");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_dst_transfer.image(),
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);

    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    // Source image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_color.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);

    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    // Destination image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_color.image(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.image(),
                     img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);

    // Source image in invalid layout in the middle of CB
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = img_general.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImageLayout-00221");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_general.image(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.image(),
                     img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);

    // Destination image in invalid layout in the middle of CB
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.image = img_dst_transfer.handle();

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImageLayout-00226");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_dst_transfer.image(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeImage, BlitOffsets) {
    RETURN_IF_SKIP(Init())

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    if (!FormatFeaturesAreSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "No blit feature format support";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.format = fmt;
    ci.extent = {64, 1, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image_1D(m_device);
    image_1D.init(&ci);
    ASSERT_TRUE(image_1D.initialized());

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {64, 64, 1};
    VkImageObj image_2D(m_device);
    image_2D.init(&ci);
    ASSERT_TRUE(image_2D.initialized());

    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {64, 64, 64};
    VkImageObj image_3D(m_device);
    image_3D.init(&ci);
    ASSERT_TRUE(image_3D.initialized());

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;

    m_commandBuffer->begin();

    // 1D, with src/dest y offsets other than (0,1)
    blit_region.srcOffsets[0] = {0, 1, 0};
    blit_region.srcOffsets[1] = {30, 1, 1};
    blit_region.dstOffsets[0] = {32, 0, 0};
    blit_region.dstOffsets[1] = {64, 1, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00245");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_1D.image(), image_1D.Layout(), image_1D.image(), image_1D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[0] = {32, 1, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00250");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_1D.image(), image_1D.Layout(), image_1D.image(), image_1D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // 2D, with src/dest z offsets other than (0,1)
    blit_region.srcOffsets[0] = {0, 0, 1};
    blit_region.srcOffsets[1] = {24, 31, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00247");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[0] = {32, 32, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00252");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Source offsets exceeding source image dimensions
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {65, 64, 1};  // src x
    blit_region.dstOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcOffset-00243");  // x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00215");   // src region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[1] = {64, 65, 1};                                                 // src y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcOffset-00244");  // y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00215");   // src region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 65};  // src z
    blit_region.srcOffsets[1] = {64, 64, 64};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcOffset-00246");  // z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00215");   // src region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Dest offsets exceeding source image dimensions
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {64, 64, 1};
    blit_region.dstOffsets[0] = {96, 64, 32};  // dst x
    blit_region.dstOffsets[1] = {64, 0, 33};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstOffset-00248");  // x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00216");   // dst region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.dstOffsets[0] = {0, 65, 32};                                                 // dst y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstOffset-00249");  // y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00216");   // dst region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.dstOffsets[0] = {0, 64, 65};  // dst z
    blit_region.dstOffsets[1] = {64, 0, 64};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstOffset-00251");  // z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00216");   // dst region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, BlitOverlap) {
    TEST_DESCRIPTION("Try to blit an image on same region.");

    RETURN_IF_SKIP(Init())

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    if (!FormatFeaturesAreSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "No blit feature format support";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = fmt;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = nullptr;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image_2D(m_device);
    image_2D.init(&ci);
    ASSERT_TRUE(image_2D.initialized());

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;

    m_commandBuffer->begin();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {31, 31, 1};
    blit_region.dstOffsets[0] = {15, 15, 0};
    blit_region.dstOffsets[1] = {47, 47, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00217");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, MiscBlitTests) {
    RETURN_IF_SKIP(Init())

    VkFormat f_color = VK_FORMAT_R32_SFLOAT;  // Need features ..BLIT_SRC_BIT & ..BLIT_DST_BIT

    if (!FormatFeaturesAreSupported(gpu(), f_color, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "No blit feature format support";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = f_color;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // 2D color image
    VkImageObj color_img(m_device);
    color_img.init(&ci);
    ASSERT_TRUE(color_img.initialized());

    // 2D multi-sample image
    ci.samples = VK_SAMPLE_COUNT_4_BIT;
    VkImageObj ms_img(m_device);
    ms_img.init(&ci);
    ASSERT_TRUE(ms_img.initialized());

    // 3D color image
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {64, 64, 8};
    VkImageObj color_3D_img(m_device);
    color_3D_img.init(&ci);
    ASSERT_TRUE(color_3D_img.initialized());

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {16, 16, 1};
    blitRegion.dstOffsets[0] = {32, 32, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // Blit with aspectMask errors
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-aspectMask-00241");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-aspectMask-00242");
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid src mip level
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.mipLevel = ci.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcSubresource-01705");  // invalid srcSubresource.mipLevel
    // Redundant unavoidable errors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcOffset-00243");  // out-of-bounds srcOffset.x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcOffset-00244");  // out-of-bounds srcOffset.y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcOffset-00246");  // out-of-bounds srcOffset.z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-pRegions-00215");  // region not contained within src image
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid dst mip level
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.mipLevel = ci.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstSubresource-01706");  // invalid dstSubresource.mipLevel
    // Redundant unavoidable errors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstOffset-00248");  // out-of-bounds dstOffset.x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstOffset-00249");  // out-of-bounds dstOffset.y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstOffset-00251");  // out-of-bounds dstOffset.z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-pRegions-00216");  // region not contained within dst image
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid src array layer
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcSubresource.baseArrayLayer = ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcSubresource-01707");  // invalid srcSubresource layer range
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid dst array layer
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.baseArrayLayer = ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstSubresource-01708");  // invalid dstSubresource layer range
                                                                                       // Redundant unavoidable errors
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blitRegion.dstSubresource.baseArrayLayer = 0;

    // Blit multi-sample image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00233");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00234");
    vk::CmdBlitImage(m_commandBuffer->handle(), ms_img.image(), ms_img.Layout(), ms_img.image(), ms_img.Layout(), 1, &blitRegion,
                     VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit 3D with baseArrayLayer != 0 or layerCount != 1
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00240");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcSubresource-01707");  // base+count > total layer count
    vk::CmdBlitImage(m_commandBuffer->handle(), color_3D_img.image(), color_3D_img.Layout(), color_3D_img.image(),
                     color_3D_img.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00240");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkImageSubresourceLayers-layerCount-01700");  // layer count == 0 (src)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkImageBlit-layerCount-08800");  // src/dst layer count mismatch
    vk::CmdBlitImage(m_commandBuffer->handle(), color_3D_img.image(), color_3D_img.Layout(), color_3D_img.image(),
                     color_3D_img.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, BlitToDepth) {
    RETURN_IF_SKIP(Init())

    const VkFormat f_depth = VK_FORMAT_D32_SFLOAT;
    if (!FormatFeaturesAreSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        GTEST_SKIP() << "Required depth VK_FORMAT_FEATURE_BLIT_SRC_BIT features not supported";
    } else if (FormatFeaturesAreSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "Required no depth VK_FORMAT_FEATURE_BLIT_DST_BIT features not supported";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = f_depth;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // 2D depth image
    VkImageObj depth_img(m_device);
    depth_img.init(&ci);
    ASSERT_TRUE(depth_img.initialized());

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {16, 16, 1};
    blitRegion.dstOffsets[0] = {32, 32, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // Blit depth image - has SRC_BIT but not DST_BIT
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), depth_img.image(), depth_img.Layout(), depth_img.image(), depth_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, MinImageTransferGranularity) {
    TEST_DESCRIPTION("Tests for validation of Queue Family property minImageTransferGranularity.");
    RETURN_IF_SKIP(Init())

    auto queue_family_properties = m_device->phy().queue_properties_;
    auto large_granularity_family =
        std::find_if(queue_family_properties.begin(), queue_family_properties.end(), [](VkQueueFamilyProperties family_properties) {
            VkExtent3D family_granularity = family_properties.minImageTransferGranularity;
            // We need a queue family that supports copy operations and has a large enough minImageTransferGranularity for the tests
            // below to make sense.
            return (family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT || family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT ||
                    family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                   family_granularity.depth >= 4 && family_granularity.width >= 4 && family_granularity.height >= 4;
        });

    if (large_granularity_family == queue_family_properties.end()) {
        GTEST_SKIP() << "No queue family has a large enough granularity for this test to be meaningful";
    }
    const size_t queue_family_index = std::distance(queue_family_properties.begin(), large_granularity_family);
    VkExtent3D granularity = queue_family_properties[queue_family_index].minImageTransferGranularity;
    vkt::CommandPool command_pool(*m_device, queue_family_index, 0);

    // Create two images of different types and try to copy between them
    VkImage srcImage;
    VkImage dstImage;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = granularity.width * 2;
    image_create_info.extent.height = granularity.height * 2;
    image_create_info.extent.depth = granularity.depth * 2;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;

    VkImageObj src_image_obj(m_device);
    src_image_obj.init(&image_create_info);
    ASSERT_TRUE(src_image_obj.initialized());
    srcImage = src_image_obj.handle();

    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageObj dst_image_obj(m_device);
    dst_image_obj.init(&image_create_info);
    ASSERT_TRUE(dst_image_obj.initialized());
    dstImage = dst_image_obj.handle();

    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    ASSERT_TRUE(command_buffer.initialized());
    command_buffer.begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset.x = 0;
    copyRegion.srcOffset.y = 0;
    copyRegion.srcOffset.z = 0;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset.x = 0;
    copyRegion.dstOffset.y = 0;
    copyRegion.dstOffset.z = 0;
    copyRegion.extent.width = granularity.width;
    copyRegion.extent.height = granularity.height;
    copyRegion.extent.depth = granularity.depth;

    // Introduce failure by setting srcOffset to a bad granularity value
    copyRegion.srcOffset.y = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    vk::CmdCopyImage(command_buffer.handle(), srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Introduce failure by setting extent to a granularity value that is bad
    // for both the source and destination image.
    copyRegion.srcOffset.y = 0;
    copyRegion.extent.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    vk::CmdCopyImage(command_buffer.handle(), srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Now do some buffer/image copies
    VkDeviceSize buffer_size = 8 * granularity.height * granularity.width * granularity.depth;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.height = granularity.height;
    region.imageExtent.width = granularity.width;
    region.imageExtent.depth = granularity.depth;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;

    // Introduce failure by setting imageExtent to a bad granularity value
    region.imageExtent.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(command_buffer.handle(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    region.imageExtent.width = granularity.width;

    // Introduce failure by setting imageOffset to a bad granularity value
    region.imageOffset.z = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(NegativeImage, Array2DImageType) {
    TEST_DESCRIPTION("Create an image with a flag specifying 2D_ARRAY_COMPATIBLE but not of imageType 3D.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Trigger check by setting imagecreateflags to 2d_array_compat and imageType to 2D
    VkImageCreateInfo ici = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                             nullptr,
                             VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR,
                             VK_IMAGE_TYPE_2D,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             {32, 32, 1},
                             1,
                             1,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_SHARING_MODE_EXCLUSIVE,
                             0,
                             nullptr,
                             VK_IMAGE_LAYOUT_UNDEFINED};
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-flags-00950");
}

TEST_F(NegativeImage, View2DImageType) {
    TEST_DESCRIPTION("Create an image with a flag specifying 2D_VIEW_COMPATIBLE but not of imageType 3D.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Trigger check by setting imagecreateflags to 2d_array_compat and imageType to 2D
    VkImageCreateInfo ici = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                             nullptr,
                             VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT,
                             VK_IMAGE_TYPE_2D,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             {32, 32, 1},
                             1,
                             1,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_SHARING_MODE_EXCLUSIVE,
                             0,
                             nullptr,
                             VK_IMAGE_LAYOUT_UNDEFINED};
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-flags-07755");
}

// INVALID_IMAGE_LAYOUT tests (one other case is hit by MapMemWithoutHostVisibleBit and not here)
TEST_F(NegativeImage, ImageLayout) {
    TEST_DESCRIPTION(
        "Hit all possible validation checks associated with the UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout error. "
        "Generally these involve having images in the wrong layout when they're copied or transitioned.");
    // 3 in ValidateCmdBufImageLayouts
    // *  -1 Attempt to submit cmd buf w/ deleted image
    // *  -2 Cmd buf submit of image w/ layout not matching first use w/ subresource
    // *  -3 Cmd buf submit of image w/ layout not matching first use w/o subresource

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    // Create src & dst images to use for copy operations
    VkImageObj src_image(m_device);
    VkImageObj dst_image(m_device);
    VkImageObj depth_image(m_device);

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;

    src_image.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    dst_image.init(&image_create_info);

    image_create_info.format = VK_FORMAT_D16_UNORM;
    image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_image.init(&image_create_info);

    m_commandBuffer->begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent.width = 1;
    copy_region.extent.height = 1;
    copy_region.extent.depth = 1;

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    // The first call hits the expected WARNING and skips the call down the chain, so call a second time to call down chain and
    // update layer state
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    // Now cause error due to src image layout changing
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImageLayout-00128");
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    // Final src error is due to bad layout type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImageLayout-01917");
    m_errorMonitor->SetUnexpectedError(
        "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    // Now verify same checks for dst
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    // Now cause error due to src image layout changing
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImageLayout-00133");
    m_errorMonitor->SetUnexpectedError(
        "is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL.");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_UNDEFINED, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImageLayout-01395");
    m_errorMonitor->SetUnexpectedError(
        "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_UNDEFINED, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent tests using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageCopy2KHR copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                              NULL,
                                              copy_region.srcSubresource,
                                              copy_region.srcOffset,
                                              copy_region.dstSubresource,
                                              copy_region.dstOffset,
                                              copy_region.extent};
        VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                NULL,
                                                src_image.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                dst_image.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                1,
                                                &copy_region2};
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // The first call hits the expected WARNING and skips the call down the chain, so call a second time to call down chain and
        // update layer state
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        // Now cause error due to src image layout changing
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImageLayout-00128");
        m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // Final src error is due to bad layout type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImageLayout-01917");
        m_errorMonitor->SetUnexpectedError(
            "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout "
            "VK_IMAGE_LAYOUT_GENERAL.");
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // Now verify same checks for dst
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // Now cause error due to src image layout changing
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImageLayout-00133");
        m_errorMonitor->SetUnexpectedError(
            "is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL.");
        copy_image_info2.dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImageLayout-01395");
        m_errorMonitor->SetUnexpectedError(
            "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout "
            "VK_IMAGE_LAYOUT_GENERAL.");
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Convert dst and depth images to TRANSFER_DST for subsequent tests
    VkImageMemoryBarrier transfer_dst_image_barrier[1] = {};
    transfer_dst_image_barrier[0] = vku::InitStructHelper();
    transfer_dst_image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transfer_dst_image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transfer_dst_image_barrier[0].srcAccessMask = 0;
    transfer_dst_image_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transfer_dst_image_barrier[0].image = dst_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    transfer_dst_image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, transfer_dst_image_barrier);
    transfer_dst_image_barrier[0].image = depth_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, transfer_dst_image_barrier);

    // Cause errors due to clearing with invalid image layouts
    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail due to explicitly prohibited layout for color clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-01394");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-00004");
    vk::CmdClearColorImage(m_commandBuffer->handle(), dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for color clear.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-00004");
    vk::CmdClearColorImage(m_commandBuffer->handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();

    VkClearDepthStencilValue depth_clear_value = {};
    clear_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Fail due to explicitly prohibited layout for depth clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-imageLayout-00012");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), depth_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &depth_clear_value, 1,
                                  &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for depth clear.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &depth_clear_value, 1,
                                  &clear_range);
    m_errorMonitor->VerifyFound();

    VkImageMemoryBarrier image_barrier[1] = {};
    // In synchronization2, if oldLayout == newLayout, we're not doing an ILT and these fields don't need to match
    // the image's layout.
    image_barrier[0] = vku::InitStructHelper();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].image = src_image.handle();
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, image_barrier);

    // Now cause error due to bad image layout transition in PipelineBarrier
    image_barrier[0] = vku::InitStructHelper();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_barrier[0].image = src_image.handle();
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-01197");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-01210");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, image_barrier);
    m_errorMonitor->VerifyFound();

    // Finally some layout errors at RenderPass create time
    // Just hacking in specific state to get to the errors we want so don't copy this unless you know what you're doing.
    VkAttachmentReference attach = {};
    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 0;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &attach;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkRenderPass rp;

    // For this error we need a valid renderpass so create default one
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach.attachment = 0;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = depth_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Can't do a CLEAR load on READ_ONLY initialLayout
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rpci.pAttachments = &attach_desc;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-format-03283");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, StorageImageLayout) {
    TEST_DESCRIPTION("Attempt to update a STORAGE_IMAGE descriptor w/o GENERAL layout.");

    RETURN_IF_SKIP(Init())

    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageTiling tiling;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), tex_format, &format_properties);
    if (format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        tiling = VK_IMAGE_TILING_LINEAR;
    } else if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    } else {
        GTEST_SKIP() << "Device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT; skipped.";
    }

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkImageObj image(m_device);
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_STORAGE_BIT, tiling, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView view = image.targetView(tex_format);

    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-04152");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, CopyImageMemory) {
    TEST_DESCRIPTION("Validate 4 invalid image memory VUIDs ");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    // Create a small image with a dedicated allocation
    VkImageObj image_no_mem(m_device);
    image_no_mem.init_no_mem(*m_device, image_info);
    VkImageObj image(m_device);
    image.init(&image_info);

    VkImageCopy copy_region;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent.width = 4;
    copy_region.extent.height = 4;
    copy_region.extent.depth = 1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImage-07966");
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_no_mem.handle(), VK_IMAGE_LAYOUT_UNDEFINED, image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImage-07966");
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    vk::CmdCopyImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, image_no_mem.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    if (copy_commands2) {
        const VkImageCopy2KHR copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                              NULL,
                                              copy_region.srcSubresource,
                                              copy_region.srcOffset,
                                              copy_region.dstSubresource,
                                              copy_region.dstOffset,
                                              copy_region.extent};
        VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                NULL,
                                                image_no_mem.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                image.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                1,
                                                &copy_region2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImage-07966");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        copy_image_info2.srcImage = image.handle();
        copy_image_info2.dstImage = image_no_mem.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImage-07966");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL..");
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImage, ImageViewBreaksParameterCompatibilityRequirements) {
    TEST_DESCRIPTION(
        "Attempts to create an Image View with a view type that does not match the image type it is being created from.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool maintenance1_support =
        IsExtensionsEnabled(VK_KHR_MAINTENANCE_1_EXTENSION_NAME) || DeviceValidationVersion() >= VK_API_VERSION_1_1;

    VkPhysicalDeviceMemoryProperties memProps;
    vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memProps);

    // Test mismatch detection for image of type VK_IMAGE_TYPE_1D
    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_1D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image1D(m_device);
    image1D.init(&imgInfo);
    ASSERT_TRUE(image1D.initialized());

    // Initialize VkImageViewCreateInfo with mismatched viewType
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image1D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subResourceRange-01021");

    // Test mismatch detection for image of type VK_IMAGE_TYPE_2D
    imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               nullptr,
               VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
               VK_IMAGE_TYPE_2D,
               VK_FORMAT_R8G8B8A8_UNORM,
               {1, 1, 1},
               1,
               6,
               VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_SHARING_MODE_EXCLUSIVE,
               0,
               nullptr,
               VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image2D(m_device);
    image2D.init(&imgInfo);
    ASSERT_TRUE(image2D.initialized());

    // Initialize VkImageViewCreateInfo with mismatched viewType
    ivci = vku::InitStructHelper();
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subResourceRange-01021");

    // Change VkImageViewCreateInfo to different mismatched viewType
    ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    ivci.subresourceRange.layerCount = 6;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01003");

    // Test mismatch detection for image of type VK_IMAGE_TYPE_3D
    imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               nullptr,
               VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
               VK_IMAGE_TYPE_3D,
               VK_FORMAT_R8G8B8A8_UNORM,
               {1, 1, 1},
               1,
               1,
               VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_SHARING_MODE_EXCLUSIVE,
               0,
               nullptr,
               VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image3D(m_device);
    image3D.init(&imgInfo);
    ASSERT_TRUE(image3D.initialized());

    // Initialize VkImageViewCreateInfo with mismatched viewType
    ivci = vku::InitStructHelper();
    ivci.image = image3D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_1D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subResourceRange-01021");

    // Change VkImageViewCreateInfo to different mismatched viewType
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // Test for error message
    if (maintenance1_support) {
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-06728");
    } else {
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subResourceRange-01021");
    }

    // Check if the device can make the image required for this test case.
    VkImageFormatProperties formProps = {{0, 0, 0}, 0, 0, 0, 0};
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(
        m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_3D, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
        &formProps);

    // If not, skip this part of the test.
    if (res || !m_device->phy().features().sparseBinding || !maintenance1_support) {
        GTEST_SKIP() << "Missing supported features";
    }

    // Initialize VkImageCreateInfo with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR and VK_IMAGE_CREATE_SPARSE_BINDING_BIT which
    // are incompatible create flags.
    imgInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
        VK_IMAGE_TYPE_3D,
        VK_FORMAT_R8G8B8A8_UNORM,
        {1, 1, 1},
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED};
    VkImage imageSparse;

    // Creating a sparse image means we should not bind memory to it.
    res = vk::CreateImage(m_device->device(), &imgInfo, NULL, &imageSparse);
    ASSERT_FALSE(res);

    // Initialize VkImageViewCreateInfo to create a view that will attempt to utilize VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR.
    ivci = vku::InitStructHelper();
    ivci.image = imageSparse;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-04971");

    // Clean up
    vk::DestroyImage(m_device->device(), imageSparse, nullptr);
}

TEST_F(NegativeImage, ImageViewFormatFeatureMismatch) {
    TEST_DESCRIPTION("Create view with a format that does not have the same features as the image format.");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    uint32_t feature_count = 5;
    // List of features to be tested
    VkFormatFeatureFlagBits features[] = {
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,            // 02274
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,         // 08932 - only need one of 2 features
        VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,            // 02275
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,         // 08931
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT  // 02277
    };
    // List of usage cases for each feature test
    VkImageUsageFlags usages[] = {
        VK_IMAGE_USAGE_SAMPLED_BIT,                  // 02274
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,         // 08932
        VK_IMAGE_USAGE_STORAGE_BIT,                  // 02275
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,         // 08931
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  // 02277
    };
    // List of errors that will be thrown in order of tests run
    // Order is done to make sure adjacent format features are different
    std::string optimal_error_codes[] = {
        "VUID-VkImageViewCreateInfo-usage-02274", "VUID-VkImageViewCreateInfo-usage-08932",
        "VUID-VkImageViewCreateInfo-usage-02275", "VUID-VkImageViewCreateInfo-usage-08931",
        "VUID-VkImageViewCreateInfo-usage-02277",  // Needs to be last since needs special format
    };

    VkFormatProperties formatProps;

    // All but one test in this loop and do last test after for special format case
    uint32_t i = 0;
    for (i = 0; i < (feature_count - 1); i++) {
        // Modify formats to have mismatched features

        // Format for image
        fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, &formatProps);
        formatProps.optimalTilingFeatures |= features[i];
        fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, formatProps);

        memset(&formatProps, 0, sizeof(formatProps));

        // Format for view
        fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, &formatProps);
        formatProps.optimalTilingFeatures = features[(i + 1) % feature_count];
        fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, formatProps);

        // Create image with modified format
        VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                     nullptr,
                                     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                     VK_IMAGE_TYPE_2D,
                                     VK_FORMAT_R32G32B32A32_UINT,
                                     {1, 1, 1},
                                     1,
                                     1,
                                     VK_SAMPLE_COUNT_1_BIT,
                                     VK_IMAGE_TILING_OPTIMAL,
                                     usages[i],
                                     VK_SHARING_MODE_EXCLUSIVE,
                                     0,
                                     nullptr,
                                     VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageObj image(m_device);
        image.init(&imgInfo);
        ASSERT_TRUE(image.initialized());

        // Initialize VkImageViewCreateInfo with modified format
        VkImageViewCreateInfo ivci = vku::InitStructHelper();
        ivci.image = image.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_R32G32B32A32_SINT;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // Test for error message
        CreateImageViewTest(*this, &ivci, optimal_error_codes[i]);
    }

    // Test for VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.  Needs special formats

    // Only run this test if format supported
    if (!FormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        GTEST_SKIP() << "VK_FORMAT_D24_UNORM_S8_UINT format not supported";
    }
    // Modify formats to have mismatched features

    // Format for image
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, &formatProps);
    formatProps.optimalTilingFeatures |= features[i];
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, formatProps);

    memset(&formatProps, 0, sizeof(formatProps));

    // Format for view
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, &formatProps);
    formatProps.optimalTilingFeatures = features[(i + 1) % feature_count];
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, formatProps);

    // Create image with modified format
    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_D24_UNORM_S8_UINT,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 usages[i],
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&imgInfo);
    ASSERT_TRUE(image.initialized());

    // Initialize VkImageViewCreateInfo with modified format
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

    // The 02277 VU is 'probably' redundant, but keeping incase a future spec change
    // This extra VU checked is because depth formats are only compatible with themselves
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-01761");
    // Test for error message
    CreateImageViewTest(*this, &ivci, optimal_error_codes[i]);
}

TEST_F(NegativeImage, ImageViewUsageCreateInfo) {
    TEST_DESCRIPTION("Usage modification via a chained VkImageViewUsageCreateInfo struct");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormatProperties formatProps;

    // Ensure image format claims support for sampled and storage, excludes color attachment
    memset(&formatProps, 0, sizeof(formatProps));
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, &formatProps);
    formatProps.optimalTilingFeatures |= (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    formatProps.optimalTilingFeatures = formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, formatProps);

    // Create image with sampled and storage usages
    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_R32G32B32A32_UINT,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&imgInfo);
    ASSERT_TRUE(image.initialized());

    // Force the imageview format to exclude storage feature, include color attachment
    memset(&formatProps, 0, sizeof(formatProps));
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    formatProps.optimalTilingFeatures = (formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, formatProps);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R32G32B32A32_SINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // ImageView creation should fail because view format doesn't support all the underlying image's usages
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-usage-02275");

    // Add a chained VkImageViewUsageCreateInfo to override original image usage bits, removing storage
    VkImageViewUsageCreateInfo usage_ci = vku::InitStructHelper();
    usage_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    // Link the VkImageViewUsageCreateInfo struct into the view's create info pNext chain
    ivci.pNext = &usage_ci;

    // ImageView should now succeed without error
    CreateImageViewTest(*this, &ivci);

    // Try a zero usage field
    usage_ci.usage = 0;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewUsageCreateInfo-usage-requiredbitmask");

    // Try an illegal bit in usage field
    usage_ci.usage = 0x10000000 | VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewUsageCreateInfo-usage-parameter");
}

TEST_F(NegativeImage, ImageViewNoSeparateStencilUsage) {
    TEST_DESCRIPTION("Verify CreateImageView create info for the case VK_EXT_separate_stencil_usage is not supported.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());

    // without VK_EXT_separate_stencil_usage explicitly enabled
    RETURN_IF_SKIP(InitState())

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageSubresourceRange range = {aspect, 0, 1, 0, 1};

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.flags = 0;
    image_view_create_info.image = VK_NULL_HANDLE;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = depth_format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange = range;

    VkImageViewUsageCreateInfo image_view_usage_create_info = vku::InitStructHelper();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_view_create_info.pNext = &image_view_usage_create_info;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());
    image_view_create_info.image = image.handle();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag

    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewCreateInfo-pNext-02662");
}

TEST_F(NegativeImage, ImageViewStencilUsageCreateInfo) {
    TEST_DESCRIPTION("Verify CreateImageView with stencil usage.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());

    RETURN_IF_SKIP(InitState())

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageSubresourceRange range = {aspect, 0, 1, 0, 1};

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.flags = 0;
    image_view_create_info.image = VK_NULL_HANDLE;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = depth_format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange = range;

    VkImageViewUsageCreateInfo image_view_usage_create_info = vku::InitStructHelper();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_view_create_info.pNext = &image_view_usage_create_info;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());
    image_view_create_info.image = image.handle();

    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag

    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewCreateInfo-pNext-02662");

    VkImageStencilUsageCreateInfoEXT image_stencil_create_info = vku::InitStructHelper();
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_create_info.pNext = &image_stencil_create_info;

    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag
    image_view_create_info.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;  // Flag other than VK_IMAGE_ASPECT_STENCIL_BIT

    VkImageObj image2(m_device);
    image2.init(&image_create_info);
    ASSERT_TRUE(image2.initialized());
    image_view_create_info.image = image2.handle();

    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in
    // VkImageStencilUsageCreateInfo::stencilUsage
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-02663");
    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-02664");
    vkt::ImageView view(*m_device, image_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, ImageViewNoMemoryBoundToImage) {
    VkResult err;

    RETURN_IF_SKIP(Init())

    // Create an image and try to create a view with no memory backing the image
    VkImage image;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    CreateImageViewTest(*this, &image_view_create_info,
                        " used with no memory bound. Memory should be bound by calling vkBindImageMemory().");
    vk::DestroyImage(m_device->device(), image, NULL);
}

TEST_F(NegativeImage, ImageViewAspect) {
    TEST_DESCRIPTION("Create an image and try to create a view with an invalid aspectMask");

    RETURN_IF_SKIP(Init())

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.layerCount = 1;
    // Cause an error by setting an invalid image aspect
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    CreateImageViewTest(*this, &image_view_create_info, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
}

TEST_F(NegativeImage, GetImageSubresourceLayout) {
    TEST_DESCRIPTION("Test vkGetImageSubresourceLayout() valid usages");

    RETURN_IF_SKIP(Init())
    VkSubresourceLayout subres_layout = {};

    // VU 00732: image must have been created with tiling equal to VK_IMAGE_TILING_LINEAR
    {
        const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;  // ERROR: violates VU 00732
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, tiling);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 0;
        subres.arrayLayer = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-image-07790");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // VU 00733: The aspectMask member of pSubresource must only have a single bit set
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_METADATA_BIT;  // ERROR: triggers VU 00733
        subres.mipLevel = 0;
        subres.arrayLayer = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-aspectMask-00997");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-08886");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // 00739 mipLevel must be less than the mipLevels specified in VkImageCreateInfo when the image was created
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 1;  // ERROR: triggers VU 00739
        subres.arrayLayer = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-mipLevel-01716");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // 00740 arrayLayer must be less than the arrayLayers specified in VkImageCreateInfo when the image was created
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 0;
        subres.arrayLayer = 1;  // ERROR: triggers VU 00740

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-arrayLayer-01717");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // 04462 If format has a depth component the aspectMask member of pResource must containt VK_IMAGE_ASPECT_DEPTH_BIT
    {
        VkFormat format = VK_FORMAT_D32_SFLOAT;
        VkFormatProperties image_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &image_format_properties);
        VkImageFormatProperties format_limits{};
        VkResult result =
            vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, &format_limits);
        if ((result == VK_SUCCESS) &&
            FormatFeaturesAreSupported(gpu(), format, VK_IMAGE_TILING_LINEAR, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
            VkImageObj img(m_device);
            img.InitNoLayout(32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

            VkImageSubresource subres = {};
            subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // ERROR: triggers VU 04462

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04462");
            vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // 04463 If format has a stencil component the aspectMask member of pResource must containt VK_IMAGE_ASPECT_STENCIL_BIT
    {
        VkFormat format = VK_FORMAT_S8_UINT;
        VkFormatProperties image_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &image_format_properties);
        VkImageFormatProperties format_limits{};
        VkResult result =
            vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, &format_limits);
        if ((result == VK_SUCCESS) &&
            FormatFeaturesAreSupported(gpu(), format, VK_IMAGE_TILING_LINEAR, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
            VkImageObj img(m_device);
            img.InitNoLayout(32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

            VkImageSubresource subres = {};
            subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // ERROR: triggers VU 04463

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04463");
            vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // 04464 If format does not contain stencil or depth component the aspectMask member of pResource must not contain
    // VK_IMAGE_ASPECT_DEPTH_BIT or VK_IMAGE_ASPECT_STENCIL_BIT
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;  // ERROR: triggers VU 00997 and 04464

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-08886");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04464");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;  // ERROR: triggers VU 00997 and 04464

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-08886");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04464");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImage, UndefinedFormat) {
    TEST_DESCRIPTION("Create image with undefined format");

    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_UNDEFINED;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-01975");
}

TEST_F(NegativeImage, ImageViewFormatMismatchUnrelated) {
    TEST_DESCRIPTION("Create an image with a color format, then try to create a depth view of it");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    VkFormatProperties formatProps;

    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), depth_format, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), depth_format, formatProps);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo imgViewInfo = vku::InitStructHelper();
    imgViewInfo.image = image.handle();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = depth_format;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Can't use depth format for view into color image - Expect INVALID_FORMAT
    CreateImageViewTest(*this, &imgViewInfo,
                        "Formats MUST be IDENTICAL unless VK_IMAGE_CREATE_MUTABLE_FORMAT BIT was set on image creation.");
}

TEST_F(NegativeImage, ImageViewNoMutableFormatBit) {
    TEST_DESCRIPTION("Create an image view with a different format, when the image does not have MUTABLE_FORMAT bit");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkFormatProperties formatProps;

    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_B8G8R8A8_UINT, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_B8G8R8A8_UINT, formatProps);

    VkImageViewCreateInfo imgViewInfo = vku::InitStructHelper();
    imgViewInfo.image = image.handle();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UINT;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Same compatibility class but no MUTABLE_FORMAT bit - Expect
    // VIEW_CREATE_ERROR
    CreateImageViewTest(*this, &imgViewInfo, "VUID-VkImageViewCreateInfo-image-01762");
}

TEST_F(NegativeImage, ImageViewDifferentClass) {
    TEST_DESCRIPTION("Passing bad parameters to CreateImageView");

    VkPhysicalDeviceFeatures device_features = {};
    RETURN_IF_SKIP(Init())
    GetPhysicalDeviceFeatures(&device_features);

    if (!(m_device->format_properties(VK_FORMAT_R8_UINT).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "Device does not support R8_UINT as color attachment";
    }

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   nullptr,
                                   0,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_R8_UINT,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    VkImageObj mutImage(m_device);
    mutImage.init(&imageInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = vku::InitStructHelper();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;  // different than createImage
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.image = mutImage.handle();
    CreateImageViewTest(*this, &imgViewInfo, "VUID-VkImageViewCreateInfo-image-01761");

    // Use CUBE_ARRAY without feature enabled
    if (device_features.imageCubeArray == false) {
        VkImageCreateInfo cubeImageInfo = imageInfo;
        cubeImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        cubeImageInfo.arrayLayers = 6;
        VkImageObj cubeImage(m_device);
        cubeImage.init(&cubeImageInfo);
        ASSERT_TRUE(cubeImage.initialized());

        VkImageViewCreateInfo cubeImgViewInfo = imgViewInfo;
        cubeImgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        cubeImgViewInfo.format = VK_FORMAT_R8_UINT;  // compatiable format
        cubeImgViewInfo.image = cubeImage.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-viewType-02961");
        CreateImageViewTest(*this, &cubeImgViewInfo, "VUID-VkImageViewCreateInfo-viewType-01004");
    }
}

TEST_F(NegativeImage, ImageViewInvalidSubresourceRange) {
    TEST_DESCRIPTION("Passing bad image subrange to CreateImageView");
    RETURN_IF_SKIP(Init())
    const bool maintenance1 = IsExtensionsEnabled(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo img_view_info_template = vku::InitStructHelper();
    img_view_info_template.image = image.handle();
    img_view_info_template.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    img_view_info_template.format = image.format();
    // subresourceRange to be filled later for the purposes of this test
    img_view_info_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_view_info_template.subresourceRange.baseMipLevel = 0;
    img_view_info_template.subresourceRange.levelCount = 0;
    img_view_info_template.subresourceRange.baseArrayLayer = 0;
    img_view_info_template.subresourceRange.layerCount = 0;

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
    }

    // Try levelCount = 0
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageSubresourceRange-levelCount-01720");
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-06724");
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-06724");
    }

    // Try layerCount = 0
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageSubresourceRange-layerCount-01721");
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
    }

    {
        VkImageObj cubeArrayImg(m_device);
        auto image_ci = vkt::Image::create_info();
        image_ci.arrayLayers = 18;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        cubeArrayImg.init(&image_ci);

        VkImageViewCreateInfo cube_img_view_info_template = vku::InitStructHelper();
        cube_img_view_info_template.image = cubeArrayImg.handle();
        cube_img_view_info_template.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        cube_img_view_info_template.format = cubeArrayImg.format();
        // subresourceRange to be filled later for the purposes of this test
        cube_img_view_info_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cube_img_view_info_template.subresourceRange.baseMipLevel = 0;
        cube_img_view_info_template.subresourceRange.levelCount = 0;
        cube_img_view_info_template.subresourceRange.baseArrayLayer = 0;
        cube_img_view_info_template.subresourceRange.layerCount = 0;

        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info);
        }
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 5};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02960");
        }
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 12, VK_REMAINING_ARRAY_LAYERS};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info);
        }
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 6, VK_REMAINING_ARRAY_LAYERS};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02962");
        }

        if (device_features.imageCubeArray == VK_TRUE) {
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 12};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 13};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02961");
            }
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 6, VK_REMAINING_ARRAY_LAYERS};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 11, VK_REMAINING_ARRAY_LAYERS};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02963");
            }
        }
    }

    {
        VkImageObj volumeImage(m_device);
        auto image_ci = vkt::Image::create_info();
        image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.extent = {8, 8, 8};
        image_ci.mipLevels = 4;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        volumeImage.init(&image_ci);

        VkImageViewCreateInfo volume_img_view_info_template = vku::InitStructHelper();
        volume_img_view_info_template.image = volumeImage.handle();
        volume_img_view_info_template.format = volumeImage.format();

        // 3D views
        {
            // first mip
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // all mips
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 4, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // too many mips
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 5, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
            }
            // invalid base mip
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 5, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
            }
            // too many layers
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-imageViewType-04973");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
            }
            // invalid base layer
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-06724");
            }
        }
        if (maintenance1) {
            // 2D views
            // first mip, first layer
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // all mips, first layer (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 4, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-04970");
            }
            // first mip, all layers (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-imageViewType-04973");
            }
            // mip 3, 8 layers (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-imageViewType-04973");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }
            // mip 3, layer 7 (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 7, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02724");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }
            // 2D array views
            // first mip, first layer
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // all mips, first layer (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 4, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-04970");
            }
            // first mip, all layers
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // mip 3, layer 0
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // mip 3, 8 layers (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }
            // mip 3, layer 7 (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 7, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02724");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }

            // Checking sparse flags are not set
            VkImageViewCreateInfo sparse_image_view_ci = volume_img_view_info_template;
            sparse_image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            // using VK_IMAGE_CREATE_SPARSE_BINDING_BIT
            if (device_features.sparseBinding) {
                VkImageObj sparse_image(m_device);
                image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
                sparse_image.Init(image_ci, 0, false);
                sparse_image_view_ci.image = sparse_image.handle();

                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
            }
            // using VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT
            if (device_features.sparseResidencyImage3D) {
                VkImageObj sparse_image(m_device);
                image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-flags-00987");
                sparse_image.Init(image_ci, 0, false);
                sparse_image_view_ci.image = sparse_image.handle();

                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
            }
            // using VK_IMAGE_CREATE_SPARSE_ALIASED_BIT
            if (device_features.sparseResidencyAliased) {
                VkImageObj sparse_image(m_device);
                image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT |
                                 VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
                sparse_image.Init(image_ci, 0, false);
                sparse_image_view_ci.image = sparse_image.handle();

                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
            }
        }
    }
}

TEST_F(NegativeImage, ImageViewLayerCount) {
    TEST_DESCRIPTION("Image and ImageView arrayLayers/layerCount parameters not being compatibile");

    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_ci = vku::InitStructHelper(nullptr);
    image_ci.flags = 0;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.extent = {128, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    VkImageObj image_1d(m_device);
    image_1d.init(&image_ci);
    ASSERT_TRUE(image_1d.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    VkImageObj image_2d(m_device);
    image_2d.init(&image_ci);
    ASSERT_TRUE(image_2d.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    VkImageObj image_3d(m_device);
    image_3d.init(&image_ci);
    ASSERT_TRUE(image_3d.initialized());

    image_ci.arrayLayers = 2;

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    VkImageObj image_1d_array(m_device);
    image_1d_array.init(&image_ci);
    ASSERT_TRUE(image_1d_array.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    VkImageObj image_2d_array(m_device);
    image_2d_array.init(&image_ci);
    ASSERT_TRUE(image_2d_array.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    VkImageFormatProperties img_limits;
    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &image_ci, &img_limits));
    std::optional<VkImageObj> image_3d_array;
    image_ci.arrayLayers = 1;  // arrayLayers must be 1 for 3D images
    if (img_limits.maxArrayLayers >= image_ci.arrayLayers) {
        image_3d_array.emplace(m_device);
        image_3d_array->init(&image_ci);
        ASSERT_TRUE(image_3d_array->initialized());
    }

    // base for each test that never changes
    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(nullptr);
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Sanity checks
    {
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = 1;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        if (image_3d_array) {
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            CreateImageViewTest(*this, &image_view_ci);
        }

        image_view_ci.subresourceRange.baseArrayLayer = 1;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        if (image_3d_array) {
            image_view_ci.subresourceRange.baseArrayLayer = 0;
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            CreateImageViewTest(*this, &image_view_ci);
        }

        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
        image_view_ci.image = image_3d.image();
        CreateImageViewTest(*this, &image_view_ci);
    }

    // layerCount is not 1 as imageView is not an array type
    {
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = 2;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04973");

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04973");

        if (image_3d_array) {
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
            CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04973");
        }
    }

    // layerCount is VK_REMAINING_ARRAY_LAYERS but not 1
    {
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04974");

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04974");
    }
}

TEST_F(NegativeImage, ImageMisc) {
    TEST_DESCRIPTION("Misc leftover valid usage errors in VkImageCreateInfo struct");

    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));

    const VkImageCreateInfo safe_image_ci = DefaultImageInfo();

    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &safe_image_ci));

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.extent = {4, 4, 4};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.arrayLayers = 6;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.mipLevels = 2;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.mipLevels = 1;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 2;
        image_ci.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-02259");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 1;
        image_ci.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-02259");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00963");

        image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00966");

        image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        image_ci.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-usage-00963");
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00966");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-00969");
    }

    // InitialLayout not VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREDEFINED
    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-initialLayout-00993");
    }

    // Storage usage can't be multisample if feature not set
    {
        // Feature should not have been set for these tests
        ASSERT_TRUE(features.shaderStorageImageMultisample == VK_FALSE);
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_STORAGE_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00968");
    }
}

TEST_F(NegativeImage, ImageMinLimits) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters violation minimum limit, such as being zero.");

    RETURN_IF_SKIP(Init())

    VkImage null_image;  // throwaway target for all the vk::CreateImage

    const VkImageCreateInfo safe_image_ci = [this]() {
        auto ci = DefaultImageInfo();
        ci.extent = {1, 1, 1};
        return ci;
    }();

    enum Dimension { kWidth = 0x1, kHeight = 0x2, kDepth = 0x4 };

    for (std::underlying_type<Dimension>::type bad_dimensions = 0x1; bad_dimensions < 0x8; ++bad_dimensions) {
        VkExtent3D extent = {1, 1, 1};

        if (bad_dimensions & kWidth) {
            extent.width = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-00944");
        }

        if (bad_dimensions & kHeight) {
            extent.height = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-00945");
        }

        if (bad_dimensions & kDepth) {
            extent.depth = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-00946");
        }

        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_3D;  // has to be 3D otherwise it might trigger the non-1 error instead
        bad_image_ci.extent = extent;

        vk::CreateImage(m_device->device(), &bad_image_ci, NULL, &null_image);

        m_errorMonitor->VerifyFound();
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.mipLevels = 0;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-mipLevels-00947");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.arrayLayers = 0;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-arrayLayers-00948");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        bad_image_ci.arrayLayers = 5;  // arrayLayers must be greater than or equal to 6
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-flags-08866");

        bad_image_ci.arrayLayers = 6;
        bad_image_ci.extent = {64, 63, 1};  // extent.width and extent.height must be equal
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-flags-08865");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_1D;
        bad_image_ci.extent = {64, 2, 1};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00956");

        bad_image_ci.imageType = VK_IMAGE_TYPE_1D;
        bad_image_ci.extent = {64, 1, 2};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00956");

        bad_image_ci.imageType = VK_IMAGE_TYPE_2D;
        bad_image_ci.extent = {64, 64, 2};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00957");

        bad_image_ci.imageType = VK_IMAGE_TYPE_2D;
        bad_image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        bad_image_ci.arrayLayers = 6;
        bad_image_ci.extent = {64, 64, 2};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00957");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_3D;
        bad_image_ci.arrayLayers = 2;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00961");
    }
}

TEST_F(NegativeImage, MaxLimitsMipLevelsAndExtent) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");
    RETURN_IF_SKIP(Init())
    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.extent = {8, 8, 1};
    image_ci.mipLevels = 4 + 1;  // 4 = log2(8) + 1
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-mipLevels-00958");

    image_ci.extent = {8, 15, 1};
    image_ci.mipLevels = 4 + 1;  // 4 = floor(log2(15)) + 1
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-mipLevels-00958");
}

TEST_F(NegativeImage, MaxLimitsMipLevels) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");
    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;
    image_ci.extent = {64, 64, 1};
    image_ci.mipLevels = 2;

    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected
    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        image_ci.format = format;

        VkImageFormatProperties img_limits;
        if (VK_SUCCESS == GPDIFPHelper(gpu(), &image_ci, &img_limits) && img_limits.maxMipLevels == 1) {
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-mipLevels-02255");
            return;  // end test
        }
    }

    GTEST_SKIP() << "Cannot find a format to test maxMipLevels limit; skipping part of test";
}

TEST_F(NegativeImage, MaxLimitsArrayLayers) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");
    RETURN_IF_SKIP(Init())
    VkImageCreateInfo image_ci = DefaultImageInfo();

    VkImageFormatProperties img_limits;
    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &image_ci, &img_limits));

    if (img_limits.maxArrayLayers == vvl::kU32Max) {
        GTEST_SKIP() << "VkImageFormatProperties::maxArrayLayers is already UINT32_MAX; skipping part of test";
    }
    image_ci.arrayLayers = img_limits.maxArrayLayers + 1;
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-arrayLayers-02256");
}

TEST_F(NegativeImage, MaxLimitsSamples) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");
    RETURN_IF_SKIP(Init())
    VkImageCreateInfo image_ci = DefaultImageInfo();

    const VkFormat first_vk_format = static_cast<VkFormat>(1);
    const VkFormat last_vk_format = static_cast<VkFormat>(130);  // avoid compressed/feature protected
    for (VkFormat format = first_vk_format; format <= last_vk_format; format = static_cast<VkFormat>(format + 1)) {
        image_ci.format = format;
        for (VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_64_BIT; samples > 0;
             samples = static_cast<VkSampleCountFlagBits>(samples >> 1)) {
            image_ci.samples = samples;
            VkImageFormatProperties img_limits;
            if (VK_SUCCESS == GPDIFPHelper(gpu(), &image_ci, &img_limits) && !(img_limits.sampleCounts & samples)) {
                CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02258");
                return;  // end test
            }
        }
    }

    GTEST_SKIP() << "Could not find a format with some unsupported samples; skipping part of test.";
}

TEST_F(NegativeImage, MaxLimitsExtent) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");
    RETURN_IF_SKIP(Init())
    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.imageType = VK_IMAGE_TYPE_3D;

    VkImageFormatProperties img_limits;
    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &image_ci, &img_limits));

    image_ci.extent = {img_limits.maxExtent.width + 1, 1, 1};
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-extent-02252");

    image_ci.extent = {1, img_limits.maxExtent.height + 1, 1};
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-extent-02253");

    image_ci.extent = {1, 1, img_limits.maxExtent.depth + 1};
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-extent-02254");
}

TEST_F(NegativeImage, MaxLimitsFramebuffer) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");
    RETURN_IF_SKIP(Init())

    const VkPhysicalDeviceLimits &dev_limits = m_device->phy().limits_;
    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // (any attachment bit)

    VkImageFormatProperties img_limits;
    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &image_ci, &img_limits));

    if (dev_limits.maxFramebufferWidth != vvl::kU32Max) {
        image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
        if (dev_limits.maxFramebufferWidth + 1 > img_limits.maxExtent.width) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02252");
        }
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00964");
    } else {
        printf("VkPhysicalDeviceLimits::maxFramebufferWidth is already UINT32_MAX; skipping part of test.\n");
    }

    if (dev_limits.maxFramebufferHeight != vvl::kU32Max) {
        image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
        if (dev_limits.maxFramebufferHeight + 1 > img_limits.maxExtent.height) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02253");
        }
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00965");
    } else {
        printf("VkPhysicalDeviceLimits::maxFramebufferHeight is already UINT32_MAX; skipping part of test.\n");
    }
}

TEST_F(NegativeImage, DepthStencilImageViewWithColorAspectBit) {
    // Create a single Image descriptor and cause it to first hit an error due
    //  to using a DS format, then cause it to hit error due to COLOR_BIT not
    //  set in aspect
    // The image format check comes 2nd in validation so we trigger it first,
    //  then when we cause aspect fail next, bad format check will be preempted

    RETURN_IF_SKIP(Init())
    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    VkImageObj image_bad(m_device);
    VkImageObj image_good(m_device);
    // One bad format and one good format for Color attachment
    const VkFormat tex_format_bad = depth_format;
    const VkFormat tex_format_good = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format_bad;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    image_bad.init(&image_create_info);

    image_create_info.format = tex_format_good;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_good.init(&image_create_info);

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image_bad.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format_bad;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    CreateImageViewTest(*this, &image_view_create_info, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
}

TEST_F(NegativeImage, CornerSampledImageNV) {
    TEST_DESCRIPTION("Test VK_NV_corner_sampled_image.");
    AddRequiredExtensions(VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceCornerSampledImageFeaturesNV corner_sampled_image_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(corner_sampled_image_features);
    if (corner_sampled_image_features.cornerSampledImage != VK_TRUE) {
        GTEST_SKIP() << "cornerSampledImage feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &corner_sampled_image_features));

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 2;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;

    // image type must be 2D or 3D
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02050");

    // cube/depth not supported
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 2;
    image_create_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02051");

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;

    // 2D width/height must be > 1
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02052");

    // 3D width/height/depth must be > 1
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.height = 2;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02053");

    image_create_info.imageType = VK_IMAGE_TYPE_2D;

    // Valid # of mip levels
    image_create_info.extent = {7, 7, 1};
    image_create_info.mipLevels = 3;  // 3 = ceil(log2(7))
    CreateImageTest(*this, &image_create_info);

    image_create_info.extent = {8, 8, 1};
    image_create_info.mipLevels = 3;  // 3 = ceil(log2(8))
    CreateImageTest(*this, &image_create_info);

    image_create_info.extent = {9, 9, 1};
    image_create_info.mipLevels = 3;  // 4 = ceil(log2(9))
    CreateImageTest(*this, &image_create_info);

    // Invalid # of mip levels
    image_create_info.extent = {8, 8, 1};
    image_create_info.mipLevels = 4;  // 3 = ceil(log2(8))
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-mipLevels-00958");
}

TEST_F(NegativeImage, Stencil) {
    TEST_DESCRIPTION("Verify ImageStencil create info.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    device_features.shaderStorageImageMultisample = VK_FALSE;  // Force multisampled storage images off

    RETURN_IF_SKIP(InitState(&device_features));

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageStencilUsageCreateInfoEXT image_stencil_create_info = vku::InitStructHelper();
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;

    image_create_info.pNext = &image_stencil_create_info;

    VkPhysicalDeviceImageFormatInfo2 image_format_info2 =
        vku::InitStructHelper(&image_stencil_create_info);
    image_format_info2.format = image_create_info.format;
    image_format_info2.type = image_create_info.imageType;
    image_format_info2.tiling = image_create_info.tiling;
    image_format_info2.usage = image_create_info.usage;
    image_format_info2.flags = image_create_info.flags;

    VkImageFormatProperties2 image_format_properties2 = vku::InitStructHelper();
    image_format_properties2.imageFormatProperties = {};

    // when including VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, must not include bits other than
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(m_device->phy().handle(), &image_format_info2, &image_format_properties2);
    m_errorMonitor->VerifyFound();
    // test vkCreateImage as well for this case
    CreateImageTest(*this, &image_create_info, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539");

    const VkPhysicalDeviceLimits &dev_limits = m_device->phy().limits_;

    if (dev_limits.maxFramebufferWidth != vvl::kU32Max) {
        // depth-stencil format image with VkImageStencilUsageCreateInfo with
        // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT set cannot have image width exceeding device maximum
        image_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        image_create_info.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
        image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-Format-02536");
    } else {
        printf("VkPhysicalDeviceLimits::maxFramebufferWidth is already UINT32_MAX; skipping part of test.\n");
    }

    if (dev_limits.maxFramebufferHeight != vvl::kU32Max) {
        // depth-stencil format image with VkImageStencilUsageCreateInfo with
        // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT set cannot have image height exceeding device maximum
        image_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        image_create_info.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
        image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02537");
    } else {
        printf("VkPhysicalDeviceLimits::maxFramebufferHeight is already UINT32_MAX; skipping part of test.\n");
    }

    // depth-stencil format image with VkImageStencilUsageCreateInfo with
    // VK_IMAGE_USAGE_STORAGE_BIT and the multisampled storage images feature
    // is not enabled, image samples must be VK_SAMPLE_COUNT_1_BIT
    image_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    image_create_info.extent = {64, 64, 1};
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02538");

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage includes
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02795");

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage does not include
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also not include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02796");

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage includes
    // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02797");

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage does not include
    // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also not include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02798");
}

TEST_F(NegativeImage, AstcDecodeMode) {
    TEST_DESCRIPTION("Tests for VUs for VK_EXT_astc_decode_mode");
    AddRequiredExtensions(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(astc_decode_features);
    if (!features2.features.textureCompressionASTC_LDR) {
        GTEST_SKIP() << "textureCompressionASTC_LDR feature not supported";
    }

    // Disable feature
    astc_decode_features.decodeModeSharedExponent = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    const VkFormat rgba_format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat ldr_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, rgba_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageObj astc_image(m_device);
    astc_image.Init(128, 128, 1, ldr_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(astc_image.initialized());

    VkImageViewASTCDecodeModeEXT astc_decode_mode = vku::InitStructHelper();
    astc_decode_mode.decodeMode = VK_FORMAT_R16G16B16A16_SFLOAT;

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper(&astc_decode_mode);
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = rgba_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // image view format is not ASTC

    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewASTCDecodeModeEXT-format-04084");

    // Non-valid decodeMode
    image_view_create_info.image = astc_image.handle();
    image_view_create_info.format = ldr_format;
    astc_decode_mode.decodeMode = ldr_format;
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230");

    // decodeModeSharedExponent not enabled
    astc_decode_mode.decodeMode = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02231");
}

TEST_F(NegativeImage, ImageViewIncompatibleFormat) {
    TEST_DESCRIPTION("Tests for VUID-VkImageViewCreateInfo-image-01761");
    // original issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2203

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   nullptr,
                                   0,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_R8_UINT,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    VkImageObj mutImage(m_device);
    mutImage.init(&imageInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = vku::InitStructHelper();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.image = mutImage.handle();

    // The Image's format is non-planar and incompatible with the ImageView's format, which should trigger
    // VUID-VkImageViewCreateInfo-image-01761
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
    CreateImageViewTest(*this, &imgViewInfo, "VUID-VkImageViewCreateInfo-image-01761");

    // With a identical format, there should be no error
    imgViewInfo.format = imageInfo.format;
    CreateImageViewTest(*this, &imgViewInfo, {});

    VkImageObj mut_compat_image(m_device);
    mut_compat_image.init(&imageInfo);
    ASSERT_TRUE(mut_compat_image.initialized());

    imgViewInfo.image = mut_compat_image.handle();
    imgViewInfo.format = VK_FORMAT_R8_SINT;  // different, but size compatible
    CreateImageViewTest(*this, &imgViewInfo, {});
}

TEST_F(NegativeImage, ImageViewIncompatibleDepthFormat) {
    TEST_DESCRIPTION("Tests for VUID-VkImageViewCreateInfo-image-01761 with depth format");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    const VkFormat depthOnlyFormat = FindSupportedDepthOnlyFormat(gpu());
    const VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   nullptr,
                                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                   VK_IMAGE_TYPE_2D,
                                   depthStencilFormat,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    VkImageObj mutImage(m_device);
    mutImage.init(&imageInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = vku::InitStructHelper();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imgViewInfo.image = mutImage.handle();
    // "Each depth/stencil format is only compatible with itself."
    imgViewInfo.format = depthOnlyFormat;
    CreateImageViewTest(*this, &imgViewInfo, "VUID-VkImageViewCreateInfo-image-01761");
}

TEST_F(NegativeImage, ImageViewMissingYcbcrConversion) {
    TEST_DESCRIPTION("Do not use VkSamplerYcbcrConversionInfo when required for an image view.");

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(features11);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.flags = 0;
    view_info.image = image.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    CreateImageViewTest(*this, &view_info, "VUID-VkImageViewCreateInfo-format-06415");
}

TEST_F(NegativeImage, ImageFormatList) {
    TEST_DESCRIPTION("Tests for VK_KHR_image_format_list");

    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    // Use sampled formats that will always be supported
    // Last format is not compatible with the rest
    const VkFormat formats[4] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8_UNORM};
    VkImageFormatListCreateInfo formatList = vku::InitStructHelper(nullptr);
    formatList.viewFormatCount = 4;
    formatList.pViewFormats = formats;

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   &formatList,
                                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_R8G8B8A8_UNORM,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    VkImageObj mutableImage(m_device);
    VkImageObj mutableImageZero(m_device);
    VkImageObj normalImage(m_device);

    // Not all 4 formats are compatible
    CreateImageTest(*this, &imageInfo, "VUID-VkImageCreateInfo-pNext-06722");

    // Should work with only first 3 in array
    formatList.viewFormatCount = 3;
    mutableImage.init(&imageInfo);
    ASSERT_TRUE(mutableImage.initialized());

    // Make sure no error if 0 format
    formatList.viewFormatCount = 0;
    formatList.pViewFormats = &formats[3];  // non-compatible format
    mutableImageZero.init(&imageInfo);
    ASSERT_TRUE(mutableImageZero.initialized());
    // reset
    formatList.viewFormatCount = 3;
    formatList.pViewFormats = formats;

    // Can't use 2 or higher formats if no mutable flag
    imageInfo.flags = 0;
    CreateImageTest(*this, &imageInfo, "VUID-VkImageCreateInfo-flags-04738");

    // Make sure no error if 1 format
    formatList.viewFormatCount = 1;
    normalImage.init(&imageInfo);
    ASSERT_TRUE(normalImage.initialized());

    VkImageViewCreateInfo imageViewInfo = vku::InitStructHelper(nullptr);
    imageViewInfo.flags = 0;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.image = mutableImage.handle();

    // Not in format list
    imageViewInfo.format = VK_FORMAT_R8_SNORM;
    m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01761");
    CreateImageViewTest(*this, &imageViewInfo, "VUID-VkImageViewCreateInfo-pNext-01585");

    imageViewInfo.format = VK_FORMAT_R8G8B8A8_SNORM;
    CreateImageViewTest(*this, &imageViewInfo, {});

    // If viewFormatCount is zero should not hit VUID 01585
    imageViewInfo.image = mutableImageZero.handle();
    CreateImageViewTest(*this, &imageViewInfo, {});
}

TEST_F(NegativeImage, ImageFormatListEnum) {
    TEST_DESCRIPTION("VkImageFormatListCreateInfo with bad enum");
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const VkFormat formats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VkFormat(0xBAD00000)};
    VkImageFormatListCreateInfo formatList = vku::InitStructHelper(nullptr);
    formatList.viewFormatCount = 2;
    formatList.pViewFormats = formats;
    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.pNext = &formatList;

    CreateImageTest(*this, &image_ci, "UNASSIGNED-GeneralParameterError-UnrecognizedValue");
}

TEST_F(NegativeImage, ImageFormatListSizeCompatible) {
    TEST_DESCRIPTION("Tests for VK_KHR_image_format_list with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT");

    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE2_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    const VkFormat formats[2] = {VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32_UINT};
    VkImageFormatListCreateInfo formatList = vku::InitStructHelper(nullptr);
    formatList.viewFormatCount = 1;
    formatList.pViewFormats = formats;

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   &formatList,
                                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    // The first image in the list should be size-compatible (128-bit)
    VkImageObj good_image(m_device);
    good_image.init(&imageInfo);

    // The second image in the list should NOT be size-compatible (64-bit)
    formatList.viewFormatCount = 2;
    CreateImageTest(*this, &imageInfo, "VUID-VkImageCreateInfo-pNext-06722");
}

TEST_F(NegativeImage, BlockTextImageViewCompatibleFormat) {
    TEST_DESCRIPTION("VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT without VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT");
    AddRequiredExtensions(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto image_ci = DefaultImageInfo();
    image_ci.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageFormatProperties image_properties;
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_ci.format, image_ci.imageType, image_ci.tiling,
                                                              image_ci.usage, image_ci.flags, &image_properties);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not valid for format, type, tiling, usage and flags combination.";
    }

    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-01572");
}

TEST_F(NegativeImage, BlockTextImageViewCompatibleFlag) {
    TEST_DESCRIPTION("VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT without VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT");
    AddRequiredExtensions(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    auto image_ci = DefaultImageInfo();
    image_ci.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;  // missing MUTABLE_FORMAT_BIT
    image_ci.format = VK_FORMAT_BC3_UNORM_BLOCK;

    VkImageFormatProperties image_properties;
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_ci.format, image_ci.imageType, image_ci.tiling,
                                                              image_ci.usage, image_ci.flags, &image_properties);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not valid for format, type, tiling, usage and flags combination.";
    }

    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-01573");
}

TEST_F(NegativeImage, SparseResidencyAliased) {
    TEST_DESCRIPTION("use VK_IMAGE_CREATE_SPARSE_ALIASED_BIT without sparseResidencyAliased.");
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.sparseResidencyAliased = VK_FALSE;
    RETURN_IF_SKIP(Init(&deviceFeatures));

    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.flags = VK_IMAGE_CREATE_SPARSE_ALIASED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-flags-00969");
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-01924");
}

TEST_F(NegativeImage, SparseResidencyLinear) {
    TEST_DESCRIPTION("use VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT with VK_IMAGE_TILING_LINEAR.");
    RETURN_IF_SKIP(Init())
    if (!m_device->phy().features().sparseResidencyImage2D) {
        GTEST_SKIP() << "Test requires unsupported SparseResidencyImage2D feature";
    }

    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;
    image_ci.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-tiling-04121");
}

TEST_F(NegativeImage, DisjointWithoutAlias) {
    TEST_DESCRIPTION("use VK_IMAGE_CREATE_DISJOINT_BIT without VK_IMAGE_CREATE_ALIAS_BIT.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())
    VkImageCreateInfo image_ci = DefaultImageInfo();
    image_ci.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    // some devices fail query on this image
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-format-01577");
}

TEST_F(NegativeImage, ImageSplitInstanceBindRegionCount) {
    TEST_DESCRIPTION("Bind image memory with VkBindImageMemoryDeviceGroupInfo but invalid flags");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    // Check that all extensions and their dependencies were enabled successfully

    RETURN_IF_SKIP(InitState())

    VkImageCreateInfo image_create_info = vku::InitStructHelper(nullptr);
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
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    vkt::DeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image.handle(), &mem_reqs);
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper(nullptr);
    mem_alloc.allocationSize = mem_reqs.size;

    for (int i = 0; i < 32; i++) {
        if (mem_reqs.memoryTypeBits & (1 << i)) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }

    image_mem.init(*m_device, mem_alloc);

    std::array<uint32_t, 2> deviceIndices = {{0, 0}};
    VkRect2D splitInstanceBindregion = {{0, 0}, {16, 16}};
    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = vku::InitStructHelper();
    bind_devicegroup_info.deviceIndexCount = 2;
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 1;
    bind_devicegroup_info.pSplitInstanceBindRegions = &splitInstanceBindregion;

    VkBindImageMemoryInfo bindInfo = vku::InitStructHelper();
    bindInfo.pNext = &bind_devicegroup_info;
    bindInfo.image = image.handle();
    bindInfo.memory = image_mem.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01627");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01633");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01634");
    vk::BindImageMemory2KHR(device(), 1, &bindInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, ImageSplitInstanceBindRegionCountWithDeviceGroup) {
    TEST_DESCRIPTION("Bind image memory with VkBindImageMemoryDeviceGroupInfo but invalid splitInstanceBindRegionCount");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = 0;
    create_device_pnext.pPhysicalDevices = nullptr;
    for (const auto &dg : physical_device_group) {
        if (dg.physicalDeviceCount > 1) {
            create_device_pnext.physicalDeviceCount = dg.physicalDeviceCount;
            create_device_pnext.pPhysicalDevices = dg.physicalDevices;
            break;
        }
    }
    if (create_device_pnext.pPhysicalDevices) {
        RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    } else {
        GTEST_SKIP() << "Test requires a physical device group with more than 1 device to use "
                        "VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper(nullptr);
    image_create_info.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkDeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image.handle(), &mem_reqs);
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper(nullptr);
    mem_alloc.allocationSize = mem_reqs.size;

    for (int i = 0; i < 32; i++) {
        if (mem_reqs.memoryTypeBits & (1 << i)) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }

    vk::AllocateMemory(device(), &mem_alloc, NULL, &image_mem);

    VkRect2D splitInstanceBindregion = {{0, 0}, {16, 16}};
    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = vku::InitStructHelper();
    bind_devicegroup_info.splitInstanceBindRegionCount = 2;
    bind_devicegroup_info.pSplitInstanceBindRegions = &splitInstanceBindregion;

    VkBindImageMemoryInfo bindInfo = vku::InitStructHelper();
    bindInfo.pNext = &bind_devicegroup_info;
    bindInfo.image = image.handle();
    bindInfo.memory = image_mem;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-splitInstanceBindRegionCount-01636");
    vk::BindImageMemory2KHR(device(), 1, &bindInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, BlockTexelViewLevelOrLayerCount) {
    TEST_DESCRIPTION(
        "Attempts to create an Image View with an image using VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, but levelCount and "
        "layerCount are not 1.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 4;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!image.IsCompatible(image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    image.Init(image_create_info, 0);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.levelCount = 4;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-07072");

    // Test for error message
    ivci.subresourceRange.layerCount = 2;
    ivci.subresourceRange.levelCount = 1;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-07072");
}

TEST_F(NegativeImage, BindIMageMemoryDeviceGroupInfo) {
    TEST_DESCRIPTION("Checks for invalid BindIMageMemoryDeviceGroupInfo.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0, skipping test";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = 0;
    create_device_pnext.pPhysicalDevices = nullptr;
    for (const auto &dg : physical_device_group) {
        if (dg.physicalDeviceCount > 1) {
            create_device_pnext.physicalDeviceCount = dg.physicalDeviceCount;
            create_device_pnext.pPhysicalDevices = dg.physicalDevices;
            break;
        }
    }
    if (create_device_pnext.pPhysicalDevices) {
        RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    } else {
        GTEST_SKIP() << "Test requires a physical device group with more than 1 device to use "
                        "VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT.";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = mem_reqs.memoryTypeBits;

    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    if (!pass) {
        GTEST_SKIP() << "Failed to set memory type.";
    }

    vkt::DeviceMemory memory;
    memory.init(*m_device, mem_alloc);

    uint32_t deviceIndex = 0;

    VkRect2D region = {};
    region.offset.x = 0;
    region.offset.y = 0;
    region.extent.width = image.width();
    region.extent.height = image.height();

    VkBindImageMemoryDeviceGroupInfo bimdgi = vku::InitStructHelper();
    bimdgi.deviceIndexCount = 1;
    bimdgi.pDeviceIndices = &deviceIndex;
    bimdgi.splitInstanceBindRegionCount = 1;
    bimdgi.pSplitInstanceBindRegions = &region;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bimdgi);
    bind_info.image = image.handle();
    bind_info.memory = memory.handle();
    bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01633");
    vk::BindImageMemory2KHR(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, BlockTexelViewType) {
    TEST_DESCRIPTION(
        "Create Image with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT and non-compressed format and ImageView with view type "
        "VK_IMAGE_VIEW_TYPE_3D.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!image.IsCompatible(image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    image.Init(image_create_info, 0);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.levelCount = 1;

    // Test for no error message, as VUID was removed
    CreateImageViewTest(*this, &ivci);
}

TEST_F(NegativeImage, BlockTexelViewFormat) {
    TEST_DESCRIPTION("Create Image with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT with non compatible formats.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;  // 64-bit block size
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!image.IsCompatible(image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    image.Init(image_create_info, 0);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.levelCount = 1;

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;  // 32-bit block size
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01583");

    ivci.format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;  // 64-bit block size, but not same format class
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01583");

    ivci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;  // 64-bit block size, and same format class
    CreateImageViewTest(*this, &ivci);
}

TEST_F(NegativeImage, ImageSubresourceRangeAspectMask) {
    TEST_DESCRIPTION("Test creating Image with invalid VkImageSubresourceRange aspectMask.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(features11);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

    if (!FormatFeaturesAreSupported(gpu(), mp_format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = mp_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());

    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = mp_format;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;

    vkt::SamplerYcbcrConversion conversion(*m_device, ycbcr_create_info);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversion.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&ycbcr_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = mp_format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT;

    CreateImageViewTest(*this, &ivci, "VUID-VkImageSubresourceRange-aspectMask-01670");
}

TEST_F(NegativeImage, CreateImageSharingModeConcurrentQueueFamilies) {
    TEST_DESCRIPTION("Checks for invalid queue families in ImageCreateInfo when sharingMode is VK_SHARING_MODE_CONCURRENT");

    AddOptionalExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent.width = 64;
    ci.extent.height = 64;
    ci.extent.depth = 1;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.sharingMode = VK_SHARING_MODE_CONCURRENT;

    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &ci));

    // Invalid pQueueFamilyIndices
    {
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices = nullptr;
        CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-sharingMode-00941");
    }

    // queueFamilyIndexCount must be greater than 1
    {
        ci.queueFamilyIndexCount = 1;
        const uint32_t queue_family = 0;
        ci.pQueueFamilyIndices = &queue_family;
        CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-sharingMode-00942");
    }

    // Each element of pQueueFamilyIndices must be unique
    {
        const std::array queue_families = {0U, 0U};
        ci.queueFamilyIndexCount = size32(queue_families);
        ci.pQueueFamilyIndices = queue_families.data();
        CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-sharingMode-01420");
    }

    // Each element of pQueueFamilyIndices must be less than pQueueFamilyPropertyCount returned by either
    // vkGetPhysicalDeviceQueueFamilyProperties or vkGetPhysicalDeviceQueueFamilyProperties2
    {
        uint32_t queue_node_count = 0;
        vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, nullptr);

        const std::array queue_families = {0U, queue_node_count};
        ci.queueFamilyIndexCount = size32(queue_families);
        ci.pQueueFamilyIndices = queue_families.data();

        CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-sharingMode-01420");
    }
}

TEST_F(NegativeImage, MultiSampleImageView) {
    TEST_DESCRIPTION("Begin conditional rendering when it is already active.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const VkPhysicalDeviceLimits &dev_limits = m_device->phy().limits_;
    if ((dev_limits.sampledImageColorSampleCounts & VK_SAMPLE_COUNT_2_BIT) == 0) {
        GTEST_SKIP() << "Required VkSampleCountFlagBits are not supported; skipping";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_create_info.format, image_create_info.imageType,
                                               image_create_info.tiling, image_create_info.usage, image_create_info.flags,
                                               &image_format_properties);

    if (image_format_properties.sampleCounts < 2) {
        GTEST_SKIP() << "Required VkSampleCountFlagBits for image format are not supported; skipping";
    }

    VkImageObj image(m_device);
    image.init(&image_create_info);

    VkImageViewCreateInfo dsvci = vku::InitStructHelper();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    dsvci.format = image_create_info.format;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subResourceRange-01021");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-04972");
    vkt::ImageView imageView(*m_device, dsvci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, Image2DViewOf3D) {
    TEST_DESCRIPTION("Checks for invalid use of 2D views of 3D images");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceImage2DViewOf3DFeaturesEXT image_2D_view_of_3D_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(image_2D_view_of_3D_features);
    if (!image_2D_view_of_3D_features.image2DViewOf3D) {
        GTEST_SKIP() << "Test requires unsupported image2DViewOf3D feature";
    }
    if (!image_2D_view_of_3D_features.sampler2DViewOf3D) {
        GTEST_SKIP() << "Test requires unsupported sampler2DViewOf3D feature";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent = {64, 64, 4};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

    VkImageObj image_3d(m_device);
    image_3d.init(&image_ci);
    ASSERT_TRUE(image_3d.initialized());
    VkImageViewCreateInfo view_ci = vku::InitStructHelper();
    view_ci.image = image_3d.handle();
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    view_ci.subresourceRange.layerCount = 1;
    view_ci.subresourceRange.baseMipLevel = 0;
    view_ci.subresourceRange.levelCount = 1;
    view_ci.subresourceRange.baseArrayLayer = 0;
    view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView view_2d_array(*m_device, view_ci);

    descriptor_set.WriteDescriptorImageInfo(0, view_2d_array.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-imageView-06712");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    descriptor_set.Clear();

    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkt::ImageView view_2d(*m_device, view_ci);
    descriptor_set.WriteDescriptorImageInfo(0, view_2d.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-imageView-07796");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    descriptor_set.Clear();

    image_ci.flags = 0;
    VkImageObj image_3d_no_flag(m_device);
    image_3d_no_flag.init(&image_ci);
    view_ci.image = image_3d_no_flag.handle();
    CreateImageViewTest(*this, &view_ci, "VUID-VkImageViewCreateInfo-image-06728");

    const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
    view_ci.subresourceRange = range;
    view_ci.image = image_3d_no_flag.handle();
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-06723");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-06724");
    CreateImageViewTest(*this, &view_ci, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
}

TEST_F(NegativeImage, Image2DViewOf3DFeature) {
    TEST_DESCRIPTION("Checks for image image_2d_view_of_3d features");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceImage2DViewOf3DFeaturesEXT image_2D_view_of_3D_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(image_2D_view_of_3D_features);
    image_2D_view_of_3D_features.image2DViewOf3D = VK_FALSE;
    image_2D_view_of_3D_features.sampler2DViewOf3D = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent = {64, 64, 4};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.flags = VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;

    VkImageObj image_3d(m_device);
    image_3d.init(&image_ci);
    ASSERT_TRUE(image_3d.initialized());
    VkImageViewCreateInfo view_ci = vku::InitStructHelper();
    view_ci.image = image_3d.handle();
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    view_ci.subresourceRange.layerCount = 1;
    view_ci.subresourceRange.baseMipLevel = 0;
    view_ci.subresourceRange.levelCount = 1;
    view_ci.subresourceRange.baseArrayLayer = 0;
    view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView view_2d_array(*m_device, view_ci);

    descriptor_set.WriteDescriptorImageInfo(0, view_2d_array.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-descriptorType-06714");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    descriptor_set.Clear();

    descriptor_set.WriteDescriptorImageInfo(1, view_2d_array.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-descriptorType-06713");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, ImageViewMinLod) {
    TEST_DESCRIPTION("Checks for image view minimum level of detail.");

    AddRequiredExtensions(VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceImageViewMinLodFeaturesEXT image_view_min_lod_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(image_view_min_lod_features);
    if (image_view_min_lod_features.minLod == VK_FALSE) {
        GTEST_SKIP() << "Test requires unsupported minLod feature";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 4;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image2D(m_device);
    image2D.init(&image_create_info);
    ASSERT_TRUE(image2D.initialized());

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 4;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewMinLodCreateInfoEXT ivml = vku::InitStructHelper();
    ivml.minLod = 4.0;
    ivci.pNext = &ivml;

    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewMinLodCreateInfoEXT-minLod-06456");
    ivml.minLod = 1.0;
    vkt::ImageView image_view(*m_device, ivci);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-06450");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, ImageViewMinLodFeature) {
    TEST_DESCRIPTION("Checks for image view minimum level of detail feature enabled.");
    RETURN_IF_SKIP(Init())
    VkImageObj image(m_device);
    // Initialize image with transfer source usage
    image.Init(128, 128, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 2;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewMinLodCreateInfoEXT ivml = vku::InitStructHelper();
    ivml.minLod = 1.0;
    ivci.pNext = &ivml;

    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewMinLodCreateInfoEXT-minLod-06455");
}

TEST_F(NegativeImage, ColorWthDepthAspect) {
    TEST_DESCRIPTION("Test creating an image with color format but depth aspect.");

    RETURN_IF_SKIP(Init())

    auto format = FindSupportedDepthStencilFormat(gpu());

    VkImageObj color_image(m_device);
    color_image.Init(64, 64, 1, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkImageViewCreateInfo civ_ci = vku::InitStructHelper();
    civ_ci.image = color_image.handle();
    civ_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    civ_ci.format = format;
    civ_ci.subresourceRange.layerCount = 1;
    civ_ci.subresourceRange.baseMipLevel = 0;
    civ_ci.subresourceRange.levelCount = 1;
    civ_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    vkt::ImageView color_image_view(*m_device, civ_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, ImageCopyMissingUsage) {
    TEST_DESCRIPTION("Test copying from src image without VK_IMAGE_USAGE_TRANSFER_SRC_BIT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    auto format = FindSupportedDepthStencilFormat(gpu());
    VkImageStencilUsageCreateInfo stencil_usage_ci = vku::InitStructHelper();
    stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageObj sampled_image(m_device);
    sampled_image.Init(image_ci);

    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj transfer_image(m_device);
    transfer_image.Init(image_ci);

    image_ci.pNext = &stencil_usage_ci;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj separate_stencil_sampled_image(m_device);
    separate_stencil_sampled_image.Init(image_ci);

    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj separate_stencil_transfer_image(m_device);
    separate_stencil_transfer_image.Init(image_ci);

    VkImageCopy region;
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffset.x = 0;
    region.srcOffset.y = 0;
    region.srcOffset.z = 0;
    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.dstOffset.x = 0;
    region.dstOffset.y = 0;
    region.dstOffset.z = 0;
    region.extent.width = 32;
    region.extent.height = 32;
    region.extent.depth = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspect-06662");
    vk::CmdCopyImage(m_commandBuffer->handle(), sampled_image.handle(), VK_IMAGE_LAYOUT_GENERAL, transfer_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspect-06663");
    vk::CmdCopyImage(m_commandBuffer->handle(), transfer_image.handle(), VK_IMAGE_LAYOUT_GENERAL, sampled_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspect-06664");
    vk::CmdCopyImage(m_commandBuffer->handle(), separate_stencil_sampled_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     separate_stencil_transfer_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-aspect-06665");
    vk::CmdCopyImage(m_commandBuffer->handle(), separate_stencil_transfer_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     separate_stencil_sampled_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, ImageCompressionControl) {
    TEST_DESCRIPTION("Checks image compression controls with invalid parameters.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    const bool multi_plane_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    VkPhysicalDeviceImageCompressionControlFeaturesEXT image_compression_control = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(image_compression_control);
    if (!image_compression_control.imageCompressionControl) {
        GTEST_SKIP() << "Test requires (unsupported) imageCompressionControl, skipping.";
    }

    // A bit set flag bit
    RETURN_IF_SKIP(InitState(nullptr, &image_compression_control));
    {
        VkImageCompressionControlEXT compression_control = vku::InitStructHelper();  // specify the desired compression settings
        compression_control.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT | VK_IMAGE_COMPRESSION_DISABLED_EXT;

        auto image_create_info = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_LINEAR);
        image_create_info.pNext = &compression_control;

        CreateImageTest(*this, &image_create_info, "VUID-VkImageCompressionControlEXT-flags-06747");
    }

    // Explicit Fixed Rate
    {
        VkImageCompressionControlEXT compression_control = vku::InitStructHelper();  // specify the desired compression settings
        compression_control.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT;
        compression_control.pFixedRateFlags = nullptr;

        auto image_create_info = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);
        image_create_info.pNext = &compression_control;

        CreateImageTest(*this, &image_create_info, "VUID-VkImageCompressionControlEXT-flags-06748");
    }

    // Image creation lambda
    const auto create_compressed_image = [&](VkFormat format, VkImageTiling imageTiling, VkImageObj &image) -> bool {
        VkImageCompressionControlEXT compression_control = vku::InitStructHelper();  // specify the desired compression settings
        compression_control.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;

        auto image_create_info =
            VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, imageTiling);
        image_create_info.pNext = &compression_control;

        bool supported = ImageFormatIsSupported(instance(), gpu(), image_create_info, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);

        if (supported) {
            image.init(&image_create_info);
        }

        return supported;
    };

    // Exceed MipmapLevel
    {
        VkImageObj image(m_device);
        if (create_compressed_image(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-mipLevel-01716");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // Exceed ArrayLayers
    {
        VkImageObj image(m_device);
        if (create_compressed_image(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-arrayLayer-01717");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // Color format aspect
    {
        VkImageObj image(m_device);
        if (create_compressed_image(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-format-08886");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // Depth format, Stencil aspect
    {
        const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());
        VkImageObj image(m_device);
        if (create_compressed_image(depth_format, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-format-04462");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }
    // Stencil format, Depth aspect
    const VkFormat stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format != VK_FORMAT_UNDEFINED) {
        VkImageObj image(m_device);
        if (create_compressed_image(stencil_format, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-format-04463");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // AspectMask should be a bitset
    {
        const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
        VkImageObj image(m_device);
        if (create_compressed_image(depth_stencil_format, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-aspectMask-00997");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // Depth/Stencil format aspect
    {
        VkImageObj image(m_device);
        if (create_compressed_image(VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-format-08886");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-format-04464");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // 2 plane format
    const VkFormat two_plane_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    if (multi_plane_extensions) {
        VkImageObj image(m_device);
        if (create_compressed_image(two_plane_format, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-tiling-08717");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_PLANE_2_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // 3 plane format
    const VkFormat three_plane_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    if (multi_plane_extensions) {
        VkImageObj image(m_device);
        if (create_compressed_image(three_plane_format, VK_IMAGE_TILING_LINEAR, image)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-tiling-08717");
            VkImageSubresource2EXT subresource = vku::InitStructHelper();
            subresource.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};

            VkImageCompressionPropertiesEXT compressionProperties = vku::InitStructHelper();
            VkSubresourceLayout2EXT layout = vku::InitStructHelper(&compressionProperties);

            vk::GetImageSubresourceLayout2EXT(m_device->handle(), image.handle(), &subresource, &layout);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeImage, GetImageSubresourceLayout2Maintenance5) {
    TEST_DESCRIPTION("Test vkGetImageSubresourceLayout2KHR with VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper(&maintenance5_features);
    GetPhysicalDeviceFeatures2(multi_draw_features);
    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));
    InitRenderTarget();

    VkImageObj image(m_device);
    auto image_create_info = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);
    image.init(&image_create_info);

    // Exceed MipmapLevel
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-mipLevel-01716");
    VkImageSubresource2KHR subresource = vku::InitStructHelper();
    subresource.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 0};
    VkSubresourceLayout2KHR layout = vku::InitStructHelper();
    vk::GetImageSubresourceLayout2KHR(m_device->handle(), image.handle(), &subresource, &layout);
    m_errorMonitor->VerifyFound();

    // Exceed ArrayLayers
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-arrayLayer-01717");
    subresource.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1};
    vk::GetImageSubresourceLayout2KHR(m_device->handle(), image.handle(), &subresource, &layout);
    m_errorMonitor->VerifyFound();

    // Color format aspect
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout2KHR-format-08886");
    subresource.imageSubresource = {VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0};
    vk::GetImageSubresourceLayout2KHR(m_device->handle(), image.handle(), &subresource, &layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, TransitionNonSparseImageLayoutWithoutBoundMemory) {
    TEST_DESCRIPTION("Try to change layout of non sparse image with no memory bound.");

    RETURN_IF_SKIP(Init())

    VkImageCreateInfo info = vkt::Image::create_info();
    info.format = VK_FORMAT_B8G8R8A8_UNORM;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj image{m_device};
    image.init_no_mem(*m_device, info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-image-01932");
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, AttachmentFeedbackLoopLayoutFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkImageCreateInfo info = vkt::Image::create_info();
    info.format = VK_FORMAT_B8G8R8A8_UNORM;
    info.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
    VkImageObj image{m_device};
    image.init(&info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-attachmentFeedbackLoopLayout-07313");
    image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    VkImageMemoryBarrier2KHR img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &img_barrier;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-attachmentFeedbackLoopLayout-07313");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);
    m_errorMonitor->VerifyFound();

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    rpci.pAttachments = &attach_desc;
    VkRenderPass rp;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07310");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07309");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentReference-attachmentFeedbackLoopLayout-07311");
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();

    VkAttachmentReference2 attach2 = vku::InitStructHelper();
    attach2.layout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    VkSubpassDescription2 subpass2 = vku::InitStructHelper();
    subpass2.colorAttachmentCount = 1;
    subpass2.pColorAttachments = &attach2;
    VkRenderPassCreateInfo2 rpci2 = vku::InitStructHelper();
    rpci2.subpassCount = 1;
    rpci2.pSubpasses = &subpass2;
    rpci2.attachmentCount = 1;
    VkAttachmentDescription2 attach_desc2 = vku::InitStructHelper();
    attach_desc2.format = VK_FORMAT_B8G8R8A8_UNORM;
    // Set loadOp to CLEAR
    attach_desc2.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc2.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc2.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    rpci2.pAttachments = &attach_desc2;
    VkRenderPass rp2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07310");
    vk::CreateRenderPass2(m_device->device(), &rpci2, NULL, &rp2);
    m_errorMonitor->VerifyFound();

    attach_desc2.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc2.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07309");
    vk::CreateRenderPass2(m_device->device(), &rpci2, NULL, &rp);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentReference2-attachmentFeedbackLoopLayout-07311");
    attach_desc2.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CreateRenderPass2(m_device->device(), &rpci2, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, SlicedDeviceFeature) {
    TEST_DESCRIPTION("Test SlicedCreateInfo feature support validation");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // NOTE: We are NOT enabling the VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::imageSlicedViewOf3D feature!
    InitState();

    VkImageObj image(m_device);
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image.init(&ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewSlicedCreateInfoEXT sliced_info = vku::InitStructHelper();
    sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
    sliced_info.sliceOffset = 0;

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&sliced_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = ci.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-None-07871");
    vkt::ImageView image_view(*m_device, ivci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, SlicedImageType) {
    TEST_DESCRIPTION("Test SlicedCreateInfo ImageType validation");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    {
        VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT slice_feature = vku::InitStructHelper();
        GetPhysicalDeviceFeatures2(slice_feature);
        if (slice_feature.imageSlicedViewOf3D == VK_FALSE) {
            GTEST_SKIP() << "Test requires (unsupported) imageSlicedViewOf3D";
        }
        InitState(nullptr, &slice_feature);
    }

    VkImageObj image(m_device);
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_2D;  // imageType should be VK_IMAGE_TYPE_3D
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 1};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image.init(&ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewSlicedCreateInfoEXT sliced_info = vku::InitStructHelper();
    sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
    sliced_info.sliceOffset = 0;

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&sliced_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;  // viewType should be VK_IMAGE_VIEW_TYPE_3D
    ivci.format = ci.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-image-07869");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-viewType-07909");
    vkt::ImageView image_view(*m_device, ivci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, SlicedMipLevel) {
    TEST_DESCRIPTION("When using VkImageViewSlicedCreateInfoEXT the image view must reference exactly 1 mip level");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    {
        VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT slice_feature = vku::InitStructHelper();
        GetPhysicalDeviceFeatures2(slice_feature);
        if (slice_feature.imageSlicedViewOf3D == VK_FALSE) {
            GTEST_SKIP() << "Test requires (unsupported) imageSlicedViewOf3D";
        }
        InitState(nullptr, &slice_feature);
    }

    VkImageObj image(m_device);
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image.init(&ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewSlicedCreateInfoEXT sliced_info = vku::InitStructHelper();
    sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
    sliced_info.sliceOffset = 0;

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&sliced_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = ci.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.layerCount = 1;

    auto get_effective_mip_levels = [&]() -> uint32_t { return ResolveRemainingLevels(ci, ivci.subresourceRange); };

    {
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 4;
        ASSERT_TRUE(get_effective_mip_levels() == 4);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-None-07870");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 2;
        ASSERT_TRUE(get_effective_mip_levels() == 2);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-None-07870");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        ivci.subresourceRange.baseMipLevel = 1;
        ivci.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        ASSERT_TRUE(get_effective_mip_levels() == 5);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-None-07870");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        ivci.subresourceRange.baseMipLevel = 3;
        ivci.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        ASSERT_TRUE(get_effective_mip_levels() == 3);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-None-07870");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImage, SlicedUsage) {
    TEST_DESCRIPTION("Test invalid sliceCount/sliceOffset of VkImageViewSlicedCreateInfoEXT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    {
        VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT slice_feature = vku::InitStructHelper();
        GetPhysicalDeviceFeatures2(slice_feature);
        if (slice_feature.imageSlicedViewOf3D == VK_FALSE) {
            GTEST_SKIP() << "Test requires (unsupported) imageSlicedViewOf3D";
        }
        InitState(nullptr, &slice_feature);
    }

    VkImageObj image(m_device);
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image.init(&ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewSlicedCreateInfoEXT sliced_info = vku::InitStructHelper();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&sliced_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = ci.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.levelCount = 1;

    auto get_effective_depth = [&]() -> uint32_t { return GetEffectiveExtent(ci, ivci.subresourceRange).depth; };

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 9;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-sliceOffset-07867");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        sliced_info.sliceCount = 0;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-sliceCount-07868");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        sliced_info.sliceCount = 8;
        sliced_info.sliceOffset = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-sliceCount-07868");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        sliced_info.sliceCount = 4;
        sliced_info.sliceOffset = 1;
        ivci.subresourceRange.baseMipLevel = 1;
        ASSERT_TRUE(get_effective_depth() == 4);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-sliceCount-07868");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }

    {
        sliced_info.sliceCount = 2;
        sliced_info.sliceOffset = 1;
        ivci.subresourceRange.baseMipLevel = 2;
        ASSERT_TRUE(get_effective_depth() == 2);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSlicedCreateInfoEXT-sliceCount-07868");
        vkt::ImageView image_view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImage, ImageViewTextureSampleWeighted) {
    TEST_DESCRIPTION("Checks for image view texture sample weighted.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceImageProcessingFeaturesQCOM image_proc_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(image_proc_features);

    if (image_proc_features.textureSampleWeighted == VK_FALSE) {
        GTEST_SKIP() << "TextureSampleWeighted feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &image_proc_features));

    VkPhysicalDeviceImageProcessingPropertiesQCOM image_proc_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(image_proc_properties);

    // check the format feature flags
    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8_UNORM, &fmt_props);
    if ((fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_WEIGHT_IMAGE_BIT_QCOM) == 0) {
        GTEST_SKIP() << "Required VK_FORMAT_FEATURE_2_WEIGHT_IMAGE_BIT_QCOM bit not supported for R8_UNORM";
    }
    fmt_props_3 = vku::InitStructHelper();
    vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);
    if ((fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_WEIGHT_SAMPLED_IMAGE_BIT_QCOM) == 0) {
        GTEST_SKIP() << "Required VK_FORMAT_FEATURE_2_WEIGHT_SAMPLED_IMAGE_BIT_QCOM bit not supported for VK_FORMAT_R8G8B8A8_UNORM";
    }

    VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();
    sci.maxLod = 0.0f;
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    sci.flags = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;

    // vkCreateSampler - expect success
    CreateSamplerTest(*this, &sci, "");
    auto sci_bad = sci;

    // vkCreateSampler - expect failure
    sci_bad.minFilter = VK_FILTER_LINEAR;  // disallowed
    CreateSamplerTest(*this, &sci_bad, "VUID-VkSamplerCreateInfo-flags-06964");
    sci_bad.minFilter = sci.minFilter;

    sci_bad.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;  // disallowed
    CreateSamplerTest(*this, &sci_bad, "VUID-VkSamplerCreateInfo-flags-06965");
    sci_bad.mipmapMode = sci.mipmapMode;

    sci_bad.maxLod = 1.0f;  // disallowed
    CreateSamplerTest(*this, &sci_bad, "VUID-VkSamplerCreateInfo-flags-06966");
    sci_bad.maxLod = sci.maxLod;

    sci_bad.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;  // disallowed
    CreateSamplerTest(*this, &sci_bad, "VUID-VkSamplerCreateInfo-flags-06967");
    sci_bad.addressModeU = sci.addressModeU;

    sci_bad.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;  // disallowed
    CreateSamplerTest(*this, &sci_bad, "VUID-VkSamplerCreateInfo-flags-06968");
    sci_bad.borderColor = sci.borderColor;

    sci_bad.compareEnable = VK_TRUE;  // disallowed
    CreateSamplerTest(*this, &sci_bad, "VUID-VkSamplerCreateInfo-flags-06970");
    sci_bad.compareEnable = sci.compareEnable;

    vkt::Sampler sampler(*m_device, sci);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 512;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 64;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkImageObj sampled_image2D(m_device);
    sampled_image2D.init(&image_create_info);
    ASSERT_TRUE(sampled_image2D.initialized());

    image_create_info.arrayLayers = 1;
    image_create_info.format = VK_FORMAT_R8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.arrayLayers = 64;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM;

    VkImageObj weight_image2D(m_device);
    weight_image2D.init(&image_create_info);
    ASSERT_TRUE(weight_image2D.initialized());

    const VkComponentMapping identity = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

    VkImageViewSampleWeightCreateInfoQCOM ivswci = vku::InitStructHelper();
    ivswci.filterCenter.x = 32;
    ivswci.filterCenter.y = 32;
    ivswci.filterSize.height = 64;
    ivswci.filterSize.width = 64;
    ivswci.numPhases = 64;  // 8 vert * 8 horiz

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.pNext = &ivswci;
    ivci.image = weight_image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R8_UNORM;
    ivci.components = identity;
    ivci.subresourceRange.layerCount = 64;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // vkCreateImageView - expect success
    auto ivswci_bad = ivswci;
    auto ivci_bad = ivci;
    ivci_bad.pNext = &ivswci_bad;
    ivswci_bad.filterSize.height = image_proc_properties.maxWeightFilterDimension.height + 1;

    // vkCreateImage - expect failure
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-06956 ");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-06957 ");
    CreateImageViewTest(*this, &ivci_bad, "VUID-VkImageViewSampleWeightCreateInfoQCOM-filterSize-06959");
    ivswci_bad.filterSize.height = ivswci.filterSize.height;

    ivswci_bad.filterSize.width = image_proc_properties.maxWeightFilterDimension.width + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-06955");
    CreateImageViewTest(*this, &ivci_bad, "VUID-VkImageViewSampleWeightCreateInfoQCOM-filterSize-06958");
    ivswci_bad.filterSize.width = ivswci.filterSize.width;

    ivswci_bad.filterCenter.x = ivswci.filterSize.width;
    ivswci_bad.filterCenter.y = ivswci.filterSize.height;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSampleWeightCreateInfoQCOM-filterCenter-06960");
    CreateImageViewTest(*this, &ivci_bad, "VUID-VkImageViewSampleWeightCreateInfoQCOM-filterCenter-06961");
    ivswci_bad.filterCenter.x = ivswci.filterCenter.x;
    ivswci_bad.filterCenter.y = ivswci.filterCenter.y;

    ivswci_bad.numPhases = image_proc_properties.maxWeightFilterPhases + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewSampleWeightCreateInfoQCOM-numPhases-06962");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-06954");
    CreateImageViewTest(*this, &ivci_bad, "VUID-VkImageViewSampleWeightCreateInfoQCOM-numPhases-06963");
    ivswci_bad.filterSize.width = ivswci.filterSize.width;

    vkt::ImageView weight_image_view(*m_device, ivci);

    ivci.pNext = nullptr;
    ivci.image = sampled_image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.components = identity;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView sampled_image_view(*m_device, ivci);

    // vkUpdateDescriptorSets - expect success
    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                      {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                      {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                      {3, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    descriptor_set.WriteDescriptorImageInfo(0, weight_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM);
    descriptor_set.WriteDescriptorImageInfo(1, sampled_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(2, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    // vkUpdateDescriptorSets - expect failure
    OneOffDescriptorSet descriptor_set_bad(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                      {3, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    descriptor_set.WriteDescriptorImageInfo(0, sampled_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM);
    descriptor_set.WriteDescriptorImageInfo(3, weight_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-06942");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-06943");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, CubeCompatibleMustBeImageType2D) {
    TEST_DESCRIPTION("If flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, imageType must be VK_IMAGE_TYPE_2D");
    RETURN_IF_SKIP(Init())
    if (!IsPlatformMockICD()) {
        // The following create info is malformed will cause validation issues on various GPUs.
        // EX: MoltenVK, GalaxyS10, and Pixel6.
        GTEST_SKIP() << "Only run on MockICD.";
    }

    auto ci = DefaultImageInfo();
    ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.extent.width = 1;
    ci.extent.height = 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-flags-08866");
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-flags-00949");

    ci.imageType = VK_IMAGE_TYPE_3D;
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-flags-08866");
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-flags-00949");
}

TEST_F(NegativeImage, ComputeImageLayout) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(fmt);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    {  // Verify invalid image layout with CmdDispatch
        vkt::CommandBuffer cmd(m_device, m_commandPool);
        cmd.begin();
        vk::CmdBindDescriptorSets(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);
        vk::CmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
        vk::CmdDispatch(cmd.handle(), 1, 1, 1);
        cmd.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_DrawState_InvalidImageLayout);
        cmd.QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
    }

    {  // Verify invalid image layout with CmdDispatchBaseKHR
        vkt::CommandBuffer cmd(m_device, m_commandPool);
        cmd.begin();
        vk::CmdBindDescriptorSets(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);
        vk::CmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
        vk::CmdDispatchBaseKHR(cmd.handle(), 0, 0, 0, 1, 1, 1);
        cmd.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_DrawState_InvalidImageLayout);
        cmd.QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImage, ComputeImageLayout11) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader using vkCmdDispatchBase");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(fmt);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdDispatchBase(m_commandBuffer->handle(), 0, 0, 0, 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_DrawState_InvalidImageLayout);
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, GetPhysicalDeviceImageFormatProperties) {
    TEST_DESCRIPTION("fail a call to GetPhysicalDeviceImageFormatProperties");
    RETURN_IF_SKIP(Init())

    // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 is a hardcoded format that is known to fail in MockICD
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Test only supported by MockICD";
    }
    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageObj image(m_device);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    image.Init(128, 128, 1, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImage, OverlappingImageCopy) {
    TEST_DESCRIPTION("Copy a range of an image to another overlapping range of the same image");

    RETURN_IF_SKIP(Init())

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image.init(&image_create_info);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_commandBuffer->begin();

    VkImageCopy image_copy{};
    image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy.srcSubresource.layerCount = 1;
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = image_copy.srcSubresource;
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-pRegions-00124");
    vk::CmdCopyImage(m_commandBuffer->handle(), image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeImage, BlitColorToDepth) {
    TEST_DESCRIPTION("Blit a color image to a depth image");
    RETURN_IF_SKIP(Init())

    VkImageObj color_image(m_device);
    color_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj depth_image(m_device);
    depth_image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), depth_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_2_BLIT_DST_BIT) == 0) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_2_BLIT_DST_BIT not supported for depth format";
    }

    VkImageBlit region;
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcSubresource.mipLevel = 0;
    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.dstSubresource.mipLevel = 0;
    region.srcOffsets[0] = {0, 0, 0};
    region.srcOffsets[1] = {32, 32, 1};
    region.dstOffsets[0] = {0, 0, 0};
    region.dstOffsets[1] = {32, 32, 1};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00231");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageBlit-aspectMask-00238");
    vk::CmdBlitImage(m_commandBuffer->handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1u, &region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeImage, ResolveDepthImage) {
    TEST_DESCRIPTION("Blit a color image to a depth image");
    RETURN_IF_SKIP(Init())

    auto depth_format = FindSupportedDepthStencilFormat(gpu());

    VkImageCreateInfo image_ci = vkt::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = depth_format;
    image_ci.extent.width = 32u;
    image_ci.extent.height = 32u;
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_ci.format, image_ci.imageType, image_ci.tiling, image_ci.usage,
                                               image_ci.flags, &image_format_properties);
    if ((image_format_properties.sampleCounts & image_ci.samples) == 0) {
        GTEST_SKIP() << "Required formats samples not supported";
    }

    VkImageObj image1(m_device);
    image1.init(&image_ci);

    VkImageObj image2(m_device);
    image2.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageResolve region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {32, 32, 1};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageResolve-aspectMask-00266");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResolveImage-dstImage-02003");
    vk::CmdResolveImage(m_commandBuffer->handle(), image1.handle(), VK_IMAGE_LAYOUT_GENERAL, image2.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}
