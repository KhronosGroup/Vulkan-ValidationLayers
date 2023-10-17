/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

TEST_F(NegativePipeline, NotBound) {
    TEST_DESCRIPTION("Pass in an invalid pipeline object handle into a Vulkan API call.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipeline badPipeline = CastToHandle<VkPipeline, uintptr_t>(0xbaadb1be);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, badPipeline);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, WrongBindPointGraphics) {
    TEST_DESCRIPTION("Bind a compute pipeline in the graphics bind point");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineBindPoint-00779");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, BasicCompute) {
    TEST_DESCRIPTION("Bind a compute pipeline (no subpasses)");
    RETURN_IF_SKIP(Init())

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform block { vec4 x; };
    void main(){
        vec4 v = 2.0 * x;
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 1024;
    vkt::Buffer buffer(*m_device, bci);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
}

TEST_F(NegativePipeline, WrongBindPointCompute) {
    TEST_DESCRIPTION("Bind a graphics pipeline in the compute bind point");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineBindPoint-00780");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, DisabledIndependentBlend) {
    TEST_DESCRIPTION(
        "Generate INDEPENDENT_BLEND by disabling independent blend and then specifying different blend states for two "
        "attachments");
    VkPhysicalDeviceFeatures features = {};
    features.independentBlend = VK_FALSE;
    RETURN_IF_SKIP(Init(&features));

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    // Create a renderPass with two color attachments
    VkAttachmentReference attachments[2] = {};
    attachments[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    attachments[1].attachment = 1;
    attachments[1].layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = attachments;
    subpass.colorAttachmentCount = 2;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;

    VkAttachmentDescription attach_desc[2] = {};
    attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = attach_desc;

    vkt::RenderPass renderpass(*m_device, rpci);
    ASSERT_TRUE(renderpass.initialized());

    CreatePipelineHelper pipe(*this, 2);
    pipe.InitState();
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.gp_ci_.renderPass = renderpass.handle();
    pipe.cb_attachments_[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    pipe.cb_attachments_[0].blendEnable = VK_TRUE;
    pipe.cb_attachments_[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    pipe.cb_attachments_[1].blendEnable = VK_FALSE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-00605");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, BlendingOnFormatWithoutBlendingSupport) {
    TEST_DESCRIPTION("Test that blending is not enabled with a format not support blending");
    VkPhysicalDeviceFeatures features = {};
    features.independentBlend = VK_FALSE;
    RETURN_IF_SKIP(Init(&features));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06041");

    VkFormat non_blending_format = VK_FORMAT_UNDEFINED;
    for (uint32_t i = 1; i <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; i++) {
        VkFormatProperties format_props = m_device->format_properties(static_cast<VkFormat>(i));
        if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) &&
            !(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)) {
            non_blending_format = static_cast<VkFormat>(i);
            break;
        }
    }

    if (non_blending_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Unable to find a color attachment format with no blending support";
    }

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    // Create a renderPass with two color attachments
    VkAttachmentReference attachment = {};
    attachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attachment;
    subpass.colorAttachmentCount = 1;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = non_blending_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = &attach_desc;

    vkt::RenderPass rp(*m_device, rpci);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.cb_attachments_[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    pipe.cb_attachments_[0].blendEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

// Is the Pipeline compatible with the expectations of the Renderpass/subpasses?
TEST_F(VkLayerTest, PipelineRenderpassCompatibility) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline that is incompatible with the requirements of its contained Renderpass/subpasses.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineColorBlendAttachmentState att_state1 = {};
    att_state1.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state1.blendEnable = VK_TRUE;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.cb_attachments_[0] = att_state1;
        helper.gp_ci_.pColorBlendState = nullptr;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-09030");
}

TEST_F(VkLayerTest, CmdBufferPipelineDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a pipeline dependency being destroyed.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    {
        // Use helper to create graphics pipeline
        CreatePipelineHelper helper(*this);
        helper.InitState();
        helper.CreateGraphicsPipeline();

        // Bind helper pipeline to command buffer
        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
        m_commandBuffer->end();

        // pipeline will be destroyed when helper goes out of scope
    }

    // Cause error by submitting command buffer that references destroyed pipeline
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, BadPipelineObject) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    constexpr uint64_t fake_pipeline_handle = 0xbaad6001;
    VkPipeline bad_pipeline = CastFromUint64<VkPipeline>(fake_pipeline_handle);

    // Enable VK_KHR_draw_indirect_count for KHR variants
    AddOptionalExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features12);
    bool has_khr_indirect = IsExtensionsEnabled(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    if (has_khr_indirect && !features12.drawIndirectCount) {
        GTEST_SKIP() << VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME << " is enabled, but the drawIndirectCount is not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features12));
    InitRenderTarget();

    // Attempt to bind an invalid Pipeline to a valid Command Buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, bad_pipeline);
    m_errorMonitor->VerifyFound();

    // Try each of the 6 flavors of Draw()
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);  // Draw*() calls must be submitted within a renderpass

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08606");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vkt::Buffer index_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 2, VK_INDEX_TYPE_UINT16);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-None-08606");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 1, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBufferCreateInfo ci = vku::InitStructHelper();
    ci.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    ci.size = 1024;
    vkt::Buffer buffer(*m_device, ci);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-None-08606");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-None-08606");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 1, 0);
    m_errorMonitor->VerifyFound();

    if (has_khr_indirect) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-08606");
        // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndirectCommand)
        vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
        m_errorMonitor->VerifyFound();

        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-08606");
            // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndirectCommand)
            vk::CmdDrawIndirectCount(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-08606");
        // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndexedIndirectCommand)
        vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
        m_errorMonitor->VerifyFound();

        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-08606");
            // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndexedIndirectCommand)
            vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
            m_errorMonitor->VerifyFound();
        }
    }

    // Also try the Dispatch variants
    m_commandBuffer->EndRenderPass();  // Compute submissions must be outside a renderpass

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08606");
    vk::CmdDispatch(m_commandBuffer->handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchIndirect-None-08606");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchIndirect-offset-00407");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer.handle(), ci.size);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, ShaderStageName) {
    TEST_DESCRIPTION("Create Pipelines with invalid state set");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Attempt to Create Gfx Pipeline w/o a VS
    VkPipelineShaderStageCreateInfo shaderStage = fs.GetStageCreateInfo();  // should be: vs.GetStageCreateInfo();

    auto set_info = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {shaderStage}; };
    constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-pStages-06896",
                                  "VUID-VkGraphicsPipelineCreateInfo-stage-02096"};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);

    // Finally, check the string validation for the shader stage pName variable.  Correct the shader stage data, and bork the
    // string before calling again
    shaderStage = vs.GetStageCreateInfo();
    const uint8_t cont_char = 0xf8;
    char bad_string[] = {static_cast<char>(cont_char), static_cast<char>(cont_char), static_cast<char>(cont_char),
                         static_cast<char>(cont_char)};
    shaderStage.pName = bad_string;

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "contains invalid characters or is badly formed");
}

TEST_F(NegativePipeline, ShaderStageBit) {
    TEST_DESCRIPTION("Create Pipelines with invalid state set");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // Make sure compute pipeline has a compute shader stage set
    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main(){
           if (gl_GlobalInvocationID.x >= 0) { return; }
        }
    )glsl";

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_VERTEX_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-stage-00701");
    cs_pipeline.CreateComputePipeline(false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, SampleRateFeatureDisable) {
    // Enable sample shading in pipeline when the feature is disabled.
    // Disable sampleRateShading here
    VkPhysicalDeviceFeatures device_features = {};
    device_features.sampleRateShading = VK_FALSE;

    RETURN_IF_SKIP(Init(&device_features));
    InitRenderTarget();

    // Cause the error by enabling sample shading...
    auto set_shading_enable = [](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.sampleShadingEnable = VK_TRUE; };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784");
}

TEST_F(NegativePipeline, SampleRateFeatureEnable) {
    // Enable sample shading in pipeline when the feature is disabled.
    RETURN_IF_SKIP(Init())
    if (m_device->phy().features().sampleRateShading == VK_FALSE) {
        GTEST_SKIP() << "SampleRateShading feature is disabled";
    }

    InitRenderTarget();

    auto range_test = [this](float value, bool positive_test) {
        auto info_override = [value](CreatePipelineHelper &helper) {
            helper.pipe_ms_state_ci_.sampleShadingEnable = VK_TRUE;
            helper.pipe_ms_state_ci_.minSampleShading = value;
        };
        if (positive_test) {
            CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit);
        } else {
            CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit,
                                              "VUID-VkPipelineMultisampleStateCreateInfo-minSampleShading-00786");
        }
    };

    range_test(NearestSmaller(0.0F), false);
    range_test(NearestGreater(1.0F), false);
    range_test(0.0F, /* positive_test= */ true);
    range_test(1.0F, /* positive_test= */ true);
}

TEST_F(NegativePipeline, DepthClipControlFeatureDisable) {
    // Enable negativeOneToOne (VK_EXT_depth_clip_control) in pipeline when the feature is disabled.
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineViewportDepthClipControlCreateInfoEXT clip_control = vku::InitStructHelper();
    clip_control.negativeOneToOne = VK_TRUE;
    auto set_shading_enable = [clip_control](CreatePipelineHelper &helper) { helper.vp_state_ci_.pNext = &clip_control; };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineViewportDepthClipControlCreateInfoEXT-negativeOneToOne-06470");
}

TEST_F(NegativePipeline, SamplePNextUnknown) {
    TEST_DESCRIPTION("Pass unknown pNext into VkPipelineMultisampleStateCreateInfo");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineSampleLocationsStateCreateInfoEXT sample_locations = vku::InitStructHelper();
    sample_locations.sampleLocationsInfo = vku::InitStructHelper();
    auto good_chain = [&sample_locations](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = &sample_locations; };
    CreatePipelineHelper::OneshotTest(*this, good_chain, kErrorBit);

    VkInstanceCreateInfo instance_ci = vku::InitStructHelper();
    auto bad_chain = [&instance_ci](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = &instance_ci; };
    CreatePipelineHelper::OneshotTest(*this, bad_chain, kErrorBit, "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext");
}

// TODO 5600 - Not all pNext structs are being passed in to check extensions
TEST_F(NegativePipeline, DISABLED_SamplePNextDisabled) {
    TEST_DESCRIPTION("Pass valid pNext VkPipelineMultisampleStateCreateInfo, but without extension enabled");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineSampleLocationsStateCreateInfoEXT sample_locations = vku::InitStructHelper();
    sample_locations.sampleLocationsInfo = vku::InitStructHelper();
    auto bad_chain = [&sample_locations](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = &sample_locations; };
    CreatePipelineHelper::OneshotTest(*this, bad_chain, kErrorBit, "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext");
}

TEST_F(NegativePipeline, SubpassRasterizationSamples) {
    TEST_DESCRIPTION("Test creating two pipelines referring to the same subpass but with different rasterization samples count");

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures device_features = {};
    device_features.variableMultisampleRate = VK_FALSE;

    RETURN_IF_SKIP(InitState(&device_features));
    InitRenderTarget();

    // Create a render pass with 1 subpass. This subpass uses no attachment.
    std::array<VkAttachmentReference, 1> attachmentRefs = {};
    attachmentRefs[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentRefs[0].attachment = VK_ATTACHMENT_UNUSED;

    VkSubpassDescription subpass = {};
    subpass.flags = VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = attachmentRefs.data();

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = m_renderTargets[0]->format();
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass renderpass(*m_device, rpci);
    ASSERT_TRUE(renderpass.initialized());

    auto render_target_view = m_renderTargets[0]->targetView(m_renderTargets[0]->format());
    VkFramebufferCreateInfo framebuffer_info = vku::InitStructHelper();
    framebuffer_info.renderPass = renderpass;
    framebuffer_info.width = m_renderTargets[0]->width();
    framebuffer_info.height = m_renderTargets[0]->height();
    framebuffer_info.layers = 1;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &render_target_view;
    vkt::Framebuffer framebuffer(*m_device, framebuffer_info);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    CreatePipelineHelper pipeline_1(*this);
    pipeline_1.InitState();
    pipeline_1.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_1.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipeline_1.gp_ci_.renderPass = renderpass.handle();
    pipeline_1.CreateGraphicsPipeline();

    CreatePipelineHelper pipeline_2(*this);
    pipeline_2.InitState();
    pipeline_2.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    pipeline_2.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipeline_2.gp_ci_.renderPass = renderpass.handle();
    pipeline_2.CreateGraphicsPipeline();

    VkRenderPassBeginInfo rpbinfo = vku::InitStructHelper();
    rpbinfo.renderPass = renderpass.handle();
    rpbinfo.framebuffer = framebuffer;
    rpbinfo.renderArea.extent.width = framebuffer_info.width;
    rpbinfo.renderArea.extent.height = framebuffer_info.height;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rpbinfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_1.Handle());

    // VkPhysicalDeviceFeatures::variableMultisampleRate is false,
    // the two pipelines refer to the same subpass, one that does not use any attachment,
    // BUT the secondly created pipeline has a different sample samples count than the 1st, this is illegal
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-subpass-00758");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_2.Handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativePipeline, RenderPassShaderResolveQCOM) {
    TEST_DESCRIPTION("Test pipeline creation VUIDs added with VK_QCOM_render_pass_shader_resolve extension.");
    AddRequiredExtensions(VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME);

    RETURN_IF_SKIP(Init())
    if (m_device->phy().features().sampleRateShading == VK_FALSE) {
        GTEST_SKIP() << "sampleRateShading feature is not supported";
    }
    InitRenderTarget();

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    // Create a renderPass with two attachments (0=Color, 1=Input)
    VkAttachmentReference attachmentRefs[2] = {};
    attachmentRefs[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentRefs[0].attachment = 0;
    attachmentRefs[1].layout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentRefs[1].attachment = 1;

    VkSubpassDescription subpass = {};
    subpass.flags = VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRefs[0];
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attachmentRefs[1];

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;

    VkAttachmentDescription attach_desc[2] = {};
    attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[1].samples = VK_SAMPLE_COUNT_4_BIT;
    attach_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = attach_desc;

    // renderpass has 1xMSAA colorAttachent and 4xMSAA inputAttachment
    vkt::RenderPass renderpass(*m_device, rpci);
    ASSERT_TRUE(renderpass.initialized());

    // renderpass2 has 1xMSAA colorAttachent and 1xMSAA inputAttachment
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::RenderPass renderpass2(*m_device, rpci);
    ASSERT_TRUE(renderpass2.initialized());

    // shader uses gl_SamplePosition which causes the SPIR-V to include SampleRateShading capability
    static const char *sampleRateFragShaderText = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main() {
            uFragColor = vec4(gl_SamplePosition.x,1,0,1);
        }
    )glsl";

    VkShaderObj fs_sampleRate(this, sampleRateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state = vku::InitStructHelper();
    ms_state.flags = 0;
    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state.sampleShadingEnable = VK_FALSE;
    ms_state.minSampleShading = 0.0f;
    ms_state.pSampleMask = nullptr;
    ms_state.alphaToCoverageEnable = VK_FALSE;
    ms_state.alphaToOneEnable = VK_FALSE;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.gp_ci_.renderPass = renderpass.handle();
    pipe.cb_attachments_[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    pipe.cb_attachments_[0].blendEnable = VK_TRUE;
    pipe.gp_ci_.pMultisampleState = &ms_state;

    // Create a pipeline with a subpass using VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // but where sample count of input attachment doesnt match rasterizationSamples
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-rasterizationSamples-04899");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state.sampleShadingEnable = VK_TRUE;
    pipe.gp_ci_.renderPass = renderpass2.handle();

    // Create a pipeline with a subpass using VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // and with sampleShadingEnable enabled in the pipeline
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-sampleShadingEnable-04900");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    ms_state.sampleShadingEnable = VK_FALSE;
    pipe.shader_stages_[1] = fs_sampleRate.GetStageCreateInfo();

    // Create a pipeline with a subpass using VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // and with SampleRateShading capability enabled in the SPIR-V fragment shader
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-SampleRateShading-06378");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, RasterizerDiscardWithFragmentShader) {
    TEST_DESCRIPTION("Create Graphics Pipeline with fragment shader and rasterizer discard");
    RETURN_IF_SKIP(Init())
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkPipelineShaderStageCreateInfo stages[] = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr};

    const VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};

    const VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_TRUE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f};

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
    ASSERT_EQ(VK_SUCCESS, err);

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                               nullptr,
                                                               0,
                                                               2,
                                                               stages,
                                                               &pipeline_vertex_input_state_create_info,
                                                               &pipeline_input_assembly_state_create_info,
                                                               nullptr,
                                                               nullptr,
                                                               &pipeline_rasterization_state_create_info,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               pipeline_layout,
                                                               m_renderPass,
                                                               0,
                                                               VK_NULL_HANDLE,
                                                               -1};

    VkPipeline pipeline;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-06894");
    vk::CreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();

    vk::DestroyPipelineLayout(m_device->handle(), pipeline_layout, nullptr);
}

TEST_F(NegativePipeline, CreateGraphicsPipelineWithBadBasePointer) {
    TEST_DESCRIPTION("Create Graphics Pipeline with pointers that must be ignored by layers");

    RETURN_IF_SKIP(Init())

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr};

    const VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};

    const VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info_template{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f};

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
        pipeline_rasterization_state_create_info_template;
    pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_TRUE;

    constexpr uint64_t fake_pipeline_id = 0xCADECADE;
    VkPipeline fake_pipeline_handle = CastFromUint64<VkPipeline>(fake_pipeline_id);

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                               nullptr,
                                                               VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                                                               1,
                                                               &vs.GetStageCreateInfo(),
                                                               &pipeline_vertex_input_state_create_info,
                                                               &pipeline_input_assembly_state_create_info,
                                                               nullptr,
                                                               nullptr,
                                                               &pipeline_rasterization_state_create_info,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               pipeline_layout,
                                                               m_renderPass,
                                                               0,
                                                               fake_pipeline_handle,
                                                               -1};

    VkPipeline pipeline;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-07984");
    vk::CreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();

    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = 6;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-07985");
    vk::CreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();

    vk::DestroyPipelineLayout(m_device->handle(), pipeline_layout, nullptr);
}

TEST_F(NegativePipeline, PipelineCreationCacheControl) {
    TEST_DESCRIPTION("Test VK_EXT_pipeline_creation_cache_control");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT cache_control_features = vku::InitStructHelper();
    cache_control_features.pipelineCreationCacheControl = VK_FALSE;  // Tests all assume feature is off
    RETURN_IF_SKIP(InitState(nullptr, &cache_control_features));
    InitRenderTarget();

    const auto set_graphics_flags = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
    };
    CreatePipelineHelper::OneshotTest(*this, set_graphics_flags, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-pipelineCreationCacheControl-02878");

    const auto set_compute_flags = [&](CreateComputePipelineHelper &helper) {
        helper.cp_ci_.flags = VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT;
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_compute_flags, kErrorBit,
                                             "VUID-VkComputePipelineCreateInfo-pipelineCreationCacheControl-02875");

    VkPipelineCache pipeline_cache;
    VkPipelineCacheCreateInfo cache_create_info = vku::InitStructHelper();
    cache_create_info.initialDataSize = 0;
    cache_create_info.flags = VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineCacheCreateInfo-pipelineCreationCacheControl-02892");
    vk::CreatePipelineCache(m_device->device(), &cache_create_info, nullptr, &pipeline_cache);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, NumSamplesMismatch) {
    // Create CommandBuffer where MSAA samples doesn't match RenderPass
    // sampleCount
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07284");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = vku::InitStructHelper();
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853");
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Render triangle (the error should trigger on the attempt to draw).
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, NumBlendAttachMismatch) {
    // Create Pipeline where the number of blend attachments doesn't match the
    // number of color attachments.  In this case, we don't add any color
    // blend attachments even though we have a color attachment.

    AddOptionalExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = vku::InitStructHelper();
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const auto set_MSAA = [&](CreatePipelineHelper &helper) {
        helper.pipe_ms_state_ci_ = pipe_ms_state_ci;
        helper.cb_ci_.attachmentCount = 0;
    };
    CreatePipelineHelper::OneshotTest(*this, set_MSAA, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-07609");
}

TEST_F(NegativePipeline, ColorBlendInvalidLogicOp) {
    TEST_DESCRIPTION("Attempt to use invalid VkPipelineColorBlendStateCreateInfo::logicOp value.");

    RETURN_IF_SKIP(Init())  // enables all supported features
    InitRenderTarget();

    if (!m_device->phy().features().logicOp) {
        GTEST_SKIP() << "Device does not support logicOp feature";
    }

    const auto set_shading_enable = [](CreatePipelineHelper &helper) {
        helper.cb_ci_.logicOpEnable = VK_TRUE;
        helper.cb_ci_.logicOp = static_cast<VkLogicOp>(VK_LOGIC_OP_SET + 1);  // invalid logicOp to be tested
    };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00607");
}

TEST_F(NegativePipeline, ColorBlendUnsupportedLogicOp) {
    TEST_DESCRIPTION("Attempt enabling VkPipelineColorBlendStateCreateInfo::logicOpEnable when logicOp feature is disabled.");

    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));
    InitRenderTarget();

    const auto set_shading_enable = [](CreatePipelineHelper &helper) { helper.cb_ci_.logicOpEnable = VK_TRUE; };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00606");
}

TEST_F(NegativePipeline, ColorBlendUnsupportedDualSourceBlend) {
    TEST_DESCRIPTION("Attempt to use dual-source blending when dualSrcBlend feature is disabled.");

    VkPhysicalDeviceFeatures features{};
    RETURN_IF_SKIP(Init(&features));
    InitRenderTarget();

    VkPipelineColorBlendAttachmentState cb_attachments = {};

    const auto set_dsb_src_color_enable = [&](CreatePipelineHelper &helper) { helper.cb_attachments_[0] = cb_attachments; };

    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;
    CreatePipelineHelper::OneshotTest(*this, set_dsb_src_color_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-00608");

    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;  // bad
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;
    CreatePipelineHelper::OneshotTest(*this, set_dsb_src_color_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-00609");

    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC1_ALPHA;  // bad
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;
    CreatePipelineHelper::OneshotTest(*this, set_dsb_src_color_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-00610");

    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;  // bad!
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;
    CreatePipelineHelper::OneshotTest(*this, set_dsb_src_color_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-00611");
}

TEST_F(NegativePipeline, DuplicateStage) {
    TEST_DESCRIPTION("Test that an error is produced for a pipeline containing multiple shaders for the same stage");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), helper.vs_->GetStageCreateInfo(),
                                 helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stage-06897");
}

TEST_F(NegativePipeline, MissingEntrypoint) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // Graphics
    {
        VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "foo");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
    }

    // Compute
    {
        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                       SPV_SOURCE_GLSL, nullptr, "foo");
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
    }

    // Multiple pipeline, middle has missing entrypoint
    {
        CreateComputePipelineHelper pipe_0(*this);  // valid
        pipe_0.InitState();
        pipe_0.LateBindPipelineInfo();
        CreateComputePipelineHelper pipe_1(*this);  // invalid
        pipe_1.cs_ = std::make_unique<VkShaderObj>(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_GLSL, nullptr, "foo");
        pipe_1.InitState();
        pipe_1.LateBindPipelineInfo();

        VkComputePipelineCreateInfo create_infos[3] = {pipe_0.cp_ci_, pipe_1.cp_ci_, pipe_0.cp_ci_};
        VkPipeline pipelines[3];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
        vk::CreateComputePipelines(m_device->device(), VK_NULL_HANDLE, 3, create_infos, nullptr, pipelines);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativePipeline, DepthStencilRequired) {
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "pDepthStencilState is NULL when rasterization is enabled and subpass uses a depth/stencil attachment");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkAttachmentDescription attachments[] = {
        {
            0,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        {
            0,
            VK_FORMAT_D16_UNORM,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
    };
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &refs[0], nullptr, &refs[1], 0, nullptr};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attachments, 1, &subpass, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = descriptorSet.GetPipelineLayout();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, NullStagepName) {
    TEST_DESCRIPTION("Test that an error is produced for a stage with a null pName pointer");

    RETURN_IF_SKIP(Init())
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.shader_stages_[0].pName = nullptr;
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-parameter");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, AMDMixedAttachmentSamplesValidateGraphicsPipeline) {
    TEST_DESCRIPTION("Verify an error message for an incorrect graphics pipeline rasterization sample count.");

    AddRequiredExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // Set a mismatched sample count
    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

    const auto set_info = [&](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_ = ms_state_ci; };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-subpass-01505");
}

TEST_F(NegativePipeline, FramebufferMixedSamplesNV) {
    TEST_DESCRIPTION("Verify VK_NV_framebuffer_mixed_samples.");
    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    if (VK_TRUE != device_features.sampleRateShading) {
        GTEST_SKIP() << "Test requires unsupported sampleRateShading feature";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    struct TestCase {
        VkSampleCountFlagBits color_samples;
        VkSampleCountFlagBits depth_samples;
        VkSampleCountFlagBits raster_samples;
        VkBool32 depth_test;
        VkBool32 sample_shading;
        uint32_t table_count;
        bool positiveTest;
        std::string vuid;
    };

    std::vector<TestCase> test_cases = {
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, VK_FALSE, VK_FALSE, 1, true,
         "VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853"},
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_8_BIT, VK_FALSE, VK_FALSE, 4, false,
         "VUID-VkPipelineCoverageModulationStateCreateInfoNV-coverageModulationTableEnable-01405"},
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_8_BIT, VK_FALSE, VK_FALSE, 2, true,
         "VUID-VkPipelineCoverageModulationStateCreateInfoNV-coverageModulationTableEnable-01405"},
        {VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_8_BIT, VK_TRUE, VK_FALSE, 1, false,
         "VUID-VkGraphicsPipelineCreateInfo-subpass-01411"},
        {VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_8_BIT, VK_TRUE, VK_FALSE, 1, true,
         "VUID-VkGraphicsPipelineCreateInfo-subpass-01411"},
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, VK_FALSE, 1, false,
         "VUID-VkGraphicsPipelineCreateInfo-subpass-01412"},
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_4_BIT, VK_FALSE, VK_FALSE, 1, true,
         "VUID-VkGraphicsPipelineCreateInfo-subpass-01412"},
        {VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, VK_FALSE, VK_TRUE, 1, false,
         "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-01415"},
        {VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, VK_FALSE, VK_FALSE, 1, true,
         "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-01415"},
        {VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_8_BIT, VK_FALSE, VK_FALSE, 1, true,
         "VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853"}};

    for (const auto &test_case : test_cases) {
        VkAttachmentDescription att[2] = {{}, {}};
        att[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        att[0].samples = test_case.color_samples;
        att[0].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
        att[1].samples = test_case.depth_samples;
        att[1].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference dr = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &cr;
        sp.pResolveAttachments = NULL;
        sp.pDepthStencilAttachment = &dr;

        VkRenderPassCreateInfo rpi = vku::InitStructHelper();
        rpi.attachmentCount = 2;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;
        vkt::RenderPass rp(*m_device, rpi);

        VkPipelineDepthStencilStateCreateInfo ds = vku::InitStructHelper();
        VkPipelineCoverageModulationStateCreateInfoNV cmi = vku::InitStructHelper();

        // Create a dummy modulation table that can be used for the positive
        // coverageModulationTableCount test.
        std::vector<float> cm_table{};

        const auto break_samples = [&cmi, &rp, &ds, &cm_table, &test_case](CreatePipelineHelper &helper) {
            cm_table.resize(test_case.table_count);

            cmi.flags = 0;
            cmi.coverageModulationTableEnable = (test_case.table_count > 1);
            cmi.coverageModulationTableCount = test_case.table_count;
            cmi.pCoverageModulationTable = cm_table.data();

            ds.depthTestEnable = test_case.depth_test;

            helper.pipe_ms_state_ci_.pNext = &cmi;
            helper.pipe_ms_state_ci_.rasterizationSamples = test_case.raster_samples;
            helper.pipe_ms_state_ci_.sampleShadingEnable = test_case.sample_shading;

            helper.gp_ci_.renderPass = rp.handle();
            helper.gp_ci_.pDepthStencilState = &ds;
        };

        if (!test_case.positiveTest) {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit, test_case.vuid);
        } else {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit);
        }
    }
}

TEST_F(NegativePipeline, FramebufferMixedSamples) {
    TEST_DESCRIPTION("Verify that the expected VUIds are hits when VK_NV_framebuffer_mixed_samples is disabled.");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    struct TestCase {
        VkSampleCountFlagBits color_samples;
        VkSampleCountFlagBits depth_samples;
        VkSampleCountFlagBits raster_samples;
        bool positiveTest;
    };

    std::vector<TestCase> test_cases = {
        {VK_SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_8_BIT,
         false},  // Fails vk::CreateRenderPass and vk::CreateGraphicsPipeline
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_8_BIT, false},  // Fails vk::CreateGraphicsPipeline
        {VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT, true}    // Pass
    };

    for (const auto &test_case : test_cases) {
        VkAttachmentDescription att[2] = {{}, {}};
        att[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        att[0].samples = test_case.color_samples;
        att[0].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = ds_format;
        att[1].samples = test_case.depth_samples;
        att[1].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference dr = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &cr;
        sp.pResolveAttachments = NULL;
        sp.pDepthStencilAttachment = &dr;

        VkRenderPassCreateInfo rpi = vku::InitStructHelper();
        rpi.attachmentCount = 2;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        if (test_case.color_samples == test_case.depth_samples) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-pDepthStencilAttachment-01418");
        }
        vkt::RenderPass rp(*m_device, rpi);

        if (test_case.color_samples == test_case.depth_samples) {
        } else {
            m_errorMonitor->VerifyFound();
            continue;
        }

        VkPipelineDepthStencilStateCreateInfo ds = vku::InitStructHelper();

        const auto break_samples = [&rp, &ds, &test_case](CreatePipelineHelper &helper) {
            helper.pipe_ms_state_ci_.rasterizationSamples = test_case.raster_samples;

            helper.gp_ci_.renderPass = rp.handle();
            helper.gp_ci_.pDepthStencilState = &ds;
        };

        if (!test_case.positiveTest) {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit,
                                              "VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853");
        } else {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit);
        }
    }
}

TEST_F(NegativePipeline, FramebufferMixedSamplesCoverageReduction) {
    TEST_DESCRIPTION("Verify VK_NV_coverage_reduction_mode.");

    AddRequiredExtensions(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    if (!IsExtensionsEnabled(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME) &&
        !IsExtensionsEnabled(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        GTEST_SKIP() << "Extensions not supported";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    struct TestCase {
        VkSampleCountFlagBits raster_samples;
        VkSampleCountFlagBits color_samples;
        VkSampleCountFlagBits depth_samples;
        VkCoverageReductionModeNV coverage_reduction_mode;
        bool positiveTest;
        std::string vuid;
    };

    std::vector<TestCase> test_cases;

    uint32_t combination_count = 0;
    std::vector<VkFramebufferMixedSamplesCombinationNV> combinations;

    vk::GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(gpu(), &combination_count, nullptr);
    if (combination_count < 1) {
        GTEST_SKIP() << "No mixed sample combinations are supported";
    }
    combinations.resize(combination_count);
    // TODO this fill can be removed once https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4138 merges
    std::fill(combinations.begin(), combinations.end(), vku::InitStruct<VkFramebufferMixedSamplesCombinationNV>());
    vk::GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(gpu(), &combination_count, &combinations[0]);

    // Pick the first supported combination for a positive test.
    test_cases.push_back({combinations[0].rasterizationSamples, static_cast<VkSampleCountFlagBits>(combinations[0].colorSamples),
                          static_cast<VkSampleCountFlagBits>(combinations[0].depthStencilSamples),
                          combinations[0].coverageReductionMode, true,
                          "VUID-VkGraphicsPipelineCreateInfo-coverageReductionMode-02722"});

    VkSampleCountFlags fb_sample_counts = m_device->phy().limits_.framebufferDepthSampleCounts;
    int max_sample_count = VK_SAMPLE_COUNT_64_BIT;
    while (max_sample_count > VK_SAMPLE_COUNT_1_BIT) {
        if (fb_sample_counts & max_sample_count) {
            break;
        }
        max_sample_count /= 2;
    }
    // Look for a valid combination that is not in the supported list for a negative test.
    bool neg_comb_found = false;
    for (int mode = VK_COVERAGE_REDUCTION_MODE_TRUNCATE_NV; mode >= 0 && !neg_comb_found; mode--) {
        for (int rs = max_sample_count; rs >= VK_SAMPLE_COUNT_1_BIT && !neg_comb_found; rs /= 2) {
            for (int ds = rs; ds >= 0 && !neg_comb_found; ds -= rs) {
                for (int cs = rs / 2; cs > 0 && !neg_comb_found; cs /= 2) {
                    bool combination_found = false;
                    for (const auto &combination : combinations) {
                        if (mode == combination.coverageReductionMode && rs == combination.rasterizationSamples &&
                            ds & combination.depthStencilSamples && cs & combination.colorSamples) {
                            combination_found = true;
                            break;
                        }
                    }

                    if (!combination_found) {
                        neg_comb_found = true;
                        test_cases.push_back({static_cast<VkSampleCountFlagBits>(rs), static_cast<VkSampleCountFlagBits>(cs),
                                              static_cast<VkSampleCountFlagBits>(ds), static_cast<VkCoverageReductionModeNV>(mode),
                                              false, "VUID-VkGraphicsPipelineCreateInfo-coverageReductionMode-02722"});
                    }
                }
            }
        }
    }

    for (const auto &test_case : test_cases) {
        VkAttachmentDescription att[2] = {{}, {}};
        att[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        att[0].samples = test_case.color_samples;
        att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
        att[1].samples = test_case.depth_samples;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference dr = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &cr;
        sp.pResolveAttachments = nullptr;
        sp.pDepthStencilAttachment = (test_case.depth_samples) ? &dr : nullptr;

        VkRenderPassCreateInfo rpi = vku::InitStructHelper();
        rpi.attachmentCount = (test_case.depth_samples) ? 2 : 1;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        vkt::RenderPass rp(*m_device, rpi);
        ASSERT_TRUE(rp.initialized());

        VkPipelineDepthStencilStateCreateInfo dss = vku::InitStructHelper();
        VkPipelineCoverageReductionStateCreateInfoNV crs = vku::InitStructHelper();

        const auto break_samples = [&rp, &dss, &crs, &test_case](CreatePipelineHelper &helper) {
            crs.flags = 0;
            crs.coverageReductionMode = test_case.coverage_reduction_mode;

            helper.pipe_ms_state_ci_.pNext = &crs;
            helper.pipe_ms_state_ci_.rasterizationSamples = test_case.raster_samples;
            helper.gp_ci_.renderPass = rp.handle();
            helper.gp_ci_.pDepthStencilState = (test_case.depth_samples) ? &dss : nullptr;
        };

        if (!test_case.positiveTest) {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit, test_case.vuid);
        } else {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit);
        }
    }
}

TEST_F(NegativePipeline, FragmentCoverageToColorNV) {
    TEST_DESCRIPTION("Verify VK_NV_fragment_coverage_to_color.");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    struct TestCase {
        VkFormat format;
        VkBool32 enabled;
        uint32_t location;
        bool positive;
    };

    const std::array<TestCase, 9> test_cases = {{
        {VK_FORMAT_R8G8B8A8_UNORM, VK_FALSE, 0, true},
        {VK_FORMAT_R8_UINT, VK_TRUE, 1, true},
        {VK_FORMAT_R16_UINT, VK_TRUE, 1, true},
        {VK_FORMAT_R16_SINT, VK_TRUE, 1, true},
        {VK_FORMAT_R32_UINT, VK_TRUE, 1, true},
        {VK_FORMAT_R32_SINT, VK_TRUE, 1, true},
        {VK_FORMAT_R32_SINT, VK_TRUE, 2, false},
        {VK_FORMAT_R8_SINT, VK_TRUE, 3, false},
        {VK_FORMAT_R8G8B8A8_UNORM, VK_TRUE, 1, false},
    }};

    for (const auto &test_case : test_cases) {
        std::array<VkAttachmentDescription, 2> att = {{{}, {}}};
        att[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        att[0].samples = VK_SAMPLE_COUNT_1_BIT;
        att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = VK_FORMAT_R8G8B8A8_UNORM;
        att[1].samples = VK_SAMPLE_COUNT_1_BIT;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (test_case.location < att.size()) {
            att[test_case.location].format = test_case.format;
        }

        const std::array<VkAttachmentReference, 3> cr = {{{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                          {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                          {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = cr.size();
        sp.pColorAttachments = cr.data();

        VkRenderPassCreateInfo rpi = vku::InitStructHelper();
        rpi.attachmentCount = att.size();
        rpi.pAttachments = att.data();
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        const std::array<VkPipelineColorBlendAttachmentState, 3> cba = {{{}, {}, {}}};

        VkPipelineColorBlendStateCreateInfo cbi = vku::InitStructHelper();
        cbi.attachmentCount = cba.size();
        cbi.pAttachments = cba.data();

        vkt::RenderPass rp(*m_device, rpi);
        ASSERT_TRUE(rp.initialized());

        VkPipelineCoverageToColorStateCreateInfoNV cci = vku::InitStructHelper();

        const auto break_samples = [&cci, &cbi, &rp, &test_case](CreatePipelineHelper &helper) {
            cci.coverageToColorEnable = test_case.enabled;
            cci.coverageToColorLocation = test_case.location;

            helper.pipe_ms_state_ci_.pNext = &cci;
            helper.gp_ci_.renderPass = rp.handle();
            helper.gp_ci_.pColorBlendState = &cbi;
        };

        if (!test_case.positive) {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit,
                                              "VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404");
        } else {
            CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit);
        }
    }
}

TEST_F(NegativePipeline, ViewportSwizzleNV) {
    AddRequiredExtensions(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPipelineViewportSwizzleStateCreateInfoNV vp_swizzle_state = vku::InitStructHelper();

    // Test invalid VkViewportSwizzleNV
    {
        const VkViewportSwizzleNV invalid_swizzles = {
            static_cast<VkViewportCoordinateSwizzleNV>(-1),
            static_cast<VkViewportCoordinateSwizzleNV>(-1),
            static_cast<VkViewportCoordinateSwizzleNV>(-1),
            static_cast<VkViewportCoordinateSwizzleNV>(-1),
        };

        vp_swizzle_state.viewportCount = 1;
        vp_swizzle_state.pViewportSwizzles = &invalid_swizzles;

        const std::vector<std::string> expected_vuids = {
            "VUID-VkViewportSwizzleNV-x-parameter", "VUID-VkViewportSwizzleNV-y-parameter", "VUID-VkViewportSwizzleNV-z-parameter",
            "VUID-VkViewportSwizzleNV-w-parameter"};

        auto break_swizzles = [&vp_swizzle_state](CreatePipelineHelper &helper) { helper.vp_state_ci_.pNext = &vp_swizzle_state; };

        CreatePipelineHelper::OneshotTest(*this, break_swizzles, kErrorBit, expected_vuids);
    }

    // Test case where VkPipelineViewportSwizzleStateCreateInfoNV::viewportCount is LESS THAN viewportCount set in
    // VkPipelineViewportStateCreateInfo
    {
        const VkViewportSwizzleNV swizzle = {
            VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
            VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV};

        vp_swizzle_state.viewportCount = 1;
        vp_swizzle_state.pViewportSwizzles = &swizzle;

        std::array<VkViewport, 2> viewports = {};
        std::array<VkRect2D, 2> scissors = {};

        viewports.fill({0, 0, 16, 16, 0, 1});
        scissors.fill({{0, 0}, {16, 16}});

        auto break_vp_count = [&vp_swizzle_state, &viewports, &scissors](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = size32(viewports);
            helper.vp_state_ci_.pViewports = viewports.data();
            helper.vp_state_ci_.scissorCount = size32(scissors);
            helper.vp_state_ci_.pScissors = scissors.data();
            helper.vp_state_ci_.pNext = &vp_swizzle_state;
            ASSERT_TRUE(vp_swizzle_state.viewportCount < helper.vp_state_ci_.viewportCount);
        };

        CreatePipelineHelper::OneshotTest(*this, break_vp_count, kErrorBit,
                                          "VUID-VkPipelineViewportSwizzleStateCreateInfoNV-viewportCount-01215");
    }
}

TEST_F(NegativePipeline, CreationFeedbackCount) {
    TEST_DESCRIPTION("Test graphics pipeline feedback stage count check.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Driver data writeback check not supported by MockICD";
    }

    VkPipelineCreationFeedbackCreateInfoEXT feedback_info = vku::InitStructHelper();
    VkPipelineCreationFeedbackEXT feedbacks[3] = {};
    // Set flags to known value that the driver has to overwrite
    feedbacks[0].flags = VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM;

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &feedback_info; };

    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit);

    if (feedback_info.pPipelineCreationFeedback->flags == VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM) {
        m_errorMonitor->SetError("ValidationLayers did not return GraphicsPipelineFeedback driver data properly.");
    }

    feedback_info.pipelineStageCreationFeedbackCount = 1;
    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-pipelineStageCreationFeedbackCount-06594");
}

TEST_F(NegativePipeline, CreationFeedbackCountCompute) {
    TEST_DESCRIPTION("Test compute pipeline feedback stage count check.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPipelineCreationFeedbackCreateInfoEXT feedback_info = vku::InitStructHelper();
    VkPipelineCreationFeedbackEXT feedbacks[3] = {};
    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 1;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    const auto set_info = [&](CreateComputePipelineHelper &helper) { helper.cp_ci_.pNext = &feedback_info; };

    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                             "VUID-VkComputePipelineCreateInfo-pipelineStageCreationFeedbackCount-06566");
}

TEST_F(NegativePipeline, LineRasterization) {
    TEST_DESCRIPTION("Test VK_EXT_line_rasterization state against feature enables.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceLineRasterizationFeaturesEXT line_rasterization_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(line_rasterization_features);

    line_rasterization_features.rectangularLines = VK_FALSE;
    line_rasterization_features.bresenhamLines = VK_FALSE;
    line_rasterization_features.smoothLines = VK_FALSE;
    line_rasterization_features.stippledRectangularLines = VK_FALSE;
    line_rasterization_features.stippledBresenhamLines = VK_FALSE;
    line_rasterization_features.stippledSmoothLines = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    {
        constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769"};
        CreatePipelineHelper::OneshotTest(
            *this,
            [&](CreatePipelineHelper &helper) {
                helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
                helper.pipe_ms_state_ci_.alphaToCoverageEnable = VK_TRUE;
            },
            kErrorBit, vuids);
    }
    {
        constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02772"};
        CreatePipelineHelper::OneshotTest(
            *this,
            [&](CreatePipelineHelper &helper) {
                helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
                helper.line_state_ci_.stippledLineEnable = VK_TRUE;
            },
            kErrorBit, vuids);
    }
    {
        constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02768",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02771"};
        CreatePipelineHelper::OneshotTest(
            *this,
            [&](CreatePipelineHelper &helper) {
                helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
                helper.line_state_ci_.stippledLineEnable = VK_TRUE;
            },
            kErrorBit, vuids);
    }
    {
        constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02770",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02773"};
        CreatePipelineHelper::OneshotTest(
            *this,
            [&](CreatePipelineHelper &helper) {
                helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
                helper.line_state_ci_.stippledLineEnable = VK_TRUE;
            },
            kErrorBit, vuids);
    }
    {
        constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                      "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02774"};
        CreatePipelineHelper::OneshotTest(
            *this,
            [&](CreatePipelineHelper &helper) {
                helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
                helper.line_state_ci_.stippledLineEnable = VK_TRUE;
            },
            kErrorBit, vuids);
    }

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776");
    vk::CmdSetLineStippleEXT(m_commandBuffer->handle(), 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdSetLineStippleEXT(m_commandBuffer->handle(), 1, 1);
}

TEST_F(NegativePipeline, NotCompatibleForSet) {
    TEST_DESCRIPTION("Check that validation path catches pipeline layout inconsistencies for bind vs. dispatch");
    RETURN_IF_SKIP(Init())

    if (m_device->compute_queues().empty()) {
        GTEST_SKIP() << "compute queue not supported";
    }

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 4;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer storage_buffer(*m_device, bci, mem_props);

    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 20;
    vkt::Buffer uniform_buffer(*m_device, bci, mem_props);

    OneOffDescriptorSet::Bindings binding_defs = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, binding_defs);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    // We now will use a slightly different Layout definition for the descriptors we acutally bind with (but that would still be
    // correct for the shader
    binding_defs[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    OneOffDescriptorSet binding_descriptor_set(m_device, binding_defs);
    const vkt::PipelineLayout binding_pipeline_layout(*m_device, {&binding_descriptor_set.layout_});

    VkDescriptorBufferInfo storage_buffer_info = {storage_buffer.handle(), 0, sizeof(uint32_t)};
    VkDescriptorBufferInfo uniform_buffer_info = {uniform_buffer.handle(), 0, 5 * sizeof(uint32_t)};

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = binding_descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &storage_buffer_info;

    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = binding_descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;  // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[1].pBufferInfo = &uniform_buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    char const *csSource = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;
        layout(set = 0, binding = 1) uniform UniformStruct { ivec4 dummy; int val; } ubo;

        void main() {
            u_index.index = ubo.val;
        }
    )glsl";

    VkShaderObj shader_module(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo stage = vku::InitStructHelper();
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module.handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = vku::InitStructHelper();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = stage;

    VkPipeline c_pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, binding_pipeline_layout.handle(), 0, 1,
                              &binding_descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08600");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyPipeline(device(), c_pipeline, nullptr);
}

TEST_F(VkLayerTest, PipelineMaxPerStageResources) {
    TEST_DESCRIPTION("Check case where pipeline is created that exceeds maxPerStageResources");

    RETURN_IF_SKIP(InitFramework())
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Spec requires a minimum of 128 so know this is setting it lower than that
    const uint32_t maxPerStageResources = 4;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxPerStageResources = maxPerStageResources;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    RETURN_IF_SKIP(InitState())
    // Adds the one color attachment
    InitRenderTarget();

    // A case where it shouldn't error because no single stage is over limit
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings_normal = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPerStageResources, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

    // vertex test
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings_vert = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPerStageResources, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    // fragment only has it at the limit because color attachment should push it over
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings_frag = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPerStageResources, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

    // compute test
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings_comp = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPerStageResources, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    // Have case where it pushes limit from two setLayouts instead of two setLayoutBindings
    std::vector<VkDescriptorSetLayoutBinding> layout_binding_combined0 = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPerStageResources, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
    std::vector<VkDescriptorSetLayoutBinding> layout_binding_combined1 = {
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};

    const vkt::DescriptorSetLayout ds_layout_normal(*m_device, layout_bindings_normal);
    const vkt::DescriptorSetLayout ds_layout_vert(*m_device, layout_bindings_vert);
    const vkt::DescriptorSetLayout ds_layout_frag(*m_device, layout_bindings_frag);
    const vkt::DescriptorSetLayout ds_layout_comp(*m_device, layout_bindings_comp);
    const vkt::DescriptorSetLayout ds_layout_combined0(*m_device, layout_binding_combined0);
    const vkt::DescriptorSetLayout ds_layout_combined1(*m_device, layout_binding_combined1);

    CreateComputePipelineHelper compute_pipe(*this);
    compute_pipe.InitState();
    compute_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds_layout_comp});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-layout-01687");
    compute_pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    {
        CreatePipelineHelper graphics_pipe(*this);
        graphics_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds_layout_normal});
        graphics_pipe.CreateGraphicsPipeline();
    }

    {
        CreatePipelineHelper graphics_pipe(*this);
        graphics_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds_layout_vert});
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-01688");
        graphics_pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper graphics_pipe(*this);
        graphics_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds_layout_frag});
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-01688");
        graphics_pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper graphics_pipe(*this);
        graphics_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds_layout_combined0, &ds_layout_combined1});
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-01688");
        graphics_pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativePipeline, PipelineExecutablePropertiesFeature) {
    TEST_DESCRIPTION("Try making calls without pipelineExecutableInfo.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_features =
        vku::InitStructHelper();
    pipeline_exe_features.pipelineExecutableInfo = VK_FALSE;  // Starting with it off

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&pipeline_exe_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // MockICD will return 0 for the executable count
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    uint32_t count;
    VkPipelineExecutableInfoKHR pipeline_exe_info = vku::InitStructHelper();
    pipeline_exe_info.pipeline = pipe.pipeline_;
    pipeline_exe_info.executableIndex = 0;

    VkPipelineInfoKHR pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.pipeline_;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipelineExecutableInfo-03276");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03278");
    vk::GetPipelineExecutableInternalRepresentationsKHR(m_device->device(), &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutableStatisticsKHR-pipelineExecutableInfo-03272");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03274");
    vk::GetPipelineExecutableStatisticsKHR(m_device->device(), &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutablePropertiesKHR-pipelineExecutableInfo-03270");
    vk::GetPipelineExecutablePropertiesKHR(m_device->device(), &pipeline_info, &count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, SampledInvalidImageViews) {
    TEST_DESCRIPTION("Test if an VkImageView is sampled at draw/dispatch that the format has valid format features enabled");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    const VkFormat sampled_format = VK_FORMAT_R8G8B8A8_UNORM;

    // Remove format features want to test if missing
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), sampled_format, &formatProps);
    formatProps.optimalTilingFeatures = (formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), sampled_format, formatProps);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, sampled_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView imageView = image.targetView(sampled_format);

    // maps to VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    char const *fs_source_combined = R"glsl(
        #version 450
        layout (set=0, binding=0) uniform sampler2D samplerColor;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(samplerColor, gl_FragCoord.xy);
           color += texture(samplerColor, gl_FragCoord.wz);
        }
    )glsl";
    VkShaderObj fs_combined(this, fs_source_combined, VK_SHADER_STAGE_FRAGMENT_BIT);

    // maps to VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE and VK_DESCRIPTOR_TYPE_SAMPLER
    char const *fs_source_seperate = R"glsl(
        #version 450
        layout (set=0, binding=0) uniform texture2D textureColor;
        layout (set=0, binding=1) uniform sampler samplers;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(sampler2D(textureColor, samplers), gl_FragCoord.xy);
        }
    )glsl";
    VkShaderObj fs_seperate(this, fs_source_seperate, VK_SHADER_STAGE_FRAGMENT_BIT);

    // maps to an unused image sampler that should not trigger validation as it is never sampled
    char const *fs_source_unused = R"glsl(
        #version 450
        layout (set=0, binding=0) uniform sampler2D samplerColor;
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(gl_FragCoord.xyz, 1.0);
        }
    )glsl";
    VkShaderObj fs_unused(this, fs_source_unused, VK_SHADER_STAGE_FRAGMENT_BIT);

    // maps to VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER but makes sure it walks function tree to find sampling
    char const *fs_source_function = R"glsl(
        #version 450
        layout (set=0, binding=0) uniform sampler2D samplerColor;
        layout(location=0) out vec4 color;
        vec4 foo() { return texture(samplerColor, gl_FragCoord.xy); }
        vec4 bar(float x) { return (x > 0.5) ? foo() : vec4(1.0,1.0,1.0,1.0); }
        void main() {
           color = bar(gl_FragCoord.x);
        }
    )glsl";
    VkShaderObj fs_function(this, fs_source_function, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipeline_combined(*this);
    pipeline_combined.InitState();
    CreatePipelineHelper pipeline_seperate(*this);
    pipeline_seperate.InitState();
    CreatePipelineHelper pipeline_unused(*this);
    pipeline_unused.InitState();
    CreatePipelineHelper pipeline_function(*this);
    pipeline_function.InitState();

    // 4 different pipelines for 4 different shaders
    // 3 are invalid and 1 (pipeline_unused) is valid
    pipeline_combined.shader_stages_[1] = fs_combined.GetStageCreateInfo();
    pipeline_seperate.shader_stages_[1] = fs_seperate.GetStageCreateInfo();
    pipeline_unused.shader_stages_[1] = fs_unused.GetStageCreateInfo();
    pipeline_function.shader_stages_[1] = fs_function.GetStageCreateInfo();

    OneOffDescriptorSet::Bindings combined_bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    OneOffDescriptorSet::Bindings seperate_bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    OneOffDescriptorSet combined_descriptor_set(m_device, combined_bindings);
    OneOffDescriptorSet seperate_descriptor_set(m_device, seperate_bindings);
    const vkt::PipelineLayout combined_pipeline_layout(*m_device, {&combined_descriptor_set.layout_});
    const vkt::PipelineLayout seperate_pipeline_layout(*m_device, {&seperate_descriptor_set.layout_});

    pipeline_combined.gp_ci_.layout = combined_pipeline_layout.handle();
    pipeline_seperate.gp_ci_.layout = seperate_pipeline_layout.handle();
    pipeline_unused.gp_ci_.layout = combined_pipeline_layout.handle();
    pipeline_function.gp_ci_.layout = combined_pipeline_layout.handle();

    pipeline_combined.CreateGraphicsPipeline();
    pipeline_seperate.CreateGraphicsPipeline();
    pipeline_unused.CreateGraphicsPipeline();
    pipeline_function.CreateGraphicsPipeline();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.minFilter = VK_FILTER_LINEAR;  // turned off feature bit for test
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.compareEnable = VK_FALSE;
    vkt::Sampler sampler_filter(*m_device, sampler_ci);

    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;  // turned off feature bit for test
    vkt::Sampler sampler_mipmap(*m_device, sampler_ci);

    VkDescriptorImageInfo combined_sampler_info = {sampler_filter.handle(), imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo seperate_sampled_image_info = {VK_NULL_HANDLE, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo seperate_sampler_info = {sampler_filter.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

    // first item is combined, second/third item are seperate
    VkWriteDescriptorSet descriptor_writes[3] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = combined_descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &combined_sampler_info;

    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = seperate_descriptor_set.set_;
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_writes[1].pImageInfo = &seperate_sampled_image_info;

    descriptor_writes[2] = vku::InitStructHelper();
    descriptor_writes[2].dstSet = seperate_descriptor_set.set_;
    descriptor_writes[2].dstBinding = 1;
    descriptor_writes[2].descriptorCount = 1;
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_writes[2].pImageInfo = &seperate_sampler_info;

    vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, nullptr);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Unused is a valid version of the combined pipeline/descriptors
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_unused.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, combined_pipeline_layout.handle(), 0, 1,
                              &combined_descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    // Test magFilter
    {
        // Same descriptor set as combined test
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_function.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Draw with invalid combined image sampler
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_combined.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Same error, but not with seperate descriptors
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_seperate.Handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, seperate_pipeline_layout.handle(), 0,
                                  1, &seperate_descriptor_set.set_, 0, nullptr);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    // Same test but for mipmap, so need to update descriptors
    {
        combined_sampler_info.sampler = sampler_mipmap.handle();
        seperate_sampler_info.sampler = sampler_mipmap.handle();
        vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, nullptr);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, combined_pipeline_layout.handle(), 0,
                                  1, &combined_descriptor_set.set_, 0, nullptr);

        // Same descriptor set as combined test
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_function.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-mipmapMode-04770");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Draw with invalid combined image sampler
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_combined.Handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-mipmapMode-04770");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Same error, but not with seperate descriptors
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_seperate.Handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, seperate_pipeline_layout.handle(), 0,
                                  1, &seperate_descriptor_set.set_, 0, nullptr);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-mipmapMode-04770");
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativePipeline, ShaderDrawParametersNotEnabled10) {
    TEST_DESCRIPTION("Validation using DrawParameters for Vulkan 1.0 without the shaderDrawParameters feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Test requires Vulkan exactly 1.0";
    }

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                          vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742",    // Extension not enabled
                                                         "VUID-VkShaderModuleCreateInfo-pCode-08740"});  // The capability not valid
    }
}

TEST_F(NegativePipeline, ShaderDrawParametersNotEnabled11) {
    TEST_DESCRIPTION("Validation using DrawParameters for Vulkan 1.1 without the shaderDrawParameters feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }
}

TEST_F(NegativePipeline, CreateFlags) {
    TEST_DESCRIPTION("Create a graphics pipeline with invalid VkPipelineCreateFlags.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkPipelineCreateFlags flags;
    const auto set_info = [&](CreatePipelineHelper &helper) { helper.gp_ci_.flags = flags; };

    flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-00764");
    flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03372");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03373");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03374");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03375");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03376");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03377");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03577");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-04947");
}

TEST_F(NegativePipeline, CreateFlagsCompute) {
    TEST_DESCRIPTION("Create a compute pipeline with invalid VkPipelineCreateFlags.");

    RETURN_IF_SKIP(Init())

    VkPipelineCreateFlags flags;
    const auto set_info = [&](CreateComputePipelineHelper &helper) { helper.cp_ci_.flags = flags; };

    flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-shaderEnqueue-09177");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03365");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03366");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03367");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03368");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03369");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03370");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03576");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-04945");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-07367");
    flags = VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-07996");
    flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-09007");
}

TEST_F(NegativePipeline, MergePipelineCachesInvalidDst) {
    TEST_DESCRIPTION("Test mergeing pipeline caches with dst cache in src list");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper other_pipe(*this);
    other_pipe.InitState();
    other_pipe.CreateGraphicsPipeline();

    VkPipelineCache dstCache = pipe.pipeline_cache_;
    VkPipelineCache srcCaches[2] = {other_pipe.pipeline_cache_, pipe.pipeline_cache_};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMergePipelineCaches-dstCache-00770");
    vk::MergePipelineCaches(m_device->device(), dstCache, 2, srcCaches);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, CreateComputesPipelineWithBadBasePointer) {
    TEST_DESCRIPTION("Create Compute Pipeline with bad base pointer");

    RETURN_IF_SKIP(Init())

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=2, local_size_y=4) in;
        void main(){
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    vkt::Pipeline test_pipeline(*m_device, compute_create_info);

    {
        compute_create_info.basePipelineHandle = VK_NULL_HANDLE;
        compute_create_info.basePipelineIndex = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-07985");
        VkPipeline pipeline;
        vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    if (test_pipeline.initialized()) {
        compute_create_info.basePipelineHandle = test_pipeline.handle();
        compute_create_info.basePipelineIndex = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-07986");
        vkt::Pipeline pipeline(*m_device, compute_create_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativePipeline, DiscardRectangle) {
    TEST_DESCRIPTION("Create a graphics pipeline invalid VkPipelineDiscardRectangleStateCreateInfoEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rectangle_properties);

    uint32_t count = discard_rectangle_properties.maxDiscardRectangles + 1;
    std::vector<VkRect2D> discard_rectangles(count);

    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rectangle_state =
        vku::InitStructHelper();
    discard_rectangle_state.discardRectangleCount = count;
    discard_rectangle_state.pDiscardRectangles = discard_rectangles.data();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &discard_rectangle_state;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkPipelineDiscardRectangleStateCreateInfoEXT-discardRectangleCount-00582");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, ColorWriteCreateInfoEXT) {
    TEST_DESCRIPTION("Test VkPipelineColorWriteCreateInfoEXT in color blend state pNext");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPipelineColorWriteCreateInfoEXT color_write = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cb_ci_.pNext = &color_write;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-07608");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkBool32 enabled = VK_FALSE;
    color_write.attachmentCount = 1;
    color_write.pColorWriteEnables = &enabled;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-pAttachments-04801");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, ColorWriteCreateInfoEXTMaxAttachments) {
    TEST_DESCRIPTION("Test VkPipelineColorWriteCreateInfoEXT in color blend state pNext with too many attachments");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    uint32_t max_color_attachments = m_device->phy().limits_.maxColorAttachments + 1;

    std::vector<VkBool32> enables(max_color_attachments, VK_TRUE);
    VkPipelineColorWriteCreateInfoEXT color_write = vku::InitStructHelper();
    color_write.attachmentCount = max_color_attachments;
    color_write.pColorWriteEnables = enables.data();

    std::vector<VkPipelineColorBlendAttachmentState> color_blends(max_color_attachments);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cb_ci_.pNext = &color_write;
    pipe.cb_ci_.attachmentCount = max_color_attachments;
    pipe.cb_ci_.pAttachments = color_blends.data();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-06655");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-07609");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateVariableSampleLocations) {
    TEST_DESCRIPTION("Validate using VkPhysicalDeviceSampleLocationsPropertiesEXT");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(sample_locations);

    if (sample_locations.variableSampleLocations) {
        GTEST_SKIP() << "VkPhysicalDeviceSampleLocationsPropertiesEXT::variableSampleLocations is supported, skipping.";
    }

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpasses[2] = {subpass, subpass};
    VkSubpassDependency subpass_dependency = {0,
                                              1,
                                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                              VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &subpass_dependency;

    vkt::RenderPass render_pass(*m_device, rpci);
    ASSERT_TRUE(render_pass.initialized());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo framebuffer_info = vku::InitStructHelper();
    framebuffer_info.renderPass = render_pass.handle();
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &image_view;
    framebuffer_info.width = 32;
    framebuffer_info.height = 32;
    framebuffer_info.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, framebuffer_info);
    ASSERT_TRUE(framebuffer.initialized());

    VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
    vk::GetPhysicalDeviceMultisamplePropertiesEXT(gpu(), VK_SAMPLE_COUNT_1_BIT, &multisample_prop);
    const uint32_t valid_count =
        multisample_prop.maxSampleLocationGridSize.width * multisample_prop.maxSampleLocationGridSize.height;

    if (valid_count == 0) {
        printf("multisample properties are not supported.\n");
        return;
    }

    std::vector<VkSampleLocationEXT> sample_location(valid_count, {0.5, 0.5});
    VkSampleLocationsInfoEXT sample_locations_info = vku::InitStructHelper();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = multisample_prop.maxSampleLocationGridSize;
    sample_locations_info.sampleLocationsCount = valid_count;
    sample_locations_info.pSampleLocations = sample_location.data();

    VkPipelineSampleLocationsStateCreateInfoEXT sample_locations_state =
        vku::InitStructHelper();
    sample_locations_state.sampleLocationsEnable = VK_TRUE;
    sample_locations_state.sampleLocationsInfo = sample_locations_info;

    VkPipelineMultisampleStateCreateInfo multi_sample_state = vku::InitStructHelper(&sample_locations_state);
    multi_sample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multi_sample_state.sampleShadingEnable = VK_FALSE;
    multi_sample_state.minSampleShading = 1.0;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.pMultisampleState = &multi_sample_state;
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();

    VkClearValue clear_value;
    clear_value.color.float32[0] = 0.25f;
    clear_value.color.float32[1] = 0.25f;
    clear_value.color.float32[2] = 0.25f;
    clear_value.color.float32[3] = 0.0f;

    VkAttachmentSampleLocationsEXT attachment_sample_locations;
    attachment_sample_locations.attachmentIndex = 0;
    attachment_sample_locations.sampleLocationsInfo = sample_locations_info;
    VkSubpassSampleLocationsEXT subpass_sample_locations;
    subpass_sample_locations.subpassIndex = 0;
    subpass_sample_locations.sampleLocationsInfo = sample_locations_info;

    VkRenderPassSampleLocationsBeginInfoEXT render_pass_sample_locations = vku::InitStructHelper();
    render_pass_sample_locations.attachmentInitialSampleLocationsCount = 1;
    render_pass_sample_locations.pAttachmentInitialSampleLocations = &attachment_sample_locations;
    render_pass_sample_locations.postSubpassSampleLocationsCount = 1;
    render_pass_sample_locations.pPostSubpassSampleLocations = &subpass_sample_locations;

    sample_location[0].x =
        0.0f;  // Invalid, VkRenderPassSampleLocationsBeginInfoEXT wont match VkPipelineSampleLocationsStateCreateInfoEXT

    VkRenderPassBeginInfo begin_info = vku::InitStructHelper(&render_pass_sample_locations);
    begin_info.renderPass = render_pass.handle();
    begin_info.framebuffer = framebuffer.handle();
    begin_info.renderArea.extent.width = 32;
    begin_info.renderArea.extent.height = 32;
    begin_info.renderArea.offset.x = 0;
    begin_info.renderArea.offset.y = 0;
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clear_value;

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &begin_info, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-variableSampleLocations-01525");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->NextSubpass();
    sample_location[0].x = 0.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-variableSampleLocations-01525");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();

    begin_info.pNext = nullptr;  // Invalid, missing VkRenderPassSampleLocationsBeginInfoEXT
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-variableSampleLocations-01525");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->NextSubpass();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(NegativePipeline, RasterizationConservativeStateCreateInfo) {
    TEST_DESCRIPTION("Test PipelineRasterizationConservativeStateCreateInfo.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(conservative_rasterization_props);

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservative_state =
        vku::InitStructHelper();
    conservative_state.extraPrimitiveOverestimationSize = -1.0f;

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.pNext = &conservative_state;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkPipelineRasterizationConservativeStateCreateInfoEXT-extraPrimitiveOverestimationSize-01769");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    conservative_state.extraPrimitiveOverestimationSize =
        conservative_rasterization_props.maxExtraPrimitiveOverestimationSize + 0.1f;
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkPipelineRasterizationConservativeStateCreateInfoEXT-extraPrimitiveOverestimationSize-01769");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineNullRenderPass) {
    TEST_DESCRIPTION("Test for a creating a pipeline with a null renderpass but VK_KHR_dynamic_rendering is not enabled");

    RETURN_IF_SKIP(Init())

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06575");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06603");
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, RasterizationOrderAttachmentAccessWithoutFeature) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_ARM_rasterization_order_attachment_access enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM rasterization_order_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(rasterization_order_features);

    rasterization_order_features.rasterizationOrderColorAttachmentAccess = 0;
    rasterization_order_features.rasterizationOrderDepthAttachmentAccess = 0;
    rasterization_order_features.rasterizationOrderStencilAttachmentAccess = 0;

    RETURN_IF_SKIP(InitState(nullptr, &rasterization_order_features));

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    VkPipelineColorBlendAttachmentState cb_as = {};
    VkPipelineColorBlendStateCreateInfo cb_ci = vku::InitStructHelper();
    cb_ci.attachmentCount = 1;
    cb_ci.pAttachments = &cb_as;

    VkAttachmentDescription attachments[2] = {};
    attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].flags = 0;
    attachments[1].format = FindSupportedDepthStencilFormat(this->gpu());
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference cAttachRef = {};
    cAttachRef.attachment = 0;
    cAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference dsAttachRef = {};
    dsAttachRef.attachment = 1;
    dsAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &cAttachRef;
    subpass.pDepthStencilAttachment = &dsAttachRef;
    subpass.flags = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_ARM |
                    VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM |
                    VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 2;
    rpci.pAttachments = attachments;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, rpci);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pDepthStencilState = &ds_ci;
        helper.gp_ci_.pColorBlendState = &cb_ci;
        helper.gp_ci_.renderPass = render_pass.handle();
    };

    // Color attachment
    cb_ci.flags = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;
    ds_ci.flags = 0;

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineColorBlendStateCreateInfo-rasterizationOrderColorAttachmentAccess-06465");

    // Depth attachment
    cb_ci.flags = 0;
    ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderDepthAttachmentAccess-06463");

    // Stencil attachment
    cb_ci.flags = 0;
    ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderStencilAttachmentAccess-06464");
}

TEST_F(NegativePipeline, RasterizationOrderAttachmentAccessNoSubpassFlags) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_ARM_rasterization_order_attachment_access enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM rasterization_order_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(rasterization_order_features);

    if (!rasterization_order_features.rasterizationOrderColorAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderDepthAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderStencilAttachmentAccess) {
        GTEST_SKIP() << "Test requires (unsupported) rasterizationOrder*AttachmentAccess";
    }

    RETURN_IF_SKIP(InitState(nullptr, &rasterization_order_features));

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    VkPipelineColorBlendAttachmentState cb_as = {};
    VkPipelineColorBlendStateCreateInfo cb_ci = vku::InitStructHelper();
    cb_ci.attachmentCount = 1;
    cb_ci.pAttachments = &cb_as;
    VkRenderPass render_pass_handle = VK_NULL_HANDLE;

    auto create_render_pass = [&](VkPipelineDepthStencilStateCreateFlags subpass_flags, vkt::RenderPass &render_pass) {
        VkAttachmentDescription attachments[2] = {};
        attachments[0].flags = 0;
        attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments[1].flags = 0;
        attachments[1].format = FindSupportedDepthStencilFormat(this->gpu());
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cAttachRef = {};
        cAttachRef.attachment = 0;
        cAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference dsAttachRef = {};
        dsAttachRef.attachment = 1;
        dsAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &cAttachRef;
        subpass.pDepthStencilAttachment = &dsAttachRef;
        subpass.flags = subpass_flags;

        VkRenderPassCreateInfo rpci = vku::InitStructHelper();
        rpci.attachmentCount = 2;
        rpci.pAttachments = attachments;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;

        render_pass.init(*this->m_device, rpci);
    };

    auto set_flgas_pipeline_createinfo = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pDepthStencilState = &ds_ci;
        helper.gp_ci_.pColorBlendState = &cb_ci;
        helper.gp_ci_.renderPass = render_pass_handle;
    };

    vkt::RenderPass render_pass_no_flags;
    create_render_pass(0, render_pass_no_flags);
    render_pass_handle = render_pass_no_flags.handle();

    // Color attachment
    if (rasterization_order_features.rasterizationOrderColorAttachmentAccess) {
        cb_ci.flags = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;
        ds_ci.flags = 0;

        // Expecting VUID-VkGraphicsPipelineCreateInfo-flags-06484 Error
        CreatePipelineHelper::OneshotTest(*this, set_flgas_pipeline_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06484");
    }

    // Depth attachment
    if (rasterization_order_features.rasterizationOrderDepthAttachmentAccess) {
        cb_ci.flags = 0;
        ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;

        // Expecting VUID-VkGraphicsPipelineCreateInfo-flags-06485 Error
        CreatePipelineHelper::OneshotTest(*this, set_flgas_pipeline_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06485");
    }

    // Stencil attachment
    if (rasterization_order_features.rasterizationOrderStencilAttachmentAccess) {
        cb_ci.flags = 0;
        ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;

        // Expecting VUID-VkGraphicsPipelineCreateInfo-flags-06486 Error
        CreatePipelineHelper::OneshotTest(*this, set_flgas_pipeline_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06486");
    }

    if (rasterization_order_features.rasterizationOrderDepthAttachmentAccess) {
        char const *fsSource = R"glsl(
            #version 450
            layout(early_fragment_tests) in;
            layout(location = 0) out vec4 uFragColor;
            void main() {
                uFragColor = vec4(0,1,0,1);
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        auto set_stages_pipeline_createinfo = [&](CreatePipelineHelper &helper) {
            helper.gp_ci_.pDepthStencilState = &ds_ci;
            helper.gp_ci_.pColorBlendState = &cb_ci;
            helper.gp_ci_.renderPass = render_pass_handle;
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        cb_ci.flags = 0;
        ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;
        vkt::RenderPass render_pass;
        create_render_pass(VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM, render_pass);
        render_pass_handle = render_pass.handle();
        CreatePipelineHelper::OneshotTest(*this, set_stages_pipeline_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06591");
    }
}

TEST_F(NegativePipeline, MismatchedRenderPassAndPipelineAttachments) {
    TEST_DESCRIPTION("Test creating a pipeline with no attachments with a render pass with attachments.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-07609");

    char const *vsSource = R"glsl(
                #version 450

                void main() {
                }
            )glsl";

    char const *fsSource = R"glsl(
                #version 450

                void main() {
                }
            )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 1;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const vkt::DescriptorSetLayout descriptor_set_layout(*m_device, {layout_binding});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout});
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.cb_ci_ = vku::InitStructHelper();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, IncompatibleScissorCountAndViewportCount) {
    TEST_DESCRIPTION("Validate creating a pipeline with incompatible scissor and viewport count, without dynamic states.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceFeatures features{};
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    if (features.multiViewport == false) {
        GTEST_SKIP() << "multiViewport feature not supported by device";
    }

    VkViewport viewports[2] = {{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}};

    auto set_viewport_state_createinfo = [&](CreatePipelineHelper &helper) {
        helper.vp_state_ci_.viewportCount = 2;
        helper.vp_state_ci_.pViewports = viewports;
    };

    CreatePipelineHelper::OneshotTest(*this, set_viewport_state_createinfo, kErrorBit,
                                      "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134");
}

TEST_F(NegativePipeline, ShaderTileImageDisabled) {
    TEST_DESCRIPTION("Validate creating graphics pipeline without shader tile image features enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
    dynamic_rendering_features.pNext = &shader_tile_image_features;
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    // None of the shader tile image read features supported skip the test.
    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    shader_tile_image_features.shaderTileImageColorReadAccess = false;
    shader_tile_image_features.shaderTileImageDepthReadAccess = false;
    shader_tile_image_features.shaderTileImageStencilReadAccess = false;

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    if (shader_tile_image_features.shaderTileImageDepthReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageDepthReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto pipeline_createinfo = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo, kErrorBit,
                                          "VUID-RuntimeSpirv-shaderTileImageDepthReadAccess-08729");
    }

    if (shader_tile_image_features.shaderTileImageStencilReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto pipeline_createinfo = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo, kErrorBit,
                                          "VUID-RuntimeSpirv-shaderTileImageStencilReadAccess-08730");
    }

    if (shader_tile_image_features.shaderTileImageColorReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageColorReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto pipeline_createinfo_with_ms = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo_with_ms, kErrorBit,
                                          "VUID-RuntimeSpirv-shaderTileImageColorReadAccess-08728");
    }
}

TEST_F(NegativePipeline, ShaderTileImage) {
    TEST_DESCRIPTION("Validate creating graphics pipeline with shader tile image extension.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
    dynamic_rendering_features.pNext = &shader_tile_image_features;
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    // None of the shader tile image read features supported skip the test.
    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();

    if (shader_tile_image_features.shaderTileImageDepthReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageDepthReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto pipeline_createinfo = [&](CreatePipelineHelper &helper) {
            ds_ci.depthWriteEnable = true;

            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.pDepthStencilState = &ds_ci;
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-08711");
    }

    if (shader_tile_image_features.shaderTileImageStencilReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkStencilOpState stencil_state = {};
        stencil_state.failOp = VK_STENCIL_OP_KEEP;
        stencil_state.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil_state.passOp = VK_STENCIL_OP_REPLACE;
        stencil_state.compareOp = VK_COMPARE_OP_LESS;
        stencil_state.compareMask = 0xff;
        stencil_state.writeMask = 0xff;
        stencil_state.reference = 0xf;

        ds_ci = {};
        ds_ci.front = stencil_state;
        ds_ci.back = stencil_state;

        auto pipeline_createinfo = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.pDepthStencilState = &ds_ci;
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-08712");
    }

    if (shader_tile_image_features.shaderTileImageColorReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageColorReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkAttachmentReference attachmentRefs[1] = {};
        attachmentRefs[0].layout = VK_IMAGE_LAYOUT_GENERAL;
        attachmentRefs[0].attachment = 0;

        VkSubpassDescription subpass = {};
        subpass.flags = 0;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRefs[0];

        VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
        rp_ci.subpassCount = 1;
        rp_ci.pSubpasses = &subpass;
        rp_ci.attachmentCount = 1;
        VkAttachmentDescription attach_desc[1] = {};
        attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
        attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        rp_ci.pAttachments = attach_desc;

        vkt::RenderPass render_pass(*m_device, rp_ci);

        // Check if the colorAttachmentRead capability enable, renderpass should be null
        auto pipeline_createinfo = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.renderPass = render_pass.handle();
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-renderPass-08710");

        // sampleShading enable and minSampleShading not equal to 1.0
        VkPipelineMultisampleStateCreateInfo ms_ci = vku::InitStructHelper();
        ms_ci.sampleShadingEnable = VK_TRUE;
        ms_ci.minSampleShading = 0;
        ms_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        auto pipeline_createinfo_with_ms = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            helper.gp_ci_.pMultisampleState = &ms_ci;
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };

        CreatePipelineHelper::OneshotTest(*this, pipeline_createinfo_with_ms, kErrorBit,
                                          "VUID-RuntimeSpirv-minSampleShading-08732");
    }
}

TEST_F(NegativePipeline, PipelineSubpassOutOfBounds) {
    TEST_DESCRIPTION("Create pipeline with subpass index larger than number of subpasses in render pass");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.subpass = 4u;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06046");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, PipelineRenderingInfoInvalidFormats) {
    TEST_DESCRIPTION("Create pipeline with invalid pipeline rendering formats");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    RETURN_IF_SKIP(InitState(nullptr, &dynamic_rendering_features));

    VkPipelineRenderingCreateInfo pipeline_rendering_ci = vku::InitStructHelper();
    pipeline_rendering_ci.depthAttachmentFormat = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pNext = &pipeline_rendering_ci;
    pipe.cb_ci_.attachmentCount = 0u;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06583");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06587");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipeline_rendering_ci.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_ci.stencilAttachmentFormat = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06584");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06588");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, RasterStateWithDepthBiasRepresentationInfo) {
    TEST_DESCRIPTION(
        "VkDepthBiasRepresentationInfoEXT in VkPipelineRasterizationStateCreateInfo pNext chain, but with "
        "VkPhysicalDeviceDepthBiasControlFeaturesEXT features disabled");

    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);

    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }

    features2.features.depthBiasClamp =
        VK_FALSE;  // Make sure validation of VkDepthBiasRepresentationInfoEXT in VkPipelineRasterizationStateCreateInfo does not
                   // rely on depthBiasClamp being enabled
    depth_bias_control_features.leastRepresentableValueForceUnormRepresentation = VK_FALSE;
    depth_bias_control_features.floatRepresentation = VK_FALSE;
    depth_bias_control_features.depthBiasExact = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    const auto create_pipe_with_depth_bias_representation = [this](VkDepthBiasRepresentationInfoEXT &depth_bias_representation) {
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
        const VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper(&depth_bias_representation);
        pipe.rs_state_ci_ = raster_state;
        pipe.CreateGraphicsPipeline();
    };

    VkDepthBiasRepresentationInfoEXT depth_bias_representation = vku::InitStructHelper();
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT;
    depth_bias_representation.depthBiasExact = VK_TRUE;
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDepthBiasRepresentationInfoEXT-leastRepresentableValueForceUnormRepresentation-08947");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDepthBiasRepresentationInfoEXT-depthBiasExact-08949");
    create_pipe_with_depth_bias_representation(depth_bias_representation);
    m_errorMonitor->VerifyFound();

    depth_bias_representation.depthBiasExact = VK_FALSE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDepthBiasRepresentationInfoEXT-floatRepresentation-08948");
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT;
    create_pipe_with_depth_bias_representation(depth_bias_representation);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, InvalidPipelineDepthBias) {
    TEST_DESCRIPTION("Create pipeline with invalid depth bias");

    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceFeatures features = {};
    RETURN_IF_SKIP(InitState(&features));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.rs_state_ci_.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_.depthBiasClamp = 0.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00754");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, MismatchedRasterizationSamples) {
    TEST_DESCRIPTION("Draw when render pass rasterization samples do not match pipeline rasterization samples");

    AddRequiredExtensions(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT msrtss_features = vku::InitStructHelper();
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper(&msrtss_features);
    GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (!msrtss_features.multisampledRenderToSingleSampled) {
        GTEST_SKIP() << "Test requires (unsupported) multisampledRenderToSingleSampled";
    }
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    RETURN_IF_SKIP(InitState(nullptr, &dynamic_rendering_features));

    VkImageObj image(m_device);
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {128u, 128u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageFormatProperties formProps;
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), image_ci.format, image_ci.imageType,
                                                              image_ci.tiling, image_ci.usage, image_ci.flags, &formProps);
    if (res != VK_SUCCESS || (formProps.sampleCounts & VK_SAMPLE_COUNT_2_BIT) == 0) {
        GTEST_SKIP() << "Required format not supported";
    }

    image.init(&image_ci);

    VkImageView image_view = image.targetView(image_ci.format);

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkMultisampledRenderToSingleSampledInfoEXT rtss = vku::InitStructHelper();
    rtss.multisampledRenderToSingleSampledEnable = VK_TRUE;
    rtss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    VkRenderingInfo rendering_info = vku::InitStructHelper(&rtss);
    rendering_info.renderArea = {{0, 0}, {1, 1}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pColorAttachments = &color_attachment;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &image_ci.format;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pNext-07935");
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativePipeline, PipelineMissingFeatures) {
    TEST_DESCRIPTION("Enabled depth bounds when the features is disabled");

    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceFeatures features = {};
    RETURN_IF_SKIP(InitState(&features));

    VkAttachmentDescription attachment = {};
    attachment.format = FindSupportedDepthStencilFormat(m_device->phy().handle());
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference dr = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription sp = {};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.pDepthStencilAttachment = &dr;

    VkRenderPassCreateInfo rpi = vku::InitStructHelper();
    rpi.attachmentCount = 1;
    rpi.pAttachments = &attachment;
    rpi.subpassCount = 1;
    rpi.pSubpasses = &sp;
    vkt::RenderPass rp(*m_device, rpi);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthBoundsTestEnable = VK_TRUE;
    pipe.gp_ci_.renderPass = rp.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDepthStencilStateCreateInfo-depthBoundsTestEnable-00598");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.ds_ci_.depthBoundsTestEnable = VK_FALSE;
    pipe.pipe_ms_state_ci_.alphaToOneEnable = VK_TRUE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineMultisampleStateCreateInfo-alphaToOneEnable-00785");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.pipe_ms_state_ci_.alphaToOneEnable = VK_FALSE;
    pipe.rs_state_ci_.depthClampEnable = VK_TRUE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRasterizationStateCreateInfo-depthClampEnable-00782");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipeline, MissingPipelineFormat) {
    TEST_DESCRIPTION("Render with required pipeline formats VK_FORMAT_UNDEFINED");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT unusued_attachment_features =
        vku::InitStructHelper(&dynamic_rendering_features);
    GetPhysicalDeviceFeatures2(unusued_attachment_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "dynamicRendering not supported";
    }
    if (!unusued_attachment_features.dynamicRenderingUnusedAttachments) {
        GTEST_SKIP() << "dynamicRenderingUnusedAttachments not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &unusued_attachment_features));

    VkFormat undefined = VK_FORMAT_UNDEFINED;
    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());

    VkImageObj color_image(m_device);
    color_image.Init(32, 32, 1, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView color_image_view = color_image.targetView(color_format);

    VkImageObj ds_image(m_device);
    ds_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView ds_image_view = ds_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1u;
    pipeline_rendering_info.pColorAttachmentFormats = &undefined;
    pipeline_rendering_info.depthAttachmentFormat = ds_format;
    pipeline_rendering_info.stencilAttachmentFormat = ds_format;

    VkClearValue color_clear_value;
    color_clear_value.color.float32[0] = 0.0f;
    color_clear_value.color.float32[1] = 0.0f;
    color_clear_value.color.float32[2] = 0.0f;
    color_clear_value.color.float32[3] = 0.0f;

    VkClearValue ds_clear_value;
    ds_clear_value.depthStencil = {1.0f, 0u};

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = color_clear_value;

    VkRenderingAttachmentInfo ds_attachment = vku::InitStructHelper();
    ds_attachment.imageView = ds_image_view;
    ds_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    ds_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ds_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ds_attachment.clearValue = ds_clear_value;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32u, 32u}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = &ds_attachment;
    rendering_info.pStencilAttachment = &ds_attachment;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    ds_state.depthTestEnable = VK_TRUE;
    ds_state.depthWriteEnable = VK_TRUE;
    ds_state.stencilTestEnable = VK_TRUE;

    VkPipelineColorBlendStateCreateInfo color_blend_state = vku::InitStructHelper();
    color_blend_state.attachmentCount = 1u;
    color_blend_state.pAttachments = &color_blend_attachment_state;

    CreatePipelineHelper color_pipe(*this);
    color_pipe.InitState();
    color_pipe.gp_ci_.pNext = &pipeline_rendering_info;
    color_pipe.ds_ci_ = ds_state;
    color_pipe.cb_ci_ = color_blend_state;
    color_pipe.CreateGraphicsPipeline();

    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = undefined;

    CreatePipelineHelper depth_pipe(*this);
    depth_pipe.InitState();
    depth_pipe.gp_ci_.pNext = &pipeline_rendering_info;
    depth_pipe.ds_ci_ = ds_state;
    color_pipe.cb_ci_ = color_blend_state;
    depth_pipe.CreateGraphicsPipeline();

    pipeline_rendering_info.depthAttachmentFormat = ds_format;
    pipeline_rendering_info.stencilAttachmentFormat = undefined;

    CreatePipelineHelper stencil_pipe(*this);
    stencil_pipe.InitState();
    stencil_pipe.gp_ci_.pNext = &pipeline_rendering_info;
    stencil_pipe.ds_ci_ = ds_state;
    color_pipe.cb_ci_ = color_blend_state;
    stencil_pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, color_pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pColorAttachments-08963");
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-08964");
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-08965");
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}
