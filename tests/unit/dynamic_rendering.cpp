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
#include "../framework/pipeline_helper.h"

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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceLinearColorAttachmentFeaturesNV linear_color_attachment = vku::InitStructHelper();
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

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkPhysicalDeviceMultiviewProperties multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_props);

    VkFormat color_format = VK_FORMAT_D32_SFLOAT;

    VkCommandBufferInheritanceRenderingInfoKHR cmd_buffer_inheritance_rendering_info = vku::InitStructHelper();
    cmd_buffer_inheritance_rendering_info.colorAttachmentCount = 1;
    cmd_buffer_inheritance_rendering_info.pColorAttachmentFormats = &color_format;
    cmd_buffer_inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_R8G8B8_UNORM;
    cmd_buffer_inheritance_rendering_info.stencilAttachmentFormat = VK_FORMAT_R8G8B8_SNORM;
    cmd_buffer_inheritance_rendering_info.viewMask = 1 << multiview_props.maxMultiviewViewCount;

    VkAttachmentSampleCountInfoAMD sample_count_info_amd = vku::InitStructHelper();
    sample_count_info_amd.pNext = &cmd_buffer_inheritance_rendering_info;
    sample_count_info_amd.colorAttachmentCount = 2;

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.pNext = &sample_count_info_amd;

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info = vku::InitStructHelper();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 0x1;

    VkCommandBuffer secondary_cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);
    ASSERT_EQ(VK_SUCCESS, err);
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

    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    CreatePipelineHelper pipe(*this, 0);
    pipe.InitState();
    pipe.gp_ci_.layout = pl.handle();
    pipe.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

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

    vkt::ImageView depth_image_view(*m_device, ivci);

    VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
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
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&shader_tile_image_features))

    if (!shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageDepthStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    ds_state.depthWriteEnable = VK_TRUE;

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});

    VkPipelineMultisampleStateCreateInfo ms_ci = vku::InitStructHelper();
    ms_ci.sampleShadingEnable = VK_TRUE;
    ms_ci.minSampleShading = 1.0;
    ms_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs->GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = &ms_ci;
    pipe.gp_ci_.pDepthStencilState = &ds_state;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&dsl});
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

    vkt::ImageView depth_image_view(*m_device, depth_view_ci);

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

    vkt::ImageView color_image_view(*m_device, color_view_ci);

    VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = color_image_view.handle();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

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
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
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
    vkt::ImageView render_target_view(*m_device, ivci);

    // Create secondary command buffer
    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_commandPool->handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);
    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &render_target_ci.format;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info =
        vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment_info = vku::InitStructHelper();
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.imageView = render_target_view.handle();
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    // Create color image
    const VkFormat color_format = VK_FORMAT_R32_SFLOAT;
    VkImageObj color_image(m_device);
    VkImageCreateInfo imci = vku::InitStructHelper();
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
    vkt::ImageView color_image_view(*m_device, color_ivci);

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
    vkt::ImageView depth_image_view(*m_device, depth_stencil_ivci);
    depth_stencil_ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vkt::ImageView stencil_image_view(*m_device, depth_stencil_ivci);
    depth_stencil_ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vkt::ImageView depth_stencil_image_view(*m_device, depth_stencil_ivci);

    // Dynamic rendering structs
    VkRect2D rect{{0, 0}, {32, 32}};
    VkRenderingAttachmentInfoKHR depth_attachment_info = vku::InitStructHelper();
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
    depth_attachment_info.imageView = depth_stencil_image_view.handle();
    VkRenderingAttachmentInfoKHR stencil_attachment_info = vku::InitStructHelper();
    stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment_info.imageView = depth_stencil_image_view.handle();
    VkRenderingAttachmentInfoKHR color_attachment_info = vku::InitStructHelper();
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.imageView = color_image_view;
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();

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

    VkRenderPassCreateInfo renderpass_ci = vku::InitStructHelper();
    renderpass_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderpass_ci.pAttachments = attachments.data();
    renderpass_ci.subpassCount = static_cast<uint32_t>(subpass_descs.size());
    renderpass_ci.pSubpasses = subpass_descs.data();
    renderpass_ci.dependencyCount = 1;
    renderpass_ci.pDependencies = &subpass_dependency;
    vkt::RenderPass renderpass(*m_device, renderpass_ci);

    std::array<VkImageView, 2> renderpass_image_views = {depth_stencil_image_view.handle(), color_image_view.handle()};

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = renderpass.handle();
    framebuffer_ci.attachmentCount = 2;
    framebuffer_ci.pAttachments = renderpass_image_views.data();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    VkRenderPassBeginInfo renderpass_bi = vku::InitStructHelper();
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
                ASSERT_EQ(VK_SUCCESS, err);
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
                ASSERT_EQ(VK_SUCCESS, err);
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
                ASSERT_EQ(VK_SUCCESS, err);
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
            m_commandBuffer = new vkt::CommandBuffer(m_device, m_commandPool);

            std::unique_ptr<vkt::CommandBuffer> secondary_cmd_buffer(
                new vkt::CommandBuffer(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY));

            VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
            const VkFormat color_format = VK_FORMAT_R32_SFLOAT;
            inheritance_rendering_info.colorAttachmentCount = begin_rendering_info.colorAttachmentCount;
            inheritance_rendering_info.pColorAttachmentFormats = &color_format;
            inheritance_rendering_info.depthAttachmentFormat = depth_format;
            inheritance_rendering_info.stencilAttachmentFormat = depth_format;
            inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
            cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
            cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;
            if (use_dynamic_rendering) {
                cmd_buffer_inheritance_info.pNext = &inheritance_rendering_info;
            } else {
                const VkResult err = vk::CreateFramebuffer(m_device->handle(), &framebuffer_ci, nullptr, &framebuffers[3]);
                ASSERT_EQ(VK_SUCCESS, err);
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
    m_commandBuffer = new vkt::CommandBuffer(m_device, m_commandPool);
    clear_cmd_test(false);
}

TEST_F(NegativeDynamicRendering, GraphicsPipelineCreateInfo) {
    TEST_DESCRIPTION("Test graphics pipeline creation with dynamic rendering.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    if (m_device->phy().limits_.maxGeometryOutputVertices == 0) {
        GTEST_SKIP() << "Device doesn't support required maxGeometryOutputVertices";
    }
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_state(2);

    VkFormat color_format[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT};

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format[0];
    pipeline_rendering_info.viewMask = 0x2;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, kGeometryMinimalGlsl, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj te(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj tc(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this, 2);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), te.GetStageCreateInfo(), tc.GetStageCreateInfo(),
                           fs.GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.tess_ci_ = vku::InitStruct<VkPipelineTessellationStateCreateInfo>();
    pipe.tess_ci_.patchControlPoints = 1;
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-09033");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06057");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06058");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.pColorBlendState = nullptr;
    pipe.cb_ci_.attachmentCount = 1;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.viewMask = 0x0;
    pipeline_rendering_info.colorAttachmentCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-09037");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    color_format[0] = VK_FORMAT_D32_SFLOAT_S8_UINT;
    pipe.cb_attachments_[0].blendEnable = VK_TRUE;
    pipe.gp_ci_.pColorBlendState = &pipe.cb_ci_;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06062");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
    color_format[0] = VK_FORMAT_R8G8B8A8_UNORM;

    pipe.cb_ci_.flags = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;
    pipe.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.ds_ci_.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06482");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06483");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, ColorAttachmentMismatch) {
    TEST_DESCRIPTION("colorAttachmentCount and attachmentCount don't match");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = vku::InitStructHelper();
    color_blend_state_create_info.attachmentCount = 0;
    color_blend_state_create_info.pAttachments = nullptr;

    VkFormat color_format[2] = {VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_format;

    {
        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.cb_ci_ = color_blend_state_create_info;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    pipeline_rendering_info.colorAttachmentCount = 2;
    {
        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicRendering, MismatchingViewMask) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering and a mismatching viewMask");
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

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

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.viewMask = 1;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.InitState();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.viewMask = 2;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewMask-06178");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipeline_color(*this);
    pipeline_color.InitState();
    pipeline_color.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_color.gp_ci_.layout = pl.handle();
    pipeline_color.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_color.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    CreatePipelineHelper pipeline_depth(*this, 0);
    pipeline_depth.InitState();
    pipeline_depth.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_depth.gp_ci_.layout = pl.handle();
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_depth.CreateGraphicsPipeline();

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    bool testStencil = false;
    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

    if (FormatIsSupported(gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D16_UNORM_S8_UINT) &&
               FormatIsSupported(gpu(), VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D16_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D24_UNORM_S8_UINT) &&
               FormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D32_SFLOAT_S8_UINT) &&
               FormatIsSupported(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        testStencil = true;
    }

    CreatePipelineHelper pipeline_stencil(*this, 0);
    if (testStencil) {
        pipeline_rendering_info.colorAttachmentCount = 0;
        pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipeline_rendering_info.stencilAttachmentFormat = stencilFormat;

        pipeline_stencil.InitState();
        pipeline_stencil.shader_stages_[1] = fs.GetStageCreateInfo();
        pipeline_stencil.gp_ci_.layout = pl.handle();
        pipeline_stencil.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
        pipeline_stencil.gp_ci_.pNext = &pipeline_rendering_info;
        pipeline_stencil.CreateGraphicsPipeline();
    }

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    m_commandBuffer->begin();

    // Mismatching color attachment count
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06179");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching color formats
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08910");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth format
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil format
    if (testStencil) {
        begin_rendering_info.pDepthAttachment = nullptr;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08917");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats2) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with attachment specified as VK_NULL_HANDLE in VkRenderingInfoKHR, but with corresponding "
        "format in VkPipelineRenderingCreateInfoKHR not set to VK_FORMAT_UNDEFINED");

    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipeline_color(*this);
    pipeline_color.InitState();
    pipeline_color.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_color.gp_ci_.layout = pl.handle();
    pipeline_color.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_color.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    CreatePipelineHelper pipeline_depth(*this, 0);
    pipeline_depth.InitState();
    pipeline_depth.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_depth.gp_ci_.layout = pl.handle();
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_depth.CreateGraphicsPipeline();

    const VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipeline_stencil(*this, 0);
    pipeline_stencil.InitState();
    pipeline_stencil.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_stencil.gp_ci_.layout = pl.handle();
    pipeline_stencil.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_stencil.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_stencil.CreateGraphicsPipeline();

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = VK_NULL_HANDLE;

    m_commandBuffer->begin();

    {
        // Mismatching color formats
        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08912");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    {
        // Mismatching depth format
        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    {
        // Mismatching stencil format
        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats3Color) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats where "
        "dynamicRenderingUnusedAttachments is enabled and neither format is VK_FORMAT_UNDEFINED");
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamic_rendering_unused_attachments_features =
        vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&dynamic_rendering_unused_attachments_features))
    InitRenderTarget();

    if (!dynamic_rendering_unused_attachments_features.dynamicRenderingUnusedAttachments) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRenderingUnusedAttachments , skipping.";
    }

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_B8G8R8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    m_commandBuffer->begin();

    // Mismatching color formats
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08911");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentFormats3DepthStencil) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats where "
        "dynamicRenderingUnusedAttachments is enabled and neither format is VK_FORMAT_UNDEFINED");
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamic_rendering_unused_attachments_features =
        vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&dynamic_rendering_unused_attachments_features))

    if (!dynamic_rendering_unused_attachments_features.dynamicRenderingUnusedAttachments) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRenderingUnusedAttachments , skipping.";
    }

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    CreatePipelineHelper pipe1(*this);
    pipe1.InitState();
    pipe1.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe1.gp_ci_.layout = pl.handle();
    pipe1.ds_ci_ = vku::InitStructHelper();
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.gp_ci_.pColorBlendState = nullptr;
    pipe1.CreateGraphicsPipeline();

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    bool testStencil = false;
    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

    if (FormatIsSupported(gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D16_UNORM_S8_UINT) &&
               FormatIsSupported(gpu(), VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D16_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D24_UNORM_S8_UINT) &&
               FormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D32_SFLOAT_S8_UINT) &&
               FormatIsSupported(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        testStencil = true;
    }

    CreatePipelineHelper pipe2(*this);
    if (testStencil) {
        pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipeline_rendering_info.stencilAttachmentFormat = stencilFormat;

        pipe2.InitState();
        pipe2.shader_stages_[1] = fs.GetStageCreateInfo();
        pipe2.gp_ci_.layout = pl.handle();
        pipe2.ds_ci_ = vku::InitStructHelper();
        pipe2.gp_ci_.pNext = &pipeline_rendering_info;
        pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe2.gp_ci_.pColorBlendState = nullptr;
        pipe2.CreateGraphicsPipeline();
    }

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    m_commandBuffer->begin();

    // Mismatching depth format
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08915");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil format
    if (testStencil) {
        begin_rendering_info.pDepthAttachment = nullptr;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08918");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentSamplesColor) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color sample counts");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    m_commandBuffer->begin();

    // Mismatching color samples
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MistmatchingAttachmentSamplesDepthStencil) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching depth/stencil sample counts");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipe1(*this);
    pipe1.InitState();
    pipe1.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe1.gp_ci_.layout = pl.handle();
    pipe1.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe1.ds_ci_ = vku::InitStructHelper();
    pipe1.gp_ci_.pColorBlendState = nullptr;
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipe2(*this);
    pipe2.InitState();
    pipe2.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe2.gp_ci_.layout = pl.handle();
    pipe2.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe2.ds_ci_ = vku::InitStructHelper();
    pipe2.gp_ci_.pColorBlendState = nullptr;
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe2.CreateGraphicsPipeline();

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    m_commandBuffer->begin();

    // Mismatching depth samples
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil samples
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MismatchingMixedAttachmentSamplesColor) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching mixed color sample counts");
    AddOptionalExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

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
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkSampleCountFlagBits counts[2] = {VK_SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_2_BIT};
    VkAttachmentSampleCountInfoAMD samples_info = vku::InitStructHelper();

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper(&samples_info);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = counts;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;

    samples_info.colorAttachmentCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06063");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    samples_info.colorAttachmentCount = 1;
    pipe.CreateGraphicsPipeline();

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    m_commandBuffer->begin();

    // Mismatching color samples
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06185");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MismatchingMixedAttachmentSamplesDepthStencil) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching mixed depth/stencil sample counts");
    AddOptionalExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())

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
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkSampleCountFlagBits counts[2] = {VK_SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_2_BIT};
    VkAttachmentSampleCountInfoAMD samples_info = vku::InitStructHelper();

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper(&samples_info);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    samples_info.colorAttachmentCount = 0;
    samples_info.pColorAttachmentSamples = nullptr;
    samples_info.depthStencilAttachmentSamples = counts[0];

    CreatePipelineHelper pipe1(*this);
    pipe1.InitState();
    pipe1.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe1.gp_ci_.layout = pl.handle();
    pipe1.ds_ci_ = vku::InitStructHelper();
    pipe1.gp_ci_.pColorBlendState = nullptr;
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipe2(*this);
    pipe2.InitState();
    pipe2.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe2.gp_ci_.layout = pl.handle();
    pipe2.ds_ci_ = vku::InitStructHelper();
    pipe2.gp_ci_.pColorBlendState = nullptr;
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe2.CreateGraphicsPipeline();

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->begin();

    // Mismatching depth samples
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-06186");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil samples
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-06187");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, AttachmentInfo) {
    TEST_DESCRIPTION("AttachmentInfo Dynamic Rendering Tests.");
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&fdm_features))

    if (!fdm_features.fragmentDensityMapNonSubsampledImages) {
        GTEST_SKIP() << "fragmentDensityMapNonSubsampledImages not supported.";
    }

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;

    if (!FormatFeaturesAreSupported(gpu(), depth_format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }

    VkImageObj image(m_device);
    VkImageObj image_fragment(m_device);
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    depth_attachment.imageView = depth_image_view;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.viewMask = 0x4;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map =
        vku::InitStructHelper();
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
    RETURN_IF_SKIP(Init())

    VkCommandBufferInheritanceRenderingInfoKHR cmd_buffer_inheritance_rendering_info = vku::InitStructHelper();
    cmd_buffer_inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.pNext = &cmd_buffer_inheritance_rendering_info;
    cmd_buffer_inheritance_info.renderPass = VK_NULL_HANDLE;

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info = vku::InitStructHelper();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 0x1;

    VkCommandBuffer secondary_cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    // Invalid RenderPass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06000");
    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
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

    vkt::RenderPass rp1(*m_device, rpci);

    cmd_buffer_inheritance_info.renderPass = rp1.handle();
    cmd_buffer_inheritance_info.subpass = 0x5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06001");
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, SecondaryCommandBuffer) {
    TEST_DESCRIPTION("VkCommandBufferBeginInfo Dynamic Rendering Tests.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init())

    vkt::CommandBuffer cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Force the failure by not setting the Renderpass and Framebuffer fields
    VkCommandBufferInheritanceInfo cmd_buf_hinfo = vku::InitStructHelper();
    cmd_buf_hinfo.renderPass = VkRenderPass(0x1);
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
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

    vkt::RenderPass rp1(*m_device, rpci);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    bool fragment_density = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    bool shading_rate = IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    InitRenderTarget();

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    // Mostly likely will only find support for this on a custom profiles
    if (!FormatFeaturesAreSupported(gpu(), depthStencilFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        shading_rate = false;
    }
    if (!FormatFeaturesAreSupported(gpu(), depthStencilFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        fragment_density = false;
    }
    if (!fragment_density && !shading_rate) {
        GTEST_SKIP() << "shading rate / fragment shading not supported";
    }

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    vkt::ImageView depth_image_view(*m_device, ivci);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (shading_rate) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-imageView-06183");
        VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate =
            vku::InitStructHelper();
        fragment_shading_rate.imageView = depth_image_view.handle();
        fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_shading_rate);
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

        const VkFormat color_format = VK_FORMAT_UNDEFINED;

        VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;

        CreatePipelineHelper pipe(*this);
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
            vku::InitStructHelper();
        fragment_density_map.imageView = depth_image_view.handle();
        fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_density_map);
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

        const VkFormat color_format = VK_FORMAT_UNDEFINED;

        VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;

        CreatePipelineHelper pipe(*this);
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-viewMask-06069");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, InfoMismatchedSamples) {
    TEST_DESCRIPTION("Test beginning rendering with mismatched sample counts.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkImageCreateInfo image_ci = vku::InitStructHelper();
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

    VkImageViewCreateInfo civ_ci = vku::InitStructHelper();
    civ_ci.image = color_image.handle();
    civ_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    civ_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    civ_ci.subresourceRange.layerCount = 1;
    civ_ci.subresourceRange.baseMipLevel = 0;
    civ_ci.subresourceRange.levelCount = 1;
    civ_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkt::ImageView color_image_view(*m_device, civ_ci);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_image_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkImageObj depth_image(m_device);
    depth_image.Init(64, 64, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkImageViewCreateInfo div_ci = vku::InitStructHelper();
    div_ci.image = depth_image.handle();
    div_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    div_ci.format = depth_format;
    div_ci.subresourceRange.layerCount = 1;
    div_ci.subresourceRange.baseMipLevel = 0;
    div_ci.subresourceRange.levelCount = 1;
    div_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vkt::ImageView depth_image_view(*m_device, div_ci);

    VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-multisampledRenderToSingleSampled-06857");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingFragmentShadingRate) {
    TEST_DESCRIPTION("Test BeginRenderingInfo with FragmentShadingRateAttachment.");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "format doesn't support FRAGMENT_SHADING_RATE_ATTACHMENT_BIT";
    }

    VkImageCreateInfo image_ci = vku::InitStructHelper(nullptr);
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

    VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate = vku::InitStructHelper();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_shading_rate);
    begin_rendering_info.layerCount = 4;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06123");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.viewMask = 0xF;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06124");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRect2D render_area = {};
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent.width = 32;
    render_area.extent.height = 32;

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = vku::InitStructHelper();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &render_area;

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = colorImageView;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&device_group_render_pass_begin_info);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR not supported";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj invalid_image(m_device);
    invalid_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView invalid_image_view = invalid_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate = vku::InitStructHelper();
    fragment_shading_rate.imageView = invalid_image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkFormat stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(stencil_format, VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06547");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, TestFragmentDensityMapRenderArea) {
    TEST_DESCRIPTION("Validate VkRenderingFragmentDensityMapAttachmentInfo attachment image view extent.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }

    VkPhysicalDeviceFragmentDensityMapPropertiesEXT fdm_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_props);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map = vku::InitStructHelper();
    fragment_density_map.imageView = image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_density_map);
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
    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = vku::InitStructHelper();
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

    RETURN_IF_SKIP(InitFramework())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkPhysicalDeviceFragmentDensityMapPropertiesEXT fdm_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_props);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map = vku::InitStructHelper();
    fragment_density_map.imageView = image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_density_map);
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

TEST_F(NegativeDynamicRendering, BarrierShaderTileFeaturesNotEnabled) {
    TEST_DESCRIPTION("Test setting memory barrier without shader tile features enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&sync2_features))
    InitRenderTarget();

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->BeginRendering(begin_rendering_info);

    VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
    barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-shaderTileImageColorReadAccess-08718");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-shaderTileImageColorReadAccess-08718");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, WithoutShaderTileImageAndBarrier) {
    TEST_DESCRIPTION("Test setting memory barrier if the shader tile image features are not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan13Features vk13features = vku::InitStructHelper();
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
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

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->BeginRendering(begin_rendering_info);

    VkMemoryBarrier2KHR memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();
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

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
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

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan13Features vk13features = vku::InitStructHelper();
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
    vk13features.pNext = &shader_tile_image_features;

    auto features2 = GetPhysicalDeviceFeatures2(vk13features);
    if (!vk13features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->BeginRendering(begin_rendering_info);

    VkMemoryBarrier2KHR memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();
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

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkBufferMemoryBarrier2KHR buf_barrier_2 = vku::InitStructHelper();
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

    VkBufferMemoryBarrier buf_barrier = vku::InitStructHelper();
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

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfoKHR stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06548");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, InheritanceRenderingInfoStencilAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a stencil attachment that has an invalid format");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkCommandBufferInheritanceRenderingInfoKHR cmd_buffer_inheritance_rendering_info = vku::InitStructHelper();
    cmd_buffer_inheritance_rendering_info.colorAttachmentCount = 1;
    cmd_buffer_inheritance_rendering_info.pColorAttachmentFormats = &color_format;
    cmd_buffer_inheritance_rendering_info.depthAttachmentFormat = depth_format;
    cmd_buffer_inheritance_rendering_info.stencilAttachmentFormat = depth_format;
    cmd_buffer_inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.pNext = &cmd_buffer_inheritance_rendering_info;
    cmd_buffer_inheritance_info.renderPass = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentSampleCountInfoNV sample_count_info_amd = vku::InitStructHelper();
    sample_count_info_amd.colorAttachmentCount = 1;
    sample_count_info_amd.depthStencilAttachmentSamples = static_cast<VkSampleCountFlagBits>(0x3);

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper(&sample_count_info_amd);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
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

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    CreatePipelineHelper pipe(*this);
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
    RETURN_IF_SKIP(InitFramework())

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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

        VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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

        VkRenderingAttachmentInfoKHR depth_attachment = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkSubpassDescription subpass = {};
    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::CommandBuffer cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.renderPass = render_pass.handle();
    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    VkSubpassDescription subpasses[2] = {};

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);

    vkt::CommandBuffer cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.renderPass = render_pass.handle();
    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.renderArea.extent = {32, 32};
    render_pass_begin_info.framebuffer = framebuffer.handle();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_commandBuffer->NextSubpass(VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, SecondaryCommandBufferContents) {
    TEST_DESCRIPTION("Execute secondary command buffers within active render pass that was not begun with VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    vkt::CommandBuffer cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.renderPass = m_renderPass;
    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper(&dynamic_rendering_features);
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

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

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

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.viewMask = 0x1;

    CreatePipelineHelper pipe(*this);
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    InitRenderTarget();

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

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkFormat color_format = VK_FORMAT_MAX_ENUM;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
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
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT library_features = vku::InitStructHelper(&multiview_features);
    RETURN_IF_SKIP(InitBasicDynamicRendering(&library_features))

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    InitRenderTarget();

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = vku::InitStructHelper();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper lib(*this);
    lib.cb_ci_ = color_blend_state_create_info;
    lib.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib.InitState();
    lib.CreateGraphicsPipeline();

    pipeline_rendering_info.viewMask = 0x1;
    VkPipelineLibraryCreateInfoKHR library_create_info = vku::InitStructHelper(&pipeline_rendering_info);
    library_create_info.libraryCount = 1;
    library_create_info.pLibraries = &lib.pipeline_;

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentLibInfo(&fs_stage.stage_ci, &library_create_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-pStages-06895");  // spec bug
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06626");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, AttachmentSampleCount) {
    TEST_DESCRIPTION("Create pipeline with invalid color attachment samples");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkSampleCountFlagBits color_attachment_samples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;

    VkAttachmentSampleCountInfoAMD samples_info = vku::InitStructHelper();
    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = &color_attachment_samples;

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper(&samples_info);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
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
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT library_features = vku::InitStructHelper(&multiview_features);
    RETURN_IF_SKIP(InitBasicDynamicRendering(&library_features))

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    InitRenderTarget();

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = vku::InitStructHelper();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper lib1(*this);
    lib1.cb_ci_ = color_blend_state_create_info;
    lib1.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    lib1.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib1.InitState();
    lib1.CreateGraphicsPipeline();

    pipeline_rendering_info.viewMask = 0x1;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper lib2(*this);
    lib2.cb_ci_ = color_blend_state_create_info;
    lib2.InitFragmentLibInfo(&fs_stage.stage_ci, &pipeline_rendering_info);
    lib2.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib2.ds_ci_ = ds_ci;
    lib2.InitState();
    lib2.CreateGraphicsPipeline();

    pipeline_rendering_info.viewMask = 0;
    VkPipelineLibraryCreateInfoKHR library_create_info = vku::InitStructHelper();
    library_create_info.libraryCount = 2;
    VkPipeline libraries[2] = {lib1.pipeline_, lib2.pipeline_};
    library_create_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo pipe_ci = vku::InitStructHelper(&library_create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06627");
    vkt::Pipeline pipe(*m_device, pipe_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, LibraryRenderPass) {
    TEST_DESCRIPTION("Create pipeline with invalid library render pass");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT library_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&library_features))

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }

    InitRenderTarget();

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = vku::InitStructHelper();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper lib(*this);
    lib.cb_ci_ = color_blend_state_create_info;
    lib.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    lib.InitState();
    lib.CreateGraphicsPipeline();

    VkPipelineLibraryCreateInfoKHR library_create_info = vku::InitStructHelper(&pipeline_rendering_info);
    library_create_info.libraryCount = 1;
    library_create_info.pLibraries = &lib.pipeline_;

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderModuleCreateInfo fs_ci = vku::InitStructHelper();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    VkPipelineShaderStageCreateInfo fs_stage_ci = vku::InitStructHelper(&fs_ci);
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentLibInfo(&fs_stage_ci, &library_create_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    // If not Frag Output with frag shader, need depth/stencil struct
    pipe.ds_ci_ = vku::InitStructHelper();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06625");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, PipelineMissingMultisampleState) {
    TEST_DESCRIPTION("Create pipeline with missing multisample state");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT library_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&library_features))

    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) graphicsPipelineLibrary";
    }

    InitRenderTarget();

    {
        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.pMultisampleState = nullptr;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06631");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
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
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }
    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingFragmentDensityMapAttachmentInfoEXT rendering_fragment_density = vku::InitStructHelper();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    begin_rendering_info.viewMask = 0;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    if (!ImageFormatIsSupported(instance(), gpu(), image_create_info, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "format doesn't support FRAGMENT_SHADING_RATE_ATTACHMENT_BIT";
    }

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingFragmentDensityMapAttachmentInfoEXT rendering_fragment_density = vku::InitStructHelper();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06158");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, FragmentDensityMapAttachmentCreateFlags) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid image create flags");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingFragmentDensityMapAttachmentInfoEXT rendering_fragment_density = vku::InitStructHelper();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06159");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, FragmentDensityMapAttachmentLayerCount) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid layer count");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }
    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingFragmentDensityMapAttachmentInfoEXT rendering_fragment_density = vku::InitStructHelper();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.viewMask = 0x1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }
    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR not supported";
    }

    InitRenderTarget();

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingFragmentShadingRateAttachmentInfoKHR rendering_fragment_shading_rate = vku::InitStructHelper();
    rendering_fragment_shading_rate.imageView = image_view;
    rendering_fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    rendering_fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    VkRenderingFragmentDensityMapAttachmentInfoEXT rendering_fragment_density =
        vku::InitStructHelper(&rendering_fragment_shading_rate);
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.viewMask = 0x1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitFramework())

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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
    begin_rendering_info.renderArea.offset.x = m_device->phy().limits_.maxFramebufferWidth - 16;
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
    begin_rendering_info.renderArea.offset.y = m_device->phy().limits_.maxFramebufferHeight - 16;
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
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }

    InitRenderTarget();

    VkPhysicalDeviceMultiviewProperties multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_props);

    if (multiview_props.maxMultiviewViewCount == 32) {
        GTEST_SKIP() << "VUID is not testable as maxMultiviewViewCount is 32";
    }

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkFormat format = FindSupportedDepthStencilFormat(gpu());
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, ResolveModeWithNonIntegerColorFormat) {
    TEST_DESCRIPTION("Use invalid resolve mode with non integer color format");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;  // not allowed for format
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveModeWithIntegerColorFormat) {
    TEST_DESCRIPTION("Use invalid resolve mode with integer color format");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_MAX_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06130");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveModeSamples) {
    TEST_DESCRIPTION("Use invalid sample count with resolve mode that is not none");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06861");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewSamples) {
    TEST_DESCRIPTION("Use resolve image view with invalid sample count");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06865");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, AttachmentImageViewLayout) {
    TEST_DESCRIPTION("Use rendering attachment image view with invalid layout");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06135");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid layout");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06136");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewLayoutSeparateDepthStencil) {
    TEST_DESCRIPTION("Use resolve image view with invalid layout");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    const bool nv_shading_rate = IsExtensionsEnabled(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    const bool khr_fragment_shading = IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (!khr_fragment_shading && !nv_shading_rate) {
        GTEST_SKIP() << "shading rate / fragment shading not supported";
    }

    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    const bool nv_shading_rate = IsExtensionsEnabled(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    const bool khr_fragment_shading = IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (!khr_fragment_shading && !nv_shading_rate) {
        GTEST_SKIP() << "shading rate / fragment shading not supported";
    }

    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06140");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewFragmentDensityLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid fragment density layout");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06141");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, ResolveImageViewReadOnlyOptimalLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid read only optimal layout");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06142");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, BeginRenderingFragmentShadingRateImageView) {
    TEST_DESCRIPTION("Test BeginRenderingInfo image view with FragmentShadingRateAttachment.");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR not supported";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    InitRenderTarget();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate = vku::InitStructHelper();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, RenderingInfoColorAttachment) {
    TEST_DESCRIPTION("Test RenderingInfo color attachment.");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkImageObj invalid_image(m_device);
    invalid_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView invalid_image_view = invalid_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = invalid_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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

    const uint32_t max_color_attachments = m_device->phy().limits_.maxColorAttachments + 1;
    std::vector<VkRenderingAttachmentInfoKHR> color_attachments(max_color_attachments);
    for (auto &attachment : color_attachments) {
        attachment = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    InitRenderTarget();

    const bool separate_ds_layouts = IsExtensionsEnabled(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);

    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    VkPhysicalDeviceDepthStencilResolveProperties depth_stencil_resolve_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(depth_stencil_resolve_properties);
    bool has_depth_resolve_mode_average =
        (depth_stencil_resolve_properties.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0;
    bool has_stencil_resolve_mode_average =
        (depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0;
    bool has_stencil_resolve_mode_zero =
        (depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) != 0;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderingAttachmentInfo stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = stencil_image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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

TEST_F(NegativeDynamicRendering, RenderAreaNegativeOffset) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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

TEST_F(NegativeDynamicRendering, ZeroRenderArea) {
    TEST_DESCRIPTION("renderArea set to zero");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-None-08994");
    begin_rendering_info.renderArea = {{0, 0}, {0, 64}};
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-None-08995");
    begin_rendering_info.renderArea = {{0, 0}, {64, 0}};
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, Pipeline) {
    TEST_DESCRIPTION("Use pipeline created with render pass in dynamic render pass.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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

    RETURN_IF_SKIP(InitFramework())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR not supported";
    }

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiviewFeaturesKHR multiview_features = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate = vku::InitStructHelper();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
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

    RETURN_IF_SKIP(InitFramework())

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR not supported";
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceMultiviewFeaturesKHR multiview_features = vku::InitStructHelper(&dynamic_rendering_features);
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(multiview_features);

    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) multiview";
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate = vku::InitStructHelper();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
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

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer1(m_device, &command_pool);
    vkt::CommandBuffer cmd_buffer2(m_device, &command_pool);
    vkt::CommandBuffer cmd_buffer3(m_device, &command_pool);

    VkRenderingInfo suspend_rendering_info = vku::InitStructHelper();
    suspend_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    suspend_rendering_info.layerCount = 1;
    suspend_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkRenderingInfo resume_rendering_info = vku::InitStructHelper();
    resume_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    resume_rendering_info.layerCount = 1;
    resume_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.layerCount = 1;
    rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();

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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06014");

    submit_info.commandBufferCount = 1;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06016");

    command_buffers[1] = cmd_buffer3.handle();
    command_buffers[2] = cmd_buffer2.handle();
    submit_info.commandBufferCount = 3;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06193");

    command_buffers[0] = cmd_buffer2.handle();
    submit_info.commandBufferCount = 1;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, SuspendingRenderPassInstanceQueueSubmit2) {
    TEST_DESCRIPTION("Test suspending render pass instance with QueueSubmit2.");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2Features synchronization2 = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&synchronization2))

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer1(m_device, &command_pool);
    vkt::CommandBuffer cmd_buffer2(m_device, &command_pool);
    vkt::CommandBuffer cmd_buffer3(m_device, &command_pool);

    VkRenderingInfo suspend_rendering_info = vku::InitStructHelper();
    suspend_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    suspend_rendering_info.layerCount = 1;
    suspend_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkRenderingInfo resume_rendering_info = vku::InitStructHelper();
    resume_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    resume_rendering_info.layerCount = 1;
    resume_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.layerCount = 1;
    rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();

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
    command_buffer_submit_info[0] = vku::InitStructHelper();
    command_buffer_submit_info[1] = vku::InitStructHelper();
    command_buffer_submit_info[2] = vku::InitStructHelper();

    command_buffer_submit_info[0].commandBuffer = cmd_buffer1.handle();
    command_buffer_submit_info[1].commandBuffer = cmd_buffer2.handle();

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 2;
    submit_info.pCommandBufferInfos = command_buffer_submit_info;
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06010");

    submit_info.commandBufferInfoCount = 1;
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06012");

    command_buffer_submit_info[1].commandBuffer = cmd_buffer3.handle();
    command_buffer_submit_info[2].commandBuffer = cmd_buffer2.handle();
    submit_info.commandBufferInfoCount = 3;
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06192");

    command_buffer_submit_info[0].commandBuffer = cmd_buffer2.handle();
    submit_info.commandBufferInfoCount = 1;
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, NullDepthStencilExecuteCommands) {
    TEST_DESCRIPTION(
        "Test for NULL depth stencil attachments in dynamic rendering with secondary command buffer with depth stencil format "
        "inheritance info");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    // Create secondary command buffer
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(gpu());

    VkCommandBufferInheritanceRenderingInfoKHR cbiri = vku::InitStructHelper();
    // format is defined, although no image view provided in dynamic rendering
    cbiri.depthAttachmentFormat = depth_stencil_format;
    cbiri.stencilAttachmentFormat = depth_stencil_format;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&cbiri);

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Prepare primary dynamic rendering cmd buffer
    VkImageObj depth_stencil(m_device);
    depth_stencil.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_stencil_view =
        depth_stencil.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo rai = vku::InitStructHelper();
    rai.imageView = depth_stencil_view;
    rai.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR ri = vku::InitStructHelper();
    ri.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    ri.layerCount = 1;
    ri.pDepthAttachment = &rai;
    ri.pStencilAttachment = &rai;
    ri.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    constexpr VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

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
    vkt::RenderPass render_pass(*m_device, rpci);

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    constexpr VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags =
        VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR | VK_RENDERING_SUSPENDING_BIT_KHR | VK_RENDERING_RESUMING_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 0;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    constexpr std::array bad_color_formats = {VK_FORMAT_R8G8B8A8_UINT};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = bad_color_formats.size();
    inheritance_rendering_info.pColorAttachmentFormats = bad_color_formats.data();
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo cmdbuff_bi = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    constexpr std::array bad_color_formats = {VK_FORMAT_R8G8B8A8_UINT};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = bad_color_formats.size();
    inheritance_rendering_info.pColorAttachmentFormats = bad_color_formats.data();
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo cmdbuff_bi = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkImageObj image(m_device);
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());
    if (depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        GTEST_SKIP() << "Insufficient depth-stencil formats supported";
    }

    image.Init(32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(depth_stencil_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = imageView;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_rendering_info.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    VkPhysicalDeviceMultiviewFeaturesKHR mv_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&mv_features))

    if (!mv_features.multiview) {
        GTEST_SKIP() << "multiview feature not supported.";
    }

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.viewMask = 0;
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.viewMask = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    bool amd_samples = IsExtensionsEnabled(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    bool nv_samples = IsExtensionsEnabled(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    if (!amd_samples && !nv_samples) {
        GTEST_SKIP() << "Test requires either VK_AMD_mixed_attachment_samples or VK_NV_framebuffer_mixed_samples";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkSampleCountFlagBits counts = {VK_SAMPLE_COUNT_2_BIT};
    VkAttachmentSampleCountInfoAMD samples_info = vku::InitStructHelper();
    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = &counts;

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info = vku::InitStructHelper(&samples_info);
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkFormat format = VK_FORMAT_R32G32B32A32_UINT;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferInheritanceRenderingInfoKHR inheritanceRenderingInfo =
        vku::InitStructHelper();
    inheritanceRenderingInfo.colorAttachmentCount = 1;
    inheritanceRenderingInfo.pColorAttachmentFormats = &format;
    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&inheritanceRenderingInfo);
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = m_framebuffer;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.begin(&cbbi);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(secondary.handle(), 3, 1, 0, 0);
    secondary.end();
}

TEST_F(NegativeDynamicRendering, CommandBufferInheritanceDepthFormat) {
    TEST_DESCRIPTION(
        "Test VkCommandBufferInheritanceRenderingInfoKHR with depthAttachmentFormat that does not include depth aspect");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    if (!m_device->phy().features().variableMultisampleRate) {
        GTEST_SKIP() << "Test requires (unsupported) variableMultisampleRate";
    }

    auto stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.depthAttachmentFormat = stencil_format;

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cmdbuf_ii = vku::InitStructHelper(&inheritance_rendering_info);
    VkCommandBufferBeginInfo cmdbuf_bi = vku::InitStructHelper();
    cmdbuf_bi.pInheritanceInfo = &cmdbuf_ii;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540");

    vk::BeginCommandBuffer(secondary.handle(), &cmdbuf_bi);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, DeviceGroupRenderArea) {
    TEST_DESCRIPTION("Begin rendering with invaid device group render area.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRect2D renderArea = {{-1, 0}, {64, 64}};

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = vku::InitStructHelper();
    device_group_render_pass_begin_info.deviceMask = 0x1;
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &renderArea;

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper(&device_group_render_pass_begin_info);
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea = {{0, -1}, {64, 64}};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea = {{0, 0}, {0, 64}};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-extent-08998");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea = {{0, 0}, {64, 0}};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-extent-08999");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea = {{0, 0}, {m_device->phy().limits_.maxFramebufferWidth + 1, 64}};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06168");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea = {{0, 0}, {64, m_device->phy().limits_.maxFramebufferWidth + 1}};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06169");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDynamicRendering, MaxFramebufferLayers) {
    TEST_DESCRIPTION("Go over maxFramebufferLayers");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = m_device->phy().limits_.maxFramebufferLayers + 1;
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

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
    vkt::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &imageView, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    m_commandBuffer->BeginRenderPass(rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-None-06161");
    m_commandBuffer->EndRendering();
    m_errorMonitor->VerifyFound();

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass-None-06170");
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();

    VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

    vkt::CommandBuffer primary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

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

    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat depth_format = VK_FORMAT_X8_D24_UNORM_PACK32;

    if (FormatFeaturesAreSupported(gpu_, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        depth_format = VK_FORMAT_D32_SFLOAT;
    }

    VkFormat stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;

    if (FormatFeaturesAreSupported(gpu_, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    VkFormat color_formats = {depth_format};
    auto pipeline_rendering_info = vku::InitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.layout = pl.handle();

    // Invalid color format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Invalid color format array
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Invalid depth format
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06587");
    // TODO (ncesario) Seems impossible hit 06585 without also hitting 06587. Since 06587 happens in stateless validation, 06585
    // never gets triggered, though has been manually tested separately by removing 06587.
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06585");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Invalid stecil format
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06588");
    // TODO (ncesario) Same scenario as with 06585 and 06587
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06586");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // mismatching depth/stencil formats
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = stencil_format;
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06589");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Non-zero viewMask
    color_formats = VK_FORMAT_R8G8B8A8_UNORM;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577");
    pipeline_rendering_info.viewMask = 1;
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, PipelineRenderingViewMaskParameter) {
    TEST_DESCRIPTION("Test pipeline rendering viewmask maximum index");
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&multiview_features))

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
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    VkPhysicalDeviceMultiviewProperties multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_props);

    if (multiview_props.maxMultiviewViewCount == 32) {
        GTEST_SKIP() << "TVUID is not testable as maxMultiviewViewCount is 32";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06578");
    pipeline_rendering_info.viewMask = 1 << multiview_props.maxMultiviewViewCount;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, CreateGraphicsPipeline) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfoKHR rendering_info = vku::InitStructHelper();
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &color_format;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &rendering_info;
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, CreateGraphicsPipelineNoInfo) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled but no rendering info struct.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, DynamicColorBlendAttchment) {
    TEST_DESCRIPTION("Test all color blend attachments are dynamically set at draw time with Dynamic Rendering.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&extended_dynamic_state3_features))

    if (!extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRendering, MismatchingDepthAttachmentFormatInSecondaryCmdBuffer) {
    TEST_DESCRIPTION("Use a pipeline with a depth attachment format that doesn't match that of the dynamic render pass");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,   VK_FORMAT_D32_SFLOAT_S8_UINT,
                                   VK_FORMAT_D16_UNORM,         VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT};
    VkFormat depth_format1 = VK_FORMAT_UNDEFINED;
    VkFormat depth_format2 = VK_FORMAT_UNDEFINED;
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), ds_formats[i], &format_props);

        if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0 ||
            (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            if (depth_format1 == VK_FORMAT_UNDEFINED) {
                depth_format1 = ds_formats[i];
            } else {
                depth_format2 = ds_formats[i];
                break;
            }
        }
    }

    if (depth_format2 == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Could not find 2 ds attachment formats";
    }

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.depthAttachmentFormat = depth_format1;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.cb_ci_.attachmentCount = 0u;
    pipe.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer secondary_cmd_buf(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.depthAttachmentFormat = depth_format2;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper();
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &secondary_cmd_buffer_inheritance_info;

    secondary_cmd_buf.begin(&secondary_cmd_buffer_begin_info);
    vk::CmdBindPipeline(secondary_cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914");
    vk::CmdDraw(secondary_cmd_buf.handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    secondary_cmd_buf.end();
}
