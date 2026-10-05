/*
 * Copyright (c) 2023-2026 The Khronos Group Inc.
 * Copyright (c) 2023-2026 Valve Corporation
 * Copyright (c) 2023-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once
// This checks any requirements needed for GPU-AV are met otherwise devices not meeting them will "fail" the tests
template <typename Test>
bool CanEnableGpuAV(Test &test) {
    // Check version first before trying to call GetPhysicalDeviceFeatures2
    if (test.DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("At least Vulkan version 1.1 is required for GPU-AV\n");
        return false;
    }
    VkPhysicalDeviceBufferDeviceAddressFeatures supported_bda_feature = vku::InitStructHelper();
    VkPhysicalDeviceTimelineSemaphoreFeatures supported_timeline_feature = vku::InitStructHelper(&supported_bda_feature);
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&supported_timeline_feature);
    test.GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.fragmentStoresAndAtomics || !features2.features.vertexPipelineStoresAndAtomics) {
        printf("fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics are required for GPU-AV\n");
        return false;
    } else if (!supported_timeline_feature.timelineSemaphore) {
        printf("timelineSemaphore are required for GPU-AV\n");
        return false;
    } else if (!supported_bda_feature.bufferDeviceAddress) {
        printf("bufferDeviceAddress are required for GPU-AV\n");
        return false;
    } else if (test.IsPlatformMockICD()) {
        printf("Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw\n");
        return false;
    }
    return true;
}

// GPU-AV reads the errors of a vkQueueSubmit when its last batch retires.
// This submits |command_buffer| in a first batch that signals |timeline| to 1
// then an empty last batch that waits for the host to signal 2
// Once 1 is reached the command buffer has completed,
// but its errors are only read after 2 is signaled and the last batch retires
inline void SubmitWithLastBatchWaitingOnHost(const vkt::Queue& queue, VkCommandBuffer command_buffer,
                                             const vkt::Semaphore& timeline) {
    const uint64_t signal_value = 1;
    const uint64_t wait_value = 2;
    VkTimelineSemaphoreSubmitInfo signal_timeline_info = vku::InitStructHelper();
    signal_timeline_info.signalSemaphoreValueCount = 1;
    signal_timeline_info.pSignalSemaphoreValues = &signal_value;
    VkTimelineSemaphoreSubmitInfo wait_timeline_info = vku::InitStructHelper();
    wait_timeline_info.waitSemaphoreValueCount = 1;
    wait_timeline_info.pWaitSemaphoreValues = &wait_value;

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submits[2];
    submits[0] = vku::InitStructHelper(&signal_timeline_info);
    submits[0].commandBufferCount = 1;
    submits[0].pCommandBuffers = &command_buffer;
    submits[0].signalSemaphoreCount = 1;
    submits[0].pSignalSemaphores = &timeline.handle();
    submits[1] = vku::InitStructHelper(&wait_timeline_info);
    submits[1].waitSemaphoreCount = 1;
    submits[1].pWaitSemaphores = &timeline.handle();
    submits[1].pWaitDstStageMask = &wait_stage;
    vk::QueueSubmit(queue, 2, submits, VK_NULL_HANDLE);
}
