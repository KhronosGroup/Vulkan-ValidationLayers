/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "error_message/log_message_type.h"

class PositiveMultiview : public VkLayerTest {};

TEST_F(PositiveMultiview, RenderPassQueries) {
    TEST_DESCRIPTION("Use queries in a render pass instance with multiview enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);

    uint32_t view_masks[] = {0x3u};
    uint32_t correlation_masks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmvci = vku::InitStructHelper();
    rpmvci.subpassCount = 1;
    rpmvci.pViewMasks = view_masks;
    rpmvci.correlationMaskCount = 1;
    rpmvci.pCorrelationMasks = correlation_masks;

    rp.CreateRenderPass(&rpmvci);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 2, 3, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 3);

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0, 2);

    m_command_buffer.BeginRenderPass(rp.Handle(), fb, 32, 32);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool, 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool, 0);
    m_command_buffer.EndRenderPass();

    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool, 0, 2, buffer, 0, 4, 0);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveMultiview, BasicRenderPass) {
    TEST_DESCRIPTION("Create render pass with view mask, with multiview feature enabled in Vulkan11Features.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);
}

TEST_F(PositiveMultiview, PushDescriptor) {
    TEST_DESCRIPTION("Begin render pass with view mask and a push descriptor.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    std::array subpasses = {vku::InitStruct<VkSubpassDescription2>(), vku::InitStruct<VkSubpassDescription2>()};
    subpasses[0].viewMask = 0x1;
    subpasses[1].viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = subpasses.size();
    render_pass_ci.pSubpasses = subpasses.data();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, render_pass, 1, &view.handle());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});
    // Create push descriptor set layout
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    // Use helper to create graphics pipeline
    CreatePipelineHelper pipe(*this);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&push_ds_layout, &ds_layout});
    pipe.gp_ci_.renderPass = render_pass;
    pipe.CreateGraphicsPipeline();

    const uint32_t data_size = sizeof(float) * 3;
    vkt::Buffer vbo(*m_device, data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info = {vbo, 0, data_size};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                &descriptor_write);
    m_command_buffer.BeginRenderPass(render_pass, fb, 32, 32);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveMultiview, MeshShader) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/issues/4194");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::multiviewMeshShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices=81) out;
        layout(max_primitives=32) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(81, 32);
            gl_MeshPrimitivesEXT[0].gl_CullPrimitiveEXT = true;
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    VkAttachmentReference color_attachment = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentDescription attach_desc = {};
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    uint32_t view_masks[] = {0x3u};
    VkRenderPassMultiviewCreateInfo rp_multiview_ci = vku::InitStructHelper();
    rp_multiview_ci.subpassCount = 1;
    rp_multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rp_multiview_ci);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rpci);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.renderPass = render_pass;
    pipe.CreateGraphicsPipeline();
}
