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

bool QueryTest::HasZeroTimestampValidBits() {
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    return (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0);
}

TEST_F(PositiveQuery, ResetQueryPoolFromDifferentCB) {
    TEST_DESCRIPTION("Reset a query on one CB and use it in another.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();

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
        submit_info[0] = LvlInitStruct<VkSubmitInfo>();
        submit_info[0].commandBufferCount = 1;
        submit_info[0].pCommandBuffers = &command_buffer[0];
        submit_info[0].signalSemaphoreCount = 0;
        submit_info[0].pSignalSemaphores = nullptr;

        submit_info[1] = LvlInitStruct<VkSubmitInfo>();
        submit_info[1].commandBufferCount = 1;
        submit_info[1].pCommandBuffers = &command_buffer[1];
        submit_info[1].signalSemaphoreCount = 0;
        submit_info[1].pSignalSemaphores = nullptr;

        vk::QueueSubmit(m_device->m_queue, 2, &submit_info[0], VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(m_device->m_queue);

    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, command_buffer);
}

TEST_F(PositiveQuery, BasicQuery) {
    TEST_DESCRIPTION("Use a couple occlusion queries");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.size = 4 * sizeof(uint64_t);
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.init(*m_device, bci, mem_props);

    VkQueryPoolCreateInfo query_pool_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = 2;
    query_pool_info.pipelineStatistics = 0;
    vk_testing::QueryPool query_pool(*m_device, query_pool_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
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
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 1);
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 2, buffer.handle(), 0, sizeof(uint64_t),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_device->m_queue);
    uint64_t samples_passed[4];
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 2, sizeof(samples_passed), samples_passed, sizeof(uint64_t),
                            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    // Now reset query pool in a different command buffer than the BeginQuery
    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_device->m_queue);
    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(PositiveQuery, DestroyQueryPoolBasedOnQueryPoolResults) {
    TEST_DESCRIPTION("Destroy a QueryPool based on vkGetQueryPoolResults");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::array<uint64_t, 4> samples_passed = {};
    constexpr uint64_t sizeof_samples_passed = samples_passed.size() * sizeof(uint64_t);
    constexpr VkDeviceSize sample_stride = sizeof(uint64_t);

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.size = sizeof_samples_passed;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.init(*m_device, bci, mem_props);

    constexpr uint32_t query_count = 2;

    VkQueryPoolCreateInfo query_pool_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = query_count;
    query_pool_info.pipelineStatistics = 0;

    VkQueryPool query_pool;
    VkResult res = vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &query_pool);
    ASSERT_VK_SUCCESS(res);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
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
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 1);
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, query_count, buffer.handle(), 0, sample_stride,
                                query_flags);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    res = vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, query_count, sizeof_samples_passed, samples_passed.data(),
                                  sample_stride, query_flags);
    ASSERT_VK_SUCCESS(res);

    // "Applications can verify that queryPool can be destroyed by checking that vkGetQueryPoolResults() without the
    // VK_QUERY_RESULT_PARTIAL_BIT flag returns VK_SUCCESS for all queries that are used in command buffers submitted for
    // execution."
    //
    // i.e. You don't have to wait for an idle queue to destroy the query pool.
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(PositiveQuery, QueryAndCopySecondaryCommandBuffers) {
    TEST_DESCRIPTION("Issue a query on a secondary command buffer and copy it on a primary.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandPoolObj command_pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj primary_buffer(m_device, &command_pool);
    VkCommandBufferObj secondary_buffer(m_device, &command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;

    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
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

    ASSERT_NO_FATAL_FAILURE(Init());
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;

    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdResetQueryPool(command_buffer[0], query_pool.handle(), 0, 1);
        vk::CmdWriteTimestamp(command_buffer[0], VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);

        vk::EndCommandBuffer(command_buffer[0]);

        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        vk::CmdCopyQueryPoolResults(command_buffer[1], query_pool.handle(), 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);

        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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

    ASSERT_NO_FATAL_FAILURE(Init());
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(PositiveQuery, WriteTimestampNoneAndAll) {
    TEST_DESCRIPTION("Test using vkCmdWriteTimestamp2 with NONE and ALL_COMMANDS.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    synchronization2.synchronization2 = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>();
    features2.pNext = &synchronization2;
    InitState(nullptr, &features2);
    if (synchronization2.synchronization2 != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceSynchronization2FeaturesKHR::synchronization2 not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 2;
    query_pool.init(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    vk::CmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_NONE_KHR, query_pool.handle(), 0);
    vk::CmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR, query_pool.handle(), 1);
    m_commandBuffer->end();
}

TEST_F(PositiveQuery, CommandBufferInheritanceFlags) {
    TEST_DESCRIPTION("Test executing secondary command buffer with VkCommandBufferInheritanceInfo::queryFlags.");
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!m_device->phy().features().inheritedQueries) {
        GTEST_SKIP() << "inheritedQueries not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;
    vk_testing::QueryPool query_pool(*m_device, qpci);

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;
    cbii.occlusionQueryEnable = VK_TRUE;
    cbii.queryFlags = VK_QUERY_CONTROL_PRECISE_BIT;

    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
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
