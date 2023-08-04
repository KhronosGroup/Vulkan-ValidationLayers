/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021-2022 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"

class DynamicRenderingCommandBufferInheritanceRenderingInfoTest : public VkLayerTest {
  public:
    void Test(bool const useLinearColorAttachmen);
};

void DynamicRenderingCommandBufferInheritanceRenderingInfoTest::Test(bool const useLinearColorAttachment) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    if (useLinearColorAttachment) {
        AddRequiredExtensions(VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto linear_color_attachment = LvlInitStruct<VkPhysicalDeviceLinearColorAttachmentFeaturesNV>();
    if (IsExtensionsEnabled(VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME)) {
        dynamic_rendering_features.pNext = &linear_color_attachment;
    }
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }
    if (useLinearColorAttachment && !linear_color_attachment.linearColorAttachment) {
        GTEST_SKIP() << "Test requires linearColorAttachment";
    }

    features2.features.variableMultisampleRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    auto multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    GetPhysicalDeviceProperties2(multiview_props);

    VkFormat color_format = VK_FORMAT_D32_SFLOAT;

    auto cmd_buffer_inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    cmd_buffer_inheritance_rendering_info.colorAttachmentCount = 1;
    cmd_buffer_inheritance_rendering_info.pColorAttachmentFormats = &color_format;
    cmd_buffer_inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_R8G8B8_UNORM;
    cmd_buffer_inheritance_rendering_info.stencilAttachmentFormat = VK_FORMAT_R8G8B8_SNORM;
    cmd_buffer_inheritance_rendering_info.viewMask = 1 << multiview_props.maxMultiviewViewCount;

    auto sample_count_info_amd = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();
    sample_count_info_amd.pNext = &cmd_buffer_inheritance_rendering_info;
    sample_count_info_amd.colorAttachmentCount = 2;

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.pNext = &sample_count_info_amd;

    auto cmd_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 0x1;

    VkCommandBuffer secondary_cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06003");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008");
    if (multiview_props.maxMultiviewViewCount != 32) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06199");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06200");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-pColorAttachmentFormats-06492");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541");

    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(DynamicRenderingCommandBufferInheritanceRenderingInfoTest, Core) {
    TEST_DESCRIPTION("VkCommandBufferInheritanceRenderingInfoKHR Dynamic Rendering Tests.");

    Test(false);
}

TEST_F(DynamicRenderingCommandBufferInheritanceRenderingInfoTest, LinearColorAttachment) {
    TEST_DESCRIPTION("VkCommandBufferInheritanceRenderingInfoKHR Dynamic Rendering Tests with linearColorAttachment.");

    Test(true);
}

TEST_F(NegativeDynamicRendering, CommandDraw) {
    TEST_DESCRIPTION("vkCmdDraw* Dynamic Rendering Tests.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.SetDepthStencil(&ds_state);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    auto multisample_state_create_info = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pMultisampleState = &multisample_state_create_info;
    create_info.renderPass = VkRenderPass(0x1);
    create_info.pNext = &pipeline_rendering_info;

    VkResult err = pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    ASSERT_VK_SUCCESS(err);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depth_format,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    vk_testing::ImageView depth_image_view(*m_device, ivci);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, CommandDrawWithShaderTileImageRead) {
    TEST_DESCRIPTION("vkCmdDraw* with shader tile image read extension using dynamic Rendering Tests.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    auto shader_tile_image_features = LvlInitStruct<VkPhysicalDeviceShaderTileImageFeaturesEXT>();
    InitBasicDynamicRendering(&shader_tile_image_features);
    if (::testing::Test::IsSkipped()) return;

    if (!shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageDepthStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = 2;
    dyn_state_ci.pDynamicStates = dyn_states;

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_state.depthWriteEnable = VK_TRUE;

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});

    auto ms_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_ci.sampleShadingEnable = VK_TRUE;
    ms_ci.minSampleShading = 1.0;
    ms_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs->GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = &ms_ci;
    pipe.gp_ci_.pDepthStencilState = &ds_state;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&dsl});
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkImageObj depth_image(m_device);
    depth_image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(depth_image.initialized());

    VkImageViewCreateInfo depth_view_ci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                           nullptr,
                                           0,
                                           depth_image.handle(),
                                           VK_IMAGE_VIEW_TYPE_2D,
                                           depth_format,
                                           {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                           {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    vk_testing::ImageView depth_image_view(*m_device, depth_view_ci);

    VkImageObj color_image(m_device);
    color_image.Init(32, 32, 1, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(color_image.initialized());

    VkImageViewCreateInfo color_view_ci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                           nullptr,
                                           0,
                                           color_image.handle(),
                                           VK_IMAGE_VIEW_TYPE_2D,
                                           color_format,
                                           {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    vk_testing::ImageView color_image_view(*m_device, color_view_ci);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = color_image_view.handle();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdSetDepthWriteEnable(m_commandBuffer->handle(), true);
    vk::CmdSetStencilWriteMask(m_commandBuffer->handle(), VK_STENCIL_FACE_FRONT_BIT, 0xff);
    vk::CmdSetStencilWriteMask(m_commandBuffer->handle(), VK_STENCIL_FACE_BACK_BIT, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDynamicStates-08715");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDynamicStates-08716");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, CmdClearAttachmentTests) {
    TEST_DESCRIPTION("Various tests for validating usage of vkCmdClearAttachments with Dynamic Rendering");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageFormatProperties image_format_properties{};
    vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), m_renderTargets[0]->format(), VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, m_renderTargets[0]->usage(), 0, &image_format_properties);
    if (image_format_properties.maxArrayLayers < 4) {
        GTEST_SKIP() << "Test needs to create image 2D array of 4 image view, but VkImageFormatProperties::maxArrayLayers is < 4. "
                        "Skipping test.";
    }

    // render pass instance is going to have 2 layers, and image view 4 layers,
    // to make sure that considered layer count is the one coming from frame buffer
    // (test would not fail if layer count used to do validation was 4)
    VkImageObj render_target(m_device);
    assert(!m_renderTargets.empty());
    const auto render_target_ci = VkImageObj::ImageCreateInfo2D(
        m_renderTargets[0]->width(), m_renderTargets[0]->height(), m_renderTargets[0]->create_info().mipLevels, 4,
        m_renderTargets[0]->format(), m_renderTargets[0]->usage(), VK_IMAGE_TILING_OPTIMAL);
    render_target.Init(render_target_ci, 0);
    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = render_target.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = render_target_ci.format;
    ivci.subresourceRange.layerCount = render_target_ci.arrayLayers;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.components.r = VK_COMPONENT_SWIZZLE_R;
    ivci.components.g = VK_COMPONENT_SWIZZLE_G;
    ivci.components.b = VK_COMPONENT_SWIZZLE_B;
    ivci.components.a = VK_COMPONENT_SWIZZLE_A;
    vk_testing::ImageView render_target_view(*m_device, ivci);

    // Create secondary command buffer
    auto secondary_cmd_buffer_alloc_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    secondary_cmd_buffer_alloc_info.commandPool = m_commandPool->handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vk_testing::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);
    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &render_target_ci.format;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info =
        LvlInitStruct<VkCommandBufferInheritanceInfo>(&inheritance_rendering_info);

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &secondary_cmd_buffer_inheritance_info;

    // Create clear rect
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    auto clear_cmds = [this, &color_attachment](VkCommandBuffer cmd_buffer, VkClearRect clear_rect) {
        // extent too wide
        VkClearRect clear_rect_too_large = clear_rect;
        clear_rect_too_large.rect.extent.width = renderPassBeginInfo().renderArea.extent.width + 4;
        clear_rect_too_large.rect.extent.height = clear_rect_too_large.rect.extent.height / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00016");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect_too_large);

        // baseLayer < render pass instance layer count
        clear_rect.baseArrayLayer = 1;
        clear_rect.layerCount = 1;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount <= render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 2;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer >= render pass instance layer count
        clear_rect.baseArrayLayer = 2;
        clear_rect.layerCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount > render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 4;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);
    };

    // Register clear commands to secondary command buffer
    secondary_cmd_buffer.begin(&secondary_cmd_buffer_begin_info);
    clear_cmds(secondary_cmd_buffer.handle(), clear_rect);
    secondary_cmd_buffer.end();

    m_commandBuffer->begin();

    auto color_attachment_info = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.imageView = render_target_view.handle();
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 2;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment_info;

    // Execute secondary command buffer
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Execute same commands as previously, but in a primary command buffer
    begin_rendering_info.flags = 0;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    clear_cmds(m_commandBuffer->handle(), clear_rect);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ClearAttachments) {
    TEST_DESCRIPTION("Call CmdClearAttachments with invalid aspect masks.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    // Create color image
    const VkFormat color_format = VK_FORMAT_R32_SFLOAT;
    VkImageObj color_image(m_device);
    auto imci = LvlInitStruct<VkImageCreateInfo>();
    imci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imci.imageType = VK_IMAGE_TYPE_2D;
    imci.format = color_format;
    imci.extent = {32, 32, 1};
    imci.mipLevels = 1;
    imci.arrayLayers = 1;
    imci.samples = VK_SAMPLE_COUNT_1_BIT;
    imci.tiling = VK_IMAGE_TILING_OPTIMAL;
    imci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imci.queueFamilyIndexCount = 0;
    imci.pQueueFamilyIndices = nullptr;
    imci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_image.Init(imci);
    ASSERT_TRUE(color_image.initialized());

    // Create correct color image view
    VkImageViewCreateInfo color_ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                        nullptr,
                                        0,
                                        color_image.handle(),
                                        VK_IMAGE_VIEW_TYPE_2D,
                                        color_format,
                                        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    vk_testing::ImageView color_image_view(*m_device, color_ivci);

    // Create depth image
    const VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkImageObj depth_image(m_device);
    depth_image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR);
    ASSERT_TRUE(depth_image.initialized());

    // Create depth image view
    VkImageViewCreateInfo depth_stencil_ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                                nullptr,
                                                0,
                                                depth_image.handle(),
                                                VK_IMAGE_VIEW_TYPE_2D,
                                                depth_format,
                                                {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                                {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};
    vk_testing::ImageView depth_image_view(*m_device, depth_stencil_ivci);
    depth_stencil_ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vk_testing::ImageView stencil_image_view(*m_device, depth_stencil_ivci);
    depth_stencil_ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vk_testing::ImageView depth_stencil_image_view(*m_device, depth_stencil_ivci);

    // Dynamic rendering structs
    VkRect2D rect{{0, 0}, {32, 32}};
    auto depth_attachment_info = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
    depth_attachment_info.imageView = depth_stencil_image_view.handle();
    auto stencil_attachment_info = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment_info.imageView = depth_stencil_image_view.handle();
    auto color_attachment_info = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.imageView = color_image_view;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();

    begin_rendering_info.renderArea = rect;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment_info;
    begin_rendering_info.pStencilAttachment = &stencil_attachment_info;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment_info;
    begin_rendering_info.viewMask = 0;

    // Render pass structs
    std::array<VkAttachmentDescription, 2> attachments = {
        {{0, depth_stencil_ivci.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
          VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},

         {0, color_ivci.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
          VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}};

    std::array<VkAttachmentReference, 4> attachment_references = {{{0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
                                                                   {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                                   {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                                   {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}};

    std::array<VkSubpassDescription, 2> subpass_descs = {};
    subpass_descs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descs[0].colorAttachmentCount = 1;
    subpass_descs[0].pColorAttachments = &attachment_references[1];
    subpass_descs[0].pDepthStencilAttachment = &attachment_references[0];

    subpass_descs[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descs[1].colorAttachmentCount = 3;
    subpass_descs[1].pColorAttachments = &attachment_references[1];
    subpass_descs[1].pDepthStencilAttachment = &attachment_references[0];

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 1;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstStageMask = subpass_dependency.srcStageMask;
    subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags = 0;

    auto renderpass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    renderpass_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderpass_ci.pAttachments = attachments.data();
    renderpass_ci.subpassCount = static_cast<uint32_t>(subpass_descs.size());
    renderpass_ci.pSubpasses = subpass_descs.data();
    renderpass_ci.dependencyCount = 1;
    renderpass_ci.pDependencies = &subpass_dependency;
    vk_testing::RenderPass renderpass(*m_device, renderpass_ci);

    std::array<VkImageView, 2> renderpass_image_views = {depth_stencil_image_view.handle(), color_image_view.handle()};

    auto framebuffer_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_ci.renderPass = renderpass.handle();
    framebuffer_ci.attachmentCount = 2;
    framebuffer_ci.pAttachments = renderpass_image_views.data();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    auto renderpass_bi = LvlInitStruct<VkRenderPassBeginInfo>();
    renderpass_bi.renderPass = renderpass.handle();
    renderpass_bi.renderArea = rect;
    renderpass_bi.clearValueCount = 2;
    std::array<VkClearValue, 2> renderpass_clear_values;
    renderpass_clear_values[0].depthStencil.depth = 1.0f;
    std::fill(&renderpass_clear_values[0].color.float32[0], &renderpass_clear_values[0].color.float32[0] + 4, 0.0f);
    renderpass_bi.pClearValues = renderpass_clear_values.data();

    auto clear_cmd_test = [&](const bool use_dynamic_rendering) {
        std::array<VkFramebuffer, 4> framebuffers = {VK_NULL_HANDLE};

        m_commandBuffer->begin();

        // Try to clear stencil, but image view does not have stencil aspect
        // This is a valid clear because the ImageView aspect are ignored
        // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5733#note_398961
        {
            if (use_dynamic_rendering) {
                depth_attachment_info.imageView = depth_image_view.handle();
                stencil_attachment_info.imageView = depth_image_view.handle();

                m_commandBuffer->BeginRendering(begin_rendering_info);
            } else {
                renderpass_image_views[0] = depth_image_view.handle();

                const VkResult err = vk::CreateFramebuffer(m_device->handle(), &framebuffer_ci, nullptr, &framebuffers[0]);
                ASSERT_VK_SUCCESS(err);
                renderpass_bi.framebuffer = framebuffers[0];
                m_commandBuffer->BeginRenderPass(renderpass_bi);
            }

            VkClearAttachment clear_stencil_attachment;
            clear_stencil_attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            clear_stencil_attachment.clearValue.depthStencil.depth = 1.0f;
            clear_stencil_attachment.clearValue.depthStencil.stencil = 0;
            VkClearRect clear_rect{rect, 0, 1};
            vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_stencil_attachment, 1, &clear_rect);

            if (use_dynamic_rendering) {
                m_commandBuffer->EndRendering();

                depth_attachment_info.imageView = depth_stencil_image_view.handle();
                stencil_attachment_info.imageView = depth_stencil_image_view.handle();
            } else {
                m_commandBuffer->NextSubpass();
                m_commandBuffer->EndRenderPass();

                renderpass_image_views[0] = depth_stencil_image_view.handle();
            }
        }

        // Try to clear depth, but image view does not have depth aspect (valid, see stencil above)
        {
            if (use_dynamic_rendering) {
                depth_attachment_info.imageView = stencil_image_view.handle();
                stencil_attachment_info.imageView = stencil_image_view.handle();

                m_commandBuffer->BeginRendering(begin_rendering_info);
            } else {
                renderpass_image_views[0] = stencil_image_view.handle();

                const VkResult err = vk::CreateFramebuffer(m_device->handle(), &framebuffer_ci, nullptr, &framebuffers[1]);
                ASSERT_VK_SUCCESS(err);
                renderpass_bi.framebuffer = framebuffers[1];
                m_commandBuffer->BeginRenderPass(renderpass_bi);
            }

            VkClearAttachment clear_depth_attachment;
            clear_depth_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            clear_depth_attachment.clearValue.depthStencil.depth = 1.0f;
            VkClearRect clear_rect{rect, 0, 1};
            vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_depth_attachment, 1, &clear_rect);

            if (use_dynamic_rendering) {
                m_commandBuffer->EndRendering();

                depth_attachment_info.imageView = depth_stencil_image_view.handle();
                stencil_attachment_info.imageView = depth_stencil_image_view.handle();
            } else {
                m_commandBuffer->NextSubpass();
                m_commandBuffer->EndRenderPass();

                renderpass_image_views[0] = depth_stencil_image_view.handle();
            }
        }

        {
            if (!use_dynamic_rendering) {
                const VkResult err = vk::CreateFramebuffer(m_device->handle(), &framebuffer_ci, nullptr, &framebuffers[2]);
                ASSERT_VK_SUCCESS(err);
                renderpass_bi.framebuffer = framebuffers[2];
            }

            // Try to clear color, but aspect also has depth
            {
                // begin rendering
                if (use_dynamic_rendering) {
                    m_commandBuffer->BeginRendering(begin_rendering_info);
                } else {
                    m_commandBuffer->BeginRenderPass(renderpass_bi);
                }

                // issue clear cmd
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearAttachment-aspectMask-00019");
                VkClearAttachment clear_depth_attachment;
                clear_depth_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
                clear_depth_attachment.colorAttachment = 0;
                VkClearRect clear_rect{rect, 0, 1};
                vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_depth_attachment, 1, &clear_rect);
                m_errorMonitor->VerifyFound();

                // end rendering
                if (use_dynamic_rendering) {
                    m_commandBuffer->EndRendering();
                } else {
                    m_commandBuffer->NextSubpass();
                    m_commandBuffer->EndRenderPass();
                }
            }

            // Try to clear color, but color attachment is out of range
            {
                // begin rendering
                if (use_dynamic_rendering) {
                    m_commandBuffer->BeginRendering(begin_rendering_info);
                } else {
                    m_commandBuffer->BeginRenderPass(renderpass_bi);
                }

                // issue clear cmd
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07271");
                VkClearAttachment clear_depth_attachment;
                clear_depth_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                clear_depth_attachment.colorAttachment = 2;
                VkClearRect clear_rect{rect, 0, 1};
                vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_depth_attachment, 1, &clear_rect);
                m_errorMonitor->VerifyFound();

                // end rendering
                if (use_dynamic_rendering) {
                    m_commandBuffer->EndRendering();
                } else {
                    m_commandBuffer->NextSubpass();
                    m_commandBuffer->EndRenderPass();
                }
            }

            // Clear color, subpass has unused attachments
            if (!use_dynamic_rendering) {
                m_commandBuffer->BeginRenderPass(renderpass_bi);
                m_commandBuffer->NextSubpass();
                std::array<VkClearAttachment, 4> clears = {{{VK_IMAGE_ASPECT_DEPTH_BIT, 0},
                                                            {VK_IMAGE_ASPECT_COLOR_BIT, 0},
                                                            {VK_IMAGE_ASPECT_COLOR_BIT, 1},
                                                            {VK_IMAGE_ASPECT_COLOR_BIT, 2}}};
                VkClearRect clear_rect{rect, 0, 1};
                vk::CmdClearAttachments(m_commandBuffer->handle(), static_cast<uint32_t>(clears.size()), clears.data(), 1,
                                        &clear_rect);
                m_commandBuffer->EndRenderPass();
            }
        }

        m_commandBuffer->end();

        {
            delete m_commandBuffer;
            m_commandBuffer = new VkCommandBufferObj(m_device, m_commandPool);

            std::unique_ptr<VkCommandBufferObj> secondary_cmd_buffer(
                new VkCommandBufferObj(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY));

            auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfo>();
            const VkFormat color_format = VK_FORMAT_R32_SFLOAT;
            inheritance_rendering_info.colorAttachmentCount = begin_rendering_info.colorAttachmentCount;
            inheritance_rendering_info.pColorAttachmentFormats = &color_format;
            inheritance_rendering_info.depthAttachmentFormat = depth_format;
            inheritance_rendering_info.stencilAttachmentFormat = depth_format;
            inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
            cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
            cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;
            if (use_dynamic_rendering) {
                cmd_buffer_inheritance_info.pNext = &inheritance_rendering_info;
            } else {
                const VkResult err = vk::CreateFramebuffer(m_device->handle(), &framebuffer_ci, nullptr, &framebuffers[3]);
                ASSERT_VK_SUCCESS(err);
                renderpass_bi.framebuffer = framebuffers[3];
                cmd_buffer_inheritance_info.renderPass = renderpass.handle();
                cmd_buffer_inheritance_info.subpass = 0;
                cmd_buffer_inheritance_info.framebuffer = framebuffers[3];
            }

            secondary_cmd_buffer->begin(&cmd_buffer_begin_info);
            // issue clear cmd to secondary cmd buffer
            std::array<VkClearAttachment, 3> clear_attachments = {};
            clear_attachments[0].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            clear_attachments[0].clearValue.depthStencil.depth = 1.0f;
            clear_attachments[1].aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            clear_attachments[1].clearValue.depthStencil.depth = 1.0f;
            clear_attachments[2].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clear_attachments[2].colorAttachment = 0;
            VkClearRect clear_rect{rect, 0, 1};
            // Expected to succeeed
            vk::CmdClearAttachments(secondary_cmd_buffer->handle(), static_cast<uint32_t>(clear_attachments.size()),
                                    clear_attachments.data(), 1, &clear_rect);

            // Clear color out of range
            VkClearAttachment clear_color_out_of_range{VK_IMAGE_ASPECT_COLOR_BIT, 1, VkClearValue{}};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-aspectMask-07271");
            vk::CmdClearAttachments(secondary_cmd_buffer->handle(), 1, &clear_color_out_of_range, 1, &clear_rect);
            m_errorMonitor->VerifyFound();
            secondary_cmd_buffer->end();

            m_commandBuffer->begin();

            // begin rendering
            if (use_dynamic_rendering) {
                begin_rendering_info.flags |= VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
                m_commandBuffer->BeginRendering(begin_rendering_info);
                begin_rendering_info.flags &= ~VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
            } else {
                m_commandBuffer->BeginRenderPass(renderpass_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            }

            vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmd_buffer->handle());

            // end rendering
            if (use_dynamic_rendering) {
                m_commandBuffer->EndRendering();
            } else {
                m_commandBuffer->NextSubpass();
                m_commandBuffer->EndRenderPass();
            }

            m_commandBuffer->end();
        }

        for (auto framebuffer : framebuffers) {
            vk::DestroyFramebuffer(m_device->handle(), framebuffer, nullptr);
        }
    };

    clear_cmd_test(true);

    delete m_commandBuffer;
    m_commandBuffer = new VkCommandBufferObj(m_device, m_commandPool);
    clear_cmd_test(false);
}

TEST_F(NegativeDynamicRendering, GraphicsPipelineCreateInfo) {
    TEST_DESCRIPTION("Test graphics pipeline creation with dynamic rendering.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    if (m_device->props.limits.maxGeometryOutputVertices == 0) {
        GTEST_SKIP() << "Device doesn't support required maxGeometryOutputVertices";
    }

    const VkPipelineLayoutObj pl(m_device);
    VkPipelineObj pipe(m_device);

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};

    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    VkFormat color_format[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT};

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format[0];
    pipeline_rendering_info.viewMask = 0x2;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    auto pipeline_tessellation_state_info = LvlInitStruct<VkPipelineTessellationStateCreateInfo>();
    pipeline_tessellation_state_info.patchControlPoints = 1;

    auto pipeline_input_assembly_state_info = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    pipeline_input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, kGeometryMinimalGlsl, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj te(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj tc(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    pipe.AddShader(&vs);
    pipe.AddShader(&gs);
    pipe.AddShader(&te);
    pipe.AddShader(&tc);
    pipe.AddShader(&fs);
    pipe.InitGraphicsPipelineCreateInfo(&create_info);

    create_info.pColorBlendState = &color_blend_state_create_info;
    create_info.pNext = &pipeline_rendering_info;
    create_info.pTessellationState = &pipeline_tessellation_state_info;
    create_info.pInputAssemblyState = &pipeline_input_assembly_state_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-09033");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06057");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06058");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    create_info.pColorBlendState = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.viewMask = 0x0;
    pipeline_rendering_info.colorAttachmentCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-09037");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    color_format[0] = VK_FORMAT_D32_SFLOAT_S8_UINT;
    color_blend_attachment_state.blendEnable = VK_TRUE;
    create_info.pColorBlendState = &color_blend_state_create_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06062");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
    color_format[0] = VK_FORMAT_R8G8B8A8_UNORM;

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;
    color_blend_state_create_info.flags = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;
    create_info.pColorBlendState = &color_blend_state_create_info;
    create_info.pDepthStencilState = &ds_ci;
    create_info.renderPass = VK_NULL_HANDLE;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06482");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06483");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, MismatchingViewMask) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering and a mismatching viewMask");
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multview";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.viewMask = 1;

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.viewMask = 2;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewMask-06178");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    const VkViewport viewport = {0, 0, 16, 16, 0, 1};
    const VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);
    pipe2.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    bool testStencil = false;
    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

    if (ImageFormatIsSupported(gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D16_UNORM_S8_UINT) &&
               ImageFormatIsSupported(gpu(), VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D16_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D24_UNORM_S8_UINT) &&
               ImageFormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D32_SFLOAT_S8_UINT) &&
               ImageFormatIsSupported(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        testStencil = true;
    }

    VkPipelineObj pipe3(m_device);

    if (testStencil) {
        pipe3.AddShader(&vs);
        pipe3.AddShader(&fs);
        pipe3.AddDefaultColorAttachment();
        pipe3.SetViewport(m_viewports);
        pipe3.SetScissor(m_scissors);
        pipe3.SetDepthStencil(&ds_state);

        pipeline_rendering_info.colorAttachmentCount = 0;
        pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipeline_rendering_info.stencilAttachmentFormat = stencilFormat;

        auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
        pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
        create_info3.pNext = &pipeline_rendering_info;

        pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);
    }

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    auto depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->begin();

    // Mismatching color attachment count
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06179");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching color formats
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08910");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth format
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil format
    if (testStencil) {
        begin_rendering_info.pDepthAttachment = nullptr;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08917");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats2) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with attachment specified as VK_NULL_HANDLE in VkRenderingInfoKHR, but with corresponding "
        "format in VkPipelineRenderingCreateInfoKHR not set to VK_FORMAT_UNDEFINED");

    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    const VkViewport viewport = {0, 0, 16, 16, 0, 1};
    const VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    VkPipelineObj pipeline_color(m_device);
    pipeline_color.AddShader(&vs);
    pipeline_color.AddShader(&fs);
    pipeline_color.AddDefaultColorAttachment();
    pipeline_color.SetViewport(m_viewports);
    pipeline_color.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipeline_color.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    pipeline_color.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipeline_depth(m_device);
    pipeline_depth.AddShader(&vs);
    pipeline_depth.AddShader(&fs);
    pipeline_depth.AddDefaultColorAttachment();
    pipeline_depth.SetViewport(m_viewports);
    pipeline_depth.SetScissor(m_scissors);
    pipeline_depth.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipeline_depth.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    pipeline_depth.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    const VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    VkPipelineObj pipeline_stencil(m_device);
    pipeline_stencil.AddShader(&vs);
    pipeline_stencil.AddShader(&fs);
    pipeline_stencil.AddDefaultColorAttachment();
    pipeline_stencil.SetViewport(m_viewports);
    pipeline_stencil.SetScissor(m_scissors);
    pipeline_stencil.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipeline_stencil.InitGraphicsPipelineCreateInfo(&create_info3);
    create_info3.pNext = &pipeline_rendering_info;

    pipeline_stencil.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    auto depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = VK_NULL_HANDLE;

    m_commandBuffer->begin();

    {
        // Mismatching color formats
        auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08912");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    {
        // Mismatching depth format
        auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    {
        // Mismatching stencil format
        auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats3) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats where "
        "dynamicRenderingUnusedAttachments is enabled and neither format is VK_FORMAT_UNDEFINED");
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    auto dynamic_rendering_unused_attachments_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT>();
    InitBasicDynamicRendering(&dynamic_rendering_unused_attachments_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_unused_attachments_features.dynamicRenderingUnusedAttachments) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRenderingUnusedAttachments , skipping.";
    }

    const VkViewport viewport = {0, 0, 16, 16, 0, 1};
    const VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_B8G8R8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);
    pipe2.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    bool testStencil = false;
    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

    if (ImageFormatIsSupported(gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D16_UNORM_S8_UINT) &&
        ImageFormatIsSupported(gpu(), VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D16_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D24_UNORM_S8_UINT) &&
        ImageFormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D32_SFLOAT_S8_UINT) &&
        ImageFormatIsSupported(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        testStencil = true;
    }

    VkPipelineObj pipe3(m_device);

    if (testStencil) {
        pipe3.AddShader(&vs);
        pipe3.AddShader(&fs);
        pipe3.AddDefaultColorAttachment();
        pipe3.SetViewport(m_viewports);
        pipe3.SetScissor(m_scissors);
        pipe3.SetDepthStencil(&ds_state);

        pipeline_rendering_info.colorAttachmentCount = 0;
        pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipeline_rendering_info.stencilAttachmentFormat = stencilFormat;

        auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
        pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
        create_info3.pNext = &pipeline_rendering_info;

        pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);
    }

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    auto depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->begin();

    // Mismatching color formats
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08911");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth format
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08915");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil format
    if (testStencil) {
        begin_rendering_info.pDepthAttachment = nullptr;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08918");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentSamples) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color/depth/stencil sample counts");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    auto multisample_state_create_info = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    create_info1.pMultisampleState = &multisample_state_create_info;

    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);
    pipe2.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    create_info2.pMultisampleState = &multisample_state_create_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkPipelineObj pipe3(m_device);

    pipe3.AddShader(&vs);
    pipe3.AddShader(&fs);
    pipe3.AddDefaultColorAttachment();
    pipe3.SetViewport(m_viewports);
    pipe3.SetScissor(m_scissors);
    pipe3.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
    create_info3.pNext = &pipeline_rendering_info;

    create_info3.pMultisampleState = &multisample_state_create_info;

    pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->begin();

    // Mismatching color samples
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth samples
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil samples
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MismatchingMixedAttachmentSamples) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching mixed color/depth/stencil sample counts");
    AddOptionalExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    const bool amd_samples = IsExtensionsEnabled(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    const bool nv_samples = IsExtensionsEnabled(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    if (!amd_samples && !nv_samples) {
        GTEST_SKIP() << "Test requires either VK_AMD_mixed_attachment_samples or VK_NV_framebuffer_mixed_samples";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkSampleCountFlagBits counts[2] = {VK_SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_2_BIT};
    auto samples_info = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>(&samples_info);

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = counts;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    samples_info.colorAttachmentCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06063");
    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);
    m_errorMonitor->VerifyFound();

    samples_info.colorAttachmentCount = 1;
    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);
    pipe2.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    samples_info.colorAttachmentCount = 0;
    samples_info.pColorAttachmentSamples = nullptr;
    samples_info.depthStencilAttachmentSamples = counts[0];

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkPipelineObj pipe3(m_device);

    pipe3.AddShader(&vs);
    pipe3.AddShader(&fs);
    pipe3.AddDefaultColorAttachment();
    pipe3.SetViewport(m_viewports);
    pipe3.SetScissor(m_scissors);
    pipe3.SetDepthStencil(&ds_state);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
    create_info3.pNext = &pipeline_rendering_info;

    pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    m_commandBuffer->begin();

    // Mismatching color samples
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06185");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth samples
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-06186");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil samples
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-06187");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, AttachmentInfo) {
    TEST_DESCRIPTION("AttachmentInfo Dynamic Rendering Tests.");
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    auto fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    InitBasicDynamicRendering(&fdm_features);
    if (::testing::Test::IsSkipped()) return;

    if (!fdm_features.fragmentDensityMapNonSubsampledImages) {
        GTEST_SKIP() << "fragmentDensityMapNonSubsampledImages not supported.";
    }

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto ds_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.SetDepthStencil(&ds_state);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;

    auto pipeline_create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&pipeline_create_info);
    pipeline_create_info.pNext = &pipeline_rendering_info;

    VkResult err = pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &pipeline_create_info);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    VkImageObj image_fragment(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    image.Init(image_create_info);
    image_fragment.Init(image_create_info);
    ASSERT_TRUE(image.initialized());
    ASSERT_TRUE(image_fragment.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depth_format,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    VkImageView depth_image_view = image.targetView(ivci);
    VkImageView depth_image_view_fragment = image_fragment.targetView(ivci);
    ASSERT_NE(depth_image_view, VK_NULL_HANDLE);
    ASSERT_NE(depth_image_view_fragment, VK_NULL_HANDLE);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    depth_attachment.imageView = depth_image_view;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.viewMask = 0x4;

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map =
        LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    fragment_density_map.imageView = depth_image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    begin_rendering_info.pNext = &fragment_density_map;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06116");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    fragment_density_map.imageView = depth_image_view_fragment;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-multiview-06127");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06108");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06145");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06861");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06862");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, BufferBeginInfoLegacy) {
    TEST_DESCRIPTION("VkCommandBufferBeginInfo Dynamic Rendering Tests.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto cmd_buffer_inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    cmd_buffer_inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.pNext = &cmd_buffer_inheritance_rendering_info;
    cmd_buffer_inheritance_info.renderPass = VK_NULL_HANDLE;

    auto cmd_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 0x1;

    VkCommandBuffer secondary_cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    // Invalid RenderPass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06000");
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();

    // Valid RenderPass
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, &subpass, 0, nullptr};

    vk_testing::RenderPass rp1;
    rp1.init(*m_device, rpci);

    cmd_buffer_inheritance_info.renderPass = rp1.handle();
    cmd_buffer_inheritance_info.subpass = 0x5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06001");
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, SecondaryCommandBuffer) {
    TEST_DESCRIPTION("VkCommandBufferBeginInfo Dynamic Rendering Tests.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Force the failure by not setting the Renderpass and Framebuffer fields
    VkCommandBufferInheritanceInfo cmd_buf_hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buf_hinfo.renderPass = VkRenderPass(0x1);
    VkCommandBufferBeginInfo cmd_buf_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06000");
    vk::BeginCommandBuffer(cb.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    // Valid RenderPass
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, &subpass, 0, nullptr};

    vk_testing::RenderPass rp1;
    rp1.init(*m_device, rpci);

    cmd_buf_hinfo.renderPass = rp1.handle();
    cmd_buf_hinfo.subpass = 0x5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06001");
    vk::BeginCommandBuffer(cb.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06002");
    vk::BeginCommandBuffer(cb.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, PipelineMissingFlags) {
    TEST_DESCRIPTION("Test dynamic rendering with pipeline missing flags.");

    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    bool fragment_density = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    bool shading_rate = IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    // Mostly likely will only find support for this on a custom profiles
    if (!ImageFormatAndFeaturesSupported(gpu(), depthStencilFormat, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        shading_rate = false;
    }
    if (!ImageFormatAndFeaturesSupported(gpu(), depthStencilFormat, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        fragment_density = false;
    }
    if (!fragment_density && !shading_rate) {
        GTEST_SKIP() << "shading rate / fragment shading not supported";
    }

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depthStencilFormat;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (shading_rate) {
        image_create_info.usage |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }
    if (fragment_density) {
        image_create_info.usage |= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    }

    VkImageFormatProperties imageFormatProperties;
    if (vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_create_info.format, image_create_info.imageType,
                                                   image_create_info.tiling, image_create_info.usage, image_create_info.flags,
                                                   &imageFormatProperties) == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "Format not supported";
    }

    image.Init(image_create_info);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depthStencilFormat,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    vk_testing::ImageView depth_image_view;
    depth_image_view.init(*m_device, ivci);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (shading_rate) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-imageView-06183");
        VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate =
            LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
        fragment_shading_rate.imageView = depth_image_view.handle();
        fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

        VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;

        const VkFormat color_format = VK_FORMAT_UNDEFINED;

        auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        m_errorMonitor->VerifyFound();
    }

    if (fragment_density) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-imageView-06184");
        VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map =
            LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
        fragment_density_map.imageView = depth_image_view.handle();
        fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_density_map);
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;

        const VkFormat color_format = VK_FORMAT_UNDEFINED;

        auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicRendering, LayerCount) {
    TEST_DESCRIPTION("Test dynamic rendering with viewMask 0 and invalid layer count.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-viewMask-06069");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, InfoMismatchedSamples) {
    TEST_DESCRIPTION("Test beginning rendering with mismatched sample counts.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj color_image(m_device);
    color_image.init(&image_ci);

    VkImageViewCreateInfo civ_ci = LvlInitStruct<VkImageViewCreateInfo>();
    civ_ci.image = color_image.handle();
    civ_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    civ_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    civ_ci.subresourceRange.layerCount = 1;
    civ_ci.subresourceRange.baseMipLevel = 0;
    civ_ci.subresourceRange.levelCount = 1;
    civ_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vk_testing::ImageView color_image_view;
    color_image_view.init(*m_device, civ_ci);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = color_image_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkImageObj depth_image(m_device);
    depth_image.Init(64, 64, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkImageViewCreateInfo div_ci = LvlInitStruct<VkImageViewCreateInfo>();
    div_ci.image = depth_image.handle();
    div_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    div_ci.format = depth_format;
    div_ci.subresourceRange.layerCount = 1;
    div_ci.subresourceRange.baseMipLevel = 0;
    div_ci.subresourceRange.levelCount = 1;
    div_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vk_testing::ImageView depth_image_view;
    depth_image_view.init(*m_device, div_ci);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-multisampledRenderToSingleSampled-06857");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingFragmentShadingRate) {
    TEST_DESCRIPTION("Test BeginRenderingInfo with FragmentShadingRateAttachment.");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!ImageFormatAndFeaturesSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "format doesn't support FRAGMENT_SHADING_RATE_ATTACHMENT_BIT";
    }

    auto image_ci = LvlInitStruct<VkImageCreateInfo>(nullptr);
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 2;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

    VkImageFormatProperties imageFormatProperties;
    if (vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_ci.format, image_ci.imageType, image_ci.tiling, image_ci.usage,
                                                   image_ci.flags, &imageFormatProperties) == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "Format not supported";
    }

    VkImageObj image(m_device);
    image.init(&image_ci);
    VkImageView image_view =
        image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 4;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06123");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.viewMask = 0xF;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06124");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    begin_rendering_info.layerCount = 2;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.viewMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06125");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, DeviceGroupRenderPassBeginInfo) {
    TEST_DESCRIPTION("Test render area of DeviceGroupRenderPassBeginInfo.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRect2D render_area = {};
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent.width = 32;
    render_area.extent.height = 32;

    auto device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &render_area;

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = colorImageView;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&device_group_render_pass_begin_info);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->EndRendering();

    render_area.offset.x = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06083");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    render_area.offset.x = 0;
    render_area.offset.y = 16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06084");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingFragmentShadingRateImage) {
    TEST_DESCRIPTION("Test BeginRendering with FragmentShadingRateAttachmentInfo with missing image usage bit.");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Skipping on AMD proprietary driver pending further investigation.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL);
    // TODO - Look into failure
    if (!image.initialized()) {
        GTEST_SKIP() << "Failed to create image";
    }
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj invalid_image(m_device);
    invalid_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView invalid_image_view = invalid_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = invalid_image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06148");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    fragment_shading_rate.imageView = image_view;

    fragment_shading_rate.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149");
    if (fragment_shading_rate.shadingRateAttachmentTexelSize.width >
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.width) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150");
    }
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (fsr_properties.minFragmentShadingRateAttachmentTexelSize.width > 1) {
        fragment_shading_rate.shadingRateAttachmentTexelSize.width =
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.width / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151");
        if (fragment_shading_rate.shadingRateAttachmentTexelSize.height /
                fragment_shading_rate.shadingRateAttachmentTexelSize.width >=
            fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156");
        }
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    fragment_shading_rate.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    fragment_shading_rate.shadingRateAttachmentTexelSize.height =
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152");
    if (fragment_shading_rate.shadingRateAttachmentTexelSize.height >
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153");
    }
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (fsr_properties.minFragmentShadingRateAttachmentTexelSize.height > 1) {
        fragment_shading_rate.shadingRateAttachmentTexelSize.height =
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.height / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154");
        if (fragment_shading_rate.shadingRateAttachmentTexelSize.width /
                fragment_shading_rate.shadingRateAttachmentTexelSize.height >
            fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155");
        }
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingDepthAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a depth attachment that has an invalid format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkFormat stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(stencil_format, VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06547");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, TestFragmentDensityMapRenderArea) {
    TEST_DESCRIPTION("Validate VkRenderingFragmentDensityMapAttachmentInfo attachment image view extent.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    auto fdm_props = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapPropertiesEXT>();
    GetPhysicalDeviceProperties2(fdm_props);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    auto fragment_density_map = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    fragment_density_map.imageView = image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_density_map);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.extent.width = 64 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 32 * fdm_props.maxFragmentDensityTexelSize.height;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06112");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 1;
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width) - 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07815");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06112");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.x);
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width);
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07815");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06112");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.extent.width = 32 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06114");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 1;
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height) - 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07816");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06114");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.y);
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height);
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07816");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06114");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    VkRect2D device_render_area = {};
    device_render_area.extent.width = 64 * fdm_props.maxFragmentDensityTexelSize.width;
    device_render_area.extent.height = 32 * fdm_props.maxFragmentDensityTexelSize.height;
    auto device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &device_render_area;
    fragment_density_map.pNext = &device_group_render_pass_begin_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06113");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    device_render_area.extent.width = 32 * fdm_props.maxFragmentDensityTexelSize.width;
    device_render_area.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06115");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, FragmentDensityMapRenderAreaWithoutDeviceGroupExt) {
    TEST_DESCRIPTION("Validate VkRenderingFragmentDensityMapAttachmentInfo attachment image view extent.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto fdm_props = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapPropertiesEXT>();
    GetPhysicalDeviceProperties2(fdm_props);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    auto fragment_density_map = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    fragment_density_map.imageView = image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_density_map);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.extent.width = 64 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 32 * fdm_props.maxFragmentDensityTexelSize.height;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06112");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.extent.width = 32 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06114");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, WithBarrier) {
    TEST_DESCRIPTION("Test setting buffer memory barrier when dynamic rendering is active.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>();
    InitBasicDynamicRendering(&sync2_features);
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->BeginRendering(begin_rendering_info);

    VkBufferObj buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    auto buf_barrier = LvlInitStruct<VkBufferMemoryBarrier2KHR>();
    buf_barrier.buffer = buffer.handle();
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    auto dependency_info = LvlInitStruct<VkDependencyInfoKHR>();
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buf_barrier;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-shaderTileImageColorReadAccess-08718");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-shaderTileImageColorReadAccess-08718");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, WithoutShaderTileImageAndBarrier) {
    TEST_DESCRIPTION("Test setting memory barrier if the shader tile image features are not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto vk13features = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    auto shader_tile_image_features = LvlInitStruct<VkPhysicalDeviceShaderTileImageFeaturesEXT>();
    vk13features.pNext = &shader_tile_image_features;

    auto features2 = GetPhysicalDeviceFeatures2(vk13features);
    if (!vk13features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    shader_tile_image_features.shaderTileImageColorReadAccess = false;
    shader_tile_image_features.shaderTileImageDepthReadAccess = false;
    shader_tile_image_features.shaderTileImageStencilReadAccess = false;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->BeginRendering(begin_rendering_info);

    auto memory_barrier_2 = LvlInitStruct<VkMemoryBarrier2KHR>();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    auto dependency_info = LvlInitStruct<VkDependencyInfoKHR>();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.pBufferMemoryBarriers = VK_NULL_HANDLE;
    dependency_info.imageMemoryBarrierCount = 0;
    dependency_info.pImageMemoryBarriers = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-shaderTileImageColorReadAccess-08718");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    auto memory_barrier = LvlInitStruct<VkMemoryBarrier>();
    memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-shaderTileImageColorReadAccess-08718");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &memory_barrier, 0,
                           nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, WithShaderTileImageAndBarrier) {
    TEST_DESCRIPTION("Test setting memory barrier if the shader tile image features are enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto vk13features = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    auto shader_tile_image_features = LvlInitStruct<VkPhysicalDeviceShaderTileImageFeaturesEXT>();
    vk13features.pNext = &shader_tile_image_features;

    auto features2 = GetPhysicalDeviceFeatures2(vk13features);
    if (!vk13features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->BeginRendering(begin_rendering_info);

    auto memory_barrier_2 = LvlInitStruct<VkMemoryBarrier2KHR>();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    auto dependency_info = LvlInitStruct<VkDependencyInfoKHR>();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.pBufferMemoryBarriers = VK_NULL_HANDLE;
    dependency_info.imageMemoryBarrierCount = 0;
    dependency_info.pImageMemoryBarriers = VK_NULL_HANDLE;

    dependency_info.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-08719");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    VkBufferObj buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    auto buf_barrier_2 = LvlInitStruct<VkBufferMemoryBarrier2KHR>();
    buf_barrier_2.buffer = buffer.handle();
    buf_barrier_2.offset = 0;
    buf_barrier_2.size = VK_WHOLE_SIZE;
    buf_barrier_2.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    buf_barrier_2.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buf_barrier_2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-08719");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.pBufferMemoryBarriers = VK_NULL_HANDLE;
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03911");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03910");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    memory_barrier_2.srcAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03903");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03903");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-08719");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, nullptr, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    VkBufferMemoryBarrier buf_barrier = LvlInitStruct<VkBufferMemoryBarrier>();
    buf_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-02815");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-02816");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buf_barrier,
                           0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-08719");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-08719");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    auto memory_barrier = LvlInitStruct<VkMemoryBarrier>();
    memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-02815");
    memory_barrier.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &memory_barrier, 0,
                           nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-08719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-02816 ");
    memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &memory_barrier, 0,
                           nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingStencilAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a stencil attachment that has an invalid format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfoKHR stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06548");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, InheritanceRenderingInfoStencilAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a stencil attachment that has an invalid format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto cmd_buffer_inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    cmd_buffer_inheritance_rendering_info.colorAttachmentCount = 1;
    cmd_buffer_inheritance_rendering_info.pColorAttachmentFormats = &color_format;
    cmd_buffer_inheritance_rendering_info.depthAttachmentFormat = depth_format;
    cmd_buffer_inheritance_rendering_info.stencilAttachmentFormat = depth_format;
    cmd_buffer_inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.pNext = &cmd_buffer_inheritance_rendering_info;
    cmd_buffer_inheritance_info.renderPass = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    auto cmd_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_cmd_buffer;
    vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541");
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, CreateGraphicsPipelineWithAttachmentSampleCount) {
    TEST_DESCRIPTION("Create pipeline with fragment shader that uses samples, but multisample state not begin set");
    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto sample_count_info_amd = LvlInitStruct<VkAttachmentSampleCountInfoNV>();
    sample_count_info_amd.colorAttachmentCount = 1;
    sample_count_info_amd.depthStencilAttachmentSamples = static_cast<VkSampleCountFlagBits>(0x3);

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>(&sample_count_info_amd);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-depthStencilAttachmentSamples-06593");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, CreatePipelineWithoutFeature) {
    TEST_DESCRIPTION("Create graphcis pipeline that uses dynamic rendering, but feature is not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-dynamicRendering-06576");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, AreaGreaterThanAttachmentExtent) {
    TEST_DESCRIPTION("Begin dynamic rendering with render area greater than extent of attachments");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea.extent.width = 64;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 1;
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width) - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07815");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.x);
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07815");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 64;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 1;
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height) - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07816");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.y);
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07816");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if ((ds_format != VK_FORMAT_UNDEFINED) && IsExtensionsEnabled(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME)) {
        VkImageObj depthImage(m_device);
        depthImage.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkImageView depthImageView = depthImage.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

        VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
        depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment.imageView = depthImageView;

        begin_rendering_info.colorAttachmentCount = 0;
        begin_rendering_info.pDepthAttachment = &depth_attachment;
        begin_rendering_info.renderArea.offset.y = 0;
        begin_rendering_info.renderArea.extent.height = 64;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, DeviceGroupAreaGreaterThanAttachmentExtent) {
    TEST_DESCRIPTION("Begin dynamic rendering with device group with render area greater than extent of attachments");
    AddOptionalExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea.extent.width = 64;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 1;
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width) - 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07815");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.x);
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width);
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07815");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 64;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 1;
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height) - 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07816");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.y);
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height);
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07816");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if ((ds_format != VK_FORMAT_UNDEFINED) && IsExtensionsEnabled(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME)) {
        VkImageObj depthImage(m_device);
        depthImage.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkImageView depthImageView = depthImage.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

        VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
        depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment.imageView = depthImageView;

        begin_rendering_info.colorAttachmentCount = 0;
        begin_rendering_info.pDepthAttachment = &depth_attachment;
        begin_rendering_info.renderArea.offset.y = 0;
        begin_rendering_info.renderArea.extent.height = 64;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, SecondaryCommandBufferIncompatibleRenderPass) {
    TEST_DESCRIPTION("Execute secondary command buffers within render pass instance with incompatible render pass");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSubpassDescription subpass = {};
    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, render_pass_ci);

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.renderPass = render_pass.handle();
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, SecondaryCommandBufferIncompatibleSubpass) {
    TEST_DESCRIPTION("Execute secondary command buffers with different subpass");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[2] = {};

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, render_pass_ci);

    auto framebuffer_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, framebuffer_ci);

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.renderPass = render_pass.handle();
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    auto render_pass_begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.renderArea.extent = {32, 32};
    render_pass_begin_info.framebuffer = framebuffer.handle();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, SecondaryCommandBufferContents) {
    TEST_DESCRIPTION("Execute secondary command buffers within active render pass that was not begun with VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.renderPass = m_renderPass;
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-contents-06018");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ShaderLayerBuiltIn) {
    TEST_DESCRIPTION("Create invalid pipeline that writes to Layer built-in");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }
    if (multiview_features.multiviewGeometryShader == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiviewGeometryShader";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static char const *gsSource = R"glsl(
        #version 450
        layout (triangles) in;
        layout (triangle_strip) out;
        layout (max_vertices = 1) out;
        void main() {
            gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
            EmitVertex();
            gl_Layer = 4;
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.viewMask = 0x1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06059");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, InputAttachmentCapability) {
    TEST_DESCRIPTION("Create invalid pipeline that uses InputAttachment capability");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char *fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, RenderingInfoColorAttachmentFormat) {
    TEST_DESCRIPTION("Create pipeline with invalid color attachment format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_MAX_ENUM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06580");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, LibraryViewMask) {
    TEST_DESCRIPTION("Create pipeline with invalid view mask");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&multiview_features);
    InitBasicDynamicRendering(&library_features);
    if (::testing::Test::IsSkipped()) return;

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    auto graphics_library_create_info = LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&pipeline_rendering_info);
    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
    auto library_create_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>(&graphics_library_create_info);

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper lib(*this);
    lib.cb_ci_ = color_blend_state_create_info;
    lib.InitInfo();
    lib.gp_ci_.pNext = &library_create_info;
    lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib.InitState();
    lib.CreateGraphicsPipeline();

    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
    library_create_info.libraryCount = 1;
    library_create_info.pLibraries = &lib.pipeline_;
    pipeline_rendering_info.viewMask = 0x1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &library_create_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.shader_stages_ = {pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-pStages-06895");  // spec bug
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06626");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, AttachmentSampleCount) {
    TEST_DESCRIPTION("Create pipeline with invalid color attachment samples");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSampleCountFlagBits color_attachment_samples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;

    auto samples_info = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();
    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = &color_attachment_samples;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>(&samples_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pColorAttachmentSamples-06592");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, LibrariesViewMask) {
    TEST_DESCRIPTION("Create pipeline with libaries that have incompatible view mask");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&multiview_features);
    InitBasicDynamicRendering(&library_features);
    if (::testing::Test::IsSkipped()) return;

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper lib1(*this);
    lib1.cb_ci_ = color_blend_state_create_info;
    lib1.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    lib1.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib1.InitState();
    lib1.CreateGraphicsPipeline();

    pipeline_rendering_info.viewMask = 0x1;

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto fs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";

    CreatePipelineHelper lib2(*this);
    lib2.cb_ci_ = color_blend_state_create_info;
    lib2.InitFragmentLibInfo(1, &fs_stage_ci, &pipeline_rendering_info);
    lib2.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib2.ds_ci_ = ds_ci;
    lib2.InitState();
    lib2.CreateGraphicsPipeline();

    pipeline_rendering_info.viewMask = 0;
    auto library_create_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    library_create_info.libraryCount = 2;
    VkPipeline libraries[2] = {lib1.pipeline_, lib2.pipeline_};
    library_create_info.pLibraries = libraries;

    auto pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&library_create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06627");
    vk_testing::Pipeline pipe(*m_device, pipe_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, LibraryRenderPass) {
    TEST_DESCRIPTION("Create pipeline with invalid library render pass");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    InitBasicDynamicRendering(&library_features);
    if (::testing::Test::IsSkipped()) return;

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper lib(*this);
    lib.cb_ci_ = color_blend_state_create_info;
    lib.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    lib.InitState();
    lib.CreateGraphicsPipeline();

    auto library_create_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>(&pipeline_rendering_info);
    library_create_info.libraryCount = 1;
    library_create_info.pLibraries = &lib.pipeline_;

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto fs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentLibInfo(1, &fs_stage_ci, &library_create_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    // If not Frag Output with frag shader, need depth/stencil struct
    pipe.ds_ci_ = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06625");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, PipelineMissingMultisampleState) {
    TEST_DESCRIPTION("Create pipeline with missing multisample state");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    InitBasicDynamicRendering(&library_features);
    if (::testing::Test::IsSkipped()) return;

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.gp_ci_.pMultisampleState = nullptr;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06631");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo()};
        pipe.gp_ci_.pMultisampleState = nullptr;
        pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
        pipe.InitState();
        // No fragment shader implies no fragment shader state and rasterizerDiscardEnable == true implies no fragment
        // output state, so there should be no error with pMultisampleState == nullptr here
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicRendering, RenderingFragmentDensityMapAttachment) {
    TEST_DESCRIPTION("Use invalid VkRenderingFragmentDensityMapAttachmentInfoEXT");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    VkImageObj image2(m_device);
    image2.Init(image_create_info);
    VkImageView image_view2 = image2.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
    rendering_fragment_density.imageView = image_view2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-apiVersion-07908");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06109");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, RenderingFragmentDensityMapAttachmentUsage) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid imageLayout");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06158");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, FragmentDensityMapAttachmentCreateFlags) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid image create flags");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06159");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, FragmentDensityMapAttachmentLayerCount) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid layer count");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                              VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.viewMask = 0x1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-apiVersion-07908");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, PNextImageView) {
    TEST_DESCRIPTION(
        "Use different image views in VkRenderingFragmentShadingRateAttachmentInfoKHR and "
        "VkRenderingFragmentDensityMapAttachmentInfoEXT");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    rendering_fragment_shading_rate.imageView = image_view;
    rendering_fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    rendering_fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    auto rendering_fragment_density =
        LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>(&rendering_fragment_shading_rate);
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.viewMask = 0x1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06126");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, RenderArea) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = -1;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06077");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = -1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06078");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.offset.x = m_device->phy().properties().limits.maxFramebufferWidth - 16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07815");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 1;
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width) - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07815");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.x);
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07815");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.offset.y = m_device->phy().properties().limits.maxFramebufferHeight - 16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07816");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 1;
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height) - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07816");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.y);
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-07816");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, InfoViewMask) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    GetPhysicalDeviceProperties2(multiview_props);

    if (multiview_props.maxMultiviewViewCount == 32) {
        GTEST_SKIP() << "VUID is not testable as maxMultiviewViewCount is 32";
    }

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 32;
    begin_rendering_info.viewMask = 1u << multiview_props.maxMultiviewViewCount;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-viewMask-06128");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ColorAttachmentFormat) {
    TEST_DESCRIPTION("Use format with missing potential format features in rendering color attachment");
    AddRequiredExtensions(VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat format = FindSupportedDepthStencilFormat(gpu());
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, ResolveModeWithNonIntegerColorFormat) {
    TEST_DESCRIPTION("Use invalid resolve mode with non integer color format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;  // not int color
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_create_info);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;  // not allowed for format
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveModeWithIntegerColorFormat) {
    TEST_DESCRIPTION("Use invalid resolve mode with integer color format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_create_info);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_MAX_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06130");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveModeSamples) {
    TEST_DESCRIPTION("Use invalid sample count with resolve mode that is not none");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06861");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewSamples) {
    TEST_DESCRIPTION("Use resolve image view with invalid sample count");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_create_info);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06864");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveImageView = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06862");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewFormatMatch) {
    TEST_DESCRIPTION("Use resolve image view with different format from image view");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06865");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, AttachmentImageViewLayout) {
    TEST_DESCRIPTION("Use rendering attachment image view with invalid layout");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06135");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid layout");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06136");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewLayoutSeparateDepthStencil) {
    TEST_DESCRIPTION("Use resolve image view with invalid layout");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06137");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, AttachmentImageViewShadingRateLayout) {
    TEST_DESCRIPTION("Use image view with invalid layout");
    AddOptionalExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    const bool nv_shading_rate = IsExtensionsEnabled(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    const bool khr_fragment_shading = IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (!khr_fragment_shading && !nv_shading_rate) {
        GTEST_SKIP() << "shading rate / fragment shading not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    // SHADING_RATE_OPTIMAL_NV is aliased FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR so VU depends on which extensions
    const char *vuid =
        khr_fragment_shading ? "VUID-VkRenderingAttachmentInfo-imageView-06143" : "VUID-VkRenderingAttachmentInfo-imageView-06138";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewShadingRateLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid shading ratelayout");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    const bool nv_shading_rate = IsExtensionsEnabled(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    const bool khr_fragment_shading = IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (!khr_fragment_shading && !nv_shading_rate) {
        GTEST_SKIP() << "shading rate / fragment shading not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    // SHADING_RATE_OPTIMAL_NV is aliased FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR so VU depends on which extensions
    const char *vuid =
        khr_fragment_shading ? "VUID-VkRenderingAttachmentInfo-imageView-06144" : "VUID-VkRenderingAttachmentInfo-imageView-06139";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, AttachmentImageViewFragmentDensityLayout) {
    TEST_DESCRIPTION("Use image view with invalid layout");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06140");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewFragmentDensityLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid fragment density layout");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06141");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewReadOnlyOptimalLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid read only optimal layout");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06142");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingFragmentShadingRateImageView) {
    TEST_DESCRIPTION("Test BeginRenderingInfo image view with FragmentShadingRateAttachment.");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Skipping on AMD proprietary driver pending further investigation.";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    // TODO - Look into failure
    if (!image.initialized()) {
        GTEST_SKIP() << "Failed to create image";
    }
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, RenderingInfoColorAttachment) {
    TEST_DESCRIPTION("Test RenderingInfo color attachment.");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj invalid_image(m_device);
    invalid_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView invalid_image_view = invalid_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_create_info);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = invalid_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveImageView = resolve_image_view;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06087");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06090");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06096");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06100");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06091");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06097");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06101");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    const uint32_t max_color_attachments = m_device->phy().properties().limits.maxColorAttachments + 1;
    std::vector<VkRenderingAttachmentInfoKHR> color_attachments(max_color_attachments);
    for (auto &attachment : color_attachments) {
        attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
        attachment.imageView = image_view;
        attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    begin_rendering_info.colorAttachmentCount = max_color_attachments;
    begin_rendering_info.pColorAttachments = color_attachments.data();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06106");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, RenderingInfoDepthAttachment) {
    TEST_DESCRIPTION("Test RenderingInfo depth attachment.");
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const bool separate_ds_layouts = IsExtensionsEnabled(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);

    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    auto depth_stencil_resolve_properties = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
    GetPhysicalDeviceProperties2(depth_stencil_resolve_properties);
    bool has_depth_resolve_mode_average =
        (depth_stencil_resolve_properties.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0;
    bool has_stencil_resolve_mode_average =
        (depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0;
    bool has_stencil_resolve_mode_zero =
        (depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) != 0;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = ds_format;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);

    VkImageObj depth_image(m_device);
    depth_image.Init(image_create_info);
    VkImageView depth_image_view = depth_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    VkImageObj stencil_image(m_device);
    stencil_image.Init(image_create_info);
    VkImageView stencil_image_view = stencil_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageObj depth_resolvel_image(m_device);
    depth_resolvel_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView depth_resolve_image_view =
        depth_resolvel_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageObj stencil_resolvel_image(m_device);
    stencil_resolvel_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView stencil_resolve_image_view =
        stencil_resolvel_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageObj invalid_image(m_device);
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    invalid_image.Init(image_create_info);
    VkImageView invalid_image_view = invalid_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    auto depth_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    depth_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    stencil_attachment.imageView = stencil_image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06085");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageView = VK_NULL_HANDLE;
    stencil_attachment.imageView = VK_NULL_HANDLE;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveImageView = depth_resolve_image_view;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveImageView = stencil_resolve_image_view;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06086");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageView = depth_image_view;
    stencil_attachment.imageView = depth_image_view;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveImageView = depth_resolve_image_view;

    if (!has_depth_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06102");
    }
    if (!has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06103");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06093");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (separate_ds_layouts) {
        depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        if (!has_depth_resolve_mode_average) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06102");
        }
        if (!has_stencil_resolve_mode_average) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06103");
        }
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-07733");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    if (has_depth_resolve_mode_average && has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06098");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.imageView = invalid_image_view;
    stencil_attachment.imageView = invalid_image_view;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06088");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06089");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageView = depth_image_view;
    stencil_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06092");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (separate_ds_layouts) {
        depth_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-07732");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    if (depth_stencil_resolve_properties.independentResolveNone == VK_FALSE && has_depth_resolve_mode_average) {
        depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        stencil_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06104");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }
    if (depth_stencil_resolve_properties.independentResolve == VK_FALSE && has_depth_resolve_mode_average &&
        has_stencil_resolve_mode_zero) {
        depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        stencil_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
        m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pDepthAttachment-06104");  // if independentResolveNone is false
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06105");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06095");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
        if (separate_ds_layouts) {
            stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-07735");
            m_commandBuffer->BeginRendering(begin_rendering_info);
            m_errorMonitor->VerifyFound();
        }
    }
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    if (has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06099");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06094");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (separate_ds_layouts) {
        stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-07734");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, RenderAreaWithDeviceGroupExt) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = -1;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06077");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = -1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06078");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, Pipeline) {
    TEST_DESCRIPTION("Use pipeline created with render pass in dynamic render pass.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-06198");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingFragmentShadingRateAttachmentSize) {
    TEST_DESCRIPTION("Test FragmentShadingRateAttachment size.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Skipping on AMD proprietary driver pending further investigation.";
    }

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    // TODO - Look into failure
    if (!image.initialized()) {
        GTEST_SKIP() << "Failed to create image";
    }
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = fragment_shading_rate.shadingRateAttachmentTexelSize.width * 64;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06119");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = fragment_shading_rate.shadingRateAttachmentTexelSize.height * 64;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06121");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, FragmentShadingRateAttachmentSizeWithDeviceGroupExt) {
    TEST_DESCRIPTION("Test FragmentShadingRateAttachment size with device group extension.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Skipping on AMD proprietary driver pending further investigation.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>(&dynamic_rendering_features);
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(multiview_features);

    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    GetPhysicalDeviceProperties2(fsr_properties);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    // TODO - Look into failure
    if (!image.initialized()) {
        GTEST_SKIP() << "Failed to create image";
    }
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = fragment_shading_rate.shadingRateAttachmentTexelSize.width * 64;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06119");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 1;
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width) - 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07815");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06119");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.x);
    begin_rendering_info.renderArea.extent.width = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.width);
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderingInfo-pNext-07815");  // if over max
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06119");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.extent.width = 0;
    begin_rendering_info.renderArea.offset.y = fragment_shading_rate.shadingRateAttachmentTexelSize.height * 64;

    VkRect2D render_area = {};
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent.width = 64 * fragment_shading_rate.shadingRateAttachmentTexelSize.width;
    render_area.extent.height = 32;

    auto device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &render_area;
    fragment_shading_rate.pNext = &device_group_render_pass_begin_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06120");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 1;
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height) - 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06120");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = vvl::MaxTypeValue(begin_rendering_info.renderArea.offset.y);
    begin_rendering_info.renderArea.extent.height = vvl::MaxTypeValue(begin_rendering_info.renderArea.extent.height);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06120");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    render_area.extent.width = 32;
    begin_rendering_info.renderArea.offset.y = fragment_shading_rate.shadingRateAttachmentTexelSize.height * 64;
    render_area.extent.height = 64 * fragment_shading_rate.shadingRateAttachmentTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06122");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, SuspendingRenderPassInstance) {
    TEST_DESCRIPTION("Test suspending render pass instance.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkCommandPoolObj command_pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buffer1(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer2(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer3(m_device, &command_pool);

    VkRenderingInfo suspend_rendering_info = LvlInitStruct<VkRenderingInfo>();
    suspend_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    suspend_rendering_info.layerCount = 1;

    VkRenderingInfo resume_rendering_info = LvlInitStruct<VkRenderingInfo>();
    resume_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    resume_rendering_info.layerCount = 1;

    VkRenderingInfo rendering_info = LvlInitStruct<VkRenderingInfo>();
    rendering_info.layerCount = 1;

    auto cmd_begin = LvlInitStruct<VkCommandBufferBeginInfo>();

    cmd_buffer1.begin(&cmd_begin);
    cmd_buffer1.BeginRendering(suspend_rendering_info);
    cmd_buffer1.EndRendering();
    cmd_buffer1.end();

    cmd_buffer2.begin(&cmd_begin);
    cmd_buffer2.BeginRendering(resume_rendering_info);
    cmd_buffer2.EndRendering();
    cmd_buffer2.end();

    cmd_buffer3.begin(&cmd_begin);
    cmd_buffer3.BeginRendering(rendering_info);
    cmd_buffer3.EndRendering();
    cmd_buffer3.end();

    VkCommandBuffer command_buffers[3] = {cmd_buffer1.handle(), cmd_buffer2.handle()};

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06014");

    submit_info.commandBufferCount = 1;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06016");

    command_buffers[1] = cmd_buffer3.handle();
    command_buffers[2] = cmd_buffer2.handle();
    submit_info.commandBufferCount = 3;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06193");

    command_buffers[0] = cmd_buffer2.handle();
    submit_info.commandBufferCount = 1;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, SuspendingRenderPassInstanceQueueSubmit2) {
    TEST_DESCRIPTION("Test suspending render pass instance with QueueSubmit2.");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    auto synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>();
    InitBasicDynamicRendering(&synchronization2);
    if (::testing::Test::IsSkipped()) return;

    VkCommandPoolObj command_pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buffer1(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer2(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer3(m_device, &command_pool);

    VkRenderingInfo suspend_rendering_info = LvlInitStruct<VkRenderingInfo>();
    suspend_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    suspend_rendering_info.layerCount = 1;

    VkRenderingInfo resume_rendering_info = LvlInitStruct<VkRenderingInfo>();
    resume_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    resume_rendering_info.layerCount = 1;

    VkRenderingInfo rendering_info = LvlInitStruct<VkRenderingInfo>();
    rendering_info.layerCount = 1;

    auto cmd_begin = LvlInitStruct<VkCommandBufferBeginInfo>();

    cmd_buffer1.begin(&cmd_begin);
    cmd_buffer1.BeginRendering(suspend_rendering_info);
    cmd_buffer1.EndRendering();
    cmd_buffer1.end();

    cmd_buffer2.begin(&cmd_begin);
    cmd_buffer2.BeginRendering(resume_rendering_info);
    cmd_buffer2.EndRendering();
    cmd_buffer2.end();

    cmd_buffer3.begin(&cmd_begin);
    cmd_buffer3.BeginRendering(rendering_info);
    cmd_buffer3.EndRendering();
    cmd_buffer3.end();

    VkCommandBufferSubmitInfo command_buffer_submit_info[3];
    command_buffer_submit_info[0] = LvlInitStruct<VkCommandBufferSubmitInfo>();
    command_buffer_submit_info[1] = LvlInitStruct<VkCommandBufferSubmitInfo>();
    command_buffer_submit_info[2] = LvlInitStruct<VkCommandBufferSubmitInfo>();

    command_buffer_submit_info[0].commandBuffer = cmd_buffer1.handle();
    command_buffer_submit_info[1].commandBuffer = cmd_buffer2.handle();

    VkSubmitInfo2KHR submit_info = LvlInitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 2;
    submit_info.pCommandBufferInfos = command_buffer_submit_info;
    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06010");

    submit_info.commandBufferInfoCount = 1;
    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06012");

    command_buffer_submit_info[1].commandBuffer = cmd_buffer3.handle();
    command_buffer_submit_info[2].commandBuffer = cmd_buffer2.handle();
    submit_info.commandBufferInfoCount = 3;
    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06192");

    command_buffer_submit_info[0].commandBuffer = cmd_buffer2.handle();
    submit_info.commandBufferInfoCount = 1;
    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, NullDepthStencilExecuteCommands) {
    TEST_DESCRIPTION(
        "Test for NULL depth stencil attachments in dynamic rendering with secondary command buffer with depth stencil format "
        "inheritance info");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    // Create secondary command buffer
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(gpu());

    auto cbiri = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    // format is defined, although no image view provided in dynamic rendering
    cbiri.depthAttachmentFormat = depth_stencil_format;
    cbiri.stencilAttachmentFormat = depth_stencil_format;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    auto cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>(&cbiri);

    auto cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.pInheritanceInfo = &cbii;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Prepare primary dynamic rendering cmd buffer
    VkDepthStencilObj depth_stencil(m_device);
    depth_stencil.Init(m_device, 32, 32, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                       VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    auto rai = LvlInitStruct<VkRenderingAttachmentInfo>();
    rai.imageView = *depth_stencil.BindInfo();
    rai.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto ri = LvlInitStruct<VkRenderingInfoKHR>();
    ri.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    ri.layerCount = 1;
    ri.pDepthAttachment = &rai;
    ri.pStencilAttachment = &rai;

    // Record secondary cmd buffer with depth stencil format
    secondary.begin(&cbbi);
    secondary.end();

    // Record primary cmd buffer with depth stencil
    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(ri);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    // Retry with null depth stencil attachment image view
    rai.imageView = VK_NULL_HANDLE;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(ri);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pDepthAttachment-06774");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pStencilAttachment-06775");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    // Retry with nullptr attachment struct
    ri.pDepthAttachment = nullptr;
    ri.pStencilAttachment = nullptr;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(ri);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pDepthAttachment-06774");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pStencilAttachment-06775");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    // Retry with no format in inheritance info
    cbiri.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    cbiri.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    secondary.begin(&cbbi);
    secondary.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(ri);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingWithSecondaryContents) {
    TEST_DESCRIPTION("Test that an error is produced when a secondary command buffer calls BeginRendering with secondary contents");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-commandBuffer-06068");
    secondary.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    secondary.end();
}

TEST_F(NegativeDynamicRendering, BadRenderPassContentsWhenCallingCmdExecuteCommands) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that hasn't set "
        "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    constexpr VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-flags-06024");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithNonNullRenderPass) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that hasn't set "
        "renderPass to VK_NULL_HANDLE in pInheritanceInfo");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 2, subpasses, 0, nullptr};
    vk_testing::RenderPass render_pass(*m_device, rpci);

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        render_pass.handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06025");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingFlags) {
    TEST_DESCRIPTION("Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching flags");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    constexpr VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags =
        VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR | VK_RENDERING_SUSPENDING_BIT_KHR | VK_RENDERING_RESUMING_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-flags-06026");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingColorAttachmentCount) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching colorAttachmentCount");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-colorAttachmentCount-06027");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingColorImageViewFormat) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching color image view format");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    constexpr std::array bad_color_formats = {VK_FORMAT_R8G8B8A8_UINT};

    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = bad_color_formats.size();
    inheritance_rendering_info.pColorAttachmentFormats = bad_color_formats.data();
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const auto cmdbuff_ii = LvlInitStruct<VkCommandBufferInheritanceInfo>(&inheritance_rendering_info);

    auto cmdbuff_bi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmdbuff_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmdbuff_bi.pInheritanceInfo = &cmdbuff_ii;

    secondary.begin(&cmdbuff_bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-imageView-06028");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithNullImageView) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands with an inherited image format that is not VK_FORMAT_UNDEFINED inside a render pass begun with "
        "CmdBeginRendering where the same image is specified as null");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    constexpr std::array bad_color_formats = {VK_FORMAT_R8G8B8A8_UINT};

    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = bad_color_formats.size();
    inheritance_rendering_info.pColorAttachmentFormats = bad_color_formats.data();
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const auto cmdbuff_ii = LvlInitStruct<VkCommandBufferInheritanceInfo>(&inheritance_rendering_info);

    auto cmdbuff_bi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmdbuff_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmdbuff_bi.pInheritanceInfo = &cmdbuff_ii;

    secondary.begin(&cmdbuff_bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-imageView-07606");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingDepthStencilImageViewFormat) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching depth/stencil image view "
        "format");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkImageObj image(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    if (depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        GTEST_SKIP() << "Insufficient depth-stencil formats supported";
    }

    image.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = imageView;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_rendering_info.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    begin_rendering_info.layerCount = 1;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pDepthAttachment-06029");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pStencilAttachment-06030");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingViewMask) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching viewMask format");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    auto mv_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>();
    InitBasicDynamicRendering(&mv_features);
    if (::testing::Test::IsSkipped()) return;

    if (!mv_features.multiview) {
        GTEST_SKIP() << "multiview feature not supported.";
    }

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.viewMask = 0;
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.viewMask = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-viewMask-06031");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingImageViewRasterizationSamples) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching rasterization samples");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    // color samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06035");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    VkImageObj depthStencilImage(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    depthStencilImage.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                           0);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.depthAttachmentFormat = depth_stencil_format;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // depth samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06036");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    inheritance_rendering_info.stencilAttachmentFormat = depth_stencil_format;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // stencil samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06037");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ExecuteCommandsWithMismatchingImageViewAttachmentSamples) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has mismatching that has mismatching "
        "attachment samples");
    AddOptionalExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    bool amd_samples = IsExtensionsEnabled(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    bool nv_samples = IsExtensionsEnabled(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    if (!amd_samples && !nv_samples) {
        GTEST_SKIP() << "Test requires either VK_AMD_mixed_attachment_samples or VK_NV_framebuffer_mixed_samples";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkSampleCountFlagBits counts = {VK_SAMPLE_COUNT_2_BIT};
    auto samples_info = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();
    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = &counts;

    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>(&samples_info);
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    secondary.end();

    m_commandBuffer->begin();

    // color samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06032");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    VkImageObj depthStencilImage(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    depthStencilImage.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                           0);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    samples_info.colorAttachmentCount = 0;
    samples_info.pColorAttachmentSamples = nullptr;
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.depthAttachmentFormat = depth_stencil_format;
    samples_info.depthStencilAttachmentSamples = VK_SAMPLE_COUNT_2_BIT;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // depth samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06033");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    inheritance_rendering_info.stencilAttachmentFormat = depth_stencil_format;

    secondary.begin(&cmdbuff__bi);
    secondary.end();

    // stencil samples mismatch
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pNext-06034");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, InSecondaryCommandBuffers) {
    TEST_DESCRIPTION("Test drawing in secondary command buffers with dynamic rendering");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkFormat format = VK_FORMAT_R32G32B32A32_UINT;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferInheritanceRenderingInfoKHR inheritanceRenderingInfo =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritanceRenderingInfo.colorAttachmentCount = 1;
    inheritanceRenderingInfo.pColorAttachmentFormats = &format;
    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>(&inheritanceRenderingInfo);
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;

    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin(&cbbi);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    secondary.Draw(3, 1, 0, 0);
    secondary.end();
}

TEST_F(NegativeDynamicRendering, CommandBufferInheritanceDepthFormat) {
    TEST_DESCRIPTION(
        "Test VkCommandBufferInheritanceRenderingInfoKHR with depthAttachmentFormat that does not include depth aspect");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    if (!m_device->phy().features().variableMultisampleRate) {
        GTEST_SKIP() << "Test requires (unsupported) variableMultisampleRate";
    }

    auto stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    auto inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.depthAttachmentFormat = stencil_format;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    auto cmdbuf_ii = LvlInitStruct<VkCommandBufferInheritanceInfo>(&inheritance_rendering_info);
    auto cmdbuf_bi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmdbuf_bi.pInheritanceInfo = &cmdbuf_ii;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540");

    vk::BeginCommandBuffer(secondary.handle(), &cmdbuf_bi);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, DeviceGroupRenderArea) {
    TEST_DESCRIPTION("Begin rendering with invaid device group render area.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRect2D renderArea = {};
    renderArea.offset.x = -1;
    renderArea.offset.y = -1;
    renderArea.extent.width = 64;
    renderArea.extent.height = 64;

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceMask = 0x1;
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &renderArea;

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&device_group_render_pass_begin_info);
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea.offset.x = 0;
    renderArea.offset.y = 0;
    renderArea.extent.width = m_device->props.limits.maxFramebufferWidth + 1;
    renderArea.extent.height = m_device->props.limits.maxFramebufferHeight + 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06168");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06169");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MaxFramebufferLayers) {
    TEST_DESCRIPTION("Go over maxFramebufferLayers");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = m_device->props.limits.maxFramebufferLayers + 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-layerCount-07817");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, EndRenderingWithIncorrectlyStartedRenderpassInstance) {
    TEST_DESCRIPTION(
        "Test EndRendering without starting the instance with BeginRendering, in the same command buffer or in a different once");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpass, 0, nullptr};
    vk_testing::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &imageView, 32, 32, 1};
    vk_testing::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi =
        LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    m_commandBuffer->BeginRenderPass(rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-None-06161");
    m_commandBuffer->EndRendering();
    m_errorMonitor->VerifyFound();

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-commandBuffer-06162");
    secondary.EndRendering();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, EndRenderpassWithBeginRenderingRenderpassInstance) {
    TEST_DESCRIPTION("Test EndRenderpass(2) starting the renderpass instance with BeginRendering");
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass-None-06170");
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();

    VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

    VkCommandBufferObj primary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    primary.begin();
    primary.BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass2-None-06171");
    vk::CmdEndRenderPass2KHR(primary.handle(), &subpassEndInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, BeginRenderingDisabled) {
    TEST_DESCRIPTION("Validate VK_KHR_dynamic_rendering VUs when disabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-dynamicRendering-06446");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-dynamicRendering-06446");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-None-06161");
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_commandBuffer->EndRendering();
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, PipelineRenderingParameters) {
    TEST_DESCRIPTION("Test pipeline rendering formats and viewmask");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    auto depth_stencil_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    create_info.pDepthStencilState = &depth_stencil_state;

    VkFormat depth_format = VK_FORMAT_X8_D24_UNORM_PACK32;

    if (ImageFormatAndFeaturesSupported(gpu_, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        depth_format = VK_FORMAT_D32_SFLOAT;
    }

    VkFormat stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;

    if (ImageFormatAndFeaturesSupported(gpu_, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    VkFormat color_formats = {depth_format};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    // Invalid color format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Invalid color format array
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Invalid depth format
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06587");
    // TODO (ncesario) Seems impossible hit 06585 without also hitting 06587. Since 06587 happens in stateless validation, 06585
    // never gets triggered, though has been manually tested separately by removing 06587.
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06585");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Invalid stecil format
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06588");
    // TODO (ncesario) Same scenario as with 06585 and 06587
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06586");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // mismatching depth/stencil formats
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = stencil_format;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06589");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Non-zero viewMask
    color_formats = VK_FORMAT_R8G8B8A8_UNORM;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577");
    pipeline_rendering_info.viewMask = 1;
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, PipelineRenderingViewMaskParameter) {
    TEST_DESCRIPTION("Test pipeline rendering viewmask maximum index");
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    InitBasicDynamicRendering(&multiview_features);
    if (::testing::Test::IsSkipped()) return;

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    auto multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    GetPhysicalDeviceProperties2(multiview_props);

    if (multiview_props.maxMultiviewViewCount == 32) {
        GTEST_SKIP() << "TVUID is not testable as maxMultiviewViewCount is 32";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06578");
    pipeline_rendering_info.viewMask = 1 << multiview_props.maxMultiviewViewCount;
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, CreateGraphicsPipeline) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    auto rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &color_format;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &rendering_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, CreateGraphicsPipelineNoInfo) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled but no rendering info struct.");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    // if there isn't a VkPipelineRenderingCreateInfoKHR, the driver is supposed to use safe default values
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, DynamicColorBlendAttchment) {
    TEST_DESCRIPTION("Test all color blend attachments are dynamically set at draw time with Dynamic Rendering.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    auto extended_dynamic_state3_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    InitBasicDynamicRendering(&extended_dynamic_state3_features);
    if (::testing::Test::IsSkipped()) return;

    if (!extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.dyn_state_ci_ = dynamic_create_info;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-firstAttachment-07478");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // once set error goes away
    VkColorComponentFlags color_component_flags = VK_COLOR_COMPONENT_R_BIT;
    vk::CmdSetColorWriteMaskEXT(m_commandBuffer->handle(), 0, 1, &color_component_flags);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginTwice) {
    TEST_DESCRIPTION("Call vkCmdBeginRendering twice in a row");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-renderpass");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->EndRendering();
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, EndTwice) {
    TEST_DESCRIPTION("Call vkCmdEndRendering twice in a row");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->EndRendering();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-renderpass");
    m_commandBuffer->EndRendering();
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MissingMultisampleState) {
    TEST_DESCRIPTION("Create pipeline with fragment shader that uses samples, but multisample state not begin set");
    InitBasicDynamicRendering();
    if (::testing::Test::IsSkipped()) return;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}
