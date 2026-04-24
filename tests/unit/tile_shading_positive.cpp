/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (C) 2026 Qualcomm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "layer_validation_tests.h"
#include "pipeline_helper.h"

class PositiveTileShading : public TileShadingTest {};

void TileShadingTest::InitBasicTileShading() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    AddRequiredFeature(vkt::Feature::tileShadingDepthAttachments);
    AddRequiredFeature(vkt::Feature::tileShadingStencilAttachments);
    AddRequiredFeature(vkt::Feature::tileShadingSampledAttachments);
}

void TileShadingTest::InitTileShadingRenderTarget(const TileShadingRenderTargetConfig &tile_shading_rp_config) {
    m_color_image.Init(*m_device, tile_shading_rp_config.rt_size.width, tile_shading_rp_config.rt_size.height, 1,
                       tile_shading_rp_config.format,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                       VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    m_color_view = m_color_image.CreateView();

    if (tile_shading_rp_config.use_render_pass2) {
        VkAttachmentDescription2 attachment_desc2 = vku::InitStructHelper();
        attachment_desc2.format = tile_shading_rp_config.format;
        attachment_desc2.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc2.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_desc2.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc2.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc2.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc2.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc2.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference2 color_ref2 = vku::InitStructHelper();
        color_ref2.attachment = 0;
        color_ref2.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_ref2.aspectMask = 0;

        VkSubpassDescription2 subpass_desc2 = vku::InitStructHelper();
        subpass_desc2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc2.viewMask = 0;
        subpass_desc2.inputAttachmentCount = 0;
        subpass_desc2.colorAttachmentCount = 1;
        subpass_desc2.pColorAttachments = &color_ref2;

        VkSubpassDependency2 subpass_dependency2 = vku::InitStructHelper();
        subpass_dependency2.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency2.dstSubpass = 0;
        subpass_dependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency2.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency2.srcAccessMask = 0;
        subpass_dependency2.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;

        VkRenderPassCreateInfo2 rp_ci2 = vku::InitStructHelper(&tile_shading_ci);
        rp_ci2.flags = 0;
        rp_ci2.attachmentCount = 1;
        rp_ci2.pAttachments = &attachment_desc2;
        rp_ci2.subpassCount = 1;
        rp_ci2.pSubpasses = &subpass_desc2;
        rp_ci2.dependencyCount = 1;
        rp_ci2.pDependencies = &subpass_dependency2;

        m_tile_shading_render_pass.Init(*m_device, rp_ci2);
    }
    else {
        VkAttachmentDescription attachment_desc{};
        attachment_desc.format = tile_shading_rp_config.format;
        attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc{};
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;

        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
        rp_ci.attachmentCount = 1;
        rp_ci.pAttachments = &attachment_desc;
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;
        rp_ci.dependencyCount = 1;
        rp_ci.pDependencies = &subpass_dependency;

        m_tile_shading_render_pass.Init(*m_device, rp_ci);
    }

    const VkImageView color_view_handle = m_color_view.handle();
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = m_tile_shading_render_pass;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &color_view_handle;
    framebuffer_ci.width = tile_shading_rp_config.rt_size.width;
    framebuffer_ci.height = tile_shading_rp_config.rt_size.height;
    framebuffer_ci.layers = 1;

    m_tile_shading_framebuffer.Init(*m_device, framebuffer_ci);
}

TEST_F(PositiveTileShading, ExecutePerTileExecutionModel) {
    TEST_DESCRIPTION("Execute per-tile execution model for a secondary command buffer.");
    InitBasicTileShading();
    RETURN_IF_SKIP(Init());

    TileShadingRenderTargetConfig tile_shading_rp_config{};
    InitTileShadingRenderTarget(tile_shading_rp_config);

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM | VK_TILE_SHADING_RENDER_PASS_PER_TILE_EXECUTION_BIT_QCOM;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = m_tile_shading_render_pass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = m_tile_shading_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    VkClearValue clear_value{};
    clear_value.color = {{0.f, 0.f, 0.f, 0.f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkCommandBuffer secondary_handle = secondary_command.handle();
    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer, &rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_handle);
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, CreateTileShadingRenderPassWithResolveAttachment) {
    TEST_DESCRIPTION("Create a tile-shading render pass with a valid resolve attachment.");
    InitBasicTileShading();
    RETURN_IF_SKIP(Init());

    {
        std::array<VkAttachmentDescription, 2> attachment_descs{};
        attachment_descs[0].format = m_render_target_fmt;
        attachment_descs[0].samples = VK_SAMPLE_COUNT_4_BIT;
        attachment_descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descs[1].format = m_render_target_fmt;
        attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference resolve_ref{};
        resolve_ref.attachment = 1;
        resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc{};
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;
        subpass_desc.pResolveAttachments = &resolve_ref;

        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper();
        tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_info.tileApronSize = {0, 0};

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_info);
        rp_ci.attachmentCount = attachment_descs.size();
        rp_ci.pAttachments = attachment_descs.data();
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;
        rp_ci.dependencyCount = 1;
        rp_ci.pDependencies = &subpass_dependency;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;
        ASSERT_EQ(vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass), VK_SUCCESS);
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
    {
        std::array<VkAttachmentDescription2, 2> attachment_descs{};
        attachment_descs[0] = vku::InitStructHelper();
        attachment_descs[0].format = m_render_target_fmt;
        attachment_descs[0].samples = VK_SAMPLE_COUNT_4_BIT;
        attachment_descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descs[1] = vku::InitStructHelper();
        attachment_descs[1].format = m_render_target_fmt;
        attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference2 color_ref = vku::InitStructHelper();
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference2 resolve_ref = vku::InitStructHelper();
        resolve_ref.attachment = 1;
        resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription2 subpass_desc = vku::InitStructHelper();
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;
        subpass_desc.pResolveAttachments = &resolve_ref;

        VkSubpassDependency2 subpass_dependency2 = vku::InitStructHelper();
        subpass_dependency2.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency2.dstSubpass = 0;
        subpass_dependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency2.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency2.srcAccessMask = 0;
        subpass_dependency2.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper();
        tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_info.tileApronSize = {0, 0};

        VkRenderPassCreateInfo2 rp_ci2 = vku::InitStructHelper(&tile_info);
        rp_ci2.attachmentCount = attachment_descs.size();
        rp_ci2.pAttachments = attachment_descs.data();
        rp_ci2.subpassCount = 1;
        rp_ci2.pSubpasses = &subpass_desc;
        rp_ci2.dependencyCount = 1;
        rp_ci2.pDependencies = &subpass_dependency2;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;
        ASSERT_EQ(vk::CreateRenderPass2(device(), &rp_ci2, nullptr, &tile_shading_render_pass), VK_SUCCESS);
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
}

TEST_F(PositiveTileShading, CreateTileShadingRenderPassWithUnusedFragmentDensityMapAttachment) {
    TEST_DESCRIPTION("Create a tile-shading render pass with an unused fragment-density-map attachment.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    InitBasicTileShading();
    RETURN_IF_SKIP(Init());

    {
        VkAttachmentDescription attachment_desc{};
        attachment_desc.format = m_render_target_fmt;
        attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc{};
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;

        VkRenderPassFragmentDensityMapCreateInfoEXT fdm_info = vku::InitStructHelper();
        fdm_info.fragmentDensityMapAttachment.attachment = VK_ATTACHMENT_UNUSED;
        fdm_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper(&fdm_info);
        tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_info.tileApronSize = {0, 0};

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_info);
        rp_ci.attachmentCount = 1;
        rp_ci.pAttachments = &attachment_desc;
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;
        ASSERT_EQ(vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass), VK_SUCCESS);
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
    {
        VkAttachmentDescription2 attachment_desc = vku::InitStructHelper();
        attachment_desc.format = m_render_target_fmt;
        attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference2 color_ref = vku::InitStructHelper();
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription2 subpass_desc = vku::InitStructHelper();
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;

        VkRenderPassFragmentDensityMapCreateInfoEXT fdm_info = vku::InitStructHelper();
        fdm_info.fragmentDensityMapAttachment.attachment = VK_ATTACHMENT_UNUSED;
        fdm_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper(&fdm_info);
        tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_info.tileApronSize = {0, 0};

        VkRenderPassCreateInfo2 rp_ci2 = vku::InitStructHelper(&tile_info);
        rp_ci2.attachmentCount = 1;
        rp_ci2.pAttachments = &attachment_desc;
        rp_ci2.subpassCount = 1;
        rp_ci2.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;
        ASSERT_EQ(vk::CreateRenderPass2(device(), &rp_ci2, nullptr, &tile_shading_render_pass), VK_SUCCESS);
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
}

TEST_F(PositiveTileShading, TileShadingDynamicRendering) {
    TEST_DESCRIPTION("Launch a dynamic rendering when tile-shading is enabled.");
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    InitBasicTileShading();
    RETURN_IF_SKIP(Init());

    constexpr uint32_t image_width = 32;
    constexpr uint32_t image_height = 32;
    vkt::Image color_image{*m_device, image_width, image_height, m_render_target_fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT };
    vkt::ImageView color_view = color_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.resolveImageView = VK_NULL_HANDLE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper();
    tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_info.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_info);
    rendering_info.renderArea = {{0, 0}, {image_width, image_height}};
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = nullptr;
    rendering_info.pStencilAttachment = nullptr;

    m_command_buffer.Begin();
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    vk::CmdEndRendering(m_command_buffer);
    m_command_buffer.End();
}

