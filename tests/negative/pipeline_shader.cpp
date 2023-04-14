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
#include "error_message/validation_error_enums.h"

#include <limits>

TEST_F(VkLayerTest, PSOPolygonModeInvalid) {
    TEST_DESCRIPTION("Attempt to use invalid polygon fill modes.");
    VkPhysicalDeviceFeatures device_features = {};
    device_features.fillModeNonSolid = VK_FALSE;
    // The sacrificial device object
    ASSERT_NO_FATAL_FAILURE(Init(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineRasterizationStateCreateInfo rs_ci = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    auto set_polygonMode = [&](CreatePipelineHelper &helper) { helper.rs_state_ci_ = rs_ci; };

    // Set polygonMode to POINT while the non-solid fill mode feature is disabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_POINT;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01413");

    // Set polygonMode to LINE while the non-solid fill mode feature is disabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_LINE;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01413");

    // Set polygonMode to FILL_RECTANGLE_NV while the extension is not enabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_FILL_RECTANGLE_NV;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414");
}

TEST_F(VkLayerTest, PipelineNotBound) {
    TEST_DESCRIPTION("Pass in an invalid pipeline object handle into a Vulkan API call.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipeline badPipeline = CastToHandle<VkPipeline, uintptr_t>(0xbaadb1be);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, badPipeline);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineWrongBindPointGraphics) {
    TEST_DESCRIPTION("Bind a compute pipeline in the graphics bind point");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineBindPoint-00779");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineBasicCompute) {
    TEST_DESCRIPTION("Bind a compute pipeline (no subpasses)");
    ASSERT_NO_FATAL_FAILURE(Init());

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform block { vec4 x; };
    void main(){
        vec4 v = 2.0 * x;
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkBufferObj buffer;
    auto bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 1024;
    buffer.init(*m_device, bci);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
}

TEST_F(VkLayerTest, PipelineWrongBindPointCompute) {
    TEST_DESCRIPTION("Bind a graphics pipeline in the compute bind point");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineBindPoint-00780");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DisabledIndependentBlend) {
    TEST_DESCRIPTION(
        "Generate INDEPENDENT_BLEND by disabling independent blend and then specifying different blend states for two "
        "attachments");
    VkPhysicalDeviceFeatures features = {};
    features.independentBlend = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkPipelineObj pipeline(m_device);
    // Create a renderPass with two color attachments
    VkAttachmentReference attachments[2] = {};
    attachments[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    attachments[1].attachment = 1;
    attachments[1].layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = attachments;
    subpass.colorAttachmentCount = 2;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
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

    vk_testing::RenderPass renderpass(*m_device, rpci);
    ASSERT_TRUE(renderpass.initialized());

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline.AddShader(&vs);
    pipeline.AddShader(&fs);

    VkPipelineColorBlendAttachmentState att_state1 = {}, att_state2 = {};
    att_state1.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state1.blendEnable = VK_TRUE;
    att_state2.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state2.blendEnable = VK_FALSE;
    pipeline.AddColorAttachment(0, att_state1);
    pipeline.AddColorAttachment(1, att_state2);
    pipeline.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline.MakeDynamic(VK_DYNAMIC_STATE_SCISSOR);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-00605");
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BlendingOnFormatWithoutBlendingSupport) {
    TEST_DESCRIPTION("Test that blending is not enabled with a format not support blending");
    VkPhysicalDeviceFeatures features = {};
    features.independentBlend = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));

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

    VkPipelineObj pipeline(m_device);
    // Create a renderPass with two color attachments
    VkAttachmentReference attachment = {};
    attachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attachment;
    subpass.colorAttachmentCount = 1;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = non_blending_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci);
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline.AddShader(&vs);
    pipeline.AddShader(&fs);

    VkPipelineColorBlendAttachmentState att_state = {};
    att_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state.blendEnable = VK_TRUE;
    pipeline.AddColorAttachment(0, att_state);
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), rp.handle());

    m_errorMonitor->VerifyFound();
}

// Is the Pipeline compatible with the expectations of the Renderpass/subpasses?
TEST_F(VkLayerTest, PipelineRenderpassCompatibility) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline that is incompatible with the requirements of its contained Renderpass/subpasses.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineColorBlendAttachmentState att_state1 = {};
    att_state1.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state1.blendEnable = VK_TRUE;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.cb_attachments_[0] = att_state1;
        helper.gp_ci_.pColorBlendState = nullptr;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-renderPass-06044");
}

TEST_F(VkLayerTest, PointSizeFailure) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize but not writing to it
    const char NoPointSizeVertShader[] = R"glsl(
        #version 450
        vec2 vertices[3];
        out gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
        };
        void main() {
            vertices[0] = vec2(-1.0, -1.0);
            vertices[1] = vec2( 1.0, -1.0);
            vertices[2] = vec2( 0.0,  1.0);
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, NoPointSizeVertShader, VK_SHADER_STAGE_VERTEX_BIT);

    // Set Input Assembly to TOPOLOGY POINT LIST
    auto set_info = [&](CreatePipelineHelper &helper) {
        // Set Input Assembly to TOPOLOGY POINT LIST
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-Vertex-07722");
}

TEST_F(VkLayerTest, InvalidTopology) {
    TEST_DESCRIPTION("InvalidTopology.");
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.geometryShader = VK_FALSE;
    deviceFeatures.tessellationShader = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(Init(&deviceFeatures));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    VkPrimitiveTopology topology;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = topology;
        helper.ia_ci_.primitiveRestartEnable = VK_TRUE;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428");

    topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428");

    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428");

    {
        topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                      "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    {
        topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                      "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    {
        topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                      "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430",
                                      "VUID-VkGraphicsPipelineCreateInfo-topology-00737"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429");

    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429");
}

TEST_F(VkLayerTest, PrimitiveTopologyListRestart) {
    TEST_DESCRIPTION("Test VK_EXT_primitive_topology_list_restart");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ptl_restart_features = LvlInitStruct<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>();
    GetPhysicalDeviceFeatures2(ptl_restart_features);

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required, skipping test.";
    }
    if (!ptl_restart_features.primitiveTopologyListRestart) {
        GTEST_SKIP() << "primitive topology list restart feature is not available, skipping test";
    }
    ptl_restart_features.primitiveTopologyListRestart = false;
    ptl_restart_features.primitiveTopologyPatchListRestart = false;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ptl_restart_features));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    VkPrimitiveTopology topology;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = topology;
        helper.ia_ci_.primitiveRestartEnable = VK_TRUE;
        helper.shader_stages_ = { vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo() };
    };

    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252");

    if (m_device->phy().features().tessellationShader) {
        topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06253",
                                      "VUID-VkGraphicsPipelineCreateInfo-topology-00737"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }
}

TEST_F(VkLayerTest, CreatePipelineLayoutExceedsSetLimit) {
    TEST_DESCRIPTION("Attempt to create a pipeline layout using more than the physical limit of SetLayouts.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &layout_binding;
    vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    // Create an array of DSLs, one larger than the physical limit
    const auto excess_layouts = 1 + m_device->phy().properties().limits.maxBoundDescriptorSets;
    std::vector<VkDescriptorSetLayout> dsl_array(excess_layouts, ds_layout.handle());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = excess_layouts;
    pipeline_layout_ci.pSetLayouts = dsl_array.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-setLayoutCount-00286");
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineExcessSubsampledPerStageDescriptors) {
    TEST_DESCRIPTION("Attempt to create a pipeline layout where total subsampled descriptors exceed limits");

    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto density_map2_properties = LvlInitStruct<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>();
    auto properties2 = GetPhysicalDeviceProperties2(density_map2_properties);

    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t max_subsampled_samplers = density_map2_properties.maxDescriptorSetSubsampledSamplers;

    // Note: Adding this check in case mock ICDs don't initialize min-max values correctly
    if (max_subsampled_samplers == 0) {
        GTEST_SKIP() << "axDescriptorSetSubsampledSamplers limit (" << max_subsampled_samplers
                     << ") must be greater than 0. Skipping.";
    }

    if (max_subsampled_samplers >= properties2.properties.limits.maxDescriptorSetSamplers) {
        GTEST_SKIP() << "test assumes maxDescriptorSetSubsampledSamplers limit (" << max_subsampled_samplers
                     << ") is less than overall sampler limit (" << properties2.properties.limits.maxDescriptorSetSamplers
                     << "). Skipping.";
    }

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.maxLod = 0.f;
    sampler_info.flags |= VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    vk_testing::Sampler sampler(*m_device, sampler_info);
    ASSERT_TRUE(sampler.initialized());

    // just make all the immutable samplers point to the same sampler
    std::vector<VkSampler> immutableSamplers;
    immutableSamplers.resize(max_subsampled_samplers);
    for (uint32_t sampler_idx = 0; sampler_idx < max_subsampled_samplers; sampler_idx++) {
        immutableSamplers[sampler_idx] = sampler.handle();
    }

    // VU 03566 - too many subsampled sampler type descriptors across stages
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dslb.descriptorCount = max_subsampled_samplers;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb.pImmutableSamplers = &immutableSamplers[0];
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dslb.descriptorCount = max_subsampled_samplers;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();
    const char *max_sampler_vuid = "VUID-VkPipelineLayoutCreateInfo-pImmutableSamplers-03566";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_sampler_vuid);
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, {&ds_layout});
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineLayoutExcessPerStageDescriptors) {
    TEST_DESCRIPTION("Attempt to create a pipeline layout where total descriptors exceed per-stage limits");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    bool descriptor_indexing =
        IsExtensionsEnabled(VK_KHR_MAINTENANCE_3_EXTENSION_NAME) && IsExtensionsEnabled(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    uint32_t max_uniform_buffers = m_device->phy().properties().limits.maxPerStageDescriptorUniformBuffers;
    uint32_t max_storage_buffers = m_device->phy().properties().limits.maxPerStageDescriptorStorageBuffers;
    uint32_t max_sampled_images = m_device->phy().properties().limits.maxPerStageDescriptorSampledImages;
    uint32_t max_storage_images = m_device->phy().properties().limits.maxPerStageDescriptorStorageImages;
    uint32_t max_samplers = m_device->phy().properties().limits.maxPerStageDescriptorSamplers;
    uint32_t max_combined = std::min(max_samplers, max_sampled_images);
    uint32_t max_input_attachments = m_device->phy().properties().limits.maxPerStageDescriptorInputAttachments;

    uint32_t sum_dyn_uniform_buffers = m_device->phy().properties().limits.maxDescriptorSetUniformBuffersDynamic;
    uint32_t sum_uniform_buffers = m_device->phy().properties().limits.maxDescriptorSetUniformBuffers;
    uint32_t sum_dyn_storage_buffers = m_device->phy().properties().limits.maxDescriptorSetStorageBuffersDynamic;
    uint32_t sum_storage_buffers = m_device->phy().properties().limits.maxDescriptorSetStorageBuffers;
    uint32_t sum_sampled_images = m_device->phy().properties().limits.maxDescriptorSetSampledImages;
    uint32_t sum_storage_images = m_device->phy().properties().limits.maxDescriptorSetStorageImages;
    uint32_t sum_samplers = m_device->phy().properties().limits.maxDescriptorSetSamplers;
    uint32_t sum_input_attachments = m_device->phy().properties().limits.maxDescriptorSetInputAttachments;

    VkPhysicalDeviceDescriptorIndexingProperties descriptor_indexing_properties =
        LvlInitStruct<VkPhysicalDeviceDescriptorIndexingProperties>();
    if (descriptor_indexing) {
        GetPhysicalDeviceProperties2(descriptor_indexing_properties);
    }

    // Devices that report UINT32_MAX for any of these limits can't run this test
    if (vvl::kU32Max ==
        std::max({max_uniform_buffers, max_storage_buffers, max_sampled_images, max_storage_images, max_samplers})) {
        GTEST_SKIP() << "Physical device limits report as UINT32_MAX";
    }

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    // VU 0fe0023e - too many sampler type descriptors in fragment stage
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dslb.descriptorCount = max_samplers;
    dslb.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb.descriptorCount = max_combined;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    VkResult err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_sampler_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03016"
                                                         : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00287";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_sampler_vuid);
    if ((max_samplers + max_combined) > sum_samplers) {
        const char *max_all_sampler_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03028"
                                                                 : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01677";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_sampler_vuid);  // expect all-stages sum too
    }
    if (max_combined > sum_sampled_images) {
        const char *max_all_sampled_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03033"
                                                                       : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01682";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_sampled_image_vuid);  // expect all-stages sum too
    }
    if (descriptor_indexing) {
        if ((max_samplers + max_combined) > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036");
        }
        if ((max_samplers + max_combined) > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03022");
        }
        if (max_combined > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041");
        }
        if (max_combined > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03025");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00240 - too many uniform buffer type descriptors in vertex stage
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb.descriptorCount = max_uniform_buffers + 1;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_uniform_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03017"
                                                         : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00288";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_uniform_vuid);
    if (dslb.descriptorCount > sum_uniform_buffers) {
        const char *max_all_uniform_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03029"
                                                                 : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01678";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_uniform_vuid);  // expect all-stages sum too
    }
    if (dslb.descriptorCount > sum_dyn_uniform_buffers) {
        const char *max_all_uniform_dynamic_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03030"
                                                                         : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01679";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_uniform_dynamic_vuid);  // expect all-stages sum too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03038");
        }
        if ((dslb.descriptorCount * 2) > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03023");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindUniformBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03037");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00242 - too many storage buffer type descriptors in compute stage
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dslb.descriptorCount = max_storage_buffers + 1;
    dslb.stageFlags = VK_SHADER_STAGE_ALL;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    dslb_vec.push_back(dslb);
    dslb.binding = 2;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dslb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_storage_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03018"
                                                         : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00289";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_storage_vuid);
    if (dslb.descriptorCount > sum_dyn_storage_buffers) {
        const char *max_all_storage_dynamic_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03032"
                                                                         : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01681";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_storage_dynamic_vuid);  // expect all-stages sum too
    }
    const uint32_t storage_buffer_count = dslb_vec[0].descriptorCount + dslb_vec[2].descriptorCount;
    if (storage_buffer_count > sum_storage_buffers) {
        const char *max_all_storage_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03031"
                                                                 : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01680";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_storage_vuid);  // expect all-stages sum too
    }
    if (descriptor_indexing) {
        if (storage_buffer_count > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindStorageBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03039");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03040");
        }
        if ((dslb.descriptorCount * 3) > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03024");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00244 - too many sampled image type descriptors in multiple stages
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dslb.descriptorCount = max_sampled_images;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dslb.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    dslb_vec.push_back(dslb);
    dslb.binding = 2;
    dslb.descriptorCount = max_combined;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_sample_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03019"
                                                              : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00290";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_sample_image_vuid);
    const uint32_t sampled_image_count = max_combined + 2 * max_sampled_images;
    if (sampled_image_count > sum_sampled_images) {
        const char *max_all_sampled_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03033"
                                                                       : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01682";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_sampled_image_vuid);  // expect all-stages sum too
    }
    if (max_combined > sum_samplers) {
        const char *max_all_sampler_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03028"
                                                                 : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01677";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_sampler_vuid);  // expect all-stages sum too
    }
    if (descriptor_indexing) {
        if (sampled_image_count > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041");
        }
        if (sampled_image_count > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03025");
        }
        if (max_combined > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036");
        }
        if (max_combined > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03022");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00246 - too many storage image type descriptors in fragment stage
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dslb.descriptorCount = 1 + (max_storage_images / 2);
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_storage_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03020"
                                                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00291";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_storage_image_vuid);
    const uint32_t storage_image_count = 2 * dslb.descriptorCount;
    if (storage_image_count > sum_storage_images) {
        const char *max_all_storage_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03034"
                                                                       : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01683";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_storage_image_vuid);  // expect all-stages sum too
    }
    if (descriptor_indexing) {
        if (storage_image_count > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindStorageImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03042");
        }
        if (storage_image_count > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindStorageImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03026");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d18 - too many input attachments in fragment stage
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dslb.descriptorCount = 1 + max_input_attachments;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_input_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03021"
                                                       : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01676";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_input_vuid);
    if (dslb.descriptorCount > sum_input_attachments) {
        const char *max_all_input_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03035"
                                                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01684";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_input_vuid);  // expect all-stages sum too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindInputAttachments) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03043");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindInputAttachments) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03027");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);
}

TEST_F(VkLayerTest, CreatePipelineLayoutExcessDescriptorsOverall) {
    TEST_DESCRIPTION("Attempt to create a pipeline layout where total descriptors exceed limits");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    const bool descriptor_indexing = IsExtensionsEnabled(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    uint32_t max_uniform_buffers = m_device->phy().properties().limits.maxPerStageDescriptorUniformBuffers;
    uint32_t max_storage_buffers = m_device->phy().properties().limits.maxPerStageDescriptorStorageBuffers;
    uint32_t max_sampled_images = m_device->phy().properties().limits.maxPerStageDescriptorSampledImages;
    uint32_t max_storage_images = m_device->phy().properties().limits.maxPerStageDescriptorStorageImages;
    uint32_t max_samplers = m_device->phy().properties().limits.maxPerStageDescriptorSamplers;
    uint32_t max_input_attachments = m_device->phy().properties().limits.maxPerStageDescriptorInputAttachments;

    uint32_t sum_dyn_uniform_buffers = m_device->phy().properties().limits.maxDescriptorSetUniformBuffersDynamic;
    uint32_t sum_uniform_buffers = m_device->phy().properties().limits.maxDescriptorSetUniformBuffers;
    uint32_t sum_dyn_storage_buffers = m_device->phy().properties().limits.maxDescriptorSetStorageBuffersDynamic;
    uint32_t sum_storage_buffers = m_device->phy().properties().limits.maxDescriptorSetStorageBuffers;
    uint32_t sum_sampled_images = m_device->phy().properties().limits.maxDescriptorSetSampledImages;
    uint32_t sum_storage_images = m_device->phy().properties().limits.maxDescriptorSetStorageImages;
    uint32_t sum_samplers = m_device->phy().properties().limits.maxDescriptorSetSamplers;
    uint32_t sum_input_attachments = m_device->phy().properties().limits.maxDescriptorSetInputAttachments;

    VkPhysicalDeviceDescriptorIndexingProperties descriptor_indexing_properties =
        LvlInitStruct<VkPhysicalDeviceDescriptorIndexingProperties>();
    if (descriptor_indexing) {
        GetPhysicalDeviceProperties2(descriptor_indexing_properties);
    }

    // Devices that report UINT32_MAX for any of these limits can't run this test
    if (vvl::kU32Max == std::max({sum_dyn_uniform_buffers, sum_uniform_buffers, sum_dyn_storage_buffers, sum_storage_buffers,
                                  sum_sampled_images, sum_storage_images, sum_samplers, sum_input_attachments})) {
        GTEST_SKIP() << "Physical device limits report as 2^32-1";
    }

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    // VU 0fe00d1a - too many sampler type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dslb.descriptorCount = sum_samplers / 2;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb.descriptorCount = sum_samplers - dslb.descriptorCount + 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    VkResult err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_sampler_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03028"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01677";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_sampler_vuid);
    if (dslb.descriptorCount > max_samplers) {
        const char *max_sampler_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03016"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00287";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_sampler_vuid);  // Expect max-per-stage samplers exceeds limits
    }
    if (dslb.descriptorCount > sum_sampled_images) {
        const char *max_all_sampled_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03033"
                                                                       : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01682";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             max_all_sampled_image_vuid);  // Expect max overall sampled image count exceeds limits
    }
    if (dslb.descriptorCount > max_sampled_images) {
        const char *max_sample_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03019"
                                                                  : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00290";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             max_sample_image_vuid);  // Expect max per-stage sampled image count exceeds limits
    }
    if (descriptor_indexing) {
        if ((sum_samplers + 1) > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036");
        }
        if (std::max(dslb_vec[0].descriptorCount, dslb_vec[1].descriptorCount) >
            descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03022");
        }
        if (dslb_vec[1].descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041");
        }
        if (dslb_vec[1].descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03025");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d1c - too many uniform buffer type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb.descriptorCount = sum_uniform_buffers + 1;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_uniform_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03029"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01678";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_uniform_vuid);
    if (dslb.descriptorCount > max_uniform_buffers) {
        const char *max_uniform_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03017"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00288";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_uniform_vuid);  // expect max-per-stage too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindUniformBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03037");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03023");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d1e - too many dynamic uniform buffer type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb.descriptorCount = sum_dyn_uniform_buffers + 1;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_uniform_dynamic_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03030"
                                                                     : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01679";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_uniform_dynamic_vuid);
    if (dslb.descriptorCount > max_uniform_buffers) {
        const char *max_uniform_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03017"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00288";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_uniform_vuid);  // expect max-per-stage too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03038");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03023");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d20 - too many storage buffer type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dslb.descriptorCount = sum_storage_buffers + 1;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_storage_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03031"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01680";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_storage_vuid);
    if (dslb.descriptorCount > max_storage_buffers) {
        const char *max_storage_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03018"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00289";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_storage_vuid);  // expect max-per-stage too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindStorageBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03039");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03024");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d22 - too many dynamic storage buffer type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    dslb.descriptorCount = sum_dyn_storage_buffers + 1;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_storage_dynamic_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03032"
                                                                     : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01681";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_storage_dynamic_vuid);
    if (dslb.descriptorCount > max_storage_buffers) {
        const char *max_storage_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03018"
                                                             : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00289";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_storage_vuid);  // expect max-per-stage too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03040");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03024");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d24 - too many sampled image type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb.descriptorCount = max_samplers;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    // revisit: not robust to odd limits.
    uint32_t remaining = (max_samplers > sum_sampled_images ? 0 : (sum_sampled_images - max_samplers) / 2);
    dslb.descriptorCount = 1 + remaining;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 2;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dslb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_sampled_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03033"
                                                                   : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01682";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_sampled_image_vuid);
    // Takes max since VUID only checks per shader stage
    if (std::max(dslb_vec[0].descriptorCount, dslb_vec[1].descriptorCount) > max_sampled_images) {
        const char *max_sample_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03019"
                                                                  : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00290";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             max_sample_image_vuid);  // Expect max-per-stage sampled images to exceed limits
    }
    if (descriptor_indexing) {
        if (max_samplers > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036");
        }
        if (max_samplers > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03022");
        }
        if ((dslb_vec[0].descriptorCount + dslb_vec[1].descriptorCount) >
            descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041");
        }
        if (std::max(dslb_vec[0].descriptorCount, dslb_vec[1].descriptorCount) >
            descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03025");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d26 - too many storage image type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dslb.descriptorCount = sum_storage_images / 2;
    dslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dslb.descriptorCount = sum_storage_images - dslb.descriptorCount + 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_storage_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03034"
                                                                   : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01683";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_storage_image_vuid);
    if (dslb.descriptorCount > max_storage_images) {
        const char *max_storage_image_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03020"
                                                                   : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00291";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_storage_image_vuid);  // expect max-per-stage too
    }
    if (descriptor_indexing) {
        if ((sum_storage_images + 1) > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindStorageImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03042");
        }
        if (std::max(dslb_vec[0].descriptorCount, dslb_vec[1].descriptorCount) >
            descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindStorageImages) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03026");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);

    // VU 0fe00d28 - too many input attachment type descriptors overall
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dslb.descriptorCount = sum_input_attachments + 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_all_input_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03035"
                                                           : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01684";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_input_vuid);
    if (dslb.descriptorCount > max_input_attachments) {
        const char *max_input_vuid = (descriptor_indexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03021"
                                                           : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01676";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_input_vuid);  // expect max-per-stage too
    }
    if (descriptor_indexing) {
        if (dslb.descriptorCount > descriptor_indexing_properties.maxDescriptorSetUpdateAfterBindInputAttachments) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03043");
        }
        if (dslb.descriptorCount > descriptor_indexing_properties.maxPerStageDescriptorUpdateAfterBindInputAttachments) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03027");
        }
    }
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);
}

TEST_F(VkLayerTest, InvalidCmdBufferPipelineDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a pipeline dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        GTEST_SKIP() << "This test should not run on Nexus Player";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        // Use helper to create graphics pipeline
        CreatePipelineHelper helper(*this);
        helper.InitInfo();
        helper.InitState();
        helper.CreateGraphicsPipeline();

        // Bind helper pipeline to command buffer
        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
        m_commandBuffer->end();

        // pipeline will be destroyed when helper goes out of scope
    }

    // Cause error by submitting command buffer that references destroyed pipeline
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkPipeline");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidPipeline) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    constexpr uint64_t fake_pipeline_handle = 0xbaad6001;
    VkPipeline bad_pipeline = CastFromUint64<VkPipeline>(fake_pipeline_handle);

    // Enable VK_KHR_draw_indirect_count for KHR variants
    AddOptionalExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.2";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    GetPhysicalDeviceFeatures2(features12);
    bool has_khr_indirect = IsExtensionsEnabled(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    if (has_khr_indirect && !features12.drawIndirectCount) {
        GTEST_SKIP() << VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME << " is enabled, but the drawIndirectCount is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features12));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Attempt to bind an invalid Pipeline to a valid Command Buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, bad_pipeline);
    m_errorMonitor->VerifyFound();

    // Try each of the 6 flavors of Draw()
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);  // Draw*() calls must be submitted within a renderpass

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02700");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBufferObj index_buffer;
    index_buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    m_commandBuffer->BindIndexBuffer(&index_buffer, 2, VK_INDEX_TYPE_UINT16);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexed-None-02700");
    m_commandBuffer->DrawIndexed(1, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBufferObj buffer;
    VkBufferCreateInfo ci = LvlInitStruct<VkBufferCreateInfo>();
    ci.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    ci.size = 1024;
    buffer.init(*m_device, ci);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-None-02700");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-None-02700");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 1, 0);
    m_errorMonitor->VerifyFound();

    if (has_khr_indirect) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-02700");
        // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndirectCommand)
        vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
        m_errorMonitor->VerifyFound();

        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-02700");
            // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndirectCommand)
            vk::CmdDrawIndirectCount(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-02700");
        // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndexedIndirectCommand)
        vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
        m_errorMonitor->VerifyFound();

        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-02700");
            // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndexedIndirectCommand)
            vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
            m_errorMonitor->VerifyFound();
        }
    }

    // Also try the Dispatch variants
    vk::CmdEndRenderPass(m_commandBuffer->handle());  // Compute submissions must be outside a renderpass

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-02700");
    vk::CmdDispatch(m_commandBuffer->handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchIndirect-None-02700");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchIndirect-offset-00407");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer.handle(), ci.size);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CmdDispatchExceedLimits) {
    TEST_DESCRIPTION("Compute dispatch with dimensions that exceed device limits");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    const bool device_group_creation = IsExtensionsEnabled(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);

    uint32_t x_count_limit = m_device->props.limits.maxComputeWorkGroupCount[0];
    uint32_t y_count_limit = m_device->props.limits.maxComputeWorkGroupCount[1];
    uint32_t z_count_limit = m_device->props.limits.maxComputeWorkGroupCount[2];
    if (std::max({x_count_limit, y_count_limit, z_count_limit}) == vvl::kU32Max) {
        GTEST_SKIP() << "device maxComputeWorkGroupCount limit reports UINT32_MAX";
    }

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];
    uint32_t y_size_limit = m_device->props.limits.maxComputeWorkGroupSize[1];
    uint32_t z_size_limit = m_device->props.limits.maxComputeWorkGroupSize[2];

    std::string spv_source = R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main"
        OpExecutionMode %main LocalSize )";
    spv_source.append(std::to_string(x_size_limit + 1) + " " + std::to_string(y_size_limit + 1) + " " +
                      std::to_string(z_size_limit + 1));
    spv_source.append(R"(
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpReturn
                OpFunctionEnd)");

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-x-06429");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-y-06430");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-z-06431");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-x-06432");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    // Create a minimal compute pipeline
    x_size_limit = (x_size_limit > 1024) ? 1024 : x_size_limit;
    y_size_limit = (y_size_limit > 1024) ? 1024 : y_size_limit;
    z_size_limit = (z_size_limit > 64) ? 64 : z_size_limit;

    uint32_t invocations_limit = m_device->props.limits.maxComputeWorkGroupInvocations;
    x_size_limit = (x_size_limit > invocations_limit) ? invocations_limit : x_size_limit;
    invocations_limit /= x_size_limit;
    y_size_limit = (y_size_limit > invocations_limit) ? invocations_limit : y_size_limit;
    invocations_limit /= y_size_limit;
    z_size_limit = (z_size_limit > invocations_limit) ? invocations_limit : z_size_limit;

    std::stringstream cs_text;
    cs_text << "#version 450\n";
    cs_text << "layout(local_size_x = " << x_size_limit << ", ";
    cs_text << "local_size_y = " << y_size_limit << ",";
    cs_text << "local_size_z = " << z_size_limit << ") in;\n";
    cs_text << "void main() {}\n";

    VkShaderObj cs_obj(this, cs_text.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cs_.reset(new VkShaderObj(this, cs_text.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.CreateComputePipeline();

    // Bind pipeline to command buffer
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    // Dispatch counts that exceed device limits
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-groupCountX-00386");
    vk::CmdDispatch(m_commandBuffer->handle(), x_count_limit + 1, y_count_limit, z_count_limit);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-groupCountY-00387");
    vk::CmdDispatch(m_commandBuffer->handle(), x_count_limit, y_count_limit + 1, z_count_limit);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-groupCountZ-00388");
    vk::CmdDispatch(m_commandBuffer->handle(), x_count_limit, y_count_limit, z_count_limit + 1);
    m_errorMonitor->VerifyFound();

    if (device_group_creation) {
        PFN_vkCmdDispatchBaseKHR fp_vkCmdDispatchBaseKHR =
            (PFN_vkCmdDispatchBaseKHR)vk::GetInstanceProcAddr(instance(), "vkCmdDispatchBaseKHR");

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-baseGroupX-00427");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), 1, 1, 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Base equals or exceeds limit
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-baseGroupX-00421");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), x_count_limit, y_count_limit - 1, z_count_limit - 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-baseGroupX-00422");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), x_count_limit - 1, y_count_limit, z_count_limit - 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-baseGroupZ-00423");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), x_count_limit - 1, y_count_limit - 1, z_count_limit, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // (Base + count) exceeds limit
        uint32_t x_base = x_count_limit / 2;
        uint32_t y_base = y_count_limit / 2;
        uint32_t z_base = z_count_limit / 2;
        x_count_limit -= x_base;
        y_count_limit -= y_base;
        z_count_limit -= z_base;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-groupCountX-00424");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), x_base, y_base, z_base, x_count_limit + 1, y_count_limit, z_count_limit);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-groupCountY-00425");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), x_base, y_base, z_base, x_count_limit, y_count_limit + 1, z_count_limit);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatchBase-groupCountZ-00426");
        fp_vkCmdDispatchBaseKHR(m_commandBuffer->handle(), x_base, y_base, z_base, x_count_limit, y_count_limit, z_count_limit + 1);
        m_errorMonitor->VerifyFound();
    } else {
        printf("KHR_DEVICE_GROUP_* extensions not supported, skipping CmdDispatchBaseKHR() tests.\n");
    }
}

TEST_F(VkLayerTest, InvalidPipelineCreateState) {
    TEST_DESCRIPTION("Create Pipelines with invalid state set");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Attempt to Create Gfx Pipeline w/o a VS
    VkPipelineShaderStageCreateInfo shaderStage = fs.GetStageCreateInfo();  // should be: vs.GetStageCreateInfo();

    auto set_info = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {shaderStage}; };
    constexpr std::array vuids = {"VUID-VkGraphicsPipelineCreateInfo-pStages-06896",
                                  "VUID-VkGraphicsPipelineCreateInfo-stage-00727"};
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

TEST_F(VkLayerTest, InvalidPipelineCreateStateBadStageBit) {
    TEST_DESCRIPTION("Create Pipelines with invalid state set");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Make sure compute pipeline has a compute shader stage set
    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main(){
           if (gl_GlobalInvocationID.x >= 0) { return; }
        }
    )glsl";

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_VERTEX_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-stage-00701");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DispatchWithUnboundSet) {
    TEST_DESCRIPTION("Dispatch with unbound descriptor set");
    ASSERT_NO_FATAL_FAILURE(Init());

    char const *cs_source = R"glsl(
        #version 450
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        layout(set = 0, binding = 0) uniform sampler2D InputTexture;
        layout(set = 1, binding = 0, rgba32f) uniform image2D OutputTexture;
        void main() {
            vec4 value = textureGather(InputTexture, vec2(0), 0);
            imageStore(OutputTexture, ivec2(0), value);
        }
    )glsl";

    OneOffDescriptorSet combined_image_set(
        m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    OneOffDescriptorSet storage_image_set(m_device,
                                          {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});

    const VkFormat combined_image_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(1, 1, 1, combined_image_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());

    auto sampler_ci = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&combined_image_set.layout_, &storage_image_set.layout_});
    cs_pipeline.CreateComputePipeline();

    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_);

    combined_image_set.WriteDescriptorImageInfo(0, image.targetView(combined_image_format), sampler.handle());
    combined_image_set.UpdateDescriptorSets();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(), 0,
                              1, &combined_image_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-02697");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingStorageImageFormatRead) {
    TEST_DESCRIPTION("Create a shader reading a storage image without an image format");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    features.shaderStorageImageReadWithoutFormat = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
    // disabled if VK_KHR_format_feature_flags2 is supported.
    //
    //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
    //
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
               OpCapability Shader
               OpCapability StorageImageReadWithoutFormat
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
               OpDecorate %12 NonReadable
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %13 = OpLoad %10 %12
         %18 = OpImageRead %7 %13 %17
               OpStore %9 %18
               OpReturn
               OpFunctionEnd
              )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingStorageImageFormatWrite) {
    TEST_DESCRIPTION("Create a shader writing a storage image without an image format");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    features.shaderStorageImageWriteWithoutFormat = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
    // disabled if VK_KHR_format_feature_flags2 is supported.
    //
    //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
    //
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main" %img
                  OpExecutionMode %main LocalSize 1 1 1
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
                  ; incase shaderStorageImageReadWithoutFormat is not supported
                  OpDecorate %img NonReadable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingStorageImageFormatReadForFormat) {
    TEST_DESCRIPTION("Create a shader reading a storage image without an image format");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vk::GetInstanceProcAddr(instance(),
                                                                                 "vkGetPhysicalDeviceFormatProperties2KHR");

    struct {
        VkFormat format;
        VkFormatProperties3KHR props;
    } tests[2] = {};
    int n_tests = 0;
    bool has_without_format_test = false, has_with_format_test = false;

    // Find storage formats with & without read without format support
    for (uint32_t fmt = VK_FORMAT_R4G4_UNORM_PACK8; fmt < VK_FORMAT_D16_UNORM; fmt++) {
        if (has_without_format_test && has_with_format_test) break;

        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        vkGetPhysicalDeviceFormatProperties2KHR(gpu(), (VkFormat)fmt, &fmt_props);

        const bool has_storage =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT_KHR) != 0;
        const bool has_read_without_format =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR) != 0;

        if (!has_storage) continue;

        if (has_read_without_format) {
            if (has_without_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_without_format_test = true;
            n_tests++;
        } else {
            if (has_with_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_with_format_test = true;
            n_tests++;
        }
    }

    if (n_tests == 0) {
        GTEST_SKIP() << "Could not build a test case.";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
               OpCapability Shader
               OpCapability StorageImageReadWithoutFormat
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
               OpDecorate %12 NonReadable
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %13 = OpLoad %10 %12
         %18 = OpImageRead %7 %13 %17
               OpStore %9 %18
               OpReturn
               OpFunctionEnd
              )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(true, false);                // need false to prevent late binding

    for (int t = 0; t < n_tests; t++) {
        VkFormat format = tests[t].format;

        VkImageObj image(m_device);
        image.Init(32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = image.targetView(format);
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = ds.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptor_write.pImageInfo = &image_info;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        {
            VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
            img_barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
            img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            img_barrier.image = image.handle();  // Image mis-matches with FB image
            img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            img_barrier.subresourceRange.baseArrayLayer = 0;
            img_barrier.subresourceRange.baseMipLevel = 0;
            img_barrier.subresourceRange.layerCount = 1;
            img_barrier.subresourceRange.levelCount = 1;
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                   0, nullptr, 0, nullptr, 1, &img_barrier);
        }

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(),
                                  0, 1, &ds.set_, 0, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpTypeImage-07028");
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        if ((tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR) == 0) {
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, MissingStorageImageFormatWriteForFormat) {
    TEST_DESCRIPTION("Create a shader writing a storage image without an image format");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vk::GetInstanceProcAddr(instance(),
                                                                             "vkGetPhysicalDeviceFormatProperties2KHR");

    struct {
        VkFormat format;
        VkFormatProperties3KHR props;
    } tests[2] = {};
    int n_tests = 0;
    bool has_without_format_test = false, has_with_format_test = false;

    // Find storage formats with & without write without format support
    for (uint32_t fmt = VK_FORMAT_R4G4_UNORM_PACK8; fmt < VK_FORMAT_D16_UNORM; fmt++) {
        if (has_without_format_test && has_with_format_test) break;

        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        vkGetPhysicalDeviceFormatProperties2KHR(gpu(), (VkFormat)fmt, &fmt_props);

        const bool has_storage =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT_KHR) != 0;
        const bool has_write_without_format =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR) != 0;

        if (!has_storage) continue;

        if (has_write_without_format) {
            if (has_without_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_without_format_test = true;
            n_tests++;
        } else {
            if (has_with_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_with_format_test = true;
            n_tests++;
        }
    }

    if (n_tests == 0) {
        GTEST_SKIP() << "Could not build a test case.";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpName %main "main"
                  OpName %img "img"
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(true, false);                // need false to prevent late binding

    for (int t = 0; t < n_tests; t++) {
        VkFormat format = tests[t].format;

        VkImageObj image(m_device);
        image.Init(32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = image.targetView(format);
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = ds.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptor_write.pImageInfo = &image_info;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        {
            VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
            img_barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
            img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            img_barrier.image = image.handle();  // Image mis-matches with FB image
            img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            img_barrier.subresourceRange.baseArrayLayer = 0;
            img_barrier.subresourceRange.baseMipLevel = 0;
            img_barrier.subresourceRange.layerCount = 1;
            img_barrier.subresourceRange.levelCount = 1;
            vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                   0, nullptr, 0, nullptr, 1, &img_barrier);
        }

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(),
                                  0, 1, &ds.set_, 0, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpTypeImage-07027");
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        if ((tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR) == 0) {
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, MissingStorageTexelBufferFormatWriteForFormat) {
    TEST_DESCRIPTION("Create a shader writing a storage texel buffer without an image format");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkSetPhysicalDeviceFormatProperties2EXT fpvkSetPhysicalDeviceFormatProperties2EXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT fpvkGetOriginalPhysicalDeviceFormatProperties2EXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatProperties2EXT, fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto fmt_props_3 = LvlInitStruct<VkFormatProperties3>();
    auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

    // set so format can be used as a storage texel buffer, but no WITHOUT_FORMAT support
    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(gpu(), format, &fmt_props);
    fmt_props.formatProperties.bufferFeatures |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
    fmt_props.formatProperties.bufferFeatures &= ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
    fmt_props_3.bufferFeatures |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
    fmt_props_3.bufferFeatures &= ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
    fpvkSetPhysicalDeviceFormatProperties2EXT(gpu(), format, fmt_props);

    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability ImageBuffer
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float Buffer 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(true, false);                // need false to prevent late binding

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vk_testing::BufferView buffer_view(*m_device, buff_view_ci);
    if (!buffer_view.initialized()) {
        // device profile layer might hide fact this is not a supported buffer view format
        GTEST_SKIP() << "Device will not be able to initialize buffer view skipped";
    }

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &buffer_view.handle();
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(), 0,
                              1, &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpTypeImage-07029");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, MissingNonReadableDecorationStorageImageFormatRead) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format not marked as non readable");

    // We need to skip this test with VK_KHR_format_feature_flags2 supported,
    // because checks for read/write without format has to be done per format
    // rather than as a device feature. The code we test here only looks at
    // the shader.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    features.shaderStorageImageReadWithoutFormat = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %l = OpLabel
          %9 = OpVariable %8 Function
               OpReturn
               OpFunctionEnd
              )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpTypeImage-06270");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingNonWritableDecorationStorageImageFormatWrite) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format but not marked a non writable");

    // We need to skip this test with VK_KHR_format_feature_flags2 supported,
    // because checks for read/write without format has to be done per format
    // rather than as a device feature. The code we test here only looks at
    // the shader.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    features.shaderStorageImageWriteWithoutFormat = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main" %img
                  OpExecutionMode %main LocalSize 1 1 1
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonReadable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %l = OpLabel
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpTypeImage-06269");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingSampledImageDepthComparisonForFormat) {
    TEST_DESCRIPTION("Verify that OpImage*Dref* operations are supported for given format ");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vk::GetInstanceProcAddr(instance(),
                                                                             "vkGetPhysicalDeviceFormatProperties2KHR");

    VkFormat format = VK_FORMAT_UNDEFINED;
    for (uint32_t fmt = VK_FORMAT_R4G4_UNORM_PACK8; fmt < VK_FORMAT_D16_UNORM; fmt++) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        vkGetPhysicalDeviceFormatProperties2KHR(gpu(), (VkFormat)fmt, &fmt_props);

        const bool has_sampling =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT_KHR) != 0;
        const bool has_sampling_img_depth_compare =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR) != 0;

        if (has_sampling && !has_sampling_img_depth_compare) {
            format = (VkFormat)fmt;
            break;
        }
    }

    if (format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Cannot find suitable format, skipping.";
    }

    const char vsSource[] = R"glsl(
        #version 450

        void main() {
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 1) uniform sampler2DShadow tex;
        void main() {
           float f = texture(tex, vec3(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());

    auto sampler_ci = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image.targetView(format), sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-06479");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidPipelineSampleRateFeatureDisable) {
    // Enable sample shading in pipeline when the feature is disabled.
    // Disable sampleRateShading here
    VkPhysicalDeviceFeatures device_features = {};
    device_features.sampleRateShading = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(Init(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Cause the error by enabling sample shading...
    auto set_shading_enable = [](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.sampleShadingEnable = VK_TRUE; };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784");
}

TEST_F(VkLayerTest, InvalidPipelineSampleRateFeatureEnable) {
    // Enable sample shading in pipeline when the feature is disabled.
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Require sampleRateShading here
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (device_features.sampleRateShading == VK_FALSE) {
        GTEST_SKIP() << "SampleRateShading feature is disabled";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

TEST_F(VkLayerTest, InvalidPipelineDepthClipControlFeatureDisable) {
    // Enable negativeOneToOne (VK_EXT_depth_clip_control) in pipeline when the feature is disabled.
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineViewportDepthClipControlCreateInfoEXT clip_control = LvlInitStruct<VkPipelineViewportDepthClipControlCreateInfoEXT>();
    clip_control.negativeOneToOne = VK_TRUE;
    auto set_shading_enable = [clip_control](CreatePipelineHelper &helper) { helper.vp_state_ci_.pNext = &clip_control; };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineViewportDepthClipControlCreateInfoEXT-negativeOneToOne-06470");
}

TEST_F(VkLayerTest, InvalidPipelineSamplePNext) {
    // Enable sample shading in pipeline when the feature is disabled.
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto sample_locations = LvlInitStruct<VkPipelineSampleLocationsStateCreateInfoEXT>();
    sample_locations.sampleLocationsInfo = LvlInitStruct<VkSampleLocationsInfoEXT>();
    auto good_chain = [&sample_locations](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = &sample_locations; };
    CreatePipelineHelper::OneshotTest(*this, good_chain, (kErrorBit | kWarningBit));

    auto instance_ci = LvlInitStruct<VkInstanceCreateInfo>();
    auto bad_chain = [&instance_ci](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = &instance_ci; };
    CreatePipelineHelper::OneshotTest(*this, bad_chain, (kErrorBit | kWarningBit),
                                      "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext");
}

TEST_F(VkLayerTest, InvalidSubpassRasterizationSamples) {
    TEST_DESCRIPTION("Test creating two pipelines referring to the same subpass but with different rasterization samples count");

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    VkPhysicalDeviceFeatures device_features = {};
    device_features.variableMultisampleRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass renderpass(*m_device, rpci);
    ASSERT_TRUE(renderpass.initialized());

    auto render_target_view = m_renderTargets[0]->targetView(m_renderTargets[0]->format());
    auto framebuffer_info = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_info.renderPass = renderpass;
    framebuffer_info.width = m_renderTargets[0]->width();
    framebuffer_info.height = m_renderTargets[0]->height();
    framebuffer_info.layers = 1;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &render_target_view;
    vk_testing::Framebuffer framebuffer(*m_device, framebuffer_info);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipeline_1(m_device);
    pipeline_1.AddShader(&vs);
    pipeline_1.AddShader(&fs);
    VkPipelineMultisampleStateCreateInfo ms_state = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_1.SetMSAA(&ms_state);

    VkPipelineObj pipeline_2(m_device);
    pipeline_2.AddShader(&vs);
    pipeline_2.AddShader(&fs);
    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    pipeline_2.SetMSAA(&ms_state);

    pipeline_1.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass.handle());
    pipeline_2.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass.handle());

    auto rpbinfo = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbinfo.renderPass = renderpass.handle();
    rpbinfo.framebuffer = framebuffer;
    rpbinfo.renderArea.extent.width = framebuffer_info.width;
    rpbinfo.renderArea.extent.height = framebuffer_info.height;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rpbinfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_1.handle());

    // VkPhysicalDeviceFeatures::variableMultisampleRate is false,
    // the two pipelines refer to the same subpass, one that does not use any attachment,
    // BUT the secondly created pipeline has a different sample samples count than the 1st, this is illegal
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-subpass-00758");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_2.handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidPipelineRenderPassShaderResolveQCOM) {
    TEST_DESCRIPTION("Test pipeline creation VUIDs added with VK_QCOM_render_pass_shader_resolve extension.");
    AddRequiredExtensions(VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    // Require sampleRateShading for these tests
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (device_features.sampleRateShading == VK_FALSE) {
        GTEST_SKIP() << "SampleRateShading feature is disabled -- skipping related checks.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkPipelineObj pipeline(m_device);
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

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
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
    vk_testing::RenderPass renderpass(*m_device, rpci);
    ASSERT_TRUE(renderpass.initialized());

    // renderpass2 has 1xMSAA colorAttachent and 1xMSAA inputAttachment
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vk_testing::RenderPass renderpass2(*m_device, rpci);
    ASSERT_TRUE(renderpass2.initialized());

    // shader uses gl_SamplePosition which causes the SPIR-V to include SampleRateShading capability
    static const char *sampleRateFragShaderText = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main() {
            uFragColor = vec4(gl_SamplePosition.x,1,0,1);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkShaderObj fs_sampleRate(this, sampleRateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline.AddShader(&vs);
    pipeline.AddShader(&fs);

    VkPipelineColorBlendAttachmentState att_state1 = {};
    att_state1.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state1.blendEnable = VK_TRUE;
    pipeline.AddColorAttachment(0, att_state1);

    VkPipelineMultisampleStateCreateInfo ms_state = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state.flags = 0;
    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state.sampleShadingEnable = VK_FALSE;
    ms_state.minSampleShading = 0.0f;
    ms_state.pSampleMask = nullptr;
    ms_state.alphaToCoverageEnable = VK_FALSE;
    ms_state.alphaToOneEnable = VK_FALSE;
    pipeline.SetMSAA(&ms_state);

    // Create a pipeline with a subpass using VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // but where sample count of input attachment doesnt match rasterizationSamples
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-rasterizationSamples-04899");
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass.handle());
    m_errorMonitor->VerifyFound();

    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state.sampleShadingEnable = VK_TRUE;
    pipeline.SetMSAA(&ms_state);

    // Create a pipeline with a subpass using VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // and with sampleShadingEnable enabled in the pipeline
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-sampleShadingEnable-04900");
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass2.handle());
    m_errorMonitor->VerifyFound();

    ms_state.sampleShadingEnable = VK_FALSE;
    VkPipelineObj pipeline2(m_device);
    pipeline2.SetMSAA(&ms_state);
    pipeline2.AddColorAttachment(0, att_state1);
    pipeline2.AddShader(&vs);
    pipeline2.AddShader(&fs_sampleRate);

    // Create a pipeline with a subpass using VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // and with SampleRateShading capability enabled in the SPIR-V fragment shader
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-SampleRateShading-06378");
    pipeline2.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass2.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RasterizerDiscardWithFragmentShader) {
    TEST_DESCRIPTION("Create Graphics Pipeline with fragment shader and rasterizer discard");
    ASSERT_NO_FATAL_FAILURE(Init());
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
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
    auto pipeline_layout_create_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    VkResult err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
    ASSERT_VK_SUCCESS(err);

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

TEST_F(VkLayerTest, CreateGraphicsPipelineWithBadBasePointer) {
    TEST_DESCRIPTION("Create Graphics Pipeline with pointers that must be ignored by layers");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

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
    auto pipeline_layout_create_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    VkResult err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
    ASSERT_VK_SUCCESS(err);

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

TEST_F(VkLayerTest, SetDepthRangeUnrestricted) {
    TEST_DESCRIPTION("Test setting minDepthBounds and maxDepthBounds without VK_EXT_depth_range_unrestricted");

    // Extension doesn't have feature bit, so not enabling extension invokes restrictions
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (VK_TRUE != device_features.depthBounds) {
        GTEST_SKIP() << "Test requires unsupported depthBounds feature";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    VkPipelineDepthStencilStateCreateInfo ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthBoundsTestEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.ds_ci_ = ds_ci;
    pipe.InitState();

    pipe.ds_ci_.minDepthBounds = 1.5f;
    pipe.ds_ci_.maxDepthBounds = 1.0f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.ds_ci_.minDepthBounds = 1.0f;
    pipe.ds_ci_.maxDepthBounds = 1.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Add dynamic depth stencil state instead
    pipe.ds_ci_.minDepthBounds = 0.0f;
    pipe.ds_ci_.maxDepthBounds = 0.0f;
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DEPTH_BOUNDS};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBounds-minDepthBounds-02508");
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 1.5f, 0.0f);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBounds-maxDepthBounds-02509");
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 0.0f, 1.5f);
    m_errorMonitor->VerifyFound();

    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 1.0f, 1.0f);
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, PSOViewportStateTests) {
    TEST_DESCRIPTION("Test VkPipelineViewportStateCreateInfo viewport and scissor count validation for non-multiViewport");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (m_device->props.limits.maxViewports < 3) {
        GTEST_SKIP() << "maxViewports is not large enough";
    }

    const auto break_vp_state = [](CreatePipelineHelper &helper) {
        helper.rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
        helper.gp_ci_.pViewportState = nullptr;
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp_state, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00750");

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[] = {scissor, scissor};

    // test viewport and scissor arrays
    using std::vector;
    struct TestCase {
        uint32_t viewport_count;
        VkViewport *viewports;
        uint32_t scissor_count;
        VkRect2D *scissors;

        vector<std::string> vuids;
    };

    vector<TestCase> test_cases = {
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {1,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
        {1, nullptr, 1, scissors, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747"}},
        {1, viewports, 1, nullptr, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}},
        {1,
         nullptr,
         1,
         nullptr,
         {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}},
        {2,
         nullptr,
         3,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747",
          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}},
        {2,
         nullptr,
         2,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}},
    };

    for (const auto &test_case : test_cases) {
        const auto break_vp = [&test_case](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }

    vector<TestCase> dyn_test_cases = {
        {0,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {1,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {1,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
        {2,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
        {0,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         nullptr,
         3,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength"}},
    };

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    for (const auto &test_case : dyn_test_cases) {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
            dyn_state_ci.dynamicStateCount = size(dyn_states);
            dyn_state_ci.pDynamicStates = dyn_states;
            helper.dyn_state_ci_ = dyn_state_ci;

            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }
}

TEST_F(VkLayerTest, PSOViewportStateMultiViewportTests) {
    TEST_DESCRIPTION("Test VkPipelineViewportStateCreateInfo viewport and scissor count validation for multiViewport feature");

    ASSERT_NO_FATAL_FAILURE(Init());  // enables all supported features

    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::multiViewport is not supported";
    }
    // at least 16 viewports supported from here on

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[] = {scissor, scissor};

    using std::vector;
    struct TestCase {
        uint32_t viewport_count;
        VkViewport *viewports;
        uint32_t scissor_count;
        VkRect2D *scissors;

        vector<std::string> vuids;
    };

    vector<TestCase> test_cases = {
        {0,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {0,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength"}},
        {2, nullptr, 2, scissors, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747"}},
        {2, viewports, 2, nullptr, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}},
        {2,
         nullptr,
         2,
         nullptr,
         {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}},
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength"}},
    };

    const auto max_viewports = m_device->phy().properties().limits.maxViewports;
    const bool max_viewports_maxxed = max_viewports == std::numeric_limits<decltype(max_viewports)>::max();
    if (max_viewports_maxxed) {
        printf("VkPhysicalDeviceLimits::maxViewports is UINT32_MAX -- skipping part of test requiring to exceed maxViewports.\n");
    } else {
        const auto too_much_viewports = max_viewports + 1;
        // avoid potentially big allocations by using only nullptr
        test_cases.push_back({too_much_viewports,
                              nullptr,
                              2,
                              scissors,
                              {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                               "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220",
                               "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747"}});
        test_cases.push_back({2,
                              viewports,
                              too_much_viewports,
                              nullptr,
                              {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                               "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220",
                               "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}});
        test_cases.push_back(
            {too_much_viewports,
             nullptr,
             too_much_viewports,
             nullptr,
             {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
              "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00747",
              "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00748"}});
    }

    for (const auto &test_case : test_cases) {
        const auto break_vp = [&test_case](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }

    vector<TestCase> dyn_test_cases = {
        {0,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {0,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength"}},
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength"}},
    };

    if (!max_viewports_maxxed) {
        const auto too_much_viewports = max_viewports + 1;
        // avoid potentially big allocations by using only nullptr
        dyn_test_cases.push_back({too_much_viewports,
                                  nullptr,
                                  2,
                                  scissors,
                                  {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                   "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}});
        dyn_test_cases.push_back({2,
                                  viewports,
                                  too_much_viewports,
                                  nullptr,
                                  {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                   "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}});
        dyn_test_cases.push_back({too_much_viewports,
                                  nullptr,
                                  too_much_viewports,
                                  nullptr,
                                  {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                   "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219"}});
    }

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    for (const auto &test_case : dyn_test_cases) {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
            dyn_state_ci.dynamicStateCount = size(dyn_states);
            dyn_state_ci.pDynamicStates = dyn_states;
            helper.dyn_state_ci_ = dyn_state_ci;

            helper.vp_state_ci_.viewportCount = test_case.viewport_count;
            helper.vp_state_ci_.pViewports = test_case.viewports;
            helper.vp_state_ci_.scissorCount = test_case.scissor_count;
            helper.vp_state_ci_.pScissors = test_case.scissors;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
    }
}

TEST_F(VkLayerTest, PSOLineWidthInvalid) {
    TEST_DESCRIPTION("Test non-1.0 lineWidth errors when pipeline is created and in vkCmdSetLineWidth");
    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    if (IsPlatform(kNexusPlayer)) {
        GTEST_SKIP() << "This test should not run on Nexus Player";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::array test_cases = {-1.0f, 0.0f, NearestSmaller(1.0f), NearestGreater(1.0f),
                                   std::numeric_limits<float>::quiet_NaN()};

    // test VkPipelineRasterizationStateCreateInfo::lineWidth
    for (const auto test_case : test_cases) {
        const auto set_lineWidth = [&](CreatePipelineHelper &helper) { helper.rs_state_ci_.lineWidth = test_case; };
        CreatePipelineHelper::OneshotTest(*this, set_lineWidth, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749");
    }

    // test vk::CmdSetLineWidth
    m_commandBuffer->begin();

    for (const auto test_case : test_cases) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineWidth-lineWidth-00788");
        vk::CmdSetLineWidth(m_commandBuffer->handle(), test_case);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, PipelineCreationCacheControl) {
    TEST_DESCRIPTION("Test VK_EXT_pipeline_creation_cache_control");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto cache_control_features = LvlInitStruct<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>();
    cache_control_features.pipelineCreationCacheControl = VK_FALSE;  // Tests all assume feature is off
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &cache_control_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    VkPipelineCacheCreateInfo cache_create_info = LvlInitStruct<VkPipelineCacheCreateInfo>();
    cache_create_info.initialDataSize = 0;
    cache_create_info.flags = VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineCacheCreateInfo-pipelineCreationCacheControl-02892");
    vk::CreatePipelineCache(m_device->device(), &cache_create_info, nullptr, &pipeline_cache);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NumSamplesMismatch) {
    // Create CommandBuffer where MSAA samples doesn't match RenderPass
    // sampleCount
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-rasterizationSamples-04740");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);  // We shouldn't need a fragment shader
    // but add it to be able to run on more devices
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.SetMSAA(&pipe_ms_state_ci);

    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853");
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    // Render triangle (the error should trigger on the attempt to draw).
    m_commandBuffer->Draw(3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NumBlendAttachMismatch) {
    // Create Pipeline where the number of blend attachments doesn't match the
    // number of color attachments.  In this case, we don't add any color
    // blend attachments even though we have a color attachment.

    AddOptionalExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const auto set_MSAA = [&](CreatePipelineHelper &helper) {
        helper.pipe_ms_state_ci_ = pipe_ms_state_ci;
        helper.cb_ci_.attachmentCount = 0;
    };
    const char *vuid = IsExtensionsEnabled(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)
                           ? "VUID-VkGraphicsPipelineCreateInfo-renderPass-07609"
                           : "VUID-VkGraphicsPipelineCreateInfo-renderPass-06042";
    CreatePipelineHelper::OneshotTest(*this, set_MSAA, kErrorBit, vuid);
}

TEST_F(VkLayerTest, CmdClearAttachmentTests) {
    TEST_DESCRIPTION("Various tests for validating usage of vkCmdClearAttachments");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageFormatProperties image_format_properties{};
    ASSERT_VK_SUCCESS(vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), m_renderTargets[0]->format(),
                                                                 VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                                 m_renderTargets[0]->usage(), 0, &image_format_properties));
    if (image_format_properties.maxArrayLayers < 4) {
        GTEST_SKIP() << "Test needs to create image 2D array of 4 image view, but VkImageFormatProperties::maxArrayLayers is < 4. "
                        "Skipping test.";
    }

    // Create frame buffer with 2 layers, and image view with 4 layers,
    // to make sure that considered layer count is the one coming from frame buffer
    // (test would not fail if layer count used to do validation was 4)
    VkImageObj render_target(m_device);
    assert(!m_renderTargets.empty());
    const auto render_target_ci = VkImageObj::ImageCreateInfo2D(
        m_renderTargets[0]->width(), m_renderTargets[0]->height(), m_renderTargets[0]->create_info().mipLevels, 4,
        m_renderTargets[0]->format(), m_renderTargets[0]->usage(), VK_IMAGE_TILING_OPTIMAL);
    render_target.Init(render_target_ci, 0);
    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = render_target.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = render_target_ci.format;
    ivci.subresourceRange.layerCount = render_target_ci.arrayLayers;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.components.r = VK_COMPONENT_SWIZZLE_R;
    ivci.components.g = VK_COMPONENT_SWIZZLE_G;
    ivci.components.b = VK_COMPONENT_SWIZZLE_B;
    ivci.components.a = VK_COMPONENT_SWIZZLE_A;
    vk_testing::ImageView render_target_view(*m_device, ivci);
    VkFramebufferCreateInfo fb_info = m_framebuffer_info;
    fb_info.layers = 2;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &render_target_view.handle();
    vk_testing::Framebuffer framebuffer(*m_device, fb_info);
    m_renderPassBeginInfo.framebuffer = framebuffer.handle();

    // Create secondary command buffer
    auto secondary_cmd_buffer_alloc_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    secondary_cmd_buffer_alloc_info.commandPool = m_commandPool->handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vk_testing::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);
    VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    secondary_cmd_buffer_inheritance_info.renderPass = m_renderPass;
    secondary_cmd_buffer_inheritance_info.framebuffer = framebuffer.handle();

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &secondary_cmd_buffer_inheritance_info;

    // Create clear rect
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    auto clear_cmds = [this, &color_attachment](VkCommandBuffer cmd_buffer, VkClearRect clear_rect) {
        // extent too wide
        VkClearRect clear_rect_too_large = clear_rect;
        clear_rect_too_large.rect.extent.width = renderPassBeginInfo().renderArea.extent.width + 4;
        clear_rect_too_large.rect.extent.height = clear_rect_too_large.rect.extent.height / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00016");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect_too_large);

        // baseLayer < render pass instance layer count
        clear_rect.baseArrayLayer = 1;
        clear_rect.layerCount = 1;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount <= render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 2;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer >= render pass instance layer count
        clear_rect.baseArrayLayer = 2;
        clear_rect.layerCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount > render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 4;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);
    };

    // Register clear commands to secondary command buffer
    secondary_cmd_buffer.begin(&secondary_cmd_buffer_begin_info);
    clear_cmds(secondary_cmd_buffer.handle(), clear_rect);
    secondary_cmd_buffer.end();

    m_commandBuffer->begin();

    // Execute secondary command buffer
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    // Execute same commands as previously, but in a primary command buffer
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    clear_cmds(m_commandBuffer->handle(), clear_rect);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ColorBlendInvalidLogicOp) {
    TEST_DESCRIPTION("Attempt to use invalid VkPipelineColorBlendStateCreateInfo::logicOp value.");

    ASSERT_NO_FATAL_FAILURE(Init());  // enables all supported features
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

TEST_F(VkLayerTest, ColorBlendUnsupportedLogicOp) {
    TEST_DESCRIPTION("Attempt enabling VkPipelineColorBlendStateCreateInfo::logicOpEnable when logicOp feature is disabled.");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto set_shading_enable = [](CreatePipelineHelper &helper) { helper.cb_ci_.logicOpEnable = VK_TRUE; };
    CreatePipelineHelper::OneshotTest(*this, set_shading_enable, kErrorBit,
                                      "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00606");
}

TEST_F(VkLayerTest, ColorBlendUnsupportedDualSourceBlend) {
    TEST_DESCRIPTION("Attempt to use dual-source blending when dualSrcBlend feature is disabled.");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

TEST_F(VkLayerTest, InvalidSPIRVCodeSize) {
    TEST_DESCRIPTION("Test that errors are produced for a spirv modules with invalid code sizes");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Invalid SPIR-V header");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderModule module;
    VkShaderModuleCreateInfo moduleCreateInfo = LvlInitStruct<VkShaderModuleCreateInfo>();
    struct icd_spv_header spv;

    spv.magic = ICD_SPV_MAGIC;
    spv.version = ICD_SPV_VERSION;
    spv.gen_magic = 0;

    moduleCreateInfo.pCode = (const uint32_t *)&spv;
    moduleCreateInfo.codeSize = 4;
    moduleCreateInfo.flags = 0;
    vk::CreateShaderModule(m_device->device(), &moduleCreateInfo, NULL, &module);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-codeSize-08735");
    std::vector<uint32_t> shader;
    VkShaderModuleCreateInfo module_create_info = LvlInitStruct<VkShaderModuleCreateInfo>();
    VkShaderModule shader_module;
    this->GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText, shader);
    module_create_info.pCode = shader.data();
    // Introduce failure by making codeSize a non-multiple of 4
    module_create_info.codeSize = shader.size() * sizeof(uint32_t) - 1;
    module_create_info.flags = 0;
    vk::CreateShaderModule(m_device->handle(), &module_create_info, NULL, &shader_module);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidSPIRVMagic) {
    TEST_DESCRIPTION("Test that an error is produced for a spirv module with a bad magic number");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Invalid SPIR-V magic number");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderModule module;
    VkShaderModuleCreateInfo moduleCreateInfo = LvlInitStruct<VkShaderModuleCreateInfo>();
    struct icd_spv_header spv;

    spv.magic = (uint32_t)~ICD_SPV_MAGIC;
    spv.version = ICD_SPV_VERSION;
    spv.gen_magic = 0;

    moduleCreateInfo.pCode = (const uint32_t *)&spv;
    moduleCreateInfo.codeSize = sizeof(spv);
    moduleCreateInfo.flags = 0;
    vk::CreateShaderModule(m_device->device(), &moduleCreateInfo, NULL, &module);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineVertexOutputNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex output that is not consumed by the fragment stage");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float x;
        void main(){
           gl_Position = vec4(1);
           x = 0;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit,
                                      "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderSpecializationApplied) {
    TEST_DESCRIPTION(
        "Make sure specialization constants get applied during shader validation by using a value that breaks compilation.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Size an array using a specialization constant of default value equal to 1.
    const char *fs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %size "size"
               OpName %array "array"
               OpDecorate %size SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
       %size = OpSpecConstant %int 1
%_arr_float_size = OpTypeArray %float %size
%_ptr_Function__arr_float_size = OpTypePointer Function %_arr_float_size
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
       %main = OpFunction %void None %3
          %5 = OpLabel
      %array = OpVariable %_ptr_Function__arr_float_size Function
         %15 = OpAccessChain %_ptr_Function_float %array %int_0
               OpStore %15 %float_0
               OpReturn
               OpFunctionEnd)";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    // Set the specialization constant to 0.
    const VkSpecializationMapEntry entry = {
        0,                // id
        0,                // offset
        sizeof(uint32_t)  // size
    };
    uint32_t data = 0;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint32_t),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06719");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderBadSpecializationOffsetOutOfBounds) {
    TEST_DESCRIPTION("Challenge core_validation with shader validation issues related to vkCreateGraphicsPipelines.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Entry offset is greater than dataSize.
    const VkSpecializationMapEntry entry = {0, 5, sizeof(uint32_t)};

    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-offset-00773");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderBadSpecializationSizeOutOfBounds) {
    TEST_DESCRIPTION("Challenge core_validation with shader validation issues related to vkCreateGraphicsPipelines.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Entry size is greater than dataSize minus offset.
    const VkSpecializationMapEntry entry = {0, 3, sizeof(uint32_t)};

    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-pMapEntries-00774");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderDescriptorTypeMismatch) {
    TEST_DESCRIPTION("Challenge core_validation with shader validation issues related to vkCreateGraphicsPipelines.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    char const *vsSource = R"glsl(
        #version 450
        layout (std140, set = 0, binding = 0) uniform buf {
            mat4 mvp;
        } ubuf;
        void main(){
           gl_Position = ubuf.mvp * vec4(1);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07989");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderDescriptorNotAccessible) {
    TEST_DESCRIPTION(
        "Create a pipeline in which a descriptor used by a shader stage does not include that stage in its stageFlags.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT /*!*/, nullptr},
                                     });

    char const *vsSource = R"glsl(
        #version 450
        layout (std140, set = 0, binding = 0) uniform buf {
            mat4 mvp;
        } ubuf;
        void main(){
           gl_Position = ubuf.mvp * vec4(1);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07988");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderPushConstantNotDeclared) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline in which a push constant range containing a push constant block member is not declared in the "
        "layout.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {};
    // Set to the wrong stage to challenge core_validation
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size = 4;

    const VkPipelineLayoutObj pipeline_layout(m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {}, {push_constant_range});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidPushConstantRange) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange structs.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceProperties device_props = {};
    vk::GetPhysicalDeviceProperties(gpu(), &device_props);
    // will be at least 256 as required from the spec
    const uint32_t maxPushConstantsSize = device_props.limits.maxPushConstantsSize;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPushConstantRange push_constant_range = {0, 0, 4};
    VkPipelineLayoutCreateInfo pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};

    // stageFlags of 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-stageFlags-requiredbitmask");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // offset over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, maxPushConstantsSize, 8};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-offset-00294");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // offset not multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 1, 8};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-offset-00295");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size of 0
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00296");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size not multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 7};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00297");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize + 4};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size over limit of non-zero offset
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 4, maxPushConstantsSize};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // Sanity check its a valid range before making duplicate
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    ASSERT_VK_SUCCESS(vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout));
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, nullptr);

    // Duplicate ranges
    VkPushConstantRange push_constant_range_duplicate[2] = {push_constant_range, push_constant_range};
    pipeline_layout_info.pushConstantRangeCount = 2;
    pipeline_layout_info.pPushConstantRanges = push_constant_range_duplicate;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, nullptr, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdPushConstantRange) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange values in vkCmdPushConstants.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Set limit to be same max as the shader usages
    const uint32_t maxPushConstantsSize = 16;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxPushConstantsSize = maxPushConstantsSize;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x[4]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0]);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    const VkPipelineLayoutObj pipeline_layout(m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    const float data[16] = {};  // dummy data to match shader size

    m_commandBuffer->begin();

    // size of 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-arraylength");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 0, data);
    m_errorMonitor->VerifyFound();

    // offset not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-00368");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 1, 4, data);
    m_errorMonitor->VerifyFound();

    // size not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-00369");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 5, data);
    m_errorMonitor->VerifyFound();

    // offset at limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-00370");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-00371");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT,
                         maxPushConstantsSize, 4, data);
    m_errorMonitor->VerifyFound();

    // size at limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-00371");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize + 4, data);
    m_errorMonitor->VerifyFound();

    // Size at limit, should be valid
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize, data);

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderNotEnabled) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline in which a capability declared by the shader requires a feature not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Some awkward steps are required to test with custom device features.
    VkPhysicalDeviceFeatures device_features = {};
    // Disable support for 64 bit floats
    device_features.shaderFloat64 = false;
    // The sacrificial device object
    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main(){
           dvec4 green = vec4(0.0, 1.0, 0.0, 1.0);
           color = vec4(green);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateShaderModuleCheckBadCapability) {
    TEST_DESCRIPTION("Create a shader in which a capability declared by the shader is not supported.");
    // Note that this failure message comes from spirv-tools, specifically the validator.

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char *spv_source = R"(
                  OpCapability ImageRect
                  OpEntryPoint Vertex %main "main"
          %main = OpFunction %void None %3
                  OpReturn
                  OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Capability ImageRect is not allowed by Vulkan");
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineFragmentInputNotProvided) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input which is not present in the outputs of the previous stage");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(VkLayerTest, CreatePipelineFragmentInputNotProvidedInBlock) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input within an interace block, which is not present in the outputs "
        "of the previous stage.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(VkLayerTest, CreatePipelineVsFsTypeMismatch) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched types across the vertex->fragment shader interface");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out int x;
        void main(){
           x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(VkLayerTest, CreatePipelineVsFsTypeMismatchInBlock) {
    TEST_DESCRIPTION(
        "Test that an error is produced for mismatched types across the vertex->fragment shader interface, when the variable is "
        "contained within an interface block");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0) int x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByLocation) {
    TEST_DESCRIPTION(
        "Test that an error is produced for location mismatches across the vertex->fragment shader interface; This should manifest "
        "as a not-written/not-consumed pair, but flushes out broken walking of the interfaces");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=1) float x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByComponent) {
    TEST_DESCRIPTION(
        "Test that an error is produced for component mismatches across the vertex->fragment shader interface. It's not enough to "
        "have the same set of locations in use; matching is defined in terms of spirv variables.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0, component=0) float x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0, component=1) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(VkLayerTest, CreatePipelineDuplicateStage) {
    TEST_DESCRIPTION("Test that an error is produced for a pipeline containing multiple shaders for the same stage");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), helper.vs_->GetStageCreateInfo(),
                                 helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stage-06897");
}

TEST_F(VkLayerTest, CreatePipelineMissingEntrypoint) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Graphics
    {
        VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "foo");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
    }

    // Compute
    {
        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(new VkShaderObj(this, bindStateMinimalShaderText, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                             SPV_SOURCE_GLSL, nullptr, "foo"));
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
    }

    // Multiple pipeline, middle has missing entrypoint
    {
        CreateComputePipelineHelper pipe_0(*this);  // valid
        pipe_0.InitInfo();
        pipe_0.InitState();
        pipe_0.LateBindPipelineInfo();
        CreateComputePipelineHelper pipe_1(*this);  // invalid
        pipe_1.InitInfo();
        pipe_1.cs_.reset(new VkShaderObj(this, bindStateMinimalShaderText, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                         SPV_SOURCE_GLSL, nullptr, "foo"));
        pipe_1.InitState();
        pipe_1.LateBindPipelineInfo();

        VkComputePipelineCreateInfo create_infos[3] = {pipe_0.cp_ci_, pipe_1.cp_ci_, pipe_0.cp_ci_};
        VkPipeline pipelines[3];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
        vk::CreateComputePipelines(m_device->device(), VK_NULL_HANDLE, 3, create_infos, nullptr, pipelines);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, CreatePipelineDepthStencilRequired) {
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "pDepthStencilState is NULL when rasterization is enabled and subpass uses a depth/stencil attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

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
    vk_testing::RenderPass rp(*m_device, rpci);

    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), rp.handle());

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineFragmentOutputNotWritten) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader which does not provide an output for one of the pipeline's color "
        "attachments");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj fs(this, bindStateMinimalShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.cb_attachments_[0].colorWriteMask = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "UNASSIGNED-CoreValidation-Shader-InputNotProduced");
}

TEST_F(VkLayerTest, CreatePipelineFragmentOutputNotConsumed) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a fragment shader which provides a spurious output with no matching attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(location=1) out vec4 y; /* no matching attachment for this */
        void main(){
           x = vec4(1);
           y = vec4(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
}

// Currently need to clarify the VU - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5520
TEST_F(VkLayerTest, DISABLED_CreatePipelineFragmentNoOutputLocation0ButAlphaToCoverageEnabled) {
    TEST_DESCRIPTION("Test that an error is produced when alpha to coverage is enabled but no output at location 0 is declared.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(0u));

    VkShaderObj fs(this, bindStateMinimalShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage");
}

// Currently need to clarify the VU - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5520
TEST_F(VkLayerTest, DISABLED_CreatePipelineFragmentNoAlphaLocation0ButAlphaToCoverageEnabled) {
    TEST_DESCRIPTION(
        "Test that an error is produced when alpha to coverage is enabled but output at location 0 doesn't have alpha component.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(0u));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 x;
        void main(){
           x = vec3(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage");
}

TEST_F(VkLayerTest, CreatePipelineFragmentOutputTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a mismatch between the fundamental type of an fragment shader output variable, and the "
        "format of the corresponding attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out ivec4 x; /* not UNORM */
        void main(){
           x = ivec4(1);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "UNASSIGNED-CoreValidation-Shader-FragmentOutputMismatch");
}

TEST_F(VkLayerTest, CreatePipelineExceedVertexMaxComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpMemberDecorate built-ins");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxVertexOutputComponents = 128;
    props.limits.maxFragmentInputComponents = 128;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // vec4 == 4 components
    // This gives 124 which is just below the set max limit
    const uint32_t numVec4 = 31;

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }

    // GLSL is defined to have a struct for the vertex shader built-in:
    //
    //    out gl_PerVertex {
    //        vec4 gl_Position;
    //        float gl_PointSize;
    //        float gl_ClipDistance[];
    //        float gl_CullDistance[];
    //    } gl_out[];
    //
    // by including gl_Position here 7 extra vertex input components are added pushing it over the 128
    // 124 + 7 > 128 limit
    vsSourceStr += "    gl_Position = x;\n";
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1);\n"
        "}\n";

    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    // maxFragmentInputComponents is not reached because GLSL should not be including any input fragment stage built-ins by default
    // only maxVertexOutputComponents is reached
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
}

TEST_F(VkLayerTest, CreatePipelineExceedFragmentMaxComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpDecorate built-ins");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxVertexOutputComponents = 128;
    props.limits.maxFragmentInputComponents = 128;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // vec4 == 4 components
    // This gives 128 which is the max limit
    const uint32_t numVec4 = 32;  // 32 * 4 == 128

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    // By added gl_PointCoord it adds 2 more components to the fragment input stage
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1) * gl_PointCoord.x;\n"
        "}\n";

    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    // maxVertexOutputComponents is not reached because GLSL should not be including any output vertex stage built-ins
    // only maxFragmentInputComponents is reached
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxVertexOutputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of output components from the vertex stage exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxVsOutComp = m_device->props.limits.maxVertexOutputComponents + overflow;
        std::string vsSourceStr = "#version 450\n\n";
        const uint32_t numVec4 = maxVsOutComp / 4;
        uint32_t location = 0;
        if (overflow == 2) {
            vsSourceStr += "layout(location=" + std::to_string(numVec4 + 1) + ") out vec4 vn;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numVec4; i++) {
                vsSourceStr += "layout(location=" + std::to_string(location) + ") out vec4 v" + std::to_string(i) + ";\n";
                location += 1;
            }
            const uint32_t remainder = maxVsOutComp % 4;
            if (remainder != 0) {
                if (remainder == 1) {
                    vsSourceStr += "layout(location=" + std::to_string(location) + ") out float" + " vn;\n";
                } else {
                    vsSourceStr +=
                        "layout(location=" + std::to_string(location) + ") out vec" + std::to_string(remainder) + " vn;\n";
                }
                location += 1;
            }
        }
        vsSourceStr +=
            "void main(){\n"
            "}\n";

        std::string fsSourceStr = R"glsl(
            #version 450
            layout(location=0) out vec4 color;
            void main(){
                color = vec4(1);
            }
        )glsl";

        VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        switch (overflow) {
            case 0: {
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
                break;
            }
            case 1: {
                // component and location limit (maxVertexOutputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            }
            case 2: {
                // just component limit (maxVertexOutputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxComponentsBlocks) {
    TEST_DESCRIPTION("Test if the max componenets checks are done properly when in a single block");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // To make the test simple, just make sure max is 128 or less (most HW is 64 or 128)
    if (m_device->props.limits.maxVertexOutputComponents > 128 || m_device->props.limits.maxFragmentInputComponents > 128) {
        GTEST_SKIP() << "maxVertexOutputComponents or maxFragmentInputComponents too high for test";
    }
    // vec4 == 4 components
    // so this put the test over 128
    const uint32_t numVec4 = 33;

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1);\n"
        "}\n";

    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    // 1 for maxVertexOutputComponents and 1 for maxFragmentInputComponents
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxFragmentInputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of input components from the fragment stage exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxFsInComp = m_device->props.limits.maxFragmentInputComponents + overflow;
        std::string fsSourceStr = "#version 450\n\n";
        const uint32_t numVec4 = maxFsInComp / 4;
        uint32_t location = 0;
        if (overflow == 2) {
            fsSourceStr += "layout(location=" + std::to_string(numVec4 + 1) + ") in float" + " vn;\n";
        } else {
            for (uint32_t i = 0; i < numVec4; i++) {
                fsSourceStr += "layout(location=" + std::to_string(location) + ") in vec4 v" + std::to_string(i) + ";\n";
                location += 1;
            }
            const uint32_t remainder = maxFsInComp % 4;
            if (remainder != 0) {
                if (remainder == 1) {
                    fsSourceStr += "layout(location=" + std::to_string(location) + ") in float" + " vn;\n";
                } else {
                    fsSourceStr +=
                        "layout(location=" + std::to_string(location) + ") in vec" + std::to_string(remainder) + " vn;\n";
                }
                location += 1;
            }
        }
        fsSourceStr +=
            "layout(location=0) out vec4 color;"
            "\n"
            "void main(){\n"
            "    color = vec4(1);\n"
            "}\n";
        VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

        m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpEntryPoint-08743");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        switch (overflow) {
            case 0: {
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
                break;
            }
            case 1: {
                // (maxFragmentInputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            }
            case 2:
                // (maxFragmentInputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineUniformBlockNotProvided) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming a uniform block which has no corresponding binding in the pipeline "
        "layout");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "not declared in pipeline layout");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    /* set up CB 0; type is UNORM by default */
    pipe.AddDefaultColorAttachment();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineNullStagepName) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a stage with a null pName pointer");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.shader_stages_[0].pName = nullptr;
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-parameter");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelinePushConstantsNotInLayout) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming push constants which are not provided in the pipeline layout");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    /* should have generated an error -- no push constant ranges provided! */
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateComputePipelineMissingDescriptor) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a compute pipeline consuming a descriptor which is not provided in the pipeline "
        "layout");

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) buffer block { vec4 x; };
        void main(){
           x = vec4(1);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-layout-07988");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateComputePipelineDescriptorTypeMismatch) {
    TEST_DESCRIPTION("Test that an error is produced for a pipeline consuming a descriptor-backed resource of a mismatched type");

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) buffer block { vec4 x; };
        void main() {
           x.x = 1.0f;
        }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-layout-07989");
}

TEST_F(VkLayerTest, MultiplePushDescriptorSets) {
    TEST_DESCRIPTION("Verify an error message for multiple push descriptor sets.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_prop = LvlInitStruct<VkPhysicalDevicePushDescriptorPropertiesKHR>();
    GetPhysicalDeviceProperties2(push_descriptor_prop);
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        GTEST_SKIP() << "maxPushDescriptors is zero";
    }

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const unsigned int descriptor_set_layout_count = 2;
    std::vector<VkDescriptorSetLayoutObj> ds_layouts;
    for (uint32_t i = 0; i < descriptor_set_layout_count; ++i) {
        dsl_binding.binding = i;
        ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding),
                                VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    }
    const auto &ds_vk_layouts = MakeVkHandles<VkDescriptorSetLayout>(ds_layouts);

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = NULL;
    pipeline_layout_ci.setLayoutCount = ds_vk_layouts.size();
    pipeline_layout_ci.pSetLayouts = ds_vk_layouts.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00293");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AMDMixedAttachmentSamplesValidateGraphicsPipeline) {
    TEST_DESCRIPTION("Verify an error message for an incorrect graphics pipeline rasterization sample count.");

    AddRequiredExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Set a mismatched sample count
    VkPipelineMultisampleStateCreateInfo ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

    const auto set_info = [&](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_ = ms_state_ci; };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-subpass-01505");
}

TEST_F(VkLayerTest, FramebufferMixedSamplesNV) {
    TEST_DESCRIPTION("Verify VK_NV_framebuffer_mixed_samples.");
    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (VK_TRUE != device_features.sampleRateShading) {
        GTEST_SKIP() << "Test requires unsupported sampleRateShading feature";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

        VkRenderPassCreateInfo rpi = LvlInitStruct<VkRenderPassCreateInfo>();
        rpi.attachmentCount = 2;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;
        vk_testing::RenderPass rp(*m_device, rpi);

        auto ds = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
        auto cmi = LvlInitStruct<VkPipelineCoverageModulationStateCreateInfoNV>();

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

TEST_F(VkLayerTest, FramebufferMixedSamples) {
    TEST_DESCRIPTION("Verify that the expected VUIds are hits when VK_NV_framebuffer_mixed_samples is disabled.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

        VkRenderPassCreateInfo rpi = LvlInitStruct<VkRenderPassCreateInfo>();
        rpi.attachmentCount = 2;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        if (test_case.color_samples == test_case.depth_samples) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-pDepthStencilAttachment-01418");
        }
        vk_testing::RenderPass rp(*m_device, rpi);

        if (test_case.color_samples == test_case.depth_samples) {
        } else {
            m_errorMonitor->VerifyFound();
            continue;
        }

        VkPipelineDepthStencilStateCreateInfo ds = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

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

TEST_F(VkLayerTest, FramebufferMixedSamplesCoverageReduction) {
    TEST_DESCRIPTION("Verify VK_NV_coverage_reduction_mode.");

    AddRequiredExtensions(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!IsExtensionsEnabled(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME) &&
        !IsExtensionsEnabled(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        GTEST_SKIP() << "Extensions not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    auto vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
        reinterpret_cast<PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV>(
            vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV"));

    ASSERT_NO_FATAL_FAILURE(vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(gpu(), &combination_count, nullptr));
    if (combination_count < 1) {
        GTEST_SKIP() << "No mixed sample combinations are supported";
    }
    combinations.resize(combination_count);
    // TODO this fill can be removed once https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4138 merges
    std::fill(combinations.begin(), combinations.end(), LvlInitStruct<VkFramebufferMixedSamplesCombinationNV>());
    ASSERT_NO_FATAL_FAILURE(
        vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(gpu(), &combination_count, &combinations[0]));

    // Pick the first supported combination for a positive test.
    test_cases.push_back({combinations[0].rasterizationSamples, static_cast<VkSampleCountFlagBits>(combinations[0].colorSamples),
                          static_cast<VkSampleCountFlagBits>(combinations[0].depthStencilSamples),
                          combinations[0].coverageReductionMode, true,
                          "VUID-VkGraphicsPipelineCreateInfo-coverageReductionMode-02722"});

    VkSampleCountFlags fb_sample_counts = m_device->phy().properties().limits.framebufferDepthSampleCounts;
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

        VkRenderPassCreateInfo rpi = LvlInitStruct<VkRenderPassCreateInfo>();
        rpi.attachmentCount = (test_case.depth_samples) ? 2 : 1;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        vk_testing::RenderPass rp(*m_device, rpi);
        ASSERT_TRUE(rp.initialized());

        VkPipelineDepthStencilStateCreateInfo dss = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
        VkPipelineCoverageReductionStateCreateInfoNV crs = LvlInitStruct<VkPipelineCoverageReductionStateCreateInfoNV>();

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

TEST_F(VkLayerTest, FragmentCoverageToColorNV) {
    TEST_DESCRIPTION("Verify VK_NV_fragment_coverage_to_color.");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

        VkRenderPassCreateInfo rpi = LvlInitStruct<VkRenderPassCreateInfo>();
        rpi.attachmentCount = att.size();
        rpi.pAttachments = att.data();
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        const std::array<VkPipelineColorBlendAttachmentState, 3> cba = {{{}, {}, {}}};

        VkPipelineColorBlendStateCreateInfo cbi = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
        cbi.attachmentCount = cba.size();
        cbi.pAttachments = cba.data();

        vk_testing::RenderPass rp(*m_device, rpi);
        ASSERT_TRUE(rp.initialized());

        VkPipelineCoverageToColorStateCreateInfoNV cci = LvlInitStruct<VkPipelineCoverageToColorStateCreateInfoNV>();

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

TEST_F(VkLayerTest, ViewportSwizzleNV) {
    AddRequiredExtensions(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vp_swizzle_state = LvlInitStruct<VkPipelineViewportSwizzleStateCreateInfoNV>();

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

TEST_F(VkLayerTest, NonSemanticInfoEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_non_semantic_info.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_shader_non_semantic_info not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    const char *source = R"(
                   OpCapability Shader
                   OpExtension "SPV_KHR_non_semantic_info"
   %non_semantic = OpExtInstImport "NonSemantic.Validation.Test"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
              %1 = OpExtInst %void %non_semantic 55 %void
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
              %2 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GraphicsPipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test graphics pipeline feedback stage count check.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Driver data writeback check not supported by MockICD";
    }

    auto feedback_info = LvlInitStruct<VkPipelineCreationFeedbackCreateInfoEXT>();
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

TEST_F(VkLayerTest, ComputePipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test compute pipeline feedback stage count check.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineCreationFeedbackCreateInfoEXT feedback_info = LvlInitStruct<VkPipelineCreationFeedbackCreateInfoEXT>();
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

TEST_F(VkLayerTest, CreatePipelineCheckShaderImageFootprintEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the shader image footprint feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Disable the image footprint feature.
    auto image_footprint_features = LvlInitStruct<VkPhysicalDeviceShaderImageFootprintFeaturesNV>();
    image_footprint_features.imageFootprint = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &image_footprint_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_NV_shader_texture_footprint  : require
        layout(set=0, binding=0) uniform sampler2D s;
        layout(location=0) out vec4 color;
        void main(){
          gl_TextureFootprint2DNV footprint;
          if (textureFootprintNV(s, vec2(1.0), 5, false, footprint)) {
            color = vec4(0.0, 1.0, 0.0, 1.0);
          } else {
            color = vec4(vec2(footprint.anchor), vec2(footprint.offset));
          }
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj ds_layout(&test_device, {binding});
    ASSERT_TRUE(ds_layout.initialized());

    const VkPipelineLayoutObj pipeline_layout(&test_device, {&ds_layout});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckFragmentShaderBarycentricEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader barycentric feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto features = m_device->phy().features();

    // Disable the fragment shader barycentric feature.
    auto fragment_shader_barycentric_features = LvlInitStruct<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV>();
    fragment_shader_barycentric_features.fragmentShaderBarycentric = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), m_device_extension_names, &features, &fragment_shader_barycentric_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_NV_fragment_shader_barycentric : require
        layout(location=0) out float value;
        void main(){
          value = gl_BaryCoordNV.x;
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckComputeShaderDerivativesEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the compute shader derivatives feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Disable the compute shader derivatives features.
    auto compute_shader_derivatives_features = LvlInitStruct<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV>();
    compute_shader_derivatives_features.computeDerivativeGroupLinear = VK_FALSE;
    compute_shader_derivatives_features.computeDerivativeGroupQuads = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &compute_shader_derivatives_features);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(&test_device, {binding});
    const VkPipelineLayoutObj pl(&test_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_NV_compute_shader_derivatives : require
        layout(local_size_x=2, local_size_y=4) in;
        layout(derivative_group_quadsNV) in;
        layout(set=0, binding=0) buffer InputOutputBuffer {
          float values[];
        };
        void main(){
           values[gl_LocalInvocationIndex] = dFdx(values[gl_LocalInvocationIndex]);
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    cs.InitFromGLSLTry(false, &test_device);

    VkComputePipelineCreateInfo cpci = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                        nullptr,
                                        0,
                                        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
                                         VK_SHADER_STAGE_COMPUTE_BIT, cs.handle(), "main", nullptr},
                                        pl.handle(),
                                        VK_NULL_HANDLE,
                                        -1};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    VkPipeline pipe = VK_NULL_HANDLE;
    vk::CreateComputePipelines(test_device.device(), VK_NULL_HANDLE, 1, &cpci, nullptr, &pipe);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipeline(test_device.device(), pipe, nullptr);
}

TEST_F(VkLayerTest, CreatePipelineCheckFragmentShaderInterlockEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader interlock feature which has not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<const char *> device_extension_names;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME)) {
        // Note: we intentionally do not add the required extension to the device extension list.
        //       in order to create the error below
    } else {
        // We skip this test if the extension is not supported by the driver as in some cases this will cause
        // the vk::CreateShaderModule to fail without generating an error message
        printf("Extension %s is not supported.\n", VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);
        return;
    }

    auto features = m_device->phy().features();

    // Disable the fragment shader interlock feature.
    auto fragment_shader_interlock_features = LvlInitStruct<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>();
    fragment_shader_interlock_features.fragmentShaderSampleInterlock = VK_FALSE;
    fragment_shader_interlock_features.fragmentShaderPixelInterlock = VK_FALSE;
    fragment_shader_interlock_features.fragmentShaderShadingRateInterlock = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &fragment_shader_interlock_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_ARB_fragment_shader_interlock : require
        layout(sample_interlock_ordered) in;
        void main(){
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckDemoteToHelperInvocation) {
    TEST_DESCRIPTION("Create a pipeline requiring the demote to helper invocation feature which has not enabled on the device.");

    AddRequiredExtensions(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto features = m_device->phy().features();

    // Disable the demote to helper invocation feature.
    auto demote_features = LvlInitStruct<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>();
    demote_features.shaderDemoteToHelperInvocation = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), m_device_extension_names, &features, &demote_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_demote_to_helper_invocation : require
        void main(){
            demote;
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckLineRasterization) {
    TEST_DESCRIPTION("Test VK_EXT_line_rasterization state against feature enables.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto line_rasterization_features = LvlInitStruct<VkPhysicalDeviceLineRasterizationFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(line_rasterization_features);

    line_rasterization_features.rectangularLines = VK_FALSE;
    line_rasterization_features.bresenhamLines = VK_FALSE;
    line_rasterization_features.smoothLines = VK_FALSE;
    line_rasterization_features.stippledRectangularLines = VK_FALSE;
    line_rasterization_features.stippledBresenhamLines = VK_FALSE;
    line_rasterization_features.stippledSmoothLines = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    PFN_vkCmdSetLineStippleEXT vkCmdSetLineStippleEXT =
        (PFN_vkCmdSetLineStippleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLineStippleEXT");
    ASSERT_TRUE(vkCmdSetLineStippleEXT != nullptr);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776");
    vkCmdSetLineStippleEXT(m_commandBuffer->handle(), 0, 0);
    m_errorMonitor->VerifyFound();
    vkCmdSetLineStippleEXT(m_commandBuffer->handle(), 1, 1);
}

TEST_F(VkLayerTest, FillRectangleNV) {
    TEST_DESCRIPTION("Verify VK_NV_fill_rectangle");
    AddRequiredExtensions(VK_NV_FILL_RECTANGLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    // Disable non-solid fill modes to make sure that the usage of VK_POLYGON_MODE_LINE and
    // VK_POLYGON_MODE_POINT will cause an error when the VK_NV_fill_rectangle extension is enabled.
    device_features.fillModeNonSolid = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPolygonMode polygon_mode = VK_POLYGON_MODE_LINE;

    auto set_polygon_mode = [&polygon_mode](CreatePipelineHelper &helper) { helper.rs_state_ci_.polygonMode = polygon_mode; };

    // Set unsupported polygon mode VK_POLYGON_MODE_LINE
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507");

    // Set unsupported polygon mode VK_POLYGON_MODE_POINT
    polygon_mode = VK_POLYGON_MODE_POINT;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507");

    // Set supported polygon mode VK_POLYGON_MODE_FILL
    polygon_mode = VK_POLYGON_MODE_FILL;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit);

    // Set supported polygon mode VK_POLYGON_MODE_FILL_RECTANGLE_NV
    polygon_mode = VK_POLYGON_MODE_FILL_RECTANGLE_NV;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit);
}

TEST_F(VkLayerTest, NotCompatibleForSet) {
    TEST_DESCRIPTION("Check that validation path catches pipeline layout inconsistencies for bind vs. dispatch");
    ASSERT_NO_FATAL_FAILURE(Init());

    auto c_queue = m_device->GetDefaultComputeQueue();
    if (nullptr == c_queue) {
        printf("Compute not supported, skipping test\n");
        return;
    }

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 4;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj storage_buffer;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    storage_buffer.init(*m_device, bci, mem_props);

    VkBufferObj uniform_buffer;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 20;
    uniform_buffer.init(*m_device, bci, mem_props);

    OneOffDescriptorSet::Bindings binding_defs = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDescriptorSetLayoutObj pipeline_dsl(m_device, binding_defs);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&pipeline_dsl});

    // We now will use a slightly different Layout definition for the descriptors we acutally bind with (but that would still be
    // correct for the shader
    binding_defs[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    OneOffDescriptorSet binding_descriptor_set(m_device, binding_defs);
    const VkPipelineLayoutObj binding_pipeline_layout(m_device, {&binding_descriptor_set.layout_});

    VkDescriptorBufferInfo storage_buffer_info = {storage_buffer.handle(), 0, sizeof(uint32_t)};
    VkDescriptorBufferInfo uniform_buffer_info = {uniform_buffer.handle(), 0, 5 * sizeof(uint32_t)};

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = binding_descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &storage_buffer_info;

    descriptor_writes[1] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = binding_descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;  // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[1].pBufferInfo = &uniform_buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;
        layout(set = 0, binding = 1) uniform UniformStruct { ivec4 dummy; int val; } ubo;

        void main() {
            u_index.index = ubo.val;
        }
    )glsl";

    VkShaderObj shader_module(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo stage = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module.handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-02697");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyPipeline(device(), c_pipeline, nullptr);
}

TEST_F(VkLayerTest, PipelineStageConditionalRenderingWithWrongQueue) {
    TEST_DESCRIPTION("Run CmdPipelineBarrier with VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT and wrong VkQueueFlagBits");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto cond_rendering_feature = LvlInitStruct<VkPhysicalDeviceConditionalRenderingFeaturesEXT>();
    GetPhysicalDeviceFeatures2(cond_rendering_feature);
    if (cond_rendering_feature.conditionalRendering == VK_FALSE) {
        GTEST_SKIP() << "conditionalRendering feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &cond_rendering_feature));

    uint32_t only_transfer_queueFamilyIndex = vvl::kU32Max;

    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    for (uint32_t i = 0; i < (uint32_t)q_props.size(); i++) {
        if (q_props[i].queueFlags == VK_QUEUE_TRANSFER_BIT) {
            only_transfer_queueFamilyIndex = i;
            break;
        }
    }

    if (only_transfer_queueFamilyIndex == vvl::kU32Max) {
        GTEST_SKIP() << "Only VK_QUEUE_TRANSFER_BIT Queue is not supported";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkCommandPoolObj commandPool(m_device, only_transfer_queueFamilyIndex);
    VkCommandBufferObj commandBuffer(m_device, &commandPool);

    commandBuffer.begin();

    VkImageMemoryBarrier imb = LvlInitStruct<VkImageMemoryBarrier>();
    imb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    imb.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.image = image.handle();
    imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-06461");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstStageMask-06462");
    vk::CmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                           VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT, 0, 0, nullptr, 0, nullptr, 1, &imb);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(VkLayerTest, CreatePipelineDynamicUniformIndex) {
    TEST_DESCRIPTION("Check for the array dynamic array index features when the SPIR-V capabilities are requested.");

    VkPhysicalDeviceFeatures features{};
    features.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::string const source{R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)"};

    {
        std::string const capability{"OpCapability UniformBufferArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }

    {
        std::string const capability{"OpCapability SampledImageArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }

    {
        std::string const capability{"OpCapability StorageBufferArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }

    {
        std::string const capability{"OpCapability StorageImageArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }
}

TEST_F(VkLayerTest, PipelineMaxPerStageResources) {
    TEST_DESCRIPTION("Check case where pipeline is created that exceeds maxPerStageResources");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
    ASSERT_NO_FATAL_FAILURE(InitState());
    // Adds the one color attachment
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    const VkDescriptorSetLayoutObj ds_layout_normal(m_device, layout_bindings_normal);
    const VkDescriptorSetLayoutObj ds_layout_vert(m_device, layout_bindings_vert);
    const VkDescriptorSetLayoutObj ds_layout_frag(m_device, layout_bindings_frag);
    const VkDescriptorSetLayoutObj ds_layout_comp(m_device, layout_bindings_comp);
    const VkDescriptorSetLayoutObj ds_layout_combined0(m_device, layout_binding_combined0);
    const VkDescriptorSetLayoutObj ds_layout_combined1(m_device, layout_binding_combined1);

    CreateComputePipelineHelper compute_pipe(*this);
    compute_pipe.InitInfo();
    compute_pipe.InitShaderInfo();
    compute_pipe.InitState();
    compute_pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds_layout_comp});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-layout-01687");
    compute_pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    CreatePipelineHelper graphics_pipe(*this);
    graphics_pipe.InitInfo();
    graphics_pipe.InitShaderInfo();

    graphics_pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds_layout_normal});
    graphics_pipe.CreateGraphicsPipeline();

    graphics_pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds_layout_vert});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-01688");
    graphics_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    graphics_pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds_layout_frag});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-01688");
    graphics_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    graphics_pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds_layout_combined0, &ds_layout_combined1});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-01688");
    graphics_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidatePipelineExecutablePropertiesFeature) {
    TEST_DESCRIPTION("Try making calls without pipelineExecutableInfo.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_features =
        LvlInitStruct<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>();
    pipeline_exe_features.pipelineExecutableInfo = VK_FALSE;  // Starting with it off

    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&pipeline_exe_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // MockICD will return 0 for the executable count
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    PFN_vkGetPipelineExecutableInternalRepresentationsKHR vkGetPipelineExecutableInternalRepresentationsKHR =
        (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)vk::GetDeviceProcAddr(
            m_device->device(), "vkGetPipelineExecutableInternalRepresentationsKHR");
    PFN_vkGetPipelineExecutableStatisticsKHR vkGetPipelineExecutableStatisticsKHR =
        (PFN_vkGetPipelineExecutableStatisticsKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetPipelineExecutableStatisticsKHR");
    PFN_vkGetPipelineExecutablePropertiesKHR vkGetPipelineExecutablePropertiesKHR =
        (PFN_vkGetPipelineExecutablePropertiesKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetPipelineExecutablePropertiesKHR");
    ASSERT_TRUE(vkGetPipelineExecutableInternalRepresentationsKHR != nullptr);
    ASSERT_TRUE(vkGetPipelineExecutableStatisticsKHR != nullptr);
    ASSERT_TRUE(vkGetPipelineExecutablePropertiesKHR != nullptr);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    uint32_t count;
    VkPipelineExecutableInfoKHR pipeline_exe_info = LvlInitStruct<VkPipelineExecutableInfoKHR>();
    pipeline_exe_info.pipeline = pipe.pipeline_;
    pipeline_exe_info.executableIndex = 0;

    VkPipelineInfoKHR pipeline_info = LvlInitStruct<VkPipelineInfoKHR>();
    pipeline_info.pipeline = pipe.pipeline_;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipelineExecutableInfo-03276");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03278");
    vkGetPipelineExecutableInternalRepresentationsKHR(m_device->device(), &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutableStatisticsKHR-pipelineExecutableInfo-03272");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03274");
    vkGetPipelineExecutableStatisticsKHR(m_device->device(), &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineExecutablePropertiesKHR-pipelineExecutableInfo-03270");
    vkGetPipelineExecutablePropertiesKHR(m_device->device(), &pipeline_info, &count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, LimitsMaxSampleMaskWords) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords.");

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Set limit to match with hardcoded values in shaders
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxSampleMaskWords = 3;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Valid input of sample mask
    char const *validSource = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main(){
           int x = gl_SampleMaskIn[2];
           int y = gl_SampleMaskIn[0];
           uFragColor = vec4(0,1,0,1) * x * y;
        }
    )glsl";
    VkShaderObj fsValid(this, validSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto validPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fsValid.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, validPipeline, kErrorBit | kWarningBit);

    // Exceed sample mask input array size
    char const *inputSource = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main(){
           int x = gl_SampleMaskIn[3];
           uFragColor = vec4(0,1,0,1) * x;
        }
    )glsl";
    VkShaderObj fsInput(this, inputSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto inputPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fsInput.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, inputPipeline, kErrorBit,
                                      "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711");

    // Exceed sample mask output array size
    char const *outputSource = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main(){
           gl_SampleMask[3] = 1;
           uFragColor = vec4(0,1,0,1);
        }
    )glsl";
    VkShaderObj fsOutput(this, outputSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto outputPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fsOutput.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, outputPipeline, kErrorBit,
                                      "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711");
}

TEST_F(VkLayerTest, SampledInvalidImageViews) {
    TEST_DESCRIPTION("Test if an VkImageView is sampled at draw/dispatch that the format has valid format features enabled");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

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

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkRenderpassObj render_pass(m_device);

    VkPipelineObj pipeline_combined(m_device);
    pipeline_combined.AddDefaultColorAttachment();
    pipeline_combined.SetViewport(m_viewports);
    pipeline_combined.SetScissor(m_scissors);
    pipeline_combined.AddShader(&vs);
    VkPipelineObj pipeline_seperate(m_device);
    pipeline_seperate.AddDefaultColorAttachment();
    pipeline_seperate.SetViewport(m_viewports);
    pipeline_seperate.SetScissor(m_scissors);
    pipeline_seperate.AddShader(&vs);
    VkPipelineObj pipeline_unused(m_device);
    pipeline_unused.AddDefaultColorAttachment();
    pipeline_unused.SetViewport(m_viewports);
    pipeline_unused.SetScissor(m_scissors);
    pipeline_unused.AddShader(&vs);
    VkPipelineObj pipeline_function(m_device);
    pipeline_function.AddDefaultColorAttachment();
    pipeline_function.SetViewport(m_viewports);
    pipeline_function.SetScissor(m_scissors);
    pipeline_function.AddShader(&vs);

    // 4 different pipelines for 4 different shaders
    // 3 are invalid and 1 (pipeline_unused) is valid
    pipeline_combined.AddShader(&fs_combined);
    pipeline_seperate.AddShader(&fs_seperate);
    pipeline_unused.AddShader(&fs_unused);
    pipeline_function.AddShader(&fs_function);

    OneOffDescriptorSet::Bindings combined_bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    OneOffDescriptorSet::Bindings seperate_bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    OneOffDescriptorSet combined_descriptor_set(m_device, combined_bindings);
    OneOffDescriptorSet seperate_descriptor_set(m_device, seperate_bindings);
    const VkPipelineLayoutObj combined_pipeline_layout(m_device, {&combined_descriptor_set.layout_});
    const VkPipelineLayoutObj seperate_pipeline_layout(m_device, {&seperate_descriptor_set.layout_});

    pipeline_combined.CreateVKPipeline(combined_pipeline_layout.handle(), render_pass.handle());
    pipeline_seperate.CreateVKPipeline(seperate_pipeline_layout.handle(), render_pass.handle());
    pipeline_unused.CreateVKPipeline(combined_pipeline_layout.handle(), render_pass.handle());
    pipeline_function.CreateVKPipeline(combined_pipeline_layout.handle(), render_pass.handle());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.minFilter = VK_FILTER_LINEAR;  // turned off feature bit for test
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.compareEnable = VK_FALSE;
    vk_testing::Sampler sampler_filter(*m_device, sampler_ci);

    sampler_ci.minFilter = VK_FILTER_NEAREST;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;  // turned off feature bit for test
    vk_testing::Sampler sampler_mipmap(*m_device, sampler_ci);

    VkDescriptorImageInfo combined_sampler_info = {sampler_filter.handle(), imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo seperate_sampled_image_info = {VK_NULL_HANDLE, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo seperate_sampler_info = {sampler_filter.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

    // first item is combined, second/third item are seperate
    VkWriteDescriptorSet descriptor_writes[3] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = combined_descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &combined_sampler_info;

    descriptor_writes[1] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = seperate_descriptor_set.set_;
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_writes[1].pImageInfo = &seperate_sampled_image_info;

    descriptor_writes[2] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[2].dstSet = seperate_descriptor_set.set_;
    descriptor_writes[2].dstBinding = 1;
    descriptor_writes[2].descriptorCount = 1;
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_writes[2].pImageInfo = &seperate_sampler_info;

    vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, nullptr);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Unused is a valid version of the combined pipeline/descriptors
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_unused.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, combined_pipeline_layout.handle(), 0, 1,
                              &combined_descriptor_set.set_, 0, nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);

    // Test magFilter
    {
        // Same descriptor set as combined test
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_function.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Draw with invalid combined image sampler
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_combined.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Same error, but not with seperate descriptors
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_seperate.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, seperate_pipeline_layout.handle(), 0,
                                  1, &seperate_descriptor_set.set_, 0, nullptr);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
        m_commandBuffer->Draw(1, 0, 0, 0);
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
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_function.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-mipmapMode-04770");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Draw with invalid combined image sampler
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_combined.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-mipmapMode-04770");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // Same error, but not with seperate descriptors
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_seperate.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, seperate_pipeline_layout.handle(), 0,
                                  1, &seperate_descriptor_set.set_, 0, nullptr);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-mipmapMode-04770");
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ShaderDrawParametersNotEnabled10) {
    TEST_DESCRIPTION("Validation using DrawParameters for Vulkan 1.0 without the shaderDrawParameters feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
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

TEST_F(VkLayerTest, ShaderDrawParametersNotEnabled11) {
    TEST_DESCRIPTION("Validation using DrawParameters for Vulkan 1.1 without the shaderDrawParameters feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

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

TEST_F(VkLayerTest, ShaderFloatControl) {
    TEST_DESCRIPTION("Test VK_KHR_shader_float_controls");

    // Need 1.1 to get SPIR-V 1.3 since OpExecutionModeId was added in SPIR-V 1.2
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    // The issue with revision 4 of this extension should not be an issue with the tests
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto shader_float_control = LvlInitStruct<VkPhysicalDeviceFloatControlsProperties>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&shader_float_control);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    if (shader_float_control.denormBehaviorIndependence == VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        GTEST_SKIP() << "denormBehaviorIndependence is NONE";
    }
    if (shader_float_control.roundingModeIndependence == VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        GTEST_SKIP() << "roundingModeIndependence is NONE";
    }

    // Check for support of 32-bit properties, but only will test if they are not supported
    // in case all 16/32/64 version are not supported will set SetUnexpectedError for capability check
    bool signed_zero_inf_nan_preserve = (shader_float_control.shaderSignedZeroInfNanPreserveFloat32 == VK_TRUE);
    bool denorm_preserve = (shader_float_control.shaderDenormPreserveFloat32 == VK_TRUE);
    bool denorm_flush_to_zero = (shader_float_control.shaderDenormFlushToZeroFloat32 == VK_TRUE);
    bool rounding_mode_rte = (shader_float_control.shaderRoundingModeRTEFloat32 == VK_TRUE);
    bool rounding_mode_rtz = (shader_float_control.shaderRoundingModeRTZFloat32 == VK_TRUE);

    // same body for each shader, only the start is different
    // this is just "float a = 1.0 + 2.0;" in SPIR-V
    const std::string source_body = R"(
             OpExecutionMode %main LocalSize 1 1 1
             OpSource GLSL 450
             OpName %main "main"
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %float = OpTypeFloat 32
%pFunction = OpTypePointer Function %float
  %float_3 = OpConstant %float 3
     %main = OpFunction %void None %3
        %5 = OpLabel
        %6 = OpVariable %pFunction Function
             OpStore %6 %float_3
             OpReturn
             OpFunctionEnd
)";

    if (!signed_zero_inf_nan_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability SignedZeroInfNanPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main SignedZeroInfNanPreserve 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(
                new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM));
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294");
    }

    if (!denorm_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormPreserve 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(
                new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM));
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297");
    }

    if (!denorm_flush_to_zero) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormFlushToZero
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormFlushToZero 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(
                new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM));
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300");
    }

    if (!rounding_mode_rte) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTE
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTE 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(
                new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM));
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303");
    }

    if (!rounding_mode_rtz) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTZ
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTZ 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(
                new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM));
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306");
    }
}

TEST_F(VkLayerTest, Storage8and16bitCapability) {
    TEST_DESCRIPTION("Test VK_KHR_8bit_storage and VK_KHR_16bit_storage not having feature bits required for SPIR-V capability");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Prevent extra errors for not having the support for the SPV extensions
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Need to explicitly turn off shaderInt16 as test will try to add and easier if all test have off
    VkPhysicalDeviceFeatures features = {};
    features.shaderInt16 = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // storageBuffer8BitAccess
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(set = 0, binding = 0) buffer SSBO { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-storageBuffer8BitAccess-06328",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",        // Int8
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // StorageBuffer8BitAccess
        }
    }
    // uniformAndStorageBuffer8BitAccess
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(set = 0, binding = 0) uniform UBO { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",                  // Int8
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // UniformAndStorageBuffer8BitAccess
        }
    }

    // storagePushConstant8
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(push_constant) uniform PushConstant { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vector<string>{"VUID-RuntimeSpirv-storagePushConstant8-06330",  // feature
                                                             "VUID-VkShaderModuleCreateInfo-pCode-08740",     // Int8
                                                             "VUID-VkShaderModuleCreateInfo-pCode-08740"});  // StoragePushConstant8
        }
    }

    // storageBuffer16BitAccess - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(set = 0, binding = 0) buffer SSBO { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",         // Float16
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // StorageBuffer16BitAccess
        }
    }

    // uniformAndStorageBuffer16BitAccess - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(set = 0, binding = 0) uniform UBO { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",                   // Float16
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // UniformAndStorageBuffer16BitAccess
        }
    }

    // storagePushConstant16 - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(push_constant) uniform PushConstant { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storagePushConstant16-06333",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",      // Float16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"       // StoragePushConstant16
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storageInputOutput16 - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(location = 0) out float16_t outData;
            void main(){
               outData = float16_t(1);
               gl_Position = vec4(0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        // Need to match in/out
        char const *fsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(location = 0) in float16_t x;
            layout(location = 0) out vec4 uFragColor;
            void main(){
               uFragColor = vec4(0,1,0,1);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if ((VK_SUCCESS == vs.InitFromGLSLTry()) && (VK_SUCCESS == fs.InitFromGLSLTry())) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature frag
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // Float16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // StorageInputOutput16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740"      // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storageBuffer16BitAccess - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(set = 0, binding = 0) buffer SSBO { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",         // Int16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"          // StorageBuffer16BitAccess
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // uniformAndStorageBuffer16BitAccess - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(set = 0, binding = 0) uniform UBO { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",                   // Int16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"                    // UniformAndStorageBuffer16BitAccess
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storagePushConstant16 - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(push_constant) uniform PushConstant { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storagePushConstant16-06333",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",      // Int16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"       // StoragePushConstant16
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storageInputOutput16 - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(location = 0) out int16_t outData;
            void main(){
               outData = int16_t(1);
               gl_Position = vec4(0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        // Need to match in/out
        char const *fsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(location = 0) flat in int16_t x;
            layout(location = 0) out vec4 uFragColor;
            void main(){
               uFragColor = vec4(0,1,0,1);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
        if ((VK_SUCCESS == vs.InitFromGLSLTry()) && (VK_SUCCESS == fs.InitFromGLSLTry())) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature frag
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // Int16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // StorageInputOutput16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740"      // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }
}

TEST_F(VkLayerTest, Storage8and16bitFeatures) {
    TEST_DESCRIPTION(
        "Test VK_KHR_8bit_storage and VK_KHR_16bit_storage where the Int8/Int16 capability are only used and since they are "
        "superset of a capabilty");

    // the following [OpCapability UniformAndStorageBuffer8BitAccess] requires the uniformAndStorageBuffer8BitAccess feature bit or
    // the generated capability checking code will catch it
    //
    // But having just [OpCapability Int8] is still a legal SPIR-V shader because the Int8 capabilty allows all storage classes in
    // the SPIR-V spec... but the shaderInt8 feature bit in Vulkan spec explains how you still need the
    // uniformAndStorageBuffer8BitAccess feature bit for Uniform storage class from Vulkan's perspective

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Prevent extra errors for not having the support for the SPV extensions
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16Int8 = LvlInitStruct<VkPhysicalDeviceShaderFloat16Int8Features>();
    auto features2 = GetPhysicalDeviceFeatures2(float16Int8);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (float16Int8.shaderInt8 == VK_TRUE) {
        // storageBuffer8BitAccess
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storageBuffer8BitAccess-06328");
        }

        // uniformAndStorageBuffer8BitAccess
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              "VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329");
        }

        // storagePushConstant8
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storagePushConstant8-06330");
        }
    }

    if (float16Int8.shaderFloat16 == VK_TRUE) {
        // storageBuffer16BitAccess - float
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");
        }

        // uniformAndStorageBuffer16BitAccess - float
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");
        }

        // storagePushConstant16 - float
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storagePushConstant16-06333");
        }

        // storageInputOutput16 - float
        {
            const std::string vs_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %var
               OpDecorate %var Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
        %ptr = OpTypePointer Output %float16
        %var = OpVariable %ptr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, vs_source, "main", nullptr);

            const std::string fs_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in %out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %in Location 0
               OpDecorate %out Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
      %inPtr = OpTypePointer Input %float16
         %in = OpVariable %inPtr Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %outPtr = OpTypePointer Output %v4float
        %out = OpVariable %outPtr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto fs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_FRAGMENT_BIT, fs_source, "main", nullptr);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // StorageInputOutput16 vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334"   // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    if (features2.features.shaderInt16 == VK_TRUE) {
        // storageBuffer16BitAccess - int
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");
        }

        // uniformAndStorageBuffer16BitAccess - int
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");
        }

        // storagePushConstant16 - int
        {
            const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storagePushConstant16-06333");
        }

        // storageInputOutput16 - int
        {
            const std::string vs_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %var
               OpDecorate %var Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
        %ptr = OpTypePointer Output %int16
        %var = OpVariable %ptr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, vs_source, "main", nullptr);

            const std::string fs_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in %out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %in Location 0
               OpDecorate %in Flat
               OpDecorate %out Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
      %inPtr = OpTypePointer Input %int16
         %in = OpVariable %inPtr Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %outPtr = OpTypePointer Output %v4float
        %out = OpVariable %outPtr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto fs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_FRAGMENT_BIT, fs_source, "main", nullptr);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // StorageInputOutput16 vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334"   // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // tests struct with multiple types
    if (float16Int8.shaderInt8 == VK_TRUE && features2.features.shaderInt16 == VK_TRUE) {
        // struct X {
        //   u16vec2 a;
        // };
        // struct {
        //   uint a;
        //   X b;
        //   uint8_t c;
        // } Data;
        const std::string spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpCapability Int16
               OpExtension "SPV_KHR_8bit_storage"
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %X 0 Offset 0
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 4
               OpMemberDecorate %Data 2 Offset 8
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
      %int16 = OpTypeInt 16 0
    %v2int16 = OpTypeVector %int16 2
      %int32 = OpTypeInt 32 0
          %X = OpTypeStruct %v2int16
       %Data = OpTypeStruct %int32 %X %int8
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
        auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr);
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
            helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                          vector<string>{"VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",    // 16 bit var
                                                         "VUID-RuntimeSpirv-storageBuffer8BitAccess-06328 "});  // 8 bit var
    }
}

TEST_F(VkLayerTest, WorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION("Test VK_KHR_workgroup_memory_explicit_layout");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto float16int8_features = LvlInitStruct<VkPhysicalDeviceShaderFloat16Int8Features>();
    auto features2 = GetPhysicalDeviceFeatures2(float16int8_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const bool support_8_bit = (float16int8_features.shaderInt8 == VK_TRUE);
    const bool support_16_bit = (float16int8_features.shaderFloat16 == VK_TRUE) && (features2.features.shaderInt16 == VK_TRUE);

    // WorkgroupMemoryExplicitLayoutKHR
    {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 8 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %first = OpTypeStruct %int
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Workgroup_int %_ %int_0
               OpStore %13 %int_2
               OpReturn
               OpFunctionEnd
        )";

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(new VkShaderObj(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
        };
        // Both missing enabling the extension and capability feature
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742"});
    }

    // WorkgroupMemoryExplicitLayout8BitAccessKHR
    if (support_8_bit) {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpCapability WorkgroupMemoryExplicitLayout8BitAccessKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 2 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %char = OpTypeInt 8 1
      %first = OpTypeStruct %char
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %char_2 = OpConstant %char 2
%_ptr_Workgroup_char = OpTypePointer Workgroup %char
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_Workgroup_char %_ %int_0
               OpStore %14 %char_2
               OpReturn
               OpFunctionEnd
        )";

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(new VkShaderObj(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
        };
        // Both missing enabling the extension and capability feature
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742"});
    }

    // WorkgroupMemoryExplicitLayout16BitAccessKHR
    if (support_16_bit) {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpCapability Int16
               OpCapability WorkgroupMemoryExplicitLayout16BitAccessKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 2 1 1
               OpMemberDecorate %first 0 Offset 0
               OpMemberDecorate %first 1 Offset 2
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %short = OpTypeInt 16 1
       %half = OpTypeFloat 16
      %first = OpTypeStruct %short %half
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %short_3 = OpConstant %short 3
%_ptr_Workgroup_short = OpTypePointer Workgroup %short
      %int_1 = OpConstant %int 1
%half_0x1_898p_3 = OpConstant %half 0x1.898p+3
%_ptr_Workgroup_half = OpTypePointer Workgroup %half
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpAccessChain %_ptr_Workgroup_short %_ %int_0
               OpStore %15 %short_3
         %19 = OpAccessChain %_ptr_Workgroup_half %_ %int_1
               OpStore %19 %half_0x1_898p_3
               OpReturn
               OpFunctionEnd
        )";

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_.reset(new VkShaderObj(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
        };
        // Both missing enabling the extension and capability feature
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742"});
    }

    // workgroupMemoryExplicitLayoutScalarBlockLayout feature
    // will fail from not passing --workgroup-scalar-block-layout in spirv-val
    {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %B
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %S 2 Offset 16
               OpMemberDecorate %S 3 Offset 28
               OpDecorate %S Block
               OpDecorate %B Aliased
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float %v3float %v3float
%_ptr_Workgroup_S = OpTypePointer Workgroup %S
          %B = OpVariable %_ptr_Workgroup_S Workgroup
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01379");
        VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ReadShaderClock) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Don't enable either feature bit on purpose
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Device scope using GL_EXT_shader_realtime_clock
    char const *vsSourceDevice = R"glsl(
        #version 450
        #extension GL_EXT_shader_realtime_clock: enable
        void main(){
           uvec2 a = clockRealtime2x32EXT();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    VkShaderObj vs_device(this, vsSourceDevice, VK_SHADER_STAGE_VERTEX_BIT);

    // Subgroup scope using ARB_shader_clock
    char const *vsSourceScope = R"glsl(
        #version 450
        #extension GL_ARB_shader_clock: enable
        void main(){
           uvec2 a = clock2x32ARB();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    VkShaderObj vs_subgroup(this, vsSourceScope, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info_device = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs_device.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info_device, kErrorBit, "VUID-RuntimeSpirv-shaderDeviceClock-06268");

    const auto set_info_subgroup = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs_subgroup.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info_subgroup, kErrorBit, "VUID-RuntimeSpirv-shaderSubgroupClock-06267");
}

TEST_F(VkLayerTest, GraphicsPipelineInvalidFlags) {
    TEST_DESCRIPTION("Create a graphics pipeline with invalid VkPipelineCreateFlags.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineCreateFlags flags;
    const auto set_info = [&](CreatePipelineHelper &helper) { helper.gp_ci_.flags = flags; };

    flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-00764");
    flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-03371");
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

TEST_F(VkLayerTest, ComputePipelineInvalidFlags) {
    TEST_DESCRIPTION("Create a compute pipeline with invalid VkPipelineCreateFlags.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPipelineCreateFlags flags;
    const auto set_info = [&](CreateComputePipelineHelper &helper) { helper.cp_ci_.flags = flags; };

    flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-03364");
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
    flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-02874");
}

TEST_F(VkLayerTest, SpecializationInvalidSizeZero) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is 0");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char *cs_src = R"glsl(
        #version 450
        layout (constant_id = 0) const int c = 3;
        layout (local_size_x = 1) in;
        void main() {
            if (gl_GlobalInvocationID.x >= c) { return; }
        }
    )glsl";

    // Set the specialization constant size to 0 (anything other than 1, 2, 4, or 8 will produce the expected error).
    VkSpecializationMapEntry entry = {
        0,  // id
        0,  // offset
        0,  // size
    };
    int32_t data = 0;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(decltype(data)),
        &data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    entry.size = sizeof(decltype(data));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkLayerTest, MergePipelineCachesInvalidDst) {
    TEST_DESCRIPTION("Test mergeing pipeline caches with dst cache in src list");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper other_pipe(*this);
    other_pipe.InitInfo();
    other_pipe.InitState();
    other_pipe.CreateGraphicsPipeline();

    VkPipelineCache dstCache = pipe.pipeline_cache_;
    VkPipelineCache srcCaches[2] = {other_pipe.pipeline_cache_, pipe.pipeline_cache_};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMergePipelineCaches-dstCache-00770");
    vk::MergePipelineCaches(m_device->device(), dstCache, 2, srcCaches);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateComputesPipelineWithBadBasePointer) {
    TEST_DESCRIPTION("Create Compute Pipeline with bad base pointer");

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=2, local_size_y=4) in;
        void main(){
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj pipeline_dsl(m_device, bindings);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = LvlInitStruct<VkComputePipelineCreateInfo>();
    compute_create_info.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    vk_testing::Pipeline test_pipeline(*m_device, compute_create_info);

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
        vk_testing::Pipeline pipeline(*m_device, compute_create_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, CreatePipelineWithDuplicatedSpecializationConstantID) {
    TEST_DESCRIPTION("Create a pipeline with non unique constantID in specialization pMapEntries.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkSpecializationMapEntry entries[2];
    entries[0].constantID = 0;
    entries[0].offset = 0;
    entries[0].size = sizeof(uint32_t);
    entries[1].constantID = 0;
    entries[1].offset = 0;
    entries[1].size = sizeof(uint32_t);

    uint32_t data = 1;
    VkSpecializationInfo specialization_info;
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-constantID-04911");
}

TEST_F(VkLayerTest, PipelineInvalidAdvancedBlend) {
    TEST_DESCRIPTION("Create a graphics pipeline with advanced blend when its disabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto blend_operation_advanced = LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
    GetPhysicalDeviceProperties2(blend_operation_advanced);

    if (blend_operation_advanced.advancedBlendAllOperations) {
        GTEST_SKIP() << "advancedBlendAllOperations is VK_TRUE, test needs it not supported.";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();

    VkPipelineColorBlendAttachmentState attachment_state = {};
    attachment_state.blendEnable = VK_TRUE;
    attachment_state.colorBlendOp = VK_BLEND_OP_XOR_EXT;
    attachment_state.alphaBlendOp = VK_BLEND_OP_XOR_EXT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &attachment_state;
    pipe.gp_ci_.pColorBlendState = &color_blend_state;

    pipe.InitState();
    // When using profiles, advancedBlendMaxColorAttachments might be zero
    m_errorMonitor->SetUnexpectedError("VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendAllOperations-01409");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineAdvancedBlendInvalidBlendOps) {
    TEST_DESCRIPTION("Advanced blending with invalid VkBlendOps");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(2));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto blend_operation_advanced = LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
    GetPhysicalDeviceProperties2(blend_operation_advanced);

    if (!blend_operation_advanced.advancedBlendAllOperations) {
        GTEST_SKIP() << "advancedBlendAllOperations is not supported.";
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    VkPipelineColorBlendAttachmentState attachment_states[2];
    memset(attachment_states, 0, sizeof(VkPipelineColorBlendAttachmentState) * 2);

    // only 1 attachment state, different blend op values
    const auto set_info_different = [&](CreatePipelineHelper &helper) {
        attachment_states[0].blendEnable = VK_TRUE;
        attachment_states[0].colorBlendOp = VK_BLEND_OP_HSL_COLOR_EXT;
        attachment_states[0].alphaBlendOp = VK_BLEND_OP_MULTIPLY_EXT;

        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = attachment_states;
        helper.gp_ci_.pColorBlendState = &color_blend_state;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info_different, kErrorBit,
                                      "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01406");

    // Test is if independent blend is not supported
    if (!blend_operation_advanced.advancedBlendIndependentBlend && blend_operation_advanced.advancedBlendMaxColorAttachments > 1) {
        const auto set_info_color = [&](CreatePipelineHelper &helper) {
            attachment_states[0].blendEnable = VK_TRUE;
            attachment_states[0].colorBlendOp = VK_BLEND_OP_MIN;
            attachment_states[0].alphaBlendOp = VK_BLEND_OP_MIN;
            attachment_states[1].blendEnable = VK_TRUE;
            attachment_states[1].colorBlendOp = VK_BLEND_OP_MULTIPLY_EXT;
            attachment_states[1].alphaBlendOp = VK_BLEND_OP_MULTIPLY_EXT;

            color_blend_state.attachmentCount = 2;
            color_blend_state.pAttachments = attachment_states;
            helper.gp_ci_.pColorBlendState = &color_blend_state;
        };
        constexpr std::array vuids = {"VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01407",
                                      "VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01408"};
        CreatePipelineHelper::OneshotTest(*this, set_info_color, kErrorBit, vuids);
    }
}

TEST_F(VkLayerTest, PipelineAdvancedBlendMaxBlendAttachment) {
    TEST_DESCRIPTION("Advanced blending with invalid VkBlendOps");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(3));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto blend_operation_advanced_props = LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
    GetPhysicalDeviceProperties2(blend_operation_advanced_props);
    if (blend_operation_advanced_props.advancedBlendMaxColorAttachments > 2) {
        GTEST_SKIP() << "advancedBlendMaxColorAttachments is too high";
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    VkPipelineColorBlendAttachmentState attachment_states[3];
    memset(attachment_states, 0, sizeof(VkPipelineColorBlendAttachmentState) * 3);

    // over max blend color attachment count
    const auto set_info = [&](CreatePipelineHelper &helper) {
        attachment_states[0].blendEnable = VK_TRUE;
        attachment_states[0].colorBlendOp = VK_BLEND_OP_MULTIPLY_EXT;
        attachment_states[0].alphaBlendOp = VK_BLEND_OP_MULTIPLY_EXT;
        attachment_states[1] = attachment_states[0];
        attachment_states[2] = attachment_states[0];

        color_blend_state.attachmentCount = 3;
        color_blend_state.pAttachments = attachment_states;
        helper.gp_ci_.pColorBlendState = &color_blend_state;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410");
}

TEST_F(VkLayerTest, InvalidPipelineDiscardRectangle) {
    TEST_DESCRIPTION("Create a graphics pipeline invalid VkPipelineDiscardRectangleStateCreateInfoEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties =
        LvlInitStruct<VkPhysicalDeviceDiscardRectanglePropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();
    phys_dev_props_2.pNext = &discard_rectangle_properties;
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    uint32_t count = discard_rectangle_properties.maxDiscardRectangles + 1;
    std::vector<VkRect2D> discard_rectangles(count);

    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rectangle_state =
        LvlInitStruct<VkPipelineDiscardRectangleStateCreateInfoEXT>();
    discard_rectangle_state.discardRectangleCount = count;
    discard_rectangle_state.pDiscardRectangles = discard_rectangles.data();

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &discard_rectangle_state;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkPipelineDiscardRectangleStateCreateInfoEXT-discardRectangleCount-00582");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineColorWriteCreateInfoEXT) {
    TEST_DESCRIPTION("Test VkPipelineColorWriteCreateInfoEXT in color blend state pNext");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineColorWriteCreateInfoEXT color_write = LvlInitStruct<VkPipelineColorWriteCreateInfoEXT>();

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.cb_ci_.pNext = &color_write;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-04802");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkBool32 enabled = VK_FALSE;
    color_write.attachmentCount = 1;
    color_write.pColorWriteEnables = &enabled;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-pAttachments-04801");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ColorBlendAdvanced) {
    TEST_DESCRIPTION("Test VkPipelineColorBlendAdvancedStateCreateInfoEXT with unsupported properties");

    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto blend_operation_advanced_props = LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
    GetPhysicalDeviceProperties2(blend_operation_advanced_props);

    if (blend_operation_advanced_props.advancedBlendCorrelatedOverlap &&
        blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor &&
        blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
        GTEST_SKIP() << "All VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT properties are enabled; nothing to test";
    }

    auto color_blend_advanced = LvlInitStruct<VkPipelineColorBlendAdvancedStateCreateInfoEXT>();
    color_blend_advanced.blendOverlap = VK_BLEND_OVERLAP_DISJOINT_EXT;
    color_blend_advanced.dstPremultiplied = VK_FALSE;
    color_blend_advanced.srcPremultiplied = VK_FALSE;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.cb_ci_.pNext = &color_blend_advanced;
    if (!blend_operation_advanced_props.advancedBlendCorrelatedOverlap) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-blendOverlap-01426");
    }
    if (!blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-dstPremultiplied-01425");
    }
    if (!blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-srcPremultiplied-01424");
    }
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateVariableSampleLocations) {
    TEST_DESCRIPTION("Validate using VkPhysicalDeviceSampleLocationsPropertiesEXT");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations = LvlInitStruct<VkPhysicalDeviceSampleLocationsPropertiesEXT>();
    VkPhysicalDeviceProperties2 phys_props = LvlInitStruct<VkPhysicalDeviceProperties2>(&sample_locations);
    GetPhysicalDeviceProperties2(phys_props);

    if (sample_locations.variableSampleLocations) {
        GTEST_SKIP() << "VkPhysicalDeviceSampleLocationsPropertiesEXT::variableSampleLocations is supported, skipping.";
    }

    PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT vkGetPhysicalDeviceMultisamplePropertiesEXT =
        (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vk::GetInstanceProcAddr(instance(),
                                                                                 "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    assert(vkGetPhysicalDeviceMultisamplePropertiesEXT != nullptr);

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

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &subpass_dependency;

    vk_testing::RenderPass render_pass(*m_device, rpci);
    ASSERT_TRUE(render_pass.initialized());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo framebuffer_info = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_info.renderPass = render_pass.handle();
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &image_view;
    framebuffer_info.width = 32;
    framebuffer_info.height = 32;
    framebuffer_info.layers = 1;

    vk_testing::Framebuffer framebuffer(*m_device, framebuffer_info);
    ASSERT_TRUE(framebuffer.initialized());

    auto multisample_prop = LvlInitStruct<VkMultisamplePropertiesEXT>();
    vkGetPhysicalDeviceMultisamplePropertiesEXT(gpu(), VK_SAMPLE_COUNT_1_BIT, &multisample_prop);
    const uint32_t valid_count =
        multisample_prop.maxSampleLocationGridSize.width * multisample_prop.maxSampleLocationGridSize.height;

    if (valid_count == 0) {
        printf("multisample properties are not supported.\n");
        return;
    }

    std::vector<VkSampleLocationEXT> sample_location(valid_count, {0.5, 0.5});
    VkSampleLocationsInfoEXT sample_locations_info = LvlInitStruct<VkSampleLocationsInfoEXT>();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = multisample_prop.maxSampleLocationGridSize;
    sample_locations_info.sampleLocationsCount = valid_count;
    sample_locations_info.pSampleLocations = sample_location.data();

    VkPipelineSampleLocationsStateCreateInfoEXT sample_locations_state =
        LvlInitStruct<VkPipelineSampleLocationsStateCreateInfoEXT>();
    sample_locations_state.sampleLocationsEnable = VK_TRUE;
    sample_locations_state.sampleLocationsInfo = sample_locations_info;

    auto multi_sample_state = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>(&sample_locations_state);
    multi_sample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multi_sample_state.sampleShadingEnable = VK_FALSE;
    multi_sample_state.minSampleShading = 1.0;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
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

    VkRenderPassSampleLocationsBeginInfoEXT render_pass_sample_locations = LvlInitStruct<VkRenderPassSampleLocationsBeginInfoEXT>();
    render_pass_sample_locations.attachmentInitialSampleLocationsCount = 1;
    render_pass_sample_locations.pAttachmentInitialSampleLocations = &attachment_sample_locations;
    render_pass_sample_locations.postSubpassSampleLocationsCount = 1;
    render_pass_sample_locations.pPostSubpassSampleLocations = &subpass_sample_locations;

    sample_location[0].x =
        0.0f;  // Invalid, VkRenderPassSampleLocationsBeginInfoEXT wont match VkPipelineSampleLocationsStateCreateInfoEXT

    VkRenderPassBeginInfo begin_info = LvlInitStruct<VkRenderPassBeginInfo>(&render_pass_sample_locations);
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

    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    sample_location[0].x = 0.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-variableSampleLocations-01525");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(m_commandBuffer->handle());

    begin_info.pNext = nullptr;  // Invalid, missing VkRenderPassSampleLocationsBeginInfoEXT
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-variableSampleLocations-01525");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->VerifyFound();
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(m_commandBuffer->handle());

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ComputeSharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints + 16);
    csSource << R"glsl(];
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ComputeSharedMemoryBooleanOverLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize with booleans");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    // "Boolean values considered as 32-bit integer values for the purpose of this calculation."
    const uint32_t max_shared_bools = max_shared_memory_size / 4;

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"glsl(
        #version 450
        shared bool a[)glsl";
    csSource << (max_shared_bools + 16);
    csSource << R"glsl(];
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ComputeSharedMemoryOverLimitWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto explicit_layout_features = LvlInitStruct<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(explicit_layout_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        GTEST_SKIP() << "workgroupMemoryExplicitLayout feature not supported";
    }

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        shared X {
            int x;
        };

        shared Y {
            int y1[)glsl";
    csSource << (max_shared_ints + 16);
    csSource << R"glsl(];
            int y2;
        };

        void main() {
            x = 0; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2));
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ComputeSharedMemorySpecConstantDefault) {
    TEST_DESCRIPTION("Validate shared memory exceed maxComputeSharedMemorySize limit with spec constants default");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(constant_id = 0) const uint Condition = 1;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    cs_source << (max_shared_ints + 16);
    cs_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
}

TEST_F(VkLayerTest, ComputeSharedMemorySpecConstantSet) {
    TEST_DESCRIPTION("Validate shared memory exceed maxComputeSharedMemorySize limit with spec constants set");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(constant_id = 0) const uint Condition = 0;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    cs_source << (max_shared_ints + 16);
    cs_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    uint32_t data = 1;  // set Condition

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                         SPV_SOURCE_GLSL, &specialization_info));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
}

// Spec doesn't clarify if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/3293
TEST_F(VkLayerTest, DISABLED_TestInvalidShaderInputAndOutputComponents) {
    TEST_DESCRIPTION("Test invalid shader layout in and out with different components.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out float r;
                layout(location = 0, component = 2) out float b;

                void main() {
                    r = 0.25f;
                    b = 0.75f;
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0) in vec3 rgb;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(rgb, 1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) in float a;
                layout(location = 0, component = 2) in float b;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit,
                                          "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                    v = vec3(1.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0) in vec4 v;

                layout (location = 0) out vec4 color;

                void main() {
                    color = v;
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                    v = vec3(1.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(1.0);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v1;
                layout(location = 1) out vec3 v2;
                layout(location = 2) out vec3 v3;

                void main() {
                    v1 = vec3(1.0);
                    v2 = vec3(2.0);
                    v3 = vec3(3.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout (location = 0) in vec3 v1;
                layout (location = 2) in vec3 v3;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(v1 * v3, 1.0);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(VkLayerTest, SpecializationInvalidSizeMismatch) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is not correct with type");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    bool int8_support = false;
    bool float64_support = false;

    // require to make enable logic simpler
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    features12.shaderInt8 = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features12.shaderInt8 == VK_TRUE) {
        int8_support = true;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (m_device->phy().features().shaderFloat64) {
        float64_support = true;
    }

    // layout (constant_id = 0) const int a = 3;
    // layout (constant_id = 1) const uint b = 3;
    // layout (constant_id = 2) const float c = 3.0f;
    // layout (constant_id = 3) const bool d = true;
    // layout (constant_id = 4) const bool f = false;
    std::string cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
               OpDecorate %c SpecId 2
               OpDecorate %d SpecId 3
               OpDecorate %f SpecId 4
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
       %bool = OpTypeBool
          %a = OpSpecConstant %int 3
          %b = OpSpecConstant %uint 3
          %c = OpSpecConstant %float 3
          %d = OpSpecConstantTrue %bool
          %f = OpSpecConstantFalse %bool
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    // use same offset to keep simple since unused data being read
    VkSpecializationMapEntry entries[5] = {
        {0, 0, 4},                 // OpTypeInt 32
        {1, 0, 4},                 // OpTypeInt 32
        {2, 0, 4},                 // OpTypeFloat 32
        {3, 0, sizeof(VkBool32)},  // OpTypeBool
        {4, 0, sizeof(VkBool32)}   // OpTypeBool
    };

    std::array<int32_t, 4> data;  // enough garbage data to grab from
    VkSpecializationInfo specialization_info = {
        5,
        entries,
        data.size() * sizeof(decltype(data)::value_type),
        data.data(),
    };

    std::unique_ptr<VkShaderObj> cs;
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };

    // Sanity check
    cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
    if (cs) {
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);

        // signed int mismatch
        entries[0].size = 0;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[0].size = 2;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[0].size = 8;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[0].size = 4;  // reset

        // unsigned int mismatch
        entries[1].size = 1;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[1].size = 8;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[1].size = 3;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[1].size = 4;  // reset

        // float mismatch
        entries[2].size = 0;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[2].size = 8;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[2].size = 7;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[2].size = 4;  // reset

        // bool mismatch
        entries[3].size = sizeof(VkBool32) / 2;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[3].size = sizeof(VkBool32) + 1;
        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    }

    if (int8_support == true) {
        // #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
        // layout (constant_id = 0) const int8_t a = int8_t(3);
        // layout (constant_id = 1) const uint8_t b = uint8_t(3);
        cs_src = R"(
               OpCapability Shader
               OpCapability Int8
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int8"
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %char = OpTypeInt 8 1
      %uchar = OpTypeInt 8 0
          %a = OpSpecConstant %char 3
          %b = OpSpecConstant %uchar 3
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
            )";

        specialization_info.mapEntryCount = 2;
        entries[0] = {0, 0, 1};  // OpTypeInt 8
        entries[1] = {1, 0, 1};  // OpTypeInt 8

        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        if (cs) {
            // Sanity check
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);

            // signed int 8 mismatch
            entries[0].size = 0;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 2;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 4;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 1;  // reset

            // unsigned int 8 mismatch
            entries[1].size = 0;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[1].size = 2;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[1].size = 4;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        }
    }

    if (float64_support == true) {
        // #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        // layout (constant_id = 0) const float64_t a = 3.0f;
        cs_src = R"(
               OpCapability Shader
               OpCapability Float64
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float64"
               OpDecorate %a SpecId 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
     %double = OpTypeFloat 64
          %a = OpSpecConstant %double 3
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
            )";

        specialization_info.mapEntryCount = 1;
        entries[0] = {0, 0, 8};  // OpTypeFloat 64

        cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
        if (cs) {
            // Sanity check
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);

            // float 64 mismatch
            entries[0].size = 1;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 2;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 4;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 16;
            cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        }
    }
}

TEST_F(VkLayerTest, ComputeWorkGroupSizeSpecConstant) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeWorkGroupSize");

    ASSERT_NO_FATAL_FAILURE(Init());
    const VkPhysicalDeviceLimits limits = m_device->phy().properties().limits;

    // Make sure compute pipeline has a compute shader stage set
    const char *cs_source = R"glsl(
        #version 450
        layout(local_size_x_id = 3, local_size_y_id = 4) in;
        void main(){}
    )glsl";

    VkSpecializationMapEntry entries[2];
    entries[0].constantID = 3;
    entries[0].offset = 0;
    entries[0].size = sizeof(uint32_t);
    entries[1].constantID = 4;
    entries[1].offset = sizeof(uint32_t);
    entries[1].size = sizeof(uint32_t);

    uint32_t data[2] = {
        1,
        limits.maxComputeWorkGroupSize[1] + 1,  // Invalid
    };

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t) * 2;
    specialization_info.pData = data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                         &specialization_info));
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-y-06430");

    data[0] = limits.maxComputeWorkGroupSize[0] + 1;  // Invalid
    data[1] = 1;
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");

    data[0] = limits.maxComputeWorkGroupSize[0];
    data[1] = limits.maxComputeWorkGroupSize[1];
    if ((data[0] + data[1]) > limits.maxComputeWorkGroupInvocations) {
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06432");
    }
}

TEST_F(VkLayerTest, ComputeWorkGroupSizeConstantDefault) {
    TEST_DESCRIPTION("Make sure constant are applied for maxComputeWorkGroupSize using WorkgroupSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %limit = OpConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %limit %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(VkLayerTest, ComputeWorkGroupSizeSpecConstantDefault) {
    TEST_DESCRIPTION("Make sure spec constant are applied for maxComputeWorkGroupSize using WorkgroupSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %limit SpecId 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %limit = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %limit %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(VkLayerTest, ComputeWorkGroupSizeLocalSizeId) {
    TEST_DESCRIPTION("Validate LocalSizeId also triggers maxComputeWorkGroupSize limit");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %limit %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %limit = OpConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM));
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(VkLayerTest, ComputeWorkGroupSizeLocalSizeIdSpecConstantDefault) {
    TEST_DESCRIPTION("Validate LocalSizeId also triggers maxComputeWorkGroupSize limit with spec constants default");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    // layout(local_size_x_id = 18, local_size_z_id = 19) in;
    // layout(local_size_x = 32) in;
    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %spec_z
               OpSource GLSL 450
               OpDecorate %spec_x SpecId 18
               OpDecorate %spec_z SpecId 19
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %spec_x = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM));
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(VkLayerTest, ComputeWorkGroupSizeLocalSizeIdSpecConstantSet) {
    TEST_DESCRIPTION("Validate LocalSizeId also triggers maxComputeWorkGroupSize limit with spec constants");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    // layout(local_size_x_id = 18, local_size_z_id = 19) in;
    // layout(local_size_x = 32) in;
    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %spec_z
               OpSource GLSL 450
               OpDecorate %spec_x SpecId 18
               OpDecorate %spec_z SpecId 19
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint 32
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = x_size_limit + 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 18;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                         SPV_SOURCE_ASM, &specialization_info));
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(VkLayerTest, NoUniformBufferStandardLayout10) {
    TEST_DESCRIPTION("Don't enable uniformBufferStandardLayout in Vulkan 1.0 and have spirv-val catch invalid shader");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // layout(std430, set = 0, binding = 0) uniform ubo430 {
    //     float floatArray430[8];
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %ubo430 0 Offset 0
               OpDecorate %ubo430 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
     %ubo430 = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_ubo430 = OpTypePointer Uniform %ubo430
          %_ = OpVariable %_ptr_Uniform_ubo430 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01379");
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NoUniformBufferStandardLayout12) {
    TEST_DESCRIPTION(
        "Don't enable uniformBufferStandardLayout in Vulkan1.2 when VK_KHR_uniform_buffer_standard_layout was promoted");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    // layout(std430, set = 0, binding = 0) uniform ubo430 {
    //     float floatArray430[8];
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %ubo430 0 Offset 0
               OpDecorate %ubo430 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
     %ubo430 = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_ubo430 = OpTypePointer Uniform %ubo430
          %_ = OpVariable %_ptr_Uniform_ubo430 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01379");
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NoScalarBlockLayout10) {
    TEST_DESCRIPTION("Don't enable scalarBlockLayout in Vulkan 1.0 and have spirv-val catch invalid shader");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // layout (scalar, set = 0, binding = 0) buffer ssbo {
    //     layout(offset = 4) vec3 x;
    // };
    //
    // Note: using BufferBlock for Vulkan 1.0
    // Note: Relaxed Block Layout would also make this valid if enabled
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %ssbo 0 Offset 4
               OpDecorate %ssbo BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
       %ssbo = OpTypeStruct %v3float
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01379");
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NoScalarBlockLayout12) {
    TEST_DESCRIPTION("Don't enable scalarBlockLayout in Vulkan1.2 when VK_EXT_scalar_block_layout was promoted");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    // layout (scalar, set = 0, binding = 0) buffer ssbo {
    //     layout(offset = 0) vec3 a;
    //     layout(offset = 12) vec2 b;
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %ssbo 0 Offset 0
               OpMemberDecorate %ssbo 1 Offset 12
               OpDecorate %ssbo Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
       %ssbo = OpTypeStruct %v3float %v2float
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01379");
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestPipelineRasterizationConservativeStateCreateInfo) {
    TEST_DESCRIPTION("Test PipelineRasterizationConservativeStateCreateInfo.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props =
        LvlInitStruct<VkPhysicalDeviceConservativeRasterizationPropertiesEXT>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&conservative_rasterization_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservative_state =
        LvlInitStruct<VkPipelineRasterizationConservativeStateCreateInfoEXT>();
    conservative_state.extraPrimitiveOverestimationSize = -1.0f;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
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

TEST_F(VkLayerTest, TestShaderZeroInitializeWorkgroupMemory) {
    TEST_DESCRIPTION("Test initializing workgroup memory in compute shader");

    AddRequiredExtensions(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool zero_initialize_workgroup_memory = AreRequiredExtensionsEnabled();

    auto zero_initialize_work_group_memory_features = LvlInitStruct<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&zero_initialize_work_group_memory_features);
    if (zero_initialize_workgroup_memory) {
        features2.pNext = &zero_initialize_work_group_memory_features;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpName %counter "counter"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
  %zero_uint = OpConstantNull %uint
    %counter = OpVariable %_ptr_Workgroup_uint Workgroup %zero_uint
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    auto cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr);
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };
    if (cs) {
        const char *vuid = zero_initialize_workgroup_memory ? "VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372"
                                                            : "VUID-RuntimeSpirv-OpVariable-06373";
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuid);
    }
}

TEST_F(VkLayerTest, TestMinAndMaxTexelGatherOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelGatherOffset and greather than maxTexelGatherOffset");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (m_device->phy().properties().limits.minTexelGatherOffset <= -100 ||
        m_device->phy().properties().limits.maxTexelGatherOffset >= 100) {
        GTEST_SKIP() << "test needs minTexelGatherOffset greater than -100 and maxTexelGatherOffset less than 100";
    }

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450

               ; Annotations
               OpDecorate %samp DescriptorSet 0
               OpDecorate %samp Binding 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %samp = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
  %float_0_5 = OpConstant %float 0.5
         %17 = OpConstantComposite %v2float %float_0_5 %float_0_5
              ; set up composite to be validated
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
      %int_0 = OpConstant %int 0
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2int %int_0 %uint_n100

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
      %color = OpVariable %_ptr_Function_v4float Function
         %14 = OpLoad %11 %samp
               ; Should trigger min and max
         %24 = OpImageGather %v4float %14 %17 %int_0 ConstOffset %offset_100
               ; Should only trigger max since uint
         %25 = OpImageGather %v4float %14 %17 %int_0 ConstOffset %offset_n100
               OpStore %color %24
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    auto cs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr);

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_ = std::move(cs);
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    cs_pipeline.LateBindPipelineInfo();
    // as commented in SPIR-V should trigger the limits as following
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06376");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestMinAndMaxTexelOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelOffset and greather than maxTexelOffset");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (m_device->phy().properties().limits.minTexelOffset <= -100 || m_device->phy().properties().limits.maxTexelOffset >= 100) {
        GTEST_SKIP() << "test needs minTexelGatherOffset greater than -100 and maxTexelGatherOffset less than 100";
    }

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %textureSampler DescriptorSet 0
               OpDecorate %textureSampler Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
%textureSampler = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
              ; set up composite to be validated
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2int %int_0 %uint_n100
         %24 = OpConstantComposite %v2int %int_0 %int_0

       %main = OpFunction %void None %3
      %label = OpLabel
         %14 = OpLoad %11 %textureSampler
         %26 = OpImage %10 %14
               ; Should trigger min and max
    %result0 = OpImageSampleImplicitLod %v4float %14 %17 ConstOffset %offset_100
    %result1 = OpImageFetch %v4float %26 %24 ConstOffset %offset_100
               ; Should only trigger max since uint
    %result2 = OpImageSampleImplicitLod %v4float %14 %17 ConstOffset %offset_n100
    %result3 = OpImageFetch %v4float %26 %24 ConstOffset %offset_n100
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkShaderObj const fs(this, spv_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    // as commented in SPIR-V should trigger the limits as following
    //
    // OpImageSampleImplicitLod
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06435");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    // // OpImageFetch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06435");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceMemoryScope) {
    TEST_DESCRIPTION("Validate using Device memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    features12.vulkanMemoryModelDeviceScope = VK_FALSE;
    if (features12.vulkanMemoryModel == VK_FALSE) {
        GTEST_SKIP() << "vulkanMemoryModel feature is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
	   }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-vulkanMemoryModel-06265");
}

TEST_F(VkLayerTest, QueueFamilyMemoryScope) {
    TEST_DESCRIPTION("Validate using QueueFamily memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    features12.vulkanMemoryModel = VK_FALSE;
    if (features12.vulkanMemoryModelDeviceScope == VK_FALSE) {
        GTEST_SKIP() << "vulkanMemoryModelDeviceScope feature is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeQueueFamily, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
	   }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-RuntimeSpirv-vulkanMemoryModel-06266", "VUID-VkShaderModuleCreateInfo-pCode-08740"});
}

TEST_F(VkLayerTest, CreatePipelineLayoutWithInvalidSetLayoutFlags) {
    TEST_DESCRIPTION("Validate setLayout flags in create pipeline layout.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mut_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
    GetPhysicalDeviceFeatures2(mut_features);
    if (!mut_features.mutableDescriptorType) {
        GTEST_SKIP() << "mutableDescriptorType not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mut_features));

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &layout_binding;

    vk_testing::DescriptorSetLayout ds_layout;
    ds_layout.init(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout_handle;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-04606");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ComputeImageLayout) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vkCmdDispatchBaseKHR =
        reinterpret_cast<PFN_vkCmdDispatchBaseKHR>(vk::GetInstanceProcAddr(instance(), "vkCmdDispatchBaseKHR"));
    ASSERT_TRUE(vkCmdDispatchBaseKHR != nullptr);

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(fmt);

    VkSamplerObj sampler(m_device);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    {  // Verify invalid image layout with CmdDispatch
        VkCommandBufferObj cmd(m_device, m_commandPool);
        cmd.begin();
        vk::CmdBindDescriptorSets(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);
        vk::CmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
        vk::CmdDispatch(cmd.handle(), 1, 1, 1);
        cmd.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_DrawState_InvalidImageLayout);
        cmd.QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
    }

    {  // Verify invalid image layout with CmdDispatchBaseKHR
        VkCommandBufferObj cmd(m_device, m_commandPool);
        cmd.begin();
        vk::CmdBindDescriptorSets(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);
        vk::CmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
        vkCmdDispatchBaseKHR(cmd.handle(), 0, 0, 0, 1, 1, 1);
        cmd.end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_DrawState_InvalidImageLayout);
        cmd.QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ComputeImageLayout_1_1) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader using vkCmdDispatchBase");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto vkCmdDispatchBaseKHR =
        reinterpret_cast<PFN_vkCmdDispatchBaseKHR>(vk::GetInstanceProcAddr(instance(), "vkCmdDispatchBaseKHR"));
    ASSERT_TRUE(vkCmdDispatchBaseKHR != nullptr);

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(fmt);

    VkSamplerObj sampler(m_device);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdDispatchBase(m_commandBuffer->handle(), 0, 0, 0, 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_DrawState_InvalidImageLayout);
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineNullRenderPass) {
    TEST_DESCRIPTION("Test for a creating a pipeline with a null renderpass but VK_KHR_dynamic_rendering is not enabled");

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06574");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06603");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineRasterizationOrderAttachmentAccessWithoutFeature) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_ARM_rasterization_order_attachment_access enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto rasterization_order_features = LvlInitStruct<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>();
    GetPhysicalDeviceFeatures2(rasterization_order_features);

    rasterization_order_features.rasterizationOrderColorAttachmentAccess = 0;
    rasterization_order_features.rasterizationOrderDepthAttachmentAccess = 0;
    rasterization_order_features.rasterizationOrderStencilAttachmentAccess = 0;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &rasterization_order_features));

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    VkPipelineColorBlendAttachmentState cb_as = {};
    auto cb_ci = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
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

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 2;
    rpci.pAttachments = attachments;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass(*m_device, rpci);

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

TEST_F(VkLayerTest, CreateGraphicsPipelineRasterizationOrderAttachmentAccessNoSubpassFlags) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_ARM_rasterization_order_attachment_access enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto rasterization_order_features = LvlInitStruct<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>();
    GetPhysicalDeviceFeatures2(rasterization_order_features);

    if (!rasterization_order_features.rasterizationOrderColorAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderDepthAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderStencilAttachmentAccess) {
        GTEST_SKIP() << "Test requires (unsupported) rasterizationOrder*AttachmentAccess";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &rasterization_order_features));

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    VkPipelineColorBlendAttachmentState cb_as = {};
    auto cb_ci = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    cb_ci.attachmentCount = 1;
    cb_ci.pAttachments = &cb_as;
    VkRenderPass render_pass_handle = VK_NULL_HANDLE;

    auto create_render_pass = [&](VkPipelineDepthStencilStateCreateFlags subpass_flags, vk_testing::RenderPass &render_pass) {
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

        VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
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

    vk_testing::RenderPass render_pass_no_flags;
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
        vk_testing::RenderPass render_pass;
        create_render_pass(VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM, render_pass);
        render_pass_handle = render_pass.handle();
        CreatePipelineHelper::OneshotTest(*this, set_stages_pipeline_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06591");
    }
}

TEST_F(VkLayerTest, TestMismatchedRenderPassAndPipelineAttachments) {
    TEST_DESCRIPTION("Test creating a pipeline with no attachments with a render pass with attachments.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06042");

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
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 1;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout});
    pipe.CreateVKPipeline(pipeline_layout.handle(), m_renderPass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IncompatibleScissorCountAndViewportCount) {
    TEST_DESCRIPTION("Validate creating a pipeline with incompatible scissor and viewport count, without dynamic states.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

TEST_F(VkLayerTest, TestCreatingPipelineWithScissorWithCount) {
    TEST_DESCRIPTION("Validate creating graphics pipeline with dynamic state scissor with count.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        dyn_state_ci.pDynamicStates = dyn_states;

        auto set_viewport_state_createinfo = [&](CreatePipelineHelper &helper) {
            helper.dyn_state_ci_ = dyn_state_ci;
            helper.vp_state_ci_.scissorCount = 0;
            helper.vp_state_ci_.viewportCount = 0;
        };

        CreatePipelineHelper::OneshotTest(*this, set_viewport_state_createinfo, kErrorBit,
                                          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136");
    }

    {
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 2;
        dyn_state_ci.pDynamicStates = dyn_states;

        VkRect2D scissors = {};

        auto set_viewport_state_createinfo = [&](CreatePipelineHelper &helper) {
            helper.dyn_state_ci_ = dyn_state_ci;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = &scissors;
            helper.vp_state_ci_.viewportCount = 0;
        };

        CreatePipelineHelper::OneshotTest(*this, set_viewport_state_createinfo, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380");
    }
}

TEST_F(VkLayerTest, DynamicSampleLocations) {
    TEST_DESCRIPTION("Validate dynamic sample locations.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto sample_locations_props = LvlInitStruct<VkPhysicalDeviceSampleLocationsPropertiesEXT>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&sample_locations_props);
    GetPhysicalDeviceProperties2(properties2);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        GTEST_SKIP() << "Required sample location sample count VK_SAMPLE_COUNT_1_BIT not supported";
    }

    auto vkCmdSetSampleLocationsEXT =
        reinterpret_cast<PFN_vkCmdSetSampleLocationsEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetSampleLocationsEXT"));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;

    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06666");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkSampleLocationEXT sample_location = {0.5f, 0.5f};

    auto sample_locations_info = LvlInitStruct<VkSampleLocationsInfoEXT>();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = {1u, 1u};
    sample_locations_info.sampleLocationsCount = 1;
    sample_locations_info.pSampleLocations = &sample_location;

    vkCmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestShaderInputOutputMismatch) {
    TEST_DESCRIPTION("Test mismatch between vertex shader output and fragment shader input.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char vsSource[] = R"glsl(
        #version 450
        layout(location = 1) out int v;
        void main() {
            v = 1;
        }
    )glsl";

    const char fsSource[] = R"glsl(
        #version 450
        layout(location = 0) out vec4 color;
        layout(location = 1) in float v;
        void main() {
           color = vec4(v);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5208
// Asserting in MoltenVK
TEST_F(VkLayerTest, DISABLED_MaxFragmentDualSrcAttachments) {
    TEST_DESCRIPTION("Test drawing with dual source blending with too many fragment output attachments.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>();
    GetPhysicalDeviceFeatures2(features2);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    uint32_t count = m_device->props.limits.maxFragmentDualSrcAttachments + 1;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(count));

    std::stringstream fsSource;
    fsSource << "#version 450\n";
    for (uint32_t i = 0; i < count; ++i) {
        fsSource << "layout(location = " << i << ") out vec4 c" << i << ";\n";
    }
    fsSource << " void main() {\n";
    for (uint32_t i = 0; i < count; ++i) {
        fsSource << "c" << i << " = vec4(0.0f);\n";
    }

    fsSource << "}";
    VkShaderObj fs(this, fsSource.str().c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState cb_attachments = {};
    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;

    CreatePipelineHelper pipe(*this, count);
    pipe.InitInfo();
    pipe.cb_attachments_[0] = cb_attachments;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Fragment-06427");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestLocalSizeIdExecutionMode) {
    TEST_DESCRIPTION("Test LocalSizeId spirv execution mode");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() != VK_API_VERSION_1_3) {
        GTEST_SKIP() << "Test requires Vulkan exactly 1.3";
    }

    const char *source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_UNIVERSAL_1_6, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-LocalSizeId-06434");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, StorageImageWriteLessComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!ImageFormatAndFeaturesSupported(gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(VkLayerTest, StorageImageWriteSpecConstantLessComponent) {
    TEST_DESCRIPTION("Test writing to image with less components with Texel being a spec constant.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // not valid GLSL, but would look like:
    // layout (constant_id = 0) const uint sc = 1;
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, sc, sc + 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
         %sc = OpSpecConstant %uint 1
      %sc_p1 = OpSpecConstantOp %uint IAdd %sc %uint_1
    %texelU3 = OpSpecConstantComposite %v3uint %uint_1 %sc %sc_p1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!ImageFormatAndFeaturesSupported(gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    uint32_t data = 2;
    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);
    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM, &specialization_info));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(VkLayerTest, StorageTexelBufferWriteLessComponent) {
    TEST_DESCRIPTION("Test writing to texel buffer with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimageBuffer storageTexelBuffer;
    // imageStore(storageTexelBuffer, 1, uvec3(1, 1, 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint Buffer 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
     %v3uint = OpTypeVector %uint 3
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %int_1 %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!BufferFormatAndFeaturesSupported(gpu(), format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage texel buffer";
    }

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(VkLayerTest, StorageImageUnknownWriteLessComponent) {
    TEST_DESCRIPTION("Test writing to image unknown format with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) readonly uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Unknown will become a 4-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
               OpDecorate %var NonReadable
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;
    if (!ImageFormatAndFeaturesSupported(gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image.targetView(format);
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-04115");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, StorageTexelBufferUnkownWriteLessComponent) {
    TEST_DESCRIPTION("Test writing to texel buffer unknown format with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) uniform uimageBuffer storageTexelBuffer;
    // imageStore(storageTexelBuffer, 1, uvec3(1, 1, 1));
    //
    // Unknown will become a 4-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
               OpDecorate %var NonReadable
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint Buffer 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
     %v3uint = OpTypeVector %uint 3
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %int_1 %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!BufferFormatAndFeaturesSupported(gpu(), format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage texel buffer";
    }

    auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
    auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UINT, &fmt_props);
    if ((fmt_props_3.bufferFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR) == 0) {
        GTEST_SKIP() << "Format doesn't support storage write without format";
    }

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vk_testing::BufferView buffer_view(*m_device, buff_view_ci);

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &buffer_view.handle();
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpImageWrite-04469");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ConservativeRasterizationPostDepthCoverage) {
    TEST_DESCRIPTION("Make sure conservativeRasterizationPostDepthCoverage is set if needed.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto conservative_rasterization_props = LvlInitStruct<VkPhysicalDeviceConservativeRasterizationPropertiesEXT>();
    GetPhysicalDeviceProperties2(conservative_rasterization_props);
    if (conservative_rasterization_props.conservativeRasterizationPostDepthCoverage) {
        GTEST_SKIP() << "need conservativeRasterizationPostDepthCoverage to not be supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::string const source{R"(
               OpCapability Shader
               OpCapability SampleMaskPostDepthCoverage
               OpCapability FragmentFullyCoveredEXT
               OpExtension "SPV_EXT_fragment_fully_covered"
               OpExtension "SPV_KHR_post_depth_coverage"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %12
               OpExecutionMode %4 OriginUpperLeft
               OpExecutionMode %4 EarlyFragmentTests
               OpExecutionMode %4 PostDepthCoverage
               OpDecorate %12 BuiltIn FullyCoveredEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
         %12 = OpVariable %_ptr_Input_bool Input
          %4 = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)"};

    VkShaderObj fs(this, source.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    auto set_info = [&](CreatePipelineHelper &info) {
        info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235");
}
