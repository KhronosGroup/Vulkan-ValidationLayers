/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2026 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "utils/math_utils.h"
#include "../framework/layer_validation_tests.h"

class NegativeDeviceAddress : public VkLayerTest {};

TEST_F(NegativeDeviceAddress, DestroyOnlyBuffer) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    const size_t copy_size = sizeof(VkCopyMemoryIndirectCommandKHR);
    vkt::Buffer indirect_buffer(*m_device, copy_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR copy_address_range = {};
    copy_address_range.address = indirect_buffer.Address();
    copy_address_range.stride = copy_size;
    copy_address_range.size = copy_size;

    VkCopyMemoryIndirectInfoKHR indirect_info = vku::InitStructHelper();
    indirect_info.copyCount = 1u;
    indirect_info.copyAddressRange = copy_address_range;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &indirect_info);
    m_command_buffer.End();

    indirect_buffer.Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, DestroyAllBuffers) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    const size_t copy_size = sizeof(VkCopyMemoryIndirectCommandKHR);
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = copy_size;
    buffer_ci.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer indirect_buffer1(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer indirect_buffer2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, indirect_buffer1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    memory_ai.allocationSize = memory_requirements.size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, indirect_buffer1, memory, 0);
    vk::BindBufferMemory(*m_device, indirect_buffer2, memory, 0);

    VkDeviceAddress address1 = indirect_buffer1.Address();
    VkDeviceAddress address2 = indirect_buffer2.Address();

    if (address1 != address2) {
        GTEST_SKIP() << "Device addresses are not equal.";
    }

    VkStridedDeviceAddressRangeKHR copy_address_range = {};
    copy_address_range.address = indirect_buffer1.Address();
    copy_address_range.stride = copy_size;
    copy_address_range.size = copy_size;

    VkCopyMemoryIndirectInfoKHR indirect_info = vku::InitStructHelper();
    indirect_info.copyCount = 1u;
    indirect_info.copyAddressRange = copy_address_range;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &indirect_info);
    m_command_buffer.End();

    indirect_buffer1.Destroy();
    indirect_buffer2.Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, GetSecondDeviceAddressAfterInvalidation) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    const size_t copy_size = sizeof(VkCopyMemoryIndirectCommandKHR);
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = copy_size;
    buffer_ci.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer indirect_buffer1(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer indirect_buffer2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, indirect_buffer1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    memory_ai.allocationSize = memory_requirements.size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, indirect_buffer1, memory, 0);
    vk::BindBufferMemory(*m_device, indirect_buffer2, memory, 0);

    VkDeviceAddress address1 = indirect_buffer1.Address();

    VkStridedDeviceAddressRangeKHR copy_address_range = {};
    copy_address_range.address = indirect_buffer1.Address();
    copy_address_range.stride = copy_size;
    copy_address_range.size = copy_size;

    VkCopyMemoryIndirectInfoKHR indirect_info = vku::InitStructHelper();
    indirect_info.copyCount = 1u;
    indirect_info.copyAddressRange = copy_address_range;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &indirect_info);
    m_command_buffer.End();

    indirect_buffer1.Destroy();
    VkDeviceAddress address2 = indirect_buffer2.Address();

    if (address1 != address2) {
        GTEST_SKIP() << "Device addresses are not equal.";
    }

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, SecondBufferUnrelatedUsage) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    const size_t copy_size = sizeof(VkCopyMemoryIndirectCommandKHR);
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = copy_size;
    buffer_ci.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer indirect_buffer1(*m_device, buffer_ci, vkt::no_mem);
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer indirect_buffer2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, indirect_buffer1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    memory_ai.allocationSize = memory_requirements.size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, indirect_buffer1, memory, 0);
    vk::BindBufferMemory(*m_device, indirect_buffer2, memory, 0);

    VkDeviceAddress address1 = indirect_buffer1.Address();
    VkDeviceAddress address2 = indirect_buffer2.Address();

    if (address1 != address2) {
        GTEST_SKIP() << "Device addresses are not equal.";
    }

    VkStridedDeviceAddressRangeKHR copy_address_range = {};
    copy_address_range.address = indirect_buffer1.Address();
    copy_address_range.stride = copy_size;
    copy_address_range.size = copy_size;

    VkCopyMemoryIndirectInfoKHR indirect_info = vku::InitStructHelper();
    indirect_info.copyCount = 1u;
    indirect_info.copyAddressRange = copy_address_range;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &indirect_info);
    m_command_buffer.End();

    indirect_buffer1.Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, DestroyBeforeCmdBufferEnd) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    const size_t copy_size = sizeof(VkCopyMemoryIndirectCommandKHR);
    vkt::Buffer indirect_buffer(*m_device, copy_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR copy_address_range = {};
    copy_address_range.address = indirect_buffer.Address();
    copy_address_range.stride = copy_size;
    copy_address_range.size = copy_size;

    VkCopyMemoryIndirectInfoKHR indirect_info = vku::InitStructHelper();
    indirect_info.copyCount = 1u;
    indirect_info.copyAddressRange = copy_address_range;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &indirect_info);
    indirect_buffer.Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-commandBuffer-00059");
    vk::EndCommandBuffer(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, MemoryToImageIndirect) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryToImageCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    VkCopyMemoryToImageIndirectCommandKHR cmd1 = {};
    cmd1.srcAddress = 0;
    cmd1.bufferRowLength = 8;
    cmd1.bufferImageHeight = 8;
    cmd1.imageSubresource = {};
    cmd1.imageOffset = {0, 0, 0};
    cmd1.imageExtent = {8, 8, 1};

    VkCopyMemoryToImageIndirectCommandKHR cmd2 = {};
    cmd2.srcAddress = 1024;
    cmd2.bufferRowLength = 4;
    cmd2.bufferImageHeight = 4;
    cmd2.imageSubresource = {};
    cmd2.imageOffset = {0, 0, 0};
    cmd2.imageExtent = {4, 4, 1};

    VkCopyMemoryToImageIndirectCommandKHR cmds[2] = {cmd1, cmd2};

    vkt::Buffer indirect_buffer(*m_device, sizeof(cmds), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkImageSubresourceLayers res_layer = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers res_layers[2] = {res_layer, res_layer};

    m_command_buffer.Begin();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;
    vkt::Image src_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, image_create_info, vkt::set_layout);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    const uint32_t stride = sizeof(VkCopyMemoryToImageIndirectCommandKHR);

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = sizeof(cmds);
    address_range.stride = stride;

    VkCopyMemoryToImageIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 2;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstImage = dst_image;
    copy_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_info.pImageSubresources = res_layers;

    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, dst_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    vk::CmdCopyMemoryToImageIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();

    indirect_buffer.Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, BindResourceHeap) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize resource_stride = std::max(heap_props.resourceHeapAlignment, heap_props.imageDescriptorAlignment);

    const VkDeviceSize heap_size =
        Align(Align(resource_stride + heap_props.minResourceHeapReservedRange, heap_props.bufferDescriptorAlignment),
              heap_props.imageDescriptorAlignment);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    vkt::Buffer resource_heap(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = resource_heap.AddressRange();
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    resource_heap.Destroy();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, BindSamplerHeap) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize sampler_stride = std::max(heap_props.samplerHeapAlignment, heap_props.samplerDescriptorAlignment);
    const VkDeviceSize heap_size =
        Align(sampler_stride + heap_props.minSamplerHeapReservedRange, heap_props.samplerDescriptorAlignment);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    vkt::Buffer sampler_heap(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = sampler_heap.AddressRange();
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    sampler_heap.Destroy();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, PartialValidRange) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize offset = heap_props.minSamplerHeapReservedRange / 2;
    if (offset == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRange is too small";
    }

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = heap_props.minSamplerHeapReservedRange;
    vkt::Buffer heap1(*m_device, buffer_ci, vkt::no_mem);
    buffer_ci.size = heap_props.minSamplerHeapReservedRange - offset;
    vkt::Buffer heap2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, heap1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    memory_ai.allocationSize = memory_requirements.size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, heap1, memory, 0);
    vk::BindBufferMemory(*m_device, heap2, memory, offset);

    VkDeviceAddress address1 = heap1.Address();
    VkDeviceAddress address2 = heap2.Address();
    (void)address2;

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange.address = address1;
    bind_info.heapRange.size = heap_props.minSamplerHeapReservedRange;
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    heap1.Destroy();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, ValidRangeTooSmall) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize offset = heap_props.minSamplerHeapReservedRange / 2;
    if (offset == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRange is too small";
    }

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = heap_props.minSamplerHeapReservedRange;
    vkt::Buffer heap1(*m_device, buffer_ci, vkt::no_mem);
    buffer_ci.size = heap_props.minSamplerHeapReservedRange - offset;
    vkt::Buffer heap2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, heap1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    memory_ai.allocationSize = memory_requirements.size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, heap1, memory, 0);
    vk::BindBufferMemory(*m_device, heap2, memory, 0);

    VkDeviceAddress address1 = heap1.Address();
    VkDeviceAddress address2 = heap2.Address();
    (void)address2;

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange.address = address1;
    bind_info.heapRange.size = heap_props.minSamplerHeapReservedRange;
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    heap1.Destroy();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddress, RangeSplitBetweenBuffers) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize offset = heap_props.minSamplerHeapReservedRange / 2;
    if (offset == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRange is too small";
    }

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = heap_props.minSamplerHeapReservedRange;
    vkt::Buffer heap1(*m_device, buffer_ci, vkt::no_mem);
    buffer_ci.size = heap_props.minSamplerHeapReservedRange - offset;
    vkt::Buffer heap2(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer heap3(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, heap1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    memory_ai.allocationSize = memory_requirements.size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, heap1, memory, 0);
    vk::BindBufferMemory(*m_device, heap2, memory, 0);
    vk::BindBufferMemory(*m_device, heap3, memory, offset);

    VkDeviceAddress address1 = heap1.Address();
    VkDeviceAddress address2 = heap2.Address();
    VkDeviceAddress address3 = heap2.Address();
    (void)address2;
    (void)address3;

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange.address = address1;
    bind_info.heapRange.size = heap_props.minSamplerHeapReservedRange;
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    heap1.Destroy();
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
