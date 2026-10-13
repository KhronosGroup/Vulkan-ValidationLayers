/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

// The non-uniform descriptor index check uses subgroup vote, and needs two invocations in a subgroup to see two indices
template <typename Test>
bool CanCheckNonUniformIndex(Test &test, VkShaderStageFlagBits stage) {
    VkPhysicalDeviceSubgroupProperties subgroup_props = vku::InitStructHelper();
    test.GetPhysicalDeviceProperties2(subgroup_props);
    return (subgroup_props.supportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT) != 0 &&
           (subgroup_props.supportedStages & stage) != 0 && subgroup_props.subgroupSize >= 2;
}
