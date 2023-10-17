/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

TEST_F(NegativeMultiview, MaxInstanceIndex) {
    TEST_DESCRIPTION("Verify if instance index in CmdDraw is greater than maxMultiviewInstanceIndex.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    multiview_features.multiview = VK_TRUE;
    VkPhysicalDeviceFeatures2 pd_features2 = vku::InitStructHelper(&multiview_features);

    RETURN_IF_SKIP(InitState(nullptr, &pd_features2));
    InitRenderTarget();

    VkPhysicalDeviceMultiviewProperties multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_props);
    if (multiview_props.maxMultiviewInstanceIndex == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "maxMultiviewInstanceIndex is uint32_t max";
    }
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");
    vk::CmdDraw(m_commandBuffer->handle(), 1, multiview_props.maxMultiviewInstanceIndex + 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeMultiview, ClearColorAttachments) {
    TEST_DESCRIPTION("Test cmdClearAttachments with active render pass that uses multiview");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (!multiview_features.multiview) {
        GTEST_SKIP() << "VkPhysicalDeviceMultiviewFeatures::multiview not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper(&renderPassMultiviewCreateInfo);
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    vkt::RenderPass renderPass(*m_device, renderPassCreateInfo);
    ASSERT_TRUE(renderPass.initialized());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                             VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper();
    framebufferCreateInfo.width = 32;
    framebufferCreateInfo.height = 32;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = renderPass.handle();
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView;

    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
    ASSERT_TRUE(framebuffer.initialized());

    // Start no RenderPass
    m_commandBuffer->begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;

    VkClearRect clear_rect = {};
    clear_rect.rect.extent.width = 32;
    clear_rect.rect.extent.height = 32;

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = renderPass.handle();
    render_pass_begin_info.framebuffer = framebuffer.handle();
    render_pass_begin_info.renderArea.extent.width = 32;
    render_pass_begin_info.renderArea.extent.height = 32;

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-baseArrayLayer-00018");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
    clear_rect.layerCount = 2;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-baseArrayLayer-00018");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
    clear_rect.baseArrayLayer = 1;
    clear_rect.layerCount = 1;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMultiview, UnboundResourcesAfterBeginRenderPassAndNextSubpass) {
    TEST_DESCRIPTION(
        "Validate all required resources are bound if multiview is enabled after vkCmdBeginRenderPass and vkCmdNextSubpass");

    constexpr unsigned multiview_count = 2u;
    constexpr unsigned extra_subpass_count = multiview_count - 1u;

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "Device does not support multiview.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    m_renderPass_subpasses.resize(multiview_count);
    for (unsigned i = 0; i < multiview_count; ++i) {
        m_renderPass_subpasses[i].colorAttachmentCount = 1;
        m_renderPass_subpasses[i].pColorAttachments = &colorAttachmentReference;
    }

    uint32_t viewMasks[multiview_count] = {};
    for (unsigned i = 0; i < multiview_count; ++i) {
        viewMasks[i] = 1u << i;
    }
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = multiview_count;
    renderPassMultiviewCreateInfo.pViewMasks = viewMasks;

    m_renderPass_info = vku::InitStructHelper(&renderPassMultiviewCreateInfo);
    m_renderPass_info.attachmentCount = 1;
    m_renderPass_info.pAttachments = &attachmentDescription;
    m_renderPass_info.subpassCount = m_renderPass_subpasses.size();
    m_renderPass_info.pSubpasses = m_renderPass_subpasses.data();

    m_renderPass_dependencies.resize(extra_subpass_count);
    for (unsigned i = 0; i < m_renderPass_dependencies.size(); ++i) {
        auto &subpass_dep = m_renderPass_dependencies[i];
        subpass_dep.srcSubpass = i;
        subpass_dep.dstSubpass = i + 1;

        subpass_dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        subpass_dep.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dep.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        subpass_dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    m_renderPass_info.dependencyCount = static_cast<uint32_t>(m_renderPass_dependencies.size());
    m_renderPass_info.pDependencies = m_renderPass_dependencies.data();

    vk::CreateRenderPass(m_device->handle(), &m_renderPass_info, nullptr, &m_renderPass);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = m_width;
    image_create_info.extent.height = m_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = multiview_count;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                             VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper();
    framebufferCreateInfo.width = m_width;
    framebufferCreateInfo.height = m_height;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView;

    vk::CreateFramebuffer(m_device->device(), &framebufferCreateInfo, nullptr, &m_framebuffer);

    VkClearValue clear{};
    clear.color = m_clear_color;
    m_renderPassClearValues.emplace_back(clear);
    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = m_framebuffer;
    m_renderPassBeginInfo.renderArea.extent.width = m_width;
    m_renderPassBeginInfo.renderArea.extent.height = m_height;
    m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();

    // Pipeline not bound test
    {
        // No need to create individual pipelines for each subpass since we are checking no bound pipeline
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        // This bind should not be valid after we begin the renderpass
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08606");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This bind should not be valid for next subpass
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
            m_commandBuffer->NextSubpass();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08606");
            vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Dynamic state (checking with line width)
    {
        // Pipeline for subpass 0
        CreatePipelineHelper pipe(*this);
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }

        m_commandBuffer->begin();
        // This line width set should not be valid for next subpass
        vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07833");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This line width set should not be valid for next subpass
            vk::CmdSetLineWidth(m_commandBuffer->handle(), 1.0f);
            m_commandBuffer->NextSubpass();
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07833");
            vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Push constants
    {
        char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo {
           mat3 m;
        } constants;
        void main(){
            vec3 v3 = constants.m[0];
        }
    )glsl";

        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 36};
        VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        vkt::PipelineLayout layout(*m_device, pipeline_layout_info, std::vector<const vkt::DescriptorSetLayout *>{});

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        const float dummy_values[16] = {};
        m_commandBuffer->begin();
        // This push constants should not be counted when render pass begins
        vk::CmdPushConstants(m_commandBuffer->handle(), layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, push_constant_range.offset,
                             push_constant_range.size, dummy_values);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-maintenance4-08602");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This push constants should not be counted when we change subpass
            vk::CmdPushConstants(m_commandBuffer->handle(), layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, push_constant_range.offset,
                                 push_constant_range.size, dummy_values);
            m_commandBuffer->NextSubpass();
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-maintenance4-08602");
            vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Descriptor sets
    {
        OneOffDescriptorSet descriptor_set{m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}}};

        VkBufferCreateInfo bci = vku::InitStructHelper();
        bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bci.size = 8;
        vkt::Buffer buffer(*m_device, bci);
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &descriptor_set.layout_.handle();

        vkt::PipelineLayout layout(*m_device, pipeline_layout_info, std::vector<vkt::DescriptorSetLayout const *>{});

        VkShaderObj const vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj const fs(this, kFragmentUniformGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        m_commandBuffer->begin();
        // This descriptor bind should not be counted when render pass begins
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08600");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This descriptor bind should not be counted when next subpass begins
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0,
                                      1, &descriptor_set.set_, 0, nullptr);
            m_commandBuffer->NextSubpass();
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08600");
            vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Vertex buffer
    {
        float const vertex_data[] = {1.0f, 0.0f};
        vkt::Buffer vbo(*m_device, sizeof(vertex_data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        VkVertexInputBindingDescription input_binding{};
        input_binding.binding = 0;
        input_binding.stride = sizeof(vertex_data);
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription input_attribs{};
        input_attribs.binding = 0;
        input_attribs.location = 0;
        input_attribs.format = VK_FORMAT_R32G32_SFLOAT;
        input_attribs.offset = 0;

        char const *const vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        void main(){
           gl_Position = vec4(input0.x, input0.y, 0.0f, 1.0f);
        }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.vi_ci_.vertexBindingDescriptionCount = 1;
        pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
        pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete
        VkDeviceSize offset = 0;

        m_commandBuffer->begin();
        // This vertex buffer bind should not be counted when render pass begins
        vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04007");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This vertex buffer bind should not be counted when next subpass begins
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            m_commandBuffer->NextSubpass();
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04007");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");
            vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    m_commandBuffer->reset();

    // Index buffer
    {
        float const vertex_data[] = {1.0f, 0.0f};
        vkt::Buffer vbo(*m_device, sizeof(vertex_data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        uint32_t const index_data[] = {0};
        vkt::Buffer ibo(*m_device, sizeof(index_data), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        VkVertexInputBindingDescription input_binding{};
        input_binding.binding = 0;
        input_binding.stride = sizeof(vertex_data);
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription input_attribs{};
        input_attribs.binding = 0;
        input_attribs.location = 0;
        input_attribs.format = VK_FORMAT_R32G32_SFLOAT;
        input_attribs.offset = 0;

        char const *const vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        void main(){
           gl_Position = vec4(input0.x, input0.y, 0.0f, 1.0f);
        }
    )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.vi_ci_.vertexBindingDescriptionCount = 1;
        pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
        pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        VkDeviceSize offset = 0;
        m_commandBuffer->begin();
        // This index buffer bind should not be counted when render pass begins
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-None-07312");
        vk::CmdDrawIndexed(m_commandBuffer->handle(), 0, 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This index buffer bind should not be counted when next subpass begins
            vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);
            m_commandBuffer->NextSubpass();
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-None-07312");
            vk::CmdDrawIndexed(m_commandBuffer->handle(), 0, 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeMultiview, BeginTransformFeedback) {
    TEST_DESCRIPTION("Test beginning transform feedback in a render pass with multiview enabled");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeaturesKHR mv_features = vku::InitStructHelper();
    VkPhysicalDeviceTransformFeedbackFeaturesEXT tf_features = vku::InitStructHelper(&mv_features);
    auto pd_features = GetPhysicalDeviceFeatures2(tf_features);

    if (!tf_features.transformFeedback) {
        GTEST_SKIP() << "transformFeedback not supported; skipped.";
    }
    if (!mv_features.multiview) {
        GTEST_SKIP() << "multiview not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &pd_features));
    InitRenderTarget();

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    VkRenderPassCreateInfo renderPassCreateInfo = vku::InitStructHelper(&renderPassMultiviewCreateInfo);
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    vkt::RenderPass render_pass;
    render_pass.init(*m_device, renderPassCreateInfo);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    auto image_view_ci = image.BasicViewCreatInfo();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    VkImageView imageView = image.targetView(image_view_ci);

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper();
    framebufferCreateInfo.width = 32;
    framebufferCreateInfo.height = 32;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = render_pass.handle();
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView;

    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.framebuffer = framebuffer.handle();
    render_pass_begin_info.renderArea.extent.width = 32;
    render_pass_begin_info.renderArea.extent.height = 32;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-None-04128");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-None-02373");
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeMultiview, Features) {
    TEST_DESCRIPTION("Checks VK_KHR_multiview features.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    std::vector<const char *> device_extensions;
    device_extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);

    // Set false to trigger VUs
    multiview_features.multiview = VK_FALSE;

    vkt::PhysicalDevice physical_device(gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features2);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledExtensionCount = device_extensions.size();
    VkDevice testDevice;

    if ((multiview_features.multiviewGeometryShader == VK_FALSE) && (multiview_features.multiviewTessellationShader == VK_FALSE)) {
        GTEST_SKIP() << "multiviewGeometryShader and multiviewTessellationShader feature not supported";
    }

    if (multiview_features.multiviewGeometryShader == VK_TRUE) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewGeometryShader-00580");
    }
    if (multiview_features.multiviewTessellationShader == VK_TRUE) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581");
    }
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMultiview, RenderPassCreateOverlappingCorrelationMasks) {
    TEST_DESCRIPTION("Create a subpass with overlapping correlation masks");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    const VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    std::array viewMasks = {0x3u};
    std::array correlationMasks = {0x1u, 0x2u};
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(
        nullptr, 1u, viewMasks.data(), 0u, nullptr, static_cast<uint32_t>(correlationMasks.size()), correlationMasks.data());
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 1u, &subpass, 0u, nullptr);

    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, false);

    // Correlation masks must not overlap
    correlationMasks[1] = 0x3u;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                         "VUID-VkRenderPassMultiviewCreateInfo-pCorrelationMasks-00841",
                         "VUID-VkRenderPassCreateInfo2-pCorrelatedViewMasks-03056");

    // Check for more specific "don't set any correlation masks when multiview is not enabled"
    if (rp2Supported) {
        viewMasks[0] = 0;
        correlationMasks[0] = 0;
        correlationMasks[1] = 0;
        safe_VkRenderPassCreateInfo2 safe_rpci2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);

        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *safe_rpci2.ptr(), {"VUID-VkRenderPassCreateInfo2-viewMask-03057"});
    }
}

TEST_F(NegativeMultiview, RenderPassViewMasksNotEnough) {
    TEST_DESCRIPTION("Create a subpass with the wrong number of view masks, or inconsistent setting of view masks");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };
    uint32_t viewMasks[] = {0x3u, 0u};
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 1u, viewMasks, 0u, nullptr, 0u, nullptr);
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 2u, subpasses, 0u, nullptr);

    // Not enough view masks
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pNext-01928",
                         "VUID-VkRenderPassCreateInfo2-viewMask-03058");
}

TEST_F(NegativeMultiview, RenderPassCreateSubpassMissingAttributesBitNVX) {
    TEST_DESCRIPTION("Create a subpass with the VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX flag missing");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitState())

    VkSubpassDescription subpasses[] = {
        {VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr,
         nullptr, 0, nullptr},
    };

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 0u, nullptr, 1u, subpasses, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-flags-00856",
                         "VUID-VkSubpassDescription2-flags-03076");
}

TEST_F(NegativeMultiview, DrawWithPipelineIncompatibleWithRenderPass) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible cases: drawing with an active renderpass that's not compatible with the bound pipeline state "
        "object's creation renderpass since only the former uses Multiview.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (!features2.features.multiViewport) {
        GTEST_SKIP() << "multiViewport feature is not supported, skipping test.\n";
    }
    if (!multiview_features.multiview) {
        GTEST_SKIP() << "multiview feature is not supported, skipping test.\n";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Set up VkRenderPassCreateInfo struct used with VK_VERSION_1_0
    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription attach = {};
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;

    uint32_t viewMasks[] = {0x3u};
    VkRenderPassMultiviewCreateInfo rpmvci = vku::InitStructHelper();
    rpmvci.subpassCount = 1;
    rpmvci.pViewMasks = viewMasks;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpmvci);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    // Set up VkRenderPassCreateInfo2 struct used with VK_VERSION_1_2
    VkAttachmentReference2 color_att2 = vku::InitStructHelper();
    color_att2.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription2 attach2 = vku::InitStructHelper();
    attach2.samples = VK_SAMPLE_COUNT_1_BIT;
    attach2.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach2.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach2.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass2 = vku::InitStructHelper();
    subpass2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass2.viewMask = 0x3u;
    subpass2.colorAttachmentCount = 1;
    subpass2.pColorAttachments = &color_att2;

    VkRenderPassCreateInfo2 rpci2 = vku::InitStructHelper();
    rpci2.attachmentCount = 1;
    rpci2.pAttachments = &attach2;
    rpci2.subpassCount = 1;
    rpci2.pSubpasses = &subpass2;

    // Create render passes with VK_VERSION_1_0 struct and vkCreateRenderPass call
    // Create rp[0] with Multiview pNext, rp[1] without Multiview pNext, rp[2] with Multiview pNext but another viewMask
    std::array<vkt::RenderPass, 3> rp;
    rp[0].init(*m_device, rpci);
    rpci.pNext = nullptr;
    rp[1].init(*m_device, rpci);
    uint32_t viewMasks2[] = {0x1u};
    rpmvci.pViewMasks = viewMasks2;
    rpci.pNext = &rpmvci;
    rp[2].init(*m_device, rpci);

    // Create render passes with VK_VERSION_1_2 struct and vkCreateRenderPass2KHR call
    // Create rp2[0] with Multiview, rp2[1] without Multiview (zero viewMask), rp2[2] with Multiview but another viewMask
    std::array<vkt::RenderPass, 3> rp2;
    if (rp2Supported) {
        rp2[0].init(*m_device, rpci2, true);
        subpass2.viewMask = 0x0u;
        rpci2.pSubpasses = &subpass2;
        rp2[1].init(*m_device, rpci2, true);
        subpass2.viewMask = 0x1u;
        rpci2.pSubpasses = &subpass2;
        rp2[2].init(*m_device, rpci2, true);
    }

    // Create image view
    VkImageObj image(m_device);
    auto ici2d = image.ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                         VK_IMAGE_TILING_OPTIMAL, 0);
    image.Init(ici2d);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    vkt::ImageView iv(*m_device, ivci);

    // Create framebuffers for rp[0] and rp2[0]
    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.renderPass = rp[0].handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &iv.handle();
    fbci.width = 128;
    fbci.height = 128;
    fbci.layers = 1;

    vkt::Framebuffer fb(*m_device, fbci);
    vkt::Framebuffer fb2;
    if (rp2Supported) {
        fbci.renderPass = rp2[0].handle();
        fb2.init(*m_device, fbci);
    }

    VkRenderPassBeginInfo rp_begin = vku::InitStructHelper();
    rp_begin.renderPass = rp[0].handle();
    rp_begin.framebuffer = fb.handle();
    rp_begin.renderArea = {{0, 0}, {128, 128}};

    // Create a graphics pipeline with rp[1]
    CreatePipelineHelper pipe_1(*this);
    pipe_1.InitState();
    pipe_1.gp_ci_.layout = pipeline_layout.handle();
    pipe_1.gp_ci_.renderPass = rp[1].handle();
    pipe_1.CreateGraphicsPipeline();

    // Create a graphics pipeline with rp[2]
    CreatePipelineHelper pipe_2(*this);
    pipe_2.InitState();
    pipe_2.gp_ci_.layout = pipeline_layout.handle();
    pipe_2.gp_ci_.renderPass = rp[2].handle();
    pipe_2.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2_1(*this);
    CreatePipelineHelper pipe2_2(*this);
    if (rp2Supported) {
        pipe2_1.InitState();
        pipe2_1.gp_ci_.layout = pipeline_layout.handle();
        pipe2_1.gp_ci_.renderPass = rp[1].handle();
        pipe2_1.CreateGraphicsPipeline();

        pipe2_2.InitState();
        pipe2_2.gp_ci_.layout = pipeline_layout.handle();
        pipe2_2.gp_ci_.renderPass = rp[2].handle();
        pipe2_2.CreateGraphicsPipeline();
    }

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = rp[0].handle();
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    // Begin rp[0] for VK_VERSION_1_0 test cases
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    // Bind rp[1]'s pipeline to command buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.Handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (error on Multiview usage should trigger on draw)
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // Bind rp[2]'s pipeline to command buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (error on non-matching viewMasks for Multiview usage should trigger on draw)
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // End rp[0]
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Begin rp2[0] for VK_VERSION_1_2 test cases
    if (rp2Supported) {
        cbii.renderPass = rp2[0].handle();
        rp_begin.renderPass = rp2[0].handle();
        rp_begin.framebuffer = fb2.handle();
        vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

        // Bind rp2[1]'s pipeline to command buffer
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2_1.Handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
        // Render triangle (error on Multiview usage should trigger on draw)
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // Bind rp2[2]'s pipeline to command buffer
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2_2.Handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
        // Render triangle (error on non-matching viewMasks for Multiview usage should trigger on draw)
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // End rp2[0]
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeMultiview, RenderPassViewMasksZero) {
    TEST_DESCRIPTION("Create a render pass with some view masks 0 and some not 0");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceMultiviewProperties render_pass_multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);
    if (render_pass_multiview_props.maxMultiviewViewCount < 2) {
        GTEST_SKIP() << "maxMultiviewViewCount lower than required";
    }

    VkSubpassDescription subpasses[2];
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x3u, 0x0};
    uint32_t correlationMasks[] = {0x1u, 0x3u};
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 2u, viewMasks, 0u, nullptr, 2u, correlationMasks);

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 2u, subpasses, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-02513", nullptr);
}

TEST_F(NegativeMultiview, RenderPassViewOffsets) {
    TEST_DESCRIPTION("Create a render pass with invalid multiview pViewOffsets");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceMultiviewProperties render_pass_multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);
    if (render_pass_multiview_props.maxMultiviewViewCount < 2) {
        GTEST_SKIP() << "maxMultiviewViewCount lower than required";
    }

    VkSubpassDescription subpasses[2];
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x1u, 0x2u};
    uint32_t correlationMasks[] = {0x1u, 0x2u};
    int32_t view_offset = 1;
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 2u, viewMasks, 1u, &view_offset, 2u, correlationMasks);

    VkSubpassDependency dependency = {};
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 2u, subpasses, 1u, &dependency);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-02512", nullptr);
}

TEST_F(NegativeMultiview, RenderPassViewMasksLimit) {
    TEST_DESCRIPTION("Create a render pass with invalid view mask");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceMultiviewProperties render_pass_multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);

    if (render_pass_multiview_props.maxMultiviewViewCount >= 32) {
        GTEST_SKIP() << "maxMultiviewViewCount too high";
    }

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMask = 1 << render_pass_multiview_props.maxMultiviewViewCount;
    uint32_t correlationMask = 0x1u;
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 1u, &viewMask, 0u, nullptr, 1u, &correlationMask);

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassMultiviewCreateInfo-pViewMasks-06697", nullptr);
}

TEST_F(NegativeMultiview, FeaturesDisabled) {
    TEST_DESCRIPTION("Create graphics pipeline using multiview features which are not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);

    if (!multiview_features.multiview) {
        GTEST_SKIP() << "Multiview support is required";
    }

    multiview_features.multiviewTessellationShader = VK_FALSE;
    multiview_features.multiviewGeometryShader = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAttachmentReference2 color_attachment = vku::InitStructHelper();
    color_attachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription2 description = vku::InitStructHelper();
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.format = VK_FORMAT_B8G8R8A8_UNORM;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 0x3u;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &description;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, rpci);
    ASSERT_TRUE(render_pass.initialized());

    if (features2.features.tessellationShader) {
        char const *tcsSource = R"glsl(
        #version 450
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
        }
        )glsl";
        char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = 1.0f;
        }
        )glsl";

        VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

        VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                     VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

        VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.renderPass = render_pass.handle();
        pipe.gp_ci_.subpass = 0;
        pipe.cb_ci_.attachmentCount = 1;
        pipe.gp_ci_.pTessellationState = &tsci;
        pipe.gp_ci_.pInputAssemblyState = &iasci;
        pipe.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
        pipe.shader_stages_.emplace_back(tes.GetStageCreateInfo());
        pipe.InitState();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06047");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
    if (features2.features.geometryShader) {
        static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 3) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
           EmitVertex();
        }
        )glsl";

        VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.renderPass = render_pass.handle();
        pipe.gp_ci_.subpass = 0;
        pipe.cb_ci_.attachmentCount = 1;
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06048");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMultiview, DynamicRenderingMaxMultiviewInstanceIndex) {
    TEST_DESCRIPTION("Draw with multiview enabled and instance index too high.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper(&dynamic_rendering_features);
    GetPhysicalDeviceFeatures2(features11);
    if (!features11.multiview) {
        GTEST_SKIP() << "multiview not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features11));

    VkPhysicalDeviceMultiviewProperties multiview_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_properties);

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.viewMask = 0x1;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.CreateGraphicsPipeline();

    VkImageObj img(m_device);
    img.Init(m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageView view = img.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 1u;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.viewMask = 0x1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(renderingInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, multiview_properties.maxMultiviewInstanceIndex);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}
