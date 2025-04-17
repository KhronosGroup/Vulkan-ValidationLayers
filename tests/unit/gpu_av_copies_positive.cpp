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

class PositiveGpuAVCopies : public GpuAVTest {};

TEST_F(PositiveGpuAVCopies, CopyBufferToImageD32) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with all depth values in the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT.");
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, sizeof(float) * 64 * 64,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, kHostVisibleMemProps);

    float *ptr = static_cast<float *>(copy_src_buffer.Memory().Map());
    for (size_t i = 0; i < 64 * 64; ++i) {
        if (i % 2) {
            ptr[i] = 1.0f;
        } else {
            ptr[i] = 0.0f;
        }
    }

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
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAVCopies, CopyBufferToImageD32U8) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with all depth values in the [0, 1] legal range. Depth image has format "
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
        if (i % 2) {
            *ptr_float = 1.0f;
        } else {
            *ptr_float = 0.0f;
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
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAVCopies, CopyBufferToImageTwoSubmit) {
    TEST_DESCRIPTION("Make sure resources are managed correctly afer a CopyBufferToImage call.");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::CommandBuffer cb_0(*m_device, m_command_pool);
    vkt::CommandBuffer cb_1(*m_device, m_command_pool);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::Buffer buffer(*m_device, 4096, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32, 32, 1};
    region.bufferOffset = 0;

    cb_0.Begin();
    vk::CmdCopyBufferToImage(cb_0.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    cb_0.End();
    m_default_queue->SubmitAndWait(cb_0);

    cb_1.Begin();
    cb_1.End();
    m_default_queue->SubmitAndWait(cb_1);
}
