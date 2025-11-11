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
#include "generated/vk_function_pointers.h"
#include "sync_helper.h"

class PositiveGpuAVCopyMemoryIndirect : public GpuAVCopyMemoryIndirect {};

void GpuAVCopyMemoryIndirect::InitGpuAVCopyMemoryIndirect(bool safe_mode) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework({}, safe_mode));
    RETURN_IF_SKIP(InitState());
}

TEST_F(PositiveGpuAVCopyMemoryIndirect, Basic) {
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect(false));

    vkt::Buffer src_payload(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 16, 0, vkt::device_address);

    auto *src_payload_ptr = (uint32_t *)src_payload.Memory().Map();
    src_payload_ptr[0] = 3;
    src_payload_ptr[1] = 6;
    src_payload_ptr[2] = 9;
    src_payload_ptr[3] = 12;

    vkt::Buffer indirect_buffer(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address();
    indirect_buffer_ptr->dstAddress = dst_payload.Address();
    indirect_buffer_ptr->size = 16;

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

    m_default_queue->SubmitAndWait(m_command_buffer);

    auto *dst_payload_ptr = (uint32_t *)dst_payload.Memory().Map();
    ASSERT_TRUE(dst_payload_ptr[0] == 3);
    ASSERT_TRUE(dst_payload_ptr[1] == 6);
    ASSERT_TRUE(dst_payload_ptr[2] == 9);
    ASSERT_TRUE(dst_payload_ptr[3] == 12);
}

TEST_F(PositiveGpuAVCopyMemoryIndirect, MultiCopy) {
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect(false));

    vkt::Buffer src1_payload(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer src2_payload(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 32, 0, vkt::device_address);

    auto *src1_payload_ptr = (uint32_t *)src1_payload.Memory().Map();
    src1_payload_ptr[0] = 3;
    src1_payload_ptr[1] = 5;
    src1_payload_ptr[2] = 7;
    src1_payload_ptr[3] = 9;

    auto *src2_payload_ptr = (uint32_t *)src2_payload.Memory().Map();
    src2_payload_ptr[0] = 2;
    src2_payload_ptr[1] = 4;
    src2_payload_ptr[2] = 6;
    src2_payload_ptr[3] = 8;

    vkt::Buffer indirect_buffer(*m_device, 512, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr[0].srcAddress = src1_payload.Address();
    indirect_buffer_ptr[0].dstAddress = dst_payload.Address();
    indirect_buffer_ptr[0].size = 4;
    indirect_buffer_ptr[1].srcAddress = src1_payload.Address() + 8;
    indirect_buffer_ptr[1].dstAddress = dst_payload.Address() + 4;
    indirect_buffer_ptr[1].size = 4;
    indirect_buffer_ptr[2].srcAddress = src2_payload.Address();
    indirect_buffer_ptr[2].dstAddress = dst_payload.Address() + 8;
    indirect_buffer_ptr[2].size = 4;
    indirect_buffer_ptr[3].srcAddress = src2_payload.Address() + 4;
    indirect_buffer_ptr[3].dstAddress = dst_payload.Address() + 12;
    indirect_buffer_ptr[3].size = 8;

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = 512;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 4;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    auto *dst_payload_ptr = (uint32_t *)dst_payload.Memory().Map();
    ASSERT_TRUE(dst_payload_ptr[0] == 3);
    ASSERT_TRUE(dst_payload_ptr[1] == 7);
    ASSERT_TRUE(dst_payload_ptr[2] == 2);
    ASSERT_TRUE(dst_payload_ptr[3] == 4);
    ASSERT_TRUE(dst_payload_ptr[4] == 6);
}

TEST_F(PositiveGpuAVCopyMemoryIndirect, Image) {
    AddRequiredFeature(vkt::Feature::indirectMemoryToImageCopy);
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect(false));

    vkt::Buffer src_payload(*m_device, 512, 0, vkt::device_address);
    vkt::Image dst_image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::Buffer indirect_buffer(*m_device, 64, 0, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryToImageIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address();
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

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVCopyMemoryIndirect, GpuUpdate) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitGpuAVCopyMemoryIndirect(false));

    vkt::Buffer src_payload(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer dst_payload(*m_device, 16, 0, vkt::device_address);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_2_TRANSFER_DST_BIT, vkt::device_address);
    auto *indirect_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)indirect_buffer.Memory().Map();
    indirect_buffer_ptr->srcAddress = src_payload.Address();
    indirect_buffer_ptr->dstAddress = dst_payload.Address();
    indirect_buffer_ptr->size = 5;  // invalid, but will update

    vkt::Buffer update_buffer(*m_device, 64, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT, vkt::device_address);
    auto *update_buffer_ptr = (VkCopyMemoryIndirectCommandKHR *)update_buffer.Memory().Map();
    update_buffer_ptr->srcAddress = src_payload.Address();
    update_buffer_ptr->dstAddress = dst_payload.Address();
    update_buffer_ptr->size = 4;

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

    VkBufferMemoryBarrier2 memory_barrier = vku::InitStructHelper();
    memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    memory_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_INDIRECT_BIT_KHR;
    memory_barrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memory_barrier.buffer = indirect_buffer;
    memory_barrier.offset = 0;
    memory_barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo dep_info = DependencyInfo(memory_barrier);
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dep_info);

    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}
