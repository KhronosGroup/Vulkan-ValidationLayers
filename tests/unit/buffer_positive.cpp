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

TEST_F(PositiveBuffer, OwnershipTranfers) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }
    vkt::Queue *no_gfx_queue = m_device->queue_family_queues(no_gfx.value())[0].get();

    vkt::CommandPool no_gfx_pool(*m_device, no_gfx.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer no_gfx_cb(m_device, &no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, no_gfx_queue);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
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
    RETURN_IF_SKIP(Init())

    const VkDeviceSize minTexelBufferOffsetAlignment = m_device->phy().limits_.minTexelBufferOffsetAlignment;
    if (minTexelBufferOffsetAlignment == 1) {
        GTEST_SKIP() << "Test requires minTexelOffsetAlignment to not be equal to 1";
    }

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Test requires support for VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT";
    }

    VkPhysicalDeviceVulkan13Properties props_1_3 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props_1_3);
    if (props_1_3.uniformTexelBufferOffsetAlignmentBytes < 4 || !props_1_3.uniformTexelBufferOffsetSingleTexelAlignment) {
        GTEST_SKIP() << "need uniformTexelBufferOffsetAlignmentBytes to be more than 4 with "
                        "uniformTexelBufferOffsetSingleTexelAlignment support";
    }

    // to prevent VUID-VkBufferViewCreateInfo-buffer-02751
    const uint32_t block_size = 4;  // VK_FORMAT_R8G8B8A8_UNORM

    const VkBufferCreateInfo buffer_info = vkt::Buffer::create_info(1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    vkt::Buffer buffer(*m_device, buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_addr_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(buffer_addr_features);
    RETURN_IF_SKIP(InitState(nullptr, &buffer_addr_features));

    // Allocate common buffer memory, all buffers will be bound to it so that they have the same starting address
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 100 * 4096 * 4096;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers. They have the same starting offset, but a growing size.
    // This is the worst case scenario for adding an element in the current buffer_address_map: inserted range will have to be split
    // for every range currently in the map.
    constexpr size_t N = 1400;
    std::vector<vkt::Buffer> buffers(N);
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkDeviceAddress ref_address = 0;

    for (size_t i = 0; i < N; ++i) {
        vkt::Buffer &buffer = buffers[i];
        buffer_ci.size = (i + 1) * 4096;
        buffer.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), buffer.handle(), buffer_memory.handle(), 0);
        VkDeviceAddress addr = buffer.address();
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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_addr_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(buffer_addr_features);
    RETURN_IF_SKIP(InitState(nullptr, &buffer_addr_features));

    // Allocate common buffer memory, all buffers will be bound to it so that they have the same starting address
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 100 * 4096 * 4096;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers. They have consecutive device address ranges, so no overlaps: no split will be needed when inserting, it
    // should be fast.
    constexpr size_t N = 1400;  // 100 * 4096;
    std::vector<vkt::Buffer> buffers(N);
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    for (size_t i = 0; i < N; ++i) {
        vkt::Buffer &buffer = buffers[i];
        buffer.init_no_mem(*m_device, buffer_ci);
        // Consecutive offsets
        vk::BindBufferMemory(device(), buffer.handle(), buffer_memory.handle(), i * buffer_ci.size);
        (void)buffer.address();
    }
}

TEST_F(PositiveBuffer, IndexBuffer2Size) {
    TEST_DESCRIPTION("Valid vkCmdBindIndexBuffer2KHR");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));
    InitRenderTarget();

    const uint32_t buffer_size = 32;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 4, 8, VK_INDEX_TYPE_UINT32);

    vk::CmdBindIndexBuffer2KHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer_size, VK_INDEX_TYPE_UINT32);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveBuffer, BufferViewUsageBasic) {
    TEST_DESCRIPTION("VkBufferUsageFlags2CreateInfoKHR with good flags.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci);

    VkBufferUsageFlags2CreateInfoKHR buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR;

    VkBufferViewCreateInfo buffer_view_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    buffer_view_ci.range = VK_WHOLE_SIZE;
    buffer_view_ci.buffer = buffer.handle();
    CreateBufferViewTest(*this, &buffer_view_ci, {});
}

TEST_F(PositiveBuffer, BufferUsageFlags2Subset) {
    TEST_DESCRIPTION("VkBufferUsageFlags2CreateInfoKHR that are a subset of the Buffer.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci);

    VkBufferUsageFlags2CreateInfoKHR buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR | VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT_KHR;

    VkBufferViewCreateInfo buffer_view_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    buffer_view_ci.range = VK_WHOLE_SIZE;
    buffer_view_ci.buffer = buffer.handle();
    CreateBufferViewTest(*this, &buffer_view_ci, {});
}

TEST_F(PositiveBuffer, BufferUsageFlags2Ignore) {
    TEST_DESCRIPTION("Ignore old flags if using VkBufferUsageFlags2CreateInfoKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

    VkBufferUsageFlags2CreateInfoKHR buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, {});

    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR |
                      VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, {});
}

TEST_F(PositiveBuffer, BufferUsageFlags2Usage) {
    TEST_DESCRIPTION("Ignore old flags if using VkBufferUsageFlags2CreateInfoKHR, even if bad.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

    VkBufferUsageFlags2CreateInfoKHR buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = 32;
    buffer_ci.usage = 0;
    CreateBufferTest(*this, &buffer_ci, {});

    buffer_ci.usage = 0xBAD0000;
    CreateBufferTest(*this, &buffer_ci, {});
}
