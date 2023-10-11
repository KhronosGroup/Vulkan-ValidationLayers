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
#include "../framework/pipeline_helper.h"
#include "generated/vk_extension_helper.h"

void DynamicStateTest::InitBasicExtendedDynamicState() {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
}

void DynamicStateTest::InitBasicExtendedDynamicState3(VkPhysicalDeviceExtendedDynamicState3FeaturesEXT &features) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    GetPhysicalDeviceFeatures2(features);
    RETURN_IF_SKIP(InitState(nullptr, &features));
}

TEST_F(PositiveDynamicState, DiscardRectanglesVersion) {
    TEST_DESCRIPTION("check version of VK_EXT_discard_rectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    if (!InstanceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetDiscardRectangleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, ViewportWithCountNoMultiViewport) {
    TEST_DESCRIPTION("DynamicViewportWithCount/ScissorWithCount without multiViewport feature not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }
    // Ensure multiViewport feature is *not* enabled for this device
    features2.features.multiViewport = 0;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, CmdSetVertexInputEXT) {
    TEST_DESCRIPTION("Test CmdSetVertexInputEXT");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Feature vertexInputDynamicState is not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // Fill with bad data as should be ignored with dynamic state
    VkVertexInputBindingDescription input_binding = {5, 7, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {5, 7, VK_FORMAT_UNDEFINED, 9};
    VkPipelineVertexInputStateCreateInfo vi_ci = vku::InitStructHelper();
    vi_ci.pVertexBindingDescriptions = &input_binding;
    vi_ci.vertexBindingDescriptionCount = 1;
    vi_ci.pVertexAttributeDescriptions = &input_attrib;
    vi_ci.vertexAttributeDescriptionCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.InitState();
    pipe.gp_ci_.pVertexInputState = &vi_ci;  // ignored
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.stride = sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vtx_buf.handle(), &offset);
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, CmdSetVertexInputEXTStride) {
    TEST_DESCRIPTION("Test CmdSetVertexInputEXT");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extdyn_features = vku::InitStructHelper();
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features =
        vku::InitStructHelper(&extdyn_features);
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);

    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Feature vertexInputDynamicState is not supported.";
    }
    if (!extdyn_features.extendedDynamicState) {
        GTEST_SKIP() << "Feature extendedDynamicState is not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT);
    pipe.InitState();
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.stride = sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vtx_buf.handle(), &offset);
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, ExtendedDynamicStateBindVertexBuffersMaintenance5) {
    TEST_DESCRIPTION("VK_KHR_maintenance5 lets you use VK_WHOLE_SIZE with VK_EXT_extended_dynamic_state");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper(&maintenance5_features);
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));

    m_commandBuffer->begin();
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize size = VK_WHOLE_SIZE;
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offset, &size, nullptr);
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DiscardRectanglesWithDynamicState) {
    TEST_DESCRIPTION("Don't check discard rectangles if dynamic state is not set");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // pass in struct, but don't set the dynamic state in the pipeline
    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rect_ci = vku::InitStructHelper();
    discard_rect_ci.discardRectangleMode = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT;
    discard_rect_ci.discardRectangleCount = 4;
    std::vector<VkRect2D> discard_rectangles(4);
    discard_rect_ci.pDiscardRectangles = discard_rectangles.data();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &discard_rect_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DynamicColorWriteNoColorAttachments) {
    TEST_DESCRIPTION("Create a graphics pipeline with no color attachments, but use dynamic color write enable.");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Extension enabed as a dependency of VK_EXT_color_write_enable
    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(color_write_features);
    if (!color_write_features.colorWriteEnable) {
        GTEST_SKIP() << "colorWriteEnable feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();

    // Create a render pass without any color attachments
    VkAttachmentReference attach = {};
    attach.attachment = 0;
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpasses = {};
    subpasses.pDepthStencilAttachment = &attach;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = m_depth_stencil_fmt;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpasses;
    vkt::RenderPass rp(*m_device, rpci);

    VkFramebufferCreateInfo &fbci = m_framebuffer_info;
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &depth_image_view;
    vkt::Framebuffer fb(*m_device, fbci);

    // Enable dynamic color write enable
    pipe.gp_ci_.renderPass = rp.handle();
    // pColorBlendState is not required since there are no color attachments
    pipe.gp_ci_.pColorBlendState = nullptr;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthTestEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    ASSERT_EQ(VK_SUCCESS, pipe.CreateGraphicsPipeline());

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderPass = rp.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    VkBool32 color_write_enable = VK_TRUE;
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1, &color_write_enable);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DepthTestEnableOverridesPipelineDepthWriteEnable) {
    RETURN_IF_SKIP(InitBasicExtendedDynamicState())

    m_depth_stencil_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_clear_via_load_op = false;
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj ds_image(m_device);
    ds_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                  0);
    auto ds_view = ds_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(1, &ds_view);

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    ds_state.depthWriteEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT);
    pipe.ds_ci_ = ds_state;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetDepthTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DepthTestEnableOverridesDynamicDepthWriteEnable) {
    TEST_DESCRIPTION("setting vkCmdSetDepthTestEnable to false cancels what ever is written to vkCmdSetDepthWriteEnable.");
    RETURN_IF_SKIP(InitBasicExtendedDynamicState())

    m_depth_stencil_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_clear_via_load_op = false;
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj ds_image(m_device);
    ds_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                  0);
    auto ds_view = ds_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(1, &ds_view);

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT);
    pipe.ds_ci_ = ds_state;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetDepthTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DynamicStateDoublePipelineBind) {
    TEST_DESCRIPTION("Validate binding a non-dynamic pipeline doesn't trigger dynamic static errors");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state2_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_no_dynamic(*this);
    pipe_no_dynamic.InitState();
    pipe_no_dynamic.CreateGraphicsPipeline();

    vkt::CommandBuffer command_buffer(m_device, m_commandPool);
    command_buffer.begin();
    vk::CmdSetPrimitiveRestartEnableEXT(command_buffer.handle(), VK_TRUE);
    command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_no_dynamic.pipeline_);
    vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(command_buffer.handle(), 1, 1, 0, 0);
    vk::CmdEndRenderPass(command_buffer.handle());
    command_buffer.end();
}

TEST_F(PositiveDynamicState, SetBeforePipeline) {
    TEST_DESCRIPTION("Pipeline set state, but prior to last bound pipeline that had it");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe_line(*this);
    pipe_line.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe_line.InitState();
    pipe_line.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_blend(*this);
    pipe_blend.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    pipe_blend.InitState();
    pipe_blend.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
    float blends[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vk::CmdSetBlendConstants(m_commandBuffer->handle(), blends);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_line.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_blend.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, AttachmentFeedbackLoopEnable) {
    TEST_DESCRIPTION("Use vkCmdSetAttachmentFeedbackLoopEnableEXT correctly");
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
    vk::CmdSetAttachmentFeedbackLoopEnableEXT(m_commandBuffer->handle(), VK_IMAGE_ASPECT_COLOR_BIT);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, SetDepthBias2EXTDepthBiasClampEnabled) {
    TEST_DESCRIPTION("Call vkCmdSetDepthBias2EXT with VkPhysicalDeviceFeatures::depthBiasClamp feature enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);

    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }
    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }
    if (!features2.features.depthBiasClamp) {
        GTEST_SKIP() << "depthBiasClamp not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // Create a pipeline with a dynamically set depth bias
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper();
    raster_state.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_ = raster_state;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0,
                0);  // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, SetDepthBias2EXTDepthBiasClampDisabled) {
    TEST_DESCRIPTION("Call vkCmdSetDepthBias2EXT with VkPhysicalDeviceFeatures::depthBiasClamp feature disabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);

    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }
    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }

    features2.features.depthBiasClamp = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // Create a pipeline with a dynamically set depth bias
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper();
    raster_state.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_ = raster_state;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 0.0f;  // depthBiasClamp feature is disabled, so depth_bias_info.depthBiasClamp must be 0
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0,
                0);  // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, SetDepthBias2EXTDepthBiasWithDepthBiasRepresentationInfo) {
    TEST_DESCRIPTION(
        "Call vkCmdSetDepthBias2EXT with VkDepthBiasRepresentationInfoEXT and VkPhysicalDeviceFeatures::depthBiasClamp and "
        "VkPhysicalDeviceDepthBiasControlFeaturesEXT "
        "features enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);

    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }
    if (!depth_bias_control_features.leastRepresentableValueForceUnormRepresentation) {
        GTEST_SKIP() << "leastRepresentableValueForceUnormRepresentation not supported";
    }
    if (!depth_bias_control_features.floatRepresentation) {
        GTEST_SKIP() << "floatRepresentation not supported";
    }
    if (!depth_bias_control_features.depthBiasExact) {
        GTEST_SKIP() << "depthBiasExact not supported";
    }
    if (!features2.features.depthBiasClamp) {
        GTEST_SKIP() << "depthBiasClamp not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // Create a pipeline with a dynamically set depth bias
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper();
    raster_state.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_ = raster_state;
    pipe.CreateGraphicsPipeline();
    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here and in the follow-up calls
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    VkDepthBiasRepresentationInfoEXT depth_bias_representation = vku::InitStructHelper();
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT;
    depth_bias_representation.depthBiasExact = VK_TRUE;
    depth_bias_info.pNext = &depth_bias_representation;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, AlphaToCoverageSetFalse) {
    TEST_DESCRIPTION("Dynamically set alphaToCoverageEnabled to false so its not checked.");

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state_features))
    if (!extended_dynamic_state_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location = 0) out float x;
        void main(){
            x = 1.0;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;  // should be ignored

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.pipe_ms_state_ci_ = ms_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetAlphaToCoverageEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, AlphaToCoverageSetTrue) {
    TEST_DESCRIPTION("Dynamically set alphaToCoverageEnabled to true, but have component set.");

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicExtendedDynamicState3(extended_dynamic_state_features))
    if (!extended_dynamic_state_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetAlphaToCoverageEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, MultisampleStateIgnored) {
    TEST_DESCRIPTION("Ignore null pMultisampleState, with alphaToOne disabled");

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
    if (!extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "extendedDynamicState3AlphaToOneEnable not supported";
    }
    features2.features.alphaToOne = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, MultisampleStateIgnoredAlphaToOne) {
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
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, InputAssemblyStateIgnored) {
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
    if (!dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_FALSE";
    }

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, ViewportStateIgnored) {
    TEST_DESCRIPTION("Ignore null pViewportState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    pipe.gp_ci_.pViewportState = nullptr;
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, ColorBlendStateIgnored) {
    TEST_DESCRIPTION("Ignore null pColorBlendState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
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
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
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
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, DepthBoundsTestEnableState) {
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
    pipe.ds_ci_.depthTestEnable = VK_TRUE;  // ignored
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetDepthBoundsTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    // don't need vkCmdSetDepthBounds since test is disabled now
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, ViewportInheritance) {
    TEST_DESCRIPTION("Dynamically set viewport multiple times");

    AddRequiredExtensions(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    auto inherited_viewport_scissor_features = vku::InitStruct<VkPhysicalDeviceInheritedViewportScissorFeaturesNV>();
    GetPhysicalDeviceFeatures2(inherited_viewport_scissor_features);
    RETURN_IF_SKIP(InitState(nullptr, &inherited_viewport_scissor_features));

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.vp_state_ci_.viewportCount = 2u;
    pipe.vp_state_ci_.scissorCount = 2u;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buffer(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer set_state(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkViewport viewports[2] = {{0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f}};
    const VkRect2D scissors[2] = {{{0, 0}, {100u, 100u}}, {{0, 0}, {100u, 100u}}};

    auto viewport_scissor_inheritance = vku::InitStruct<VkCommandBufferInheritanceViewportScissorInfoNV>();
    viewport_scissor_inheritance.viewportScissor2D = VK_TRUE;
    viewport_scissor_inheritance.viewportDepthCount = 2u;
    viewport_scissor_inheritance.pViewportDepths = viewports;

    auto hinfo = vku::InitStruct<VkCommandBufferInheritanceInfo>(&viewport_scissor_inheritance);
    hinfo.renderPass = m_renderPass;
    hinfo.subpass = 0;
    hinfo.framebuffer = m_framebuffer;

    auto info = vku::InitStruct<VkCommandBufferBeginInfo>();
    info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;

    cmd_buffer.begin(&info);
    vk::CmdBindPipeline(cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(cmd_buffer.handle(), 3, 1, 0, 0);
    cmd_buffer.end();

    set_state.begin();
    vk::CmdSetViewport(set_state.handle(), 1u, 1u, &viewports[1]);
    set_state.end();

    m_commandBuffer->begin();
    vk::CmdSetViewport(m_commandBuffer->handle(), 0u, 2u, viewports);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0u, 1u, viewports);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0u, 2u, scissors);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &cmd_buffer.handle());

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    auto submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveDynamicState, AttachmentFeedbackLoopEnableAspectMask) {
    TEST_DESCRIPTION("Valid aspect masks for vkCmdSetAttachmentFeedbackLoopEnableEXT");
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
    vk::CmdSetAttachmentFeedbackLoopEnableEXT(m_commandBuffer->handle(), VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    m_commandBuffer->end();
}
