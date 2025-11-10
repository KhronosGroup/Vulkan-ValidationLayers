/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "sync_helper.h"

class NegativeGpuAVCopyMemoryIndirect : public GpuAVCopyMemoryIndirect {};

TEST_F(NegativeGpuAVCopyMemoryIndirect, SrcAddressAlignment) {
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 16, 0, vkt::device_address);

    vkt::Buffer indirect_buffer(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address() + 1;
    indirect_buffer_ptr->dstAddress = dst_payload.Address();
    indirect_buffer_ptr->size = 8;

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 1;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryIndirectCommandKHR-srcAddress-10958");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopyMemoryIndirect, DstAddressAlignment) {
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 32, 0, vkt::device_address);

    vkt::Buffer indirect_buffer(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr[0].srcAddress = src_payload.Address();
    indirect_buffer_ptr[0].dstAddress = dst_payload.Address();
    indirect_buffer_ptr[0].size = 8;
    indirect_buffer_ptr[1].srcAddress = src_payload.Address();
    indirect_buffer_ptr[1].dstAddress = dst_payload.Address() + 15;
    indirect_buffer_ptr[1].size = 8;

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 2;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryIndirectCommandKHR-dstAddress-10959");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopyMemoryIndirect, SizeAlignment) {
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 16, 0, vkt::device_address);

    vkt::Buffer indirect_buffer(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address();
    indirect_buffer_ptr->dstAddress = dst_payload.Address();
    indirect_buffer_ptr->size = 5;

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 1;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryIndirectCommandKHR-size-10960");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopyMemoryIndirect, ImageSrcAddress) {
    AddRequiredFeature(vkt::Feature::indirectMemoryToImageCopy);
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 512, 0, vkt::device_address);
    vkt::Image dst_image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::Buffer indirect_buffer(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryToImageIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address() + 1;
    indirect_buffer_ptr->bufferRowLength = 8;
    indirect_buffer_ptr->bufferImageHeight = 8;
    indirect_buffer_ptr->imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    indirect_buffer_ptr->imageOffset = {0, 0, 0};
    indirect_buffer_ptr->imageExtent = {8, 8, 1};

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryToImageIndirectCommandKHR);

    VkCopyMemoryToImageIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 1;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstImage = dst_image;
    copy_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkImageSubresourceLayers res_layer = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_info.pImageSubresources = &res_layer;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryToImageIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageIndirectCommandKHR-srcAddress-10963");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopyMemoryIndirect, ImageExtent) {
    AddRequiredFeature(vkt::Feature::indirectMemoryToImageCopy);
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 512, 0, vkt::device_address);
    vkt::Image dst_image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::Buffer indirect_buffer(*m_device, 128, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryToImageIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr[0].srcAddress = src_payload.Address();
    indirect_buffer_ptr[0].bufferRowLength = 1;  // invalid
    indirect_buffer_ptr[0].bufferImageHeight = 8;
    indirect_buffer_ptr[0].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    indirect_buffer_ptr[0].imageOffset = {0, 0, 0};
    indirect_buffer_ptr[0].imageExtent = {8, 8, 1};
    indirect_buffer_ptr[1].srcAddress = src_payload.Address();
    indirect_buffer_ptr[1].bufferRowLength = 8;
    indirect_buffer_ptr[1].bufferImageHeight = 1;  // invalid
    indirect_buffer_ptr[1].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    indirect_buffer_ptr[1].imageOffset = {0, 0, 0};
    indirect_buffer_ptr[1].imageExtent = {8, 8, 1};

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryToImageIndirectCommandKHR);

    VkCopyMemoryToImageIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 1;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstImage = dst_image;
    copy_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkImageSubresourceLayers res_layer = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_info.pImageSubresources = &res_layer;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryToImageIndirectKHR(m_command_buffer, &copy_info);

    copy_info.copyAddressRange.address = indirect_buffer.Address() + sizeof(VkCopyMemoryToImageIndirectCommandKHR);
    vk::CmdCopyMemoryToImageIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageIndirectCommandKHR-bufferRowLength-10964");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageIndirectCommandKHR-bufferImageHeight-10965");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVCopyMemoryIndirect, Combined) {
    AddRequiredFeature(vkt::Feature::indirectMemoryToImageCopy);
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 512, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 16, 0, vkt::device_address);
    vkt::Image dst_image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::Buffer indirect_buffer1(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer indirect_buffer2(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer1_ptr = (VkCopyMemoryToImageIndirectCommandKHR *)indirect_buffer1.Memory().Map();
    indirect_buffer1_ptr->srcAddress = src_payload.Address() + 1;
    indirect_buffer1_ptr->bufferRowLength = 8;
    indirect_buffer1_ptr->bufferImageHeight = 8;
    indirect_buffer1_ptr->imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    indirect_buffer1_ptr->imageOffset = {0, 0, 0};
    indirect_buffer1_ptr->imageExtent = {8, 8, 1};

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer1.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryToImageIndirectCommandKHR);

    VkCopyMemoryToImageIndirectInfoKHR copy_image_info = vku::InitStructHelper();
    copy_image_info.copyCount = 1;
    copy_image_info.copyAddressRange = address_range;
    copy_image_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_image_info.dstImage = dst_image;
    copy_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkImageSubresourceLayers res_layer = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_image_info.pImageSubresources = &res_layer;

    auto *indirect_buffer2_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer2.Memory().Map();
    indirect_buffer2_ptr->srcAddress = src_payload.Address() + 1;
    indirect_buffer2_ptr->dstAddress = dst_payload.Address();
    indirect_buffer2_ptr->size = 4;

    address_range.address = indirect_buffer2.Address();
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_buffer_info = vku::InitStructHelper();
    copy_buffer_info.copyCount = 1;
    copy_buffer_info.copyAddressRange = address_range;
    copy_buffer_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_buffer_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_buffer_info);
    vk::CmdCopyMemoryToImageIndirectKHR(m_command_buffer, &copy_image_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryIndirectCommandKHR-srcAddress-10958");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageIndirectCommandKHR-srcAddress-10963");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeGpuAVCopyMemoryIndirect, GpuUpdate) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect());

    vkt::Buffer src_payload(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 16, 0, vkt::device_address);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_2_TRANSFER_DST_BIT, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address();
    indirect_buffer_ptr->dstAddress = dst_payload.Address();
    indirect_buffer_ptr->size = 4;  // valid, but will update to be invalid

    vkt::Buffer update_buffer(*m_device, 64, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT, vkt::device_address);
    auto *update_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)update_buffer.Memory().Map();
    update_buffer_ptr->srcAddress = src_payload.Address();
    update_buffer_ptr->dstAddress = dst_payload.Address();
    update_buffer_ptr->size = 5;

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 64;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 1;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    VkBufferCopy buffer_copy = {0, 0, sizeof(VkCopyMemoryIndirectCommandKHR)};
    vk::CmdCopyBuffer(m_command_buffer, update_buffer, indirect_buffer, 1, &buffer_copy);

    VkMemoryBarrier2 memory_barrier = vku::InitStructHelper();
    memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    memory_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_INDIRECT_BIT_KHR;
    memory_barrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    VkDependencyInfo dep_info = DependencyInfo(memory_barrier);
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dep_info);

    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryIndirectCommandKHR-size-10960");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}