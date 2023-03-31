/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"

TEST_F(VkLayerTest, PipelineRobustnessDisabled) {
    TEST_DESCRIPTION("Create a pipeline using VK_EXT_pipeline_robustness but with pipelineRobustness == VK_FALSE");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    CreateComputePipelineHelper pipe(*this);

    VkPipelineRobustnessCreateInfoEXT pipeline_robustness_info;

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06926");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06927");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.vertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06928");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06929");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineRobustnessRobustBufferAccess2Unsupported) {
    TEST_DESCRIPTION("Create a pipeline using VK_EXT_pipeline_robustness with robustBufferAccess2 being unsupported");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsExtensionsEnabled(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)) {
        auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
        GetPhysicalDeviceFeatures2(robustness2_features);

        if (robustness2_features.robustBufferAccess2) {
            GTEST_SKIP() << "robustBufferAccess2 is supported";
        }
    }

    auto pipeline_robustness_features = LvlInitStruct<VkPhysicalDevicePipelineRobustnessFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_robustness_features);

    if (!pipeline_robustness_features.pipelineRobustness) {
        GTEST_SKIP() << "pipelineRobustness is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    CreateComputePipelineHelper pipe(*this);

    VkPipelineRobustnessCreateInfoEXT pipeline_robustness_info;

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-robustBufferAccess2-06931");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-robustBufferAccess2-06932");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.vertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-robustBufferAccess2-06933");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineRobustnessRobustImageAccess2Unsupported) {
    TEST_DESCRIPTION("Create a pipeline using VK_EXT_pipeline_robustness with robustImageAccess2 being unsupported");

    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsExtensionsEnabled(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)) {
        auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
        GetPhysicalDeviceFeatures2(robustness2_features);

        if (robustness2_features.robustImageAccess2) {
            GTEST_SKIP() << "robustImageAccess2 is supported";
        }
    }

    auto pipeline_robustness_features = LvlInitStruct<VkPhysicalDevicePipelineRobustnessFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_robustness_features);

    if (!pipeline_robustness_features.pipelineRobustness) {
        GTEST_SKIP() << "pipelineRobustness is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    CreateComputePipelineHelper pipe(*this);

    VkPipelineRobustnessCreateInfoEXT pipeline_robustness_info;

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-robustImageAccess2-06934");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineRobustnessRobustImageAccessUnsupported) {
    TEST_DESCRIPTION("Create a pipeline using VK_EXT_pipeline_robustness with robustImageAccess being unsupported");

    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME);

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsExtensionsEnabled(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME)) {
        auto robust_image_features = LvlInitStruct<VkPhysicalDeviceImageRobustnessFeatures>();
        GetPhysicalDeviceFeatures2(robust_image_features);

        if (robust_image_features.robustImageAccess) {
            GTEST_SKIP() << "robustImageAccess is supported";
        }
    }

    auto pipeline_robustness_features = LvlInitStruct<VkPhysicalDevicePipelineRobustnessFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_robustness_features);

    if (!pipeline_robustness_features.pipelineRobustness) {
        GTEST_SKIP() << "pipelineRobustness is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    CreateComputePipelineHelper pipe(*this);

    VkPipelineRobustnessCreateInfoEXT pipeline_robustness_info;

    pipe.InitInfo();
    pipe.InitState();
    pipeline_robustness_info = LvlInitStruct<VkPipelineRobustnessCreateInfoEXT>();
    pipeline_robustness_info.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT;
    pipe.cp_ci_.pNext = &pipeline_robustness_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRobustnessCreateInfoEXT-robustImageAccess-06930");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}