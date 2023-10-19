/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(NegativeQuery, PerformanceCreation) {
    TEST_DESCRIPTION("Create performance query without support");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &performance_features));
    auto queueFamilyProperties = m_device->phy().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;

    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;
        break;
    }

    if (counters.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counters.size();
    std::vector<uint32_t> counterIndices;
    for (uint32_t c = 0; c < counters.size(); c++) counterIndices.push_back(c);
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    vkt::QueryPool query_pool;

    // Missing pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryType-03222");
    query_pool.init(*m_device, query_pool_ci);
    m_errorMonitor->VerifyFound();

    query_pool_ci.pNext = &perf_query_pool_ci;

    // Invalid counter indices
    counterIndices.push_back(counters.size());
    perf_query_pool_ci.counterIndexCount++;
    perf_query_pool_ci.pCounterIndices = counterIndices.data();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolPerformanceCreateInfoKHR-pCounterIndices-03321");
    query_pool.init(*m_device, query_pool_ci);
    m_errorMonitor->VerifyFound();
    perf_query_pool_ci.counterIndexCount--;
    counterIndices.pop_back();

    // Success
    query_pool.init(*m_device, query_pool_ci);

    m_commandBuffer->begin();

    // Missing acquire lock
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03223");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeQuery, PerformanceCounterCommandbufferScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the command buffer counter scope");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &performance_features));

    auto queueFamilyProperties = m_device->phy().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_BUFFER_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_BUFFER_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with command buffer scope.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Not the first command.
    {
        VkBufferCreateInfo buf_info = vku::InitStructHelper();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_EQ(VK_SUCCESS, err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_EQ(VK_SUCCESS, err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03224");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    // Not last command.
    {
        VkBufferCreateInfo buf_info = vku::InitStructHelper();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_EQ(VK_SUCCESS, err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_EQ(VK_SUCCESS, err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();

        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-queryPool-03227");
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceCounterRenderPassScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the render pass counter scope");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &performance_features));
    auto queueFamilyProperties = m_device->phy().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_RENDER_PASS_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_RENDER_PASS_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with render pass scope.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Inside a render pass.
    {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03225");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceReleaseProfileLockBeforeSubmit) {
    TEST_DESCRIPTION("Verify that we get an error if we release the profiling lock during the recording of performance queries");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &performance_features));

    auto queueFamilyProperties = m_device->phy().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with render pass scope.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
        m_commandBuffer->end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    {
        VkBufferCreateInfo buf_info = vku::InitStructHelper();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_EQ(VK_SUCCESS, err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_EQ(VK_SUCCESS, err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);

        // Release while recording.
        vk::ReleaseProfilingLockKHR(device());
        {
            VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
            VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
            ASSERT_TRUE(result == VK_SUCCESS);
        }

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-03220");
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(queue);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceIncompletePasses) {
    TEST_DESCRIPTION("Verify that we get an error if we don't submit a command buffer for each passes before getting the results.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

    // Vulkan 1.1 is a dependency of VK_KHR_video_queue, but both the version and the extension
    // is optional from the point of view of this test case
    SetTargetApiVersion(VK_API_VERSION_1_1);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_features = vku::InitStructHelper(&host_query_reset_features);
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR doesn't match up with profile queues";
    }

    VkPhysicalDevicePerformanceQueryPropertiesKHR perf_query_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(perf_query_props);

    RETURN_IF_SKIP(InitState(nullptr, &performance_features));
    auto queueFamilyProperties = m_device->phy().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;
    uint32_t nPasses = 0;

    // Find all counters with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) counterIndices.push_back(counterIdx);
        }
        if (counterIndices.empty()) continue;  // might not be a scope command

        VkQueryPoolPerformanceCreateInfoKHR create_info = vku::InitStructHelper();
        create_info.queueFamilyIndex = idx;
        create_info.counterIndexCount = counterIndices.size();
        create_info.pCounterIndices = &counterIndices[0];

        vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(gpu(), &create_info, &nPasses);

        if (nPasses < 2) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        const VkDeviceSize buf_size =
            std::max((VkDeviceSize)4096, (VkDeviceSize)(sizeof(VkPerformanceCounterResultKHR) * counterIndices.size()));

        VkBufferCreateInfo buf_info = vku::InitStructHelper();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = buf_size;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_EQ(VK_SUCCESS, err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = mem_reqs.size;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_EQ(VK_SUCCESS, err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        vk::ResetQueryPoolEXT(m_device->device(), query_pool.handle(), 0, 1);

        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, buf_size, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
        m_commandBuffer->end();

        // Invalid pass index
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = nPasses;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPerformanceQuerySubmitInfoKHR-counterPassIndex-03221");
            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }

        // Leave the last pass out.
        for (uint32_t passIdx = 0; passIdx < (nPasses - 1); passIdx++) {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = passIdx;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);

        std::vector<VkPerformanceCounterResultKHR> results;
        results.resize(counterIndices.size());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03231");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyFound();

        // submit the last pass
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = nPasses - 1;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);

        // The stride is too small to return the data
        if (counterIndices.size() > 2) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04519");
            vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1,
                                    sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                    sizeof(VkPerformanceCounterResultKHR) * (results.size() - 1), 0);
            m_errorMonitor->VerifyFound();
        }

        // Invalid stride
        {
            std::vector<VkPerformanceCounterResultKHR> results_invalid_stride;
            results_invalid_stride.resize(counterIndices.size() * 2);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03229");
            vk::GetQueryPoolResults(
                device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size(),
                &results_invalid_stride[0], sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size() + 4,
                VK_QUERY_RESULT_WAIT_BIT);
            m_errorMonitor->VerifyFound();
        }

        // Invalid flags for vkCmdCopyQueryPoolResults
        if (perf_query_props.allowCommandBufferQueryCopies) {
            m_commandBuffer->begin(&command_buffer_begin_info);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-03233");
            vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer, 0,
                                        sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                        VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
            m_errorMonitor->VerifyFound();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-03233");
            vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer, 0,
                                        sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_PARTIAL_BIT);
            m_errorMonitor->VerifyFound();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-03233");
            vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer, 0,
                                        sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_64_BIT);
            m_errorMonitor->VerifyFound();
            if (IsExtensionsEnabled(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-03233");
                vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer, 0,
                                            sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                            VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        }

        // Invalid flags for vkGetQueryPoolResults
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_PARTIAL_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();
        if (IsExtensionsEnabled(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
            vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
            m_errorMonitor->VerifyFound();
        }

        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceResetAndBegin) {
    TEST_DESCRIPTION("Verify that we get an error if we reset & begin a performance query within the same primary command buffer.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_features = vku::InitStructHelper(&host_query_reset_features);
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &performance_features));

    auto queueFamilyProperties = m_device->phy().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = vku::InitStructHelper();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_EQ(VK_SUCCESS, err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_EQ(VK_SUCCESS, err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginQuery-None-02863");

        m_commandBuffer->reset();
        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
        m_commandBuffer->end();

        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = 0;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);
        m_errorMonitor->VerifyFound();

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, HostResetNotEnabled) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT without enabling the feature");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-None-02665");
    vk::ResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetFirstQuery) {
    TEST_DESCRIPTION("Bad firstQuery in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(host_query_reset_features);
    RETURN_IF_SKIP(InitState(nullptr, &host_query_reset_features));

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02666");
    vk::ResetQueryPoolEXT(m_device->device(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetBadRange) {
    TEST_DESCRIPTION("Bad range in vkResetQueryPoolEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(host_query_reset_features);
    RETURN_IF_SKIP(InitState(nullptr, &host_query_reset_features));

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02667");
    vk::ResetQueryPool(m_device->device(), query_pool.handle(), 0, 2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetQueryPool) {
    TEST_DESCRIPTION("Invalid queryPool in vkResetQueryPoolEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(host_query_reset_features);
    RETURN_IF_SKIP(InitState(nullptr, &host_query_reset_features));

    // Create and destroy a query pool.
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);

    // Attempt to reuse the query pool handle.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parameter");
    vk::ResetQueryPool(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetDevice) {
    TEST_DESCRIPTION("Device not matching queryPool in vkResetQueryPoolEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(host_query_reset_features);
    RETURN_IF_SKIP(InitState(nullptr, &host_query_reset_features));

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    // Create a second device with the feature enabled.
    vkt::QueueCreateInfoArray queue_info(m_device->phy().queue_properties_);
    auto features = m_device->phy().features();

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&host_query_reset_features);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice second_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(gpu(), &device_create_info, nullptr, &second_device));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parent");
    // Run vk::ResetQueryPoolExt on the wrong device.
    vk::ResetQueryPool(second_device, query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(NegativeQuery, CmdBufferQueryPoolDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a query pool dependency being destroyed.");
    RETURN_IF_SKIP(Init())

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    qpci.queryCount = 1;
    VkResult result = vk::CreateQueryPool(m_device->device(), &qpci, nullptr, &query_pool);
    ASSERT_EQ(VK_SUCCESS, result);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00070");
    // Destroy query pool dependency prior to submit to cause ERROR
    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, BeginQueryOnTimestampPool) {
    TEST_DESCRIPTION("Call CmdBeginQuery on a TIMESTAMP query pool.");

    RETURN_IF_SKIP(Init())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02804");
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    vk::BeginCommandBuffer(m_commandBuffer->handle(), &begin_info);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PoolCreate) {
    TEST_DESCRIPTION("Attempt to create a query pool for PIPELINE_STATISTICS without enabling pipeline stats for the device.");

    RETURN_IF_SKIP(Init())

    vkt::QueueCreateInfoArray queue_info(m_device->phy().queue_properties_);

    VkDevice local_device;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    auto features = m_device->phy().features();
    // Intentionally disable pipeline stats
    features.pipelineStatisticsQuery = VK_FALSE;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();
    VkResult err = vk::CreateDevice(gpu(), &device_create_info, nullptr, &local_device);
    ASSERT_EQ(VK_SUCCESS, err);

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    qpci.queryCount = 1;
    VkQueryPool query_pool;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryType-00791");
    vk::CreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryCount-02763");
    vk::CreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(local_device, nullptr);
}

TEST_F(NegativeQuery, Sizes) {
    TEST_DESCRIPTION("Invalid size of using queries commands.");

    RETURN_IF_SKIP(Init())

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMemoryRequirements mem_reqs = {};
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);
    const VkDeviceSize buffer_size = mem_reqs.size;

    const uint32_t query_pool_size = 4;
    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = query_pool_size;
    vkt::QueryPool occlusion_query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00796");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool.handle(), query_pool_size, 1);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool.handle(), 1, query_pool_size);
    m_errorMonitor->VerifyFound();

    // Actually reset all queries so they can be used
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, query_pool_size);

    vk::CmdBeginQuery(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, 0);

    // Query index to large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-query-00810");
    vk::CmdEndQuery(m_commandBuffer->handle(), occlusion_query_pool.handle(), query_pool_size);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0);

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00820");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool.handle(), query_pool_size, 1, buffer.handle(), 0, 0,
                                0);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool.handle(), 1, query_pool_size, buffer.handle(), 0, 0,
                                0);
    m_errorMonitor->VerifyFound();

    // Offset larger than buffer size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstOffset-00819");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, 1, buffer.handle(), buffer_size + 4, 0,
                                0);
    m_errorMonitor->VerifyFound();

    // Buffer does not have enough storage from offset to contain result of each query
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, 2, buffer.handle(), buffer_size - 4, 4,
                                0);
    m_errorMonitor->VerifyFound();

    // Query is not a timestamp type
    if (HasZeroTimestampValidBits()) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-timestampValidBits-00829");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-queryPool-01416");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, occlusion_query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00813");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    vk::GetQueryPoolResults(m_device->device(), occlusion_query_pool.handle(), query_pool_size, 1, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    vk::GetQueryPoolResults(m_device->device(), occlusion_query_pool.handle(), 1, query_pool_size, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PreciseBit) {
    TEST_DESCRIPTION("Check for correct Query Precise Bit circumstances.");
    RETURN_IF_SKIP(Init());

    // These tests require that the device support pipeline statistics query
    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    if (VK_TRUE != device_features.pipelineStatisticsQuery) {
        GTEST_SKIP() << "Test requires unsupported pipelineStatisticsQuery feature";
    }

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Test for precise bit when query type is not OCCLUSION
    if (features.occlusionQueryPrecise) {
        vkt::Event event(*m_device);

        VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
        query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_create_info.queryCount = 3;
        query_pool_create_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
        vkt::QueryPool query_pool(*m_device, query_pool_create_info);

        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, query_pool_create_info.queryCount);
        m_commandBuffer->end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

        m_commandBuffer->begin();
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_errorMonitor->VerifyFound();
        // vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_commandBuffer->end();

        const size_t out_data_size = 64;
        uint8_t data[out_data_size];
        // The dataSize is too small to return the data
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 3, 8, &data, 12, 0);
        m_errorMonitor->VerifyFound();
    }

    // Test for precise bit when precise feature is not available
    features.occlusionQueryPrecise = false;
    vkt::Device test_device(gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = vku::InitStructHelper();
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkCommandBuffer cmd_buffer2;
    err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer2);
    ASSERT_EQ(VK_SUCCESS, err);

    VkEvent event;
    VkEventCreateInfo event_create_info = vku::InitStructHelper();
    vk::CreateEvent(test_device.handle(), &event_create_info, nullptr, &event);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 2;
    vk::CreateQueryPool(test_device.handle(), &query_pool_create_info, nullptr, &query_pool);

    vk::BeginCommandBuffer(cmd_buffer2, &begin_info);
    vk::CmdResetQueryPool(cmd_buffer2, query_pool, 0, query_pool_create_info.queryCount);
    vk::EndCommandBuffer(cmd_buffer2);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer2;
    vk::QueueSubmit(test_device.graphics_queues().front()->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(test_device.graphics_queues().front()->handle());

    vk::BeginCommandBuffer(cmd_buffer, &begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");
    vk::CmdBeginQuery(cmd_buffer, query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
    m_errorMonitor->VerifyFound();
    vk::EndCommandBuffer(cmd_buffer);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    // The dataSize is too small to return the data
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-dataSize-00817");
    vk::GetQueryPoolResults(test_device.handle(), query_pool, 0, 2, 8, &data, out_data_size / 2, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(test_device.handle(), query_pool, nullptr);
    vk::DestroyEvent(test_device.handle(), event, nullptr);
    vk::DestroyCommandPool(test_device.handle(), command_pool, nullptr);
}

TEST_F(NegativeQuery, PoolPartialTimestamp) {
    TEST_DESCRIPTION("Request partial result on timestamp query.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    // Use setup as a positive test...
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-00827");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 0, 8, VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // Submit cmd buffer and wait for it.
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    // Attempt to obtain partial results.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-00818");
    uint32_t data_space[16];
    m_errorMonitor->SetUnexpectedError("Cannot get query results on queryPool");
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, sizeof(uint32_t),
                            VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    // Destroy query pool.
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(NegativeQuery, PerformanceQueryIntel) {
    TEST_DESCRIPTION("Call CmdCopyQueryPoolResults for an Intel performance query.");

    AddRequiredExtensions(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkInitializePerformanceApiInfoINTEL performance_api_info_intel = vku::InitStructHelper();
    vk::InitializePerformanceApiINTEL(m_device->device(), &performance_api_info_intel);

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-02734");
    m_commandBuffer->begin();
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 8, 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use query pool.");

    RETURN_IF_SKIP(Init())
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }
    InitRenderTarget();

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();
    // Use query pool to create binding with cmd buffer
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_commandBuffer->end();

    // Submit cmd buffer and then destroy query pool while in-flight
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);
    // Now that cmd buffer done we can safely destroy query_pool
    m_errorMonitor->SetUnexpectedError("If queryPool is not VK_NULL_HANDLE, queryPool must be a valid VkQueryPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove QueryPool obj");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(NegativeQuery, WriteTimeStamp) {
    TEST_DESCRIPTION("Test for invalid query slot in query pool.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    bool sync2 = IsExtensionsEnabled(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = vku::InitStructHelper();
    synchronization2.synchronization2 = VK_TRUE;
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper();
    if (sync2) {
        features2.pNext = &synchronization2;
    }
    InitState(nullptr, &features2);
    sync2 &= (synchronization2.synchronization2 == VK_TRUE);
    InitRenderTarget();
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-query-04904");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, CmdEndQueryIndexedEXTIndex) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    InitState();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_properties);
    if (transform_feedback_properties.maxTransformFeedbackStreams < 1) {
        GTEST_SKIP() << "maxTransformFeedbackStreams < 1";
    }

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    qpci.queryCount = 1;

    vkt::QueryPool tf_query_pool(*m_device, qpci);
    ASSERT_TRUE(tf_query_pool.initialized());

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    vkt::QueryPool query_pool(*m_device, qpci);
    ASSERT_TRUE(query_pool.initialized());

    m_commandBuffer->begin();
    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);

    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-None-02342");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-query-02343");
    vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CmdEndQueryIndexedEXTPrimitiveGenerated) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_properties);

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryCount = 1;

    qpci.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    vkt::QueryPool tf_query_pool(*m_device, qpci);

    qpci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    vkt::QueryPool pg_query_pool(*m_device, qpci);

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    vkt::QueryPool query_pool(*m_device, qpci);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-02339");
    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0, 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    if (!primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), pg_query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, TransformFeedbackStream) {
    TEST_DESCRIPTION(
        "Call CmdBeginQuery with query type transform feedback stream when transformFeedbackQueries is not supported.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);
    if (transform_feedback_props.transformFeedbackQueries) {
        GTEST_SKIP() << "Transform feedback queries are supported";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02328");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, GetResultsFlags) {
    TEST_DESCRIPTION("Test GetQueryPoolResults with invalid pData and stride");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, qpci);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    VkQueryResultFlags flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-04811");
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, data + 1, 4, flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, ResultStatusOnly) {
    TEST_DESCRIPTION("Request result status only query result.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);
    if (!query_pool.initialized()) {
        GTEST_SKIP() << "Required query not supported";
    }

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04810");
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, &data, sizeof(uint32_t), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, DestroyActiveQueryPool) {
    TEST_DESCRIPTION("Destroy query pool after GetQueryPoolResults() without VK_QUERY_RESULT_PARTIAL_BIT returns VK_SUCCESS");

    RETURN_IF_SKIP(Init())
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_create_info, nullptr, &query_pool);

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();
    cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_commandBuffer->begin(&cmd_begin);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    // Submit the command buffer again, making query pool in use and invalid to destroy
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
}

TEST_F(NegativeQuery, MultiviewBeginQuery) {
    TEST_DESCRIPTION("Test CmdBeginQuery in subpass with multiview");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan11Features features_1_1 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(features_1_1);
    if (!features_1_1.multiview) {
        GTEST_SKIP() << "Test requires VkPhysicalDeviceVulkan11Features::multiview feature.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;

    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmv_ci = vku::InitStructHelper();
    rpmv_ci.subpassCount = 1;
    rpmv_ci.pViewMasks = viewMasks;
    rpmv_ci.correlationMaskCount = 1;
    rpmv_ci.pCorrelationMasks = correlationMasks;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&rpmv_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attach;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkImageObj image(m_device);
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 4;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image.init(&image_ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo iv_ci = vku::InitStructHelper();
    iv_ci.image = image.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    iv_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.baseArrayLayer = 0;
    iv_ci.subresourceRange.layerCount = 4;
    vkt::ImageView image_view(*m_device, iv_ci);

    VkImageView image_view_handle = image_view.handle();

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper();
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &image_view_handle;
    fb_ci.width = 64;
    fb_ci.height = 64;
    fb_ci.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, fb_ci);

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;

    vkt::QueryPool query_pool(*m_device, qpci);

    VkRenderPassBeginInfo rp_begin = vku::InitStructHelper();
    rp_begin.renderPass = render_pass.handle();
    rp_begin.framebuffer = framebuffer.handle();
    rp_begin.renderArea.extent = {64, 64};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_begin);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-query-00808");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, PipelineStatisticsQuery) {
    TEST_DESCRIPTION("Test unsupported pipeline statistics queries");

    RETURN_IF_SKIP(Init())

    const std::optional<uint32_t> graphics_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT);
    const std::optional<uint32_t> compute_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!graphics_queue_family_index && !compute_queue_family_index) {
        GTEST_SKIP() << "required queue families not found";
    }

    if (graphics_queue_family_index) {
        vkt::CommandPool command_pool(*m_device, graphics_queue_family_index.value());

        vkt::CommandBuffer command_buffer(m_device, &command_pool);
        command_buffer.begin();

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_ci.queryCount = 1;
        query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

        vkt::QueryPool query_pool(*m_device, query_pool_ci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00805");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.end();
    }

    if (compute_queue_family_index) {
        vkt::CommandPool command_pool(*m_device, compute_queue_family_index.value());

        vkt::CommandBuffer command_buffer(m_device, &command_pool);
        command_buffer.begin();

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_ci.queryCount = 1;
        query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;

        vkt::QueryPool query_pool(*m_device, query_pool_ci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00804");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.end();
    }
}

TEST_F(NegativeQuery, TestGetQueryPoolResultsDataAndStride) {
    TEST_DESCRIPTION("Test pData and stride multiple in GetQueryPoolResults");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-02828");
    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, &data, 3, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGenerated) {
    TEST_DESCRIPTION("Test unsupported primitives generated queries");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_properties);

    const std::optional<uint32_t> compute_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!compute_queue_family_index) {
        GTEST_SKIP() << "required queue family not found, skipping test";
    }
    vkt::CommandPool command_pool(*m_device, compute_queue_family_index.value());

    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_ci.queryCount = 1;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-06687");
    vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06689");
    vk::CmdBeginQueryIndexedEXT(command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;

    vkt::QueryPool occlusion_query_pool(*m_device, query_pool_ci);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06692");
    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, 0, 1);
    m_errorMonitor->VerifyFound();

    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGeneratedFeature) {
    TEST_DESCRIPTION("Test missing primitives generated query feature");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_ci.queryCount = 1;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-06688");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06693");
    vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGeneratedDiscardEnabled) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithRasterizerDiscard feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    primitives_generated_features.primitivesGeneratedQueryWithRasterizerDiscard = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    VkPipelineRasterizationStateCreateInfo rs_ci = vku::InitStructHelper();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_ = rs_ci;
    // Rasterization discard enable prohibits fragment shader.
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGeneratedStreams) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithNonZeroStreams feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceTransformFeedbackPropertiesEXT xfb_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(xfb_props);
    if (!xfb_props.transformFeedbackRasterizationStreamSelect) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedbackRasterizationStreamSelect is VK_FALSE";
    }

    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features = vku::InitStructHelper();
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_features =
        vku::InitStructHelper(&transform_feedback_features);
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported.";
    }
    if (transform_feedback_features.geometryStreams == VK_FALSE) {
        GTEST_SKIP() << "geometryStreams feature not supported, skipping tests.";
    }
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "geometryStreams feature not supported, skipping tests.";
    }
    primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    VkPipelineRasterizationStateStreamCreateInfoEXT rasterization_streams = vku::InitStructHelper();
    rasterization_streams.rasterizationStream = 1;

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.pNext = &rasterization_streams;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CommandBufferMissingOcclusion) {
    TEST_DESCRIPTION(
        "Test executing secondary command buffer without VkCommandBufferInheritanceInfo::occlusionQueryEnable enabled while "
        "occlusion query is active.");
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
    cbii.occlusionQueryEnable = VK_FALSE;  // Invalid

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-00102");
    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CommandBufferInheritanceFlags) {
    TEST_DESCRIPTION("Test executing secondary command buffer with bad VkCommandBufferInheritanceInfo::queryFlags.");
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
    cbii.queryFlags = 0;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-00103");
    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, MultiviewEndQuery) {
    TEST_DESCRIPTION("Test CmdEndQuery in subpass with multiview");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    const bool indexed_queries = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_att;
    subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[1].colorAttachmentCount = 1;
    subpasses[1].pColorAttachments = &color_att;

    uint32_t viewMasks[2] = {0x1u, 0x3u};
    uint32_t correlationMasks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmv_ci = vku::InitStructHelper();
    rpmv_ci.subpassCount = 2;
    rpmv_ci.pViewMasks = viewMasks;
    rpmv_ci.correlationMaskCount = 1;
    rpmv_ci.pCorrelationMasks = correlationMasks;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = 0;
    dependency.dstSubpass = 1;
    dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&rpmv_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attach;
    rp_ci.subpassCount = 2;
    rp_ci.pSubpasses = subpasses;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &dependency;

    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkImageObj image(m_device);
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 4;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image.init(&image_ci);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo iv_ci = vku::InitStructHelper();
    iv_ci.image = image.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    iv_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.baseArrayLayer = 0;
    iv_ci.subresourceRange.layerCount = 4;
    vkt::ImageView image_view(*m_device, iv_ci);

    VkImageView image_view_handle = image_view.handle();

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper();
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &image_view_handle;
    fb_ci.width = 64;
    fb_ci.height = 64;
    fb_ci.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, fb_ci);

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;

    vkt::QueryPool query_pool(*m_device, qpci);

    VkRenderPassBeginInfo rp_begin = vku::InitStructHelper();
    rp_begin.renderPass = render_pass.handle();
    rp_begin.framebuffer = framebuffer.handle();
    rp_begin.renderArea.extent = {64, 64};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_begin);

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 1, 0);

    m_commandBuffer->NextSubpass();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-query-00812");
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 1);
    m_commandBuffer->end();

    if (indexed_queries) {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(rp_begin);
        vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 1, 0, 0);
        m_commandBuffer->NextSubpass();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-query-02345");
        vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        vk::CmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
        m_commandBuffer->end();
    }
}

TEST_F(NegativeQuery, NullQueryPoolCreateInfo) {
    TEST_DESCRIPTION("Invalid usage without meshShaderQueries enabled");
    RETURN_IF_SKIP(Init())
    VkQueryPool pool = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateQueryPool-pCreateInfo-parameter");
    vk::CreateQueryPool(m_device->handle(), nullptr, nullptr, &pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, MeshShaderQueries) {
    TEST_DESCRIPTION("Invalid usage without meshShaderQueries enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkQueryPool pool = VK_NULL_HANDLE;

    VkQueryPoolCreateInfo query_pool_info = vku::InitStructHelper();
    query_pool_info.queryType = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = 1;
    query_pool_info.pipelineStatistics = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-meshShaderQueries-07068");
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &pool);
    m_errorMonitor->VerifyFound();

    query_pool_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    query_pool_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-meshShaderQueries-07069");
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &pool);
    m_errorMonitor->VerifyFound();

    query_pool_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-meshShaderQueries-07069");
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, WriteTimestampWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdWriteTimestamp(2) with queryPool being invalid.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2Features synchronization2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(synchronization2);
    if (synchronization2.synchronization2 == VK_FALSE) {
        GTEST_SKIP() << "synchronization2 feature is not available";
    }
    RETURN_IF_SKIP(InitState(nullptr, &synchronization2));

    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-queryPool-parameter");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, bad_query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp2-queryPool-parameter");
    vk::CmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_NONE_KHR, bad_query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, DestroyWithoutQueryPool) {
    TEST_DESCRIPTION("call vkDestryQueryPool with queryPool being invalid.");
    RETURN_IF_SKIP(Init())
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-parameter");
    vk::DestroyQueryPool(device(), bad_query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, GetQueryPoolResultsWithoutQueryPool) {
    TEST_DESCRIPTION("call vkGetQueryPoolResults with queryPool being invalid.");
    RETURN_IF_SKIP(Init())
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryPool-parameter");
    vk::GetQueryPoolResults(device(), bad_query_pool, 0, 1, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CmdEndQueryWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdEndQuery with queryPool being invalid.");
    RETURN_IF_SKIP(Init())

    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-queryPool-parameter");
    vk::CmdEndQuery(m_commandBuffer->handle(), bad_query_pool, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, CmdCopyQueryPoolResultsWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdCopyQueryPoolResults with queryPool being invalid.");
    RETURN_IF_SKIP(Init())

    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryPool-parameter");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), bad_query_pool, 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeQuery, CmdResetQueryPoolWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdResetQueryPool with queryPool being invalid.");
    RETURN_IF_SKIP(Init())
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-queryPool-parameter");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), bad_query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, ResetQueryPoolWithoutQueryPool) {
    TEST_DESCRIPTION("call vkResetQueryPool with queryPool being invalid.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parameter");
    vk::ResetQueryPool(device(), bad_query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, ActiveEndQuery) {
    TEST_DESCRIPTION("Check all queries for vkCmdEndQuery are active");
    RETURN_IF_SKIP(Init())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-None-01923");
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, ActiveCmdResetQueryPool) {
    TEST_DESCRIPTION("Check all queries for vkCmdResetQueryPool are not active");
    RETURN_IF_SKIP(Init())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-None-02841");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, ActiveCmdCopyQueryPoolResults) {
    TEST_DESCRIPTION("Check all queries for vkCmdCopyQueryPoolResults are not active");
    RETURN_IF_SKIP(Init())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-None-07429");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 0,
                                VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, CmdExecuteCommandsActiveQueries) {
    TEST_DESCRIPTION("Check query types when calling vkCmdExecuteCommands");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported.";
    }
    if (features2.features.inheritedQueries == VK_FALSE) {
        GTEST_SKIP() << "inheritedQueries feature is not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandBufferInheritanceInfo cmd_buf_hinfo = vku::InitStructHelper();
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

    vk::BeginCommandBuffer(secondary.handle(), &cmd_buf_info);
    vk::EndCommandBuffer(secondary.handle());

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-07594");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &secondary.handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, CmdExecuteBeginActiveQuery) {
    TEST_DESCRIPTION("Begin a query in secondary command buffer that is already active in primary command buffer");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())
    if (m_device->phy().features().inheritedQueries == VK_FALSE) {
        GTEST_SKIP() << "inheritedQueries feature is not supported";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 2;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;
    cbii.occlusionQueryEnable = VK_TRUE;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin(&cbbi);
    vk::CmdBeginQuery(secondary.handle(), query_pool.handle(), 1u, 0u);
    vk::CmdEndQuery(secondary.handle(), query_pool.handle(), 1u);
    secondary.end();

    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0u, 0u);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00105");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &secondary.handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0u);
    m_commandBuffer->end();
}

TEST_F(NegativeQuery, PerformanceQueryReset) {
    TEST_DESCRIPTION("Invalid performance query pool reset");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    auto performance_query_features = vku::InitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(performance_query_features);

    if (!performance_query_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Test requires (unsupported) performanceCounterQueryPools";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    uint32_t counterCount = 0u;
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->phy(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, nullptr, nullptr);
    std::vector<VkPerformanceCounterKHR> counters(counterCount, vku::InitStruct<VkPerformanceCounterKHR>());
    std::vector<VkPerformanceCounterDescriptionKHR> counterDescriptions(counterCount,
                                                                        vku::InitStruct<VkPerformanceCounterDescriptionKHR>());
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->phy(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, counters.data(), counterDescriptions.data());

    uint32_t enabledCounter = counterCount;
    for (uint32_t i = 0; i < counterCount; ++i) {
        if (counters[i].scope == VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR) {
            enabledCounter = i;
            break;
        }
    }
    if (enabledCounter == counterCount) {
        GTEST_SKIP() << "No counter with scope VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR found";
    }

    auto query_pool_performance_ci = vku::InitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    query_pool_performance_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    query_pool_performance_ci.counterIndexCount = 1u;
    query_pool_performance_ci.pCounterIndices = &enabledCounter;

    uint32_t num_passes = 0u;
    vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(m_device->phy(), &query_pool_performance_ci, &num_passes);

    auto query_pool_ci = vku::InitStruct<VkQueryPoolCreateInfo>(&query_pool_performance_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1u;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    auto acquire_profiling_lock_info = vku::InitStruct<VkAcquireProfilingLockInfoKHR>();
    acquire_profiling_lock_info.timeout = std::numeric_limits<uint64_t>::max();
    vk::AcquireProfilingLockKHR(*m_device, &acquire_profiling_lock_info);

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

    vkt::CommandBuffer command_buffer(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    command_buffer.begin();
    vk::CmdBeginQuery(command_buffer.handle(), query_pool, 0u, 0u);
    vk::CmdEndQuery(command_buffer.handle(), query_pool, 0u);
    command_buffer.end();

    for (uint32_t i = 0; i < 2; ++i) {
        m_commandBuffer->begin();
        if (i == 0) {
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0u, 0u);
            vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0u);
        } else {
            vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &command_buffer.handle());
        }
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0u, 1u);
        m_commandBuffer->end();

        auto performance_query_submit_info = vku::InitStruct<VkPerformanceQuerySubmitInfoKHR>();
        performance_query_submit_info.counterPassIndex = 0u;

        auto submit_info = vku::InitStruct<VkSubmitInfo>(&performance_query_submit_info);
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-02862");
        vk::QueueSubmit(m_default_queue, 1u, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
        vk::DeviceWaitIdle(*m_device);
    }

    vk::ReleaseProfilingLockKHR(*m_device);
}

TEST_F(NegativeQuery, GetQueryPoolResultsWithoutReset) {
    TEST_DESCRIPTION("Get query pool results without ever resetting the query");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    uint32_t data = 0u;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-QueryPool-NotReset");
    vk::GetQueryPoolResults(*m_device, query_pool.handle(), 0u, 1u, sizeof(uint32_t), &data, sizeof(uint32_t), 0u);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0u, 1u, buffer.handle(), 0u, sizeof(uint32_t), 0u);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-QueryPool-NotReset");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeQuery, InvalidMeshQueryAtDraw) {
    TEST_DESCRIPTION("Draw with vertex shader with mesh query active");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShaderQueries) {
        GTEST_SKIP() << "Mesh shader queries are not supported.";
    }
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0u, 0u);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-stage-07073");
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0u);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
