/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Collabora, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy) {
    TEST_DESCRIPTION("Test correct non overlapping sparse buffers' copy");

    RETURN_IF_SKIP(Init())

    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Requires unsupported sparseBinding feature.";
    }

    const std::optional<uint32_t> sparse_index = m_device->QueueFamilyMatching(VK_QUEUE_SPARSE_BINDING_BIT, 0u);
    if (!sparse_index) {
        GTEST_SKIP() << "Required queue families not present";
    }

    // 2 semaphores needed since we need to bind twice before copying
    vkt::Semaphore semaphore(*m_device);
    vkt::Semaphore semaphore2(*m_device);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 256;

    VkBufferCreateInfo b_info =
        vkt::Buffer::create_info(copy_info.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse;
    buffer_sparse.init_no_mem(*m_device, b_info);
    vkt::Buffer buffer_sparse2;
    buffer_sparse2.init_no_mem(*m_device, b_info);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem2;
    buffer_mem2.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_infos[2] = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind;
    buffer_memory_bind_infos[1].buffer = buffer_sparse2.handle();
    buffer_memory_bind_infos[1].bindCount = 1;
    buffer_memory_bind_infos[1].pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 2;
    bind_info.pBufferBinds = buffer_memory_bind_infos;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->graphics_queues()[sparse_index.value()]->handle();

    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    // Set up complete

    m_commandBuffer->begin();
    // This copy is be completely legal as long as we change the memory for buffer_sparse to not overlap with
    // buffer_sparse2's memory on queue submission, or viceversa
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_sparse.handle(), buffer_sparse2.handle(), 1, &copy_info);
    m_commandBuffer->end();

    // Rebind buffer_mem2 so it does not overlap
    buffer_memory_bind.memory = buffer_mem2.handle();
    bind_info.bufferBindCount = 1;
    bind_info.waitSemaphoreCount = 1;
    bind_info.pWaitSemaphores = &semaphore.handle();
    bind_info.pSignalSemaphores = &semaphore2.handle();
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);

    // Submitting copy command with non overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore2.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy2) {
    TEST_DESCRIPTION("Non overlapping ranges copies should not trigger errors");

    RETURN_IF_SKIP(Init())

    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Requires unsupported sparseBinding feature.";
    }

    const std::optional<uint32_t> sparse_index = m_device->QueueFamilyMatching(VK_QUEUE_SPARSE_BINDING_BIT, 0u);
    if (!sparse_index) {
        GTEST_SKIP() << "Required queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr VkDeviceSize copy_size = 16;
    // Consecutive ranges
    std::array<VkBufferCopy, 3> copy_info_list = {};
    copy_info_list[0].srcOffset = 0 * copy_size;
    copy_info_list[0].dstOffset = 1 * copy_size;
    copy_info_list[0].size = copy_size;

    copy_info_list[1].srcOffset = 2 * copy_size;
    copy_info_list[1].dstOffset = 3 * copy_size;
    copy_info_list[1].size = copy_size;

    copy_info_list[2].srcOffset = 4 * copy_size;
    copy_info_list[2].dstOffset = 5 * copy_size;
    copy_info_list[2].size = copy_size;

    VkBufferCreateInfo b_info =
        vkt::Buffer::create_info(256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse;
    buffer_sparse.init_no_mem(*m_device, b_info);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind_1 = {};
    buffer_memory_bind_1.size = buffer_mem_reqs.size;
    buffer_memory_bind_1.memory = buffer_mem.handle();

    std::array<VkSparseBufferMemoryBindInfo, 1> buffer_memory_bind_infos = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind_1;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = size32(buffer_memory_bind_infos);
    bind_info.pBufferBinds = buffer_memory_bind_infos.data();
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->graphics_queues()[sparse_index.value()]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.wait(kWaitTimeout));
    // Set up complete

    m_commandBuffer->begin();
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_sparse.handle(), buffer_sparse.handle(), size32(copy_info_list),
                      copy_info_list.data());
    m_commandBuffer->end();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy3) {
    TEST_DESCRIPTION("Test that overlaps are computed in buffer space, not memory space");

    RETURN_IF_SKIP(Init())

    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Requires unsupported sparseBinding feature.";
    }

    const std::optional<uint32_t> sparse_index = m_device->QueueFamilyMatching(VK_QUEUE_SPARSE_BINDING_BIT, 0u);
    if (!sparse_index) {
        GTEST_SKIP() << "Required queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    VkBufferCreateInfo buffer_ci =
        vkt::Buffer::create_info(4096 * 32, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr);
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse;
    buffer_sparse.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    buffer_sparse.destroy();
    buffer_ci.size = 2 * buffer_mem_reqs.alignment;
    buffer_sparse.init_no_mem(*m_device, buffer_ci);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;                          // srcOffset is the start of buffer_mem_1, or 0 in this space.
    copy_info.dstOffset = buffer_mem_reqs.alignment;  // dstOffset is the start of buffer_mem_2, or 0 in this space
                                                      // => since overlaps are computed in buffer space, none should be detected
    copy_info.size = buffer_mem_reqs.alignment;

    vkt::DeviceMemory buffer_mem_1(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem_2(*m_device, buffer_mem_alloc);

    std::array<VkSparseMemoryBind, 2> buffer_memory_binds = {};
    buffer_memory_binds[0].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[0].memory = buffer_mem_1.handle();
    buffer_memory_binds[1].resourceOffset = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].memory = buffer_mem_2.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_info.pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->graphics_queues()[sparse_index.value()]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.wait(kWaitTimeout));
    // Set up complete

    m_commandBuffer->begin();
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_sparse.handle(), buffer_sparse.handle(), 1, &copy_info);
    m_commandBuffer->end();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    vk::QueueWaitIdle(m_default_queue);
    vk::QueueWaitIdle(sparse_queue);
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy4) {
    TEST_DESCRIPTION("Test coyping from a range that spans two different memory chunks");

    RETURN_IF_SKIP(Init())

    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Requires unsupported sparseBinding feature.";
    }

    const std::optional<uint32_t> sparse_index = m_device->QueueFamilyMatching(VK_QUEUE_SPARSE_BINDING_BIT, 0u);
    if (!sparse_index) {
        GTEST_SKIP() << "Required queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    VkBufferCreateInfo buffer_ci =
        vkt::Buffer::create_info(4096 * 32, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr);
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse;
    buffer_sparse.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    if (buffer_mem_reqs.alignment <= 1) {
        GTEST_SKIP() << "Buffer copy will not work as intended if VkMemoryRequirements::alignment is not superior to 1";
    }
    buffer_sparse.destroy();
    buffer_ci.size = 2 * buffer_mem_reqs.alignment;
    buffer_sparse.init_no_mem(*m_device, buffer_ci);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem_1(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem_2(*m_device, buffer_mem_alloc);

    std::array<VkSparseMemoryBind, 2> buffer_memory_binds = {};
    buffer_memory_binds[0].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[0].memory = buffer_mem_1.handle();
    buffer_memory_binds[1].resourceOffset = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].memory = buffer_mem_2.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_info.pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->graphics_queues()[sparse_index.value()]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.wait(kWaitTimeout));
    // Set up complete

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;                              // srcOffset is the start of buffer_mem_1, or 0 in this space.
    copy_info.dstOffset = buffer_mem_reqs.alignment / 2;  // dstOffset is the start of buffer_mem_2, or 0 in this space
                                                          // => since overlaps are computed in buffer space, none should be detected
    copy_info.size = buffer_mem_reqs.alignment / 2;
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_sparse.handle(), buffer_sparse.handle(), 1, &copy_info);
    m_commandBuffer->end();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    vk::QueueWaitIdle(m_default_queue);
    vk::QueueWaitIdle(sparse_queue);
}
