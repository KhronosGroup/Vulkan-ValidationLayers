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

#include "layer_validation_tests.h"
#include "pipeline_helper.h"

class NegativeTileShading : public TileShadingTest {};

TEST_F(NegativeTileShading, EndPerTileExecutionWithNonTileShadingRenderPass) {
    RETURN_IF_SKIP(InitBasicTileShading());
    InitRenderTarget();

    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer, &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndPerTileExecutionQCOM-None-10666");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndPerTileExecutionQCOM-None-10667");
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, BeginPerTileExecutionWithNonTileShadingRenderPass) {
    RETURN_IF_SKIP(InitBasicTileShading());
    InitRenderTarget();

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer, &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginPerTileExecutionQCOM-None-10664");
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, BeginPerTileExecutionButTileShadingPerTileFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to launch per-tile execution model in a tile-shading render pass scope, "
                     "but tileShadingPerTileDispatch or tileShadingPerTileDraw feature is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    RETURN_IF_SKIP(Init());

    InitTileShadingRenderTarget();

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginPerTileExecutionQCOM-None-10665");
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, RenderPassButTileShadingFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to create a tile-shading render pass, but tileShading feature is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription attachment_desc{};
    attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-tileShading-10658");
    VkRenderPass rp = VK_NULL_HANDLE;
    vk::CreateRenderPass(device(), &rp_ci, nullptr, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, NonZeroTileApronSizeButTileShadingApronFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to use non-zero tile-apron size when creates a render pass, but "
                     "tileShadingApron feature is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    VkAttachmentDescription attachment_desc{};
    attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize, tile_shading_props.maxApronSize};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;

    VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-flags-10659");
    vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass);
    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
}

TEST_F(NegativeTileShading, AnisotropicApronSizeButTileShadingAnisotropicApronFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to use anisotropic apron size when creates a render pass, but "
                     "tileShadingAnisotropicApron feature is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingApron);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    VkAttachmentDescription attachment_desc{};
    attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize, std::max(tile_shading_props.maxApronSize - 1, 1U)};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;

    VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-tileShadingAnisotropicApron-10661");
    vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass);
    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
}

TEST_F(NegativeTileShading, LargerTileApronSize) {
    TEST_DESCRIPTION("Try to use tile apron size larger than maxApronSize when creates a render pass.");
    AddRequiredFeature(vkt::Feature::tileShadingApron);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    VkAttachmentDescription attachment_desc{};
    attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize + 1, tile_shading_props.maxApronSize + 1};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;

    VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-tileApronSize-10662");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-tileApronSize-10663");
    vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass);
    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
}

TEST_F(NegativeTileShading, RenderPassWithPerTileExecutionBit) {
    TEST_DESCRIPTION("Try to create a tile-shading render pass with per-tile-execution bit, but "
                     "tileShadingPerTileDispatch and tileShadingPerTileDraw features are not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    RETURN_IF_SKIP(Init());

    {
        VkAttachmentDescription attachment_desc{};
        attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
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

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM | VK_TILE_SHADING_RENDER_PASS_PER_TILE_EXECUTION_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
        rp_ci.attachmentCount = 1;
        rp_ci.pAttachments = &attachment_desc;
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-vkCreateRenderPass-flags-10646");
        m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-flags-10660");
        vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass);
        m_errorMonitor->VerifyFound();
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
    {
        VkAttachmentDescription2 attachment_desc2 = vku::InitStructHelper();
        attachment_desc2.format = VK_FORMAT_R8G8B8A8_UNORM;
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

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM | VK_TILE_SHADING_RENDER_PASS_PER_TILE_EXECUTION_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo2 rpci2 = vku::InitStructHelper(&tile_shading_ci);
        rpci2.flags = 0;
        rpci2.attachmentCount = 1;
        rpci2.pAttachments = &attachment_desc2;
        rpci2.subpassCount = 1;
        rpci2.pSubpasses = &subpass_desc2;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-vkCreateRenderPass2-flags-10649");
        m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-flags-10660");
        vk::CreateRenderPass2(device(), &rpci2, nullptr, &tile_shading_render_pass);
        m_errorMonitor->VerifyFound();
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
}

TEST_F(NegativeTileShading, DynamicRenderingButTileShadingFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to launch a tile-shading dynamic rendering, but tileShading feature isn't enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    constexpr uint32_t image_width = 64;
    constexpr uint32_t image_height = 64;
    vkt::Image color_image{*m_device, image_width, image_height, m_render_target_fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {image_width, image_height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-tileShading-10658");
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DynamicRenderingWithApronSizeButRequiredFeaturesNotEnabled) {
    TEST_DESCRIPTION("Try to launch a tile-shading dynamic rendering with specified anisotropic apron size, "
                     "but tileShadingApron and tileShadingAnisotropicApron features aren't enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::tileShading);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    constexpr uint32_t image_width = 64;
    constexpr uint32_t image_height = 64;
    vkt::Image color_image{*m_device, image_width, image_height, m_render_target_fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize, std::max(tile_shading_props.maxApronSize - 1, 1U)};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {image_width, image_height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-flags-10659");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassTileShadingCreateInfoQCOM-tileShadingAnisotropicApron-10661");
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, RenderPassWithUndefinedFormatResolveAttachment) {
    TEST_DESCRIPTION("Try to create a tile-shading render pass, but provides a undefined format resolve attachment.");
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
        attachment_descs[1].format = VK_FORMAT_UNDEFINED;
        attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
        rp_ci.attachmentCount = attachment_descs.size();
        rp_ci.pAttachments = attachment_descs.data();
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06698");
        m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo-pResolveAttachments-10647");
        vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass);
        m_errorMonitor->VerifyFound();
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
        attachment_descs[1].format = VK_FORMAT_UNDEFINED;
        attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo2 rp_ci2 = vku::InitStructHelper(&tile_shading_ci);
        rp_ci2.attachmentCount = attachment_descs.size();
        rp_ci2.pAttachments = attachment_descs.data();
        rp_ci2.subpassCount = 1;
        rp_ci2.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-pResolveAttachments-10650");
        m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription2-format-09332");
        vk::CreateRenderPass2(device(), &rp_ci2, nullptr, &tile_shading_render_pass);
        m_errorMonitor->VerifyFound();
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
}

TEST_F(NegativeTileShading, RenderPassWithFragmentDensityMapAttachment) {
    TEST_DESCRIPTION("Try to create a tile-shading render pass, but provides an available fragment-density-map attachment.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicTileShading());

    {
        std::array<VkAttachmentDescription, 2> attachment_descs{};
        attachment_descs[0].format = m_render_target_fmt;
        attachment_descs[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descs[1].format = VK_FORMAT_R8G8_UNORM;
        attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[1].finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc{};
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;

        VkRenderPassFragmentDensityMapCreateInfoEXT fdm_ci = vku::InitStructHelper();
        fdm_ci.fragmentDensityMapAttachment.attachment = 1;
        fdm_ci.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper(&fdm_ci);
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
        rp_ci.attachmentCount = attachment_descs.size();
        rp_ci.pAttachments = attachment_descs.data();
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo-fragmentDensityMapAttachment-10648");
        vk::CreateRenderPass(device(), &rp_ci, nullptr, &tile_shading_render_pass);
        m_errorMonitor->VerifyFound();
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
    {
        std::array<VkAttachmentDescription2, 2> attachment_descs{};
        attachment_descs[0] = vku::InitStructHelper();
        attachment_descs[0].format = m_render_target_fmt;
        attachment_descs[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descs[1] = vku::InitStructHelper();
        attachment_descs[1].format = VK_FORMAT_R8G8_UNORM;
        attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descs[1].finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        VkAttachmentReference2 color_ref = vku::InitStructHelper();
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription2 subpass_desc = vku::InitStructHelper();
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;

        VkRenderPassFragmentDensityMapCreateInfoEXT fdm_ci = vku::InitStructHelper();
        fdm_ci.fragmentDensityMapAttachment.attachment = 1;
        fdm_ci.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper(&fdm_ci);
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo2 rp_ci2 = vku::InitStructHelper(&tile_shading_ci);
        rp_ci2.attachmentCount = attachment_descs.size();
        rp_ci2.pAttachments = attachment_descs.data();
        rp_ci2.subpassCount = 1;
        rp_ci2.pSubpasses = &subpass_desc;

        VkRenderPass tile_shading_render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-fragmentDensityMapAttachment-10651");
        vk::CreateRenderPass2(device(), &rp_ci2, nullptr, &tile_shading_render_pass);
        m_errorMonitor->VerifyFound();
        vk::DestroyRenderPass(device(), tile_shading_render_pass, nullptr);
    }
}

TEST_F(NegativeTileShading, DynamicRenderingInSimultaneousUseBitCommandBuffer) {
    TEST_DESCRIPTION("Try to launch a dynamic rendering in a recorded command buffer "
                     "with simultaneous-use-bit used.");
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicTileShading());

    constexpr uint32_t image_width = 32;
    constexpr uint32_t image_height = 32;
    vkt::Image color_image{*m_device, image_width, image_height, m_render_target_fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {image_width, image_height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-flags-10641");
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DynamicRenderingWithPerTileExecutionBit) {
    TEST_DESCRIPTION("Try to launch a tile-shading dynamic rendering but per-tile-execution bit is used.");
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicTileShading());

    constexpr uint32_t image_width = 32;
    constexpr uint32_t image_height = 32;
    vkt::Image color_image{*m_device, image_width, image_height, m_render_target_fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM | VK_TILE_SHADING_RENDER_PASS_PER_TILE_EXECUTION_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {image_width, image_height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-flags-10642");
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DynamicRenderingButPerTileExecutionModelEnabled) {
    TEST_DESCRIPTION("Try to end a tile-shading dynamic rendering, but per-tile-execution model is still enabled.");
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = m_color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.resolveImageView = VK_NULL_HANDLE;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = clear_value;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndRendering-None-10645");
    vk::CmdEndRendering(m_command_buffer);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    vk::CmdEndRendering(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, RenderPassWithSimultaneousUseBit) {
    TEST_DESCRIPTION("Try to launch a tile-shading render pass in a recorded command buffer "
                     "with simultaneous-use-bit used.");
    RETURN_IF_SKIP(InitBasicTileShading());

    tile_shading_rp_config.use_render_pass2 = true;
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

    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = VK_SUBPASS_CONTENTS_INLINE;

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass2-flags-10652");
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdBeginRenderPass2(m_command_buffer, &rp_begin_info, &subpass_begin_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, RenderPassButPerTileExecutionModelEnabled) {
    TEST_DESCRIPTION("Try to end a tile-shading render pass, but per-tile execution model is still enabled.");
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

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndRenderPass-None-10653");
    vk::CmdEndRenderPass(m_command_buffer);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DynamicRenderingWithFragmentDensityMapAttachment) {
    TEST_DESCRIPTION("Try to launch a dynamic rendering when tile-shading is enabled and fragment-density-map attachment is provided.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    AddRequiredFeature(vkt::Feature::fragmentDensityMapNonSubsampledImages);
    RETURN_IF_SKIP(InitBasicTileShading());

    vkt::Image color_image{*m_device, 32, 32, m_render_target_fmt,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT};
    vkt::ImageView color_view = color_image.CreateView();
    vkt::Image fdm_image{*m_device, 32, 32, VK_FORMAT_R8G8_UNORM, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT};
    vkt::ImageView fdm_view = fdm_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingFragmentDensityMapAttachmentInfoEXT fdm_attachment = vku::InitStructHelper();
    fdm_attachment.imageView = fdm_view;
    fdm_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper(&fdm_attachment);
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-imageView-10643");
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, SubpassDescriptionWithZeroApronSize) {
    TEST_DESCRIPTION("Try to create a tile-shading render pass, but provides a tile-shading-apron subpass "
                     "description with zero apron size.");
    RETURN_IF_SKIP(InitBasicTileShading());

    {
        VkAttachmentDescription attachment_desc{};
        attachment_desc.flags = 0;
        attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc{};
        subpass_desc.flags = VK_SUBPASS_DESCRIPTION_TILE_SHADING_APRON_BIT_QCOM;
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.inputAttachmentCount = 0;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_ref;

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
        rp_ci.flags = 0;
        rp_ci.attachmentCount = 1;
        rp_ci.pAttachments = &attachment_desc;
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass_desc;

        VkRenderPass rp = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription-flags-10683");
        vk::CreateRenderPass(device(), &rp_ci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
    }
    {
        VkAttachmentDescription2 attachment_desc2 = vku::InitStructHelper();
        attachment_desc2.flags = 0;
        attachment_desc2.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachment_desc2.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc2.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc2.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc2.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc2.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc2.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc2.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference2 color_ref2 = vku::InitStructHelper();
        color_ref2.attachment = 0;
        color_ref2.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_ref2.aspectMask = 0;

        VkSubpassDescription2 subpass_desc2 = vku::InitStructHelper();
        subpass_desc2.flags = VK_SUBPASS_DESCRIPTION_TILE_SHADING_APRON_BIT_QCOM;
        subpass_desc2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc2.viewMask = 0;
        subpass_desc2.inputAttachmentCount = 0;
        subpass_desc2.colorAttachmentCount = 1;
        subpass_desc2.pColorAttachments = &color_ref2;

        VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
        tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
        tile_shading_ci.tileApronSize = {0, 0};

        VkRenderPassCreateInfo2 rpci2 = vku::InitStructHelper(&tile_shading_ci);
        rpci2.flags = 0;
        rpci2.attachmentCount = 1;
        rpci2.pAttachments = &attachment_desc2;
        rpci2.subpassCount = 1;
        rpci2.pSubpasses = &subpass_desc2;

        VkRenderPass rp = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-flags-10683");
        vk::CreateRenderPass2(device(), &rpci2, nullptr, &rp);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTileShading, BeginNonTileShadingCommandBufferWithTileShadingRenderPass) {
    TEST_DESCRIPTION("Begin a command buffer without tile-shading-enable bit, but a tile-shading render pass "
                     "is included in the inheritance information.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;
    tile_shading_ci.flags = 0;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = m_tile_shading_render_pass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = m_tile_shading_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-10617");
    vk::BeginCommandBuffer(secondary_command, &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, BeginTileShadingCommandBufferWithNonTileShadingRenderPass) {
    TEST_DESCRIPTION("Begin a command buffer with tile-shading-enable bit, but the inheritance information "
                     "includes a non-tile-shading render pass.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitRenderTarget();

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = {0, 0};
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = RenderPass();
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = &inheritance_info;

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-10618");
     vk::BeginCommandBuffer(secondary_command, &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, BeginTileShadingCommandBufferButHasInconsistentApronSize) {
    TEST_DESCRIPTION("Begin a command buffer with tile-shading-enable bit, but the inheritance information "
                     "has tileApronSize that isn't equal to that tileApronSize used to create render pass.");
    AddRequiredFeature(vkt::Feature::tileShadingApron);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize, tile_shading_props.maxApronSize};
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = m_tile_shading_render_pass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = m_tile_shading_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-10619");
     vk::BeginCommandBuffer(secondary_command, &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, ExecuteTileShadingCommandWithoutTileShadingEnableBit) {
    TEST_DESCRIPTION("Execute a secondary command inside a tile-shading render pass scope, but that secondary "
                     "command hasn't been recorded with tile-shading-enable bit.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;
    tile_shading_ci.flags = 0;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = m_tile_shading_render_pass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = m_tile_shading_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkCommandBufferBeginInfo-flags-10617");
    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-10620");
    m_command_buffer.ExecuteCommands(secondary_command);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, ExecuteTileShadingCommandInPerTileExecutionModelScope) {
    TEST_DESCRIPTION("Execute a secondary command inside the per-tile execution model scope, but that secondary "
                     "command only has been recorded with tile-shading-enable bit.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = tile_shading_rp_config.tile_apron_size;
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = m_tile_shading_render_pass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = m_tile_shading_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

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

    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-10621");
    m_command_buffer.ExecuteCommands(secondary_command);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, ExecuteTileShadingCommandWithInconsistentTileApronSize) {
    TEST_DESCRIPTION("Execute a secondary command inside a tile-shading render pass scope, but that secondary "
                     "command has been recorded with a tile-apron size inconsistent with that of the render pass.");
    AddRequiredFeature(vkt::Feature::tileShadingApron);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize, tile_shading_props.maxApronSize};
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = m_tile_shading_render_pass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = m_tile_shading_framebuffer;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-tileApronSize-10622");
    m_command_buffer.ExecuteCommands(secondary_command);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, ExecuteTileShadingCommandInNonTileShadingRenderPass) {
    TEST_DESCRIPTION("Execute a secondary command inside a non-tile-shading render pass scope, but that secondary "
                     "command has been recorded with tile-shading-enable bit.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitRenderTarget();

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = {0, 0};
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = RenderPass();
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = RenderPass();
    rp_begin_info.framebuffer = Framebuffer();
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = {m_width, m_height};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-10623");
    m_command_buffer.ExecuteCommands(secondary_command);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, ExecutePerTileExecutionCommandButPerTileExecutionModelNotEnabled) {
    TEST_DESCRIPTION("Execute a secondary command that has been recorded with per-tile-execution bit, "
                     "but the per-tile execution model isn't enabled.");
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

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea.offset = {0, 0};
    rp_begin_info.renderArea.extent = tile_shading_rp_config.rt_size;
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_value;

    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-10624");
    m_command_buffer.ExecuteCommands(secondary_command);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, ExecuteNonZeroTileSizeCommand) {
    TEST_DESCRIPTION("Execute a secondary command without a render pass, but that secondary "
                     "command has been recorded with a non-zero tile-apron size.");
    AddRequiredFeature(vkt::Feature::tileShadingApron);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    vkt::CommandBuffer secondary_command{*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.tileApronSize = {tile_shading_props.maxApronSize, tile_shading_props.maxApronSize};
    tile_shading_ci.flags = 0;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&tile_shading_ci);
    inheritance_info.renderPass = nullptr;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = nullptr;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = &inheritance_info;

    VkClearValue clear_value{};
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    secondary_command.Begin(&begin_info);
    secondary_command.End();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-tileApronSize-10625");
    m_command_buffer.ExecuteCommands(secondary_command);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, LaunchTileAttachmentMemoryBarrierButUseWrongStageMask) {
    TEST_DESCRIPTION("Try to launch tile attachment memory barrier but provides a wrong stage mask.");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTileShading());

    m_command_buffer.Begin();
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-10670");
        m_command_buffer.Barrier(barrier2);
        m_errorMonitor->VerifyFound();
    }
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-10671");
        m_command_buffer.Barrier(barrier2);
        m_errorMonitor->VerifyFound();
    }
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-10670");
        m_command_buffer.Barrier(barrier2);
        m_errorMonitor->VerifyFound();
    }
    {
        VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-10671");
        m_command_buffer.Barrier(barrier2);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DebugMarkerInsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Try to begin or end debug marker inside the per-tile execution model scope.");
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

    {
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdDebugMarkerBeginEXT-None-10614");
        vk::CmdDebugMarkerBeginEXT(m_command_buffer, &marker_info);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_command_buffer.Reset();
    }
    {
        m_command_buffer.Begin();
        vk::CmdDebugMarkerBeginEXT(m_command_buffer, &marker_info);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdDebugMarkerEndEXT-None-10615");
        vk::CmdDebugMarkerEndEXT(m_command_buffer);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        vk::CmdDebugMarkerEndEXT(m_command_buffer);
        m_command_buffer.End();
    }
}

TEST_F(NegativeTileShading, ClearAttachmentInsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Try to clear attachment inside the per-tile execution model scope.");
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
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-None-10616");
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, WriteTimestampInsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Try to write timestamp inside the per-tile execution model scope.");
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
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-None-10640");
        vk::CmdWriteTimestamp(m_command_buffer,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              query_pool, 0);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_command_buffer.Reset();
    }
    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp2-None-10639");
        vk::CmdWriteTimestamp2(m_command_buffer,
                               VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                               query_pool, 0);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeTileShading, WaitEventsInsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Try to wait events inside the per-tile execution model scope.");
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
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-None-10655");
        vk::CmdWaitEvents(m_command_buffer,
                          1, &event_handle,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          0, nullptr,
                          0, nullptr,
                          0, nullptr);
        m_errorMonitor->VerifyFound();
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
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-None-10654");
        vk::CmdWaitEvents2(m_command_buffer, 1, &event_handle, &dependency_info);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeTileShading, TransformFeedbackInsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Try to begin or end transform feedback inside the per-tile execution model scope.");
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

    {
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, xfb_pipe);
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer,
                                               0, 1, &xfb_handle, &xfb_offset, &xfb_size);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-10656");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer,
                                         0,
                                         0,
                                         nullptr,
                                         nullptr);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_command_buffer.Reset();
    }
    {
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, xfb_pipe);
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer,
                                               0, 1, &xfb_handle, &xfb_offset, &xfb_size);
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer,
                                         0,
                                         0,
                                         nullptr,
                                         nullptr);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-None-10657");
        vk::CmdEndTransformFeedbackEXT(m_command_buffer,
                                       0,
                                       0,
                                       nullptr,
                                       nullptr);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        vk::CmdEndTransformFeedbackEXT(m_command_buffer,
                                       0,
                                       0,
                                       nullptr,
                                       nullptr);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeTileShading, QueryInsidePerTileExecutionModelScope) {
    TEST_DESCRIPTION("Try to begin or end query inside the per-tile execution model scope.");
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

    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-10681");
        vk::CmdBeginQuery(m_command_buffer, query_pool, 0, 0);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_command_buffer.Reset();
    }
    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
        vk::CmdBeginQuery(m_command_buffer, query_pool, 0, 0);
        m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-None-07007");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-None-10682");
        vk::CmdEndQuery(m_command_buffer, query_pool, 0);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        m_command_buffer.EndRenderPass();
        vk::CmdEndQuery(m_command_buffer, query_pool, 0);
        m_command_buffer.End();
    }
}
