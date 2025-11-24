/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"

class StressCore : public VkLayerTest {};

TEST_F(StressCore, BufferCopies) {
    TEST_DESCRIPTION("Do many buffer copies, make sure perf is good");

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 1024;
    vkt::Buffer src_buffer(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer dst_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), src_buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    buffer_mem_alloc.allocationSize *= 2;

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);
    src_buffer.BindMemory(buffer_mem, 0);
    dst_buffer.BindMemory(buffer_mem, 1024);

    constexpr VkDeviceSize copy_size = 1024 / 4;
    std::vector<VkBufferCopy> copy_info_list(4);
    copy_info_list[3].srcOffset = 0;
    copy_info_list[3].dstOffset = 0;
    copy_info_list[3].size = copy_size;

    copy_info_list[2].srcOffset = copy_size;
    copy_info_list[2].dstOffset = copy_size;
    copy_info_list[2].size = copy_size;

    copy_info_list[1].srcOffset = 2 * copy_size;
    copy_info_list[1].dstOffset = 2 * copy_size;
    copy_info_list[1].size = copy_size;

    copy_info_list[0].srcOffset = 3 * copy_size;
    copy_info_list[0].dstOffset = 3 * copy_size;
    copy_info_list[0].size = copy_size;

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + 4] = copy_info_list[i % 4];
    }

    m_command_buffer.Begin();

    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, size32(copy_info_list), copy_info_list.data());

    m_command_buffer.End();
}

TEST_F(StressCore, BufferCopiesSparseBuffer) {
    TEST_DESCRIPTION("Validate 10,000 buffer copies");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr VkDeviceSize copy_size = 16;
    std::vector<VkBufferCopy> copy_info_list(4);
    copy_info_list[3].srcOffset = 0;
    copy_info_list[3].dstOffset = 16;
    copy_info_list[3].size = copy_size;

    copy_info_list[2].srcOffset = 64 + copy_size + 0 * 16;
    copy_info_list[2].dstOffset = 32;
    copy_info_list[2].size = copy_size;

    copy_info_list[1].srcOffset = 64 + copy_size + 1 * 16;
    copy_info_list[1].dstOffset = 48;
    copy_info_list[1].size = copy_size;

    copy_info_list[0].srcOffset = 64 + copy_size + 2 * 16;
    copy_info_list[0].dstOffset = 64;
    copy_info_list[0].size = copy_size;

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + 4] = copy_info_list[i % 4];
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind_1 = {};
    buffer_memory_bind_1.size = buffer_mem_reqs.size;
    buffer_memory_bind_1.memory = buffer_mem;

    std::array<VkSparseBufferMemoryBindInfo, 1> buffer_memory_bind_infos = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse;
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind_1;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = size32(buffer_memory_bind_infos);
    bind_info.pBufferBinds = buffer_memory_bind_infos.data();
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, buffer_sparse, buffer_sparse, size32(copy_info_list), copy_info_list.data());
    m_command_buffer.End();

    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(StressCore, BufferCopiesSparseBuffer2) {
    TEST_DESCRIPTION("Validate 10,000 buffer copies, buffer is bound to multiple VkDeviceMemory");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr int memory_chunks_count = 100;
    constexpr VkDeviceSize memory_size = 0x10000;
    constexpr VkDeviceSize copy_size = memory_size / 8;
    std::vector<VkBufferCopy> copy_info_list(4 * memory_chunks_count);
    for (int i = 0; i < memory_chunks_count * 4; i += 4) {
        copy_info_list[i + 3].srcOffset = memory_size * (i / 4);
        copy_info_list[i + 3].dstOffset = memory_size * (i / 4) + copy_size;
        copy_info_list[i + 3].size = copy_size;

        copy_info_list[i + 2].srcOffset = copy_info_list[i + 3].dstOffset + copy_size;
        copy_info_list[i + 2].dstOffset = copy_info_list[i + 2].srcOffset + copy_size;
        copy_info_list[i + 2].size = copy_size;

        copy_info_list[i + 1].srcOffset = copy_info_list[i + 2].dstOffset + copy_size;
        copy_info_list[i + 1].dstOffset = copy_info_list[i + 1].srcOffset + copy_size;
        copy_info_list[i + 1].size = copy_size;

        copy_info_list[i + 0].srcOffset = copy_info_list[i + 1].dstOffset + copy_size;
        copy_info_list[i + 0].dstOffset = copy_info_list[i + 0].srcOffset + copy_size;
        copy_info_list[i + 0].size = copy_size;
    }

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + memory_chunks_count * 4] = copy_info_list[i % (4 * memory_chunks_count)];
    }

    VkBufferCreateInfo b_info = vkt::Buffer::CreateInfo(memory_chunks_count * memory_size,
                                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    std::vector<vkt::DeviceMemory> memory_chunks(memory_chunks_count);

    for (auto& mem : memory_chunks) {
        mem.Init(*m_device, buffer_mem_alloc);
    }

    std::vector<VkSparseMemoryBind> buffer_memory_binds(memory_chunks_count);
    for (auto [i, mem_bind] : vvl::enumerate(buffer_memory_binds)) {
        mem_bind.size = memory_size;
        mem_bind.memory = memory_chunks[i];
        mem_bind.resourceOffset = i * memory_size;
        mem_bind.memoryOffset = 0;
    }

    std::array<VkSparseBufferMemoryBindInfo, 1> buffer_memory_bind_infos = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse;
    buffer_memory_bind_infos[0].bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_infos[0].pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = size32(buffer_memory_bind_infos);
    bind_info.pBufferBinds = buffer_memory_bind_infos.data();
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, buffer_sparse, buffer_sparse, size32(copy_info_list), copy_info_list.data());
    m_command_buffer.End();

    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(StressCore, SignalSemaphoreManyTimes) {
    // On good enough machines (2025) this should take less than a second, on slower machines maybe few seconds.
    // With the regression, expect the running time to range from tens of seconds up to a few minutes.
    TEST_DESCRIPTION("Signaling semaphore many time without waiting should not cause slow down");
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    const int N = 10000;
    for (int i = 1; i <= N; i++) {
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, i));
    }
    m_device->Wait();
}
