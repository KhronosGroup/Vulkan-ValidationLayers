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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(NegativePipelineAdvancedBlend, BlendOps) {
    TEST_DESCRIPTION("Advanced blending with invalid VkBlendOps");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget(2);

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_operation_advanced);

    if (!blend_operation_advanced.advancedBlendAllOperations) {
        GTEST_SKIP() << "advancedBlendAllOperations is not supported.";
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state = vku::InitStructHelper();
    VkPipelineColorBlendAttachmentState attachment_states[2];
    memset(attachment_states, 0, sizeof(VkPipelineColorBlendAttachmentState) * 2);

    // only 1 attachment state, different blend op values
    const auto set_info_different = [&](CreatePipelineHelper &helper) {
        attachment_states[0].blendEnable = VK_TRUE;
        attachment_states[0].colorBlendOp = VK_BLEND_OP_HSL_COLOR_EXT;
        attachment_states[0].alphaBlendOp = VK_BLEND_OP_MULTIPLY_EXT;

        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = attachment_states;
        helper.gp_ci_.pColorBlendState = &color_blend_state;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info_different, kErrorBit,
                                      "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01406");

    // Test is if independent blend is not supported
    if (!blend_operation_advanced.advancedBlendIndependentBlend && blend_operation_advanced.advancedBlendMaxColorAttachments > 1) {
        const auto set_info_color = [&](CreatePipelineHelper &helper) {
            attachment_states[0].blendEnable = VK_TRUE;
            attachment_states[0].colorBlendOp = VK_BLEND_OP_MIN;
            attachment_states[0].alphaBlendOp = VK_BLEND_OP_MIN;
            attachment_states[1].blendEnable = VK_TRUE;
            attachment_states[1].colorBlendOp = VK_BLEND_OP_MULTIPLY_EXT;
            attachment_states[1].alphaBlendOp = VK_BLEND_OP_MULTIPLY_EXT;

            color_blend_state.attachmentCount = 2;
            color_blend_state.pAttachments = attachment_states;
            helper.gp_ci_.pColorBlendState = &color_blend_state;
        };
        constexpr std::array vuids = {"VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01407",
                                      "VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01408"};
        CreatePipelineHelper::OneshotTest(*this, set_info_color, kErrorBit, vuids);
    }
}

TEST_F(NegativePipelineAdvancedBlend, MaxBlendAttachment) {
    TEST_DESCRIPTION("Advanced blending with invalid VkBlendOps");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget(3);

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_operation_advanced_props);
    if (blend_operation_advanced_props.advancedBlendMaxColorAttachments > 2) {
        GTEST_SKIP() << "advancedBlendMaxColorAttachments is too high";
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state = vku::InitStructHelper();
    VkPipelineColorBlendAttachmentState attachment_states[3];
    memset(attachment_states, 0, sizeof(VkPipelineColorBlendAttachmentState) * 3);

    // over max blend color attachment count
    const auto set_info = [&](CreatePipelineHelper &helper) {
        attachment_states[0].blendEnable = VK_TRUE;
        attachment_states[0].colorBlendOp = VK_BLEND_OP_MULTIPLY_EXT;
        attachment_states[0].alphaBlendOp = VK_BLEND_OP_MULTIPLY_EXT;
        attachment_states[1] = attachment_states[0];
        attachment_states[2] = attachment_states[0];

        color_blend_state.attachmentCount = 3;
        color_blend_state.pAttachments = attachment_states;
        helper.gp_ci_.pColorBlendState = &color_blend_state;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410");
}

TEST_F(NegativePipelineAdvancedBlend, Properties) {
    TEST_DESCRIPTION("Test VkPipelineColorBlendAdvancedStateCreateInfoEXT with unsupported properties");

    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_operation_advanced_props);

    if (blend_operation_advanced_props.advancedBlendCorrelatedOverlap &&
        blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor &&
        blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
        GTEST_SKIP() << "All VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT properties are enabled; nothing to test";
    }

    VkPipelineColorBlendAdvancedStateCreateInfoEXT color_blend_advanced = vku::InitStructHelper();
    color_blend_advanced.blendOverlap = VK_BLEND_OVERLAP_DISJOINT_EXT;
    color_blend_advanced.dstPremultiplied = VK_FALSE;
    color_blend_advanced.srcPremultiplied = VK_FALSE;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cb_ci_.pNext = &color_blend_advanced;
    if (!blend_operation_advanced_props.advancedBlendCorrelatedOverlap) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-blendOverlap-01426");
    }
    if (!blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-dstPremultiplied-01425");
    }
    if (!blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-srcPremultiplied-01424");
    }
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineInvalidAdvancedBlend) {
    TEST_DESCRIPTION("Create a graphics pipeline with advanced blend when its disabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_operation_advanced);

    if (blend_operation_advanced.advancedBlendAllOperations) {
        GTEST_SKIP() << "advancedBlendAllOperations is VK_TRUE, test needs it not supported.";
    }

    CreatePipelineHelper pipe(*this);

    VkPipelineColorBlendAttachmentState attachment_state = {};
    attachment_state.blendEnable = VK_TRUE;
    attachment_state.colorBlendOp = VK_BLEND_OP_XOR_EXT;
    attachment_state.alphaBlendOp = VK_BLEND_OP_XOR_EXT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = vku::InitStructHelper();
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &attachment_state;
    pipe.gp_ci_.pColorBlendState = &color_blend_state;

    pipe.InitState();
    // When using profiles, advancedBlendMaxColorAttachments might be zero
    m_errorMonitor->SetUnexpectedError("VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendAllOperations-01409");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}
