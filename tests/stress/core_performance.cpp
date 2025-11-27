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

// Tests to detect significant performance regressions.
// The heuristic is that each individual test should not get "stuck" for too long.
// If the test runs for tens of seconds or minutes it can be a regression.
//
// The tests should be designed to take enough time so that performance regression causes
// a noticeable slowdown. For example, if a test takes 0.2 seconds, a 10x regression would
// make it run in about 2 seconds, which may still seem reasonable. A rule of thumb is for
// each test to take around a second or max few seconds on development machine.
//
// When appicable, the iteration can be exposed to tweak the test's running time.
class PerformanceCore : public VkLayerTest {};

TEST_F(PerformanceCore, SignalSemaphoreFromQueueSubmitManyTimes) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5171
    // In case of a regression this test may have quadratic complexity in the iteration count
    TEST_DESCRIPTION("Signal semaphore from vkQueueSubmit many time without waiting");
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    const int N = 15'000;
    for (int i = 1; i <= N; i++) {
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, i));
    }
    m_device->Wait();
}

TEST_F(PerformanceCore, SignalSemaphoreFromHostManyTimes) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9264
    // In case of a regression this test may have quadratic complexity in the iteration count
    TEST_DESCRIPTION("Signal semaphore from vkSignalSemaphore many time without waiting");
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    const int N = 100'000;
    for (int i = 1; i <= N; i++) {
        semaphore.Signal(i);
    }
    m_device->Wait();
}
