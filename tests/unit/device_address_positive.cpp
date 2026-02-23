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
#include "../framework/layer_validation_tests.h"
#include "containers/range.h"

class PositiveDeviceAddress : public VkLayerTest {};

TEST_F(PositiveDeviceAddress, DestroyOneOutOfMultipleBuffers) {
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

    bool pass =
        m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDeviceAddress, DestroyOneBufferBeforeCopy) {
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

    bool pass =
        m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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

    indirect_buffer1.Destroy();

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &indirect_info);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDeviceAddress, ValidSubRange1) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize heap_size = heap_props.minSamplerHeapReservedRange + 256;
    const VkDeviceSize offset = heap_size / 2;

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = heap_size + offset * 2;
    vkt::Buffer heap1(*m_device, buffer_ci, vkt::no_mem);
    buffer_ci.size = heap_size;
    vkt::Buffer heap2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, heap1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    // make large enough if offset is not perfectly in the middle of the allocation
    memory_ai.allocationSize = memory_requirements.size * 2;
    if ((offset % memory_requirements.alignment) != 0) {
        GTEST_SKIP() << "Alignment not met";
    }

    bool pass =
        m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, heap1, memory, 0);
    vk::BindBufferMemory(*m_device, heap2, memory, offset);

    VkDeviceAddress address1 = heap1.Address();
    VkDeviceAddress address2 = heap2.Address();
    vvl::range<VkDeviceAddress> range1(address1, address1 + heap_size + offset * 2);
    vvl::range<VkDeviceAddress> range2(address2, address2 + heap_size);
    if (!range1.includes(range2)) {
        GTEST_SKIP() << "Address are not overlapping";
    }
    (void)address1;

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {address2, heap_size};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    heap2.Destroy();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDeviceAddress, ValidSubRange2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&heap_props);
    GetPhysicalDeviceProperties2(props2);

    const VkDeviceSize heap_size = heap_props.minSamplerHeapReservedRange + 256;
    const VkDeviceSize offset = heap_size / 2;

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = heap_size + offset * 2;
    vkt::Buffer heap1(*m_device, buffer_ci, vkt::no_mem);
    buffer_ci.size = heap_size;
    vkt::Buffer heap2(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements memory_requirements;
    vk::GetBufferMemoryRequirements(*m_device, heap1, &memory_requirements);

    VkMemoryAllocateFlagsInfo memory_allocate_flags = vku::InitStructHelper();
    memory_allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&memory_allocate_flags);
    // make large enough if offset is not perfectly in the middle of the allocation
    memory_ai.allocationSize = memory_requirements.size * 2;

    bool pass =
        m_device->Physical().SetMemoryType(memory_requirements.memoryTypeBits, &memory_ai, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_ai);

    vk::BindBufferMemory(*m_device, heap1, memory, 0);
    vk::BindBufferMemory(*m_device, heap2, memory, offset);

    VkDeviceAddress address1 = heap1.Address();
    VkDeviceAddress address2 = heap2.Address();
    (void)address1;

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {address2, heap_size};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    heap1.Destroy();
    m_default_queue->SubmitAndWait(m_command_buffer);
}
