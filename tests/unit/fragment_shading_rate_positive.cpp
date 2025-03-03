/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/render_pass_helper.h"

class PositiveFragmentShadingRate : public VkLayerTest {};

TEST_F(PositiveFragmentShadingRate, StageInVariousAPIs) {
    TEST_DESCRIPTION("Specify shading rate pipeline stage with attachmentFragmentShadingRate feature enabled");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);
    const vkt::Event event(*m_device);
    const vkt::Event event2(*m_device);

    m_command_buffer.Begin();
    // Different API calls to cover three category of VUIDs: 07316, 07318, 07314
    vk::CmdResetEvent2KHR(m_command_buffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vk::CmdSetEvent(m_command_buffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_command_buffer.End();
}

TEST_F(PositiveFragmentShadingRate, StageWithPipelineBarrier) {
    TEST_DESCRIPTION("Test pipeline barrier with VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR stage");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);

    RETURN_IF_SKIP(Init());

    VkImageFormatProperties format_props = {};
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties(
        m_device->Physical().handle(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0, &format_props);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Image options not supported";
    }

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &imageMemoryBarrier);
    m_command_buffer.End();
}

TEST_F(PositiveFragmentShadingRate, Attachments) {
    TEST_DESCRIPTION("Create framebuffer with a fragment shading rate attachment that has layout count 1.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.SetViewMask(0x2);
    rp.CreateRenderPass();
    vkt::Image image(*m_device, 1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vkt::ImageView imageView = image.CreateView();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &imageView.handle(),
                                 fsr_properties.minFragmentShadingRateAttachmentTexelSize.width,
                                 fsr_properties.minFragmentShadingRateAttachmentTexelSize.height);
    ASSERT_TRUE(framebuffer.initialized());
}

TEST_F(PositiveFragmentShadingRate, FragmentDensityMapOffset) {
    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    AddRequiredFeature(vkt::Feature::fragmentDensityMapOffset);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent = {32u, 32u, 1u};
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 1u;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_create_info);
    vkt::ImageView image_view = image.CreateView();

    VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_ci = vku::InitStructHelper();
    fragment_density_map_ci.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_R8G8_UNORM;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass_description = {};

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&fragment_density_map_ci);
    render_pass_ci.attachmentCount = 1u;
    render_pass_ci.pAttachments = &attachment_description;
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass_description;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1u, &image_view.handle());

    VkRenderPassBeginInfo begin_info = vku::InitStructHelper();
    begin_info.renderPass = render_pass.handle();
    begin_info.framebuffer = framebuffer.handle();
    begin_info.renderArea = {{0, 0}, {32u, 32u}};

    m_command_buffer.Begin();

    vk::CmdBeginRenderPass(m_command_buffer.handle(), &begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fdm_offset_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_offset_properties);

    int32_t width = static_cast<int32_t>(fdm_offset_properties.fragmentDensityOffsetGranularity.width);
    int32_t height = static_cast<int32_t>(fdm_offset_properties.fragmentDensityOffsetGranularity.height);
    VkOffset2D offset = {width, height};
    VkSubpassFragmentDensityMapOffsetEndInfoQCOM fdm_offset_end_info = vku::InitStructHelper();
    fdm_offset_end_info.fragmentDensityOffsetCount = 1u;
    fdm_offset_end_info.pFragmentDensityOffsets = &offset;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper(&fdm_offset_end_info);

    vk::CmdEndRenderPass2KHR(m_command_buffer.handle(), &subpass_end_info);

    m_command_buffer.End();
}

TEST_F(PositiveFragmentShadingRate, FragmentDensityMapOffsetMultiview) {
    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    AddRequiredFeature(vkt::Feature::fragmentDensityMapOffset);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent = {32u, 32u, 1u};
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 2u;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_create_info);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_ci = vku::InitStructHelper();
    fragment_density_map_ci.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkAttachmentDescription2 attachment_description = vku::InitStructHelper();
    attachment_description.format = VK_FORMAT_R8G8_UNORM;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription2 subpass_description = vku::InitStructHelper();
    subpass_description.viewMask = 0x3u;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper(&fragment_density_map_ci);
    render_pass_ci.attachmentCount = 1u;
    render_pass_ci.pAttachments = &attachment_description;
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass_description;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1u, &image_view.handle());

    VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fdm_offset_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_offset_properties);
    int32_t width = static_cast<int32_t>(fdm_offset_properties.fragmentDensityOffsetGranularity.width);
    int32_t height = static_cast<int32_t>(fdm_offset_properties.fragmentDensityOffsetGranularity.height);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer);
    VkOffset2D offsets[2] = {{width, height}, {width * 2, height * 2}};
    VkSubpassFragmentDensityMapOffsetEndInfoQCOM fdm_offset_end_info = vku::InitStructHelper();
    fdm_offset_end_info.fragmentDensityOffsetCount = 2u;
    fdm_offset_end_info.pFragmentDensityOffsets = offsets;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper(&fdm_offset_end_info);
    vk::CmdEndRenderPass2KHR(m_command_buffer.handle(), &subpass_end_info);
    m_command_buffer.End();
}

TEST_F(PositiveFragmentShadingRate, FragmentDensityMapOffsetEmptyRenderPass) {
    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    AddRequiredFeature(vkt::Feature::fragmentDensityMapOffset);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32u, 32u, 1u, VK_FORMAT_R8G8_UNORM, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    vkt::ImageView image_view = image.CreateView();

    VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_ci = vku::InitStructHelper();
    fragment_density_map_ci.fragmentDensityMapAttachment.attachment = VK_ATTACHMENT_UNUSED;
    fragment_density_map_ci.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_R8G8_UNORM;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass_description = {};

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&fragment_density_map_ci);
    render_pass_ci.attachmentCount = 1u;
    render_pass_ci.pAttachments = &attachment_description;
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass_description;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1u, &image_view.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer);

    VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fdm_offset_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_offset_properties);

    int32_t width = static_cast<int32_t>(fdm_offset_properties.fragmentDensityOffsetGranularity.width);
    int32_t height = static_cast<int32_t>(fdm_offset_properties.fragmentDensityOffsetGranularity.height);
    VkOffset2D offset = {width, height};
    VkSubpassFragmentDensityMapOffsetEndInfoQCOM fdm_offset_end_info = vku::InitStructHelper();
    fdm_offset_end_info.fragmentDensityOffsetCount = 1u;
    fdm_offset_end_info.pFragmentDensityOffsets = &offset;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper(&fdm_offset_end_info);
    vk::CmdEndRenderPass2KHR(m_command_buffer.handle(), &subpass_end_info);
    m_command_buffer.End();
}
