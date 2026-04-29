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
