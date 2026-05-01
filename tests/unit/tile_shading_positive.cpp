/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 * Copyright (C) 2026 Qualcomm Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "binding.h"
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
    RETURN_IF_SKIP(Init());
}

void TileShadingTest::InitTileShadingRenderTarget() {
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
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

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
    secondary_command.Begin(&begin_info);
    secondary_command.End();

    VkClearValue clear_value{};
    clear_value.color = {{0.f, 0.f, 0.f, 0.f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_command_buffer.ExecuteCommands(secondary_command);
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, RenderPassWithResolveAttachment) {
    TEST_DESCRIPTION("Create a tile-shading render pass with a valid resolve attachment.");
    RETURN_IF_SKIP(InitBasicTileShading());

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

        vkt::RenderPass rp(*m_device, rp_ci);
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

        vkt::RenderPass rp(*m_device, rp_ci2);
    }
}

TEST_F(PositiveTileShading, RenderPassWithUnusedFragmentDensityMapAttachment) {
    TEST_DESCRIPTION("Create a tile-shading render pass with an unused fragment-density-map attachment.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicTileShading());

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

        vkt::RenderPass rp(*m_device, rp_ci);
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

        vkt::RenderPass rp(*m_device, rp_ci2);
    }
}

TEST_F(PositiveTileShading, DynamicRendering) {
    TEST_DESCRIPTION("Launch a dynamic rendering when tile-shading is enabled.");
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicTileShading());

    vkt::Image color_image{*m_device, 32, 32, m_render_target_fmt,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT};
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
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, LaunchTileAttachmentMemoryBarrier) {
    TEST_DESCRIPTION("Launch correct tile attachment memory barrier.");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTileShading());

    m_command_buffer.Begin();
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        m_command_buffer.Barrier(barrier2);
    }
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
        m_command_buffer.Barrier(barrier2);
    }
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM;
        m_command_buffer.Barrier(barrier2);
    }
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM;
        m_command_buffer.Barrier(barrier2);
    }
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, LaunchQueryOutsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Launch query outside the per-tile execution model scope.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    vkt::QueryPool query_pool{*m_device, VK_QUERY_TYPE_OCCLUSION, 1};

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0, 0);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, WaitEventsOutsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Wait events outside the per-tile execution model scope.");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    vkt::Event event{*m_device};
    const VkEvent event_handle = event.handle();

    VkClearValue clear_value = {};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    {
        m_command_buffer.Begin();
        m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                    0, nullptr, 0, nullptr, 0, nullptr);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_command_buffer.Reset();
    }
    {
        VkMemoryBarrier2 mem_barrier2 = vku::InitStructHelper();
        mem_barrier2.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        mem_barrier2.srcAccessMask = 0;
        mem_barrier2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        mem_barrier2.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM;

        VkDependencyInfo dependency_info = vku::InitStructHelper();
        dependency_info.memoryBarrierCount = 1;
        dependency_info.pMemoryBarriers = &mem_barrier2;

        m_command_buffer.Begin();
        vk::CmdWaitEvents2(m_command_buffer, 1, &event_handle, &dependency_info);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(PositiveTileShading, WriteTimestampOutsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Write timestamp outside the per-tile execution model scope.");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    vkt::QueryPool query_pool{*m_device, VK_QUERY_TYPE_TIMESTAMP, 1};

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
        vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_command_buffer.Reset();
    }
    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
        vk::CmdWriteTimestamp2(m_command_buffer, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, query_pool, 0);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(PositiveTileShading, LaunchTransformFeedbackOutsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Launch transform feedback outside the per-tile execution model scope.");
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    const char *vs_source = R"glsl(
        #version 460

        layout(xfb_buffer = 0, xfb_stride = 16) out;
        layout(location = 0, xfb_offset = 0) out vec4 xfb_out;

        vec2 positions[3] = vec2[3](
            vec2( 0.0, -0.5),
            vec2( 0.5,  0.5),
            vec2(-0.5,  0.5)
        );

        void main() {
            vec4 out_position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
            xfb_out = out_position;
            gl_Position = out_position;
        }
    )glsl";
    const char *fs_source = R"glsl(
        #version 460

        layout(location = 0) out vec4 out_color;

        void main() {
            out_color = vec4(1.0);
        }
    )glsl";

    VkShaderObj vs{*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3};
    VkShaderObj fs{*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3};

    CreatePipelineHelper xfb_pipe{*this};
    xfb_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    xfb_pipe.gp_ci_.renderPass = m_tile_shading_render_pass;
    xfb_pipe.CreateGraphicsPipeline();

    constexpr VkDeviceSize xfb_offset = 0;
    constexpr VkDeviceSize xfb_size = 4096;
    vkt::Buffer xfb_buffer{*m_device, xfb_size,
                           VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    VkBuffer xfb_handle = xfb_buffer.handle();

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, xfb_pipe);
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer, 0, 1, &xfb_handle, &xfb_offset, &xfb_size);
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer, 0, 0, nullptr, nullptr);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    vk::CmdEndTransformFeedbackEXT(m_command_buffer, 0, 0, nullptr, nullptr);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, ClearAttachmentOutsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Clear attachment outside the per-tile execution model scope.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkClearAttachment clear_attachment{};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.colorAttachment = 0;
    clear_attachment.clearValue = clear_value;

    VkClearRect clear_rect{};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = tile_shading_rp_config.rt_size;
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveTileShading, LaunchDebugMarkerOutsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Launch debug marker outside the per-tile execution model scope.");
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    VkDebugMarkerMarkerInfoEXT marker_info = vku::InitStructHelper();
    marker_info.pMarkerName = "tile-shading-marker";
    marker_info.color[0] = 1.0f;
    marker_info.color[1] = 0.0f;
    marker_info.color[2] = 0.0f;
    marker_info.color[3] = 1.0f;

    m_command_buffer.Begin();
    vk::CmdDebugMarkerBeginEXT(m_command_buffer, &marker_info);
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    vk::CmdDebugMarkerEndEXT(m_command_buffer);
    m_command_buffer.End();
}
