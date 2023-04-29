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
