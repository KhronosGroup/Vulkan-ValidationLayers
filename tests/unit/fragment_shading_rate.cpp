/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"

TEST_F(NegativeFragmentShadingRate, Values) {
    TEST_DESCRIPTION("Specify invalid fragment shading rate values");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    fsr_features.pipelineFragmentShadingRate = true;

    VkPhysicalDeviceFeatures2 device_features = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fsr_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &device_features));

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    fragmentSize.width = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;

    fragmentSize.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;

    fragmentSize.width = 8;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 8;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;
    m_commandBuffer->end();
}

TEST_F(NegativeFragmentShadingRate, ValuesNoFeatures) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeFragmentShadingRate, CombinerOpsNoFeatures) {
    TEST_DESCRIPTION("Specify combiner operations when only pipeline rate is supported");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(fsr_features);

    fsr_features.pipelineFragmentShadingRate = VK_TRUE;
    fsr_features.primitiveFragmentShadingRate = VK_FALSE;
    fsr_features.attachmentFragmentShadingRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();

    combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    m_commandBuffer->end();
}

TEST_F(NegativeFragmentShadingRate, CombinerOpsNoPipelineRate) {
    TEST_DESCRIPTION("Specify pipeline rate when only attachment or primitive rate are supported");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (fsr_features.attachmentFragmentShadingRate == VK_FALSE && fsr_features.primitiveFragmentShadingRate == VK_FALSE) {
        GTEST_SKIP() << "requires attachmentFragmentShadingRate or primitiveFragmentShadingRate";
    }

    fsr_features.pipelineFragmentShadingRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    fragmentSize.width = 2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508");
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;
}

TEST_F(NegativeFragmentShadingRate, CombinerOpsLimit) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.fragmentShadingRateNonTrivialCombinerOps) {
        GTEST_SKIP() << "requires fragmentShadingRateNonTrivialCombinerOps to be unsupported.";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (!fsr_features.primitiveFragmentShadingRate && !fsr_features.attachmentFragmentShadingRate) {
        GTEST_SKIP() << "requires primitiveFragmentShadingRate or attachmentFragmentShadingRate to be supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_commandBuffer->begin();
    if (fsr_features.primitiveFragmentShadingRate) {
        combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512");
        vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
        m_errorMonitor->VerifyFound();
        combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }

    if (fsr_features.attachmentFragmentShadingRate) {
        combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512");
        vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
        m_errorMonitor->VerifyFound();
        combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }
    m_commandBuffer->end();
}

TEST_F(NegativeFragmentShadingRate, PrimitiveFragmentShadingRateWriteMultiViewportLimitDynamic) {
    TEST_DESCRIPTION("Test dynamic validation of the primitiveFragmentShadingRateWithMultipleViewports limit");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        GTEST_SKIP() << "Test requires primitiveFragmentShadingRateWithMultipleViewports to be unsupported.";
    }

    auto eds_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(&eds_features);
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (!fsr_features.primitiveFragmentShadingRate) {
        GTEST_SKIP() << "Test requires primitiveFragmentShadingRate to be supported.";
    }

    if (!features2.features.multiViewport) {
        GTEST_SKIP() << "%s requires multiViewport to be supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_fragment_shading_rate : enable
        void main() {
            gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
        }
    )glsl";

    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    std::vector<VkRect2D> scissors = {{{0, 0}, {16, 16}}, {{1, 1}, {16, 16}}};
    pipe.SetScissor(scissors);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    const VkPipelineLayoutObj pl(m_device);
    {
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        pipe.AddShader(&vs);
        VkResult err = pipe.CreateVKPipeline(pl.handle(), renderPass());
        ASSERT_VK_SUCCESS(err);
    }
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    VkViewport viewports[] = {{0, 0, 16, 16, 0, 1}, {1, 1, 16, 16, 0, 1}};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2, viewports);

    // error produced here.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapReferences) {
    TEST_DESCRIPTION("Create a subpass with the wrong attachment information for a fragment density map ");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) &&
        !IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Extensions not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_LOAD,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_PREINITIALIZED,
                                      VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    // Set 1 instead of 0
    VkAttachmentReference ref = {1, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    auto rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-fragmentDensityMapAttachment-06471",
                         nullptr);

    // Set wrong VkImageLayout
    ref = {0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);
    rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02549", nullptr);

    // Set wrong load operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_CLEAR,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_PREINITIALIZED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);
    rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02550", nullptr);

    // Set wrong store operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_LOAD,
              VK_ATTACHMENT_STORE_OP_STORE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_PREINITIALIZED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);
    rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02551", nullptr);
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapLayerCount) {
    TEST_DESCRIPTION("Specify a fragment density map attachment with incorrect layerCount");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(&multiview_features);
    auto features2 = GetPhysicalDeviceFeatures2(fdm_features);

    if (fdm_features.fragmentDensityMap != VK_TRUE) {
        GTEST_SKIP() << "requires fragmentDensityMap feature";
    } else if (multiview_features.multiview != VK_TRUE) {
        GTEST_SKIP() << "requires multiview feature";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    auto rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);

    // Create a renderPass with viewMask 0
    auto subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.viewMask = 0;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>(&rpfdmi);
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci, true /*khr*/);

    VkImageObj image(m_device);
    image.InitNoLayout(image.ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                               VK_IMAGE_TILING_OPTIMAL, 0));
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 2,
                                             VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    auto fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    VkFramebuffer fb;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-02747");
    vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();

    // Set viewMask to non-zero - requires multiview
    subpass.viewMask = 0x10;
    vk_testing::RenderPass rp_mv(*m_device, rpci, true /*khr*/);

    fb_info.renderPass = rp_mv.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-02746");
    vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Fragment Density Maps are used.");

    // VK_EXT_fragment_density_map2 requires VK_EXT_fragment_density_map
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool fdm2Supported = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT density_map_features =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT density_map2_features =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>(&density_map_features);
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(density_map2_features);

    if (density_map_features.fragmentDensityMapDynamic == VK_FALSE) {
        GTEST_SKIP() << "fragmentDensityMapDynamic not supported";
    }

    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&density_map2_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto density_map2_properties = LvlInitStruct<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>();
    auto properties2 = GetPhysicalDeviceProperties2(density_map2_properties);

    // Test sampler parameters

    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.maxLod = 0.0;
    sampler_info_ref.flags |= VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;

    // min max filters must match
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02574");
    sampler_info.minFilter = sampler_info_ref.minFilter;
    sampler_info.magFilter = sampler_info_ref.magFilter;

    // mipmapMode must be SAMPLER_MIPMAP_MODE_NEAREST
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02575");
    sampler_info.mipmapMode = sampler_info_ref.mipmapMode;

    // minLod and maxLod must be 0.0
    sampler_info.minLod = 1.0;
    sampler_info.maxLod = 1.0;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02576");
    sampler_info.minLod = sampler_info_ref.minLod;
    sampler_info.maxLod = sampler_info_ref.maxLod;

    // addressMode must be CLAMP_TO_EDGE or CLAMP_TO_BORDER
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02577");
    sampler_info.addressModeU = sampler_info_ref.addressModeU;

    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02577");
    sampler_info.addressModeV = sampler_info_ref.addressModeV;

    // some features cannot be enabled for subsampled samplers
    if (features2.features.samplerAnisotropy == VK_TRUE) {
        sampler_info.anisotropyEnable = VK_TRUE;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02578");
        sampler_info.anisotropyEnable = sampler_info_ref.anisotropyEnable;
        sampler_info.anisotropyEnable = VK_FALSE;
    }

    sampler_info.compareEnable = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02579");
    sampler_info.compareEnable = sampler_info_ref.compareEnable;

    sampler_info.unnormalizedCoordinates = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02580");
    sampler_info.unnormalizedCoordinates = sampler_info_ref.unnormalizedCoordinates;

    // Test image parameters

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.flags = 0;

    // only VK_IMAGE_TYPE_2D is supported
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.extent.height = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02557");

    // only VK_SAMPLE_COUNT_1_BIT is supported
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-samples-02558");

    // tiling must be VK_IMAGE_TILING_OPTIMAL
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02565");

    // only 2D
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02566");

    // no cube maps
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 64;
    image_create_info.arrayLayers = 6;
    image_create_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02567");

    // mipLevels must be 1
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 2;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02568");

    // Test image view parameters

    // create a valid density map image
    image_create_info.flags = 0;
    image_create_info.mipLevels = 1;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    VkImageObj densityImage(m_device);
    densityImage.init(&image_create_info);
    ASSERT_TRUE(densityImage.initialized());

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = densityImage.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // density maps can't be sparse (or protected)
    if (features2.features.sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
        VkImageObj image(m_device);
        image.init_no_mem(*m_device, image_create_info);
        ASSERT_TRUE(image.initialized());

        ivci.image = image.handle();
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-04116");
    }

    if (fdm2Supported) {
        if (!density_map2_features.fragmentDensityMapDeferred) {
            ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT;
            ivci.image = densityImage.handle();
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-03567");
        } else {
            ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT;
            ivci.flags |= VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
            ivci.image = densityImage.handle();
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-03568");
        }
        if (density_map2_properties.maxSubsampledArrayLayers < properties2.properties.limits.maxImageArrayLayers) {
            image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.arrayLayers = density_map2_properties.maxSubsampledArrayLayers + 1;
            VkImageObj image(m_device);
            image.init(&image_create_info);
            ASSERT_TRUE(image.initialized());
            ivci.image = image.handle();
            ivci.flags = 0;
            ivci.subresourceRange.layerCount = density_map2_properties.maxSubsampledArrayLayers + 1;
            m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-imageViewType-04973");
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-03569");
        }
    }
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapDisabled) {
    TEST_DESCRIPTION("Checks for when the fragment density map features are not enabled.");

    // VK_EXT_fragment_density_map2 requires VK_EXT_fragment_density_map
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkImageObj image2D(m_device);
    image2D.init(&image_create_info);
    ASSERT_TRUE(image2D.initialized());

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Flags must not be set if the feature is not enabled
    ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-02572");
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapReferenceAttachment) {
    TEST_DESCRIPTION(
        "Test creating a framebuffer with fragment density map reference to an attachment with layer count different from 1");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires Vulkan version 1.0";
    }

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fdm_features);
    fdm_features.fragmentDensityMap = true;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, 0));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkAttachmentReference ref;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    rpfdmi.fragmentDensityMapAttachment = ref;

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_R8G8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &ref;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vk_testing::RenderPass render_pass(*m_device, rpci);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView imageView =
        image.targetView(VK_FORMAT_R8G8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 4, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = render_pass.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-02747");
    vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, DeviceFeatureCombinations) {
    TEST_DESCRIPTION(
        "Specify invalid combinations of fragment shading rate, shading rate image, and fragment density map features");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    bool sri_extension = IsExtensionsEnabled(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    bool fdm_extension = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);

    if (!fdm_extension && !sri_extension) {
        GTEST_SKIP() << "requires VK_NV_shading_rate_image or VK_EXT_fragment_density_map";
    }

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_query_features =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceShadingRateImageFeaturesNV sri_query_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_query_features =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();

    if (fdm_extension) {
        fsr_query_features.pNext = &fdm_query_features;
    }
    if (sri_extension) {
        if (fdm_extension) {
            fdm_query_features.pNext = &sri_query_features;
        } else {
            fsr_query_features.pNext = &sri_query_features;
        }
    }
    GetPhysicalDeviceFeatures2(fsr_query_features);

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceShadingRateImageFeaturesNV sri_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    device_ci.pNext = &features2;

    VkDevice testDevice;

    if (sri_query_features.shadingRateImage) {
        sri_features.shadingRateImage = true;
        fsr_features.pNext = &sri_features;
        if (fsr_query_features.pipelineFragmentShadingRate) {
            fsr_features.pipelineFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-shadingRateImage-04478");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.pipelineFragmentShadingRate = false;
        }
        if (fsr_query_features.primitiveFragmentShadingRate) {
            fsr_features.primitiveFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-shadingRateImage-04479");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.primitiveFragmentShadingRate = false;
        }
        if (fsr_query_features.attachmentFragmentShadingRate) {
            fsr_features.attachmentFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-shadingRateImage-04480");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.attachmentFragmentShadingRate = false;
        }
        fsr_features.pNext = nullptr;
    }

    if (fdm_query_features.fragmentDensityMap) {
        fdm_features.fragmentDensityMap = true;
        fsr_features.pNext = &fdm_features;
        if (fsr_query_features.pipelineFragmentShadingRate) {
            fsr_features.pipelineFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-fragmentDensityMap-04481");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.pipelineFragmentShadingRate = false;
        }
        if (fsr_query_features.primitiveFragmentShadingRate) {
            fsr_features.primitiveFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-fragmentDensityMap-04482");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.primitiveFragmentShadingRate = false;
        }
        if (fsr_query_features.attachmentFragmentShadingRate) {
            fsr_features.attachmentFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-fragmentDensityMap-04483");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.attachmentFragmentShadingRate = false;
        }
        fsr_features.pNext = nullptr;
    }
}

TEST_F(NegativeFragmentShadingRate, FramebufferUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);
    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto attach = LvlInitStruct<VkAttachmentReference2KHR>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    auto fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    auto subpass = LvlInitStruct<VkSubpassDescription2KHR>(&fsr_attachment);

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2KHR>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8_UINT);

    auto fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04548");
    vk_testing::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FramebufferDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "This test is crashing on some AMD + Windows platforms without any validation errors getting hit; requires "
                        "investigation.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>();
    if (IsExtensionsEnabled(VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        fsr_features.pNext = &multiview_features;
    }
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate not supported.";
    }

    if (fsr_properties.layeredShadingRateAttachments != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRatePropertiesKHR::layeredShadingRateAttachments not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    auto fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    auto subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    VkImageCreateInfo ici = VkImageObj::ImageCreateInfo2D(
        1, 1, 1, 2, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, VK_IMAGE_TILING_OPTIMAL, 0);
    image.InitNoLayout(ici);
    auto image_view_ci = image.BasicViewCreatInfo();
    image_view_ci.subresourceRange.layerCount = 2;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    const auto imageView = image.targetView(image_view_ci);

    auto fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04539");
    vk_testing::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04540");
    fb.init(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    VkImageView imageViewLayered = image.targetView(VK_FORMAT_R8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2);
    fb_info.pAttachments = &imageViewLayered;
    fb_info.layers = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04538");
    fb.init(*m_device, fb_info);
    m_errorMonitor->VerifyFound();

    if (multiview_features.multiview) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04538");
        fb.init(*m_device, fb_info);
        m_errorMonitor->VerifyFound();

        subpass.viewMask = 0x4;
        vk_testing::RenderPass rp2(*m_device, rpci, true);
        ASSERT_TRUE(rp2.initialized());
        subpass.viewMask = 0;

        fb_info.renderPass = rp2.handle();
        fb_info.layers = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04537");
        fb.init(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeFragmentShadingRate, Attachments) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "requires attachmentFragmentShadingRate feature";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    auto attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    auto fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    auto subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass rp;

    rpci.flags = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-flags-04521");
    vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    rpci.flags = 0;
    attach_desc.format =
        FindFormatWithoutFeatures(gpu(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    if (attach_desc.format != VK_FORMAT_UNDEFINED) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-pAttachments-04586");
        vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
    }
    attach_desc.format = VK_FORMAT_R8_UINT;

    attach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04524");
    vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width + 1;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04525");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04528");
    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width ==
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.width) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04527");
    }
    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height ==
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04530");
    }
    vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width / 2;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height / 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04526");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04529");
    vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width * 2;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04527");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04530");
    vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width /
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.height >
        fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
        fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04531");
        vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
        fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    }

    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height /
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.width >
        fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
        fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04532");
        vk::CreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
        fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    }
}

TEST_F(NegativeFragmentShadingRate, IncompatibleFragmentRateShadingAttachmentInExecuteCommands) {
    TEST_DESCRIPTION(
        "Test incompatible fragment shading rate attachments "
        "calling CmdExecuteCommands");

    // Enable KHR_fragment_shading_rate
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    // Create a render pass without a Fragment Shading Rate attachment
    auto col_attach = LvlInitStruct<VkAttachmentDescription2>();
    col_attach.format = VK_FORMAT_R8G8B8A8_UNORM;
    col_attach.samples = VK_SAMPLE_COUNT_1_BIT;
    col_attach.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    col_attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass_no_fsr = LvlInitStruct<VkSubpassDescription2>();

    VkRenderPassCreateInfo2 rcpi_no_fsr = LvlInitStruct<VkRenderPassCreateInfo2KHR>();
    rcpi_no_fsr.attachmentCount = 1;
    rcpi_no_fsr.pAttachments = &col_attach;
    rcpi_no_fsr.subpassCount = 1;
    rcpi_no_fsr.pSubpasses = &subpass_no_fsr;

    vk_testing::RenderPass rp_no_fsr(*m_device, rcpi_no_fsr, true);

    // Create 2 render passes with fragment shading rate attachments with
    // differing shadingRateAttachmentTexelSize values
    VkAttachmentReference2 fsr_attach_1 = LvlInitStruct<VkAttachmentReference2>();
    fsr_attach_1.layout = VK_IMAGE_LAYOUT_GENERAL;
    fsr_attach_1.attachment = 0;

    VkExtent2D texel_size_1 = {8, 8};

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment_1 = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment_1.shadingRateAttachmentTexelSize = texel_size_1;
    fsr_attachment_1.pFragmentShadingRateAttachment = &fsr_attach_1;
    VkSubpassDescription2 fsr_subpass_1 = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment_1);

    VkAttachmentReference2 fsr_attach_2 = LvlInitStruct<VkAttachmentReference2>();
    fsr_attach_2.layout = VK_IMAGE_LAYOUT_GENERAL;
    fsr_attach_2.attachment = 0;

    VkExtent2D texel_size_2 = {32, 32};

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment_2 = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment_2.shadingRateAttachmentTexelSize = texel_size_2;
    fsr_attachment_2.pFragmentShadingRateAttachment = &fsr_attach_2;

    VkSubpassDescription2 fsr_subpass_2 = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment_2);

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2KHR rpci_fsr_1 = LvlInitStruct<VkRenderPassCreateInfo2KHR>();
    rpci_fsr_1.subpassCount = 1;
    rpci_fsr_1.pSubpasses = &fsr_subpass_1;
    rpci_fsr_1.attachmentCount = 1;
    rpci_fsr_1.pAttachments = &attach_desc;

    vk_testing::RenderPass rp_fsr_1(*m_device, rpci_fsr_1, true);
    ASSERT_TRUE(rp_fsr_1.initialized());

    VkRenderPassCreateInfo2KHR rpci_fsr_2 = LvlInitStruct<VkRenderPassCreateInfo2KHR>();
    rpci_fsr_2.subpassCount = 1;
    rpci_fsr_2.pSubpasses = &fsr_subpass_2;
    rpci_fsr_2.attachmentCount = 1;
    rpci_fsr_2.pAttachments = &attach_desc;

    vk_testing::RenderPass rp_fsr_2(*m_device, rpci_fsr_2, true);
    ASSERT_TRUE(rp_fsr_2.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    // Create a frame buffer with a render pass with FSR attachment
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp_fsr_1.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    vk_testing::Framebuffer framebuffer_fsr;
    framebuffer_fsr.init(*m_device, fb_info);

    // Create a frame buffer with a render pass without FSR attachment
    VkFramebufferCreateInfo fb_info_0 = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info_0.renderPass = rp_no_fsr.handle();
    fb_info_0.attachmentCount = 1;
    fb_info_0.pAttachments = &imageView;
    fb_info_0.width = 32;
    fb_info_0.height = 32;
    fb_info_0.layers = 1;

    vk_testing::Framebuffer framebuffer_no_fsr;
    framebuffer_no_fsr.init(*m_device, fb_info_0);

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Inheritance info without FSR attachment
    const VkCommandBufferInheritanceInfo cmdbuff_ii_no_fsr = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        rp_no_fsr.handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi_no_fsr = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                   nullptr,  // pNext
                                                   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii_no_fsr};
    cmdbuff__bi_no_fsr.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    // Render pass begin info for no FSR attachment
    const VkRenderPassBeginInfo rp_bi_no_fsr = LvlInitStruct<VkRenderPassBeginInfo>(
        nullptr, rp_no_fsr.handle(), framebuffer_no_fsr.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    // Inheritance info with FSR attachment
    const VkCommandBufferInheritanceInfo cmdbuff_ii_fsr = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        rp_fsr_2.handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi_fsr = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                nullptr,  // pNext
                                                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii_fsr};
    cmdbuff__bi_fsr.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    // Render pass begin info with FSR attachment
    const VkRenderPassBeginInfo rp_bi_fsr = LvlInitStruct<VkRenderPassBeginInfo>(
        nullptr, rp_fsr_1.handle(), framebuffer_fsr.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    // Test case where primary command buffer does not have an FSR attachment but
    // secondary command buffer does.
    {
        secondary.begin(&cmdbuff__bi_fsr);
        secondary.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(rp_bi_no_fsr, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();
    secondary.reset();

    // Test case where primary command buffer has FSR attachment but secondary
    // command buffer does not.
    {
        secondary.begin(&cmdbuff__bi_no_fsr);
        secondary.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(rp_bi_fsr, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();
    secondary.reset();

    // Test case where both command buffers have FSR attachments but they are
    // incompatible.
    {
        secondary.begin(&cmdbuff__bi_fsr);
        secondary.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(rp_bi_fsr, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();
    secondary.reset();
}

TEST_F(NegativeFragmentShadingRate, ShadingRateUsage) {
    TEST_DESCRIPTION("Specify invalid usage of the fragment shading rate image view usage.");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    GetPhysicalDeviceFeatures2(fsr_features);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "requires attachmentFragmentShadingRate feature";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &fsr_features));

    const VkFormat format =
        FindFormatWithoutFeatures(gpu(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    if (format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "No format found without shading rate attachment support";
    }

    VkImageFormatProperties imageFormatProperties;
    if (vk::GetPhysicalDeviceImageFormatProperties(gpu(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                   VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0,
                                                   &imageFormatProperties) == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "Format not supported";
    }

    VkImageObj image(m_device);
    // Initialize image with transfer source usage
    image.Init(128, 128, 1, format, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo createinfo = LvlInitStruct<VkImageViewCreateInfo>();
    createinfo.image = image.handle();
    createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createinfo.format = format;
    createinfo.subresourceRange.layerCount = 1;
    createinfo.subresourceRange.baseMipLevel = 0;
    createinfo.subresourceRange.levelCount = 1;
    if (FormatIsColor(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    } else if (FormatHasDepth(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (FormatHasStencil(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    // Create a view with the fragment shading rate attachment usage, but that doesn't support it
    CreateImageViewTest(*this, &createinfo, "VUID-VkImageViewCreateInfo-usage-04550");

    auto fsrProperties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsrProperties);

    if (!fsrProperties.layeredShadingRateAttachments) {
        if (IsPlatform(kMockICD)) {
            GTEST_SKIP() << "Test not supported by MockICD, doesn't correctly advertise format support for fragment shading "
                            "rate attachments";
        } else {
            VkImageObj image2(m_device);
            image2.Init(VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8_UINT,
                                                      VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                                      VK_IMAGE_TILING_OPTIMAL));
            ASSERT_TRUE(image2.initialized());

            createinfo.image = image2.handle();
            createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            createinfo.format = VK_FORMAT_R8_UINT;
            createinfo.subresourceRange.layerCount = 2;
            createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            CreateImageViewTest(*this, &createinfo, "VUID-VkImageViewCreateInfo-usage-04551");
        }
    }
}

TEST_F(NegativeFragmentShadingRate, Pipeline) {
    TEST_DESCRIPTION("Specify invalid fragment shading rate values");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    fsr_features.pipelineFragmentShadingRate = true;

    VkPhysicalDeviceFeatures2 device_features = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fsr_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    fsr_ci.fragmentSize.width = 0;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04494");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 0;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04495");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.fragmentSize.width = 3;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04496");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 3;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04497");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.fragmentSize.width = 8;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04498");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 8;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04499");
    fsr_ci.fragmentSize.height = 1;
}

TEST_F(NegativeFragmentShadingRate, PipelineFeatureUsage) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    fsr_ci.fragmentSize.width = 2;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 2;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04501");
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04502");
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
}

TEST_F(NegativeFragmentShadingRate, PipelineCombinerOpsLimit) {
    TEST_DESCRIPTION("Specify invalid use of combiner ops when non trivial ops aren't supported");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.fragmentShadingRateNonTrivialCombinerOps) {
        GTEST_SKIP() << "requires fragmentShadingRateNonTrivialCombinerOps to be unsupported";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (!fsr_features.primitiveFragmentShadingRate && !fsr_features.attachmentFragmentShadingRate) {
        GTEST_SKIP() << "requires primitiveFragmentShadingRate or attachmentFragmentShadingRate to be supported";
    }

    fsr_features.pipelineFragmentShadingRate = VK_TRUE;
    fsr_features.primitiveFragmentShadingRate = VK_TRUE;
    fsr_features.attachmentFragmentShadingRate = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    if (fsr_features.primitiveFragmentShadingRate) {
        fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
        fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }

    if (fsr_features.attachmentFragmentShadingRate) {
        fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
        fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }
}

TEST_F(NegativeFragmentShadingRate, PrimitiveWriteMultiViewportLimit) {
    TEST_DESCRIPTION("Test static validation of the primitiveFragmentShadingRateWithMultipleViewports limit");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    const bool vil_extension = IsExtensionsEnabled(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    const bool va2_extension = IsExtensionsEnabled(VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        GTEST_SKIP() << "requires primitiveFragmentShadingRateWithMultipleViewports to be unsupported.";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(fsr_features);

    if (!fsr_features.primitiveFragmentShadingRate) {
        GTEST_SKIP() << "requires primitiveFragmentShadingRate to be supported.";
    }

    if (!features2.features.multiViewport) {
        GTEST_SKIP() << "requires multiViewport to be supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &fsr_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test PrimitiveShadingRate writes with multiple viewports
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        VkViewport viewports[2] = {{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}};
        VkRect2D scissors[2] = {};

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.vp_state_ci_.viewportCount = 2;
            info.vp_state_ci_.pViewports = viewports;
            info.vp_state_ci_.scissorCount = 2;
            info.vp_state_ci_.pScissors = scissors;
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503");
    }

    // Test PrimitiveShadingRate writes with ViewportIndex writes in a geometry shader
    if (features2.features.geometryShader) {
        char const *vsSource = R"glsl(
            #version 450
            void main() {}
        )glsl";

        static char const *gsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            layout (points) in;
            layout (points) out;
            layout (max_vertices = 1) out;
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
                gl_ViewportIndex = 0;
                gl_PointSize = 1.0f;
                EmitVertex();
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504");
    }

    // Test PrimitiveShadingRate writes with ViewportIndex writes in a vertex shader
    if (vil_extension) {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            #extension GL_ARB_shader_viewport_layer_array : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                gl_ViewportIndex = 0;
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504");
    }

    if (va2_extension) {
        // Test PrimitiveShadingRate writes with ViewportIndex writes in a geometry shader
        if (features2.features.geometryShader) {
            char const *vsSource = R"glsl(
                #version 450
                void main() {}
            )glsl";

            static char const *gsSource = R"glsl(
                #version 450
                #extension GL_EXT_fragment_shading_rate : enable
                #extension GL_NV_viewport_array2 : enable
                layout (points) in;
                layout (points) out;
                layout (max_vertices = 1) out;
                void main() {
                   gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                   gl_ViewportMask[0] = 0;
                   gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
                   gl_PointSize = 1.0f;
                   EmitVertex();
                }
            )glsl";

            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
            VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            };

            CreatePipelineHelper::OneshotTest(
                *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505");
        }

        // Test PrimitiveShadingRate writes with ViewportIndex writes in a vertex shader
        if (vil_extension) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_fragment_shading_rate : enable
                #extension GL_NV_viewport_array2 : enable
                void main() {
                    gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                    gl_ViewportMask[0] = 0;
                }
            )glsl";

            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            };

            CreatePipelineHelper::OneshotTest(
                *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505");
        }
    }
}

TEST_F(NegativeFragmentShadingRate, Ops) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(fsr_features);
    if (!fsr_features.primitiveFragmentShadingRate) {
        GTEST_SKIP() << "primitiveFragmentShadingRate not available";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    // Pass an invalid value for op 0
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_ENUM_KHR;
    // if fragmentShadingRateNonTrivialCombinerOps is not supported
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06567");
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    // Pass an invalid value for op 1
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_ENUM_KHR;
    // if fragmentShadingRateNonTrivialCombinerOps is not supported
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06568");
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapAttachmentCount) {
    TEST_DESCRIPTION("Test attachmentCount of VkRenderPassFragmentDensityMapCreateInfoEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(fdm_features);
    if (fdm_features.fragmentDensityMap != VK_TRUE) {
        GTEST_SKIP() << "requires fragmentDensityMap feature";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference ref = {};
    ref.attachment = 1;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    auto rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    rpfdmi.fragmentDensityMapAttachment = ref;

    // Create a renderPass with viewMask 0
    auto subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.viewMask = 0;

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo2>(&rpfdmi);
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-fragmentDensityMapAttachment-06472");
    vk::CreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapOffsetQCOM) {
    TEST_DESCRIPTION("Ensure RenderPass end meets the requirements for VK_QCOM_fragment_density_map_offset");

    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    std::array<VkAttachmentDescription2, 7> attachments = {
        // FDM attachments
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8_UNORM, VK_SAMPLE_COUNT_1_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT),
        // input attachments
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL),
        // color attachments
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        // depth attachment
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
        // resolve attachment
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        // preserve attachments
        LvlInitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
    };

    /* TODO
    std::array<VkAttachmentReference2, 1> fdm = {
        LvlInitStruct<VkAttachmentReference2>(nullptr, 0u, VK_IMAGE_LAYOUT_GENERAL, VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
    };
    */

    std::array<VkAttachmentReference2, 1> input = {
        LvlInitStruct<VkAttachmentReference2>(nullptr, 1u, VK_IMAGE_LAYOUT_GENERAL, VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
    };

    std::array<VkAttachmentReference2, 2> color = {
        LvlInitStruct<VkAttachmentReference2>(nullptr, 2u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
        LvlInitStruct<VkAttachmentReference2>(nullptr, 3u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
    };
    auto depth = LvlInitStruct<VkAttachmentReference2>(nullptr, 4u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                       VkImageAspectFlags{VK_IMAGE_ASPECT_DEPTH_BIT});
    std::vector<VkAttachmentReference2> resolve = {
        LvlInitStruct<VkAttachmentReference2>(nullptr, 5u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
        LvlInitStruct<VkAttachmentReference2>(nullptr, VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0u),
    };
    std::vector<uint32_t> preserve = {6};

    auto subpass = LvlInitStruct<VkSubpassDescription2>(nullptr, 0u, VK_PIPELINE_BIND_POINT_GRAPHICS, 0u, size32(input),
                                                        input.data(), size32(color), color.data(), resolve.data(), &depth,
                                                        size32(preserve), preserve.data());

    // Create a renderPass with a single color attachment for fragment density map
    auto fragment_density_map_create_info = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    fragment_density_map_create_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>(&fragment_density_map_create_info, 0u, size32(attachments),
                                                       attachments.data(), 1u, &subpass, 0u, nullptr, 0u, nullptr);

    // Create rp2[0] without Multiview (zero viewMask), rp2[1] with Multiview
    vk_testing::RenderPass rp2[2];
    rp2[0].init(*m_device, rpci, true);

    subpass.viewMask = 0x3u;
    rp2[1].init(*m_device, rpci, true);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 16;
    image_create_info.extent.height = 16;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.flags = 0;

    VkImageObj fdm_image(m_device);
    fdm_image.init(&image_create_info);
    ASSERT_TRUE(fdm_image.initialized());

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;

    VkImageObj input_image(m_device);
    input_image.init(&image_create_info);
    ASSERT_TRUE(input_image.initialized());

    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj color_image1(m_device);
    color_image1.init(&image_create_info);
    ASSERT_TRUE(color_image1.initialized());

    VkImageObj color_image2(m_device);
    color_image2.init(&image_create_info);
    ASSERT_TRUE(color_image2.initialized());

    image_create_info.format = ds_format;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageObj depth_image(m_device);
    depth_image.init(&image_create_info);
    ASSERT_TRUE(depth_image.initialized());

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImageObj resolve_image(m_device);
    resolve_image.init(&image_create_info);
    ASSERT_TRUE(resolve_image.initialized());

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    VkImageObj preserve_image(m_device);
    preserve_image.init(&image_create_info);
    ASSERT_TRUE(preserve_image.initialized());

    // Create view attachment
    VkImageView iv[7];
    vk_testing::ImageView iv0;
    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = fdm_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.flags = 0;
    ivci.subresourceRange.layerCount = 3;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv0.init(*m_device, ivci);
    ASSERT_TRUE(iv0.initialized());
    iv[0] = iv0.handle();

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.image = input_image.handle();
    vk_testing::ImageView iv1;
    iv1.init(*m_device, ivci);
    ASSERT_TRUE(iv1.initialized());
    iv[1] = iv1.handle();

    ivci.image = color_image1.handle();
    vk_testing::ImageView iv2;
    iv2.init(*m_device, ivci);
    ASSERT_TRUE(iv2.initialized());
    iv[2] = iv2.handle();

    ivci.image = color_image2.handle();
    vk_testing::ImageView iv3;
    iv3.init(*m_device, ivci);
    ASSERT_TRUE(iv3.initialized());
    iv[3] = iv3.handle();

    ivci.format = ds_format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    ivci.image = depth_image.handle();
    vk_testing::ImageView iv4;
    iv4.init(*m_device, ivci);
    ASSERT_TRUE(iv4.initialized());
    iv[4] = iv4.handle();

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.image = resolve_image.handle();
    vk_testing::ImageView iv5;
    iv5.init(*m_device, ivci);
    ASSERT_TRUE(iv5.initialized());
    iv[5] = iv5.handle();

    ivci.image = preserve_image.handle();
    vk_testing::ImageView iv6;
    iv6.init(*m_device, ivci);
    ASSERT_TRUE(iv6.initialized());
    iv[6] = iv6.handle();

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.flags = 0;
    fbci.width = 16;
    fbci.height = 16;
    fbci.layers = 1;
    fbci.renderPass = rp2[0].handle();
    fbci.attachmentCount = 7;
    fbci.pAttachments = iv;

    vk_testing::Framebuffer fb1(*m_device, fbci);

    fbci.renderPass = rp2[1].handle();
    vk_testing::Framebuffer fb2(*m_device, fbci);

    // define renderpass begin info
    auto rpbi1 =
        LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp2[0].handle(), fb1.handle(), VkRect2D{{0, 0}, {16u, 16u}}, 0u, nullptr);
    auto rpbi2 =
        LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp2[1].handle(), fb2.handle(), VkRect2D{{0, 0}, {16u, 16u}}, 0u, nullptr);

    if (rp2Supported) {
        auto offsetting = LvlInitStruct<VkSubpassFragmentDensityMapOffsetEndInfoQCOM>();
        auto subpassEndInfo = LvlInitStruct<VkSubpassEndInfoKHR>(&offsetting);
        VkOffset2D m_vOffsets[2];
        offsetting.pFragmentDensityOffsets = m_vOffsets;
        offsetting.fragmentDensityOffsetCount = 2;

        auto fdm_offset_properties = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM>();
        GetPhysicalDeviceProperties2(fdm_offset_properties);

        m_vOffsets[0].x = 1;
        m_vOffsets[0].y = 1;

        m_vOffsets[1].x = 1;
        m_vOffsets[1].y = 1;

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        // begin renderpass that uses rbpi1 renderpass begin info
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi1, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503");

        if (fdm_offset_properties.fragmentDensityOffsetGranularity.width > 1) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        }

        if (fdm_offset_properties.fragmentDensityOffsetGranularity.height > 1) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06511");
        vk::CmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        // begin renderpass that uses rbpi2 renderpass begin info
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi2, VK_SUBPASS_CONTENTS_INLINE);
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06510");
        vk::CmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeFragmentShadingRate, ShadingRateImageNV) {
    TEST_DESCRIPTION("Test VK_NV_shading_rate_image.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables shading_rate_image but disables multiViewport
    auto shading_rate_image_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(shading_rate_image_features);
    if (!shading_rate_image_features.shadingRateImage) {
        GTEST_SKIP() << "shadingRateImage not supported";
    }

    features2.features.multiViewport = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test shading rate image creation
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8_UINT;
    image_create_info.extent.width = 4;
    image_create_info.extent.height = 4;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    // image type must be 2D
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-02082");

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.arrayLayers = 6;

    // must be single sample
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-samples-02083");

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    // tiling must be optimal
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-shadingRateImage-07727");

    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Should succeed.
    VkImageObj image(m_device);
    image.init(&image_create_info);

    // Test image view creation
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // view type must be 2D or 2D_ARRAY
    {
        ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        ivci.subresourceRange.layerCount = 6;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02086");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-01003");
        vk_testing::ImageView view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange.layerCount = 1;
    }

    // format must be R8_UINT
    {
        ivci.format = VK_FORMAT_R8_UNORM;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02087");
        vk_testing::ImageView view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
        ivci.format = VK_FORMAT_R8_UINT;
    }

    vk_testing::ImageView view(*m_device, ivci);

    // Test pipeline creation
    VkPipelineViewportShadingRateImageStateCreateInfoNV vsrisci = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV};

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[20] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[20] = {scissor, scissor};
    VkDynamicState dynPalette = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 1, &dynPalette};

    // viewportCount must be 0 or 1 when multiViewport is disabled
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 2;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 2;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 2;
        };
        constexpr std::array vuids = {"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                      "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                      "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"};
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, vuids);
    }

    // viewportCounts must match
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 0;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056");
    }

    // pShadingRatePalettes must not be NULL.
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 1;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04057"}));
    }

    // Create an image without the SRI bit
    VkImageObj nonSRIimage(m_device);
    nonSRIimage.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(nonSRIimage.initialized());
    VkImageView nonSRIview = nonSRIimage.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    // Test SRI layout on non-SRI image
    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
    img_barrier.image = nonSRIimage.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();

    // Error trying to convert it to SRI layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-02088");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // succeed converting it to GENERAL
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // if the view is non-NULL, it must be R8_UINT, USAGE_SRI, image layout must match, layout must be valid
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageView-02060");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageView-02061");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageView-02062");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageLayout-02063");
    vk::CmdBindShadingRateImageNV(m_commandBuffer->handle(), nonSRIview, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_errorMonitor->VerifyFound();

    VkShadingRatePaletteEntryNV paletteEntries[100] = {};
    VkShadingRatePaletteNV palette = {100, paletteEntries};
    VkShadingRatePaletteNV palettes[] = {palette, palette};

    // errors on firstViewport/viewportCount
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069");
    vk::CmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 20, 2, palettes);
    m_errorMonitor->VerifyFound();

    // shadingRatePaletteEntryCount must be in range
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071");
    vk::CmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 0, 1, palettes);
    m_errorMonitor->VerifyFound();

    VkCoarseSampleLocationNV locations[100] = {
        {0, 0, 0},    {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 1, 1},  // duplicate
        {1000, 0, 0},                                              // pixelX too large
        {0, 1000, 0},                                              // pixelY too large
        {0, 0, 1000},                                              // sample too large
    };

    // Test custom sample orders, both via pipeline state and via dynamic state
    {
        VkCoarseSampleOrderCustomNV sampOrdBadShadingRate = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV, 1, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 3, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleLocationCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV,
                                                                     2, 2, locations};
        VkCoarseSampleOrderCustomNV sampOrdDuplicateLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                 1 * 2 * 2, &locations[1]};
        VkCoarseSampleOrderCustomNV sampOrdOutOfRangeLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                  1 * 2 * 2, &locations[4]};
        VkCoarseSampleOrderCustomNV sampOrdTooLargeSampleLocationCount = {
            VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 64, &locations[8]};
        VkCoarseSampleOrderCustomNV sampOrdGood = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2, 1 * 2 * 2,
                                                   &locations[0]};

        VkPipelineViewportCoarseSampleOrderStateCreateInfoNV csosci = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV};
        csosci.sampleOrderType = VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV;
        csosci.customSampleOrderCount = 1;

        using std::vector;
        struct TestCase {
            const VkCoarseSampleOrderCustomNV *order;
            vector<std::string> vuids;
        };

        vector<TestCase> test_cases = {
            {&sampOrdBadShadingRate, {"VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073"}},
            {&sampOrdBadSampleCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074", "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdBadSampleLocationCount, {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdDuplicateLocations, {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdOutOfRangeLocations,
             {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077", "VUID-VkCoarseSampleLocationNV-pixelX-02078",
              "VUID-VkCoarseSampleLocationNV-pixelY-02079", "VUID-VkCoarseSampleLocationNV-sample-02080"}},
            {&sampOrdTooLargeSampleLocationCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076",
              "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdGood, {}},
        };

        for (const auto &test_case : test_cases) {
            const auto break_vp = [&](CreatePipelineHelper &helper) {
                helper.vp_state_ci_.pNext = &csosci;
                csosci.pCustomSampleOrders = test_case.order;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
        }

        for (const auto &test_case : test_cases) {
            for (uint32_t i = 0; i < test_case.vuids.size(); ++i) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.vuids[i]);
            }
            vk::CmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, 1, test_case.order);
            if (test_case.vuids.size()) {
                m_errorMonitor->VerifyFound();
            } else {
            }
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081");
        vk::CmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV, 1, &sampOrdGood);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeFragmentShadingRate, StageUsage) {
    TEST_DESCRIPTION("Specify shading rate pipeline stage with attachmentFragmentShadingRate feature disabled");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>();
    sync2_features.synchronization2 = VK_TRUE;  // sync2 extension guarantees feature support
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sync2_features));

    auto query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    const vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);
    const vk_testing::Event event(*m_device, LvlInitStruct<VkEventCreateInfo>());
    const vk_testing::Event event2(*m_device, LvlInitStruct<VkEventCreateInfo>());

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent2-stageMask-07316");
    vk::CmdResetEvent2KHR(*m_commandBuffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-07318");
    vk::CmdSetEvent(*m_commandBuffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-shadingRateImage-07314");
    vk::CmdWriteTimestamp(*m_commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeFragmentShadingRate, StageUsageNV) {
    TEST_DESCRIPTION(
        "Specify shading rate pipeline stage with shading rate features disabled and NV shading rate extension enabled");
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>();
    sync2_features.synchronization2 = VK_TRUE;  // sync2 extension guarantees feature support
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sync2_features));

    auto query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    const vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);
    const vk_testing::Event event(*m_device, LvlInitStruct<VkEventCreateInfo>());
    const vk_testing::Event event2(*m_device, LvlInitStruct<VkEventCreateInfo>());

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent2-stageMask-07316");
    vk::CmdResetEvent2KHR(*m_commandBuffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-07318");
    vk::CmdSetEvent(*m_commandBuffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-shadingRateImage-07314");
    vk::CmdWriteTimestamp(*m_commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}
TEST_F(NegativeFragmentShadingRate, ImageMaxLimitsQCOM) {
    TEST_DESCRIPTION("Tests physical device limits for VK_QCOM_fragment_density_map_offset.");
    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;
    auto image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.flags = 0;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8_UNORM;
    image_ci.extent = {1, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageFormatProperties img_limits;
    ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

    image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
    if (dev_limits.maxFramebufferWidth + 1 > img_limits.maxExtent.width) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02252");
    }
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06514");

    image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
    if (dev_limits.maxFramebufferHeight + 1 > img_limits.maxExtent.height) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02253");
    }
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06515");
}
