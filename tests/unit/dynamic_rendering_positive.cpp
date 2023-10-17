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

void DynamicRenderingTest::InitBasicDynamicRendering(void* pNextFeatures) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper(pNextFeatures);
    GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering , skipping.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &dynamic_rendering_features));
}

TEST_F(PositiveDynamicRendering, BasicUsage) {
    TEST_DESCRIPTION("Most simple way to use dynamic rendering");
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
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, Draw) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
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
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, DrawMultiBind) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering and multiple CmdBindPipeline calls.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto depth_format = FindSupportedDepthOnlyFormat(gpu());

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    CreatePipelineHelper pipe2(*this);
    pipe2.InitState();
    pipe2.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe2.gp_ci_.layout = pl.handle();
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();

    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, BeginQuery) {
    TEST_DESCRIPTION("Test calling vkCmdBeginQuery with a dynamic render pass.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&sync2_features))

    InitRenderTarget();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;

    vkt::QueryPool query_pool;
    query_pool.init(*m_device, qpci);

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, PipeWithDiscard) {
    TEST_DESCRIPTION("Create dynamic rendering pipeline with rasterizer discard.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;  // D16_UNORM has guaranteed support

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.ds_ci_ = ds_ci;
    pipe.gp_ci_.layout = pl.handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, UseStencilAttachmentWithIntegerFormatAndDepthStencilResolve) {
    TEST_DESCRIPTION("Use stencil attachment with integer format and depth stencil resolve extension");
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_S8_UINT format not supported";
    }

    VkPhysicalDeviceDepthStencilResolveProperties depth_stencil_resolve_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(depth_stencil_resolve_properties);
    if ((depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
        GTEST_SKIP() << "VK_RESOLVE_MODE_AVERAGE_BIT not supported for VK_FORMAT_S8_UINT";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_S8_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_create_info);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    stencil_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.pStencilAttachment = &stencil_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, FragmentDensityMapSubsampledBit) {
    TEST_DESCRIPTION("Test creating an image with subsampled bit.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT);

    image_ci.flags = 0;
    image_ci.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

    VkImageObj fdm_image(m_device);
    fdm_image.Init(image_ci);
    VkImageView fdm_image_view = fdm_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map = vku::InitStructHelper();
    fragment_density_map.imageView = fdm_image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    begin_rendering_info.pNext = &fragment_density_map;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
}

TEST_F(PositiveDynamicRendering, SuspendResumeDraw) {
    TEST_DESCRIPTION("Resume and suspend at vkCmdBeginRendering time");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer cb1(m_device, m_commandPool);
    vkt::CommandBuffer cb2(m_device, m_commandPool);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT_KHR | VK_RENDERING_SUSPENDING_BIT_KHR;
    cb1.begin();
    cb1.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cb1.handle(), 3, 1, 0, 0);
    cb1.EndRendering();
    cb1.end();

    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT_KHR;
    cb2.begin();
    cb2.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(cb2.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cb2.handle(), 3, 1, 0, 0);
    cb2.EndRendering();
    cb2.end();

    std::array<VkCommandBuffer, 3> cbs = {m_commandBuffer->handle(), cb1.handle(), cb2.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = static_cast<uint32_t>(cbs.size());
    submit.pCommandBuffers = cbs.data();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveDynamicRendering, CreateGraphicsPipeline) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    char const* fsSource = R"glsl(
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

    VkAttachmentReference attachment = {};
    attachment.layout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.attachment = 0;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attachment;
    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.gp_ci_.pNext = &rendering_info;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, CreateGraphicsPipelineNoInfo) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled but no rendering info struct.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())
    char const* fsSource = R"glsl(
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

    VkAttachmentReference attachment = {};
    attachment.layout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.attachment = 0;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attachment;
    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, CommandDrawWithShaderTileImageRead) {
    TEST_DESCRIPTION("vkCmdDraw* with shader tile image read extension using dynamic Rendering Tests.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDynamicRendering(&shader_tile_image_features))

    // shader tile image read features not supported skip the test.
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
    vk::CmdSetDepthWriteEnable(m_commandBuffer->handle(), false);
    vk::CmdSetStencilWriteMask(m_commandBuffer->handle(), VK_STENCIL_FACE_FRONT_BIT, 0);
    vk::CmdSetStencilWriteMask(m_commandBuffer->handle(), VK_STENCIL_FACE_BACK_BIT, 0);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, DualSourceBlending) {
    TEST_DESCRIPTION("Test drawing with dynamic rendering and dual source blending.");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    if (!m_device->phy().features().dualSrcBlend) {
        GTEST_SKIP() << "Test requires (unsupported) dualSrcBlend";
    }

    char const* fsSource = R"glsl(
        #version 450
        layout(location = 0) out vec4 c0;
        void main(){
           c0 = vec4(1.0f);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState cb_attachments = {};
    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.cb_attachments_[0] = cb_attachments;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
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

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, ExecuteCommandsWithNullImageView) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands with an inherited image format of VK_FORMAT_UNDEFINED inside a render pass begun with "
        "CmdBeginRendering where the same image is specified as null");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    constexpr std::array bad_color_formats = {VK_FORMAT_UNDEFINED};

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

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, SuspendPrimaryResumeInSecondary) {
    TEST_DESCRIPTION("Suspend in primary and resume in secondary");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_formats = {VK_FORMAT_UNDEFINED};
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT_KHR;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    // Primary suspends render
    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();

    // Secondary resumes render
    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT_KHR;
    secondary.begin();
    secondary.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(secondary.handle(), 3, 1, 0, 0);
    secondary.EndRendering();
    secondary.end();

    // Execute secondary
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());

    m_commandBuffer->end();

    // Submit
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveDynamicRendering, SuspendSecondaryResumeInPrimary) {
    TEST_DESCRIPTION("Suspend in secondary and resume in primary");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    char const* fsSource = R"glsl(
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

    VkFormat color_formats = {VK_FORMAT_UNDEFINED};
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = 0;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    // First primary with secondary that suspends render
    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    begin_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT_KHR;
    secondary.begin();
    secondary.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(secondary.handle(), 3, 1, 0, 0);
    secondary.EndRendering();
    secondary.end();

    // Execute secondary
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());

    m_commandBuffer->end();

    // Second Primary resumes render
    vkt::CommandBuffer cb(m_device, m_commandPool);
    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT_KHR;
    cb.begin();
    cb.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(cb.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cb.handle(), 3, 1, 0, 0);
    cb.EndRendering();
    cb.end();

    // Submit
    std::array<VkCommandBuffer, 2> cbs = {m_commandBuffer->handle(), cb.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = static_cast<uint32_t>(cbs.size());
    submit.pCommandBuffers = cbs.data();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveDynamicRendering, WithShaderTileImageAndBarrier) {
    TEST_DESCRIPTION("Test setting memory barrier with shader tile image features are enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper(&sync2_features);
    RETURN_IF_SKIP(InitBasicDynamicRendering(&shader_tile_image_features))
    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    InitRenderTarget();

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    VkMemoryBarrier2KHR memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.pBufferMemoryBarriers = VK_NULL_HANDLE;
    dependency_info.imageMemoryBarrierCount = 0;
    dependency_info.pImageMemoryBarriers = VK_NULL_HANDLE;

    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_commandBuffer->EndRendering();

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &memory_barrier, 0,
                           nullptr, 0, nullptr);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, MatchingAttachmentFormats) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with attachment specified as VK_NULL_HANDLE in VkRenderingInfoKHR, and with corresponding "
        "format in VkPipelineRenderingCreateInfoKHR set to VK_FORMAT_UNDEFINED");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_UNDEFINED};
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
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    CreatePipelineHelper pipeline_depth(*this, 0);
    pipeline_depth.InitState();
    pipeline_depth.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_depth.gp_ci_.layout = pl.handle();
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_depth.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

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
    const VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
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
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
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
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
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
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, MatchingAttachmentFormats2) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with dynamicRenderingUnusedAttachments enabled and matching formats");
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

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipeline_color(*this);
    pipeline_color.InitState();
    pipeline_color.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_color.gp_ci_.layout = pl.handle();
    pipeline_color.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_color.CreateGraphicsPipeline();

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipeline_depth(*this, 0);
    pipeline_depth.InitState();
    pipeline_depth.shader_stages_[1] = fs.GetStageCreateInfo();
    pipeline_depth.gp_ci_.layout = pl.handle();
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.gp_ci_.pNext = &pipeline_rendering_info;
    pipeline_depth.CreateGraphicsPipeline();

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
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

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
        // Null color image view
        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();

        // Matching color formats
        color_attachment.imageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();
        color_attachment.imageView = VK_NULL_HANDLE;
    }

    {
        // Null depth image view
        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();

        // Matching depth format
        depth_stencil_attachment.imageView = depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();
        depth_stencil_attachment.imageView = VK_NULL_HANDLE;
    }

    {
        // Null stencil image view
        VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();

        // Matching stencil format
        depth_stencil_attachment.imageView = depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_STENCIL_BIT);
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRendering();
        depth_stencil_attachment.imageView = VK_NULL_HANDLE;
    }

    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, ExecuteCommandsFlags) {
    TEST_DESCRIPTION("Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has same flags");
    RETURN_IF_SKIP(InitBasicDynamicRendering())

    constexpr VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    inheritance_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cb_inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);
    cb_inheritance_info.renderPass = VK_NULL_HANDLE;
    cb_inheritance_info.subpass = 0;
    cb_inheritance_info.framebuffer = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo cb_begin_info = vku::InitStructHelper();
    cb_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cb_begin_info.pInheritanceInfo = &cb_inheritance_info;
    secondary.begin(&cb_begin_info);
    secondary.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveDynamicRendering, ColorAttachmentMismatch) {
    TEST_DESCRIPTION("colorAttachmentCount and attachmentCount don't match but it is dynamically ignored");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    RETURN_IF_SKIP(InitBasicDynamicRendering(&extended_dynamic_state3_features))
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

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 0;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    pipe.CreateGraphicsPipeline();
}
