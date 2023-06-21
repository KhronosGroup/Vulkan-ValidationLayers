/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

class PositiveBuffer : public VkPositiveLayerTest {};

TEST_F(PositiveBuffer, OwnershipTranfers) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx.value())[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj no_gfx_cb(m_device, &no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, no_gfx_queue);

    // Create a buffer
    const VkDeviceSize buffer_size = 256;
    uint8_t data[buffer_size] = {0xFF};
    VkConstantBufferObj buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized());
    auto buffer_barrier = buffer.buffer_memory_barrier(0, 0, 0, VK_WHOLE_SIZE);

    // Let gfx own it.
    buffer_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ValidOwnershipTransferOp(m_errorMonitor, m_commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             &buffer_barrier, nullptr);

    // Transfer it to non-gfx
    buffer_barrier.dstQueueFamilyIndex = no_gfx.value();
    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, &buffer_barrier, nullptr);

    // Transfer it to gfx
    buffer_barrier.srcQueueFamilyIndex = no_gfx.value();
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, &buffer_barrier, nullptr);
}

TEST_F(PositiveBuffer, TexelBufferAlignmentIn13) {
    TEST_DESCRIPTION("texelBufferAlignment is enabled by default in 1.3.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    const VkDeviceSize minTexelBufferOffsetAlignment = m_device->props.limits.minTexelBufferOffsetAlignment;
    if (minTexelBufferOffsetAlignment == 1) {
        GTEST_SKIP() << "Test requires minTexelOffsetAlignment to not be equal to 1";
    }

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Test requires support for VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT";
    }

    auto props_1_3 = LvlInitStruct<VkPhysicalDeviceVulkan13Properties>();
    GetPhysicalDeviceProperties2(props_1_3);
    if (props_1_3.uniformTexelBufferOffsetAlignmentBytes < 4 || !props_1_3.uniformTexelBufferOffsetSingleTexelAlignment) {
        GTEST_SKIP() << "need uniformTexelBufferOffsetAlignmentBytes to be more than 4 with "
                        "uniformTexelBufferOffsetSingleTexelAlignment support";
    }

    // to prevent VUID-VkBufferViewCreateInfo-buffer-02751
    const uint32_t block_size = 4;  // VK_FORMAT_R8G8B8A8_UNORM

    const VkBufferCreateInfo buffer_info = VkBufferObj::create_info(1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
    buff_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    buff_view_ci.range = VK_WHOLE_SIZE;
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.offset = minTexelBufferOffsetAlignment + block_size;
    CreateBufferViewTest(*this, &buff_view_ci, {});
}

TEST_F(PositiveBuffer, DISABLED_PerfGetBufferAddressWorstCase) {
    TEST_DESCRIPTION("Add elements to buffer_address_map, worst case scenario");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_addr_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(buffer_addr_features);
    if (!buffer_addr_features.bufferDeviceAddress) {
        GTEST_SKIP() << "bufferDeviceAddress not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Allocate common buffer memory, all buffers will be bound to it so that they have the same starting address
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 100 * 4096 * 4096;
    vk_testing::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers. They have the same starting offset, but a growing size.
    // This is the worst case scenario for adding an element in the current buffer_address_map: inserted range will have to be split
    // for every range currently in the map.
    constexpr size_t N = 1400;
    std::vector<VkBufferObj> buffers(N);
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkDeviceAddress ref_address = 0;

    for (size_t i = 0; i < N; ++i) {
        VkBufferObj &buffer = buffers[i];
        buffer_ci.size = (i + 1) * 4096;
        buffer.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), buffer.handle(), buffer_memory.handle(), 0);
        VkDeviceAddress addr = buffer.address(DeviceValidationVersion());
        if (ref_address == 0) {
            ref_address = addr;
        }
        if (addr != ref_address) {
            GTEST_SKIP() << "At iteration " << i << ", retrieved buffer address (" << addr << ") != reference address ("
                         << ref_address << ")";
        }
    }
}

TEST_F(PositiveBuffer, DISABLED_PerfGetBufferAddressGoodCase) {
    TEST_DESCRIPTION("Add elements to buffer_address_map, good case scenario");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_addr_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(buffer_addr_features);
    if (!buffer_addr_features.bufferDeviceAddress) {
        GTEST_SKIP() << "bufferDeviceAddress not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Allocate common buffer memory, all buffers will be bound to it so that they have the same starting address
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 100 * 4096 * 4096;
    vk_testing::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers. They have consecutive device address ranges, so no overlaps: no split will be needed when inserting, it
    // should be fast.
    constexpr size_t N = 1400;  // 100 * 4096;
    std::vector<VkBufferObj> buffers(N);
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    for (size_t i = 0; i < N; ++i) {
        VkBufferObj &buffer = buffers[i];
        buffer.init_no_mem(*m_device, buffer_ci);
        // Consecutive offsets
        vk::BindBufferMemory(device(), buffer.handle(), buffer_memory.handle(), i * buffer_ci.size);
        (void)buffer.address(DeviceValidationVersion());
    }
}
