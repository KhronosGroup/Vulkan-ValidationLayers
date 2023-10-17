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
#include "../framework/pipeline_helper.h"

TEST_F(NegativeDynamicState, DepthBiasNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Depth Bias dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    pipe.rs_state_ci_.lineWidth = 1.0f;
    pipe.rs_state_ci_.depthBiasEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07834");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, LineWidthNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Line Width dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07833");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, LineStippleNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Line Stipple dynamic state is required but not correctly bound.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceLineRasterizationFeaturesEXT line_rasterization_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(line_rasterization_features);
    if (!line_rasterization_features.stippledBresenhamLines || !line_rasterization_features.bresenhamLines) {
        GTEST_SKIP() << "Stipple Bresenham lines not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    VkPipelineRasterizationLineStateCreateInfoEXT line_state = vku::InitStructHelper();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_EXT);
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    line_state.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
    line_state.stippledLineEnable = VK_TRUE;
    line_state.lineStippleFactor = 1;
    line_state.lineStipplePattern = 0;
    pipe.rs_state_ci_.pNext = &line_state;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07849");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, ViewportNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Viewport dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07831");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, ScissorNotBound) {
    TEST_DESCRIPTION("Run a simple draw calls to validate failure when Scissor dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07832");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, BlendConstantsNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Blend Constants dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    pipe.cb_attachments_[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    pipe.cb_attachments_[0].blendEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07835");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, DepthBoundsNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Depth Bounds dynamic state is required but not correctly bound.");

    RETURN_IF_SKIP(Init())
    if (!m_device->phy().features().depthBounds) {
        GTEST_SKIP() << "Device does not support depthBounds test";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj depth_image(m_device);
    depth_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView depth_image_view =
        depth_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    InitRenderTarget(1, &depth_image_view);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthWriteEnable = VK_TRUE;
    pipe.ds_ci_.depthBoundsTestEnable = VK_TRUE;
    pipe.gp_ci_.renderPass = renderPass();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07836");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, StencilReadNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Stencil Read dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj depth_image(m_device);
    depth_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView depth_image_view =
        depth_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    InitRenderTarget(1, &depth_image_view);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthWriteEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    pipe.gp_ci_.renderPass = renderPass();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07837");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, StencilWriteNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Stencil Write dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj depth_image(m_device);
    depth_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView depth_image_view =
        depth_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    InitRenderTarget(1, &depth_image_view);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthWriteEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    pipe.gp_ci_.renderPass = renderPass();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07838");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, StencilRefNotBound) {
    TEST_DESCRIPTION(
        "Run a simple draw calls to validate failure when Stencil Ref dynamic state is required but not correctly bound.");
    RETURN_IF_SKIP(Init())
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj depth_image(m_device);
    depth_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView depth_image_view =
        depth_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    InitRenderTarget(1, &depth_image_view);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthWriteEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    pipe.gp_ci_.renderPass = renderPass();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07839");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, SetScissorParam) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetScissor without multiViewport feature");

    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));

    const VkRect2D scissor = {{0, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    m_commandBuffer->begin();

    // array tests
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00593");
    vk::CmdSetScissor(m_commandBuffer->handle(), 1, 1, scissors);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-arraylength");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-00594");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 2, scissors);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00593");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-00594");
    vk::CmdSetScissor(m_commandBuffer->handle(), 1, 2, scissors);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-pScissors-parameter");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, nullptr);
    m_errorMonitor->VerifyFound();

    struct TestCase {
        VkRect2D scissor;
        std::string vuid;
    };

    std::vector<TestCase> test_cases = {{{{-1, 0}, {16, 16}}, "VUID-vkCmdSetScissor-x-00595"},
                                        {{{0, -1}, {16, 16}}, "VUID-vkCmdSetScissor-x-00595"},
                                        {{{1, 0}, {vvl::kI32Max, 16}}, "VUID-vkCmdSetScissor-offset-00596"},
                                        {{{vvl::kI32Max, 0}, {1, 16}}, "VUID-vkCmdSetScissor-offset-00596"},
                                        {{{0, 0}, {uint32_t{vvl::kI32Max} + 1, 16}}, "VUID-vkCmdSetScissor-offset-00596"},
                                        {{{0, 1}, {16, vvl::kI32Max}}, "VUID-vkCmdSetScissor-offset-00597"},
                                        {{{0, vvl::kI32Max}, {16, 1}}, "VUID-vkCmdSetScissor-offset-00597"},
                                        {{{0, 0}, {16, uint32_t{vvl::kI32Max} + 1}}, "VUID-vkCmdSetScissor-offset-00597"}};

    for (const auto &test_case : test_cases) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.vuid);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &test_case.scissor);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SetScissorParamMultiviewport) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetScissor with multiViewport feature enabled");

    RETURN_IF_SKIP(Init())

    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::multiViewport is not supported";
    }

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-arraylength");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    const auto max_scissors = m_device->phy().limits_.maxViewports;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-pScissors-parameter");
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, max_scissors, nullptr);
    m_errorMonitor->VerifyFound();

    const uint32_t too_big_max_scissors = 65536 + 1;  // let's say this is too much to allocate
    if (max_scissors >= too_big_max_scissors) {
        printf("VkPhysicalDeviceLimits::maxViewports is too large to practically test against -- skipping part of test.\n");
    } else {
        const VkRect2D scissor = {{0, 0}, {16, 16}};
        const std::vector<VkRect2D> scissors(max_scissors + 1, scissor);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00592");
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, max_scissors + 1, scissors.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00592");
        vk::CmdSetScissor(m_commandBuffer->handle(), max_scissors, 1, scissors.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-firstScissor-00592");
        vk::CmdSetScissor(m_commandBuffer->handle(), 1, max_scissors, scissors.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissor-scissorCount-arraylength");
        vk::CmdSetScissor(m_commandBuffer->handle(), 1, 0, scissors.data());
        m_errorMonitor->VerifyFound();
    }
}

template <typename ExtType, typename Parm>
void ExtendedDynStateCalls(ErrorMonitor *error_monitor, VkCommandBuffer cmd_buf, ExtType ext_call, const char *vuid, Parm parm) {
    error_monitor->SetDesiredFailureMsg(kErrorBit, vuid);
    ext_call(cmd_buf, parm);
    error_monitor->VerifyFound();
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state VUs");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    // First test attempted uses of VK_EXT_extended_dynamic_state without it being enabled.
    extended_dynamic_state_features.extendedDynamicState = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    const VkDynamicState dyn_states[] = {
        VK_DYNAMIC_STATE_CULL_MODE_EXT,           VK_DYNAMIC_STATE_FRONT_FACE_EXT,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,  VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,      VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT,
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,   VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,    VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT,
        VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT, VK_DYNAMIC_STATE_STENCIL_OP_EXT,
    };
    VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03378");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);
    commandBuffer.begin();

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetCullModeEXT, "VUID-vkCmdSetCullMode-None-08971",
                          VK_CULL_MODE_NONE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetDepthBoundsTestEnableEXT,
                          "VUID-vkCmdSetDepthBoundsTestEnable-None-08971", VK_FALSE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetDepthCompareOpEXT,
                          "VUID-vkCmdSetDepthCompareOp-None-08971", VK_COMPARE_OP_NEVER);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetDepthTestEnableEXT,
                          "VUID-vkCmdSetDepthTestEnable-None-08971", VK_FALSE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetDepthWriteEnableEXT,
                          "VUID-vkCmdSetDepthWriteEnable-None-08971", VK_FALSE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetFrontFaceEXT, "VUID-vkCmdSetFrontFace-None-08971",
                          VK_FRONT_FACE_CLOCKWISE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetPrimitiveTopologyEXT,
                          "VUID-vkCmdSetPrimitiveTopology-None-08971", VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-None-08971");
    VkRect2D scissor = {{0, 0}, {1, 1}};
    vk::CmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetStencilOp-None-08971");
    vk::CmdSetStencilOpEXT(commandBuffer.handle(), VK_STENCIL_FACE_BACK_BIT, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_ZERO,
                           VK_STENCIL_OP_ZERO, VK_COMPARE_OP_NEVER);
    m_errorMonitor->VerifyFound();

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vk::CmdSetStencilTestEnableEXT,
                          "VUID-vkCmdSetStencilTestEnable-None-08971", VK_FALSE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-None-08971");
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    vk::CmdSetViewportWithCountEXT(commandBuffer.handle(), 1, &viewport);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateViewportScissorPipeline) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state pipeline creation with Viewport/Scissor");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    // Verify viewportCount and scissorCount are specified as zero.
    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Verify non-count and count dynamic states aren't used together
    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
        pipe.InitState();
        pipe.vp_state_ci_.viewportCount = 0;
        pipe.vp_state_ci_.scissorCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04132");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
        pipe.InitState();
        pipe.vp_state_ci_.viewportCount = 1;
        pipe.vp_state_ci_.scissorCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04133");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateDuplicate) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state Duplicate dynamic state");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    const VkDynamicState dyn_states[] = {
        VK_DYNAMIC_STATE_CULL_MODE_EXT,           VK_DYNAMIC_STATE_FRONT_FACE_EXT,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,  VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,      VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT,
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,   VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,    VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT,
        VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT, VK_DYNAMIC_STATE_STENCIL_OP_EXT,
    };

    // Verify dupes of every state.
    for (size_t i = 0; i < size(dyn_states); ++i) {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(dyn_states[i]);
        pipe.AddDynamicState(dyn_states[i]);
        if (dyn_states[i] == VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) {
            pipe.vp_state_ci_.viewportCount = 0;
        }
        if (dyn_states[i] == VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) {
            pipe.vp_state_ci_.scissorCount = 0;
        }
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateBindVertexBuffers) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state Duplicate dynamic state");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "extendedDynamicState no supported";
    }

    // Verify each vkCmdSet command
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription inputBinding = {0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    pipe.vi_ci_.pVertexBindingDescriptions = &inputBinding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32_SFLOAT, 0};
    pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    std::vector<VkBuffer> buffers(m_device->phy().limits_.maxVertexInputBindings + 1ull, buffer.handle());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);

    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);
    commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-firstBinding-03355");
    vk::CmdBindVertexBuffers2EXT(commandBuffer.handle(), m_device->phy().limits_.maxVertexInputBindings, 1, buffers.data(),
                                 offsets.data(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-firstBinding-03356");
    vk::CmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, m_device->phy().limits_.maxVertexInputBindings + 1, buffers.data(),
                                 offsets.data(), 0, 0);
    m_errorMonitor->VerifyFound();

    {
        vkt::Buffer bufferWrongUsage(*m_device, 16, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pBuffers-03359");
        VkBuffer buffers2[1] = {bufferWrongUsage.handle()};
        VkDeviceSize offsets2[1] = {};
        vk::CmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pBuffers-04111");
        m_errorMonitor->SetUnexpectedError("UNASSIGNED-GeneralParameterError-RequiredParameter");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindVertexBuffers2-pBuffers-parameter");
        VkBuffer buffers2[1] = {VK_NULL_HANDLE};
        VkDeviceSize offsets2[1] = {16};
        VkDeviceSize strides[1] = {m_device->phy().limits_.maxVertexInputBindingStride + 1ull};
        vk::CmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
        m_errorMonitor->VerifyFound();

        buffers2[0] = buffers[0];
        VkDeviceSize sizes[1] = {16};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pOffsets-03357");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pSizes-03358");
        vk::CmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, sizes, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pStrides-03362");
        vk::CmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, strides);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateBindVertexBuffersWholeSize) {
    TEST_DESCRIPTION("Test VK_WHOLE_SIZE with VK_EXT_extended_dynamic_state");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));

    m_commandBuffer->begin();
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize size = VK_WHOLE_SIZE;
    VkDeviceSize offset = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pSizes-03358");
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offset, &size, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateViewportScissorDraw) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state viewport/scissor draw state");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        pipe.vp_state_ci_.viewportCount = 0;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewportCount-03417");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        pipe.vp_state_ci_.scissorCount = 0;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-scissorCount-03418");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        pipe.vp_state_ci_.viewportCount = 0;
        pipe.vp_state_ci_.scissorCount = 0;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        VkRect2D scissor = {{0, 0}, {1, 1}};
        vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1, &scissor);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewportCount-03419");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateSetViewportScissor) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state viewport/scissor draw state");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    std::vector<VkBuffer> buffers(m_device->phy().limits_.maxVertexInputBindings + 1ull, buffer.handle());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription inputBinding = {0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    pipe.vi_ci_.pVertexBindingDescriptions = &inputBinding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32_SFLOAT, 0};
    pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // set everything once
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 1, &viewport);
    VkRect2D scissor = {{1, 0}, {16, 16}};
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1, &scissor);
    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VkDeviceSize strides[] = {1};
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, buffers.data(), offsets.data(), 0, strides);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pStrides-06209");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    strides[0] = 4;
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, buffers.data(), offsets.data(), 0, strides);

    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();

    if (features2.features.multiViewport) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-viewportCount-03394");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetViewportWithCount-viewportCount-arraylength");
        VkViewport viewport2 = {
            0, 0, 1, 1, 0.0f, 0.0f,
        };
        vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 0, &viewport2);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-offset-03400");
        VkRect2D scissor2 = {{1, 0}, {vvl::kI32Max, 16}};
        vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1, &scissor2);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-offset-03401");
        VkRect2D scissor2 = {{0, 1}, {16, vvl::kI32Max}};
        vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1, &scissor2);
        m_errorMonitor->VerifyFound();
    }

    if (features2.features.multiViewport) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-scissorCount-03397");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetScissorWithCount-scissorCount-arraylength");
        vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 0, 0);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-x-03399");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-x-03399");
    VkRect2D scissor3 = {{-1, -1}, {0, 0}};
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1, &scissor3);
    m_errorMonitor->VerifyFound();

    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 0, nullptr, nullptr, nullptr, nullptr);

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicStateEnabledNoMultiview) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    features2.features.multiViewport = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    InitRenderTarget();

    vkt::CommandBuffer commandBuffer(m_device, m_commandPool);
    commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-viewportCount-03395");
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    VkViewport viewports[] = {viewport, viewport};
    vk::CmdSetViewportWithCountEXT(commandBuffer.handle(), size(viewports), viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-scissorCount-03398");
    VkRect2D scissor = {{0, 0}, {1, 1}};
    VkRect2D scissors[] = {scissor, scissor};
    vk::CmdSetScissorWithCountEXT(commandBuffer.handle(), size(scissors), scissors);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicState2Disabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 VUs");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Add first extension to catch bugs where layers check from wrong feature bit
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features =
        vku::InitStructHelper(&extended_dynamic_state_features);
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }

    // Attempt using VK_EXT_extended_dynamic_state2 without it being enabled.
    extended_dynamic_state2_features.extendedDynamicState2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04868");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    vkt::CommandBuffer command_buffer(m_device, m_commandPool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizerDiscardEnable-None-08970");
    vk::CmdSetRasterizerDiscardEnableEXT(command_buffer.handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBiasEnable-None-08970");
    vk::CmdSetDepthBiasEnableEXT(command_buffer.handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPrimitiveRestartEnable-None-08970");
    vk::CmdSetPrimitiveRestartEnableEXT(command_buffer.handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicState2PatchControlPointsDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 PatchControlPoints VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2LogicOp, skipping";
    }

    // Attempt using VK_EXT_extended_dynamic_state2 without it being enabled.
    extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04870");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    vkt::CommandBuffer command_buffer(m_device, m_commandPool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPatchControlPointsEXT-None-08574");
    vk::CmdSetPatchControlPointsEXT(command_buffer.handle(), 3);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicState2LogicOpDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2LogicOp VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2LogicOp) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2LogicOp, skipping";
    }

    // Attempt using VK_EXT_extended_dynamic_state2 without it being enabled.
    extended_dynamic_state2_features.extendedDynamicState2LogicOp = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04869");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    vkt::CommandBuffer command_buffer(m_device, m_commandPool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLogicOpEXT-None-08544");
    vk::CmdSetLogicOpEXT(command_buffer.handle(), VK_LOGIC_OP_AND);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicState2Enabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 LogicOp VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT, VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT};

    for (size_t i = 0; i < size(dyn_states); ++i) {
        // Verify duplicates of every dynamic state.
        {
            CreatePipelineHelper pipe(*this);
            pipe.AddDynamicState(dyn_states[i]);
            pipe.AddDynamicState(dyn_states[i]);
            pipe.InitState();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
            pipe.CreateGraphicsPipeline();
            m_errorMonitor->VerifyFound();
        }

        // Calling draw without setting the dynamic state is an error
        {
            CreatePipelineHelper pipe(*this);
            pipe.AddDynamicState(dyn_states[i]);
            pipe.InitState();
            pipe.CreateGraphicsPipeline();

            vkt::CommandBuffer command_buffer(m_device, m_commandPool);
            command_buffer.begin();
            command_buffer.BeginRenderPass(m_renderPassBeginInfo);

            vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

            if (dyn_states[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04876");
            if (dyn_states[i] == VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT)
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04877");
            if (dyn_states[i] == VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT)
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04879");
            vk::CmdDraw(command_buffer.handle(), 1, 1, 0, 0);
            m_errorMonitor->VerifyFound();
            vk::CmdEndRenderPass(command_buffer.handle());
            command_buffer.end();
        }
    }
}

TEST_F(NegativeDynamicState, ExtendedDynamicState2PatchControlPointsEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 PatchControlPoints VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2PatchControlPoints, skipping";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // Verify dupes of the dynamic state.
    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        vkt::CommandBuffer command_buffer(m_device, m_commandPool);
        command_buffer.begin();
        command_buffer.BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        // Calling draw without setting the dynamic state is an error
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04875");
        vk::CmdDraw(command_buffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // setting an invalid value for patchControlpoints is an error
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPatchControlPointsEXT-patchControlPoints-04874");
        vk::CmdSetPatchControlPointsEXT(command_buffer.handle(), 0x1000);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(command_buffer.handle());
        command_buffer.end();
    }
}

TEST_F(NegativeDynamicState, ExtendedDynamicState2LogicOpEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 LogicOp VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2LogicOp) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2LogicOp, skipping";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    InitRenderTarget();

    // Verify dupes of the dynamic state.
    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        vkt::CommandBuffer command_buffer(m_device, m_commandPool);
        command_buffer.begin();
        command_buffer.BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        // Calling draw without setting the dynamic state is an error
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-logicOp-04878");
        vk::CmdDraw(command_buffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(command_buffer.handle());
        command_buffer.end();
    }
}

void NegativeDynamicState::ExtendedDynamicState3PipelineFeatureDisabled(VkDynamicState dynamic_state, const char *vuid) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(dynamic_state);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledTessellationDomainOrigin) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3TessellationDomainOrigin-07370");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledDepthClampEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClampEnable-07371");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledPolygonMode) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3PolygonMode-07372");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledRasterizationSamples) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationSamples-07373");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledSampleMask) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleMask-07374");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledAlphaToCoverageEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToCoverageEnable-07375");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledAlphaToOneEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToOneEnable-07376");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledLogicOpEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LogicOpEnable-07377");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledColorBlendEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEnable-07378");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledColorBlendEquation) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEquation-07379");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledColorWriteMask) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorWriteMask-07380");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledRasterizationStream) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationStream-07381");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledConservativeRasterizationMode) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ConservativeRasterizationMode-07382");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledExtraPrimitiveOverestimationSize) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ExtraPrimitiveOverestimationSize-07383");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledDepthClipEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipEnable-07384");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledSampleLocationsEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleLocationsEnable-07385");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledColorBlendAdvanced) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendAdvanced-07386");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledProvokingVertexMode) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ProvokingVertexMode-07387");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledLineRasterizationMode) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineRasterizationMode-07388");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledLineStippleEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineStippleEnable-07389");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledDepthClipNegativeOneToOne) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipNegativeOneToOne-07390");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledViewportWScalingEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportWScalingEnable-07391");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledViewportSwizzle) {
    ExtendedDynamicState3PipelineFeatureDisabled(VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV,
                                                 "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportSwizzle-07392");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledCoverageToColorEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorEnable-07393");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledCoverageToColorLocation) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorLocation-07394");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledCoverageModulationMode) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationMode-07395");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledCoverageModulationTableEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTableEnable-07396");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledCoverageModulationTable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTable-07397");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledCoverageReductionMode) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageReductionMode-07398");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledRepresentativeFragmentTestEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RepresentativeFragmentTestEnable-07399");
}

TEST_F(NegativeDynamicState, PipelineFeatureDisabledShadingRateImageEnable) {
    ExtendedDynamicState3PipelineFeatureDisabled(
        VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV,
        "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ShadingRateImageEnable-07400");
}

TEST_F(NegativeDynamicState, ExtendedDynamicState3CmdSetFeatureDisabled) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 calling vkCmdSet* without feature");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // Check feature is enable for each set command.
    vkt::CommandBuffer command_buffer(m_device, m_commandPool);
    command_buffer.begin();
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetTessellationDomainOriginEXT-None-08576");
        vk::CmdSetTessellationDomainOriginEXT(command_buffer.handle(), VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetDepthClampEnableEXT-None-08582");
        vk::CmdSetDepthClampEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPolygonModeEXT-None-08566");
        vk::CmdSetPolygonModeEXT(command_buffer.handle(), VK_POLYGON_MODE_FILL);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetRasterizationSamplesEXT-None-08552");
        vk::CmdSetRasterizationSamplesEXT(command_buffer.handle(), VK_SAMPLE_COUNT_1_BIT);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetSampleMaskEXT-None-08504");
        VkSampleMask sampleMask = 1U;
        vk::CmdSetSampleMaskEXT(command_buffer.handle(), VK_SAMPLE_COUNT_1_BIT, &sampleMask);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetAlphaToCoverageEnableEXT-None-08506");
        vk::CmdSetAlphaToCoverageEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetAlphaToOneEnableEXT-None-08508");
        vk::CmdSetAlphaToOneEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLogicOpEnableEXT-None-08542");
        vk::CmdSetLogicOpEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetColorBlendEnableEXT-None-08536");
        VkBool32 enable = VK_FALSE;
        vk::CmdSetColorBlendEnableEXT(command_buffer.handle(), 0U, 1U, &enable);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetColorBlendEquationEXT-None-08538");
        VkColorBlendEquationEXT equation = {
            VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        };
        vk::CmdSetColorBlendEquationEXT(command_buffer.handle(), 0U, 1U, &equation);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteMaskEXT-None-08540");
        VkColorComponentFlags const components = {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                  VK_COLOR_COMPONENT_A_BIT};
        vk::CmdSetColorWriteMaskEXT(command_buffer.handle(), 0U, 1U, &components);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetRasterizationStreamEXT-None-08550");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412");
        vk::CmdSetRasterizationStreamEXT(command_buffer.handle(), 0U);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetConservativeRasterizationModeEXT-None-08570");
        vk::CmdSetConservativeRasterizationModeEXT(command_buffer.handle(), VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-None-08572");
        vk::CmdSetExtraPrimitiveOverestimationSizeEXT(command_buffer.handle(), 0.0f);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetDepthClipEnableEXT-None-08584");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthClipEnableEXT-depthClipEnable-07451");
        vk::CmdSetDepthClipEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetSampleLocationsEnableEXT-None-08554");
        vk::CmdSetSampleLocationsEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetColorBlendAdvancedEXT-None-08592");
        VkColorBlendAdvancedEXT const advanced = {VK_BLEND_OP_BLUE_EXT, VK_FALSE, VK_FALSE, VK_BLEND_OVERLAP_UNCORRELATED_EXT,
                                                  VK_FALSE};
        vk::CmdSetColorBlendAdvancedEXT(command_buffer.handle(), 0U, 1U, &advanced);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetProvokingVertexModeEXT-None-08580");
        vk::CmdSetProvokingVertexModeEXT(command_buffer.handle(), VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-None-08558");
        vk::CmdSetLineRasterizationModeEXT(command_buffer.handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetLineStippleEnableEXT-None-08560");
        vk::CmdSetLineStippleEnableEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-None-08586");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-depthClipControl-07453");
        vk::CmdSetDepthClipNegativeOneToOneEXT(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetViewportWScalingEnableNV-None-08594");
        vk::CmdSetViewportWScalingEnableNV(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetViewportSwizzleNV-None-08578");
        VkViewportSwizzleNV const swizzle = {
            VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
            VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV};
        vk::CmdSetViewportSwizzleNV(command_buffer.handle(), 0U, 1U, &swizzle);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageToColorEnableNV-None-08524");
        vk::CmdSetCoverageToColorEnableNV(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageToColorLocationNV-None-08526");
        vk::CmdSetCoverageToColorLocationNV(command_buffer.handle(), 0U);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageModulationModeNV-None-08530");
        vk::CmdSetCoverageModulationModeNV(command_buffer.handle(), VK_COVERAGE_MODULATION_MODE_NONE_NV);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageModulationTableEnableNV-None-08532");
        vk::CmdSetCoverageModulationTableEnableNV(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageModulationTableNV-None-08534");
        float const modulation = 1.0f;
        vk::CmdSetCoverageModulationTableNV(command_buffer.handle(), 1U, &modulation);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetShadingRateImageEnableNV-None-08556");
        vk::CmdSetShadingRateImageEnableNV(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-None-08522");
        vk::CmdSetRepresentativeFragmentTestEnableNV(command_buffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageReductionModeNV-None-08528");
        vk::CmdSetCoverageReductionModeNV(command_buffer.handle(), VK_COVERAGE_REDUCTION_MODE_MERGE_NV);
        m_errorMonitor->VerifyFound();
    }

    command_buffer.end();
}

TEST_F(NegativeDynamicState, ExtendedDynamicState3DuplicateStatePipeline) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 Duplicate state in pipeline");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkDynamicState const dyn_states[] = {VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT,
                                         VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
                                         VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
                                         VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
                                         VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                                         VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
                                         VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT,
                                         VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
                                         VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT,
                                         VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                                         VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT,
                                         VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
                                         VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT,
                                         VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV,
                                         VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV,
                                         VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV,
                                         VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV};

    // Verify dupes of every state.
    for (size_t i = 0; i < size(dyn_states); ++i) {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(dyn_states[i]);
        pipe.AddDynamicState(dyn_states[i]);
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, DrawNotSetTessellationDomainOrigin) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3TessellationDomainOrigin) {
        GTEST_SKIP() << "extendedDynamicState3TessellationDomainOrigin not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT);
    VkPipelineTessellationDomainOriginStateCreateInfo tess_domain_ci = vku::InitStructHelper();
    tess_domain_ci.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT;
    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper(&tess_domain_ci);
    pipe.gp_ci_.pTessellationState = &tess_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07619");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdSetTessellationDomainOriginEXT(m_commandBuffer->handle(), VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetDepthClampEnable) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable) {
        GTEST_SKIP() << "extendedDynamicState3DepthClampEnable not supported";
    }
    features2.features.depthClamp = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07620");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthClampEnableEXT-depthClamp-07449");
    vk::CmdSetDepthClampEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetPolygonMode) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3PolygonMode) {
        GTEST_SKIP() << "extendedDynamicState3PolygonMode not supported";
    }
    features2.features.fillModeNonSolid = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_POLYGON_MODE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07621");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPolygonModeEXT-fillModeNonSolid-07424");
    vk::CmdSetPolygonModeEXT(m_commandBuffer->handle(), VK_POLYGON_MODE_POINT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPolygonModeEXT-polygonMode-parameter");
    // 07425 is effectively handled by VUID-vkCmdSetPolygonModeEXT-polygonMode-parameter since it triggers when the enum is used
    // without the extension being enabled m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
    // "VUID-vkCmdSetPolygonModeEXT-polygonMode-07425");
    vk::CmdSetPolygonModeEXT(m_commandBuffer->handle(), VK_POLYGON_MODE_FILL_RECTANGLE_NV);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetAlphaToOneEnable) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "extendedDynamicState3AlphaToOneEnable not supported";
    }
    features2.features.alphaToOne = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07625");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetAlphaToOneEnableEXT-alphaToOne-07607");
    vk::CmdSetAlphaToOneEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetLogicOpEnable) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable) {
        GTEST_SKIP() << "extendedDynamicState3LogicOpEnable not supported";
    }
    features2.features.logicOp = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07626");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLogicOpEnableEXT-logicOp-07366");
    vk::CmdSetLogicOpEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetColorBlendEquation) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendEquation not supported";
    }
    features2.features.dualSrcBlend = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-firstAttachment-07477");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07357");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07358");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07359");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07360");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-colorBlendOp-07361");
    VkColorBlendEquationEXT const equation = {VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_OP_ZERO_EXT,
                                              VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_OP_ZERO_EXT};
    vk::CmdSetColorBlendEquationEXT(m_commandBuffer->handle(), 0U, 1U, &equation);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetRasterizationStream) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&transform_feedback_features);
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3RasterizationStream) {
        GTEST_SKIP() << "extendedDynamicState3RasterizationStream not supported";
    }
    transform_feedback_features.transformFeedback = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07630");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412");
    if (!transform_feedback_props.transformFeedbackRasterizationStreamSelect) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07413");
    }
    vk::CmdSetRasterizationStreamEXT(m_commandBuffer->handle(), transform_feedback_props.maxTransformFeedbackStreams + 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetExtraPrimitiveOverestimationSize) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ExtraPrimitiveOverestimationSize) {
        GTEST_SKIP() << "extendedDynamicState3ExtraPrimitiveOverestimationSize not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07632");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extraPrimitiveOverestimationSize-07428");
    vk::CmdSetExtraPrimitiveOverestimationSizeEXT(m_commandBuffer->handle(), -1.0F);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetColorBlendAdvanced) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendAdvanced not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_operation_advanced);

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-firstAttachment-07479");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    if (!blend_operation_advanced.advancedBlendNonPremultipliedSrcColor) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505");
    }
    if (!blend_operation_advanced.advancedBlendNonPremultipliedDstColor) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506");
    }
    if (!blend_operation_advanced.advancedBlendCorrelatedOverlap) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendAdvancedEXT-blendOverlap-07507");
    }
    VkColorBlendAdvancedEXT advanced = {VK_BLEND_OP_ZERO_EXT, VK_TRUE, VK_TRUE, VK_BLEND_OVERLAP_DISJOINT_EXT, VK_FALSE};
    vk::CmdSetColorBlendAdvancedEXT(m_commandBuffer->handle(), 0U, 1U, &advanced);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetProvokingVertexMode) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ProvokingVertexMode) {
        GTEST_SKIP() << "extendedDynamicState3ProvokingVertexMode not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    m_commandBuffer->begin();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07636");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetProvokingVertexModeEXT-provokingVertexMode-07447");
    vk::CmdSetProvokingVertexModeEXT(m_commandBuffer->handle(), VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetLineRasterizationMode) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode) {
        GTEST_SKIP() << "extendedDynamicState3LineRasterizationMode not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    m_commandBuffer->begin();
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07637");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07418");
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07419");
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07420");
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetLineStippleEnable) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceLineRasterizationFeaturesEXT line_raster_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&line_raster_features);
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3LineStippleEnable) {
        GTEST_SKIP() << "dynamic state 3 features not supported";
    }
    if (!line_raster_features.stippledRectangularLines) {
        GTEST_SKIP() << "stippledRectangularLines features not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    m_commandBuffer->begin();
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07638");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetColorWriteMask) {
    TEST_DESCRIPTION("VK_EXT_extended_dynamic_state3 dynamic state not set before drawing");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "extendedDynamicState3ColorWriteMask not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteMaskEXT-pColorWriteMasks-parameter");
    vk::CmdSetColorWriteMaskEXT(m_commandBuffer->handle(), 0U, 1U, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, VertexInputDynamicStateDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04807
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04807");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // VUID-vkCmdSetVertexInputEXT-None-08546
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-None-08546");
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, VertexInputDynamicStateEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) vertexInputDynamicState, skipping";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        std::vector<VkVertexInputBindingDescription2EXT> bindings(m_device->phy().limits_.maxVertexInputBindings + 1u, binding);
        for (uint32_t i = 0; i < bindings.size(); ++i) bindings[i].binding = i;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), m_device->phy().limits_.maxVertexInputBindings + 1u, bindings.data(), 0,
                                 nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0};
        std::vector<VkVertexInputAttributeDescription2EXT> attributes(m_device->phy().limits_.maxVertexInputAttributes + 1u,
                                                                      attribute);
        for (uint32_t i = 0; i < attributes.size(); ++i) attributes[i].location = i;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, m_device->phy().limits_.maxVertexInputAttributes + 1u,
                                 attributes.data());
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-binding-04793
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-binding-04793");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794
    {
        VkVertexInputBindingDescription2EXT bindings[2] = {
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1},
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1}};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 2, bindings, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attributes[2] = {
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0}};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 2, attributes);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-binding-04796
    {
        VkVertexInputBindingDescription2EXT binding = {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                                                       nullptr,
                                                       m_device->phy().limits_.maxVertexInputBindings + 1u,
                                                       0,
                                                       VK_VERTEX_INPUT_RATE_VERTEX,
                                                       1};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-binding-04796");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-stride-04797
    {
        VkVertexInputBindingDescription2EXT binding = {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                                                       nullptr,
                                                       0,
                                                       m_device->phy().limits_.maxVertexInputBindingStride + 1u,
                                                       VK_VERTEX_INPUT_RATE_VERTEX,
                                                       1};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-stride-04797");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-divisor-04798
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_INSTANCE, 0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-04798");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-divisor-04799
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_INSTANCE, 2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-04799");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-location-06228
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                           nullptr,
                                                           m_device->phy().limits_.maxVertexInputAttributes + 1u,
                                                           0,
                                                           VK_FORMAT_R32G32B32A32_SFLOAT,
                                                           0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-location-06228");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-binding-06229
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                           nullptr,
                                                           0,
                                                           m_device->phy().limits_.maxVertexInputBindings + 1u,
                                                           VK_FORMAT_R32G32B32A32_SFLOAT,
                                                           0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-binding-06229");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdSetVertexInputEXT-binding-04793");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-offset-06230
    if (m_device->phy().limits_.maxVertexInputAttributeOffset <
        std::numeric_limits<decltype(m_device->phy().limits_.maxVertexInputAttributeOffset)>::max()) {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
            m_device->phy().limits_.maxVertexInputAttributeOffset + 1u};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-offset-06230");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-format-04805
    {
        const VkFormat format = VK_FORMAT_D16_UNORM;
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu(), format, &format_props);
        if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
            VkVertexInputBindingDescription2EXT binding = {
                VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
            VkVertexInputAttributeDescription2EXT attribute = {
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, format, 0};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-format-04805");
            vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
            m_errorMonitor->VerifyFound();
        }
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, VertexInputDynamicStateDivisor) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when VK_EXT_vertex_attribute_divisor is enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vertex_attribute_divisor_features = vku::InitStructHelper();
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features =
        vku::InitStructHelper(&vertex_attribute_divisor_features);
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!vertex_attribute_divisor_features.vertexAttributeInstanceRateDivisor) {
        GTEST_SKIP() << "Test requires (unsupported) vertexAttributeInstanceRateDivisor, skipping";
    }
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) vertexInputDynamicState, skipping";
    }

    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vertex_attribute_divisor_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(vertex_attribute_divisor_properties);

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    // VUID-VkVertexInputBindingDescription2EXT-divisor-06226
    if (vertex_attribute_divisor_properties.maxVertexAttribDivisor < 0xFFFFFFFFu) {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,       nullptr, 0, 0, VK_VERTEX_INPUT_RATE_INSTANCE,
            vertex_attribute_divisor_properties.maxVertexAttribDivisor + 1u};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-06226");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-divisor-06227
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,  nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX,
            vertex_attribute_divisor_properties.maxVertexAttribDivisor};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-06227");
        vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, RasterizationSamples) {
    TEST_DESCRIPTION("Make sure VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT is updating rasterizationSamples");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples) {
        GTEST_SKIP() << "extendedDynamicState3RasterizationSamples not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;  // is ignored since dynamic

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.pipe_ms_state_ci_ = ms_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdSetRasterizationSamplesEXT(m_commandBuffer->handle(), VK_SAMPLE_COUNT_4_BIT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07284");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // Should be valid now
    vk::CmdSetRasterizationSamplesEXT(m_commandBuffer->handle(), VK_SAMPLE_COUNT_1_BIT);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ColorBlendAttchment) {
    TEST_DESCRIPTION("Test all color blend attachments are dynamically set at draw time.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);

    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    constexpr uint32_t color_attachments = 2;
    InitRenderTarget(color_attachments);

    std::stringstream fsSource;
    fsSource << "#version 450\n";
    for (uint32_t i = 0; i < color_attachments; ++i) {
        fsSource << "layout(location = " << i << ") out vec4 c" << i << ";\n";
    }
    fsSource << " void main() {\n";
    for (uint32_t i = 0; i < color_attachments; ++i) {
        fsSource << "c" << i << " = vec4(0.0f);\n";
    }

    fsSource << "}";
    VkShaderObj fs(this, fsSource.str().c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this, color_attachments);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    VkBool32 color_blend_enabled[2] = {VK_FALSE, VK_FALSE};
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &color_blend_enabled[0]);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-firstAttachment-07476");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // all are set now so should work
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 2, &color_blend_enabled[0]);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

void NegativeDynamicState::InitLineRasterizationFeatureDisabled() {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceLineRasterizationFeaturesEXT line_raster_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&line_raster_features);
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode ||
        !extended_dynamic_state3_features.extendedDynamicState3LineStippleEnable) {
        GTEST_SKIP() << "dynamic state 3 features not supported";
    }

    if (!line_raster_features.rectangularLines || !line_raster_features.bresenhamLines || !line_raster_features.smoothLines) {
        GTEST_SKIP() << "line rasterization features not supported";
    }

    line_raster_features.stippledRectangularLines = VK_FALSE;
    line_raster_features.stippledBresenhamLines = VK_FALSE;
    line_raster_features.stippledSmoothLines = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();
}

TEST_F(NegativeDynamicState, RasterizationLineModeDefault) {
    TEST_DESCRIPTION("tests VK_EXT_line_rasterization dynamic state with VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT");
    RETURN_IF_SKIP(InitLineRasterizationFeatureDisabled())

    // set both from dynamic state, don't need a VkPipelineRasterizationLineStateCreateInfoEXT in pNext
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    pipe.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;  // ignored
    pipe.line_state_ci_.stippledLineEnable = VK_TRUE;                                        // ignored
    pipe.line_state_ci_.lineStippleFactor = 1;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-stippledLineEnable-07498");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // is valid now
    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, RasterizationLineModeRectangular) {
    TEST_DESCRIPTION("tests VK_EXT_line_rasterization dynamic state with VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT");
    RETURN_IF_SKIP(InitLineRasterizationFeatureDisabled())

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    pipe.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
    pipe.line_state_ci_.stippledLineEnable = VK_TRUE;  // ignored
    pipe.line_state_ci_.lineStippleFactor = 1;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-stippledLineEnable-07495");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, RasterizationLineModeBresenham) {
    TEST_DESCRIPTION("tests VK_EXT_line_rasterization dynamic state with VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT");
    RETURN_IF_SKIP(InitLineRasterizationFeatureDisabled())

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    pipe.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
    pipe.line_state_ci_.lineStippleFactor = 1;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-stippledLineEnable-07496");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, RasterizationLineModeSmooth) {
    TEST_DESCRIPTION("tests VK_EXT_line_rasterization dynamic state with VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT");
    RETURN_IF_SKIP(InitLineRasterizationFeatureDisabled())

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    pipe.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
    pipe.line_state_ci_.lineStippleFactor = 1;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-stippledLineEnable-07497");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, PipelineColorWriteCreateInfoEXTDynaimcState3) {
    TEST_DESCRIPTION("Test VkPipelineColorWriteCreateInfoEXT in color blend state pNext with VK_EXT_extended_dynamic_state3");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPipelineColorWriteCreateInfoEXT color_write = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cb_ci_.pNext = &color_write;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-07608");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5208
TEST_F(NegativeDynamicState, DISABLED_MaxFragmentDualSrcAttachmentsDynamicBlendEnable) {
    TEST_DESCRIPTION(
        "Test drawing with dual source blending with too many fragment output attachments, but using dynamic blending.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    uint32_t count = m_device->phy().limits_.maxFragmentDualSrcAttachments + 1;
    InitRenderTarget(count);

    std::stringstream fsSource;
    fsSource << "#version 450\n";
    for (uint32_t i = 0; i < count; ++i) {
        fsSource << "layout(location = " << i << ") out vec4 c" << i << ";\n";
    }
    fsSource << " void main() {\n";
    for (uint32_t i = 0; i < count; ++i) {
        fsSource << "c" << i << " = vec4(0.0f);\n";
    }

    fsSource << "}";
    VkShaderObj fs(this, fsSource.str().c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    // This is all ignored, but checking it will be ignored
    VkPipelineColorBlendAttachmentState cb_attachments = {};
    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad, but ignored
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;

    CreatePipelineHelper pipe(*this, count);
    pipe.cb_attachments_[0] = cb_attachments;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    VkBool32 color_blend_enabled[2] = {VK_TRUE, VK_FALSE};
    VkColorBlendEquationEXT color_blend_equation = {
        VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    VkColorComponentFlags color_component_flags = VK_COLOR_COMPONENT_R_BIT;

    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &color_blend_enabled[0]);  // enables
    vk::CmdSetColorBlendEquationEXT(m_commandBuffer->handle(), 0, 1, &color_blend_equation);
    vk::CmdSetColorWriteMaskEXT(m_commandBuffer->handle(), 0, 1, &color_component_flags);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Fragment-06427");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // disables blending so no error should appear
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &color_blend_enabled[1]);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ColorWriteDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_color_write_enable VUs when disabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04800
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04800");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, ColorWriteNotSet) {
    TEST_DESCRIPTION("Validate dynamic state color write enable was set before draw command");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_enable_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(color_write_enable_features);

    if (color_write_enable_features.colorWriteEnable == VK_FALSE) {
        GTEST_SKIP() << "colorWriteEnable feature is not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget(2);

    VkPipelineColorBlendAttachmentState color_blend[2] = {};
    color_blend[0].blendEnable = VK_TRUE;
    color_blend[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    color_blend[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    color_blend[0].colorBlendOp = VK_BLEND_OP_ADD;
    color_blend[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend[0].alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend[1].blendEnable = VK_TRUE;
    color_blend[1].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    color_blend[1].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    color_blend[1].colorBlendOp = VK_BLEND_OP_ADD;
    color_blend[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend[1].alphaBlendOp = VK_BLEND_OP_ADD;

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = color_blend;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07749");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBool32 color_write_enable[] = {VK_TRUE, VK_FALSE};
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1, color_write_enable);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-attachmentCount-07750");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 2, color_write_enable);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ColorWriteEnableAttachmentCount) {
    TEST_DESCRIPTION("Invalid usage of attachmentCount for vkCmdSetColorWriteEnableEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    // Feature required to be supported for extension
    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_features = vku::InitStructHelper();
    color_write_features.colorWriteEnable = VK_TRUE;
    VkPhysicalDeviceFeatures2 pd_features2 = vku::InitStructHelper(&color_write_features);
    RETURN_IF_SKIP(InitState(nullptr, &pd_features2));
    InitRenderTarget();

    // need a valid array to index into
    std::vector<VkBool32> color_write_enable(m_device->phy().limits_.maxColorAttachments + 1, VK_TRUE);

    CreatePipelineHelper helper(*this);
    helper.InitState();
    helper.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);

    // Value can't be zero
    // TODO: The generated code is not use the correct implicit VUID, but at least its still correctly validating
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-arraylength");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID_Undefined");
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 0, color_write_enable.data());
    m_errorMonitor->VerifyFound();

    // over the limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-06656");
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), m_device->phy().limits_.maxColorAttachments + 1,
                                  color_write_enable.data());
    m_errorMonitor->VerifyFound();

    // mismatch of attachmentCount value is allowed for dynamic
    // see https://gitlab.khronos.org/vulkan/vulkan/-/issues/2868
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 2, color_write_enable.data());

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ColorWriteEnableFeature) {
    TEST_DESCRIPTION("Invalid usage of vkCmdSetColorWriteEnableEXT with feature not enabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkBool32 color_write_enable[2] = {VK_TRUE, VK_FALSE};

    CreatePipelineHelper helper(*this);
    helper.InitState();
    helper.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteEnableEXT-None-04803");
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1, color_write_enable);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DiscardRectanglesNotSet) {
    TEST_DESCRIPTION("Validate dynamic state for VK_EXT_discard_rectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rect_ci = vku::InitStructHelper();
    discard_rect_ci.discardRectangleMode = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT;
    discard_rect_ci.discardRectangleCount = 4;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &discard_rect_ci;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07751");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // only fill in [0, 1, 3] index
    VkRect2D discard_rectangles[2] = {{{0, 0}, {16, 16}}, {{0, 0}, {16, 16}}};
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 2, discard_rectangles);
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 3, 1, discard_rectangles);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07751");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, StateNotSetWithCommandBufferResetBitmask) {
    TEST_DESCRIPTION("Make sure state tracker of dynamic state accounts for resetting command buffers");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rect_ci = vku::InitStructHelper();
    discard_rect_ci.discardRectangleMode = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT;
    discard_rect_ci.discardRectangleCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &discard_rect_ci;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkRect2D discard_rectangles = {{0, 0}, {16, 16}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // The dynamic state was not set for this lifetime of this command buffer
    // implicitly via vkBeginCommandBuffer
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07751");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    // set again for this command buffer
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // reset command buffer from the pool
    vk::ResetCommandPool(device(), m_commandPool->handle(), 0);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07751");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, StateNotSetWithCommandBufferReset) {
    TEST_DESCRIPTION("Make sure state tracker of dynamic state accounts for resetting command buffers");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(sample_locations_props);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        GTEST_SKIP() << "Required sample location sample count VK_SAMPLE_COUNT_1_BIT not supported";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    pipe.CreateGraphicsPipeline();

    VkSampleLocationEXT sample_location = {0.5f, 0.5f};
    VkSampleLocationsInfoEXT sample_locations_info = vku::InitStructHelper();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = {1u, 1u};
    sample_locations_info.sampleLocationsCount = 1;
    sample_locations_info.pSampleLocations = &sample_location;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // The dynamic state was not set for this lifetime of this command buffer
    // implicitly via vkBeginCommandBuffer
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06666");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    // set again for this command buffer
    vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // reset command buffer from the pool
    vk::ResetCommandPool(device(), m_commandPool->handle(), 0);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06666");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SampleLocations) {
    TEST_DESCRIPTION("Test invalid cases of VK_EXT_sample_location");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(sample_locations_props);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        GTEST_SKIP() << "VK_SAMPLE_COUNT_1_BIT sampleLocationSampleCounts is not supported";
    }

    const bool support_64_sample_count = ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_64_BIT) != 0);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // If S8_UINT is supported, check not having depth with sample location compatible bit
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_S8_UINT, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
        image_create_info.flags = VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;
        image_create_info.format = VK_FORMAT_S8_UINT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-01533");
    }

    const VkFormat depth_format = FindSupportedDepthStencilFormat(gpu());

    image_create_info.flags = 0;  // image will not have needed flag
    image_create_info.format = depth_format;
    VkImageObj depth_image(m_device);
    depth_image.init(&image_create_info);
    ASSERT_TRUE(depth_image.initialized());
    VkImageView depth_image_view = depth_image.targetView(depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkImageObj color_image(m_device);
    color_image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(color_image.initialized());
    VkImageView color_image_view = color_image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    VkAttachmentDescription descriptions[2] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, depth_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
    m_renderPass_attachments.push_back(descriptions[0]);
    m_renderPass_attachments.push_back(descriptions[1]);
    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_stencil_ref = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &color_ref, nullptr, &depth_stencil_ref, 0, nullptr};
    m_renderPass_subpasses.push_back(subpass);
    m_renderPass_info = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 2u, descriptions, 1u, &subpass, 0u, nullptr);
    vk::CreateRenderPass(m_device->device(), &m_renderPass_info, NULL, &m_renderPass);

    // Create a framebuffer
    m_framebuffer_attachments.push_back(color_image_view);
    m_framebuffer_attachments.push_back(depth_image_view);
    m_framebuffer_info =
        vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, m_renderPass, 2u, m_framebuffer_attachments.data(), 128u, 128u, 1u);
    vk::CreateFramebuffer(m_device->handle(), &m_framebuffer_info, nullptr, &m_framebuffer);

    VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
    vk::GetPhysicalDeviceMultisamplePropertiesEXT(gpu(), VK_SAMPLE_COUNT_1_BIT, &multisample_prop);
    // 1 from VK_SAMPLE_COUNT_1_BIT
    const uint32_t valid_count =
        multisample_prop.maxSampleLocationGridSize.width * multisample_prop.maxSampleLocationGridSize.height * 1;

    if (valid_count <= 1) {
        GTEST_SKIP() << "Need a maxSampleLocationGridSize width x height greater than 1";
    }

    std::vector<VkSampleLocationEXT> sample_location(valid_count, {0.5, 0.5});
    VkSampleLocationsInfoEXT sample_locations_info = vku::InitStructHelper();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = multisample_prop.maxSampleLocationGridSize;
    sample_locations_info.sampleLocationsCount = valid_count;
    sample_locations_info.pSampleLocations = sample_location.data();

    VkPipelineSampleLocationsStateCreateInfoEXT sample_location_state = vku::InitStructHelper();
    sample_location_state.sampleLocationsEnable = VK_TRUE;
    sample_location_state.sampleLocationsInfo = sample_locations_info;

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = vku::InitStructHelper(&sample_location_state);
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    VkPipelineDepthStencilStateCreateInfo pipe_ds_state_ci = vku::InitStructHelper();
    pipe_ds_state_ci.depthTestEnable = VK_TRUE;
    pipe_ds_state_ci.stencilTestEnable = VK_FALSE;

    {
        CreatePipelineHelper pipe(*this);
        pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
        pipe.InitState();
        pipe.gp_ci_.pDepthStencilState = &pipe_ds_state_ci;

        // Set invalid grid size width
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.width =
            multisample_prop.maxSampleLocationGridSize.width + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07610");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.width = multisample_prop.maxSampleLocationGridSize.width;

        // Set invalid grid size height
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
            multisample_prop.maxSampleLocationGridSize.height + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07611");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height = multisample_prop.maxSampleLocationGridSize.height;

        // Test to make sure the modulo is correct due to akward wording in spec
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
            multisample_prop.maxSampleLocationGridSize.height * 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07611");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height = multisample_prop.maxSampleLocationGridSize.height;

        if (multisample_prop.maxSampleLocationGridSize.height > 1) {
            // Expects there to be no 07611 vuid
            sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
                multisample_prop.maxSampleLocationGridSize.height / 2;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
            pipe.CreateGraphicsPipeline();
            m_errorMonitor->VerifyFound();
            sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
                multisample_prop.maxSampleLocationGridSize.height;
        }

        // non-matching rasterizationSamples
        pipe.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07612");
        // if grid size is different
        m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07610");
        m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07611");
        m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        pipe.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    // Creates valid pipelines with dynamic state
    CreatePipelineHelper dynamic_pipe(*this);
    dynamic_pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    dynamic_pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    dynamic_pipe.InitState();
    dynamic_pipe.gp_ci_.pDepthStencilState = &pipe_ds_state_ci;
    dynamic_pipe.CreateGraphicsPipeline();

    auto rp_begin =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, m_renderPass, m_framebuffer, VkRect2D{{0, 0}, {128u, 128u}}, 0u, nullptr);
    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_begin);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, dynamic_pipe.pipeline_);

    // test trying to use unsupported sample count
    if (support_64_sample_count == false) {
        sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_64_BIT;
        sample_locations_info.sampleLocationsCount = valid_count * 64;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsPerPixel-01526");
        vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
        m_errorMonitor->VerifyFound();

        sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
        sample_locations_info.sampleLocationsCount = valid_count;
    }

    // Test invalid sample location count
    sample_locations_info.sampleLocationsCount = valid_count + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
    vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    m_errorMonitor->VerifyFound();
    sample_locations_info.sampleLocationsCount = valid_count;

    // Test image was never created with VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT
    vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-sampleLocationsEnable-02689");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SetViewportParam) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetViewport without multiViewport feature");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));

    const VkViewport vp = {0.0, 0.0, 64.0, 64.0, 0.0, 1.0};
    const VkViewport viewports[] = {vp, vp};

    m_commandBuffer->begin();

    // array tests
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01224");
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 1, viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-01225");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 2, viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01224");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-01225");
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 2, viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-pViewports-parameter");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, nullptr);
    m_errorMonitor->VerifyFound();

    // core viewport tests
    using std::vector;
    struct TestCase {
        VkViewport vp;
        std::string veid;
    };

    // not necessarily boundary values (unspecified cast rounding), but guaranteed to be over limit
    const auto one_past_max_w = NearestGreater(static_cast<float>(m_device->phy().limits_.maxViewportDimensions[0]));
    const auto one_past_max_h = NearestGreater(static_cast<float>(m_device->phy().limits_.maxViewportDimensions[1]));

    const auto min_bound = m_device->phy().limits_.viewportBoundsRange[0];
    const auto max_bound = m_device->phy().limits_.viewportBoundsRange[1];
    const auto one_before_min_bounds = NearestSmaller(min_bound);
    const auto one_past_max_bounds = NearestGreater(max_bound);

    const auto below_zero = NearestSmaller(0.0f);
    const auto past_one = NearestGreater(1.0f);

    vector<TestCase> test_cases = {
        {{0.0, 0.0, 0.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-width-01770"},
        {{0.0, 0.0, one_past_max_w, 64.0, 0.0, 1.0}, "VUID-VkViewport-width-01771"},
        {{0.0, 0.0, NAN, 64.0, 0.0, 1.0}, "VUID-VkViewport-width-01770"},
        {{0.0, 0.0, 64.0, one_past_max_h, 0.0, 1.0}, "VUID-VkViewport-height-01773"},
        {{one_before_min_bounds, 0.0, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01774"},
        {{one_past_max_bounds, 0.0, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01232"},
        {{NAN, 0.0, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01774"},
        {{0.0, one_before_min_bounds, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-y-01775"},
        {{0.0, NAN, 64.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-y-01775"},
        {{max_bound, 0.0, 1.0, 64.0, 0.0, 1.0}, "VUID-VkViewport-x-01232"},
        {{0.0, max_bound, 64.0, 1.0, 0.0, 1.0}, "VUID-VkViewport-y-01233"},
        {{0.0, 0.0, 64.0, 64.0, below_zero, 1.0}, "VUID-VkViewport-minDepth-01234"},
        {{0.0, 0.0, 64.0, 64.0, past_one, 1.0}, "VUID-VkViewport-minDepth-01234"},
        {{0.0, 0.0, 64.0, 64.0, NAN, 1.0}, "VUID-VkViewport-minDepth-01234"},
        {{0.0, 0.0, 64.0, 64.0, 0.0, below_zero}, "VUID-VkViewport-maxDepth-01235"},
        {{0.0, 0.0, 64.0, 64.0, 0.0, past_one}, "VUID-VkViewport-maxDepth-01235"},
        {{0.0, 0.0, 64.0, 64.0, 0.0, NAN}, "VUID-VkViewport-maxDepth-01235"},
    };

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        test_cases.push_back({{0.0, 0.0, 64.0, 0.0, 0.0, 1.0}, "VUID-VkViewport-apiVersion-07917"});
        test_cases.push_back({{0.0, 0.0, 64.0, NAN, 0.0, 1.0}, "VUID-VkViewport-apiVersion-07917"});
    } else {
        test_cases.push_back({{0.0, 0.0, 64.0, NAN, 0.0, 1.0}, "VUID-VkViewport-height-01773"});
    }

    for (const auto &test_case : test_cases) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.veid);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &test_case.vp);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, SetViewportParamMaintenance1) {
    TEST_DESCRIPTION("Verify errors are detected on misuse of SetViewport with a negative viewport extension enabled.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    const auto &limits = m_device->phy().limits_;
    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-height-01773");
    // not necessarily boundary values (unspecified cast rounding), but guaranteed to be over limit
    const float one_before_min_h = NearestSmaller(-static_cast<float>(limits.maxViewportDimensions[1]));
    VkViewport viewport = {0.0, 0.0, 64.0, one_before_min_h, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-height-01773");
    const float one_past_max_h = NearestGreater(static_cast<float>(limits.maxViewportDimensions[1]));
    viewport = {0.0, 0.0, 64.0, one_past_max_h, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-height-01773");
    viewport = {0.0, 0.0, 64.0, NAN, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();

    const float min_bound = limits.viewportBoundsRange[0];
    const float max_bound = limits.viewportBoundsRange[1];
    const float one_before_min_bound = NearestSmaller(min_bound);
    const float one_past_max_bound = NearestGreater(max_bound);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-y-01775");
    viewport = {0.0, one_before_min_bound, 64.0, 1.0, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-y-01776");
    viewport = {0.0, one_past_max_bound, 64.0, -1.0, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-y-01777");
    viewport = {0.0, min_bound, 64.0, -1.0, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkViewport-y-01233");
    viewport = {0.0, max_bound, 64.0, 1.0, 0.0, 1.0};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, SetViewportParamMultiviewport) {
    TEST_DESCRIPTION("Test parameters of vkCmdSetViewport with multiViewport feature enabled");

    RETURN_IF_SKIP(Init())

    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::multiViewport is not supported";
    }

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 0, nullptr);
    m_errorMonitor->VerifyFound();

    const auto max_viewports = m_device->phy().limits_.maxViewports;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-pViewports-parameter");
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, max_viewports, nullptr);
    m_errorMonitor->VerifyFound();

    const uint32_t too_big_max_viewports = 65536 + 1;  // let's say this is too much to allocate
    if (max_viewports >= too_big_max_viewports) {
        printf("VkPhysicalDeviceLimits::maxViewports is too large to practically test against -- skipping part of test.\n");
    } else {
        const VkViewport vp = {0.0, 0.0, 64.0, 64.0, 0.0, 1.0};
        const std::vector<VkViewport> viewports(max_viewports + 1, vp);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01223");
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, max_viewports + 1, viewports.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01223");
        vk::CmdSetViewport(m_commandBuffer->handle(), max_viewports, 1, viewports.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-firstViewport-01223");
        vk::CmdSetViewport(m_commandBuffer->handle(), 1, max_viewports, viewports.data());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
        vk::CmdSetViewport(m_commandBuffer->handle(), 1, 0, viewports.data());
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, CmdSetDiscardRectangleEXTOffsets) {
    TEST_DESCRIPTION("Test CmdSetDiscardRectangleEXT with invalid offsets in pDiscardRectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rectangle_properties);

    if (discard_rectangle_properties.maxDiscardRectangles == 0) {
        GTEST_SKIP() << "Discard rectangles are not supported";
    }

    VkRect2D discard_rectangles = {};
    discard_rectangles.offset.x = -1;
    discard_rectangles.offset.y = 0;
    discard_rectangles.extent.width = 64;
    discard_rectangles.extent.height = 64;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-x-00587");
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();

    discard_rectangles.offset.x = 0;
    discard_rectangles.offset.y = -32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-x-00587");
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, CmdSetDiscardRectangleEXTRectangleCountOverflow) {
    TEST_DESCRIPTION("Test CmdSetDiscardRectangleEXT with invalid offsets in pDiscardRectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkRect2D discard_rectangles = {};
    discard_rectangles.offset.x = 1;
    discard_rectangles.offset.y = 0;
    discard_rectangles.extent.width = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
    discard_rectangles.extent.height = 64;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-offset-00588");
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();

    discard_rectangles.offset.x = 0;
    discard_rectangles.offset.y = std::numeric_limits<int32_t>::max();
    discard_rectangles.extent.width = 64;
    discard_rectangles.extent.height = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-offset-00589");
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, CmdSetDiscardRectangleEXTRectangleCount) {
    TEST_DESCRIPTION("Test CmdSetDiscardRectangleEXT with invalid offsets in pDiscardRectangles");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rectangle_properties);

    VkRect2D discard_rectangles = {};
    discard_rectangles.offset.x = 0;
    discard_rectangles.offset.y = 0;
    discard_rectangles.extent.width = 64;
    discard_rectangles.extent.height = 64;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-firstDiscardRectangle-00585");
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), discard_rectangle_properties.maxDiscardRectangles, 1,
                                  &discard_rectangles);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, DiscardRectanglesVersion) {
    TEST_DESCRIPTION("check version of VK_EXT_discard_rectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    if (DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 1";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;

    CreatePipelineHelper pipe(*this);
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07855");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.dyn_state_ci_.dynamicStateCount = 0;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEnableEXT-specVersion-07851");
    vk::CmdSetDiscardRectangleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

// Not possible to hit the desired failure messages given invalid enums.
TEST_F(NegativeDynamicState, DISABLED_ExtensionDynamicStatesSetWOExtensionEnabled) {
    TEST_DESCRIPTION("Create a graphics pipeline with Extension dynamic states without enabling the required Extensions.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    using std::vector;
    struct TestCase {
        uint32_t dynamic_state_count;
        VkDynamicState dynamic_state;

        char const *errmsg;
    };

    vector<TestCase> dyn_test_cases = {
        {1, VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
         "contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but VK_NV_clip_space_w_scaling"},
        {1, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
         "contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but VK_EXT_discard_rectangles"},
        {1, VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, "contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but VK_EXT_sample_locations"},
    };

    for (const auto &test_case : dyn_test_cases) {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(test_case.dynamic_state);
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.errmsg);
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, ViewportAndScissorUndefinedDrawState) {
    TEST_DESCRIPTION("Test viewport and scissor dynamic state that is not set before draw");

    RETURN_IF_SKIP(Init())

    // TODO: should also test on !multiViewport
    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "Device does not support multiple viewports/scissors; skipped.\n";
    }

    InitRenderTarget();

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};

    CreatePipelineHelper pipeline_dyn_vp(*this);
    pipeline_dyn_vp.InitState();
    pipeline_dyn_vp.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline_dyn_vp.CreateGraphicsPipeline();

    CreatePipelineHelper pipeline_dyn_sc(*this);
    pipeline_dyn_sc.InitState();
    pipeline_dyn_sc.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipeline_dyn_sc.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07831");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_dyn_vp.Handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 1, &viewport);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07832");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_dyn_sc.Handle());
    vk::CmdSetScissor(m_commandBuffer->handle(), 1, 1, &scissor);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, Duplicate) {
    TEST_DESCRIPTION("Create a pipeline with duplicate dynamic states set.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkDynamicState dynamic_states[4] = {VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                                        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE};

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.dyn_state_ci_ = vku::InitStructHelper();
    pipe.dyn_state_ci_.flags = 0;
    pipe.dyn_state_ci_.dynamicStateCount = 4;
    pipe.dyn_state_ci_.pDynamicStates = dynamic_states;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Should error twice since 2 sets of duplicates now
    dynamic_states[3] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, NonGraphics) {
    TEST_DESCRIPTION("Create a pipeline with non graphics dynamic states set.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03578");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, ViewportCountWithExtendedDynamicState) {
    TEST_DESCRIPTION("Create a pipeline with invalid viewport count with extended dynamic state.");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkDynamicState dynamic_state = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.dyn_state_ci_ = vku::InitStructHelper();
    pipe.dyn_state_ci_.dynamicStateCount = 1;
    pipe.dyn_state_ci_.pDynamicStates = &dynamic_state;
    pipe.vp_state_ci_.viewportCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.dyn_state_ci_.dynamicStateCount = 0;
    pipe.vp_state_ci_.viewportCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, PipelineColorBlendStateCreateInfoArrayNonDynamic) {
    TEST_DESCRIPTION("Validate VkPipelineColorBlendStateCreateInfo array with no extensions");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const auto set_info = [](CreatePipelineHelper &helper) { helper.cb_ci_.pAttachments = nullptr; };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07353");
}

TEST_F(NegativeDynamicState, PipelineColorBlendStateCreateInfoArrayDynamic) {
    TEST_DESCRIPTION("Validate VkPipelineColorBlendStateCreateInfo array with VK_EXT_extended_dynamic_state3");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation) {
        GTEST_SKIP() << "features not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state3_features));
    InitRenderTarget();

    {
        const auto set_info = [](CreatePipelineHelper &helper) { helper.cb_ci_.pAttachments = nullptr; };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                          "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07353");
    }

    // invalid if using only some dynamic state
    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
        pipe.InitState();
        pipe.cb_ci_.pAttachments = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07353");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, SettingCommands) {
    TEST_DESCRIPTION("Verify if pipeline doesn't setup dynamic state, but set dynamic commands");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdSetLineWidth(m_commandBuffer->handle(), 1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08608");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

void NegativeDynamicState::ExtendedDynamicStateDrawNotSet(VkDynamicState dynamic_state, const char *vuid) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(dynamic_state);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetCullMode) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_CULL_MODE, "VUID-vkCmdDraw-None-07840");
}

TEST_F(NegativeDynamicState, DrawNotSetFrontFace) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_FRONT_FACE, "VUID-vkCmdDraw-None-07841");
}

TEST_F(NegativeDynamicState, DrawNotSetPrimitiveTopology) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, "VUID-vkCmdDraw-None-07842");
}

TEST_F(NegativeDynamicState, DrawNotSetDepthTestEnable) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, "VUID-vkCmdDraw-None-07843");
}

TEST_F(NegativeDynamicState, DrawNotSetDepthWriteEnable) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, "VUID-vkCmdDraw-None-07844");
}

TEST_F(NegativeDynamicState, DrawNotSetDepthCompareOp) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP, "VUID-vkCmdDraw-None-07845");
}

TEST_F(NegativeDynamicState, DrawNotSetDepthBoundsTestEnable) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, "VUID-vkCmdDraw-None-07846");
}

TEST_F(NegativeDynamicState, DrawNotSetStencilTestEnable) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE, "VUID-vkCmdDraw-None-07847");
}

TEST_F(NegativeDynamicState, DrawNotSetStencilOp) {
    ExtendedDynamicStateDrawNotSet(VK_DYNAMIC_STATE_STENCIL_OP, "VUID-vkCmdDraw-None-07848");
}

TEST_F(NegativeDynamicState, DepthRangeUnrestricted) {
    TEST_DESCRIPTION("Test setting minDepthBounds and maxDepthBounds without VK_EXT_depth_range_unrestricted");

    // Extension doesn't have feature bit, so not enabling extension invokes restrictions
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    if (VK_TRUE != device_features.depthBounds) {
        GTEST_SKIP() << "Test requires unsupported depthBounds feature";
    }
    RETURN_IF_SKIP(InitState())

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthBoundsTestEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.ds_ci_ = ds_ci;
    pipe.InitState();

    pipe.ds_ci_.minDepthBounds = 1.5f;
    pipe.ds_ci_.maxDepthBounds = 1.0f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.ds_ci_.minDepthBounds = 1.0f;
    pipe.ds_ci_.maxDepthBounds = 1.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Add dynamic depth stencil state instead
    pipe.ds_ci_.minDepthBounds = 0.0f;
    pipe.ds_ci_.maxDepthBounds = 0.0f;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBounds-minDepthBounds-00600");
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 1.5f, 0.0f);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBounds-maxDepthBounds-00601");
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 0.0f, 1.5f);
    m_errorMonitor->VerifyFound();

    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 1.0f, 1.0f);
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DepthBoundsTestEnableState) {
    TEST_DESCRIPTION("Dynamically set depthBoundsTestEnable and not call vkCmdSetDepthBounds before the draw");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));

    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "extendedDynamicState no supported";
    }

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthTestEnable = VK_FALSE;  // ignored
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetDepthBoundsTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07836");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ViewportStateIgnored) {
    TEST_DESCRIPTION("Ignore null pViewportState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    pipe.gp_ci_.pViewportState = nullptr;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-09024");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // missing VK_EXT_extended_dynamic_state3
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-09024");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, Viewport) {
    TEST_DESCRIPTION("Test VkPipelineViewportStateCreateInfo viewport and scissor count validation for non-multiViewport");

    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));
    InitRenderTarget();

    if (m_device->phy().limits_.maxViewports < 3) {
        GTEST_SKIP() << "maxViewports is not large enough";
    }

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[] = {scissor, scissor};

    // test viewport and scissor arrays
    using std::vector;
    struct TestCase {
        uint32_t viewport_count;
        VkViewport *viewports;
        uint32_t scissor_count;
        VkRect2D *scissors;

        vector<std::string> vuids;
    };

    vector<TestCase> test_cases = {
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {1,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
        {1, nullptr, 1, scissors, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130"}},
        {1, viewports, 1, nullptr, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}},
        {1,
         nullptr,
         1,
         nullptr,
         {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}},
        {2,
         nullptr,
         3,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130",
          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}},
        {2,
         nullptr,
         2,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}},
    };

    for (const auto &test_case : test_cases) {
        const auto break_vp = [&test_case](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }

    vector<TestCase> dyn_test_cases = {
        {0,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {1,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {1,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
        {2,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
        {0,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136", "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         nullptr,
         3,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"}},
    };

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    for (const auto &test_case : dyn_test_cases) {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
            dyn_state_ci.dynamicStateCount = size(dyn_states);
            dyn_state_ci.pDynamicStates = dyn_states;
            helper.dyn_state_ci_ = dyn_state_ci;

            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }
}

TEST_F(NegativeDynamicState, MultiViewport) {
    TEST_DESCRIPTION("Test VkPipelineViewportStateCreateInfo viewport and scissor count validation for multiViewport feature");

    RETURN_IF_SKIP(Init())  // enables all supported features

    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::multiViewport is not supported";
    }
    // at least 16 viewports supported from here on

    InitRenderTarget();

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[] = {scissor, scissor};

    using std::vector;
    struct TestCase {
        uint32_t viewport_count;
        VkViewport *viewports;
        uint32_t scissor_count;
        VkRect2D *scissors;

        vector<std::string> vuids;
    };

    vector<TestCase> test_cases = {
        {0,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {0,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"}},
        {2, nullptr, 2, scissors, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130"}},
        {2, viewports, 2, nullptr, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}},
        {2,
         nullptr,
         2,
         nullptr,
         {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}},
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"}},
    };

    const auto max_viewports = m_device->phy().limits_.maxViewports;
    const bool max_viewports_maxxed = max_viewports == std::numeric_limits<decltype(max_viewports)>::max();
    if (max_viewports_maxxed) {
        printf("VkPhysicalDeviceLimits::maxViewports is UINT32_MAX -- skipping part of test requiring to exceed maxViewports.\n");
    } else {
        const auto too_much_viewports = max_viewports + 1;
        // avoid potentially big allocations by using only nullptr
        test_cases.push_back({too_much_viewports,
                              nullptr,
                              2,
                              scissors,
                              {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                               "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134",
                               "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130"}});
        test_cases.push_back({2,
                              viewports,
                              too_much_viewports,
                              nullptr,
                              {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                               "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134",
                               "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}});
        test_cases.push_back(
            {too_much_viewports,
             nullptr,
             too_much_viewports,
             nullptr,
             {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
              "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130",
              "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131"}});
    }

    for (const auto &test_case : test_cases) {
        const auto break_vp = [&test_case](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }

    vector<TestCase> dyn_test_cases = {
        {0,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}},
        {0,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"}},
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"}},
    };

    if (!max_viewports_maxxed) {
        const auto too_much_viewports = max_viewports + 1;
        // avoid potentially big allocations by using only nullptr
        dyn_test_cases.push_back({too_much_viewports,
                                  nullptr,
                                  2,
                                  scissors,
                                  {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                   "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}});
        dyn_test_cases.push_back({2,
                                  viewports,
                                  too_much_viewports,
                                  nullptr,
                                  {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                   "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134"}});
        dyn_test_cases.push_back({too_much_viewports,
                                  nullptr,
                                  too_much_viewports,
                                  nullptr,
                                  {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                   "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219"}});
    }

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    for (const auto &test_case : dyn_test_cases) {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
            dyn_state_ci.dynamicStateCount = size(dyn_states);
            dyn_state_ci.pDynamicStates = dyn_states;
            helper.dyn_state_ci_ = dyn_state_ci;

            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }
}

TEST_F(NegativeDynamicState, ScissorWithCount) {
    TEST_DESCRIPTION("Validate creating graphics pipeline with dynamic state scissor with count.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        pipe.InitState();
        pipe.vp_state_ci_.scissorCount = 0;
        pipe.vp_state_ci_.viewportCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        pipe.InitState();
        VkRect2D scissors = {};
        pipe.vp_state_ci_.scissorCount = 1;
        pipe.vp_state_ci_.pScissors = &scissors;
        pipe.vp_state_ci_.viewportCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, DrawNotSetSampleLocations) {
    TEST_DESCRIPTION("Validate dynamic sample locations.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    InitRenderTarget();

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(sample_locations_props);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        GTEST_SKIP() << "Required sample location sample count VK_SAMPLE_COUNT_1_BIT not supported";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06666");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkSampleLocationEXT sample_location = {0.5f, 0.5f};

    VkSampleLocationsInfoEXT sample_locations_info = vku::InitStructHelper();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = {1u, 1u};
    sample_locations_info.sampleLocationsCount = 1;
    sample_locations_info.pSampleLocations = &sample_location;

    vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, LineWidth) {
    TEST_DESCRIPTION("Test non-1.0 lineWidth errors when pipeline is created and in vkCmdSetLineWidth");
    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));
    InitRenderTarget();

    const std::array test_cases = {-1.0f, 0.0f, NearestSmaller(1.0f), NearestGreater(1.0f),
                                   std::numeric_limits<float>::quiet_NaN()};

    // test VkPipelineRasterizationStateCreateInfo::lineWidth
    for (const auto test_case : test_cases) {
        const auto set_lineWidth = [&](CreatePipelineHelper &helper) { helper.rs_state_ci_.lineWidth = test_case; };
        CreatePipelineHelper::OneshotTest(*this, set_lineWidth, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749");
    }

    // test vk::CmdSetLineWidth
    m_commandBuffer->begin();

    for (const auto test_case : test_cases) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineWidth-lineWidth-00788");
        vk::CmdSetLineWidth(m_commandBuffer->handle(), test_case);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicState, SetAfterStaticPipeline) {
    TEST_DESCRIPTION("Pipeline without state is set and tried to use vkCmdSet");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe_line(*this);
    pipe_line.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe_line.InitState();
    pipe_line.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_static(*this);
    pipe_static.InitState();
    pipe_static.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_static.pipeline_);  // ignored
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_line.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_static.pipeline_);
    vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08608");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetAttachmentFeedbackLoopEnable) {
    TEST_DESCRIPTION("Set state in pipeline, but never set in command buffer before draw");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT feedback_loop_dynamic_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(feedback_loop_dynamic_features);
    RETURN_IF_SKIP(InitState(nullptr, &feedback_loop_dynamic_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08877");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, AttachmentFeedbackLoopEnableFeatures) {
    TEST_DESCRIPTION("Call vkCmdSetAttachmentFeedbackLoopEnableEXT without features enabled");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    // features not enabled
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopDynamicState-08862");  // attachmentFeedbackLoopDynamicState
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopLayout-08864");  // attachmentFeedbackLoopLayout
    vk::CmdSetAttachmentFeedbackLoopEnableEXT(m_commandBuffer->handle(), VK_IMAGE_ASPECT_COLOR_BIT);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, AttachmentFeedbackLoopEnableAspectMask) {
    TEST_DESCRIPTION("Bad aspect masks for vkCmdSetAttachmentFeedbackLoopEnableEXT");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT feedback_loop_dynamic_features = vku::InitStructHelper();
    VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT feedback_loop_features =
        vku::InitStructHelper(&feedback_loop_dynamic_features);
    GetPhysicalDeviceFeatures2(feedback_loop_features);
    RETURN_IF_SKIP(InitState(nullptr, &feedback_loop_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-aspectMask-08863");
    vk::CmdSetAttachmentFeedbackLoopEnableEXT(m_commandBuffer->handle(), VK_IMAGE_ASPECT_PLANE_0_BIT);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SetDepthBias2EXTDepthBiasClampDisabled) {
    TEST_DESCRIPTION("Call vkCmdSetDepthBias2EXT with VkPhysicalDeviceFeatures::depthBiasClamp feature disabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);
    features2.features.depthBiasClamp = VK_FALSE;

    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }
    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDepthBiasInfoEXT-depthBiasClamp-08950");
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SetDepthBias2EXTDepthBiasControlFeaturesDisabled) {
    TEST_DESCRIPTION(
        "Call vkCmdSetDepthBias2EXT with VkPhysicalDeviceFeatures::depthBiasClamp and VkPhysicalDeviceDepthBiasControlFeaturesEXT "
        "features disabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);
    features2.features.depthBiasClamp = VK_FALSE;
    depth_bias_control_features.leastRepresentableValueForceUnormRepresentation = VK_FALSE;
    depth_bias_control_features.floatRepresentation = VK_FALSE;
    depth_bias_control_features.depthBiasExact = VK_FALSE;

    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }
    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe_line(*this);
    pipe_line.InitState();
    pipe_line.CreateGraphicsPipeline();

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 0.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    VkDepthBiasRepresentationInfoEXT depth_bias_representation = vku::InitStructHelper();
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT;
    depth_bias_representation.depthBiasExact = VK_TRUE;
    depth_bias_info.pNext = &depth_bias_representation;

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDepthBiasRepresentationInfoEXT-leastRepresentableValueForceUnormRepresentation-08947");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDepthBiasRepresentationInfoEXT-depthBiasExact-08949");
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);
    m_errorMonitor->VerifyFound();

    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT;
    depth_bias_representation.depthBiasExact = VK_FALSE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDepthBiasRepresentationInfoEXT-floatRepresentation-08948");
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);
    m_errorMonitor->VerifyFound();

    // Perform a successful call to vk::CmdSetDepthBias2EXT, but bound pipeline has not set depth bias as a dynamic state
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_line.pipeline_);
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08608");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, AlphaToCoverageOutputNoAlpha) {
    TEST_DESCRIPTION("Dynamically set alphaToCoverageEnabled to true and not have component set.");

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state_features))
    if (!extended_dynamic_state_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 x;
        void main(){
           x = vec3(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_FALSE;  // should be ignored

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.pipe_ms_state_ci_ = ms_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetAlphaToCoverageEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-alphaToCoverageEnable-08919");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, ShadingRateImageEnableNotSet) {
    TEST_DESCRIPTION("Create pipeline with VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV but dont set the dynamic state.");

    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state_features))
    if (!extended_dynamic_state_features.extendedDynamicState3ShadingRateImageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3ShadingRateImageEnable";
    }
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07647");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, CoverageReductionModeNotSet) {
    TEST_DESCRIPTION("Create pipeline with VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV but dont set the dynamic state.");

    AddRequiredExtensions(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state_features))
    if (!extended_dynamic_state_features.extendedDynamicState3CoverageReductionMode) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3CoverageReductionMode";
    }
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07649");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, DrawNotSetExclusiveScissor) {
    TEST_DESCRIPTION("Validate dynamic exclusive scissor.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusiveScissorFeatures = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(exclusiveScissorFeatures);
    if (exclusiveScissorFeatures.exclusiveScissor == VK_FALSE) {
        GTEST_SKIP() << "exclusiveScissor not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    uint32_t count;
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> properties(count);
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, properties.data());
    for (const auto &p : properties) {
        if (std::strcmp(p.extensionName, VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME) == 0) {
            if (p.specVersion < 2) {
                GTEST_SKIP() << "exclusiveScissor specVersion 2 not supported.";
            }
        }
    }
    InitRenderTarget();

    CreatePipelineHelper pipe1(*this);
    pipe1.InitState();
    pipe1.AddDynamicState(VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV);
    pipe1.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2(*this);
    pipe2.InitState();
    pipe2.AddDynamicState(VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV);
    pipe2.AddDynamicState(VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV);
    pipe2.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07878");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07879");
    VkBool32 exclusiveScissorEnable = VK_TRUE;
    vk::CmdSetExclusiveScissorEnableNV(m_commandBuffer->handle(), 0u, 1u, &exclusiveScissorEnable);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SetDepthBiasClampDisabled) {
    TEST_DESCRIPTION("Call vkCmdSetDepthBias with depthBiasClamp disabled");
    VkPhysicalDeviceFeatures device_features = {};
    device_features.depthBiasClamp = VK_FALSE;
    RETURN_IF_SKIP(Init(&device_features));

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBias-depthBiasClamp-00790");
    vk::CmdSetDepthBias(m_commandBuffer->handle(), 0.0f, 1.0f, 0.0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, MultisampleStateIgnored) {
    TEST_DESCRIPTION("dont ignore null pMultisampleState because missing some dynamic states");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3RasterizationSamples";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3SampleMask) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3SampleMask";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    features2.features.alphaToOne = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT);
    // missing VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, MultisampleStateIgnoredAlphaToOne) {
    TEST_DESCRIPTION("Ignore null pMultisampleState with alphaToOne enabled");
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state3_features))
    if (!extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3RasterizationSamples";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3SampleMask) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3SampleMask";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToOneEnable";
    }
    if (!m_device->phy().features().alphaToOne) {
        GTEST_SKIP() << "Test requires (unsupported) alphaToOne";
    }
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    // missing VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, InputAssemblyStateIgnored) {
    TEST_DESCRIPTION("Ignore null pInputAssemblyState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features =
        vku::InitStructHelper(&extended_dynamic_state_features);
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state3_features))
    InitRenderTarget();

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamic_state_3_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dynamic_state_3_props);
    if (dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_TRUE";
    }

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-dynamicPrimitiveTopologyUnrestricted-09031");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, ColorBlendStateIgnored) {
    TEST_DESCRIPTION("Ignore null pColorBlendState");
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state3_features))
    InitRenderTarget();

    if (!extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable) {
        GTEST_SKIP() << "extendedDynamicState3LogicOpEnable not supported";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendEnable not supported";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendEquation not supported";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "extendedDynamicState3ColorWriteMask not supported";
    }

    CreatePipelineHelper pipe(*this);
    // Missing VK_DYNAMIC_STATE_LOGIC_OP_EXT
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);

    VkPipelineColorBlendAttachmentState att_state = {};
    att_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state.blendEnable = VK_TRUE;
    pipe.cb_attachments_[0] = att_state;
    pipe.gp_ci_.pColorBlendState = nullptr;

    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-09030");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, VertexInputLocationMissing) {
    TEST_DESCRIPTION("Shader uses a location not provided with dynamic vertex input");

    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dsf = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dsf);
    if (!vertex_input_dsf.vertexInputDynamicState) {
        GTEST_SKIP() << "vertexInputDynamicState not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec4 x;
        layout(location = 1) in vec4 y;
        layout(location = 0) out vec4 c;
        void main() {
           c = x * y;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-Input-07939");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0u;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0u, 1u, &buffer.handle(), &offset);

    VkVertexInputBindingDescription2EXT vertexInputBindingDescription = vku::InitStructHelper();
    vertexInputBindingDescription.binding = 0u;
    vertexInputBindingDescription.stride = sizeof(float) * 4;
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBindingDescription.divisor = 1u;
    VkVertexInputAttributeDescription2EXT vertexAttributeDescription = vku::InitStructHelper();
    vertexAttributeDescription.location = 0u;
    vertexAttributeDescription.binding = 0u;
    vertexAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttributeDescription.offset = 0u;
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1u, &vertexInputBindingDescription, 1u, &vertexAttributeDescription);

    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicState, MissingCmdSetVertexInput) {
    TEST_DESCRIPTION("Validate VK_EXT_color_write_enable VUs when disabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features =
        vku::InitStructHelper(&extended_dynamic_state_features);
    GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) vertexInputDynamicState, skipping";
    }

    RETURN_IF_SKIP(InitState(nullptr, &vertex_input_dynamic_state_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04912");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04914");
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, MissingCmdBindVertexBuffers2) {
    TEST_DESCRIPTION("Validate VK_EXT_color_write_enable VUs when disabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStrides-04913");
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0u, 0u, nullptr, nullptr, nullptr, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStrides-04913");
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, AdvancedBlendMaxAttachments) {
    TEST_DESCRIPTION("Attempt to use more than maximum attachments in subpass when advanced blend is enabled");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamic_state_3_features = vku::InitStructHelper(&dynamic_rendering_features);
    GetPhysicalDeviceFeatures2(dynamic_state_3_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }
    if (!dynamic_state_3_features.extendedDynamicState3ColorBlendEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3ColorBlendEnable";
    }
    if (!dynamic_state_3_features.extendedDynamicState3ColorBlendAdvanced) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3ColorBlendAdvanced";
    }

    RETURN_IF_SKIP(InitState(nullptr, &dynamic_state_3_features));

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_advanced_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_advanced_props);
    uint32_t attachment_count = blend_advanced_props.advancedBlendMaxColorAttachments + 1;

    if (attachment_count > m_device->phy().limits_.maxColorAttachments) {
        GTEST_SKIP() << "advancedBlendMaxColorAttachments is equal to maxColorAttachments";
    }

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::vector<std::unique_ptr<VkImageObj>> images(attachment_count);
    std::vector<VkImageView> image_views(attachment_count);
    std::vector<VkRenderingAttachmentInfo> rendering_attachment_info(attachment_count);
    for (uint32_t i = 0; i < attachment_count; ++i) {
        images[i] = std::make_unique<VkImageObj>(m_device);
        images[i]->init(&image_ci);
        image_views[i] = images[i]->targetView(image_ci.format);
        rendering_attachment_info[i] = vku::InitStructHelper();
        rendering_attachment_info[i].imageView = image_views[i];
        rendering_attachment_info[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rendering_attachment_info[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        rendering_attachment_info[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        rendering_attachment_info[i].clearValue.color = m_clear_color;
    }

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = attachment_count;
    rendering_info.pColorAttachments = rendering_attachment_info.data();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    for (uint32_t i = 0; i < attachment_count; ++i) {
        VkBool32 color_blend_enable = i == 0;
        vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), i, 1u, &color_blend_enable);
        VkColorBlendAdvancedEXT color_blend_advanced;
        color_blend_advanced.advancedBlendOp = VK_BLEND_OP_ADD;
        color_blend_advanced.srcPremultiplied = VK_FALSE;
        color_blend_advanced.dstPremultiplied = VK_FALSE;
        color_blend_advanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
        color_blend_advanced.clampResults = VK_FALSE;
        vk::CmdSetColorBlendAdvancedEXT(m_commandBuffer->handle(), i, 1u, &color_blend_advanced);
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-advancedBlendMaxColorAttachments-07480");
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, MissingColorAttachmentBlendBit) {
    TEST_DESCRIPTION(
        "Dynamically enable blend for color attachment that does not have VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamic_state_3_features = vku::InitStructHelper(&dynamic_rendering_features);
    GetPhysicalDeviceFeatures2(dynamic_state_3_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "dynamicRendering not supported";
    }
    if (!dynamic_state_3_features.extendedDynamicState3ColorBlendEnable) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendEnable not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &dynamic_state_3_features));

    VkFormat format = VK_FORMAT_R32G32B32A32_SINT;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), format, &format_properties);

    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0 ||
        (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) != 0) {
        GTEST_SKIP() << "Required foramt features not available";
    }

    VkImageObj image(m_device);
    image.Init(32u, 32u, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(format);

    VkClearValue clear_value;
    clear_value.color = {{0, 0, 0, 0}};

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = clear_value;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32u, 32u}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pColorAttachments = &color_attachment;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(rendering_info);
    VkBool32 enable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0u, 1u, &enable);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pColorBlendEnables-07470");
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicState, SampleLocationsSamplesMismatch) {
    TEST_DESCRIPTION("Dynamically set sample locations samples that don't match that of the pipeline");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    RETURN_IF_SKIP(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    pipe.CreateGraphicsPipeline();

    VkSampleLocationEXT sample_locations[2] = {{0.5f, 0.5f}, {0.5f, 0.5f}};

    VkSampleLocationsInfoEXT sapmle_locations_info = vku::InitStructHelper();
    sapmle_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_2_BIT;
    sapmle_locations_info.sampleLocationGridSize = {1u, 1u};
    sapmle_locations_info.sampleLocationsCount = 2u;
    sapmle_locations_info.pSampleLocations = sample_locations;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sapmle_locations_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-sampleLocationsPerPixel-07482");
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
