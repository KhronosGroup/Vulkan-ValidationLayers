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
#include "shader_object_helper.h"

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

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&tile_shading_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    tile_shading_rp_config.tile_apron_size = {tile_shading_props.maxApronSize, tile_shading_props.maxApronSize};
    InitTileShadingRenderTarget();

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

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci_primary = vku::InitStructHelper();
    tile_shading_ci_primary.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci_primary.tileApronSize = {0, 0};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci_primary);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &subpass_dependency;

    vkt::RenderPass zero_apron_rp{*m_device, rp_ci};

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
    rp_begin_info.renderPass = zero_apron_rp;
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
        vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
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
        vk::CmdWriteTimestamp2(m_command_buffer, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, query_pool, 0);
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
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer, 0, 1, &xfb_handle, &xfb_offset, &xfb_size);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-10656");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer, 0, 0, nullptr, nullptr);
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
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer, 0, 1, &xfb_handle, &xfb_offset, &xfb_size);
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer, 0, 0, nullptr, nullptr);
        vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
        m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-None-10657");
        vk::CmdEndTransformFeedbackEXT(m_command_buffer, 0, 0, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
        vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
        vk::CmdEndTransformFeedbackEXT(m_command_buffer, 0, 0, nullptr, nullptr);
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

TEST_F(NegativeTileShading, DispatchTileButPerTileExecutionModelNotEnabled) {
    TEST_DESCRIPTION("Try to launch dispatch tile, but per-tile execution model isn't enabled.");
    AddRequiredFeature(vkt::Feature::tileShadingDispatchTile);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.CreateComputePipeline();

    VkDispatchTileInfoQCOM dispatch_tile_info = vku::InitStructHelper();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchTileQCOM-None-10672");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchTileQCOM-None-10668");
    vk::CmdDispatchTileQCOM(m_command_buffer, &dispatch_tile_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DispatchTileButPerTileDispatchFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to launch dispatch tile, but tileShadingDispatchTile feature isn't enabled.");
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.CreateComputePipeline();

    VkDispatchTileInfoQCOM dispatch_tile_info = vku::InitStructHelper();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchTileQCOM-None-10669");
    vk::CmdDispatchTileQCOM(m_command_buffer, &dispatch_tile_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DispatchInsidePerTileExecutionModelButPerTileDispatchFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to launch dispatch inside the per-tile execution model scope, "
                     "but tileShadingPerTileDispatch feature isn't enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    RETURN_IF_SKIP(Init());
    InitTileShadingRenderTarget();

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.CreateComputePipeline();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10674");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, DrawInsidePerTileExecutionModelButPerTileDrawFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to launch draw inside the per-tile execution model scope, "
                     "but tileShadingPerTileDraw feature isn't enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    RETURN_IF_SKIP(Init());
    InitTileShadingRenderTarget();

    CreatePipelineHelper graphics_pipe{*this};
    graphics_pipe.gp_ci_.renderPass = m_tile_shading_render_pass;
    graphics_pipe.CreateGraphicsPipeline();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10677");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileShadingDrawButHasActiveGeometryShader) {
    TEST_DESCRIPTION("Try to launch a draw inside a tile-shading render pass scope, but the graphics pipeline "
                     "has an active geometry shader.");
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    VkShaderObj vs{*m_device, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3};
    VkShaderObj gs{*m_device, kGeometryMinimalGlsl, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_3};
    VkShaderObj fs{*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3};

    CreatePipelineHelper tile_shading_graphics_pipe{*this};
    tile_shading_graphics_pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    tile_shading_graphics_pipe.gp_ci_.renderPass = m_tile_shading_render_pass;
    tile_shading_graphics_pipe.CreateGraphicsPipeline();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, tile_shading_graphics_pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10678");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileShadingDrawButHasActiveGeometryShaderObject) {
    TEST_DESCRIPTION("Try to launch a draw inside a tile-shading render pass scope, but a geometry "
                     "shader object is bound.");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitTileShadingRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto geom_spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    auto vert_ci = ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT);
    vert_ci.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    vkt::Shader vert_shader{*m_device, vert_ci};
    vkt::Shader geom_shader{*m_device, ShaderCreateInfo(geom_spv, VK_SHADER_STAGE_GEOMETRY_BIT)};
    vkt::Shader frag_shader{*m_device, ShaderCreateInfoNoNextStage(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT)};

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = m_color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper();
    tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_info.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_info);
    rendering_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, geom_shader, frag_shader);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10678");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, PerTileDrawButAccessImageWithFeedbackLoopLayout) {
    TEST_DESCRIPTION("Try to launch a per-tile draw inside a tile-shading render pass scope, but accesses "
                     "an tile attachment with VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT.");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFeedbackLoopLayout);
    RETURN_IF_SKIP(InitBasicTileShading());

    constexpr uint32_t width = 64;
    constexpr uint32_t height = 64;
    constexpr VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    vkt::Image color_image{*m_device, width, height, color_format,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
                           VK_IMAGE_USAGE_SAMPLED_BIT};
    vkt::ImageView color_view = color_image.CreateView();
    vkt::Sampler sampler{*m_device, SafeSaneSamplerCreateInfo()};

    VkAttachmentDescription attachment_desc{};
    attachment_desc.format = color_format;
    attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;

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
    subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT | VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_desc;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &subpass_dependency;
    vkt::RenderPass tile_shading_render_pass{*m_device, rp_ci};

    const VkImageView color_view_handle = color_view.handle();
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = tile_shading_render_pass;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &color_view_handle;
    framebuffer_ci.width = width;
    framebuffer_ci.height = height;
    framebuffer_ci.layers = 1;
    vkt::Framebuffer tile_shading_framebuffer{*m_device, framebuffer_ci};

    const char* vs_source = R"glsl(
        #version 460

        vec2 pos[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2( 3.0, -1.0),
            vec2(-1.0,  3.0)
        );

        void main() {
            gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
        }
    )glsl";

    const char* fs_source = R"glsl(
        #version 460

        layout(set = 0, binding = 0) uniform sampler2D tile_img;
        layout(location = 0) out vec4 out_color;

        void main() {
            vec4 color = texture(tile_img, vec2(0.5, 0.5));
            out_color = color;
        }
    )glsl";

    VkShaderObj vs{*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3};
    VkShaderObj fs{*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3};

    CreatePipelineHelper tile_shading_graphics_pipe{*this};
    tile_shading_graphics_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    tile_shading_graphics_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    tile_shading_graphics_pipe.gp_ci_.renderPass = tile_shading_render_pass;
    tile_shading_graphics_pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
    tile_shading_graphics_pipe.CreateGraphicsPipeline();
    tile_shading_graphics_pipe.descriptor_set_->WriteDescriptorImageInfo(0, color_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                         VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT);
    tile_shading_graphics_pipe.descriptor_set_->UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = tile_shading_render_pass;
    rp_begin_info.framebuffer = tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, {width, height}};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, tile_shading_graphics_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              tile_shading_graphics_pipe.pipeline_layout_, 0, 1,
                              &tile_shading_graphics_pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10679");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, ImageDescriptorMismatchTileMemory) {
    TEST_DESCRIPTION("Try to create tile memory storage buffer and use it within a dispatch tile invocation.");
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::tileShadingDispatchTile);
    RETURN_IF_SKIP(InitBasicTileShading());

    constexpr uint32_t width = 64;
    constexpr uint32_t height = 64;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image{*m_device, image_ci, vkt::no_mem};

    VkImageMemoryRequirementsInfo2 image_info = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    VkMemoryRequirements2 image_reqs = vku::InitStructHelper(&tile_mem_reqs);
    image_info.image = image;
    vk::GetImageMemoryRequirements2(device(), &image_info, &image_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Image not eligible for tile memory binding, skipping test.";
    }

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = tile_mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(image_reqs.memoryRequirements.memoryTypeBits, &alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Failed to find an eligible tile memory type, skipping test.";
    }

    vkt::DeviceMemory image_memory{*m_device, alloc_info};
    vk::BindImageMemory(device(), image, image_memory, 0);
    vkt::ImageView image_view = image.CreateView();

    OneOffDescriptorSet descriptor_set{m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}}};
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(set = 0, binding = 0, tile_attachmentQCOM, rgba8) uniform readonly image2D storage_image;
        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;

        void main(){
            vec4 data = imageLoad(storage_image, ivec2(0));
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT};
    compute_pipe.pipeline_layout_ = vkt::PipelineLayout{*m_device, {&descriptor_set.layout_}};
    compute_pipe.CreateComputePipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.imageView = image_view;

    VkRenderPassTileShadingCreateInfoQCOM tile_info = vku::InitStructHelper();
    tile_info.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_info.tileApronSize = {0, 0};

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper(&tile_info);
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {width, height}};

    VkDispatchTileInfoQCOM dispatch_tile_info = vku::InitStructHelper();
    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchTileQCOM-commandBuffer-10746");
    vk::CmdDispatchTileQCOM(m_command_buffer, &dispatch_tile_info);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, CapabilityInComputeShaderFeatureNotEnabled) {
    TEST_DESCRIPTION("Create compute shader module with TileShadingQCOM capability, but the tileShading feature isn't enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main TileShadingRateQCOM 1 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10698");
    VkShaderObj cs{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, CapabilityInFragmentShaderFeatureNotEnabled) {
    TEST_DESCRIPTION("Create fragment shader module with TileShadingQCOM capability, but the tileShadingFragmentStage feature isn't enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char* fs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %main "main" %out_color
                OpExecutionMode %main OriginUpperLeft
                OpDecorate %out_color Location 0
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
       %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
 %ptr_out_v4f = OpTypePointer Output %v4float
   %out_color = OpVariable %ptr_out_v4f Output
     %float_1 = OpConstant %float 1
     %float_0 = OpConstant %float 0
    %vec4_red = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpStore %out_color %vec4_red
                OpReturn
                OpFunctionEnd
    )asm";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10699");
    VkShaderObj fs{*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, CapabilityInComputeShaderOutsideTileShadingRenderPass) {
    TEST_DESCRIPTION("Invoke a tile shading compute shader kernel outside a tile shading render pass.");
    RETURN_IF_SKIP(InitBasicTileShading());

    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %storage_image
               OpExecutionMode %main TileShadingRateQCOM 1 1 1
               OpDecorate %storage_image NonWritable
               OpDecorate %storage_image Binding 0
               OpDecorate %storage_image DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %8 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_TileAttachmentQCOM_8 = OpTypePointer TileAttachmentQCOM %8
%storage_image = OpVariable %_ptr_TileAttachmentQCOM_8 TileAttachmentQCOM
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    compute_pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10700");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10701");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, CapabilityInComputeShaderObjectOutsideTileShadingRenderPass) {
    TEST_DESCRIPTION("Invoke a tile shading compute shader object outside a tile shading render pass.");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicTileShading());

    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main TileShadingRateQCOM 1 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    std::vector<uint32_t> cs_spv{};
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, cs_source, cs_spv);
    vkt::Shader cs{*m_device, ShaderCreateInfo(cs_spv, VK_SHADER_STAGE_COMPUTE_BIT)};

    m_command_buffer.Begin();
    m_command_buffer.BindCompShader(cs);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10700");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10701");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, CapabilityInFragmentShaderOutsideTileShadingRenderPass) {
    TEST_DESCRIPTION("Invoke a tile shading fragment shader kernel outside a tile shading render pass.");
    AddRequiredFeature(vkt::Feature::tileShadingFragmentStage);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitRenderTarget();

    const char* vs_source = R"asm(
                OpCapability Shader
                OpMemoryModel Logical GLSL450
                OpEntryPoint Vertex %main "main" %out_pos
                OpDecorate %out_pos BuiltIn Position
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
       %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
 %ptr_out_v4f = OpTypePointer Output %v4float
     %out_pos = OpVariable %ptr_out_v4f Output
     %float_0 = OpConstant %float 0
     %float_1 = OpConstant %float 1
    %vec4_pos = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpStore %out_pos %vec4_pos
                OpReturn
                OpFunctionEnd
    )asm";

    const char* fs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %main "main" %out_color
                OpExecutionMode %main OriginUpperLeft
                OpDecorate %out_color Location 0
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
       %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
 %ptr_out_v4f = OpTypePointer Output %v4float
   %out_color = OpVariable %ptr_out_v4f Output
     %float_1 = OpConstant %float 1
     %float_0 = OpConstant %float 0
    %vec4_red = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpStore %out_color %vec4_red
                OpReturn
                OpFunctionEnd
    )asm";

    VkShaderObj vs{*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    VkShaderObj fs{*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};

    CreatePipelineHelper graphics_pipe{*this};
    graphics_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    graphics_pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer, &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipe);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10700");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, CapabilityInFragmentShaderObjectOutsideTileShadingRenderPass) {
    TEST_DESCRIPTION("Invoke a tile shading fragment shader object outside a tile shading render pass.");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::tileShadingFragmentStage);
    RETURN_IF_SKIP(InitBasicTileShading());
    InitDynamicRenderTarget();

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    const char* fs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %main "main" %out_color
                OpExecutionMode %main OriginUpperLeft
                OpDecorate %out_color Location 0
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
       %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
 %ptr_out_v4f = OpTypePointer Output %v4float
   %out_color = OpVariable %ptr_out_v4f Output
     %float_1 = OpConstant %float 1
     %float_0 = OpConstant %float 0
    %vec4_red = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpStore %out_color %vec4_red
                OpReturn
                OpFunctionEnd
    )asm";

    std::vector<uint32_t> fs_spv{};
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, fs_source, fs_spv);

    vkt::Shader vs{*m_device, ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT)};
    vkt::Shader fs{*m_device, ShaderCreateInfoNoNextStage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT)};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vs, fs);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-TileShadingQCOM-10700");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, MaxTileShadingRateWidth) {
    TEST_DESCRIPTION("Create a compute shader module with TileShadingRateQCOM.x exceeding "
                     "VkPhysicalDeviceTileShadingPropertiesQCOM::maxTileShadingRate::width.");
    RETURN_IF_SKIP(InitBasicTileShading());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(tile_shading_props);
    uint32_t invalid_width = 1;
    while (invalid_width <= tile_shading_props.maxTileShadingRate.width) {
        invalid_width <<= 1;
    }

    std::string cs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main"
                OpExecutionMode %main TileShadingRateQCOM )asm" + std::to_string(invalid_width) + " 1 1\n" +
                R"asm(
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpReturn
                OpFunctionEnd
    )asm";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-x-10702");
    VkShaderObj cs{*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, MaxTileShadingRateHeight) {
    TEST_DESCRIPTION("Create a compute shader module with TileShadingRateQCOM.y exceeding "
                     "VkPhysicalDeviceTileShadingPropertiesQCOM::maxTileShadingRate::height.");
    RETURN_IF_SKIP(InitBasicTileShading());

    VkPhysicalDeviceTileShadingPropertiesQCOM tile_shading_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(tile_shading_props);
    uint32_t invalid_height = 1;
    while (invalid_height <= tile_shading_props.maxTileShadingRate.height) {
        invalid_height <<= 1;
    }

    std::string cs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main"
                OpExecutionMode %main TileShadingRateQCOM 1 )asm" + std::to_string(invalid_height) + " 1\n" +
                R"asm(
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpReturn
                OpFunctionEnd
    )asm";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-y-10703");
    VkShaderObj cs{*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, TileShadingColorAttachmentsFeatureNotEnabled) {
    TEST_DESCRIPTION("Launch a compute pass that reads a TileAttachmentQCOM image backed by the "
                     "color attachment of the current subpass, but tileShadingColorAttachments is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    RETURN_IF_SKIP(Init());
    InitTileShadingRenderTarget();

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM, rgba8) uniform readonly image2D tile_img;

        void main() {
            uvec2 pixel_loc = gl_TileOffsetQCOM + uvec2(gl_GlobalInvocationID.xy);
            vec4 color = imageLoad(tile_img, ivec2(pixel_loc));
            if (color.x > 0.5) {}
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeImage-10707");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileShadingDepthAttachmentsFeatureNotEnabled) {
    TEST_DESCRIPTION("Launch a compute pass that reads a TileAttachmentQCOM image backed by the "
                     "depth aspect of the depth attachment, but tileShadingDepthAttachments is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    constexpr uint32_t width = 64;
    constexpr uint32_t height = 64;
    constexpr VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    constexpr VkFormat ds_format = VK_FORMAT_D32_SFLOAT;

    vkt::Image color_image{*m_device, width, height, color_format,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    VkImageCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.imageType = VK_IMAGE_TYPE_2D;
    ds_ci.format = ds_format;
    ds_ci.extent = {width, height, 1};
    ds_ci.mipLevels = 1;
    ds_ci.arrayLayers = 1;
    ds_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ds_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ds_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    ds_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image ds_image{*m_device, ds_ci, vkt::set_layout};
    vkt::ImageView ds_view = ds_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM, r32f) uniform readonly image2D tile_img;

        void main() {
            uvec2 pixel_loc = gl_TileOffsetQCOM + uvec2(gl_GlobalInvocationID.xy);
            vec4 depth = imageLoad(tile_img, ivec2(pixel_loc));
            if (depth.x > 0.5) {}
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, ds_view, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = ds_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {width, height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = &depth_attachment;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeImage-10708");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileShadingStencilAttachmentsFeatureNotEnabled) {
    TEST_DESCRIPTION("Launch a compute pass that reads a TileAttachmentQCOM image backed by the "
                     "stencil aspect of the depth/stencil attachment, but tileShadingStencilAttachments is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    AddRequiredFeature(vkt::Feature::tileShadingDepthAttachments);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    constexpr uint32_t width = 64;
    constexpr uint32_t height = 64;
    constexpr VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormat ds_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}) {
        if (FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL,
                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
            ds_format = format;
            break;
        }
    }
    if (ds_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "No depth/stencil format supports combined DEPTH_STENCIL_ATTACHMENT and STORAGE usage, skipping test.";
    }

    vkt::Image color_image{*m_device, width, height, color_format,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    VkImageCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.imageType = VK_IMAGE_TYPE_2D;
    ds_ci.format = ds_format;
    ds_ci.extent = {width, height, 1};
    ds_ci.mipLevels = 1;
    ds_ci.arrayLayers = 1;
    ds_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ds_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ds_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    ds_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image ds_image{*m_device, ds_ci, vkt::set_layout};
    vkt::ImageView stencil_view = ds_image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM, r8ui) uniform readonly uimage2D tile_img;

        void main() {
            uvec2 pixel_loc = gl_TileOffsetQCOM + uvec2(gl_GlobalInvocationID.xy);
            uvec4 stencil = imageLoad(tile_img, ivec2(pixel_loc));
            if (stencil.x > 0u) {}
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, stencil_view, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingAttachmentInfo stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = stencil_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, {width, height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pStencilAttachment = &stencil_attachment;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeImage-10709");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileShadingInputAttachmentsFeatureNotEnabled) {
    TEST_DESCRIPTION("Launch a compute pass that reads a TileAttachmentQCOM image backed by the "
                     "input attachment of the current subpass, but tileShadingInputAttachments is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    RETURN_IF_SKIP(Init());

    constexpr uint32_t width = 64;
    constexpr uint32_t height = 64;
    constexpr VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    vkt::Image color_image{*m_device, width, height, color_format,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT};
    vkt::ImageView color_view = color_image.CreateView();

    vkt::Image input_image{*m_device, width, height, color_format,
                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT};
    vkt::ImageView input_view = input_image.CreateView();

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM, rgba8) uniform readonly image2D tile_img;

        void main() {
            uvec2 pixel_loc = gl_TileOffsetQCOM + uvec2(gl_GlobalInvocationID.xy);
            vec4 color = imageLoad(tile_img, ivec2(pixel_loc));
            if (color.x > 0.5) {}
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, input_view, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    std::array<VkAttachmentDescription, 2> attachment_descs{};
    attachment_descs[0].format = color_format;
    attachment_descs[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_descs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_descs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_descs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_descs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_descs[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment_descs[1].format = color_format;
    attachment_descs[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_descs[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment_descs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_descs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_descs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_descs[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachment_descs[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_ref{0, VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference input_ref{1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass_desc{};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_ref;
    subpass_desc.inputAttachmentCount = 1;
    subpass_desc.pInputAttachments = &input_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&tile_shading_ci);
    rp_ci.attachmentCount = attachment_descs.size();
    rp_ci.pAttachments = attachment_descs.data();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass_desc;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &dependency;
    vkt::RenderPass tile_shading_render_pass{*m_device, rp_ci};

    VkImageView fb_attachments[2]{color_view.handle(), input_view.handle()};
    vkt::Framebuffer tile_shading_framebuffer{*m_device, tile_shading_render_pass, 2, fb_attachments, width, height};

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = tile_shading_render_pass;
    rp_begin_info.framebuffer = tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0, 0}, {width, height}};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeImage-10710");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileShadingSampledAttachmentsFeatureNotEnabled) {
    TEST_DESCRIPTION("Launch a compute pass that samples a TileAttachmentQCOM image backed by an attachment "
                     "of the current subpass, but tileShadingSampledAttachments is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileShading);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDraw);
    AddRequiredFeature(vkt::Feature::tileShadingPerTileDispatch);
    AddRequiredFeature(vkt::Feature::tileShadingColorAttachments);
    RETURN_IF_SKIP(Init());
    InitTileShadingRenderTarget();

    vkt::Sampler sampler{*m_device, SafeSaneSamplerCreateInfo()};

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM) uniform sampler2D sampled_tex;

        void main() {
            uvec2 pixel_loc = gl_TileOffsetQCOM + uvec2(gl_GlobalInvocationID.xy);
            vec4 color = texture(sampled_tex, vec2(pixel_loc) / vec2(64.0, 64.0));
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    compute_pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeSampledImage-10711");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, MaxTileShadingRateDepth) {
    TEST_DESCRIPTION("Launch a compute pass with TileShadingRateQCOM.z exceeding "
                     "VkPhysicalDeviceTileShadingPropertiesQCOM::maxTileShadingRate::depth.");
    AddRequiredExtensions(VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileProperties);
    AddRequiredFeature(vkt::Feature::tileShadingDispatchTile);
    RETURN_IF_SKIP(InitBasicTileShading());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Don't support query dynamic tile properties on Vulkan Mock Device, skipping test.";
    }

    InitTileShadingRenderTarget();

    uint32_t tile_prop_count = 0;
    std::vector<VkTilePropertiesQCOM> tile_props{};
    vk::GetFramebufferTilePropertiesQCOM(device(), m_tile_shading_framebuffer, &tile_prop_count, nullptr);
    tile_props.resize(tile_prop_count, vku::InitStructHelper());
    vk::GetFramebufferTilePropertiesQCOM(device(), m_tile_shading_framebuffer, &tile_prop_count, tile_props.data());

    uint32_t invalid_depth = 1;
    for (uint32_t index = 0; index < tile_prop_count; ++index) {
        invalid_depth = std::max(invalid_depth, tile_props[index].tileSize.depth);
    }
    invalid_depth <<= 1;

    const std::string cs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main"
                OpExecutionMode %main TileShadingRateQCOM 1 1 )asm" + std::to_string(invalid_depth) + '\n' +
                R"asm(
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpReturn
                OpFunctionEnd
    )asm";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    compute_pipe.CreateComputePipeline();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();
    VkDispatchTileInfoQCOM dispatch_tile_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-z-10704");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileSize-10705");
    vk::CmdDispatchTileQCOM(m_command_buffer, &dispatch_tile_info);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, MaxTileShadingRateDepthInDynamicRendering) {
    TEST_DESCRIPTION("Launch a compute pass with TileShadingRateQCOM.z exceeding "
                     "VkPhysicalDeviceTileShadingPropertiesQCOM::maxTileShadingRate::depth.");
    AddRequiredExtensions(VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileProperties);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::tileShadingDispatchTile);
    RETURN_IF_SKIP(InitBasicTileShading());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Don't support query dynamic tile properties on Vulkan Mock Device, skipping test.";
    }

    tile_shading_rp_config.rt_size = {512, 512};
    InitTileShadingRenderTarget();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = m_color_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassTileShadingCreateInfoQCOM tile_shading_ci = vku::InitStructHelper();
    tile_shading_ci.flags = VK_TILE_SHADING_RENDER_PASS_ENABLE_BIT_QCOM;
    tile_shading_ci.tileApronSize = {0, 0};

    VkRenderingInfo rendering_info = vku::InitStructHelper(&tile_shading_ci);
    rendering_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    VkTilePropertiesQCOM tile_props = vku::InitStructHelper();
    vk::GetDynamicRenderingTilePropertiesQCOM(device(), &rendering_info, &tile_props);
    const uint32_t invalid_depth = tile_props.tileSize.depth << 1;

    const std::string cs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main"
                OpExecutionMode %main TileShadingRateQCOM 1 1 )asm" + std::to_string(invalid_depth) + '\n' +
                R"asm(
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpReturn
                OpFunctionEnd
    )asm";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    compute_pipe.CreateComputePipeline();

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();
    VkDispatchTileInfoQCOM dispatch_tile_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-z-10704");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileSize-10705");
    vk::CmdDispatchTileQCOM(m_command_buffer, &dispatch_tile_info);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, TileImageConsumedByTexelPointerOpButAtomicOpsFeatureNotEnabled) {
    TEST_DESCRIPTION("Use OpImageTexelPointer that consumes an image variable with TileAttachmentQCOM storage class, "
                     "but tileShadingAtomicOps is not enabled.");
    RETURN_IF_SKIP(InitBasicTileShading());

    const char* cs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main" %tile_img
                OpExecutionMode %main TileShadingRateQCOM 1 1 1
                OpDecorate %tile_img DescriptorSet 0
                OpDecorate %tile_img Binding 0
        %void = OpTypeVoid
          %fn = OpTypeFunction %void
        %uint = OpTypeInt 32 0
         %int = OpTypeInt 32 1
       %v2int = OpTypeVector %int 2
       %int_0 = OpConstant %int 0
       %coord = OpConstantComposite %v2int %int_0 %int_0
      %uint_0 = OpConstant %uint 0
    %image_ty = OpTypeImage %uint 2D 0 0 0 2 R32ui
 %tile_ptr_ty = OpTypePointer TileAttachmentQCOM %image_ty
    %tile_img = OpVariable %tile_ptr_ty TileAttachmentQCOM
%texel_ptr_ty = OpTypePointer Image %uint
        %main = OpFunction %void None %fn
       %label = OpLabel
         %ptr = OpImageTexelPointer %texel_ptr_ty %tile_img %coord %uint_0
                OpReturn
                OpFunctionEnd
    )asm";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-10706");
    VkShaderObj cs{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, TileImageConsumedByTexelPointerOpViaAccessChainButAtomicOpsFeatureNotEnabled) {
    TEST_DESCRIPTION("Use OpImageTexelPointer on a tile attachment image pointer obtained via OpAccessChain, "
                     "but tileShadingAtomicOps feature is not enabled.");
    RETURN_IF_SKIP(InitBasicTileShading());

    const char* cs_source = R"asm(
                OpCapability Shader
                OpCapability TileShadingQCOM
                OpExtension "SPV_QCOM_tile_shading"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main" %tile_img
                OpExecutionMode %main TileShadingRateQCOM 1 1 1
                OpDecorate %tile_img DescriptorSet 0
                OpDecorate %tile_img Binding 0
        %void = OpTypeVoid
          %fn = OpTypeFunction %void
        %uint = OpTypeInt 32 0
         %int = OpTypeInt 32 1
       %v2int = OpTypeVector %int 2
       %int_0 = OpConstant %int 0
       %int_1 = OpConstant %int 1
       %coord = OpConstantComposite %v2int %int_0 %int_0
      %uint_0 = OpConstant %uint 0
    %image_ty = OpTypeImage %uint 2D 0 0 0 2 R32ui
      %arr_ty = OpTypeArray %image_ty %int_1
 %tile_ptr_ty = OpTypePointer TileAttachmentQCOM %arr_ty
  %img_ptr_ty = OpTypePointer TileAttachmentQCOM %image_ty
    %tile_img = OpVariable %tile_ptr_ty TileAttachmentQCOM
%texel_ptr_ty = OpTypePointer Image %uint
        %main = OpFunction %void None %fn
       %label = OpLabel
      %elem_p = OpAccessChain %img_ptr_ty %tile_img %int_0
         %ptr = OpImageTexelPointer %texel_ptr_ty %elem_p %coord %uint_0
                OpReturn
                OpFunctionEnd
    )asm";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-10706");
    VkShaderObj cs{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, TileImageAtomicOpButAtomicOpsFeatureNotEnabled) {
    TEST_DESCRIPTION("Execute an image atomic operation on a tile attachment, "
                     "but tileShadingAtomicOps feature is not enabled.");
    RETURN_IF_SKIP(InitBasicTileShading());

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM, r32ui) uniform uimage2D tile_img;

        void main() {
            imageAtomicExchange(tile_img, ivec2(0, 0), 1u);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-10706");
    VkShaderObj cs{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileShading, UseSampleWeightedImageOpButTileShadingImageProcessingNotEnabled) {
    TEST_DESCRIPTION("Use OpImageSampleWeightedQCOM with an OpTypeSampledImage declared in the TileAttachmentQCOM storage class, "
                     "but tileShadingImageProcessing is not enabled.");
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::textureSampleWeighted);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkFormat sampled_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16_UNORM}) {
        const auto features = m_device->FormatFeaturesOptimal(format);
        if ((features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0 &&
            (features & VK_FORMAT_FEATURE_2_WEIGHT_SAMPLED_IMAGE_BIT_QCOM) != 0) {
            sampled_format = format;
            break;
        }
    }
    VkFormat weight_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_R16_SFLOAT, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R8_UNORM}) {
        const auto features = m_device->FormatFeaturesOptimal(format);
        if ((features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0 &&
            (features & VK_FORMAT_FEATURE_2_WEIGHT_IMAGE_BIT_QCOM) != 0) {
            weight_format = format;
            break;
        }
    }
    if (sampled_format == VK_FORMAT_UNDEFINED || weight_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Failed to find a format supporting OpImageSampleWeightedQCOM, skipping test.";
    }

    tile_shading_rp_config.format = sampled_format;
    InitTileShadingRenderTarget();

    VkPhysicalDeviceImageProcessingPropertiesQCOM image_processing_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(image_processing_props);

    VkImageCreateInfo weight_image_ci = vku::InitStructHelper();
    weight_image_ci.imageType = VK_IMAGE_TYPE_2D;
    weight_image_ci.format = weight_format;
    weight_image_ci.extent = {image_processing_props.maxWeightFilterDimension.width,
                              image_processing_props.maxWeightFilterDimension.height, 1};
    weight_image_ci.mipLevels = 1;
    weight_image_ci.arrayLayers = 1;
    weight_image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    weight_image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    weight_image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM;
    vkt::Image weight_image{*m_device, weight_image_ci};

    VkImageViewSampleWeightCreateInfoQCOM weight_view_weight_ci = vku::InitStructHelper();
    weight_view_weight_ci.filterCenter = {0, 0};
    weight_view_weight_ci.filterSize = image_processing_props.maxWeightFilterDimension;
    weight_view_weight_ci.numPhases = 1;
    VkImageViewCreateInfo weight_view_ci = vku::InitStructHelper(&weight_view_weight_ci);
    weight_view_ci.image = weight_image;
    weight_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    weight_view_ci.format = weight_format;
    weight_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView weight_view{*m_device, weight_view_ci};

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.flags = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;
    sampler_ci.magFilter = VK_FILTER_NEAREST;
    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeV = sampler_ci.addressModeU;
    sampler_ci.addressModeW = sampler_ci.addressModeU;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_image_processing: require
        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM) uniform sampler2D sampled_tex;
        layout(set = 0, binding = 1) uniform texture2DArray weight_tex;
        layout(set = 0, binding = 2) uniform sampler processing_sampler;

        void main() {
            vec4 result = textureWeightedQCOM(
                sampled_tex,
                vec2(0.5, 0.5),
                sampler2DArray(weight_tex, processing_sampler)
            );
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    compute_pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
    };
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(1, weight_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(2, nullptr, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileShadingImageProcessing-10712");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, UseBoxFilterImageOpButTileShadingImageProcessingNotEnabled) {
    TEST_DESCRIPTION("Use OpImageBoxFilterQCOM with an OpTypeSampledImage declared in TileAttachmentQCOM storage class, "
                     "but tileShadingImageProcessing is not enabled.");
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::textureBoxFilter);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkFormat sampled_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16_UNORM}) {
        const auto features = m_device->FormatFeaturesOptimal(format);
        if ((features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0 &&
            (features & VK_FORMAT_FEATURE_2_BOX_FILTER_SAMPLED_BIT_QCOM) != 0) {
            sampled_format = format;
            break;
        }
    }
    if (sampled_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Failed to find a format supporting OpImageBoxFilterQCOM, skipping test.";
    }

    tile_shading_rp_config.format = sampled_format;
    InitTileShadingRenderTarget();

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.flags = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;
    sampler_ci.magFilter = VK_FILTER_NEAREST;
    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeV = sampler_ci.addressModeU;
    sampler_ci.addressModeW = sampler_ci.addressModeU;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_image_processing: require
        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM) uniform sampler2D sampled_tex;

        void main() {
            vec4 result = textureBoxFilterQCOM(
                sampled_tex,
                vec2(0.5, 0.5),
                vec2(2.0, 2.0)
            );
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    compute_pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0,0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileShadingImageProcessing-10712");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, UseBlockMatchImageOpButTileShadingImageProcessingNotEnabled) {
    TEST_DESCRIPTION("Use OpImageBlockMatchSADQCOM and OpImageBlockMatchSSDQCOM with an OpTypeSampledImage declared in "
                     "TileAttachmentQCOM storage class as the target image, but tileShadingImageProcessing is not enabled.");
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::textureBlockMatch);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkFormat sampled_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8_UNORM, VK_FORMAT_R16_UNORM}) {
        const auto features = m_device->FormatFeaturesOptimal(format);
        if ((features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0 &&
            (features & VK_FORMAT_FEATURE_2_BLOCK_MATCHING_BIT_QCOM) != 0) {
            sampled_format = format;
            break;
        }
    }
    if (sampled_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Failed to find a format supporting OpImageBlockMatchSADQCOM, skipping test.";
    }

    tile_shading_rp_config.format = sampled_format;
    tile_shading_rp_config.block_match_usage = true;
    InitTileShadingRenderTarget();

    vkt::Image ref_image{*m_device, tile_shading_rp_config.rt_size.width, tile_shading_rp_config.rt_size.height, sampled_format,
                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM};
    vkt::ImageView ref_view = ref_image.CreateView();

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.flags = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.magFilter = VK_FILTER_NEAREST;
    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeV = sampler_ci.addressModeU;
    sampler_ci.addressModeW = sampler_ci.addressModeU;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_image_processing: require
        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM) uniform texture2D target_tex;
        layout(set = 0, binding = 1) uniform texture2D ref_tex;
        layout(set = 0, binding = 2) uniform sampler processing_sampler;

        void main() {
            uvec2 target_coord = uvec2(0, 0);
            uvec2 ref_coord = uvec2(4, 4);
            uvec2 block_size = uvec2(4, 4);
            vec4 result1 = textureBlockMatchSADQCOM(
                sampler2D(target_tex, processing_sampler), target_coord,
                sampler2D(ref_tex, processing_sampler), ref_coord,
                block_size
            );
            vec4 result2 = textureBlockMatchSSDQCOM(
                sampler2D(target_tex, processing_sampler), target_coord,
                sampler2D(ref_tex, processing_sampler), ref_coord,
                block_size
            );
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    compute_pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
    };
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(1, ref_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(2, nullptr, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileShadingImageProcessing-10712");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, UseBlockMatchWindowImageOpButTileShadingImageProcessingNotEnabled) {
    TEST_DESCRIPTION("Use OpImageBlockMatchWindowSSDQCOM and OpImageBlockMatchWindowSADQCOM with an OpTypeSampledImage in "
                     "TileAttachmentQCOM storage class as the target image, but tileShadingImageProcessing is not enabled.");
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::textureBlockMatch);
    AddRequiredFeature(vkt::Feature::textureBlockMatch2);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkFormat sampled_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R8G8B8A8_UNORM}) {
        const auto features = m_device->FormatFeaturesOptimal(format);
        if ((features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0 &&
            (features & VK_FORMAT_FEATURE_2_BLOCK_MATCHING_BIT_QCOM) != 0) {
            sampled_format = format;
            break;
        }
    }
    if (sampled_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Failed to find a format supporting OpImageBlockMatchWindowSSDQCOM, skipping test.";
    }

    VkPhysicalDeviceImageProcessing2PropertiesQCOM image_processing2_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(image_processing2_props);

    tile_shading_rp_config.format = sampled_format;
    tile_shading_rp_config.block_match_usage = true;
    InitTileShadingRenderTarget();

    vkt::Image ref_image{*m_device, tile_shading_rp_config.rt_size.width, tile_shading_rp_config.rt_size.height, sampled_format,
                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM};
    vkt::ImageView ref_view = ref_image.CreateView();

    VkSamplerBlockMatchWindowCreateInfoQCOM block_match_window_ci = vku::InitStructHelper();
    block_match_window_ci.windowExtent = image_processing2_props.maxBlockMatchWindow;
    block_match_window_ci.windowCompareMode = VK_BLOCK_MATCH_WINDOW_COMPARE_MODE_MIN_QCOM;
    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper(&block_match_window_ci);
    sampler_ci.flags = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.magFilter = VK_FILTER_NEAREST;
    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_ci.addressModeV = sampler_ci.addressModeU;
    sampler_ci.addressModeW = sampler_ci.addressModeU;
    sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_image_processing: require
        #extension GL_QCOM_image_processing2: require
        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM) uniform texture2D target_tex;
        layout(set = 0, binding = 1) uniform texture2D ref_tex;
        layout(set = 0, binding = 2) uniform sampler processing_sampler;

        void main() {
            uvec2 target_coord = uvec2(0, 0);
            uvec2 ref_coord = uvec2(0, 0);
            uvec2 block_size = uvec2(4, 4);
            vec4 result1 = textureBlockMatchWindowSSDQCOM(
                sampler2D(target_tex, processing_sampler), target_coord,
                sampler2D(ref_tex, processing_sampler), ref_coord,
                block_size
            );
            vec4 result2 = textureBlockMatchWindowSADQCOM(
                sampler2D(target_tex, processing_sampler), target_coord,
                sampler2D(ref_tex, processing_sampler), ref_coord,
                block_size
            );
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    compute_pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
    };
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(1, ref_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(2, nullptr, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileShadingImageProcessing-10712");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTileShading, UseBlockMatchGatherImageOpButTileShadingImageProcessingNotEnabled) {
    TEST_DESCRIPTION("Use OpImageBlockMatchGatherSSDQCOM and OpImageBlockMatchGatherSADQCOM with an OpTypeSampledImage declared in "
                     "TileAttachmentQCOM storage class as the target image, but tileShadingImageProcessing is not enabled.");
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_IMAGE_PROCESSING_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::textureBlockMatch);
    AddRequiredFeature(vkt::Feature::textureBlockMatch2);
    RETURN_IF_SKIP(InitBasicTileShading());

    VkFormat sampled_format = VK_FORMAT_UNDEFINED;
    for (VkFormat format : {VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R8G8B8A8_UNORM}) {
        const auto features = m_device->FormatFeaturesOptimal(format);
        if ((features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0 &&
            (features & VK_FORMAT_FEATURE_2_BLOCK_MATCHING_BIT_QCOM) != 0) {
            sampled_format = format;
            break;
        }
    }
    if (sampled_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Failed to find a format supporting OpImageBlockMatchGatherSSDQCOM, skipping test.";
    }

    VkPhysicalDeviceImageProcessing2PropertiesQCOM image_processing2_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(image_processing2_props);

    tile_shading_rp_config.format = sampled_format;
    tile_shading_rp_config.block_match_usage = true;
    InitTileShadingRenderTarget();

    vkt::Image ref_image{*m_device, tile_shading_rp_config.rt_size.width, tile_shading_rp_config.rt_size.height, sampled_format,
                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM};
    vkt::ImageView ref_view = ref_image.CreateView();

    VkSamplerBlockMatchWindowCreateInfoQCOM block_match_window_ci = vku::InitStructHelper();
    block_match_window_ci.windowExtent = image_processing2_props.maxBlockMatchWindow;
    block_match_window_ci.windowCompareMode = VK_BLOCK_MATCH_WINDOW_COMPARE_MODE_MIN_QCOM;
    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper(&block_match_window_ci);
    sampler_ci.flags = VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM;
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.magFilter = VK_FILTER_NEAREST;
    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_ci.addressModeV = sampler_ci.addressModeU;
    sampler_ci.addressModeW = sampler_ci.addressModeU;
    sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 460

        #extension GL_QCOM_image_processing: require
        #extension GL_QCOM_image_processing2: require
        #extension GL_QCOM_tile_shading : require

        layout(shading_rate_xQCOM = 1, shading_rate_yQCOM = 1, shading_rate_zQCOM = 1) in;
        layout(set = 0, binding = 0, tile_attachmentQCOM) uniform texture2D target_tex;
        layout(set = 0, binding = 1) uniform texture2D ref_tex;
        layout(set = 0, binding = 2) uniform sampler processing_sampler;

        void main() {
            uvec2 target_coord = uvec2(0, 0);
            uvec2 ref_coord = uvec2(0, 0);
            uvec2 block_size = uvec2(4, 4);
            vec4 result1 = textureBlockMatchGatherSSDQCOM(
                sampler2D(target_tex, processing_sampler), target_coord,
                sampler2D(ref_tex, processing_sampler), ref_coord,
                block_size
            );
            vec4 result2 = textureBlockMatchGatherSADQCOM(
                sampler2D(target_tex, processing_sampler), target_coord,
                sampler2D(ref_tex, processing_sampler), ref_coord,
                block_size
            );
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL};
    compute_pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
    };
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, m_color_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(1, ref_view, nullptr,
                                                          VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_IMAGE_LAYOUT_GENERAL);
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(2, nullptr, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    VkClearValue clear_color{};
    clear_color.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper();
    rp_begin_info.renderPass = m_tile_shading_render_pass;
    rp_begin_info.framebuffer = m_tile_shading_framebuffer;
    rp_begin_info.renderArea = {{0, 0}, tile_shading_rp_config.rt_size};
    rp_begin_info.clearValueCount = 1;
    rp_begin_info.pClearValues = &clear_color;

    VkPerTileBeginInfoQCOM per_tile_begin_info = vku::InitStructHelper();
    VkPerTileEndInfoQCOM per_tile_end_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_begin_info);
    vk::CmdBeginPerTileExecutionQCOM(m_command_buffer, &per_tile_begin_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tileShadingImageProcessing-10712");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndPerTileExecutionQCOM(m_command_buffer, &per_tile_end_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
