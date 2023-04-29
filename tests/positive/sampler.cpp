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

class PositiveSampler : public VkPositiveLayerTest {};

TEST_F(PositiveSampler, SamplerMirrorClampToEdgeWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_sampler_mirror_clamp_to_edge in 1.1 before samplerMirrorClampToEdge feature was added");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires Vulkan 1.1 exactly";
    }

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    vk_testing::Sampler sampler(*m_device, sampler_info);
}

TEST_F(PositiveSampler, SamplerMirrorClampToEdgeWithoutFeature12) {
    TEST_DESCRIPTION("Use VK_KHR_sampler_mirror_clamp_to_edge in 1.2 using the extension");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    vk_testing::Sampler sampler(*m_device, sampler_info);
}

TEST_F(PositiveSampler, SamplerMirrorClampToEdgeWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_sampler_mirror_clamp_to_edge in 1.2 with feature bit enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    features12.samplerMirrorClampToEdge = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features12.samplerMirrorClampToEdge != VK_TRUE) {
        printf("samplerMirrorClampToEdge not supported, skipping test\n");
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    vk_testing::Sampler sampler(*m_device, sampler_info);
}
