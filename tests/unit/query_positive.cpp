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
#include "../framework/pipeline_helper.h"
#include "generated/vk_extension_helper.h"

bool QueryTest::HasZeroTimestampValidBits() {
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    return (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0);
}

TEST_F(PositiveQuery, ResetQueryPoolFromDifferentCB) {
    TEST_DESCRIPTION("Reset a query on one CB and use it in another.");

    RETURN_IF_SKIP(Init())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

        vk::BeginCommandBuffer(command_buffer[0], &begin_info);
        vk::CmdResetQueryPool(command_buffer[0], query_pool.handle(), 0, 1);
        vk::EndCommandBuffer(command_buffer[0]);

        vk::BeginCommandBuffer(command_buffer[1], &begin_info);
        vk::CmdBeginQuery(command_buffer[1], query_pool.handle(), 0, 0);
        vk::CmdEndQuery(command_buffer[1], query_pool.handle(), 0);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info[2]{};
        submit_info[0] = vku::InitStructHelper();
        submit_info[0].commandBufferCount = 1;
        submit_info[0].pCommandBuffers = &command_buffer[0];
        submit_info[0].signalSemaphoreCount = 0;
        submit_info[0].pSignalSemaphores = nullptr;

        submit_info[1] = vku::InitStructHelper();
        submit_info[1].commandBufferCount = 1;
        submit_info[1].pCommandBuffers = &command_buffer[1];
        submit_info[1].signalSemaphoreCount = 0;
        submit_info[1].pSignalSemaphores = nullptr;

        vk::QueueSubmit(m_default_queue, 2, &submit_info[0], VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(m_default_queue);

    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, command_buffer);
}

TEST_F(PositiveQuery, BasicQuery) {
    TEST_DESCRIPTION("Use a couple occlusion queries");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.size = 4 * sizeof(uint64_t);
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, bci, mem_props);

    VkQueryPoolCreateInfo query_pool_info = vku::InitStructHelper();
    query_pool_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = 2;
    query_pool_info.pipelineStatistics = 0;
    vkt::QueryPool query_pool(*m_device, query_pool_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 2);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 1);
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 2, buffer.handle(), 0, sizeof(uint64_t),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_default_queue);
    uint64_t samples_passed[4];
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 2, sizeof(samples_passed), samples_passed, sizeof(uint64_t),
                            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    // Now reset query pool in a different command buffer than the BeginQuery
    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveQuery, DestroyQueryPoolBasedOnQueryPoolResults) {
    TEST_DESCRIPTION("Destroy a QueryPool based on vkGetQueryPoolResults");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    std::array<uint64_t, 4> samples_passed = {};
    constexpr uint64_t sizeof_samples_passed = samples_passed.size() * sizeof(uint64_t);
    constexpr VkDeviceSize sample_stride = sizeof(uint64_t);

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.size = sizeof_samples_passed;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, bci, mem_props);

    constexpr uint32_t query_count = 2;

    VkQueryPoolCreateInfo query_pool_info = vku::InitStructHelper();
    query_pool_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = query_count;
    query_pool_info.pipelineStatistics = 0;

    VkQueryPool query_pool;
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &query_pool);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // If VK_QUERY_RESULT_WAIT_BIT is not set, vkGetQueryPoolResults may return VK_NOT_READY
    constexpr VkQueryResultFlags query_flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, query_count);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 1, 0);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 1);
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, query_count, buffer.handle(), 0, sample_stride,
                                query_flags);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    VkResult res = vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, query_count, sizeof_samples_passed,
                                           samples_passed.data(), sample_stride, query_flags);

    if (res == VK_SUCCESS) {
        // "Applications can verify that queryPool can be destroyed by checking that vkGetQueryPoolResults() without the
        // VK_QUERY_RESULT_PARTIAL_BIT flag returns VK_SUCCESS for all queries that are used in command buffers submitted for
        // execution."
        //
        // i.e. You don't have to wait for an idle queue to destroy the query pool.
        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
        vk::QueueWaitIdle(m_default_queue);
    } else {
        // some devices (pixel 7) will return VK_NOT_READY
        vk::QueueWaitIdle(m_default_queue);
        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
    }
}

TEST_F(PositiveQuery, QueryAndCopySecondaryCommandBuffers) {
    TEST_DESCRIPTION("Issue a query on a secondary command buffer and copy it on a primary.");

    RETURN_IF_SKIP(Init())
    if ((m_device->phy().queue_properties_.empty()) || (m_device->phy().queue_properties_[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer primary_buffer(m_device, &command_pool);
    vkt::CommandBuffer secondary_buffer(m_device, &command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = vku::InitStructHelper();
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;

    vkt::Buffer buffer(*m_device, buff_create_info);

    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    {
        VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
        begin_info.pInheritanceInfo = &hinfo;
        secondary_buffer.begin(&begin_info);
        vk::CmdResetQueryPool(secondary_buffer.handle(), query_pool.handle(), 0, 1);
        vk::CmdWriteTimestamp(secondary_buffer.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
        secondary_buffer.end();

        primary_buffer.begin();
        vk::CmdExecuteCommands(primary_buffer.handle(), 1, &secondary_buffer.handle());
        vk::CmdCopyQueryPoolResults(primary_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 0,
                                    VK_QUERY_RESULT_WAIT_BIT);
        primary_buffer.end();
    }

    primary_buffer.QueueCommandBuffer();
    vk::QueueWaitIdle(queue);
}

TEST_F(PositiveQuery, QueryAndCopyMultipleCommandBuffers) {
    TEST_DESCRIPTION("Issue a query and copy from it on a second command buffer.");

    RETURN_IF_SKIP(Init())
    if ((m_device->phy().queue_properties_.empty()) || (m_device->phy().queue_properties_[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkt::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = vku::InitStructHelper();
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;

    vkt::Buffer buffer(*m_device, buff_create_info);

    {
        VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdResetQueryPool(command_buffer[0], query_pool.handle(), 0, 1);
        vk::CmdWriteTimestamp(command_buffer[0], VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);

        vk::EndCommandBuffer(command_buffer[0]);

        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        vk::CmdCopyQueryPoolResults(command_buffer[1], query_pool.handle(), 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);

        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 2;
        submit_info.pCommandBuffers = command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(queue);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 2, command_buffer);
}

TEST_F(PositiveQuery, DestroyQueryPoolAfterGetQueryPoolResults) {
    TEST_DESCRIPTION("Destroy query pool after GetQueryPoolResults() without VK_QUERY_RESULT_PARTIAL_BIT returns VK_SUCCESS");

    RETURN_IF_SKIP(Init())
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveQuery, WriteTimestampNoneAndAll) {
    TEST_DESCRIPTION("Test using vkCmdWriteTimestamp2 with NONE and ALL_COMMANDS.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = vku::InitStructHelper();
    synchronization2.synchronization2 = VK_TRUE;
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper();
    features2.pNext = &synchronization2;
    InitState(nullptr, &features2);
    if (synchronization2.synchronization2 != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceSynchronization2FeaturesKHR::synchronization2 not supported";
    }
    InitRenderTarget();
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 2;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    vk::CmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_NONE_KHR, query_pool.handle(), 0);
    vk::CmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR, query_pool.handle(), 1);
    m_commandBuffer->end();
}

TEST_F(PositiveQuery, CommandBufferInheritanceFlags) {
    TEST_DESCRIPTION("Test executing secondary command buffer with VkCommandBufferInheritanceInfo::queryFlags.");
    RETURN_IF_SKIP(Init())
    if (!m_device->phy().features().inheritedQueries) {
        GTEST_SKIP() << "inheritedQueries not supported";
    }
    InitRenderTarget();

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, qpci);

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;
    cbii.occlusionQueryEnable = VK_TRUE;
    cbii.queryFlags = VK_QUERY_CONTROL_PRECISE_BIT;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
}

TEST_F(PositiveQuery, PerformanceQueries) {
    TEST_DESCRIPTION("Test performance queries.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    auto performance_query_features = vku::InitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(performance_query_features);

    if (!performance_query_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Test requires (unsupported) performanceCounterQueryPools";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    uint32_t counterCount = 0u;
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->phy(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, nullptr, nullptr);
    std::vector<VkPerformanceCounterKHR> counters(counterCount, vku::InitStruct<VkPerformanceCounterKHR>());
    std::vector<VkPerformanceCounterDescriptionKHR> counterDescriptions(counterCount,
                                                                        vku::InitStruct<VkPerformanceCounterDescriptionKHR>());
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->phy(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, counters.data(), counterDescriptions.data());

    std::vector<uint32_t> enabledCounters(128);
    const uint32_t enabledCounterCount = std::min(counterCount, static_cast<uint32_t>(enabledCounters.size()));
    for (uint32_t i = 0; i < enabledCounterCount; ++i) {
        enabledCounters[i] = i;
    }

    auto query_pool_performance_ci = vku::InitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    query_pool_performance_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    query_pool_performance_ci.counterIndexCount = enabledCounterCount;
    query_pool_performance_ci.pCounterIndices = enabledCounters.data();

    uint32_t num_passes = 0u;
    vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(m_device->phy(), &query_pool_performance_ci, &num_passes);

    auto query_pool_ci = vku::InitStruct<VkQueryPoolCreateInfo>(&query_pool_performance_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1u;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    {
        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0u, 1u);
        m_commandBuffer->end();

        auto submit_info = vku::InitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::DeviceWaitIdle(*m_device);
    }

    vkt::CommandBuffer cmd_buffer(m_device, m_commandPool);

    auto acquire_profiling_lock_info = vku::InitStruct<VkAcquireProfilingLockInfoKHR>();
    acquire_profiling_lock_info.timeout = std::numeric_limits<uint64_t>::max();

    vk::AcquireProfilingLockKHR(*m_device, &acquire_profiling_lock_info);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    cmd_buffer.begin(&info);

    vk::CmdBeginQuery(cmd_buffer.handle(), query_pool.handle(), 0u, 0u);

    vk::CmdPipelineBarrier(cmd_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0u, 0u,
                           nullptr, 0u, nullptr, 0u, nullptr);

    vk::CmdEndQuery(cmd_buffer.handle(), query_pool.handle(), 0u);

    cmd_buffer.end();

    for (uint32_t counterPass = 0u; counterPass < num_passes; ++counterPass) {
        auto performance_query_submit_info = vku::InitStruct<VkPerformanceQuerySubmitInfoKHR>();
        performance_query_submit_info.counterPassIndex = counterPass;

        auto submit_info = vku::InitStruct<VkSubmitInfo>(&performance_query_submit_info);
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = &cmd_buffer.handle();
        vk::QueueSubmit(m_default_queue, 1u, &submit_info, VK_NULL_HANDLE);
        vk::DeviceWaitIdle(*m_device);
    }

    vk::ReleaseProfilingLockKHR(*m_device);

    std::vector<VkPerformanceCounterResultKHR> recordedCounters(enabledCounterCount);
    vk::GetQueryPoolResults(*m_device, query_pool.handle(), 0u, 1u, sizeof(VkPerformanceCounterResultKHR) * enabledCounterCount,
                            recordedCounters.data(), sizeof(VkPerformanceCounterResultKHR) * enabledCounterCount, 0u);
}
