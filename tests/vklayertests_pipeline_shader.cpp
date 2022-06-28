/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"
#include "core_validation_error_enums.h"

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
    m_errorMonitor->ExpectSuccess();
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
    pipe.cs_ = layer_data::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
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

    m_errorMonitor->VerifyNotFound();
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

TEST_F(VkLayerTest, PipelineWrongBindPointRayTracing) {
    TEST_DESCRIPTION("Bind a graphics pipeline in the ray-tracing bind point");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_RAY_TRACING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NV_RAY_TRACING_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineBindPoint-02392");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipe.pipeline_);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineBadVertexAttributeFormat) {
    TEST_DESCRIPTION("Test that pipeline validation catches invalid vertex attribute formats");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs;
    memset(&input_attribs, 0, sizeof(input_attribs));

    // Pick a really bad format for this purpose and make sure it should fail
    input_attribs.format = VK_FORMAT_BC2_UNORM_BLOCK;
    VkFormatProperties format_props = m_device->format_properties(input_attribs.format);
    if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) != 0) {
        printf("%s Format unsuitable for test; skipped.\n", kSkipPrefix);
        return;
    }

    input_attribs.location = 0;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkVertexInputAttributeDescription-format-00623");
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
    pipeline.AddShader(&vs);

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
        printf("%s Unable to find a color attachment format with no blending support. Skipping test.\n", kSkipPrefix);
        return;
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

    VkRenderPass rp;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    pipeline.AddShader(&vs);

    VkPipelineColorBlendAttachmentState att_state = {};
    att_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state.blendEnable = VK_TRUE;
    pipeline.AddColorAttachment(0, att_state);
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), rp);

    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(m_device->device(), rp, NULL);
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Pipeline topology is set to POINT_LIST");
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

    topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      std::vector<string>{"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                                          "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429"});

    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      std::vector<string>{"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                                          "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429"});

    topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      std::vector<string>{"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                                          "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430",
                                                          "VUID-VkGraphicsPipelineCreateInfo-topology-00737"});

    topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429");

    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429");
}

TEST_F(VkLayerTest, PrimitiveTopologyListRestart) {
    TEST_DESCRIPTION("Test VK_EXT_primitive_topology_list_restart");
    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    auto ptl_restart_features = LvlInitStruct<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ptl_restart_features);
    m_device_extension_names.push_back(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }
    if (!ptl_restart_features.primitiveTopologyListRestart) {
        printf("%s primitive topology list restart feature is not available, skipping test\n", kSkipPrefix);
        return;
    }
    ptl_restart_features.primitiveTopologyListRestart = false;
    ptl_restart_features.primitiveTopologyPatchListRestart = false;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
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
    topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      std::vector<string>{"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06253",
                                                          "VUID-VkGraphicsPipelineCreateInfo-topology-00737"});
}

TEST_F(VkLayerTest, PointSizeGeomShaderFailure) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, but not in the final geometry stage.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        printf("%s Device does not support the required geometry shader features; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and writing to it
    static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
           EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Pipeline topology is set to POINT_LIST");
}

TEST_F(VkLayerTest, BuiltinBlockOrderMismatchVsGs) {
    TEST_DESCRIPTION("Use different order of gl_Position and gl_PointSize in builtin block interface between VS and GS.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().geometryShader || !m_device->phy().features().shaderTessellationAndGeometryPointSize) {
        printf("%s Device does not support geometry shaders; Skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Compiled using the GLSL code below. GlslangValidator rearranges the members, but here they are kept in the order provided.
    // #version 450
    // layout (points) in;
    // layout (points) out;
    // layout (max_vertices = 1) out;
    // in gl_PerVertex {
    //     float gl_PointSize;
    //     vec4 gl_Position;
    // } gl_in[];
    // void main() {
    //     gl_Position = gl_in[0].gl_Position;
    //     gl_PointSize = gl_in[0].gl_PointSize;
    //     EmitVertex();
    // }

    const char *gsSource = R"(
               OpCapability Geometry
               OpCapability GeometryPointSize
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_ %gl_in
               OpExecutionMode %main InputPoints
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputPoints
               OpExecutionMode %main OutputVertices 1
               OpSource GLSL 450
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn Position
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%gl_PerVertex_0 = OpTypeStruct %float %v4float
%_arr_gl_PerVertex_0_uint_1 = OpTypeArray %gl_PerVertex_0 %uint_1
%_ptr_Input__arr_gl_PerVertex_0_uint_1 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_1
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_1 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %21 = OpAccessChain %_ptr_Input_v4float %gl_in %int_0 %int_1
         %22 = OpLoad %v4float %21
         %24 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %24 %22
         %27 = OpAccessChain %_ptr_Input_float %gl_in %int_0 %int_0
         %28 = OpLoad %float %27
         %30 = OpAccessChain %_ptr_Output_float %_ %int_1
               OpStore %30 %28
               OpEmitVertex
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Builtin variable inside block doesn't match between");
}

TEST_F(VkLayerTest, BuiltinBlockSizeMismatchVsGs) {
    TEST_DESCRIPTION("Use different number of elements in builtin block interface between VS and GS.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().geometryShader || !m_device->phy().features().shaderTessellationAndGeometryPointSize) {
        printf("%s Device does not support geometry shaders; Skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
            float gl_ClipDistance[];
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Number of elements inside builtin block differ between stages");
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
    VkDescriptorSetLayout ds_layout = {};
    VkResult err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    // Create an array of DSLs, one larger than the physical limit
    const auto excess_layouts = 1 + m_device->phy().properties().limits.maxBoundDescriptorSets;
    std::vector<VkDescriptorSetLayout> dsl_array(excess_layouts, ds_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = excess_layouts;
    pipeline_layout_ci.pSetLayouts = dsl_array.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-setLayoutCount-00286");
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // Clean up
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);
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

    bool descriptor_indexing = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    descriptor_indexing = descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    descriptor_indexing =
        descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

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

    VkPhysicalDeviceDescriptorIndexingProperties descriptor_indexing_properties = {};
    if (descriptor_indexing) {
        descriptor_indexing_properties = GetDescriptorIndexingProperties(instance(), gpu());
    }

    // Devices that report UINT32_MAX for any of these limits can't run this test
    if (UINT32_MAX == std::max({max_uniform_buffers, max_storage_buffers, max_sampled_images, max_storage_images, max_samplers})) {
        printf("%s Physical device limits report as 2^32-1. Skipping test.\n", kSkipPrefix);
        return;
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

    bool descriptor_indexing = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    descriptor_indexing = descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    descriptor_indexing =
        descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

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

    VkPhysicalDeviceDescriptorIndexingProperties descriptor_indexing_properties = {};
    if (descriptor_indexing) {
        descriptor_indexing_properties = GetDescriptorIndexingProperties(instance(), gpu());
    }

    // Devices that report UINT32_MAX for any of these limits can't run this test
    if (UINT32_MAX == std::max({sum_dyn_uniform_buffers, sum_uniform_buffers, sum_dyn_storage_buffers, sum_storage_buffers,
                                sum_sampled_images, sum_storage_images, sum_samplers, sum_input_attachments})) {
        printf("%s Physical device limits report as 2^32-1. Skipping test.\n", kSkipPrefix);
        return;
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
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
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
    uint64_t fake_pipeline_handle = 0xbaad6001;
    VkPipeline bad_pipeline = reinterpret_cast<VkPipeline &>(fake_pipeline_handle);

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
        auto fpCmdDrawIndirectCountKHR =
            (PFN_vkCmdDrawIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndirectCountKHR");
        ASSERT_NE(fpCmdDrawIndirectCountKHR, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-02700");
        // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndirectCommand)
        fpCmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
        m_errorMonitor->VerifyFound();

        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            auto fpCmdDrawIndirectCount =
                (PFN_vkCmdDrawIndirectCount)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndirectCount");
            if (nullptr == fpCmdDrawIndirectCount) {
                m_errorMonitor->ExpectSuccess();
                m_errorMonitor->SetError("No ProcAddr for 1.2 core vkCmdDrawIndirectCount");
                m_errorMonitor->VerifyNotFound();
            } else {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-None-02700");
                // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndirectCommand)
                fpCmdDrawIndirectCount(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
                m_errorMonitor->VerifyFound();
            }
        }

        auto fpCmdDrawIndexedIndirectCountKHR =
            (PFN_vkCmdDrawIndexedIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndexedIndirectCountKHR");
        ASSERT_NE(fpCmdDrawIndexedIndirectCountKHR, nullptr);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-02700");
        // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndexedIndirectCommand)
        fpCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
        m_errorMonitor->VerifyFound();

        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            auto fpCmdDrawIndexedIndirectCount =
                (PFN_vkCmdDrawIndexedIndirectCount)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndexedIndirectCount");
            if (nullptr == fpCmdDrawIndexedIndirectCount) {
                m_errorMonitor->ExpectSuccess();
                m_errorMonitor->SetError("No ProcAddr for 1.2 core vkCmdDrawIndirectCount");
                m_errorMonitor->VerifyNotFound();
            } else {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-None-02700");
                // stride must be a multiple of 4 and must be greater than or equal to sizeof(VkDrawIndexedIndirectCommand)
                fpCmdDrawIndexedIndirectCount(m_commandBuffer->handle(), buffer.handle(), 0, buffer.handle(), 512, 1, 512);
                m_errorMonitor->VerifyFound();
            }
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

    // Enable KHX device group extensions, if available
    if (InstanceExtensionSupported(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool khx_dg_ext_available = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
        khx_dg_ext_available = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t x_count_limit = m_device->props.limits.maxComputeWorkGroupCount[0];
    uint32_t y_count_limit = m_device->props.limits.maxComputeWorkGroupCount[1];
    uint32_t z_count_limit = m_device->props.limits.maxComputeWorkGroupCount[2];
    if (std::max({x_count_limit, y_count_limit, z_count_limit}) == UINT32_MAX) {
        printf("%s device maxComputeWorkGroupCount limit reports UINT32_MAX, test not possible, skipping.\n", kSkipPrefix);
        return;
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

    char cs_text[128] = "";
    sprintf(cs_text, "#version 450\nlayout(local_size_x = %d, local_size_y = %d, local_size_z = %d) in;\nvoid main() {}\n",
            x_size_limit, y_size_limit, z_size_limit);

    VkShaderObj cs_obj(this, cs_text, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cs_.reset(new VkShaderObj(this, cs_text, VK_SHADER_STAGE_COMPUTE_BIT));
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

    if (khx_dg_ext_available) {
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
        printf("%s KHX_DEVICE_GROUP_* extensions not supported, skipping CmdDispatchBaseKHR() tests.\n", kSkipPrefix);
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stage-00727");

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

TEST_F(VkLayerTest, MissingStorageImageFormatRead) {
    TEST_DESCRIPTION("Create a shader reading a storage image without an image format");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceFeatures feat;
    vk::GetPhysicalDeviceFeatures(gpu(), &feat);
    if (feat.shaderStorageImageReadWithoutFormat) {
        printf("%s format less storage image read supported.\n", kSkipPrefix);
        return;
    }

    // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
    // disabled if VK_KHR_format_feature_flags2 is supported.
    //
    //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
    //
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        printf("%s %s supported, skipping.\n", kSkipPrefix, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
        return;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-features-shaderStorageImageReadWithoutFormat");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingStorageImageFormatWrite) {
    TEST_DESCRIPTION("Create a shader writing a storage image without an image format");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceFeatures feat;
    vk::GetPhysicalDeviceFeatures(gpu(), &feat);
    if (feat.shaderStorageImageWriteWithoutFormat) {
        printf("%s format less storage image write supported.\n", kSkipPrefix);
        return;
    }

    // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
    // disabled if VK_KHR_format_feature_flags2 is supported.
    //
    //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
    //
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        printf("%s %s supported, skipping.\n", kSkipPrefix, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
        return;
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpName %main "main"
                  OpName %img "img"
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-features-shaderStorageImageWriteWithoutFormat");
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
        printf("%s Could not build a test case.\n", kSkipPrefix);
        return;
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpTypeImage-06424");
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        if (tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR)
            m_errorMonitor->VerifyNotFound();
        else
            m_errorMonitor->VerifyFound();
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
        printf("%s Could not build a test case.\n", kSkipPrefix);
        return;
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpTypeImage-06423");
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        if (tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR)
            m_errorMonitor->VerifyNotFound();
        else
            m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MissingStorageTexelBufferFormatWriteForFormat) {
    TEST_DESCRIPTION("Create a shader writing a storage texel buffer without an image format");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

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
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto fmt_props_3 = LvlInitStruct<VkFormatProperties3>();
    auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

    // set so format can be used as a storage texel buffer, but no WITH_FORMAT support
    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(gpu(), format, &fmt_props);
    fmt_props.formatProperties.bufferFeatures |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
    fmt_props.formatProperties.linearTilingFeatures &= ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
    fmt_props_3.bufferFeatures |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
    fmt_props_3.linearTilingFeatures &= ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
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
    VkBufferView buffer_view;
    VkResult err = vk::CreateBufferView(m_device->device(), &buff_view_ci, NULL, &buffer_view);
    if (err != VK_SUCCESS) {
        // device profile layer might hide fact this is not a supported buffer view format
        printf("%s Device will not be able to initialize buffer view skipped.\n", kSkipPrefix);
        return;
    }

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &buffer_view;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(), 0,
                              1, &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-OpTypeImage-06423");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyBufferView(m_device->handle(), buffer_view, nullptr);
}

TEST_F(VkLayerTest, MissingNonReadableDecorationStorageImageFormatRead) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format not marked as non readable");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceFeatures feat;
    vk::GetPhysicalDeviceFeatures(gpu(), &feat);
    if (feat.shaderStorageImageReadWithoutFormat) {
        printf("%s format less storage image read supported.\n", kSkipPrefix);
        return;
    }

    // We need to skip this test with VK_KHR_format_feature_flags2 supported,
    // because checks for read/write without format has to be done per format
    // rather than as a device feature. The code we test here only looks at
    // the shader.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        printf("%s %s supported, skipping.\n", kSkipPrefix, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
        return;
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

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceFeatures feat;
    vk::GetPhysicalDeviceFeatures(gpu(), &feat);
    if (feat.shaderStorageImageWriteWithoutFormat) {
        printf("%s format less storage image write supported.\n", kSkipPrefix);
        return;
    }

    // We need to skip this test with VK_KHR_format_feature_flags2 supported,
    // because checks for read/write without format has to be done per format
    // rather than as a device feature. The code we test here only looks at
    // the shader.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        printf("%s %s supported, skipping.\n", kSkipPrefix, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
        return;
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpName %main "main"
                  OpName %img "img"
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
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
    m_errorMonitor->ExpectSuccess();

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
        printf("%s Cannot find suitable format, skipping.\n", kSkipPrefix);
        return;
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

    VkSampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    ASSERT_VK_SUCCESS(vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler));

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image.targetView(format), sampler,
                                                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->VerifyNotFound();

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
        printf("%s SampleRateShading feature is disabled -- skipping related checks.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto range_test = [this](float value, bool positive_test) {
        auto info_override = [value](CreatePipelineHelper &helper) {
            helper.pipe_ms_state_ci_.sampleShadingEnable = VK_TRUE;
            helper.pipe_ms_state_ci_.minSampleShading = value;
        };
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit,
                                          "VUID-VkPipelineMultisampleStateCreateInfo-minSampleShading-00786", positive_test);
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
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Set up the extension structs
    auto sampleLocations = chain_util::Init<VkPipelineSampleLocationsStateCreateInfoEXT>();
    sampleLocations.sampleLocationsInfo.sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
    auto coverageToColor = chain_util::Init<VkPipelineCoverageToColorStateCreateInfoNV>();
    auto coverageModulation = chain_util::Init<VkPipelineCoverageModulationStateCreateInfoNV>();
    auto discriminatrix = [this](const char *name) { return DeviceExtensionSupported(gpu(), nullptr, name); };
    chain_util::ExtensionChain chain(discriminatrix, &m_device_extension_names);
    chain.Add(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, sampleLocations);
    chain.Add(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME, coverageToColor);
    chain.Add(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME, coverageModulation);
    const void *extension_head = chain.Head();

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (extension_head) {
        auto good_chain = [extension_head](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = extension_head; };
        CreatePipelineHelper::OneshotTest(*this, good_chain, (kErrorBit | kWarningBit), "No error", true);
    } else {
        printf("%s Required extension not present -- skipping positive checks.\n", kSkipPrefix);
    }

    auto instance_ci = chain_util::Init<VkInstanceCreateInfo>();
    auto bad_chain = [&instance_ci](CreatePipelineHelper &helper) { helper.pipe_ms_state_ci_.pNext = &instance_ci; };
    CreatePipelineHelper::OneshotTest(*this, bad_chain, (kErrorBit | kWarningBit),
                                      "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext");
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

TEST_F(VkLayerTest, CreateGraphicsPipelineWithBadBasePointer) {
    TEST_DESCRIPTION("Create Graphics Pipeline with pointers that must be ignored by layers");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);

    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);

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

    uint64_t fake_pipeline_id = 0xCADECADE;
    VkPipeline fake_pipeline_handle = reinterpret_cast<VkPipeline &>(fake_pipeline_id);

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-00722");
    vk::CreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();

    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = 6;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-00723");
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
        printf("%s Test requires unsupported  depthBounds feature.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    if (m_depth_stencil_fmt == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt,
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
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBounds-minDepthBounds-02508");
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 1.5f, 0.0f);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBounds-maxDepthBounds-02509");
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 0.0f, 1.5f);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdSetDepthBounds(m_commandBuffer->handle(), 1.0f, 1.0f);
    m_errorMonitor->VerifyNotFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, VertexAttributeDivisorExtension) {
    TEST_DESCRIPTION("Test VUIDs added with VK_EXT_vertex_attribute_divisor extension.");

    bool inst_ext = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (inst_ext) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    }
    if (inst_ext && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    vadf.vertexAttributeInstanceRateDivisor = VK_TRUE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vadf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props =
        LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&pdvad_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.stride = 12;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    if (pdvad_props.maxVertexAttribDivisor < pvids_ci.vertexBindingDivisorCount) {
        printf("%sThis device does not support %d vertexBindingDivisors, skipping tests\n", kSkipPrefix,
               pvids_ci.vertexBindingDivisorCount);
        return;
    }

    using std::vector;
    struct TestCase {
        uint32_t div_binding;
        uint32_t div_divisor;
        uint32_t desc_binding;
        VkVertexInputRate desc_rate;
        vector<std::string> vuids;
    };

    // clang-format off
    vector<TestCase> test_cases = {
        {   0,
            1,
            0,
            VK_VERTEX_INPUT_RATE_VERTEX,
            {"VUID-VkVertexInputBindingDivisorDescriptionEXT-inputRate-01871"}
        },
        {   dev_limits.maxVertexInputBindings + 1,
            1,
            0,
            VK_VERTEX_INPUT_RATE_INSTANCE,
            {"VUID-VkVertexInputBindingDivisorDescriptionEXT-binding-01869",
             "VUID-VkVertexInputBindingDivisorDescriptionEXT-inputRate-01871"}
        }
    };

    if (UINT32_MAX != pdvad_props.maxVertexAttribDivisor) {  // Can't test overflow if maxVAD is UINT32_MAX
        test_cases.push_back(
            {   0,
                pdvad_props.maxVertexAttribDivisor + 1,
                0,
                VK_VERTEX_INPUT_RATE_INSTANCE,
                {"VUID-VkVertexInputBindingDivisorDescriptionEXT-divisor-01870"}
            } );
    }
    // clang-format on

    for (const auto &test_case : test_cases) {
        const auto bad_divisor_state = [&test_case, &vibdd, &pvids_ci, &vibd](CreatePipelineHelper &helper) {
            vibdd.binding = test_case.div_binding;
            vibdd.divisor = test_case.div_divisor;
            vibd.binding = test_case.desc_binding;
            vibd.inputRate = test_case.desc_rate;
            helper.vi_ci_.pNext = &pvids_ci;
            helper.vi_ci_.vertexBindingDescriptionCount = 1;
            helper.vi_ci_.pVertexBindingDescriptions = &vibd;
        };
        CreatePipelineHelper::OneshotTest(*this, bad_divisor_state, kErrorBit, test_case.vuids);
    }
}

TEST_F(VkLayerTest, VertexAttributeDivisorDisabled) {
    TEST_DESCRIPTION("Test instance divisor feature disabled for VK_EXT_vertex_attribute_divisor extension.");

    bool inst_ext = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (inst_ext) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    }
    if (inst_ext && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    vadf.vertexAttributeInstanceRateDivisor = VK_FALSE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vadf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props =
        LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&pdvad_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    vibdd.binding = 0;
    vibdd.divisor = 2;
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.binding = vibdd.binding;
    vibd.stride = 12;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    if (pdvad_props.maxVertexAttribDivisor < pvids_ci.vertexBindingDivisorCount) {
        printf("%sThis device does not support %d vertexBindingDivisors, skipping tests\n", kSkipPrefix,
               pvids_ci.vertexBindingDivisorCount);
        return;
    }

    const auto instance_rate = [&pvids_ci, &vibd](CreatePipelineHelper &helper) {
        helper.vi_ci_.pNext = &pvids_ci;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexBindingDescriptions = &vibd;
    };
    CreatePipelineHelper::OneshotTest(*this, instance_rate, kErrorBit,
                                      "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateDivisor-02229");
}

TEST_F(VkLayerTest, VertexAttributeDivisorInstanceRateZero) {
    TEST_DESCRIPTION("Test instanceRateZero feature of VK_EXT_vertex_attribute_divisor extension.");

    bool inst_ext = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (inst_ext) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    }
    if (inst_ext && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    vadf.vertexAttributeInstanceRateDivisor = VK_TRUE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vadf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    vibdd.binding = 0;
    vibdd.divisor = 0;
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.binding = vibdd.binding;
    vibd.stride = 12;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    const auto instance_rate = [&pvids_ci, &vibd](CreatePipelineHelper &helper) {
        helper.vi_ci_.pNext = &pvids_ci;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexBindingDescriptions = &vibd;
    };
    CreatePipelineHelper::OneshotTest(
        *this, instance_rate, kErrorBit,
        "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateZeroDivisor-02228");
}

/*// TODO : This test should be good, but needs Tess support in compiler to run
TEST_F(VkLayerTest, InvalidPatchControlPoints)
{
    // Attempt to Create Gfx Pipeline w/o a VS
    VkResult        err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
        "Invalid Pipeline CreateInfo State: VK_PRIMITIVE_TOPOLOGY_PATCH
primitive ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
        ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
        ds_pool_ci.poolSizeCount = 1;
        ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    err = vk::CreateDescriptorPool(m_device->device(),
VK_DESCRIPTOR_POOL_USAGE_NON_FREE, 1, &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
        dsl_binding.binding = 0;
        dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dsl_binding.descriptorCount = 1;
        dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
        dsl_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
        ds_layout_ci.bindingCount = 1;
        ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL,
&ds_layout);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSet descriptorSet;
    err = vk::AllocateDescriptorSets(m_device->device(), ds_pool,
VK_DESCRIPTOR_SET_USAGE_NON_FREE, 1, &ds_layout, &descriptorSet);
    ASSERT_VK_SUCCESS(err);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        pipeline_layout_ci.pNext = NULL;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &ds_layout;

    VkPipelineLayout pipeline_layout;
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL,
&pipeline_layout);
    ASSERT_VK_SUCCESS(err);

    VkPipelineShaderStageCreateInfo shaderStages[3];
    memset(&shaderStages, 0, 3 * sizeof(VkPipelineShaderStageCreateInfo));

    VkShaderObj vs(this,bindStateVertShaderText,VK_SHADER_STAGE_VERTEX_BIT);
    // Just using VS txt for Tess shaders as we don't care about functionality
    VkShaderObj tc(this,bindStateVertShaderText,VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj te(this,bindStateVertShaderText,VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    shaderStages[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].shader = vs.handle();
    shaderStages[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    shaderStages[1].stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    shaderStages[1].shader = tc.handle();
    shaderStages[2] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    shaderStages[2].stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    shaderStages[2].shader = te.handle();

    VkPipelineInputAssemblyStateCreateInfo iaCI = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
        iaCI.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    VkPipelineTessellationStateCreateInfo tsCI = LvlInitStruct<VkPipelineTessellationStateCreateInfo>();
        tsCI.patchControlPoints = 0; // This will cause an error

    VkGraphicsPipelineCreateInfo gp_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
        gp_ci.stageCount = 3;
        gp_ci.pStages = shaderStages;
        gp_ci.pVertexInputState = NULL;
        gp_ci.pInputAssemblyState = &iaCI;
        gp_ci.pTessellationState = &tsCI;
        gp_ci.pViewportState = NULL;
        gp_ci.pRasterizationState = NULL;
        gp_ci.pMultisampleState = NULL;
        gp_ci.pDepthStencilState = NULL;
        gp_ci.pColorBlendState = NULL;
        gp_ci.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
        gp_ci.layout = pipeline_layout;
        gp_ci.renderPass = renderPass();

    VkPipelineCacheCreateInfo pc_ci = LvlInitStruct<VkPipelineCacheCreateInfo>();
        pc_ci.initialSize = 0;
        pc_ci.initialData = 0;
        pc_ci.maxSize = 0;

    VkPipeline pipeline;
    VkPipelineCache pipelineCache;

    err = vk::CreatePipelineCache(m_device->device(), &pc_ci, NULL,
&pipelineCache);
    ASSERT_VK_SUCCESS(err);
    err = vk::CreateGraphicsPipelines(m_device->device(), pipelineCache, 1,
&gp_ci, NULL, &pipeline);

    m_errorMonitor->VerifyFound();

    vk::DestroyPipelineCache(m_device->device(), pipelineCache, NULL);
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);
    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}
*/

TEST_F(VkLayerTest, PSOViewportStateTests) {
    TEST_DESCRIPTION("Test VkPipelineViewportStateCreateInfo viewport and scissor count validation for non-multiViewport");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
        {0, viewports, 1, scissors, {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         1,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {1, viewports, 0, scissors, {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
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
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         nullptr,
         3,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {0, nullptr, 0, nullptr},
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

// Set Extension dynamic states without enabling the required Extensions.
TEST_F(VkLayerTest, ExtensionDynamicStatesSetWOExtensionEnabled) {
    TEST_DESCRIPTION("Create a graphics pipeline with Extension dynamic states without enabling the required Extensions.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    using std::vector;
    struct TestCase {
        uint32_t dynamic_state_count;
        VkDynamicState dynamic_state;

        char const *errmsg;
    };

    vector<TestCase> dyn_test_cases = {
        {1, VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
         "contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but VK_NV_clip_space_w_scaling"},
        {1, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
         "contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but VK_EXT_discard_rectangles"},
        {1, VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, "contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but VK_EXT_sample_locations"},
    };

    for (const auto &test_case : dyn_test_cases) {
        VkDynamicState state[1];
        state[0] = test_case.dynamic_state;
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
            dyn_state_ci.dynamicStateCount = test_case.dynamic_state_count;
            dyn_state_ci.pDynamicStates = state;
            helper.dyn_state_ci_ = dyn_state_ci;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.errmsg);
    }
}

TEST_F(VkLayerTest, PSOViewportStateMultiViewportTests) {
    TEST_DESCRIPTION("Test VkPipelineViewportStateCreateInfo viewport and scissor count validation for multiViewport feature");

    ASSERT_NO_FATAL_FAILURE(Init());  // enables all supported features

    if (!m_device->phy().features().multiViewport) {
        printf("%s VkPhysicalDeviceFeatures::multiViewport is not supported -- skipping test.\n", kSkipPrefix);
        return;
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
        printf("%s VkPhysicalDeviceLimits::maxViewports is UINT32_MAX -- skipping part of test requiring to exceed maxViewports.\n",
               kSkipPrefix);
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

TEST_F(VkLayerTest, DynViewportAndScissorUndefinedDrawState) {
    TEST_DESCRIPTION("Test viewport and scissor dynamic state that is not set before draw");

    ASSERT_NO_FATAL_FAILURE(Init());

    // TODO: should also test on !multiViewport
    if (!m_device->phy().features().multiViewport) {
        printf("%s Device does not support multiple viewports/scissors; skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkPipelineLayoutObj pipeline_layout(m_device);

    VkPipelineObj pipeline_dyn_vp(m_device);
    pipeline_dyn_vp.AddShader(&vs);
    pipeline_dyn_vp.AddShader(&fs);
    pipeline_dyn_vp.AddDefaultColorAttachment();
    pipeline_dyn_vp.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline_dyn_vp.SetScissor(m_scissors);
    ASSERT_VK_SUCCESS(pipeline_dyn_vp.CreateVKPipeline(pipeline_layout.handle(), m_renderPass));

    VkPipelineObj pipeline_dyn_sc(m_device);
    pipeline_dyn_sc.AddShader(&vs);
    pipeline_dyn_sc.AddShader(&fs);
    pipeline_dyn_sc.AddDefaultColorAttachment();
    pipeline_dyn_sc.SetViewport(m_viewports);
    pipeline_dyn_sc.MakeDynamic(VK_DYNAMIC_STATE_SCISSOR);
    ASSERT_VK_SUCCESS(pipeline_dyn_sc.CreateVKPipeline(pipeline_layout.handle(), m_renderPass));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-commandBuffer-02701");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_dyn_vp.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 1,
                       &m_viewports[0]);  // Forgetting to set needed 0th viewport (PSO viewportCount == 1)
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-commandBuffer-02701");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_dyn_sc.handle());
    vk::CmdSetScissor(m_commandBuffer->handle(), 1, 1,
                      &m_scissors[0]);  // Forgetting to set needed 0th scissor (PSO scissorCount == 1)
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, PSOLineWidthInvalid) {
    TEST_DESCRIPTION("Test non-1.0 lineWidth errors when pipeline is created and in vkCmdSetLineWidth");
    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::vector<float> test_cases = {-1.0f, 0.0f, NearestSmaller(1.0f), NearestGreater(1.0f), NAN};

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

TEST_F(VkLayerTest, VUID_VkVertexInputBindingDescription_binding_00618) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputBindingDescription-binding-00618: binding must be less than "
        "VkPhysicalDeviceLimits::maxVertexInputBindings");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when binding is greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings.
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = m_device->props.limits.maxVertexInputBindings;

    const auto set_binding = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_binding, kErrorBit, "VUID-VkVertexInputBindingDescription-binding-00618");
}

TEST_F(VkLayerTest, VUID_VkVertexInputBindingDescription_stride_00619) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputBindingDescription-stride-00619: stride must be less than or equal to "
        "VkPhysicalDeviceLimits::maxVertexInputBindingStride");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when stride is greater than VkPhysicalDeviceLimits::maxVertexInputBindingStride.
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.stride = m_device->props.limits.maxVertexInputBindingStride + 1;

    const auto set_binding = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_binding, kErrorBit, "VUID-VkVertexInputBindingDescription-stride-00619");
}

TEST_F(VkLayerTest, VUID_VkVertexInputAttributeDescription_location_00620) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputAttributeDescription-location-00620: location must be less than "
        "VkPhysicalDeviceLimits::maxVertexInputAttributes");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when location is greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.location = m_device->props.limits.maxVertexInputAttributes;

    const auto set_attribute = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_attribute, kErrorBit,
                                      vector<string>{"VUID-VkVertexInputAttributeDescription-location-00620",
                                                     "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615"});
}

TEST_F(VkLayerTest, VUID_VkVertexInputAttributeDescription_binding_00621) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputAttributeDescription-binding-00621: binding must be less than "
        "VkPhysicalDeviceLimits::maxVertexInputBindings");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when binding is greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.binding = m_device->props.limits.maxVertexInputBindings;

    const auto set_attribute = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_attribute, kErrorBit,
                                      vector<string>{"VUID-VkVertexInputAttributeDescription-binding-00621",
                                                     "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615"});
}

TEST_F(VkLayerTest, VUID_VkVertexInputAttributeDescription_offset_00622) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputAttributeDescription-offset-00622: offset must be less than or equal to "
        "VkPhysicalDeviceLimits::maxVertexInputAttributeOffset");

    EnableDeviceProfileLayer();

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    uint32_t maxVertexInputAttributeOffset = 0;
    {
        VkPhysicalDeviceProperties device_props = {};
        vk::GetPhysicalDeviceProperties(gpu(), &device_props);
        maxVertexInputAttributeOffset = device_props.limits.maxVertexInputAttributeOffset;
        if (maxVertexInputAttributeOffset == 0xFFFFFFFF) {
            // Attempt to artificially lower maximum offset
            PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
                (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
            if (!fpvkSetPhysicalDeviceLimitsEXT) {
                printf("%s All offsets are valid & device_profile_api not found; skipped.\n", kSkipPrefix);
                return;
            }
            device_props.limits.maxVertexInputAttributeOffset = device_props.limits.maxVertexInputBindingStride - 2;
            fpvkSetPhysicalDeviceLimitsEXT(gpu(), &device_props.limits);
            maxVertexInputAttributeOffset = device_props.limits.maxVertexInputAttributeOffset;
        }
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = m_device->props.limits.maxVertexInputBindingStride;
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // Test when offset is greater than maximum.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.format = VK_FORMAT_R8_UNORM;
    vertex_input_attribute_description.offset = maxVertexInputAttributeOffset + 1;

    const auto set_attribute = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_attribute, kErrorBit, "VUID-VkVertexInputAttributeDescription-offset-00622");
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

    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-subpass-00757");
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
    CreatePipelineHelper::OneshotTest(*this, set_MSAA, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06042");
}

TEST_F(VkLayerTest, CmdClearAttachmentTests) {
    TEST_DESCRIPTION("Various tests for validating usage of vkCmdClearAttachments");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Main thing we care about for this test is that the VkImage obj we're
    // clearing matches Color Attachment of FB
    //  Also pass down other dummy params to keep driver and paramchecker happy
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    clear_rect.rect.extent.width = renderPassBeginInfo().renderArea.extent.width + 4;
    clear_rect.rect.extent.height = clear_rect.rect.extent.height / 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00016");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    // baseLayer >= view layers
    clear_rect.rect.extent.width = (uint32_t)m_width;
    clear_rect.baseArrayLayer = 1;
    clear_rect.layerCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00017");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    // baseLayer + layerCount > view layers
    clear_rect.rect.extent.width = (uint32_t)m_width;
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearAttachments-pRects-00017");
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidVertexBindingDescriptions) {
    TEST_DESCRIPTION(
        "Attempt to create a graphics pipeline where:"
        "1) count of vertex bindings exceeds device's maxVertexInputBindings limit"
        "2) requested bindings include a duplicate binding value");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t binding_count = m_device->props.limits.maxVertexInputBindings + 1;

    std::vector<VkVertexInputBindingDescription> input_bindings(binding_count);
    for (uint32_t i = 0; i < binding_count; ++i) {
        input_bindings[i].binding = i;
        input_bindings[i].stride = 4;
        input_bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
    // Let the last binding description use same binding as the first one
    input_bindings[binding_count - 1].binding = 0;

    VkVertexInputAttributeDescription input_attrib;
    input_attrib.binding = 0;
    input_attrib.location = 0;
    input_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    input_attrib.offset = 0;

    const auto set_Info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = input_bindings.data();
        helper.vi_ci_.vertexBindingDescriptionCount = binding_count;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(
        *this, set_Info, kErrorBit,
        vector<string>{"VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                       "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616"});
}

TEST_F(VkLayerTest, InvalidVertexAttributeDescriptions) {
    TEST_DESCRIPTION(
        "Attempt to create a graphics pipeline where:"
        "1) count of vertex attributes exceeds device's maxVertexInputAttributes limit"
        "2) requested location include a duplicate location value"
        "3) binding used by one attribute is not defined by a binding description");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    input_binding.binding = 0;
    input_binding.stride = 4;
    input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    const uint32_t attribute_count = m_device->props.limits.maxVertexInputAttributes + 1;
    std::vector<VkVertexInputAttributeDescription> input_attribs(attribute_count);
    for (uint32_t i = 0; i < attribute_count; ++i) {
        input_attribs[i].binding = 0;
        input_attribs[i].location = i;
        input_attribs[i].format = VK_FORMAT_R32G32B32_SFLOAT;
        input_attribs[i].offset = 0;
    }
    // Let the last input_attribs description use same location as the first one
    input_attribs[attribute_count - 1].location = 0;
    // Let the last input_attribs description use binding which is not defined
    input_attribs[attribute_count - 1].binding = 1;

    const auto set_Info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = input_attribs.data();
        helper.vi_ci_.vertexAttributeDescriptionCount = attribute_count;
    };
    CreatePipelineHelper::OneshotTest(
        *this, set_Info, kErrorBit,
        vector<string>{"VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
                       "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                       "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617"});
}

TEST_F(VkLayerTest, ColorBlendInvalidLogicOp) {
    TEST_DESCRIPTION("Attempt to use invalid VkPipelineColorBlendStateCreateInfo::logicOp value.");

    ASSERT_NO_FATAL_FAILURE(Init());  // enables all supported features
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().logicOp) {
        printf("%s Device does not support logicOp feature; skipped.\n", kSkipPrefix);
        return;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01376");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "not consumed by fragment shader");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Type mismatch on descriptor slot 0.0 ");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Shader uses descriptor slot 0.0 ");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
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
    m_errorMonitor->ExpectSuccess();
    ASSERT_VK_SUCCESS(vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout));
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, nullptr);
    m_errorMonitor->VerifyNotFound();

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

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
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

    m_errorMonitor->ExpectSuccess();
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
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->ExpectSuccess();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize, data);
    m_errorMonitor->VerifyNotFound();

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "not written by vertex shader");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "not written by vertex shader");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Type mismatch on location 0");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Type mismatch on location 0");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
        std::vector<std::string>{
            "vertex shader writes to output location 1.0 which is not consumed by fragment shader.Enable VK_KHR_maintenance4 device extension to allow relaxed interface matching between input and output vectors.",
            "fragment shader consumes input location 0.0 which is not written by vertex shader"});
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
    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<std::string>{
            "location 0.1 which is not written by vertex shader",
            "vertex shader writes to output location 0.0 which is not consumed by fragment shader.Enable VK_KHR_maintenance4 "
            "device extension to allow relaxed interface matching between input and output vectors."});
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByPrecision) {
    TEST_DESCRIPTION("Test that the RelaxedPrecision decoration is validated to match");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out mediump float x;
        void main() { gl_Position = vec4(0); x = 1.0; }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in highp float x;
        layout(location=0) out vec4 color;
        void main() { color = vec4(x); }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "differ in precision");
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByPrecisionBlock) {
    TEST_DESCRIPTION("Test that the RelaxedPrecision decoration is validated to match");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0) mediump float x; };
        void main() { gl_Position = vec4(0); x = 1.0; }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) highp float x; };
        layout(location=0) out vec4 color;
        void main() { color = vec4(x); }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "differ in precision");
}

TEST_F(VkLayerTest, CreatePipelineAttribNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex attribute which is not consumed by the vertex shader");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit, "location 0 not consumed by vertex shader");
}

TEST_F(VkLayerTest, CreatePipelineAttribLocationMismatch) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a location mismatch on vertex attributes. This flushes out bad behavior in the "
        "interface walker");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };

    m_errorMonitor->SetUnexpectedError("Vertex shader consumes input at location 1 but not provided");
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit, "location 0 not consumed by vertex shader");
}

TEST_F(VkLayerTest, CreatePipelineAttribNotProvided) {
    TEST_DESCRIPTION("Test that an error is produced for a vertex shader input which is not provided by a vertex attribute");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x; /* not provided */
        void main(){
           gl_Position = x;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Vertex shader consumes input at location 0 but not provided");
}

TEST_F(VkLayerTest, CreatePipelineAttribTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a mismatch between the fundamental type (float/int/uint) of an attribute and the "
        "vertex shader input that consumes it");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in int x; /* attrib provided float */
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "location 0 does not match vertex shader input type");
}

TEST_F(VkLayerTest, CreatePipelineDuplicateStage) {
    TEST_DESCRIPTION("Test that an error is produced for a pipeline containing multiple shaders for the same stage");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), helper.vs_->GetStageCreateInfo(),
                                 helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stage-00726");
}

TEST_F(VkLayerTest, CreatePipelineMissingEntrypoint) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                   "foo");

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pName-00707");
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
    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), rp);

    m_errorMonitor->VerifyFound();

    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, CreatePipelineTessPatchDecorationMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a variable output from the TCS without the patch decoration, but consumed in the TES "
        "with the decoration.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().tessellationShader) {
        printf("%s Device does not support tessellation shaders; skipped.\n", kSkipPrefix);
        return;
    }

    char const *tcsSource = R"glsl(
        #version 450
        layout(location=0) out int x[];
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
           x[gl_InvocationID] = gl_InvocationID;
        }
    )glsl";
    char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        layout(location=0) patch in int x;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = x;
        }
    )glsl";
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pTessellationState = &tsci;
        helper.gp_ci_.pInputAssemblyState = &iasci;
        helper.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
        helper.shader_stages_.emplace_back(tes.GetStageCreateInfo());
    };
    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        "is per-vertex in tessellation control shader stage but per-patch in tessellation evaluation shader stage");
}

TEST_F(VkLayerTest, CreatePipelineTessErrors) {
    TEST_DESCRIPTION("Test various errors when creating a graphics pipeline with tessellation stages active.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().tessellationShader) {
        printf("%s Device does not support tessellation shaders; skipped.\n", kSkipPrefix);
        return;
    }

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
           gl_Position.w = 0;
        }
    )glsl";
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};
    VkPipelineInputAssemblyStateCreateInfo iasci_bad = iasci;
    VkPipelineInputAssemblyStateCreateInfo *p_iasci = nullptr;
    VkPipelineTessellationStateCreateInfo tsci_bad = tsci;
    VkPipelineTessellationStateCreateInfo *p_tsci = nullptr;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pTessellationState = p_tsci;
        helper.gp_ci_.pInputAssemblyState = p_iasci;
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.shader_stages_.insert(helper.shader_stages_.end(), shader_stages.begin(), shader_stages.end());
    };

    iasci_bad.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // otherwise we get a failure about invalid topology
    p_iasci = &iasci_bad;
    // Pass a tess control shader without a tess eval shader
    shader_stages = {tcs.GetStageCreateInfo()};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00729");

    // Pass a tess eval shader without a tess control shader
    shader_stages = {tes.GetStageCreateInfo()};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00730");

    p_iasci = &iasci;
    shader_stages = {};
    // Pass patch topology without tessellation shaders
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-00737");

    shader_stages = {tcs.GetStageCreateInfo(), tes.GetStageCreateInfo()};
    // Pass a NULL pTessellationState (with active tessellation shader stages)
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00731");

    // Pass an invalid pTessellationState (bad sType)
    tsci_bad.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    p_tsci = &tsci_bad;
    shader_stages = {tcs.GetStageCreateInfo(), tes.GetStageCreateInfo()};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-sType-sType");

    // Pass out-of-range patchControlPoints
    p_iasci = &iasci;
    tsci_bad = tsci;
    tsci_bad.patchControlPoints = 0;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214");

    tsci_bad.patchControlPoints = m_device->props.limits.maxTessellationPatchSize + 1;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214");

    p_tsci = &tsci;
    // Pass an invalid primitive topology
    iasci_bad = iasci;
    iasci_bad.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    p_iasci = &iasci_bad;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00736");
}

TEST_F(VkLayerTest, CreatePipelineAttribBindingConflict) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a vertex attribute setup where multiple bindings provide the same location");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    /* Two binding descriptions for binding 0 */
    VkVertexInputBindingDescription input_bindings[2];
    memset(input_bindings, 0, sizeof(input_bindings));

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* attrib provided float */
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.vi_ci_.pVertexBindingDescriptions = input_bindings;
        helper.vi_ci_.vertexBindingDescriptionCount = 2;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Attachment 0 not written by fragment shader");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit,
                                      "fragment shader writes to output location 1 with no matching attachment");
}

TEST_F(VkLayerTest, CreatePipelineFragmentNoOutputLocation0ButAlphaToCoverageEnabled) {
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
    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        "fragment shader doesn't declare alpha output at location 0 even though alpha to coverage is enabled.");
}

TEST_F(VkLayerTest, CreatePipelineFragmentNoAlphaLocation0ButAlphaToCoverageEnabled) {
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
    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        "fragment shader doesn't declare alpha output at location 0 even though alpha to coverage is enabled.");
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
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "does not match fragment shader output type");
}

TEST_F(VkLayerTest, CreatePipelineExceedVertexMaxComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpMemberDecorate built-ins");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;

    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
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

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;

    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
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
    for (int overflow = 0; overflow < 3; ++overflow) {
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
            case 2:
                // just component limit (maxVertexOutputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            case 1:
                // component and location limit (maxVertexOutputComponents)
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            default:
                assert(0);
            case 0:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
                break;
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxComponentsBlocks) {
    TEST_DESCRIPTION("Test if the max componenets checks are done properly when in a single block");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // To make the test simple, just make sure max is 128 or less (most HW is 64 or 128)
    if (m_device->props.limits.maxVertexOutputComponents > 128 || m_device->props.limits.maxFragmentInputComponents > 128) {
        printf("%s maxVertexOutputComponents or maxFragmentInputComponents too high for test; skipped.\n", kSkipPrefix);
        return;
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
                                      vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272",
                                                     "VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxTessellationControlInputOutputComponents) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of per-vertex input and/or output components to the tessellation control "
        "stage exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (int overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();
        VkPhysicalDeviceFeatures feat;
        vk::GetPhysicalDeviceFeatures(gpu(), &feat);
        if (!feat.tessellationShader) {
            printf("%s tessellation shader stage(s) unsupported.\n", kSkipPrefix);
            return;
        }

        // Tessellation control stage
        std::string tcsSourceStr =
            "#version 450\n"
            "\n";
        // Input components
        const uint32_t maxTescInComp = m_device->props.limits.maxTessellationControlPerVertexInputComponents + overflow;
        const uint32_t numInVec4 = maxTescInComp / 4;
        uint32_t inLocation = 0;
        if (overflow == 2) {
            tcsSourceStr += "layout(location=" + std::to_string(numInVec4 + 1) + ") in vec4 vnIn[];\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numInVec4; i++) {
                tcsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in vec4 v" + std::to_string(i) + "In[];\n";
                inLocation += 1;
            }
            const uint32_t inRemainder = maxTescInComp % 4;
            if (inRemainder != 0) {
                if (inRemainder == 1) {
                    tcsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in float" + " vnIn[];\n";
                } else {
                    tcsSourceStr +=
                        "layout(location=" + std::to_string(inLocation) + ") in vec" + std::to_string(inRemainder) + " vnIn[];\n";
                }
                inLocation += 1;
            }
        }

        // Output components
        const uint32_t maxTescOutComp = m_device->props.limits.maxTessellationControlPerVertexOutputComponents + overflow;
        const uint32_t numOutVec4 = maxTescOutComp / 4;
        uint32_t outLocation = 0;
        if (overflow == 2) {
            tcsSourceStr += "layout(location=" + std::to_string(numOutVec4 + 1) + ") out vec4 vnOut[3];\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numOutVec4; i++) {
                tcsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec4 v" + std::to_string(i) + "Out[3];\n";
                outLocation += 1;
            }
            const uint32_t outRemainder = maxTescOutComp % 4;
            if (outRemainder != 0) {
                if (outRemainder == 1) {
                    tcsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out float" + " vnOut[3];\n";
                } else {
                    tcsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec" + std::to_string(outRemainder) +
                                    " vnOut[3];\n";
                }
                outLocation += 1;
            }
        }

        tcsSourceStr += "layout(vertices=3) out;\n";
        // Finalize
        tcsSourceStr +=
            "\n"
            "void main(){\n"
            "}\n";

        VkShaderObj tcs(this, tcsSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        VkShaderObj tes(this, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
        inputAssemblyInfo.flags = 0;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessInfo = LvlInitStruct<VkPipelineTessellationStateCreateInfo>();
        tessInfo.flags = 0;
        tessInfo.patchControlPoints = 3;

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                                     helper.fs_->GetStageCreateInfo()};
            helper.gp_ci_.pTessellationState = &tessInfo;
            helper.gp_ci_.pInputAssemblyState = &inputAssemblyInfo;
        };
        // maxTessellationControlPerVertexInputComponents and maxTessellationControlPerVertexOutputComponents
        switch (overflow) {
            case 2:
                // in and out component limit
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            case 1:
                // (in and out component limit) and (in and out location limit)
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272",
                                   "VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            default:
                assert(0);
            case 0:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
                break;
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxTessellationEvaluationInputOutputComponents) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of input and/or output components to the tessellation evaluation stage "
        "exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (int overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();
        VkPhysicalDeviceFeatures feat;
        vk::GetPhysicalDeviceFeatures(gpu(), &feat);
        if (!feat.tessellationShader) {
            printf("%s tessellation shader stage(s) unsupported.\n", kSkipPrefix);
            return;
        }

        // Tessellation evaluation stage
        std::string tesSourceStr =
            "#version 450\n"
            "\n"
            "layout (triangles) in;\n"
            "\n";
        // Input components
        const uint32_t maxTeseInComp = m_device->props.limits.maxTessellationEvaluationInputComponents + overflow;
        const uint32_t numInVec4 = maxTeseInComp / 4;
        uint32_t inLocation = 0;
        if (overflow == 2) {
            tesSourceStr += "layout(location=" + std::to_string(numInVec4 + 1) + ") in vec4 vnIn[];\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numInVec4; i++) {
                tesSourceStr += "layout(location=" + std::to_string(inLocation) + ") in vec4 v" + std::to_string(i) + "In[];\n";
                inLocation += 1;
            }
            const uint32_t inRemainder = maxTeseInComp % 4;
            if (inRemainder != 0) {
                if (inRemainder == 1) {
                    tesSourceStr += "layout(location=" + std::to_string(inLocation) + ") in float" + " vnIn[];\n";
                } else {
                    tesSourceStr +=
                        "layout(location=" + std::to_string(inLocation) + ") in vec" + std::to_string(inRemainder) + " vnIn[];\n";
                }
                inLocation += 1;
            }
        }

        // Output components
        const uint32_t maxTeseOutComp = m_device->props.limits.maxTessellationEvaluationOutputComponents + overflow;
        const uint32_t numOutVec4 = maxTeseOutComp / 4;
        uint32_t outLocation = 0;
        if (overflow == 2) {
            tesSourceStr += "layout(location=" + std::to_string(numOutVec4 + 1) + ") out vec4 vnOut;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numOutVec4; i++) {
                tesSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec4 v" + std::to_string(i) + "Out;\n";
                outLocation += 1;
            }
            const uint32_t outRemainder = maxTeseOutComp % 4;
            if (outRemainder != 0) {
                if (outRemainder == 1) {
                    tesSourceStr += "layout(location=" + std::to_string(outLocation) + ") out float" + " vnOut;\n";
                } else {
                    tesSourceStr +=
                        "layout(location=" + std::to_string(outLocation) + ") out vec" + std::to_string(outRemainder) + " vnOut;\n";
                }
                outLocation += 1;
            }
        }

        // Finalize
        tesSourceStr +=
            "\n"
            "void main(){\n"
            "}\n";

        VkShaderObj tcs(this, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        VkShaderObj tes(this, tesSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
        inputAssemblyInfo.flags = 0;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessInfo = LvlInitStruct<VkPipelineTessellationStateCreateInfo>();
        tessInfo.flags = 0;
        tessInfo.patchControlPoints = 3;

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                                     helper.fs_->GetStageCreateInfo()};
            helper.gp_ci_.pTessellationState = &tessInfo;
            helper.gp_ci_.pInputAssemblyState = &inputAssemblyInfo;
        };

        // maxTessellationEvaluationInputComponents and maxTessellationEvaluationOutputComponents
        switch (overflow) {
            case 2:
                // in and out component limit
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            case 1:
                // (in and out component limit) and (in and out location limit)
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272",
                                   "VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            default:
                assert(0);
            case 0:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
                break;
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxGeometryInputOutputComponents) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of input and/or output components to the geometry stage exceeds the device "
        "limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (int overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();
        VkPhysicalDeviceFeatures feat;
        vk::GetPhysicalDeviceFeatures(gpu(), &feat);
        if (!feat.geometryShader) {
            printf("%s geometry shader stage unsupported.\n", kSkipPrefix);
            return;
        }

        std::string gsSourceStr =
            "#version 450\n"
            "\n"
            "layout(triangles) in;\n"
            "layout(invocations=1) in;\n";

        // Input components
        const uint32_t maxGeomInComp = m_device->props.limits.maxGeometryInputComponents + overflow;
        const uint32_t numInVec4 = maxGeomInComp / 4;
        uint32_t inLocation = 0;
        if (overflow == 2) {
            gsSourceStr += "layout(location=" + std::to_string(numInVec4 + 1) + ") in vec4 vnIn[];\n";
        } else {
            for (uint32_t i = 0; i < numInVec4; i++) {
                gsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in vec4 v" + std::to_string(i) + "In[];\n";
                inLocation += 1;
            }
            const uint32_t inRemainder = maxGeomInComp % 4;
            if (inRemainder != 0) {
                if (inRemainder == 1) {
                    gsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in float" + " vnIn[];\n";
                } else {
                    gsSourceStr +=
                        "layout(location=" + std::to_string(inLocation) + ") in vec" + std::to_string(inRemainder) + " vnIn[];\n";
                }
                inLocation += 1;
            }
        }

        // Output components
        const uint32_t maxGeomOutComp = m_device->props.limits.maxGeometryOutputComponents + overflow;
        const uint32_t numOutVec4 = maxGeomOutComp / 4;
        uint32_t outLocation = 0;
        if (overflow == 2) {
            gsSourceStr += "layout(location=" + std::to_string(numOutVec4) + ") out vec4 vnOut;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numOutVec4; i++) {
                gsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec4 v" + std::to_string(i) + "Out;\n";
                outLocation += 1;
            }
            const uint32_t outRemainder = maxGeomOutComp % 4;
            if (outRemainder != 0) {
                if (outRemainder == 1) {
                    gsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out float" + " vnOut;\n";
                } else {
                    gsSourceStr +=
                        "layout(location=" + std::to_string(outLocation) + ") out vec" + std::to_string(outRemainder) + " vnOut;\n";
                }
                outLocation += 1;
            }
        }

        // Finalize
        int max_vertices = overflow ? (m_device->props.limits.maxGeometryTotalOutputComponents / maxGeomOutComp + 1) : 1;
        gsSourceStr += "layout(triangle_strip, max_vertices = " + std::to_string(max_vertices) +
                       ") out;\n"
                       "\n"
                       "void main(){\n"
                       "}\n";

        VkShaderObj gs(this, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        // maxGeometryInputComponents and maxGeometryOutputComponents
        switch (overflow) {
            case 2:
                // in and out component limit
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            case 1:
                // (in and out component limit) and (in and out location limit) and maxGeometryTotalOutputComponents
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272",
                                   "VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272",
                                   "VUID-RuntimeSpirv-Location-06272"});
                break;
            default:
                assert(0);
            case 0:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
                break;
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxFragmentInputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of input components from the fragment stage exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (int overflow = 0; overflow < 3; ++overflow) {
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

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        switch (overflow) {
            case 2:
                // just component limit (maxFragmentInputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            case 1:
                // component and location limit (maxFragmentInputComponents)
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
                break;
            default:
                assert(0);
            case 0:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
                break;
        }
    }
}

TEST_F(VkLayerTest, CreatePipelineExceedMaxGeometryInstanceVertexCount) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of output vertices/instances in the geometry stage exceeds the device "
        "limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    for (int overflow = 0; overflow < 2; ++overflow) {
        m_errorMonitor->Reset();
        VkPhysicalDeviceFeatures feat;
        vk::GetPhysicalDeviceFeatures(gpu(), &feat);
        if (!feat.geometryShader) {
            printf("%s geometry shader stage unsupported.\n", kSkipPrefix);
            return;
        }

        std::string gsSourceStr = R"(
               OpCapability Geometry
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main"
               OpExecutionMode %main InputPoints
               OpExecutionMode %main OutputTriangleStrip
               )";
        if (overflow) {
            gsSourceStr += "OpExecutionMode %main Invocations " +
                           std::to_string(m_device->props.limits.maxGeometryShaderInvocations + 1) +
                           "\n\
                OpExecutionMode %main OutputVertices " +
                           std::to_string(m_device->props.limits.maxGeometryOutputVertices + 1);
        } else {
            gsSourceStr += R"(
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputVertices 1
               )";
        }
        gsSourceStr += R"(
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";
        VkShaderObj gs(this, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        if (overflow) {
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vector<string>{"VUID-VkPipelineShaderStageCreateInfo-stage-00714",
                                                             "VUID-VkPipelineShaderStageCreateInfo-stage-00715"});
        } else {
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineInputAttachmentMissing) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming an input attachment which is not included in the subpass "
        "description");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "consumes input attachment index 0 but not provided in subpass");
}

TEST_F(VkLayerTest, CreatePipelineInputAttachmentTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming an input attachment with a format having a different fundamental "
        "type");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "input attachment 0 format of VK_FORMAT_R8G8B8A8_UINT does not match");

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
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});

    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkAttachmentDescription descs[2] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UINT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkAttachmentReference color = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference input = {
        1,
        VK_IMAGE_LAYOUT_GENERAL,
    };

    VkSubpassDescription sd = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &input, 1, &color, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, descs, 1, &sd, 0, nullptr};
    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // error here.
    pipe.CreateVKPipeline(pl.handle(), rp);

    m_errorMonitor->VerifyFound();

    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, CreatePipelineInputAttachmentMissingArray) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming an input attachment which is not included in the subpass "
        "description -- array case");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[1];
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(xs[0]);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "consumes input attachment index 0 but not provided in subpass");
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Shader uses descriptor slot 0.0");
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
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                             "but descriptor of type VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER");
}

TEST_F(VkLayerTest, MultiplePushDescriptorSets) {
    TEST_DESCRIPTION("Verify an error message for multiple push descriptor sets.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s Push Descriptors Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
        return;
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (VK_TRUE != device_features.sampleRateShading) {
        printf("%s Test requires unsupported sampleRateShading feature.\n", kSkipPrefix);
        return;
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
         "VUID-VkGraphicsPipelineCreateInfo-subpass-00757"},
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
         "VUID-VkGraphicsPipelineCreateInfo-subpass-00757"}};

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

        VkRenderPass rp;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-pDepthStencilAttachment-01418");
        VkResult err = vk::CreateRenderPass(m_device->device(), &rpi, nullptr, &rp);
        m_errorMonitor->VerifyNotFound();

        ASSERT_VK_SUCCESS(err);

        auto ds = lvl_init_struct<VkPipelineDepthStencilStateCreateInfo>();
        auto cmi = lvl_init_struct<VkPipelineCoverageModulationStateCreateInfoNV>();

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

            helper.gp_ci_.renderPass = rp;
            helper.gp_ci_.pDepthStencilState = &ds;
        };

        CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit, test_case.vuid, test_case.positiveTest);

        vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    }
}

TEST_F(VkLayerTest, FramebufferMixedSamples) {
    TEST_DESCRIPTION("Verify that the expected VUIds are hits when VK_NV_framebuffer_mixed_samples is disabled.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

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

        VkRenderPass rp;

        if (test_case.color_samples == test_case.depth_samples) {
            m_errorMonitor->ExpectSuccess();
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-pDepthStencilAttachment-01418");
        }

        VkResult err = vk::CreateRenderPass(m_device->device(), &rpi, nullptr, &rp);

        if (test_case.color_samples == test_case.depth_samples) {
            m_errorMonitor->VerifyNotFound();
        } else {
            m_errorMonitor->VerifyFound();
            continue;
        }

        ASSERT_VK_SUCCESS(err);

        VkPipelineDepthStencilStateCreateInfo ds = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

        const auto break_samples = [&rp, &ds, &test_case](CreatePipelineHelper &helper) {
            helper.pipe_ms_state_ci_.rasterizationSamples = test_case.raster_samples;

            helper.gp_ci_.renderPass = rp;
            helper.gp_ci_.pDepthStencilState = &ds;
        };

        CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-subpass-00757",
                                          test_case.positiveTest);

        vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    }
}

TEST_F(VkLayerTest, FramebufferMixedSamplesCoverageReduction) {
    TEST_DESCRIPTION("Verify VK_NV_coverage_reduction_mode.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    } else if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    } else {
        printf("%s Neither %s nor %s are supported, skipping tests\n", kSkipPrefix, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME,
               VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
        return;
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
        printf("%s No mixed sample combinations are supported, skipping tests.\n", kSkipPrefix);
        return;
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

        CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit, test_case.vuid, test_case.positiveTest);
    }
}

TEST_F(VkLayerTest, FragmentCoverageToColorNV) {
    TEST_DESCRIPTION("Verify VK_NV_fragment_coverage_to_color.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
        return;
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

        CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit,
                                          "VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404",
                                          test_case.positive);
    }
}

TEST_F(VkLayerTest, ViewportSwizzleNV) {
    TEST_DESCRIPTION("Verify VK_NV_viewprot_swizzle.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkViewportSwizzleNV invalid_swizzles = {
        VkViewportCoordinateSwizzleNV(-1),
        VkViewportCoordinateSwizzleNV(-1),
        VkViewportCoordinateSwizzleNV(-1),
        VkViewportCoordinateSwizzleNV(-1),
    };

    VkPipelineViewportSwizzleStateCreateInfoNV vp_swizzle_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV};
    vp_swizzle_state.viewportCount = 1;
    vp_swizzle_state.pViewportSwizzles = &invalid_swizzles;

    const std::vector<std::string> expected_vuids = {"VUID-VkViewportSwizzleNV-x-parameter", "VUID-VkViewportSwizzleNV-y-parameter",
                                                     "VUID-VkViewportSwizzleNV-z-parameter",
                                                     "VUID-VkViewportSwizzleNV-w-parameter"};

    auto break_swizzles = [&vp_swizzle_state](CreatePipelineHelper &helper) { helper.vp_state_ci_.pNext = &vp_swizzle_state; };

    CreatePipelineHelper::OneshotTest(*this, break_swizzles, kErrorBit, expected_vuids);

    struct TestCase {
        VkBool32 rasterizerDiscardEnable;
        uint32_t vp_count;
        uint32_t swizzel_vp_count;
        bool positive;
    };

    const std::array<TestCase, 3> test_cases = {{{VK_TRUE, 1, 2, true}, {VK_FALSE, 1, 1, true}, {VK_FALSE, 1, 2, false}}};

    std::array<VkViewportSwizzleNV, 2> swizzles = {
        {{VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
          VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV},
         {VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
          VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV}}};

    for (const auto &test_case : test_cases) {
        assert(test_case.vp_count <= swizzles.size());

        vp_swizzle_state.viewportCount = test_case.swizzel_vp_count;
        vp_swizzle_state.pViewportSwizzles = swizzles.data();

        auto break_vp_count = [&vp_swizzle_state, &test_case](CreatePipelineHelper &helper) {
            helper.rs_state_ci_.rasterizerDiscardEnable = test_case.rasterizerDiscardEnable;
            helper.vp_state_ci_.viewportCount = test_case.vp_count;

            helper.vp_state_ci_.pNext = &vp_swizzle_state;
        };

        CreatePipelineHelper::OneshotTest(*this, break_vp_count, kErrorBit,
                                          "VUID-VkPipelineViewportSwizzleStateCreateInfoNV-viewportCount-01215",
                                          test_case.positive);
    }
}

TEST_F(VkLayerTest, CooperativeMatrixNV) {
    TEST_DESCRIPTION("Test VK_NV_cooperative_matrix.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 2> required_device_extensions = {
        {VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME)) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto cooperative_matrix_features = LvlInitStruct<VkPhysicalDeviceCooperativeMatrixFeaturesNV>(&float16_features);
    auto memory_model_features = LvlInitStruct<VkPhysicalDeviceVulkanMemoryModelFeaturesKHR>(&cooperative_matrix_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&memory_model_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (memory_model_features.vulkanMemoryModel == VK_FALSE) {
        printf("%s vulkanMemoryModel feature not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_NV_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        layout(constant_id = 0) const uint C0 = 1;
        layout(constant_id = 1) const uint C1 = 1;
        void main() {
           // Bad type
           fcoopmatNV<16, gl_ScopeSubgroup, 3, 5> badSize = fcoopmatNV<16, gl_ScopeSubgroup, 3, 5>(float16_t(0.0));
           // Not a valid multiply when C0 != C1
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> A;
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> B;
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> C;
           coopMatMulAddNV(A, B, C);
        }
    )glsl";

    const uint32_t specData[] = {
        16,
        8,
    };
    VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };

    VkSpecializationInfo specInfo = {
        2,
        entries,
        sizeof(specData),
        specData,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06719");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixType");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixMulAdd");
    m_errorMonitor->SetUnexpectedError("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06719");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SubgroupSupportedProperties) {
    TEST_DESCRIPTION(
        "Test shader validation support for subgroup VkPhysicalDeviceSubgroupProperties such as supportedStages, and "
        "supportedOperations, quadOperationsInAllStages.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());
    // Don't enable the extenion on purpose
    const bool extension_support_partitioned =
        DeviceExtensionSupported(gpu(), nullptr, VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // 1.1 and up only.
    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s Vulkan 1.1 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s DevSim doesn't support Vulkan 1.1, skipping tests\n", kSkipPrefix);
        return;
    }

    // Gather all aspects supported
    VkPhysicalDeviceSubgroupProperties subgroup_prop = GetSubgroupProperties(instance(), gpu());
    VkSubgroupFeatureFlags subgroup_operations = subgroup_prop.supportedOperations;
    const bool feature_support_basic = ((subgroup_operations & VK_SUBGROUP_FEATURE_BASIC_BIT) != 0);
    const bool feature_support_vote = ((subgroup_operations & VK_SUBGROUP_FEATURE_VOTE_BIT) != 0);
    const bool feature_support_arithmetic = ((subgroup_operations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) != 0);
    const bool feature_support_ballot = ((subgroup_operations & VK_SUBGROUP_FEATURE_BALLOT_BIT) != 0);
    const bool feature_support_shuffle = ((subgroup_operations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) != 0);
    const bool feature_support_relative = ((subgroup_operations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) != 0);
    const bool feature_support_culstered = ((subgroup_operations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) != 0);
    const bool feature_support_quad = ((subgroup_operations & VK_SUBGROUP_FEATURE_QUAD_BIT) != 0);
    const bool feature_support_partitioned = ((subgroup_operations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) != 0);
    const bool vertex_support = ((subgroup_prop.supportedStages & VK_SHADER_STAGE_VERTEX_BIT) != 0);
    const bool vertex_quad_support = (subgroup_prop.quadOperationsInAllStages == VK_TRUE);

    std::string vsSource;
    std::vector<const char *> errors;
    // There is no 'supportedOperations' check due to it would be redundant to the Capability check done first in VUID 01091 since
    // each 'supportedOperations' flag is 1:1 map to a SPIR-V Capability
    const char *operation_vuid = "VUID-VkShaderModuleCreateInfo-pCode-01091";
    const char *stage_vuid = "VUID-RuntimeSpirv-None-06343";
    const char *quad_vuid = "VUID-RuntimeSpirv-None-06342";

    // Same pipeline creation for each subgroup test
    auto info_override = [&](CreatePipelineHelper &info) {
        info.vs_.reset(new VkShaderObj(this, vsSource.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1));
        info.shader_stages_ = {info.vs_->GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
        info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    // Basic
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_basic: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               if (subgroupElect()) { ssbo.x += 2.0; }
               gl_Position = vec4(ssbo.x);
            }
        )glsl";
        errors.clear();
        if (feature_support_basic == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Vote
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_vote: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               if (subgroupAll(ssbo.y == 0)) { ssbo.x += 2.0; }
               gl_Position = vec4(ssbo.x);
            }
        )glsl";
        errors.clear();
        if (feature_support_vote == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Arithmetic
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_arithmetic: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupMax(ssbo.x);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_arithmetic == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Ballot
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_ballot: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupBroadcastFirst(ssbo.x);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_ballot == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Shuffle
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_shuffle: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupShuffle(ssbo.x, 1);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_shuffle == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Shuffle Relative
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_shuffle_relative: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupShuffleUp(ssbo.x, 1);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_relative == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Clustered
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_clustered: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupClusteredAdd(ssbo.x, 2);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_culstered == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Quad
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_quad: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupQuadSwapHorizontal(ssbo.x);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_quad == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_quad_support == false) {
            errors.push_back(quad_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }

    // Partitoned
    if (extension_support_partitioned) {
        vsSource = R"glsl(
            #version 450
            #extension GL_NV_shader_subgroup_partitioned: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               uvec4 a = subgroupPartitionNV(ssbo.x); // forces OpGroupNonUniformPartitionNV
               gl_Position = vec4(float(a.x));
            }
        )glsl";
        errors.clear();
        // Extension not enabled on purpose if supported
        errors.push_back("VUID-VkShaderModuleCreateInfo-pCode-04147");
        if (feature_support_partitioned == false) {
            // errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors, /*positive_test*/ (errors.size() == 0));
    }
}

TEST_F(VkLayerTest, SubgroupRequired) {
    TEST_DESCRIPTION("Test that the minimum required functionality for subgroups is present.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    // 1.1 and up only.
    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s Vulkan 1.1 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s DevSim doesn't support Vulkan 1.1, skipping tests\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceSubgroupProperties subgroup_prop = GetSubgroupProperties(instance(), gpu());

    auto queue_family_properties = m_device->phy().queue_properties();

    bool foundGraphics = false;
    bool foundCompute = false;

    for (auto queue_family : queue_family_properties) {
        if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            foundCompute = true;
            break;
        }

        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            foundGraphics = true;
        }
    }

    if (!(foundGraphics || foundCompute)) return;

    ASSERT_GE(subgroup_prop.subgroupSize, 1u);

    if (foundCompute) {
        ASSERT_TRUE(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT);
    }

    ASSERT_TRUE(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT);
}

TEST_F(VkLayerTest, SubgroupExtendedTypesEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 2> required_device_extensions = {
        {VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto extended_types_features = LvlInitStruct<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(&float16_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&extended_types_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    VkPhysicalDeviceSubgroupProperties subgroup_prop = GetSubgroupProperties(instance(), gpu());
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) || !float16_features.shaderFloat16 ||
        !extended_types_features.shaderSubgroupExtendedTypes) {
        printf("%s Required features not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_arithmetic : enable
        #extension GL_EXT_shader_subgroup_extended_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
           subgroupAdd(float16_t(0.0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, SubgroupExtendedTypesDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 2> required_device_extensions = {
        {VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto extended_types_features = LvlInitStruct<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(&float16_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&extended_types_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    VkPhysicalDeviceSubgroupProperties subgroup_prop = GetSubgroupProperties(instance(), gpu());
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) || !float16_features.shaderFloat16) {
        printf("%s Required features not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    // Disabled extended types support, and expect an error
    extended_types_features.shaderSubgroupExtendedTypes = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_arithmetic : enable
        #extension GL_EXT_shader_subgroup_extended_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
           subgroupAdd(float16_t(0.0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-None-06275");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NonSemanticInfoEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_non_semantic_info.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this test. \n", kSkipPrefix,
               VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
        return;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-04147");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GraphicsPipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test graphics pipeline feedback stage count check.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto feedback_info = LvlInitStruct<VkPipelineCreationFeedbackCreateInfoEXT>();
    VkPipelineCreationFeedbackEXT feedbacks[3] = {};
    // Set flags to known value that the driver has to overwrite
    feedbacks[0].flags = VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM;

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &feedback_info; };

    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-pipelineStageCreationFeedbackCount-06594", true);

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Driver data writeback check not supported by MockICD, skipping.\n", kSkipPrefix);
    } else {
        m_errorMonitor->ExpectSuccess();
        if (feedback_info.pPipelineCreationFeedback->flags == VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM) {
            m_errorMonitor->SetError("ValidationLayers did not return GraphicsPipelineFeedback driver data properly.");
        }
        m_errorMonitor->VerifyNotFound();
    }

    feedback_info.pipelineStageCreationFeedbackCount = 1;
    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-pipelineStageCreationFeedbackCount-06594", false);
}

TEST_F(VkLayerTest, ComputePipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test compute pipeline feedback stage count check.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineCreationFeedbackCreateInfoEXT feedback_info = LvlInitStruct<VkPipelineCreationFeedbackCreateInfoEXT>();
    VkPipelineCreationFeedbackEXT feedbacks[3] = {};
    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 1;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    const auto set_info = [&](CreateComputePipelineHelper &helper) { helper.cp_ci_.pNext = &feedback_info; };

    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                             "VUID-VkComputePipelineCreateInfo-pipelineStageCreationFeedbackCount-06566");
}

TEST_F(VkLayerTest, NVRayTracingPipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test NV ray tracing pipeline feedback stage count check.");

    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto rtnv_props = LvlInitStruct<VkPhysicalDeviceRayTracingPropertiesNV>();
    GetPhysicalDeviceProperties2(rtnv_props);

    if (rtnv_props.maxDescriptorSetAccelerationStructures < 1) {
        GTEST_SKIP() << "maxDescriptorSetAccelerationStructures < 1";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto feedback_info = LvlInitStruct<VkPipelineCreationFeedbackCreateInfoEXT>();
    VkPipelineCreationFeedbackEXT feedbacks[4] = {};

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](CreateNVRayTracingPipelineHelper &helper) { helper.rp_ci_.pNext = &feedback_info; };

    feedback_info.pipelineStageCreationFeedbackCount = 3;
    CreateNVRayTracingPipelineHelper::OneshotPositiveTest(*this, set_feedback);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    CreateNVRayTracingPipelineHelper::OneshotTest(*this, set_feedback,
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderImageFootprintEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the shader image footprint feature which has not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME)) {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME);
        return;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-04147");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckComputeShaderDerivativesEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the compute shader derivatives feature which has not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)) {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
        return;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-04147");
    VkPipeline pipe = VK_NULL_HANDLE;
    vk::CreateComputePipelines(test_device.device(), VK_NULL_HANDLE, 1, &cpci, nullptr, &pipe);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipeline(test_device.device(), pipe, nullptr);
    m_errorMonitor->VerifyNotFound();
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
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-04147");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckLineRasterization) {
    TEST_DESCRIPTION("Test VK_EXT_line_rasterization state against feature enables.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto line_rasterization_features = LvlInitStruct<VkPhysicalDeviceLineRasterizationFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&line_rasterization_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    line_rasterization_features.rectangularLines = VK_FALSE;
    line_rasterization_features.bresenhamLines = VK_FALSE;
    line_rasterization_features.smoothLines = VK_FALSE;
    line_rasterization_features.stippledRectangularLines = VK_FALSE;
    line_rasterization_features.stippledBresenhamLines = VK_FALSE;
    line_rasterization_features.stippledSmoothLines = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper::OneshotTest(
        *this,
        [&](CreatePipelineHelper &helper) {
            helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
            helper.pipe_ms_state_ci_.alphaToCoverageEnable = VK_TRUE;
        },
        kErrorBit,
        std::vector<const char *>{"VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769"});

    CreatePipelineHelper::OneshotTest(
        *this,
        [&](CreatePipelineHelper &helper) {
            helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
            helper.line_state_ci_.stippledLineEnable = VK_TRUE;
        },
        kErrorBit,
        std::vector<const char *>{"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02772"});

    CreatePipelineHelper::OneshotTest(
        *this,
        [&](CreatePipelineHelper &helper) {
            helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
            helper.line_state_ci_.stippledLineEnable = VK_TRUE;
        },
        kErrorBit,
        std::vector<const char *>{"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02768",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02771"});

    CreatePipelineHelper::OneshotTest(
        *this,
        [&](CreatePipelineHelper &helper) {
            helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
            helper.line_state_ci_.stippledLineEnable = VK_TRUE;
        },
        kErrorBit,
        std::vector<const char *>{"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02770",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02773"});

    CreatePipelineHelper::OneshotTest(
        *this,
        [&](CreatePipelineHelper &helper) {
            helper.line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
            helper.line_state_ci_.stippledLineEnable = VK_TRUE;
        },
        kErrorBit,
        std::vector<const char *>{"VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                  "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02774"});

    PFN_vkCmdSetLineStippleEXT vkCmdSetLineStippleEXT =
        (PFN_vkCmdSetLineStippleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLineStippleEXT");
    ASSERT_TRUE(vkCmdSetLineStippleEXT != nullptr);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776");
    vkCmdSetLineStippleEXT(m_commandBuffer->handle(), 0, 0);
    m_errorMonitor->VerifyFound();
    vkCmdSetLineStippleEXT(m_commandBuffer->handle(), 1, 1);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, FillRectangleNV) {
    TEST_DESCRIPTION("Verify VK_NV_fill_rectangle");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    // Disable non-solid fill modes to make sure that the usage of VK_POLYGON_MODE_LINE and
    // VK_POLYGON_MODE_POINT will cause an error when the VK_NV_fill_rectangle extension is enabled.
    device_features.fillModeNonSolid = VK_FALSE;

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_FILL_RECTANGLE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_FILL_RECTANGLE_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_NV_FILL_RECTANGLE_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPolygonMode polygon_mode = VK_POLYGON_MODE_LINE;

    auto set_polygon_mode = [&polygon_mode](CreatePipelineHelper &helper) { helper.rs_state_ci_.polygonMode = polygon_mode; };

    // Set unsupported polygon mode VK_POLYGON_MODE_LINE
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507", false);

    // Set unsupported polygon mode VK_POLYGON_MODE_POINT
    polygon_mode = VK_POLYGON_MODE_POINT;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507", false);

    // Set supported polygon mode VK_POLYGON_MODE_FILL
    polygon_mode = VK_POLYGON_MODE_FILL;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507", true);

    // Set supported polygon mode VK_POLYGON_MODE_FILL_RECTANGLE_NV
    polygon_mode = VK_POLYGON_MODE_FILL_RECTANGLE_NV;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507", true);
}

TEST_F(VkLayerTest, NotCompatibleForSet) {
    TEST_DESCRIPTION("Check that validation path catches pipeline layout inconsistencies for bind vs. dispatch");
    m_errorMonitor->ExpectSuccess();
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
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-02697");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-PipelineLayoutsIncompatible");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyPipeline(device(), c_pipeline, nullptr);
}

TEST_F(VkLayerTest, RayTracingPipelineShaderGroupsNV) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_RAY_TRACING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NV_RAY_TRACING_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_NV);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_NV);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_NV);

    m_errorMonitor->VerifyNotFound();

    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesNV"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesNV != nullptr);

    VkPipeline pipeline = VK_NULL_HANDLE;

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_info.generalShader = VK_SHADER_UNUSED_NV;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Two raygen stages
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[1].module = rgen_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = 1;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyNotFound();
        vk::DestroyPipeline(m_device->device(), pipeline, NULL);
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 5;  // Invalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 5;  // Invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateRayTracingPipelineNV) {
    TEST_DESCRIPTION("Validate vkCreateRayTracingPipelinesNV and CreateInfo parameters during ray-tracing pipeline creation");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_RAY_TRACING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NV_RAY_TRACING_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto pipleline_features = LvlInitStruct<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&pipleline_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    // Set this to true as it is a required feature
    pipleline_features.pipelineCreationCacheControl = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_NV);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_NV);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_NV);
    m_errorMonitor->VerifyNotFound();
    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesNV"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesNV != nullptr);
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    ;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoNV group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
    group_create_info.generalShader = VK_SHADER_UNUSED_NV;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        pipeline_ci.basePipelineIndex = -1;
        uint64_t fake_pipeline_id = 0xCADECADE;
        VkPipeline fake_pipeline_handle = reinterpret_cast<VkPipeline &>(fake_pipeline_id);
        pipeline_ci.basePipelineHandle = fake_pipeline_handle;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03421");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesNV-flags-03415");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03422");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02957");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02904");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03456");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03458");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03459");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03460");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03461");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03462");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03463");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03588");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-04948");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    // test for vkCreateRayTracingPipelinesNV
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        // appending twice as it is generated twice in auto-validation code
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCreateRayTracingPipelinesNV-createInfoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCreateRayTracingPipelinesNV-createInfoCount-arraylength");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 0, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}
TEST_F(VkLayerTest, RayTracingPipelineCreateInfoKHR) {
    TEST_DESCRIPTION("Validate CreateInfo parameters during ray-tracing pipeline creation");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!ray_tracing_features.rayTracingPipeline) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_KHR);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_KHR);
    m_errorMonitor->VerifyNotFound();
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 1;  // Bad index here
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;
    VkPipelineLibraryCreateInfoKHR library_count_zero = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 0};
    VkPipelineLibraryCreateInfoKHR library_count_one = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 1};
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.stageCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03600");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.stageCount = 1;
        pipeline_ci.groupCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03601");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.groupCount = 1;
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_one;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetUnexpectedError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        pipeline_ci.basePipelineIndex = -1;
        uint64_t fake_pipeline_id = 0xCADECADE;
        VkPipeline fake_pipeline_handle = reinterpret_cast<VkPipeline &>(fake_pipeline_id);
        pipeline_ci.basePipelineHandle = fake_pipeline_handle;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03421");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03422");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDynamicState dynamic_state = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        VkPipelineDynamicStateCreateInfo dynamic_states = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dynamic_states.dynamicStateCount = 1;
        dynamic_states.pDynamicStates = &dynamic_state;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pDynamicState = &dynamic_states;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesKHR-flags-03816");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingPipelineShaderGroupsKHR) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!ray_tracing_features.rayTracingPipeline) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_KHR, SPV_ENV_VULKAN_1_2);

    m_errorMonitor->VerifyNotFound();

    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLibraryCreateInfoKHR library_info = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 0};

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 5;  // IKHRalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 5;  // invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, PipelineStageConditionalRenderingWithWrongQueue) {
    TEST_DESCRIPTION("Run CmdPipelineBarrier with VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT and wrong VkQueueFlagBits");
    ASSERT_NO_FATAL_FAILURE(Init());
    uint32_t only_transfer_queueFamilyIndex = UINT32_MAX;

    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    for (uint32_t i = 0; i < (uint32_t)q_props.size(); i++) {
        if (q_props[i].queueFlags == VK_QUEUE_TRANSFER_BIT) {
            only_transfer_queueFamilyIndex = i;
            break;
        }
    }

    if (only_transfer_queueFamilyIndex == UINT32_MAX) {
        printf("%s Only VK_QUEUE_TRANSFER_BIT Queue is not supported.\n", kSkipPrefix);
        return;
    }

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                      VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
                                      (VkDependencyFlags)0};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 1, &dependency};
    VkRenderPass rp;

    vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);

    VkCommandPoolObj commandPool(m_device, only_transfer_queueFamilyIndex);
    VkCommandBufferObj commandBuffer(m_device, &commandPool);

    commandBuffer.begin();
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};
    vk::CmdBeginRenderPass(commandBuffer.handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

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

    vk::CmdEndRenderPass(commandBuffer.handle());
    commandBuffer.end();
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
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
                                          "VUID-VkShaderModuleCreateInfo-pCode-01091");
    }

    {
        std::string const capability{"OpCapability SampledImageArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-01091");
    }

    {
        std::string const capability{"OpCapability StorageBufferArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-01091");
    }

    {
        std::string const capability{"OpCapability StorageImageArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-01091");
    }
}

TEST_F(VkLayerTest, VertexStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if vertexPipelineStoresAndAtomics disable.");

    VkPhysicalDeviceFeatures features{};
    features.vertexPipelineStoresAndAtomics = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test StoreOp
    {
        char const *vsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageStore(si0, ivec2(0), vec4(0));
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-RuntimeSpirv-NonWritable-06341");
    }

    // Test AtomicOp
    {
        char const *vsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageAtomicExchange(si0, ivec2(0), 1);
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
                info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
            };

            // extra VU for not enabling atomic float support
            CreatePipelineHelper::OneshotTest(
                *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                std::vector<string>{"VUID-RuntimeSpirv-None-06282", "VUID-RuntimeSpirv-NonWritable-06341"});
        }
    }
}

TEST_F(VkLayerTest, FragmentStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if fragmentStoresAndAtomics disable.");

    VkPhysicalDeviceFeatures features{};
    features.fragmentStoresAndAtomics = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test StoreOp
    {
        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageStore(si0, ivec2(0), vec4(0));
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-RuntimeSpirv-NonWritable-06340");
    }

    // Test AtomicOp
    {
        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageAtomicExchange(si0, ivec2(0), 1);
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
        if (VK_SUCCESS == fs.InitFromGLSLTry()) {
            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
                info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
            };

            // extra VU for not enabling atomic float support
            CreatePipelineHelper::OneshotTest(
                *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                std::vector<string>{"VUID-RuntimeSpirv-None-06282", "VUID-RuntimeSpirv-NonWritable-06340"});
        }
    }
}

TEST_F(VkLayerTest, DuplicateDynamicStates) {
    TEST_DESCRIPTION("Create a pipeline with duplicate dynamic states set.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_states[4] = {VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                                        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE};

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.dyn_state_ci_ = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    pipe.dyn_state_ci_.flags = 0;
    pipe.dyn_state_ci_.dynamicStateCount = 4;
    pipe.dyn_state_ci_.pDynamicStates = dynamic_states;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Should error twice since 2 sets of duplicates now
    dynamic_states[3] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NonGraphicsDynamicStates) {
    TEST_DESCRIPTION("Create a pipeline with non graphics dynamic states set.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_state = VK_DYNAMIC_STATE_MAX_ENUM;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.dyn_state_ci_ = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    pipe.dyn_state_ci_.flags = 0;
    pipe.dyn_state_ci_.dynamicStateCount = 1;
    pipe.dyn_state_ci_.pDynamicStates = &dynamic_state;

    dynamic_state = VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03578");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineMaxPerStageResources) {
    TEST_DESCRIPTION("Check case where pipeline is created that exceeds maxPerStageResources");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
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
    m_errorMonitor->ExpectSuccess();
    graphics_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();

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

TEST_F(VkLayerTest, ValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR) {
    TEST_DESCRIPTION("Validate vkGetRayTracingCaptureReplayShaderGroupHandlesKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    auto rt_pipeline_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    rt_pipeline_features.rayTracingPipelineShaderGroupHandleCaptureReplay = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&rt_pipeline_features);
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor, false,
                                        false, false, &features2)) {
        return;
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_tracing_features.rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE) {
        printf("%s rayTracingShaderGroupHandleCaptureReplay not enabled.\n", kSkipPrefix);
        return;
    }
    CreateNVRayTracingPipelineHelper rt_pipe(*this);
    rt_pipe.InitInfo(true /*isKHR*/);
    rt_pipe.rp_ci_KHR_.flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
    rt_pipe.InitState();
    VkResult err = rt_pipe.CreateKHRRayTracingPipeline();
    ASSERT_VK_SUCCESS(err);

    VkBuffer buffer;
    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = 4096;
    VkDeviceMemory mem;
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(device(), buffer, mem, 0);
    PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR vkGetRayTracingCaptureReplayShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-arraylength");
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 1, 1, 0, &buffer);
    m_errorMonitor->VerifyFound();

    // dataSize must be at least VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleCaptureReplaySize
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    // Check only when the reported size is
    if (ray_tracing_properties.shaderGroupHandleCaptureReplaySize > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
        vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 1, 1,
                                                          (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
        m_errorMonitor->VerifyFound();
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483");
    // In CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo rp_ci_KHR_.groupCount = groups_KHR_.size();
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 2, rt_pipe.groups_KHR_.size(),
                                                      (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-04051");
    // In CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo rp_ci_KHR_.groupCount = groups_KHR_.size();
    uint32_t invalid_firstgroup = rt_pipe.groups_KHR_.size() + 1;
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, invalid_firstgroup, 0,
                                                      (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidatePipelineExecutablePropertiesFeature) {
    TEST_DESCRIPTION("Try making calls without pipelineExecutableInfo.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
        return;
    }

    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_features =
        LvlInitStruct<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>();
    pipeline_exe_features.pipelineExecutableInfo = VK_FALSE;  // Starting with it off

    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&pipeline_exe_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // MockICD will return 0 for the executable count
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
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

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
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
    CreatePipelineHelper::OneshotTest(*this, validPipeline, kErrorBit | kWarningBit, "", true);

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

TEST_F(VkLayerTest, InvalidFragmentShadingRatePipeline) {
    TEST_DESCRIPTION("Specify invalid fragment shading rate values");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    fsr_features.pipelineFragmentShadingRate = true;

    VkPhysicalDeviceFeatures2 device_features = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fsr_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    fsr_ci.fragmentSize.width = 0;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04494");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 0;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04495");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.fragmentSize.width = 3;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04496");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 3;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04497");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.fragmentSize.width = 8;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04498");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 8;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04499");
    fsr_ci.fragmentSize.height = 1;
}

TEST_F(VkLayerTest, InvalidFragmentShadingRatePipelineFeatureUsage) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    fsr_ci.fragmentSize.width = 2;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 2;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04501");
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04502");
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
}

TEST_F(VkLayerTest, InvalidFragmentShadingRatePipelineCombinerOpsLimit) {
    TEST_DESCRIPTION("Specify invalid use of combiner ops when non trivial ops aren't supported");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    if (fsr_properties.fragmentShadingRateNonTrivialCombinerOps) {
        printf("%s requires fragmentShadingRateNonTrivialCombinerOps to be unsupported.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!fsr_features.primitiveFragmentShadingRate && !fsr_features.attachmentFragmentShadingRate) {
        printf("%s requires primitiveFragmentShadingRate or attachmentFragmentShadingRate to be supported.\n", kSkipPrefix);
        return;
    }

    fsr_features.pipelineFragmentShadingRate = VK_TRUE;
    fsr_features.primitiveFragmentShadingRate = VK_TRUE;
    fsr_features.attachmentFragmentShadingRate = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    if (fsr_features.primitiveFragmentShadingRate) {
        fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
        fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }

    if (fsr_features.attachmentFragmentShadingRate) {
        fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
        fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }
}

TEST_F(VkLayerTest, InvalidPrimitiveFragmentShadingRateWriteMultiViewportLimit) {
    TEST_DESCRIPTION("Test static validation of the primitiveFragmentShadingRateWithMultipleViewports limit");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    bool vil_extension = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    if (vil_extension) {
        m_device_extension_names.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    }

    bool va2_extension = DeviceExtensionSupported(gpu(), nullptr, VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);
    if (va2_extension) {
        m_device_extension_names.push_back(VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        printf("%s requires primitiveFragmentShadingRateWithMultipleViewports to be unsupported.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!fsr_features.primitiveFragmentShadingRate) {
        printf("%s requires primitiveFragmentShadingRate to be supported.\n", kSkipPrefix);
        return;
    }

    if (!features2.features.multiViewport) {
        printf("%s requires multiViewport to be supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test PrimitiveShadingRate writes with multiple viewports
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        VkViewport viewports[2] = {{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}};
        VkRect2D scissors[2] = {};

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo()};
            info.vp_state_ci_.viewportCount = 2;
            info.vp_state_ci_.pViewports = viewports;
            info.vp_state_ci_.scissorCount = 2;
            info.vp_state_ci_.pScissors = scissors;
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503");
    }

    // Test PrimitiveShadingRate writes with ViewportIndex writes in a geometry shader
    if (features2.features.geometryShader) {
        char const *vsSource = R"glsl(
            #version 450
            void main() {}
        )glsl";

        static char const *gsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            layout (points) in;
            layout (points) out;
            layout (max_vertices = 1) out;
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
                gl_ViewportIndex = 0;
                gl_PointSize = 1.0f;
                EmitVertex();
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504");
    }

    // Test PrimitiveShadingRate writes with ViewportIndex writes in a vertex shader
    if (vil_extension) {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            #extension GL_ARB_shader_viewport_layer_array : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                gl_ViewportIndex = 0;
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        auto info_override = [&](CreatePipelineHelper &info) { info.shader_stages_ = {vs.GetStageCreateInfo()}; };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504");
    }

    if (va2_extension) {
        // Test PrimitiveShadingRate writes with ViewportIndex writes in a geometry shader
        if (features2.features.geometryShader) {
            char const *vsSource = R"glsl(
                #version 450
                void main() {}
            )glsl";

            static char const *gsSource = R"glsl(
                #version 450
                #extension GL_EXT_fragment_shading_rate : enable
                #extension GL_NV_viewport_array2 : enable
                layout (points) in;
                layout (points) out;
                layout (max_vertices = 1) out;
                void main() {
                   gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                   gl_ViewportMask[0] = 0;
                   gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
                   gl_PointSize = 1.0f;
                   EmitVertex();
                }
            )glsl";

            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
            VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo()};
            };

            CreatePipelineHelper::OneshotTest(
                *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505");
        }

        // Test PrimitiveShadingRate writes with ViewportIndex writes in a vertex shader
        if (vil_extension) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_fragment_shading_rate : enable
                #extension GL_NV_viewport_array2 : enable
                void main() {
                    gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                    gl_ViewportMask[0] = 0;
                }
            )glsl";

            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

            auto info_override = [&](CreatePipelineHelper &info) { info.shader_stages_ = {vs.GetStageCreateInfo()}; };

            CreatePipelineHelper::OneshotTest(
                *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505");
        }
    }
}

TEST_F(VkLayerTest, SampledInvalidImageViews) {
    TEST_DESCRIPTION("Test if an VkImageView is sampled at draw/dispatch that the format has valid format features enabled");
    VkResult err;

    if (!EnableDeviceProfileLayer()) {
        printf("%s Couldn't enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Required extensions are not present.\n", kSkipPrefix);
        return;
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
    sampler_ci.compareEnable = VK_FALSE;
    VkSampler sampler;
    err = vk::CreateSampler(device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    VkDescriptorImageInfo combined_sampler_info = {sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo seperate_sampled_image_info = {VK_NULL_HANDLE, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo seperate_sampler_info = {sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

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
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyNotFound();

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
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, seperate_pipeline_layout.handle(), 0, 1,
                              &seperate_descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // cleanup
    vk::DestroySampler(device(), sampler, nullptr);
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
                                          vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147",    // Extension not enabled
                                                         "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // The capability not valid
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
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01091");
    }
}

TEST_F(VkLayerTest, ShaderFloatControl) {
    TEST_DESCRIPTION("Test VK_KHR_shader_float_controls");

    // Need 1.1 to get SPIR-V 1.3 since OpExecutionModeId was added in SPIR-V 1.2
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    // The issue with revision 4 of this extension should not be an issue with the tests
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto shader_float_control = LvlInitStruct<VkPhysicalDeviceFloatControlsProperties>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&shader_float_control);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

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
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
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
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
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
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
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
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
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
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306");
    }
}

TEST_F(VkLayerTest, Storage8and16bit) {
    TEST_DESCRIPTION("Test VK_KHR_8bit_storage and VK_KHR_16bit_storage");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool support_8_bit = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    bool support_16_bit = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_16BIT_STORAGE_EXTENSION_NAME);

    if ((support_8_bit == false) && (support_16_bit == false)) {
        printf("%s Extension %s and %s are not supported.\n", kSkipPrefix, VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
               VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
        return;
    } else if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) == false) {
        // need for all shaders, but not guaranteed from driver to have support
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
        return;
    } else {
        m_device_extension_names.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
        if (support_8_bit == true) {
            m_device_extension_names.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
        }
        if (support_16_bit == true) {
            m_device_extension_names.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
        }
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
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int8
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StorageBuffer8BitAccess
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

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int8
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // UniformAndStorageBuffer8BitAccess
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

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int8
                                                             "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StoragePushConstant8
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
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Float16
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StorageBuffer16BitAccess
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

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Float16
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // UniformAndStorageBuffer16BitAccess
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
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Float16
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StoragePushConstant16
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

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if ((VK_SUCCESS == vs.InitFromGLSLTry()) && (VK_SUCCESS == fs.InitFromGLSLTry())) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Float16 vert
                               "VUID-VkShaderModuleCreateInfo-pCode-01091",    // StorageInputOutput16 vert
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StorageInputOutput16 frag
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
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int16
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StorageBuffer16BitAccess
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

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int16
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // UniformAndStorageBuffer16BitAccess
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
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int16
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StoragePushConstant16
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

        m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
        if ((VK_SUCCESS == vs.InitFromGLSLTry()) && (VK_SUCCESS == fs.InitFromGLSLTry())) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            CreatePipelineHelper::OneshotTest(
                *this, set_info, kErrorBit,
                vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091",    // Int16 vert
                               "VUID-VkShaderModuleCreateInfo-pCode-01091",    // StorageInputOutput16 vert
                               "VUID-VkShaderModuleCreateInfo-pCode-01091"});  // StorageInputOutput16 frag
        }
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&float16int8_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
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
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147"});
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
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147"});
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
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147"});
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv");
        VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ReadShaderClock) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_CLOCK_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
        return;
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

TEST_F(VkLayerTest, UsingProvokingVertexModeLastVertexExtWithoutEnabled) {
    TEST_DESCRIPTION("Test using VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but it doesn't enable provokingVertexLast.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto provoking_vertex_state_ci = LvlInitStruct<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>();
    provoking_vertex_state_ci.provokingVertexMode = VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;
    pipe.rs_state_ci_.pNext = &provoking_vertex_state_ci;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkPipelineRasterizationProvokingVertexStateCreateInfoEXT-provokingVertexMode-04883");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NotSupportProvokingVertexModePerPipeline) {
    TEST_DESCRIPTION(
        "Test using different VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but it doesn't support provokingVertexModePerPipeline.");

    bool inst_ext = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (inst_ext) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping tests\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported, skipping tests\n", kSkipPrefix, VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
        return;
    }

    auto provoking_vertex_properties = LvlInitStruct<VkPhysicalDeviceProvokingVertexPropertiesEXT>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&provoking_vertex_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &properties2);
    if (provoking_vertex_properties.provokingVertexModePerPipeline == VK_TRUE) {
        printf("%s provokingVertexModePerPipeline is VK_TRUE, skipping tests\n", kSkipPrefix);
        return;
    }

    auto provoking_vertex_features = LvlInitStruct<VkPhysicalDeviceProvokingVertexFeaturesEXT>();
    provoking_vertex_features.provokingVertexLast = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&provoking_vertex_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    auto provoking_vertex_state_ci = LvlInitStruct<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>();
    provoking_vertex_state_ci.provokingVertexMode = VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT;
    pipe1.rs_state_ci_.pNext = &provoking_vertex_state_ci;
    pipe1.InitState();
    pipe1.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    provoking_vertex_state_ci.provokingVertexMode = VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;
    pipe2.rs_state_ci_.pNext = &provoking_vertex_state_ci;
    pipe2.InitState();
    pipe2.CreateGraphicsPipeline();

    CreatePipelineHelper pipe3(*this);
    pipe3.InitInfo();
    pipe3.InitState();
    pipe3.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
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
    pipe.cs_ = layer_data::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                                    &specialization_info);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    entry.size = sizeof(decltype(data));
    pipe.cs_ = layer_data::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                                    &specialization_info);
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
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

TEST_F(VkLayerTest, ValidateGeometryShaderEnabled) {
    TEST_DESCRIPTION("Validate geometry shader feature is enabled if geometry shader stage is used");

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.geometryShader = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(Init(&deviceFeatures));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, bindStateGeomShaderText, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkPipelineShaderStageCreateInfo-stage-00704", "VUID-VkShaderModuleCreateInfo-pCode-01091"});
}

TEST_F(VkLayerTest, ValidateTessellationShaderEnabled) {
    TEST_DESCRIPTION(
        "Validate tessellation shader feature is enabled if tessellation control or tessellation evaluation shader stage is used");

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.tessellationShader = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(Init(&deviceFeatures));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *tcsSource = R"glsl(
        #version 450
        layout(location=0) out int x[];
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
           x[gl_InvocationID] = gl_InvocationID;
        }
    )glsl";
    char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        layout(location=0) patch in int x;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = x;
        }
    )glsl";

    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        helper.gp_ci_.pTessellationState = &tsci;
        helper.gp_ci_.pInputAssemblyState = &iasci;
        helper.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
        helper.shader_stages_.emplace_back(tes.GetStageCreateInfo());
    };

    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkPipelineShaderStageCreateInfo-stage-00705", "VUID-VkShaderModuleCreateInfo-pCode-01091",
                            "VUID-VkShaderModuleCreateInfo-pCode-01091",
                            "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430"});
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

    VkPipeline test_pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &test_pipeline);

    {
        compute_create_info.basePipelineHandle = VK_NULL_HANDLE;
        compute_create_info.basePipelineIndex = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-00698");
        VkPipeline pipeline;
        vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    if (test_pipeline != VK_NULL_HANDLE) {
        compute_create_info.basePipelineHandle = test_pipeline;
        compute_create_info.basePipelineIndex = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-00699");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-flags-00700");
        VkPipeline pipeline;
        vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &pipeline);
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

TEST_F(VkLayerTest, PipelineSubgroupSizeControl) {
    TEST_DESCRIPTION("Test Subgroub Size Control");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT sscf = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>();
    sscf.subgroupSizeControl = VK_TRUE;
    sscf.computeFullSubgroups = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&sscf);
    vk::GetPhysicalDeviceFeatures2(gpu(), &pd_features2);
    if (sscf.subgroupSizeControl == VK_FALSE || sscf.computeFullSubgroups == VK_FALSE || sscf.subgroupSizeControl == VK_FALSE) {
        printf("%s Required features are not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto subgroup_properties = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT>();
    auto props = LvlInitStruct<VkPhysicalDeviceProperties2>(&subgroup_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &props);

    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        printf("%s Required shader stage not present in requiredSubgroupSizeStages, skipping test.\n", kSkipPrefix);
        return;
    }

    auto subgroup_size_control = LvlInitStruct<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    VkPhysicalDeviceVulkan11Properties props11 = LvlInitStruct<VkPhysicalDeviceVulkan11Properties>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&props11);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    {
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.pNext = &subgroup_size_control;
        cs_pipeline.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02754");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    if (subgroup_properties.maxSubgroupSize > 1) {
        std::stringstream csSource;
        csSource << R"glsl(
        #version 450
        layout(local_size_x = )glsl";
        csSource << subgroup_properties.maxSubgroupSize + 1;
        csSource << R"glsl() in;
        void main() {}
        )glsl";
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT |
                                         VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02758");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    if (props11.subgroupSize > 1) {
        std::stringstream csSource;
        csSource << R"glsl(
        #version 450
        layout(local_size_x = )glsl";
        csSource << props11.subgroupSize + 1;
        csSource << R"glsl() in;
        void main() {}
        )glsl";
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02759");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, SubgroupSizeControlFeaturesNotEnabled) {
    TEST_DESCRIPTION("Use subgroup size control features when they are not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT sscf = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>();
    sscf.subgroupSizeControl = VK_FALSE;
    sscf.computeFullSubgroups = VK_FALSE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&sscf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    VkPhysicalDeviceVulkan11Properties props11 = LvlInitStruct<VkPhysicalDeviceVulkan11Properties>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&props11);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    if (props11.subgroupSize == 0) {
        printf("%s subgroupSize is 0, skipping test.\n", kSkipPrefix);
        return;
    }

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"(
        #version 450
        layout(local_size_x = )";
    csSource << props11.subgroupSize;
    csSource << R"() in;
        void main(){
        }
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02784");
    pipe.CreateComputePipeline(true, false);
    m_errorMonitor->VerifyFound();

    pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02785");
    pipe.CreateComputePipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ShaderAtomicInt64) {
    TEST_DESCRIPTION("Test VK_KHR_shader_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }


    // Create device without VK_KHR_shader_atomic_int64 extension or features enabled
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    if (!available_features.shaderInt64) {
        printf("%s VkPhysicalDeviceFeatures::shaderInt64 is not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required for SPIR-V 1.3, skipping test.\n", kSkipPrefix);
        return;
    }

    // For sanity check without GL_EXT_shader_atomic_int64
    std::string cs_positive = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
           y = x + 1;
        }
    )glsl";

    std::string cs_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_atomic_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
    )glsl";

    // clang-format off
    // StorageBuffer storage class
    std::string cs_storage_buffer = cs_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    // StorageBuffer storage class using AtomicStore
    // atomicStore is slightly different than other atomics, so good edge case
    std::string cs_store = cs_base + R"glsl(
           atomicStore(y, 1ul, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    // Workgroup storage class
    std::string cs_workgroup = cs_base + R"glsl(
           atomicAdd(x, 1);
           barrier();
           y = x + 1;
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_.reset(new VkShaderObj(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    current_shader = cs_positive.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

    // shaderBufferInt64Atomics
    current_shader = cs_storage_buffer.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-RuntimeSpirv-None-06278"});
    current_shader = cs_store.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-RuntimeSpirv-None-06278"});

    // shaderSharedInt64Atomics
    current_shader = cs_workgroup.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-RuntimeSpirv-None-06279"});
}

TEST_F(VkLayerTest, PipelineInvalidAdvancedBlend) {
    TEST_DESCRIPTION("Create a graphics pipeline with advanced blend when its disabled");
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced =
        LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&blend_operation_advanced);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    if (blend_operation_advanced.advancedBlendAllOperations == VK_TRUE) {
        printf("%s blend_operation_advanced.advancedBlendAllOperations is VK_TRUE.\n", kSkipPrefix);
        return;
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();

    VkPipelineColorBlendAttachmentState attachment_state = {};
    attachment_state.blendEnable = VK_TRUE;
    attachment_state.colorBlendOp = VK_BLEND_OP_XOR_EXT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &attachment_state;
    pipe.gp_ci_.pColorBlendState = &color_blend_state;

    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendAllOperations-01409");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvlidPipelineDiscardRectangle) {
    TEST_DESCRIPTION("Create a graphics pipeline invalid VkPipelineDiscardRectangleStateCreateInfoEXT");

    bool inst_ext = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (inst_ext) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping tests\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties =
        LvlInitStruct<VkPhysicalDeviceDiscardRectanglePropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();
    phys_dev_props_2.pNext = &discard_rectangle_properties;
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

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

TEST_F(VkLayerTest, ShaderImageAtomicInt64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    // Create device without VK_EXT_shader_image_atomic_int64 extension or features enabled
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    if (!available_features.shaderInt64) {
        printf("%s VkPhysicalDeviceFeatures::shaderInt64 is not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required for SPIR-V 1.3, skipping test.\n", kSkipPrefix);
        return;
    }

    // clang-format off
    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        layout(set = 0, binding = 1, r64ui) uniform u64image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    std::unique_ptr<VkShaderObj> current_shader;
    const auto set_info = [&current_shader](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::move(current_shader);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    // shaderImageInt64Atomics
    // Need 01091 VUID check for both Int64ImageEXT and Int64Atomics.. test could be rewritten to be more complex in order to set
    // capability requirements with other features, but this is simpler
    current_shader = layer_data::make_unique<VkShaderObj>(this, cs_image_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT,
                                                          SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-01091",
                                "VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06288"});
    }

    // glslang doesn't omit Int64Atomics for store currently
    current_shader = layer_data::make_unique<VkShaderObj>(this, cs_image_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT,
                                                          SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                                "VUID-RuntimeSpirv-None-06288"});
    }

    current_shader = layer_data::make_unique<VkShaderObj>(this, cs_image_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT,
                                                          SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-01091",
                                "VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06288"});
    }

    current_shader = layer_data::make_unique<VkShaderObj>(this, cs_image_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT,
                                                          SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-01091",
                                "VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06288"});
    }
}

TEST_F(VkLayerTest, ShaderAtomicFloat) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    // Create device without VK_EXT_shader_atomic_float extension or features enabled
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required for SPIR-V 1.3, skipping test.\n", kSkipPrefix);
        return;
    }

    // clang-format off
    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_add = cs_32_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_store = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_32_add = cs_32_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_store = cs_32_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_add = cs_64_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_store = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_64_add = cs_64_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_store = cs_64_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = layer_data::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                          SPV_SOURCE_GLSL_TRY);
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        if (VK_SUCCESS != helper.cs_.get()->InitFromGLSLTry()) {
            helper.override_skip_ = true;
        }
        helper.dsl_bindings_ = current_bindings;
    };

    // shaderBufferFloat32Atomics
    current_shader = cs_buffer_float_32_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

    current_shader = cs_buffer_float_32_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

    current_shader = cs_buffer_float_32_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

    // shaderBufferFloat32AtomicAdd
    current_shader = cs_buffer_float_32_add.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                            "VUID-RuntimeSpirv-None-06280"});

    // shaderSharedFloat32Atomics
    current_shader = cs_shared_float_32_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

    current_shader = cs_shared_float_32_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

    current_shader = cs_shared_float_32_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

    // shaderSharedFloat32AtomicAdd
    current_shader = cs_shared_float_32_add.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                            "VUID-RuntimeSpirv-None-06281"});

    // shaderBufferFloat64Atomics
    if (available_features.shaderFloat64) {
        current_shader = cs_buffer_float_64_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

        current_shader = cs_buffer_float_64_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

        current_shader = cs_buffer_float_64_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

        // shaderBufferFloat64AtomicAdd
        current_shader = cs_buffer_float_64_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                                "VUID-RuntimeSpirv-None-06280"});

        // shaderSharedFloat64Atomics
        current_shader = cs_shared_float_64_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

        current_shader = cs_shared_float_64_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

        current_shader = cs_shared_float_64_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

        // shaderSharedFloat64AtomicAdd
        current_shader = cs_shared_float_64_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                                "VUID-RuntimeSpirv-None-06281"});
    } else {
        printf("Skipping 64-bit float tests\n");
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    // shaderImageFloat32Atomics
    current_shader = cs_image_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06282");

    current_shader = cs_image_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06282");

    current_shader = cs_image_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06282");

    // shaderImageFloat32AtomicAdd
    current_shader = cs_image_add.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-01091", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                            "VUID-RuntimeSpirv-None-06282"});
}

TEST_F(VkLayerTest, ShaderAtomicFloat2) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    // Create device without VK_EXT_shader_atomic_float2 extension or features enabled
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    // Still check for proper 16-bit storage/float support for most tests
    auto float16int8_features = LvlInitStruct<VkPhysicalDeviceShaderFloat16Int8Features>();
    auto storage_16_bit_features = LvlInitStruct<VkPhysicalDevice16BitStorageFeatures>(&float16int8_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&storage_16_bit_features);

    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    const bool support_16_bit =
        (float16int8_features.shaderFloat16 == VK_TRUE) && (storage_16_bit_features.storageBuffer16BitAccess == VK_TRUE);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // clang-format off
    std::string cs_16_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_16bit_storage: enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared float16_t x;
        layout(set = 0, binding = 0) buffer ssbo { float16_t y; };
        void main() {
    )glsl";

     std::string cs_buffer_float_16_add = cs_16_base + R"glsl(
           atomicAdd(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_store = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_buffer_float_16_min = cs_16_base + R"glsl(
           atomicMin(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_max = cs_16_base + R"glsl(
           atomicMax(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_add = cs_16_base + R"glsl(
           y = atomicAdd(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_store = cs_16_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_shared_float_16_min = cs_16_base + R"glsl(
           y = atomicMin(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_max = cs_16_base + R"glsl(
           y = atomicMax(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_min = cs_32_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_max = cs_32_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_32_min = cs_32_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_max = cs_32_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_min = cs_64_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_max = cs_64_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_64_min = cs_64_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_max = cs_64_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_image_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_32_min = cs_image_32_base + R"glsl(
           y = imageAtomicMin(z, ivec2(1, 1), y);
        }
    )glsl";

    std::string cs_image_32_max = cs_image_32_base + R"glsl(
           y = imageAtomicMax(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [this, &current_shader, &current_bindings](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
        helper.cs_ =
            VkShaderObj::CreateFromGLSL(*this, VK_SHADER_STAGE_COMPUTE_BIT, current_shader, "main", nullptr, SPV_ENV_VULKAN_1_1);
        // Skip the test if shader failed to compile
        helper.override_skip_ = !static_cast<bool>(helper.cs_);
        helper.dsl_bindings_ = current_bindings;
    };

    if (support_16_bit) {
        // shaderBufferFloat16Atomics
        current_shader = cs_buffer_float_16_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

        current_shader = cs_buffer_float_16_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

        current_shader = cs_buffer_float_16_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06280");

        // shaderBufferFloat16AtomicAdd
        current_shader = cs_buffer_float_16_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                                "VUID-RuntimeSpirv-None-06280"});

        // shaderBufferFloat16AtomicMinMax
        current_shader = cs_buffer_float_16_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06280"});

        current_shader = cs_buffer_float_16_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06280"});

        // shaderSharedFloat16Atomics
        current_shader = cs_shared_float_16_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

        current_shader = cs_shared_float_16_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

        current_shader = cs_shared_float_16_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06281");

        // shaderSharedFloat16AtomicAdd
        current_shader = cs_shared_float_16_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-VkShaderModuleCreateInfo-pCode-04147",
                                "VUID-RuntimeSpirv-None-06281"});

        // shaderSharedFloat16AtomicMinMax
        current_shader = cs_shared_float_16_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06281"});

        current_shader = cs_shared_float_16_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06281"});
    } else {
        printf("Skipping 16-bit tests\n");
    }

    // shaderBufferFloat32AtomicMinMax
    current_shader = cs_buffer_float_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06280"});

    current_shader = cs_buffer_float_32_max.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06280"});

    // shaderSharedFloat32AtomicMinMax
    current_shader = cs_shared_float_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06281"});

    current_shader = cs_shared_float_32_max.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06281"});

    if (features2.features.shaderFloat64 == VK_TRUE) {
        // shaderBufferFloat64AtomicMinMax
        current_shader = cs_buffer_float_64_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06280"});

        current_shader = cs_buffer_float_64_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06280"});

        // shaderSharedFloat64AtomicMinMax
        current_shader = cs_shared_float_64_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06281"});

        current_shader = cs_shared_float_64_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06281"});
    } else {
        printf("Skipping 64-bit float tests\n");
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    // shaderSharedFloat32AtomicMinMax
    current_shader = cs_image_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06282"});

    current_shader = cs_image_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-04147", "VUID-RuntimeSpirv-None-06282"});
}

TEST_F(VkLayerTest, BindLibraryPipeline) {
    TEST_DESCRIPTION("Test binding a pipeline that was created with library flag");

    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&gpl_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Required for graphics pipeline libraries
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipeline(*this);
    pipeline.InitVertexInputLibInfo();
    pipeline.InitState();
    ASSERT_VK_SUCCESS(pipeline.CreateGraphicsPipeline(true, false));
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-03382");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestPipelineColorWriteCreateInfoEXT) {
    TEST_DESCRIPTION("Test VkPipelineColorWriteCreateInfoEXT in color blend state pNext");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s test requires %s extension. Skipping.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME)) {
        printf("%s test requires %s extension. Skipping.\n", kSkipPrefix, VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
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

    std::vector<VkBool32> max_enabled(m_device->props.limits.maxColorAttachments + 1, VK_TRUE);
    color_write.attachmentCount = m_device->props.limits.maxColorAttachments + 1;
    color_write.pColorWriteEnables = max_enabled.data();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-06655");
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
    auto pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&blend_operation_advanced_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &pd_props2);

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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations = LvlInitStruct<VkPhysicalDeviceSampleLocationsPropertiesEXT>();
    VkPhysicalDeviceProperties2 phys_props = LvlInitStruct<VkPhysicalDeviceProperties2>(&sample_locations);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_props);

    if (sample_locations.variableSampleLocations) {
        printf("%s VkPhysicalDeviceSampleLocationsPropertiesEXT::variableSampleLocations is supported, skipping.\n", kSkipPrefix);
        return;
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
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpasses[2] = {subpass, subpass};

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    vk::CreateRenderPass(device(), &rpci, NULL, &render_pass);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo framebuffer_info = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &image_view;
    framebuffer_info.width = 32;
    framebuffer_info.height = 32;
    framebuffer_info.layers = 1;

    VkFramebuffer framebuffer;
    vk::CreateFramebuffer(m_device->handle(), &framebuffer_info, nullptr, &framebuffer);

    VkMultisamplePropertiesEXT multisample_prop = {};
    vkGetPhysicalDeviceMultisamplePropertiesEXT(gpu(), VK_SAMPLE_COUNT_1_BIT, &multisample_prop);
    const uint32_t valid_count =
        multisample_prop.maxSampleLocationGridSize.width * multisample_prop.maxSampleLocationGridSize.height;

    if (valid_count == 0) {
        printf("%s multisample properties are not supported, skipping.\n", kSkipPrefix);
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

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.gp_ci_.pNext = &sample_locations_state;
    pipe.gp_ci_.renderPass = render_pass;
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
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = framebuffer;
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&explicit_layout_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &explicit_layout_features));

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        printf("%s workgroupMemoryExplicitLayout feature not supported.\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, TestInvalidShaderInputAndOutputComponents) {
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

                layout(location = 0, component = 0) in vec3 rgb;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(rgb, 1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit,
                                          "UNASSIGNED-CoreValidation-Shader-InputNotProduced");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out vec3 v;

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
        CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit,
                                          "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
    }
}

TEST_F(VkLayerTest, SpecializationInvalidSizeMismatch) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is not correct with type");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    bool int8_support = false;
    bool float64_support = false;

    // require to make enable logic simpler
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    features12.shaderInt8 = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features12);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
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
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);

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
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);

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
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);

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

TEST_F(VkLayerTest, UsingRasterizationStateStreamExtWithoutEnabled) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT but it doesn't enable geometryStreams.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    transform_feedback_features.geometryStreams = VK_FALSE; // Invalid

    // Extension enabled via VK_EXT_transform_feedback dependency
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&transform_feedback_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto rasterization_state_stream_ci = LvlInitStruct<VkPipelineRasterizationStateStreamCreateInfoEXT>();
    pipe.rs_state_ci_.pNext = &rasterization_state_stream_ci;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-geometryStreams-02324");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestPipelineRasterizationStateStreamCreateInfoEXT) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT with invalid rasterizationStream.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    transform_feedback_features.geometryStreams = VK_TRUE;

    // Extension enabled via dependencies
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&transform_feedback_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transfer_feedback_props =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transfer_feedback_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    if (!transfer_feedback_props.transformFeedbackRasterizationStreamSelect &&
        transfer_feedback_props.maxTransformFeedbackStreams == 0) {
        printf("%s VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackRasterizationStreamSelect is 0; skipped.\n",
               kSkipPrefix);
        return;
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto rasterization_state_stream_ci = LvlInitStruct<VkPipelineRasterizationStateStreamCreateInfoEXT>();
    rasterization_state_stream_ci.rasterizationStream = transfer_feedback_props.maxTransformFeedbackStreams;
    pipe.rs_state_ci_.pNext = &rasterization_state_stream_ci;
    pipe.InitState();

    if (transfer_feedback_props.transformFeedbackRasterizationStreamSelect) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02325");
    } else {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02326");
    }
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv");
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv");
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_COMPUTE_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestWrongPipelineType) {
    TEST_DESCRIPTION("Use a compute pipeline in GetRayTracingShaderGroupStackSizeKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateComputePipeline();

    PFN_vkGetRayTracingShaderGroupStackSizeKHR vkGetRayTracingShaderGroupStackSizeKHR =
        (PFN_vkGetRayTracingShaderGroupStackSizeKHR)vk::GetInstanceProcAddr(instance(), "vkGetRayTracingShaderGroupStackSizeKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-04622");
    vkGetRayTracingShaderGroupStackSizeKHR(device(), pipe.pipeline_, 0, VK_SHADER_GROUP_SHADER_GENERAL_KHR);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestPipelineRasterizationConservativeStateCreateInfo) {
    TEST_DESCRIPTION("Test PipelineRasterizationConservativeStateCreateInfo.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    m_device_extension_names.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
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

TEST_F(VkLayerTest, TestRuntimeSpirvTransformFeedback) {
    TEST_DESCRIPTION("Test runtime spirv transform feedback.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Test currently crashes with valid SPIR-V
    // Using EmitStreamVertex() with transfer_feedback_props.maxTransformFeedbackStreams
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        printf("%s Test does not run on AMD proprietary driver, skipping tests\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    transform_feedback_features.transformFeedback = VK_TRUE;
    transform_feedback_features.geometryStreams = VK_TRUE;
    VkPhysicalDeviceVulkan12Features features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>(&transform_feedback_features);

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features12);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (features2.features.geometryShader == VK_FALSE) {
        printf("%s Device does not support the required geometry shader features; skipped.\n", kSkipPrefix);
        return;
    }
    if (!transform_feedback_features.transformFeedback || !transform_feedback_features.geometryStreams) {
        printf("%s transformFeedback or geometryStreams feature is not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &pd_props2);

    // seen sometimes when using devsim and will crash
    if (transform_feedback_props.maxTransformFeedbackStreams == 0) {
        printf("%s maxTransformFeedbackStreams is zero, skipping test.\n", kSkipPrefix);
        return;
    }

    {
        std::stringstream vsSource;
        vsSource << R"asm(
               OpCapability Shader
               OpCapability TransformFeedback
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %tf
               OpExecutionMode %main Xfb

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %8

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride )asm";
        vsSource << transform_feedback_props.maxTransformFeedbackBufferDataStride + 4;
        vsSource << R"asm(
               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )asm";

        auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, vsSource.str().c_str(), "main", nullptr);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-XfbStride-06313");
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %tf
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 1

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %10

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf Stream 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
     %int_17 = OpConstant %int )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackStreams;
        gsSource << R"asm(
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_17
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_GEOMETRY_BIT, gsSource.str().c_str(), "main", nullptr);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEmitStreamVertex-06310");
    }

    if (transform_feedback_props.transformFeedbackStreamsLinesTriangles == VK_FALSE) {
        const char *gsSource = R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a %b
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %11
               OpName %b "b"  ; id %12

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream 0
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 4
               OpDecorate %a Offset 0
               OpDecorate %b Location 1
               OpDecorate %b Stream 0
               OpDecorate %b XfbBuffer 1
               OpDecorate %b XfbStride 4
               OpDecorate %b Offset 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output
          %b = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpEmitStreamVertex %int_1
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_GEOMETRY_BIT, gsSource, "main", nullptr);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                          "VUID-RuntimeSpirv-transformFeedbackStreamsLinesTriangles-06311");
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %10

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream 0
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 20
               OpDecorate %a Offset  )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackBufferDataSize;
        gsSource << R"asm(

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_GEOMETRY_BIT, gsSource.str().c_str(), "main", nullptr);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        std::vector<std::string> vuids = {"VUID-RuntimeSpirv-Offset-06308"};
        if (transform_feedback_props.maxTransformFeedbackBufferDataSize + 4 >=
            transform_feedback_props.maxTransformFeedbackStreamDataSize) {
            vuids.push_back("VUID-RuntimeSpirv-XfbBuffer-06309");
        }
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %10

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream  )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackStreams;
        gsSource << R"asm(
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 4
               OpDecorate %a Offset 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_GEOMETRY_BIT, gsSource.str().c_str(), "main", nullptr);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Stream-06312");
    }

    {
        uint32_t offset = transform_feedback_props.maxTransformFeedbackBufferDataSize / 2;
        uint32_t count = transform_feedback_props.maxTransformFeedbackStreamDataSize / offset + 1;
        // Limit to 25, because we are dynamically adding variables using letters as names
        if (count < 25) {
            std::stringstream gsSource;
            gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main"
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4)asm";

            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + i;
                gsSource << "\nOpName %var" << v << " \"" << v << "\"";
            }
            gsSource << "\n; Annotations\n";

            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + i;
                gsSource << "OpDecorate %var" << v << " Location " << i << "\n";
                gsSource << "OpDecorate %var" << v << " Stream 0\n";
                gsSource << "OpDecorate %var" << v << " XfbBuffer " << i << "\n";
                gsSource << "OpDecorate %var" << v << " XfbStride 20\n";
                gsSource << "OpDecorate %var" << v << " Offset " << offset << "\n";
            }
            gsSource << R"asm(

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float)asm";

            gsSource << "\n";
            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + i;
                gsSource << "%var" << v << " = OpVariable %_ptr_Output_float Output\n";
            }

            gsSource << R"asm(

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

            auto gs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_GEOMETRY_BIT, gsSource.str().c_str(), "main", nullptr);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(),
                                         helper.fs_->GetStageCreateInfo()};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-XfbBuffer-06309");
        }
    }
}

TEST_F(VkLayerTest, TestMinAndMaxTexelGatherOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelGatherOffset and greather than maxTexelGatherOffset");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (m_device->phy().properties().limits.minTexelGatherOffset <= -100 ||
        m_device->phy().properties().limits.maxTexelGatherOffset >= 100) {
        printf("%s test needs minTexelGatherOffset greater than -100 and maxTexelGatherOffset less than 100. Skipping.\n",
               kSkipPrefix);
        return;
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
        printf("%s test needs minTexelOffset greater than -100 and maxTexelOffset less than 100. Skipping.\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, RayTracingLibraryFlags) {
    TEST_DESCRIPTION("Validate ray tracing pipeline flags match library flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!ray_tracing_features.rayTracingPipeline) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj pipeline_layout(m_device, {});

    const char *ray_generation_shader = R"glsl(
        #version 460 core
        #extension GL_KHR_ray_tracing : enable
        void main() {
        }
    )glsl";

    VkShaderObj rgen_shader(this, ray_generation_shader, VK_SHADER_STAGE_RAYGEN_BIT_NV);

    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = LvlInitStruct<VkRayTracingPipelineInterfaceCreateInfoKHR>();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    VkPipeline invalid_library = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &library);

    pipeline_ci.flags = 0;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &invalid_library);

    VkPipelineLibraryCreateInfoKHR library_ci = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    library_ci.libraryCount = 1;
    library_ci.pLibraries = &library;

    pipeline_ci.pLibraryInfo = &library_ci;
    VkPipeline pipeline = VK_NULL_HANDLE;

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04718");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04719");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04720");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04721");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04722");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04723");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    {
        pipeline_ci.flags = 0;
        library_ci.pLibraries = &invalid_library;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyPipeline(m_device->handle(), library, nullptr);
    vk::DestroyPipeline(m_device->handle(), invalid_library, nullptr);
}

TEST_F(VkLayerTest, DeviceMemoryScope) {
    TEST_DESCRIPTION("Validate using Device memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features12);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    features12.vulkanMemoryModelDeviceScope = VK_FALSE;
    if (features12.vulkanMemoryModel == VK_FALSE) {
        printf("%s vulkanMemoryModel feature is not supported, skipping test.\n", kSkipPrefix);
        return;
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
        helper.cs_ = layer_data::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features12);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    features12.vulkanMemoryModel = VK_FALSE;
    if (features12.vulkanMemoryModelDeviceScope == VK_FALSE) {
        printf("%s vulkanMemoryModelDeviceScope feature is not supported, skipping test.\n", kSkipPrefix);
        return;
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
        helper.cs_ = layer_data::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-RuntimeSpirv-vulkanMemoryModel-06266", "VUID-VkShaderModuleCreateInfo-pCode-01091"});
}

TEST_F(VkLayerTest, CreatePipelineLayoutWithInvalidSetLayoutFlags) {
    TEST_DESCRIPTION("Validate setLayout flags in create pipeline layout.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mut_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
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
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE;
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

TEST_F(VkLayerTest, TestUsingDisabledMultiviewFeatures) {
    TEST_DESCRIPTION("Create graphics pipeline using multiview features which are not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    VkPhysicalDeviceMultiviewFeatures multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&multiview_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    multiview_features.multiviewTessellationShader = VK_FALSE;
    multiview_features.multiviewGeometryShader = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2 color_attachment = LvlInitStruct<VkAttachmentReference2>();
    color_attachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription2 description = LvlInitStruct<VkAttachmentDescription2>();
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.format = VK_FORMAT_B8G8R8A8_UNORM;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 0x3u;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &description;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass(*m_device, rpci);
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
        pipe.InitInfo();
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

        VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.gp_ci_.renderPass = render_pass.handle();
        pipe.gp_ci_.subpass = 0;
        pipe.cb_ci_.attachmentCount = 1;
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.InitState();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06048");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
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
    pipe.cs_ = layer_data::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
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
    pipe.cs_ = layer_data::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
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
    m_errorMonitor->ExpectSuccess();

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

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06574");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06603");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineRasterizationOrderAttachmentAccessWithoutFeature) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_ARM_rasterization_order_attachment_access enabled");
    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);

    auto rasterization_order_features = LvlInitStruct<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&rasterization_order_features);

    ASSERT_NO_FATAL_FAILURE(InitFrameworkAndRetrieveFeatures(features2));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    rasterization_order_features.rasterizationOrderColorAttachmentAccess = 0;
    rasterization_order_features.rasterizationOrderDepthAttachmentAccess = 0;
    rasterization_order_features.rasterizationOrderStencilAttachmentAccess = 0;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);

    auto rasterization_order_features = LvlInitStruct<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&rasterization_order_features);

    ASSERT_NO_FATAL_FAILURE(InitFrameworkAndRetrieveFeatures(features2));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!rasterization_order_features.rasterizationOrderColorAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderDepthAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderStencilAttachmentAccess) {
        printf("%s Test requires (unsupported) rasterizationOrderAttachmentAccess , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->VerifyNotFound();

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

TEST_F(VkLayerTest, InvalidPipelineRenderingParameters) {
    TEST_DESCRIPTION("Test pipeline rendering formats and viewmask");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
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

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    auto depth_stencil_state = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    create_info.pDepthStencilState = &depth_stencil_state;

    VkFormat depth_format = VK_FORMAT_X8_D24_UNORM_PACK32;

    if (ImageFormatAndFeaturesSupported(m_gpu, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        depth_format = VK_FORMAT_D32_SFLOAT;
    }

    VkFormat stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;

    if (ImageFormatAndFeaturesSupported(m_gpu, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    VkFormat color_formats = {depth_format};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    // Invalid color format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06581");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Invalid color format array
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Invalid depth format
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06587");
    // TODO (ncesario) Seems impossible hit 06585 without also hitting 06587. Since 06587 happens in stateless validation, 06585
    // never gets triggered, though has been manually tested separately by removing 06587.
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06585");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Invalid stecil format
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06588");
    // TODO (ncesario) Same scenario as with 06585 and 06587
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06586");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // mismatching depth/stencil formats
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = stencil_format;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06589");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    // Non-zero viewMask
    color_formats = VK_FORMAT_R8G8B8A8_UNORM;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577");
    pipeline_rendering_info.viewMask = 1;
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidPipelineRenderingViewMaskParameter) {
    TEST_DESCRIPTION("Test pipeline rendering viewmask maximum index");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    multiview_features.multiview = VK_TRUE;
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>(&multiview_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (!multiview_features.multiview) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
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

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    VkPhysicalDeviceMultiviewProperties multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&multiview_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    if (multiview_props.maxMultiviewViewCount == 32) {
        printf("%s VUID is not testable as maxMultiviewViewCount is 32, skipping test\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06578");
    pipeline_rendering_info.viewMask = 1 << multiview_props.maxMultiviewViewCount;
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
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
                                          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136");
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
    vk::GetPhysicalDeviceProperties2(gpu(), &properties2);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        printf("%s Required sample location sample count VK_SAMPLE_COUNT_1_BIT not supported, skipping test.\n", kSkipPrefix);
        return;
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

    m_errorMonitor->ExpectSuccess();
    vkCmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyNotFound();

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-InterfaceTypeMismatch");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PrimitivesGeneratedQueryAndDiscardEnabled) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithRasterizerDiscard feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&primitives_generated_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        printf("%s primitivesGeneratedQuery feature is not supported.\n", kSkipPrefix);
        return;
    }
    primitives_generated_features.primitivesGeneratedQueryWithRasterizerDiscard = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto rs_ci = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_ = rs_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    auto query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PrimitivesGeneratedQueryStreams) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithNonZeroStreams feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto xfb_props = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(xfb_props);
    if (!xfb_props.transformFeedbackRasterizationStreamSelect) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedbackRasterizationStreamSelect is VK_FALSE";
    }

    auto transform_feedback_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    auto primitives_generated_features =
        LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>(&transform_feedback_features);
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported.";
    }
    if (transform_feedback_features.geometryStreams == VK_FALSE) {
        GTEST_SKIP() << "geometryStreams feature not supported, skipping tests.";
    }
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "geometryStreams feature not supported, skipping tests.";
    }
    primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto rasterization_streams = LvlInitStruct<VkPipelineRasterizationStateStreamCreateInfoEXT>();
    rasterization_streams.rasterizationStream = 1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_.pNext = &rasterization_streams;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    auto query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateOps) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");
    m_errorMonitor->ExpectSuccess();

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!fsr_features.primitiveFragmentShadingRate) {
        printf("%s primitiveFragmentShadingRate not available.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->VerifyNotFound();

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = LvlInitStruct<VkPipelineFragmentShadingRateStateCreateInfoKHR>();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    // Pass an invalid value for op 0
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_ENUM_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06567");
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    // Pass an invalid value for op 1
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_ENUM_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06568");
}

TEST_F(VkLayerTest, TestMaxFragmentDualSrcAttachments) {
    TEST_DESCRIPTION("Test drawing with dual source blending with too many fragment output attachments.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>();
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        printf("%s dualSrcBlend feature is not available, skipping test.\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, TestSubgroupUniformControlFlow) {
    TEST_DESCRIPTION("Test SubgroupUniformControlFlow spirv execution mode");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    auto ssucff = LvlInitStruct<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR>();
    ssucff.shaderSubgroupUniformControlFlow = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ssucff));

    const char *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_subgroup_uniform_control_flow"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main SubgroupUniformControlFlowKHR

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4

               ; Annotations
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
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

TEST_F(VkLayerTest, TestComputeLocalWorkgroupSize) {
    TEST_DESCRIPTION("Test size of local workgroud with requiredSubgroupSize.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto sscf = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>();
    auto features2  = LvlInitStruct<VkPhysicalDeviceFeatures2>(&sscf);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (sscf.subgroupSizeControl == VK_FALSE || sscf.computeFullSubgroups == VK_FALSE || sscf.subgroupSizeControl == VK_FALSE) {
        printf("%s Required features are not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto subgroup_properties = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT>();
    auto props = LvlInitStruct<VkPhysicalDeviceProperties2>(&subgroup_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &props);

    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        printf("%s Required shader stage not present in requiredSubgroupSizeStages, skipping test.\n", kSkipPrefix);
        return;
    }

    auto subgroup_size_control = LvlInitStruct<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    uint32_t size = static_cast<uint32_t>(
        std::ceil(std::sqrt(subgroup_size_control.requiredSubgroupSize * subgroup_properties.maxComputeWorkgroupSubgroups)));


    if (size <= 1024) {
        std::stringstream csSource;
        csSource << R"glsl(
        #version 450
        layout(local_size_x=
    )glsl";
        csSource << size;
        csSource << R"glsl(, local_size_y=
    )glsl";
        csSource << size;
        csSource << R"glsl(, local_size_z=2) in;
        void main(){
           if (gl_GlobalInvocationID.x >= 0) { return; }
        }
    )glsl";

        CreateComputePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        pipe.InitState();
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &subgroup_size_control;
        if (size * size * 2 > m_device->props.limits.maxComputeWorkGroupInvocations) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-x-06432");
        }
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02756");
        pipe.CreateComputePipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    if (subgroup_properties.maxSubgroupSize > 1) {
        std::stringstream csSource;
        csSource << R"glsl(
            #version 450
            layout(local_size_x=
        )glsl";
        csSource << subgroup_properties.maxSubgroupSize + 1;
        csSource << R"glsl(, local_size_y=1, local_size_z=1) in;
            void main(){
            if (gl_GlobalInvocationID.x >= 0) { return; }
            }
        )glsl";

        CreateComputePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        pipe.InitState();
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &subgroup_size_control;
        pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02757");
        pipe.CreateComputePipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MissingSubgroupSizeControlFeature) {
    TEST_DESCRIPTION("Test using subgroupSizeControl feature when it's not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto subgroup_properties = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT>();
    auto props = LvlInitStruct<VkPhysicalDeviceProperties2>(&subgroup_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &props);
    auto subgroup_size_control = LvlInitStruct<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.pNext = &subgroup_size_control;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02755");
    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02755");
    }
    pipe.CreateComputePipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingPipelineMaxResources) {
    TEST_DESCRIPTION("Create ray tracing pipeline with too many resources.");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!ray_tracing_features.rayTracingPipeline) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }
    if (!ray_tracing_features.rayTraversalPrimitiveCulling) {
        printf("%s Feature rayTraversalPrimitiveCulling is not supported.\n", kSkipPrefix);
        return;
    }

    const uint32_t maxPerStageResources = 4;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxPerStageResources = maxPerStageResources;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_device->phy().properties().limits.maxPerStageResources,
         VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr}};

    const VkDescriptorSetLayoutObj ds_layout(m_device, layout_bindings);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout});
    const char *empty_shader = R"glsl(
        #version 460
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingPipelineCreateInfoKHR create_info = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    create_info.layout = pipeline_layout.handle();
    create_info.stageCount = 1;
    create_info.pStages = &stage_create_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03428");
    VkPipeline pipeline;
    vkCreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidRayTracingPipelineFlags) {
    TEST_DESCRIPTION("Validate ray tracing pipeline flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!ray_tracing_features.rayTracingPipeline) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }
    if (!ray_tracing_features.rayTraversalPrimitiveCulling) {
        printf("%s Feature rayTraversalPrimitiveCulling is not supported.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const char *empty_shader = R"glsl(
        #version 460
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingPipelineCreateInfoKHR create_info = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    create_info.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
    create_info.layout = empty_pipeline_layout.handle();
    create_info.stageCount = 1;
    create_info.pStages = &stage_create_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546");
    VkPipeline pipeline;
    vkCreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDepthStencilStateForReadOnlyLayout) {
    TEST_DESCRIPTION("Create graphics pipeline with invalid depth stencil state for subpass that uses read only image layout.");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    VkAttachmentDescription attachment = {};
    attachment.flags = 0;
    attachment.format = ds_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference dsAttachRef = {};
    dsAttachRef.attachment = 0;
    dsAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &dsAttachRef;

    VkRenderPassCreateInfo rp_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, rp_ci);

    auto ds = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds.depthWriteEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.LateBindPipelineInfo();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.gp_ci_.pDepthStencilState = &ds;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06039");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();

    ds.depthWriteEnable = VK_FALSE;
    ds.front.failOp = VK_STENCIL_OP_ZERO;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06040");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineDynamicRendering) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    m_errorMonitor->ExpectSuccess();

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    auto rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &color_format;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &rendering_info;

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineDynamicRenderingNoInfo) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled but no rendering info struct.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    m_errorMonitor->ExpectSuccess();

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    // if there isn't a VkPipelineRenderingCreateInfoKHR, the driver is supposed to use safe default values
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}
