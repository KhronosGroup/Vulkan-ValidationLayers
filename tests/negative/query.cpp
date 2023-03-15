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

#include "cast_utils.h"
#include "enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"

class VkLayerQueryTest : public VkLayerTest {};

TEST_F(VkLayerQueryTest, QueryPerformanceCreation) {
    TEST_DESCRIPTION("Create performance query without support");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features));
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;

    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;
        break;
    }

    if (counters.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counters.size();
    std::vector<uint32_t> counterIndices;
    for (uint32_t c = 0; c < counters.size(); c++) counterIndices.push_back(c);
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    // Missing pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryType-03222");
    VkQueryPool query_pool;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    query_pool_ci.pNext = &perf_query_pool_ci;

    // Invalid counter indices
    counterIndices.push_back(counters.size());
    perf_query_pool_ci.counterIndexCount++;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolPerformanceCreateInfoKHR-pCounterIndices-03321");
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
    perf_query_pool_ci.counterIndexCount--;
    counterIndices.pop_back();

    // Success
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();

    // Missing acquire lock
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03223");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, QueryPerformanceCounterCommandbufferScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the command buffer counter scope");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_BUFFER_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
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

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Not the first command.
    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03224");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();

        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);

    vkReleaseProfilingLockKHR(device());
}

TEST_F(VkLayerQueryTest, QueryPerformanceCounterRenderPassScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the render pass counter scope");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_RENDER_PASS_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
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

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Inside a render pass.
    {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03225");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, QueryPerformanceReleaseProfileLockBeforeSubmit) {
    TEST_DESCRIPTION("Verify that we get an error if we release the profiling lock during the recording of performance queries");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
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

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);

        // Release while recording.
        vkReleaseProfilingLockKHR(device());
        {
            VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
            VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
            ASSERT_TRUE(result == VK_SUCCESS);
        }

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, QueryPerformanceIncompletePasses) {
    TEST_DESCRIPTION("Verify that we get an error if we don't submit a command buffer for each passes before getting the results.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto host_query_reset_features = LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>(&host_query_reset_features);
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }
    if (!host_query_reset_features.hostQueryReset) {
        GTEST_SKIP() << "Missing host query reset.";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR doens't match up with profile queues";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    ASSERT_TRUE(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;
    uint32_t nPasses = 0;

    // Find all counters with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) counterIndices.push_back(counterIdx);
        }
        if (counterIndices.empty()) continue;  // might not be a scope command

        VkQueryPoolPerformanceCreateInfoKHR create_info = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
        create_info.queueFamilyIndex = idx;
        create_info.counterIndexCount = counterIndices.size();
        create_info.pCounterIndices = &counterIndices[0];

        vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(gpu(), &create_info, &nPasses);

        if (nPasses < 2) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);
    PFN_vkResetQueryPoolEXT fpvkResetQueryPoolEXT =
        (PFN_vkResetQueryPoolEXT)vk::GetInstanceProcAddr(instance(), "vkResetQueryPoolEXT");

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);

        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
        m_commandBuffer->end();

        // Invalid pass index
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = nPasses;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
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
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = passIdx;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
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
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyFound();

        // submit the last pass
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = nPasses - 1;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
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
            vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    &results[0], sizeof(VkPerformanceCounterResultKHR) * (results.size() - 1), 0);
            m_errorMonitor->VerifyFound();
        }

        // Invalid stride
        {
            std::vector<VkPerformanceCounterResultKHR> results_invalid_stride;
            results_invalid_stride.resize(counterIndices.size() * 2);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03229");
            vk::GetQueryPoolResults(
                device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size(),
                &results_invalid_stride[0], sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size() + 4,
                VK_QUERY_RESULT_WAIT_BIT);
            m_errorMonitor->VerifyFound();
        }

        // Invalid flags
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_PARTIAL_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();

        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, QueryPerformanceResetAndBegin) {
    TEST_DESCRIPTION("Verify that we get an error if we reset & begin a performance query within the same primary command buffer.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto host_query_reset_features = LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>(&host_query_reset_features);
    GetPhysicalDeviceFeatures2(performance_features);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }
    if (!host_query_reset_features.hostQueryReset) {
        GTEST_SKIP() << "Missing host query reset.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    ASSERT_TRUE(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
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

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginQuery-None-02863");

        m_commandBuffer->reset();
        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
        m_commandBuffer->end();

        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = 0;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
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

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, HostQueryResetNotEnabled) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT without enabling the feature");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-None-02665");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, HostQueryResetBadFirstQuery) {
    TEST_DESCRIPTION("Bad firstQuery in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02666");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 1, 0);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpvkResetQueryPool = (PFN_vkResetQueryPool)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPool");
        if (nullptr == fpvkResetQueryPool) {
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkResetQueryPool");
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02666");
            fpvkResetQueryPool(m_device->device(), query_pool, 1, 0);
            m_errorMonitor->VerifyFound();
        }
    }

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, HostQueryResetBadRange) {
    TEST_DESCRIPTION("Bad range in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02667");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 2);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, HostQueryResetInvalidQueryPool) {
    TEST_DESCRIPTION("Invalid queryPool in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    // Create and destroy a query pool.
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);

    // Attempt to reuse the query pool handle.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parameter");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, HostQueryResetWrongDevice) {
    TEST_DESCRIPTION("Device not matching queryPool in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    // Create a second device with the feature enabled.
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    auto features = m_device->phy().features();

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&host_query_reset_features);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice second_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &device_create_info, nullptr, &second_device));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parent");
    // Run vk::ResetQueryPoolExt on the wrong device.
    fpvkResetQueryPoolEXT(second_device, query_pool, 0, 1);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(VkLayerQueryTest, InvalidCmdBufferQueryPoolDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a query pool dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    qpci.queryCount = 1;
    VkResult result = vk::CreateQueryPool(m_device->device(), &qpci, nullptr, &query_pool);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkQueryPool");
    // Destroy query pool dependency prior to submit to cause ERROR
    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, BeginQueryOnTimestampPool) {
    TEST_DESCRIPTION("Call CmdBeginQuery on a TIMESTAMP query pool.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02804");
    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();

    vk::BeginCommandBuffer(m_commandBuffer->handle(), &begin_info);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, InvalidQueryPoolCreate) {
    TEST_DESCRIPTION("Attempt to create a query pool for PIPELINE_STATISTICS without enabling pipeline stats for the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);

    VkDevice local_device;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
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
    ASSERT_VK_SUCCESS(err);

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
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

TEST_F(VkLayerQueryTest, InvalidQuerySizes) {
    TEST_DESCRIPTION("Invalid size of using queries commands.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (IsPlatform(kPixel2XL)) {
        GTEST_SKIP() << "This test should not run on Pixel 2 XL";
    }

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    const uint32_t timestampValidBits = queue_props[m_device->graphics_queue_node_index_].timestampValidBits;

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkMemoryRequirements mem_reqs = {};
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);
    const VkDeviceSize buffer_size = mem_reqs.size;

    const uint32_t query_pool_size = 4;
    VkQueryPool occlusion_query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = query_pool_size;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &occlusion_query_pool);

    m_commandBuffer->begin();

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00796");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, query_pool_size, 1);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, 1, query_pool_size);
    m_errorMonitor->VerifyFound();

    // Actually reset all queries so they can be used
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, 0, query_pool_size);

    vk::CmdBeginQuery(m_commandBuffer->handle(), occlusion_query_pool, 0, 0);

    // Query index to large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-query-00810");
    vk::CmdEndQuery(m_commandBuffer->handle(), occlusion_query_pool, query_pool_size);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_commandBuffer->handle(), occlusion_query_pool, 0);

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00820");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, query_pool_size, 1, buffer.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, 1, query_pool_size, buffer.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Offset larger than buffer size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstOffset-00819");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, 0, 1, buffer.handle(), buffer_size + 4, 0, 0);
    m_errorMonitor->VerifyFound();

    // Buffer does not have enough storage from offset to contain result of each query
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, 0, 2, buffer.handle(), buffer_size - 4, 4, 0);
    m_errorMonitor->VerifyFound();

    // Query is not a timestamp type
    if (timestampValidBits == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-timestampValidBits-00829");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-queryPool-01416");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, occlusion_query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00813");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    vk::GetQueryPoolResults(m_device->device(), occlusion_query_pool, query_pool_size, 1, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    vk::GetQueryPoolResults(m_device->device(), occlusion_query_pool, 1, query_pool_size, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), occlusion_query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, QueryPreciseBit) {
    TEST_DESCRIPTION("Check for correct Query Precise Bit circumstances.");
    ASSERT_NO_FATAL_FAILURE(Init());

    // These tests require that the device support pipeline statistics query
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (VK_TRUE != device_features.pipelineStatisticsQuery) {
        GTEST_SKIP() << "Test requires unsupported pipelineStatisticsQuery feature";
    }

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Test for precise bit when query type is not OCCLUSION
    if (features.occlusionQueryPrecise) {
        VkEvent event;
        VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
        vk::CreateEvent(m_device->handle(), &event_create_info, nullptr, &event);

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

        VkQueryPool query_pool;
        VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_create_info.queryCount = 3;
        query_pool_create_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
        vk::CreateQueryPool(m_device->handle(), &query_pool_create_info, nullptr, &query_pool);

        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_errorMonitor->VerifyFound();
        // vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_commandBuffer->end();

        const size_t out_data_size = 64;
        uint8_t data[out_data_size];
        // The dataSize is too small to return the data
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 3, 8, &data, 12, 0);
        m_errorMonitor->VerifyFound();

        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
        vk::DestroyEvent(m_device->handle(), event, nullptr);
    }

    // Test for precise bit when precise feature is not available
    features.occlusionQueryPrecise = false;
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(test_device.handle(), &event_create_info, nullptr, &event);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    vk::BeginCommandBuffer(cmd_buffer, &begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 2;
    vk::CreateQueryPool(test_device.handle(), &query_pool_create_info, nullptr, &query_pool);

    vk::CmdResetQueryPool(cmd_buffer, query_pool, 0, 2);
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

TEST_F(VkLayerQueryTest, QueryPoolPartialTimestamp) {
    TEST_DESCRIPTION("Request partial result on timestamp query.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
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
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

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

TEST_F(VkLayerQueryTest, PerformanceQueryIntel) {
    TEST_DESCRIPTION("Call CmdCopyQueryPoolResults for an Intel performance query.");

    AddRequiredExtensions(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto performance_api_info_intel = LvlInitStruct<VkInitializePerformanceApiInfoINTEL>();
    PFN_vkInitializePerformanceApiINTEL vkInitializePerformanceApiINTEL =
        (PFN_vkInitializePerformanceApiINTEL)vk::GetDeviceProcAddr(m_device->device(), "vkInitializePerformanceApiINTEL");
    vkInitializePerformanceApiINTEL(m_device->device(), &performance_api_info_intel);

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-02734");
    m_commandBuffer->begin();
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 0, 8, 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    // Destroy query pool.
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, QueryPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use query pool.");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();
    // Use query pool to create binding with cmd buffer
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_commandBuffer->end();

    // Submit cmd buffer and then destroy query pool while in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    // Now that cmd buffer done we can safely destroy query_pool
    m_errorMonitor->SetUnexpectedError("If queryPool is not VK_NULL_HANDLE, queryPool must be a valid VkQueryPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove QueryPool obj");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, WriteTimeStampInvalidQuery) {
    TEST_DESCRIPTION("Test for invalid query slot in query pool.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    bool sync2 = IsExtensionsEnabled(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    synchronization2.synchronization2 = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>();
    if (sync2) {
        features2.pNext = &synchronization2;
    }
    InitState(nullptr, &features2);
    sync2 &= (synchronization2.synchronization2 == VK_TRUE);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    query_pool.init(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-query-04904");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();
    if (sync2) {
        auto vkCmdWriteTimestamp2KHR =
            (PFN_vkCmdWriteTimestamp2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdWriteTimestamp2KHR");

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp2-query-04903");
        vkCmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 1);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(VkLayerQueryTest, InvalidCmdEndQueryIndexedEXTIndex) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    InitState();

    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(transform_feedback_properties);
    if (transform_feedback_properties.maxTransformFeedbackStreams < 1) {
        GTEST_SKIP() << "maxTransformFeedbackStreams < 1";
    }

    auto fpCmdBeginQueryIndexedEXT =
        (PFN_vkCmdBeginQueryIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT");
    ASSERT_NE(fpCmdBeginQueryIndexedEXT, nullptr);
    auto fpCmdEndQueryIndexedEXT =
        (PFN_vkCmdEndQueryIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndQueryIndexedEXT");
    ASSERT_NE(fpCmdEndQueryIndexedEXT, nullptr);

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    qpci.queryCount = 1;

    vk_testing::QueryPool tf_query_pool(*m_device, qpci);
    ASSERT_TRUE(tf_query_pool.initialized());

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    vk_testing::QueryPool query_pool(*m_device, qpci);
    ASSERT_TRUE(query_pool.initialized());

    m_commandBuffer->begin();
    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-02346");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                            transform_feedback_properties.maxTransformFeedbackStreams);

    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-02347");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-None-02342");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-query-02343");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, InvalidCmdEndQueryIndexedEXTPrimitiveGenerated) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_properties);
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    auto vkCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    auto vkCmdEndQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdEndQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndQueryIndexedEXT"));

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryCount = 1;

    vk_testing::QueryPool tf_query_pool;
    qpci.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    tf_query_pool.init(*m_device, qpci);

    vk_testing::QueryPool pg_query_pool;
    qpci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    pg_query_pool.init(*m_device, qpci);

    vk_testing::QueryPool query_pool;
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool.init(*m_device, qpci);

    m_commandBuffer->begin();

    vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    vkCmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                            transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    if (!primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), pg_query_pool.handle(), 0, 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    vkCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, BeginQueryTypeTransformFeedbackStream) {
    TEST_DESCRIPTION(
        "Call CmdBeginQuery with query type transform feedback stream when transformFeedbackQueries is not supported.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    VkPhysicalDeviceProperties2 phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_props);

    GetPhysicalDeviceProperties2(phys_dev_props_2);
    if (transform_feedback_props.transformFeedbackQueries) {
        GTEST_SKIP() << "Transform feedback queries are supported";
    }

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02328");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, GetQueryPoolResultsFlags) {
    TEST_DESCRIPTION("Test GetQueryPoolResults with invalid pData and stride");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;
    query_pool.init(*m_device, qpci);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    VkQueryResultFlags flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-04811");
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, data + 1, 4, flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, QueryPoolResultStatusOnly) {
    TEST_DESCRIPTION("Request result status only query result.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    query_pool_ci.queryCount = 1;
    VkResult err = vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Required query not supported";
    }

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04810");
    vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, out_data_size, &data, sizeof(uint32_t), 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerQueryTest, DestroyActiveQueryPool) {
    TEST_DESCRIPTION("Destroy query pool after GetQueryPoolResults() without VK_QUERY_RESULT_PARTIAL_BIT returns VK_SUCCESS");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_create_info, nullptr, &query_pool);

    auto cmd_begin = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_commandBuffer->begin(&cmd_begin);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    // Submit the command buffer again, making query pool in use and invalid to destroy
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
}

TEST_F(VkLayerQueryTest, BeginQueryWithMultiview) {
    TEST_DESCRIPTION("Test CmdBeginQuery in subpass with multiview");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features_1_1 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = GetPhysicalDeviceFeatures2(features_1_1);
    if (!features_1_1.multiview) {
        GTEST_SKIP() << "Test requires VkPhysicalDeviceVulkan11Features::multiview feature.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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
    auto rpmv_ci = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    rpmv_ci.subpassCount = 1;
    rpmv_ci.pViewMasks = viewMasks;
    rpmv_ci.correlationMaskCount = 1;
    rpmv_ci.pCorrelationMasks = correlationMasks;

    auto rp_ci = LvlInitStruct<VkRenderPassCreateInfo>(&rpmv_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attach;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, rp_ci);

    VkImageObj image(m_device);
    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
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

    vk_testing::ImageView image_view;
    VkImageViewCreateInfo iv_ci = LvlInitStruct<VkImageViewCreateInfo>();
    iv_ci.image = image.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    iv_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.baseArrayLayer = 0;
    iv_ci.subresourceRange.layerCount = 4;
    image_view.init(*m_device, iv_ci);

    VkImageView image_view_handle = image_view.handle();

    auto fb_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &image_view_handle;
    fb_ci.width = 64;
    fb_ci.height = 64;
    fb_ci.layers = 1;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, fb_ci);

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, qpci);

    auto rp_begin = LvlInitStruct<VkRenderPassBeginInfo>();
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

TEST_F(VkLayerQueryTest, PipelineStatisticsQuery) {
    TEST_DESCRIPTION("Test unsupported pipeline statistics queries");

    ASSERT_NO_FATAL_FAILURE(Init());

    const std::optional<uint32_t> graphics_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT);
    const std::optional<uint32_t> compute_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!graphics_queue_family_index && !compute_queue_family_index) {
        GTEST_SKIP() << "required queue families not found";
    }

    if (graphics_queue_family_index) {
        VkCommandPoolObj command_pool(m_device, graphics_queue_family_index.value());

        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_ci.queryCount = 1;
        query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

        vk_testing::QueryPool query_pool;
        query_pool.init(*m_device, query_pool_ci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00805");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.end();
    }

    if (compute_queue_family_index) {
        VkCommandPoolObj command_pool(m_device, compute_queue_family_index.value());

        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_ci.queryCount = 1;
        query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;

        vk_testing::QueryPool query_pool;
        query_pool.init(*m_device, query_pool_ci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00804");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.end();
    }
}

TEST_F(VkLayerQueryTest, TestGetQueryPoolResultsDataAndStride) {
    TEST_DESCRIPTION("Test pData and stride multiple in GetQueryPoolResults");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    query_pool.init(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-02828");
    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, &data, 3, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, PrimitivesGeneratedQuery) {
    TEST_DESCRIPTION("Test unsupported primitives generated queries");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_properties);
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    const std::optional<uint32_t> compute_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!compute_queue_family_index) {
        GTEST_SKIP() << "required queue family not found, skipping test";
    }
    VkCommandPoolObj command_pool(m_device, compute_queue_family_index.value());

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();

    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_ci.queryCount = 1;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-06687");
    vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    auto fpvkCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    auto fpvkCmdEndQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdEndQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndQueryIndexedEXT"));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06689");
    fpvkCmdBeginQueryIndexedEXT(command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;

    vk_testing::QueryPool occlusion_query_pool;
    occlusion_query_pool.init(*m_device, query_pool_ci);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06692");
    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, 0, 1);
    m_errorMonitor->VerifyFound();

    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    fpvkCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, PrimitivesGeneratedQueryFeature) {
    TEST_DESCRIPTION("Test missing primitives generated query feature");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_ci.queryCount = 1;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-06688");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    auto fpvkCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06693");
    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerQueryTest, PrimitivesGeneratedQueryAndDiscardEnabled) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithRasterizerDiscard feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    primitives_generated_features.primitivesGeneratedQueryWithRasterizerDiscard = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto rs_ci = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_ = rs_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    auto query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);

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

TEST_F(VkLayerQueryTest, PrimitivesGeneratedQueryStreams) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithNonZeroStreams feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto xfb_props = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(xfb_props);
    if (!xfb_props.transformFeedbackRasterizationStreamSelect) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedbackRasterizationStreamSelect is VK_FALSE";
    }

    auto transform_feedback_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    auto primitives_generated_features =
        LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>(&transform_feedback_features);
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
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto rasterization_streams = LvlInitStruct<VkPipelineRasterizationStateStreamCreateInfoEXT>();
    rasterization_streams.rasterizationStream = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_.pNext = &rasterization_streams;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    auto query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);

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

TEST_F(VkLayerQueryTest, CommandBufferMissingOcclusionQueryEnabled) {
    TEST_DESCRIPTION(
        "Test executing secondary command buffer without VkCommandBufferInheritanceInfo::occlusionQueryEnable enabled while "
        "occlusion query is active.");
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    VkPhysicalDeviceFeatures features{};
    GetPhysicalDeviceFeatures(&features);
    if (features.inheritedQueries == VK_FALSE) {
        GTEST_SKIP() << "inheritedQueries not supported";
    }

    features = {};
    features.inheritedQueries = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &qpci, nullptr, &query_pool);

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;
    cbii.occlusionQueryEnable = VK_FALSE;  // Invalid

    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBuffer-00102");
    m_commandBuffer->begin();
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(device(), query_pool, nullptr);
}
