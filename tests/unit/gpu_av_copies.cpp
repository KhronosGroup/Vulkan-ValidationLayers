/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 * Copyright (c) 2023-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class NegativeGpuAVCopies : public GpuAVTest {};

TEST_F(NegativeGpuAVCopies, BufferToImageD32) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT.");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, sizeof(float) * 64 * 64,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, kHostVisibleMemProps);

    float *ptr = static_cast<float *>(copy_src_buffer.Memory().Map());
    for (size_t i = 0; i < 64 * 64; ++i) {
        ptr[i] = 0.1f;
    }
    ptr[4094] = 42.0f;

    vkt::Image copy_dst_image(*m_device, 64, 64, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkBufferImageCopy buffer_image_copy_1;
    buffer_image_copy_1.bufferOffset = 0;
    buffer_image_copy_1.bufferRowLength = 0;
    buffer_image_copy_1.bufferImageHeight = 0;
    buffer_image_copy_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    buffer_image_copy_1.imageOffset = {0, 0, 0};
    buffer_image_copy_1.imageExtent = {64, 64, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy_1);

    VkBufferImageCopy buffer_image_copy_2 = buffer_image_copy_1;
    buffer_image_copy_2.imageOffset = {32, 32, 0};
    buffer_image_copy_2.imageExtent = {32, 32, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy_2);

    m_command_buffer.End();
    // VUID-vkCmdCopyBufferToImage-pRegions-07931
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopies, BufferToImageD32Vk13) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, sizeof(float) * 64 * 64,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, kHostVisibleMemProps);

    float *ptr = static_cast<float *>(copy_src_buffer.Memory().Map());
    for (size_t i = 0; i < 64 * 64; ++i) {
        ptr[i] = 0.1f;
    }
    ptr[4094] = 42.0f;

    vkt::Image copy_dst_image(*m_device, 64, 64, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkBufferImageCopy2 region_1 = vku::InitStructHelper();
    region_1.bufferOffset = 0;
    region_1.bufferRowLength = 0;
    region_1.bufferImageHeight = 0;
    region_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    region_1.imageOffset = {0, 0, 0};
    region_1.imageExtent = {64, 64, 1};

    VkCopyBufferToImageInfo2 buffer_image_copy = vku::InitStructHelper();
    buffer_image_copy.srcBuffer = copy_src_buffer;
    buffer_image_copy.dstImage = copy_dst_image;
    buffer_image_copy.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_copy.regionCount = 1;
    buffer_image_copy.pRegions = &region_1;

    vk::CmdCopyBufferToImage2(m_command_buffer, &buffer_image_copy);

    region_1.imageOffset = {32, 32, 0};
    region_1.imageExtent = {32, 32, 1};

    vk::CmdCopyBufferToImage2(m_command_buffer, &buffer_image_copy);

    m_command_buffer.End();
    // VUID-VkCopyBufferToImageInfo2-pRegions-07931
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopies, BufferToImageD32U8) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT_S8_UINT.");
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, 5 * 64 * 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                kHostVisibleMemProps);

    auto ptr = static_cast<uint8_t *>(copy_src_buffer.Memory().Map());
    std::memset(ptr, 0, static_cast<size_t>(copy_src_buffer.CreateInfo().size));
    for (size_t i = 0; i < 64 * 64; ++i) {
        auto ptr_float = reinterpret_cast<float *>(ptr + 5 * i);
        if (i == 64 * 64 - 1) {
            *ptr_float = 42.0f;
        } else {
            *ptr_float = 0.1f;
        }
    }

    vkt::Image copy_dst_image(*m_device, 64, 64, VK_FORMAT_D32_SFLOAT_S8_UINT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    buffer_image_copy.imageOffset = {33, 33, 0};
    buffer_image_copy.imageExtent = {31, 31, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy);

    m_command_buffer.End();
    // VUID-vkCmdCopyBufferToImage-pRegions-07931
    m_errorMonitor->SetDesiredError("has a float value at offset 20475 that is not in the range [0, 1]");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopies, BufferToImageD32U8Vk13) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT_S8_UINT.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, 5 * 64 * 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                kHostVisibleMemProps);

    auto ptr = static_cast<uint8_t *>(copy_src_buffer.Memory().Map());
    std::memset(ptr, 0, static_cast<size_t>(copy_src_buffer.CreateInfo().size));
    for (size_t i = 0; i < 64 * 64; ++i) {
        auto ptr_float = reinterpret_cast<float *>(ptr + 5 * i);
        if (i == 64 * 64 - 1) {
            *ptr_float = 42.0f;
        } else {
            *ptr_float = 0.1f;
        }
    }

    vkt::Image copy_dst_image(*m_device, 64, 64, VK_FORMAT_D32_SFLOAT_S8_UINT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkBufferImageCopy2 region_1 = vku::InitStructHelper();
    region_1.bufferOffset = 0;
    region_1.bufferRowLength = 0;
    region_1.bufferImageHeight = 0;
    region_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    region_1.imageOffset = {33, 33, 0};
    region_1.imageExtent = {31, 31, 1};

    VkCopyBufferToImageInfo2 buffer_image_copy = vku::InitStructHelper();
    buffer_image_copy.srcBuffer = copy_src_buffer;
    buffer_image_copy.dstImage = copy_dst_image;
    buffer_image_copy.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_copy.regionCount = 1;
    buffer_image_copy.pRegions = &region_1;

    vk::CmdCopyBufferToImage2(m_command_buffer, &buffer_image_copy);

    m_command_buffer.End();
    // VUID-VkCopyBufferToImageInfo2-pRegions-07931
    m_errorMonitor->SetDesiredError("has a float value at offset 20475 that is not in the range [0, 1]");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}