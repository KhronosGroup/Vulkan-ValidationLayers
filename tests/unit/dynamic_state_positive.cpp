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

void DynamicStateTest::InitBasicExtendedDynamicState() {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &extended_dynamic_state_features));
}

void DynamicStateTest::InitBasicExtendedDynamicState3(VkPhysicalDeviceExtendedDynamicState3FeaturesEXT &features) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    GetPhysicalDeviceFeatures2(features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features));
}

TEST_F(PositiveDynamicState, DiscardRectanglesVersion) {
    TEST_DESCRIPTION("check version of VK_EXT_discard_rectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!InstanceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dyn_state_ci_ = dyn_state_ci;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }
    // Ensure multiViewport feature is *not* enabled for this device
    features2.features.multiViewport = 0;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
    };
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, CmdSetVertexInputEXT) {
    TEST_DESCRIPTION("Test CmdSetVertexInputEXT");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto vertex_input_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Feature vertexInputDynamicState is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Fill with bad data as should be ignored with dynamic state
    VkVertexInputBindingDescription input_binding = {5, 7, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {5, 7, VK_FORMAT_UNDEFINED, 9};
    auto vi_ci = LvlInitStruct<VkPipelineVertexInputStateCreateInfo>();
    vi_ci.pVertexBindingDescriptions = &input_binding;
    vi_ci.vertexBindingDescriptionCount = 1;
    vi_ci.pVertexAttributeDescriptions = &input_attrib;
    vi_ci.vertexAttributeDescriptionCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VERTEX_INPUT_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.gp_ci_.pVertexInputState = &vi_ci;  // ignored
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = LvlInitStruct<VkVertexInputBindingDescription2EXT>();
    binding.binding = 0;
    binding.stride = sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = LvlInitStruct<VkVertexInputAttributeDescription2EXT>();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    VkBufferObj vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto extdyn_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto vertex_input_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>(&extdyn_features);
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);

    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Feature vertexInputDynamicState is not supported.";
    }
    if (!extdyn_features.extendedDynamicState) {
        GTEST_SKIP() << "Feature extendedDynamicState is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = LvlInitStruct<VkVertexInputBindingDescription2EXT>();
    binding.binding = 0;
    binding.stride = sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = LvlInitStruct<VkVertexInputAttributeDescription2EXT>();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    VkBufferObj vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan 1.1 is required";
    }

    auto maintenance5_features = LvlInitStruct<VkPhysicalDeviceMaintenance5FeaturesKHR>();
    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>(&maintenance5_features);
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &extended_dynamic_state_features));

    m_commandBuffer->begin();
    VkBufferObj buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize size = VK_WHOLE_SIZE;
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offset, &size, nullptr);
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DiscardRectanglesWithDynamicState) {
    TEST_DESCRIPTION("Don't check discard rectangles if dynamic state is not set");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // pass in struct, but don't set the dynamic state in the pipeline
    auto discard_rect_ci = LvlInitStruct<VkPipelineDiscardRectangleStateCreateInfoEXT>();
    discard_rect_ci.discardRectangleMode = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT;
    discard_rect_ci.discardRectangleCount = 4;
    std::vector<VkRect2D> discard_rectangles(4);
    discard_rect_ci.pDiscardRectangles = discard_rectangles.data();

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME << " not supported";
    }

    // Extension enabed as a dependency of VK_EXT_color_write_enable
    auto color_write_features = LvlInitStruct<VkPhysicalDeviceColorWriteEnableFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(color_write_features);
    if (!color_write_features.colorWriteEnable) {
        GTEST_SKIP() << "colorWriteEnable feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
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
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpasses;
    vk_testing::RenderPass rp(*m_device, rpci);

    VkFramebufferCreateInfo &fbci = m_framebuffer_info;
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = m_depthStencil->BindInfo();
    vk_testing::Framebuffer fb(*m_device, fbci);

    // Enable dynamic color write enable
    pipe.gp_ci_.renderPass = rp.handle();
    // pColorBlendState is not required since there are no color attachments
    pipe.gp_ci_.pColorBlendState = nullptr;
    VkDynamicState dyn_state = VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT;
    auto dynamic_state = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_state.dynamicStateCount = 1;
    dynamic_state.pDynamicStates = &dyn_state;
    pipe.gp_ci_.pDynamicState = &dynamic_state;
    pipe.ds_ci_ = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.ds_ci_.depthTestEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline());

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
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DepthTestEnableOverridesPipelineDepthWriteEnable) {
    InitBasicExtendedDynamicState();
    if (::testing::Test::IsSkipped()) return;

    m_depth_stencil_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_clear_via_load_op = false;
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj ds_image(m_device);
    ds_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                  0);
    auto ds_view = ds_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(1, &ds_view));

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT};
    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_state.depthWriteEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.ds_ci_ = ds_state;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetDepthTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DepthTestEnableOverridesDynamicDepthWriteEnable) {
    TEST_DESCRIPTION("setting vkCmdSetDepthTestEnable to false cancels what ever is written to vkCmdSetDepthWriteEnable.");
    InitBasicExtendedDynamicState();
    if (::testing::Test::IsSkipped()) return;

    m_depth_stencil_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_clear_via_load_op = false;
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    VkImageObj ds_image(m_device);
    ds_image.Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                  0);
    auto ds_view = ds_image.targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(1, &ds_view));

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT, VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT};
    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.ds_ci_ = ds_state;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetDepthTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, DynamicStateDoublePipelineBind) {
    TEST_DESCRIPTION("Validate binding a non-dynamic pipeline doesn't trigger dynamic static errors");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &extended_dynamic_state2_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_state = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = &dyn_state;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_no_dynamic(*this);
    pipe_no_dynamic.InitInfo();
    pipe_no_dynamic.InitState();
    pipe_no_dynamic.CreateGraphicsPipeline();

    VkCommandBufferObj command_buffer(m_device, m_commandPool);
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

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_BLEND_CONSTANTS};

    CreatePipelineHelper pipe_line(*this);
    pipe_line.InitInfo();
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = &dyn_states[0];
    pipe_line.dyn_state_ci_ = dyn_state_ci;
    pipe_line.InitState();
    pipe_line.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_blend(*this);
    pipe_blend.InitInfo();
    dyn_state_ci.pDynamicStates = &dyn_states[1];
    pipe_blend.dyn_state_ci_ = dyn_state_ci;
    pipe_blend.InitState();
    pipe_blend.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
    float blends[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vk::CmdSetBlendConstants(m_commandBuffer->handle(), blends);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_line.pipeline_);
    m_commandBuffer->Draw(1, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_blend.pipeline_);
    m_commandBuffer->Draw(1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicState, AttachmentFeedbackLoopEnable) {
    TEST_DESCRIPTION("Use vkCmdSetAttachmentFeedbackLoopEnableEXT correctly");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto feedback_loop_dynamic_features = LvlInitStruct<VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT>();
    auto feedback_loop_features =
        LvlInitStruct<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>(&feedback_loop_dynamic_features);
    GetPhysicalDeviceFeatures2(feedback_loop_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &feedback_loop_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT};
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto depth_bias_control_features =
        LvlInitStruct<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(&extended_dynamic_state2_features);
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

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create a pipeline with a dynamically set depth bias
    const VkPipelineLayoutObj pl(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.AddShader(&vs);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.MakeDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS);
    auto raster_state = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    raster_state.depthBiasEnable = VK_TRUE;
    pipe.SetRasterization(&raster_state);
    pipe.CreateVKPipeline(pl.handle(), m_renderPass);

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    auto depth_bias_info = LvlInitStruct<VkDepthBiasInfoEXT>();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto depth_bias_control_features =
        LvlInitStruct<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(&extended_dynamic_state2_features);
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);

    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }
    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }

    features2.features.depthBiasClamp = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create a pipeline with a dynamically set depth bias
    const VkPipelineLayoutObj pl(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.AddShader(&vs);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.MakeDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS);
    auto raster_state = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    raster_state.depthBiasEnable = VK_TRUE;
    pipe.SetRasterization(&raster_state);
    pipe.CreateVKPipeline(pl.handle(), m_renderPass);

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    auto depth_bias_info = LvlInitStruct<VkDepthBiasInfoEXT>();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 0.0f;  // depthBiasClamp feature is disabled, so depth_bias_info.depthBiasClamp must be 0
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto depth_bias_control_features =
        LvlInitStruct<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(&extended_dynamic_state2_features);
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

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create a pipeline with a dynamically set depth bias
    const VkPipelineLayoutObj pl(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.AddShader(&vs);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.MakeDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS);
    auto raster_state = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    raster_state.depthBiasEnable = VK_TRUE;
    pipe.SetRasterization(&raster_state);
    pipe.CreateVKPipeline(pl.handle(), m_renderPass);

    m_commandBuffer->begin();

    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);

    auto depth_bias_info = LvlInitStruct<VkDepthBiasInfoEXT>();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_commandBuffer->handle(), &depth_bias_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here and in the follow-up calls
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    auto depth_bias_representation = LvlInitStruct<VkDepthBiasRepresentationInfoEXT>();
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

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    InitBasicExtendedDynamicState3(extended_dynamic_state_features);
    if (::testing::Test::IsSkipped()) return;
    if (!extended_dynamic_state_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(location = 0) out float x;
        void main(){
            x = 1.0;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;  // should be ignored

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dyn_state_ci_ = dyn_state_ci;
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

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    InitBasicExtendedDynamicState3(extended_dynamic_state_features);
    if (::testing::Test::IsSkipped()) return;
    if (!extended_dynamic_state_features.extendedDynamicState3AlphaToCoverageEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToCoverageEnable";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dyn_state_ci_ = dyn_state_ci;
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