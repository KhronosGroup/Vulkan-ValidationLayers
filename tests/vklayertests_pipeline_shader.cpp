/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google, Inc.
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
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, PSOPolygonModeInvalid) {
    TEST_DESCRIPTION("Attempt to use invalid polygon fill modes.");
    VkPhysicalDeviceFeatures device_features = {};
    device_features.fillModeNonSolid = VK_FALSE;
    // The sacrificial device object
    ASSERT_NO_FATAL_FAILURE(Init(&device_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineRasterizationStateCreateInfo rs_ci = {};
    rs_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_ci.pNext = nullptr;
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    auto set_polygonMode = [&](CreatePipelineHelper &helper) { helper.rs_state_ci_ = rs_ci; };

    // Set polygonMode to POINT while the non-solid fill mode feature is disabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_POINT;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit,
                                      "polygonMode cannot be VK_POLYGON_MODE_POINT or VK_POLYGON_MODE_LINE");

    // Set polygonMode to LINE while the non-solid fill mode feature is disabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_LINE;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit,
                                      "polygonMode cannot be VK_POLYGON_MODE_POINT or VK_POLYGON_MODE_LINE");

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-00605");

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

    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;

    VkAttachmentDescription attach_desc[2] = {};
    attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    VkRenderPass renderpass;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &renderpass);
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    pipeline.AddShader(&vs);

    VkPipelineColorBlendAttachmentState att_state1 = {}, att_state2 = {};
    att_state1.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state1.blendEnable = VK_TRUE;
    att_state2.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state2.blendEnable = VK_FALSE;
    pipeline.AddColorAttachment(0, att_state1);
    pipeline.AddColorAttachment(1, att_state2);
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderpass);
    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(m_device->device(), renderpass, NULL);
}

TEST_F(VkLayerTest, BlendingOnFormatWithoutBlendingSupport) {
    TEST_DESCRIPTION("Test that blending is not enabled with a format not support blending");
    VkPhysicalDeviceFeatures features = {};
    features.independentBlend = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-blendEnable-02023");

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

    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = non_blending_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    VkRenderPass rp;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    pipeline.AddShader(&vs);

    VkPipelineColorBlendAttachmentState att_state = {};
    att_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state.blendEnable = VK_TRUE;
    pipeline.AddColorAttachment(0, att_state);
    pipeline.CreateVKPipeline(descriptorSet.GetPipelineLayout(), rp);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, non_blending_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView imageView = image.targetView(non_blending_format);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);

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

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rpbi);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(m_device->device(), rp, NULL);
    vk::DestroyFramebuffer(m_device->device(), fb, NULL);
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
        helper.cb_attachments_ = att_state1;
        helper.gp_ci_.pColorBlendState = nullptr;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00753");
}

TEST_F(VkLayerTest, PointSizeFailure) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize but not writing to it
    const char NoPointSizeVertShader[] =
        "#version 450\n"
        "vec2 vertices[3];\n"
        "out gl_PerVertex\n"
        "{\n"
        "    vec4 gl_Position;\n"
        "    float gl_PointSize;\n"
        "};\n"
        "void main() {\n"
        "    vertices[0] = vec2(-1.0, -1.0);\n"
        "    vertices[1] = vec2( 1.0, -1.0);\n"
        "    vertices[2] = vec2( 0.0,  1.0);\n"
        "    gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);\n"
        "}\n";
    VkShaderObj vs(m_device, NoPointSizeVertShader, VK_SHADER_STAGE_VERTEX_BIT, this);

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

    VkShaderObj vs(m_device, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);

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
    static char const *gsSource =
        "#version 450\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 1) out;\n"
        "void main() {\n"
        "   gl_Position = vec4(1.0, 0.5, 0.5, 0.0);\n"
        "   EmitVertex();\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj gs(m_device, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, this);

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

    const std::string gsSource = R"(
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

    VkShaderObj vs(m_device, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj gs(m_device, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, this);

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

    static const char *gsSource =
        "#version 450\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 1) out;\n"
        "in gl_PerVertex\n"
        "{\n"
        "    vec4 gl_Position;\n"
        "    float gl_PointSize;\n"
        "    float gl_ClipDistance[];\n"
        "} gl_in[];\n"
        "void main()\n"
        "{\n"
        "    gl_Position = gl_in[0].gl_Position;\n"
        "    gl_PointSize = gl_in[0].gl_PointSize;\n"
        "    EmitVertex();\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj gs(m_device, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, this);

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

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
    ds_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &layout_binding;
    VkDescriptorSetLayout ds_layout = {};
    VkResult err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    // Create an array of DSLs, one larger than the physical limit
    const auto excess_layouts = 1 + m_device->phy().properties().limits.maxBoundDescriptorSets;
    std::vector<VkDescriptorSetLayout> dsl_array(excess_layouts, ds_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check extension support
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
        printf("%s test requires %s extension. Skipping.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT density_map2_properties =
        lvl_init_struct<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>();
    VkPhysicalDeviceProperties2KHR properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&density_map2_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t max_subsampled_samplers = density_map2_properties.maxDescriptorSetSubsampledSamplers;

    // Note: Adding this check in case mock ICDs don't initialize min-max values correctly
    if (max_subsampled_samplers == 0) {
        printf("%s maxDescriptorSetSubsampledSamplers limit (%d) must be greater than 0. Skipping.\n", kSkipPrefix,
               max_subsampled_samplers);
        return;
    }

    if (max_subsampled_samplers >= properties2.properties.limits.maxDescriptorSetSamplers) {
        printf("%s test assumes maxDescriptorSetSubsampledSamplers limit (%d) is less than overall sampler limit (%d). Skipping.\n",
               kSkipPrefix, max_subsampled_samplers, properties2.properties.limits.maxDescriptorSetSamplers);
        return;
    }

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
    ds_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_ci.pNext = NULL;
    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.flags |= VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    VkSampler sampler = VK_NULL_HANDLE;
    VkResult err = vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // just make all the immutable samplers point to the same sampler
    std::vector<VkSampler> immutableSamplers;
    immutableSamplers.resize(max_subsampled_samplers);
    for (uint32_t sampler_idx = 0; sampler_idx < max_subsampled_samplers; sampler_idx++) {
        immutableSamplers[sampler_idx] = sampler;
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
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_sampler_vuid = "VUID-VkPipelineLayoutCreateInfo-pImmutableSamplers-03566";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_sampler_vuid);

    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);  // Unnecessary but harmless if test passed
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, NULL);
}

TEST_F(VkLayerTest, CreatePipelineLayoutExcessPerStageDescriptors) {
    TEST_DESCRIPTION("Attempt to create a pipeline layout where total descriptors exceed per-stage limits");

    bool descriptor_indexing = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    descriptor_indexing = descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    descriptor_indexing =
        descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
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
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
    ds_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_ci.pNext = NULL;
    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
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
    descriptor_indexing = descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    descriptor_indexing =
        descriptor_indexing && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (descriptor_indexing) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
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
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
    ds_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_ci.pNext = NULL;
    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkPhysicalDeviceVulkan12Features features12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr};
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            features12.drawIndirectCount = VK_TRUE;
        }
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features12));
    bool has_khr_indirect = DeviceExtensionEnabled(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
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
    VkBufferCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer.handle(), 0);
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
    pipe.cs_.reset(new VkShaderObj(m_device, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "exceeds device limit maxComputeWorkGroupSize[0]");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "exceeds device limit maxComputeWorkGroupSize[1]");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "exceeds device limit maxComputeWorkGroupSize[2]");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "features-limits-maxComputeWorkGroupInvocations");
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

    VkShaderObj cs_obj(m_device, cs_text, VK_SHADER_STAGE_COMPUTE_BIT, this);
    pipe.cs_.reset(new VkShaderObj(m_device, cs_text, VK_SHADER_STAGE_COMPUTE_BIT, this));
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

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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
    char const *csSource =
        "#version 450\n"
        "layout(local_size_x=1, local_size_y=1, local_size_z=1) in;\n"
        "void main(){\n"
        "   if (gl_GlobalInvocationID.x >= 0) { return; }\n"
        "}\n";

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.InitInfo();
    cs_pipeline.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_VERTEX_BIT;  // override with wrong value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkComputePipelineCreateInfo-stage-00701");
    cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
    m_errorMonitor->VerifyFound();
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

TEST_F(VkLayerTest, CreateGraphicsPipelineWithBadBasePointer) {
    TEST_DESCRIPTION("Create Graphics Pipeline with pointers that must be ignored by layers");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);

    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);

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
    auto pipeline_layout_create_info = lvl_init_struct<VkPipelineLayoutCreateInfo>();
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

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = {};
    vadf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    vadf.vertexAttributeInstanceRateDivisor = VK_TRUE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = {};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &vadf;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props = {};
    pdvad_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 pd_props2 = {};
    pd_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    pd_props2.pNext = &pdvad_props;
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = {};
    pvids_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
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

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = {};
    vadf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    vadf.vertexAttributeInstanceRateDivisor = VK_FALSE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    VkPhysicalDeviceFeatures2 pd_features2 = {};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &vadf;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props = {};
    pdvad_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 pd_props2 = {};
    pd_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    pd_props2.pNext = &pdvad_props;
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    vibdd.binding = 0;
    vibdd.divisor = 2;
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = {};
    pvids_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
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

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = {};
    vadf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    vadf.vertexAttributeInstanceRateDivisor = VK_TRUE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    VkPhysicalDeviceFeatures2 pd_features2 = {};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &vadf;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    vibdd.binding = 0;
    vibdd.divisor = 0;
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = {};
    pvids_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
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

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
        ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        ds_pool_ci.pNext = NULL;
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

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
        ds_layout_ci.sType =
VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ds_layout_ci.pNext = NULL;
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

    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
        pipeline_layout_ci.sType =
VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_ci.pNext = NULL;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &ds_layout;

    VkPipelineLayout pipeline_layout;
    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL,
&pipeline_layout);
    ASSERT_VK_SUCCESS(err);

    VkPipelineShaderStageCreateInfo shaderStages[3];
    memset(&shaderStages, 0, 3 * sizeof(VkPipelineShaderStageCreateInfo));

    VkShaderObj vs(m_device,bindStateVertShaderText,VK_SHADER_STAGE_VERTEX_BIT,
this);
    // Just using VS txt for Tess shaders as we don't care about functionality
    VkShaderObj
tc(m_device,bindStateVertShaderText,VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
this);
    VkShaderObj
te(m_device,bindStateVertShaderText,VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
this);

    shaderStages[0].sType  =
VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].shader = vs.handle();
    shaderStages[1].sType  =
VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    shaderStages[1].shader = tc.handle();
    shaderStages[2].sType  =
VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[2].stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    shaderStages[2].shader = te.handle();

    VkPipelineInputAssemblyStateCreateInfo iaCI = {};
        iaCI.sType =
VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        iaCI.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    VkPipelineTessellationStateCreateInfo tsCI = {};
        tsCI.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tsCI.patchControlPoints = 0; // This will cause an error

    VkGraphicsPipelineCreateInfo gp_ci = {};
        gp_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        gp_ci.pNext = NULL;
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

    VkPipelineCacheCreateInfo pc_ci = {};
        pc_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        pc_ci.pNext = NULL;
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
        {0,
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
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {1,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {0,
         viewports,
         0,
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
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
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
        {0,
         nullptr,
         0,
         nullptr,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
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
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {1,
         viewports,
         2,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {0,
         viewports,
         0,
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
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01220"}},
        {2,
         viewports,
         0,
         scissors,
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
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
         {"VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
          "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}},
    };

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    for (const auto &test_case : dyn_test_cases) {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
            dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
            VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
            dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
            VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
            dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic viewport(s) 0 are used by pipeline state object, ");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_dyn_vp.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 1, 1,
                       &m_viewports[0]);  // Forgetting to set needed 0th viewport (PSO viewportCount == 1)
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Dynamic scissor(s) 0 are used by pipeline state object, ");
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
    } else {
        printf("%s VK_EXT_pipeline_creation_cache_control not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT cache_control_features = {};
    cache_control_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT;
    cache_control_features.pNext = nullptr;
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
    VkPipelineCacheCreateInfo cache_create_info = {};
    cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    cache_create_info.pNext = nullptr;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Num samples mismatch! ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.pNext = NULL;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);  // We shouldn't need a fragment shader
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

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.pNext = NULL;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const auto set_MSAA = [&](CreatePipelineHelper &helper) {
        helper.pipe_ms_state_ci_ = pipe_ms_state_ci;
        helper.cb_ci_.attachmentCount = 0;
    };
    CreatePipelineHelper::OneshotTest(*this, set_MSAA, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-attachmentCount-00746");
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

    const auto set_dsb_src_color_enable = [&](CreatePipelineHelper &helper) { helper.cb_attachments_ = cb_attachments; };

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
    VkShaderModuleCreateInfo moduleCreateInfo;
    struct icd_spv_header spv;

    spv.magic = ICD_SPV_MAGIC;
    spv.version = ICD_SPV_VERSION;
    spv.gen_magic = 0;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.pCode = (const uint32_t *)&spv;
    moduleCreateInfo.codeSize = 4;
    moduleCreateInfo.flags = 0;
    vk::CreateShaderModule(m_device->device(), &moduleCreateInfo, NULL, &module);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-01376");
    std::vector<unsigned int> shader;
    VkShaderModuleCreateInfo module_create_info;
    VkShaderModule shader_module;
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.pNext = NULL;
    this->GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText, shader);
    module_create_info.pCode = shader.data();
    // Introduce failure by making codeSize a non-multiple of 4
    module_create_info.codeSize = shader.size() * sizeof(unsigned int) - 1;
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
    VkShaderModuleCreateInfo moduleCreateInfo;
    struct icd_spv_header spv;

    spv.magic = (uint32_t)~ICD_SPV_MAGIC;
    spv.version = ICD_SPV_VERSION;
    spv.gen_magic = 0;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.pCode = (const uint32_t *)&spv;
    moduleCreateInfo.codeSize = sizeof(spv) + 16;
    moduleCreateInfo.flags = 0;
    vk::CreateShaderModule(m_device->device(), &moduleCreateInfo, NULL, &module);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineVertexOutputNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex output that is not consumed by the fragment stage");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "layout(location=0) out float x;\n"
        "void main(){\n"
        "   gl_Position = vec4(1);\n"
        "   x = 0;\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit, "not consumed by fragment shader");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderSpecializationApplied) {
    TEST_DESCRIPTION(
        "Make sure specialization constants get applied during shader validation by using a value that breaks compilation.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Size an array using a specialization constant of default value equal to 1.
    std::string const fs_src = R"(
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
    VkShaderObj fs(m_device, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-module-04145");
}

TEST_F(VkLayerTest, CreatePipelineCheckShaderBadSpecializationOffsetOutOfBounds) {
    TEST_DESCRIPTION("Challenge core_validation with shader validation issues related to vkCreateGraphicsPipelines.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource =
        "#version 450\n"
        "layout (constant_id = 0) const float r = 0.0f;\n"
        "layout(location = 0) out vec4 uFragColor;\n"
        "void main(){\n"
        "   uFragColor = vec4(r,1,0,1);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *fsSource =
        "#version 450\n"
        "layout (constant_id = 0) const float r = 0.0f;\n"
        "layout(location = 0) out vec4 uFragColor;\n"
        "void main(){\n"
        "   uFragColor = vec4(r,1,0,1);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout (std140, set = 0, binding = 0) uniform buf {\n"
        "    mat4 mvp;\n"
        "} ubuf;\n"
        "void main(){\n"
        "   gl_Position = ubuf.mvp * vec4(1);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout (std140, set = 0, binding = 0) uniform buf {\n"
        "    mat4 mvp;\n"
        "} ubuf;\n"
        "void main(){\n"
        "   gl_Position = ubuf.mvp * vec4(1);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(push_constant, std430) uniform foo { float x; } consts;\n"
        "void main(){\n"
        "   gl_Position = vec4(consts.x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Shader push-constant buffer member 0 at offset 0 is not declared in pipeline layout");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
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

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   dvec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "   color = vec4(green);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device);

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Shader requires VkPhysicalDeviceFeatures::shaderFloat64 but is not enabled on the device");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateShaderModuleCheckBadCapability) {
    TEST_DESCRIPTION("Create a shader in which a capability declared by the shader is not supported.");
    // Note that this failure message comes from spirv-tools, specifically the validator.

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string spv_source = R"(
                  OpCapability ImageRect
                  OpEntryPoint Vertex %main "main"
          %main = OpFunction %void None %3
                  OpReturn
                  OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Capability ImageRect is not allowed by Vulkan");

    std::vector<unsigned int> spv;
    VkShaderModuleCreateInfo module_create_info;
    VkShaderModule shader_module;
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.pNext = NULL;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, spv_source.data(), spv);
    module_create_info.pCode = spv.data();
    module_create_info.codeSize = spv.size() * sizeof(unsigned int);
    module_create_info.flags = 0;

    VkResult err = vk::CreateShaderModule(m_device->handle(), &module_create_info, NULL, &shader_module);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyShaderModule(m_device->handle(), shader_module, NULL);
    }
}

TEST_F(VkLayerTest, CreatePipelineFragmentInputNotProvided) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input which is not present in the outputs of the previous stage");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in float x;\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(x);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *fsSource =
        "#version 450\n"
        "\n"
        "in block { layout(location=0) float x; } ins;\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(ins.x);\n"
        "}\n";

    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "not written by vertex shader");
}

TEST_F(VkLayerTest, CreatePipelineVsFsTypeMismatchArraySize) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched array sizes across the vertex->fragment shader interface");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out float x[2];\n"
        "void main(){\n"
        "   x[0] = 0; x[1] = 0;\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in float x[1];\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(x[0]);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        "Type mismatch on location 0.0: 'ptr to output arr[2] of float32' vs 'ptr to input arr[1] of float32'");
}

TEST_F(VkLayerTest, CreatePipelineVsFsTypeMismatch) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched types across the vertex->fragment shader interface");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out int x;\n"
        "void main(){\n"
        "   x = 0;\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in float x;\n" /* VS writes int */
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "out block { layout(location=0) int x; } outs;\n"
        "void main(){\n"
        "   outs.x = 0;\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "in block { layout(location=0) float x; } ins;\n" /* VS writes int */
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(ins.x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "out block { layout(location=1) float x; } outs;\n"
        "void main(){\n"
        "   outs.x = 0;\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "in block { layout(location=0) float x; } ins;\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(ins.x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "location 0.0 which is not written by vertex shader");
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByComponent) {
    TEST_DESCRIPTION(
        "Test that an error is produced for component mismatches across the vertex->fragment shader interface. It's not enough to "
        "have the same set of locations in use; matching is defined in terms of spirv variables.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "\n"
        "out block { layout(location=0, component=0) float x; } outs;\n"
        "void main(){\n"
        "   outs.x = 0;\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "in block { layout(location=0, component=1) float x; } ins;\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(ins.x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "location 0.1 which is not written by vertex shader");
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByPrecision) {
    TEST_DESCRIPTION("Test that the RelaxedPrecision decoration is validated to match");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "layout(location=0) out mediump float x;\n"
        "void main() { gl_Position = vec4(0); x = 1.0; }\n";
    char const *fsSource =
        "#version 450\n"
        "layout(location=0) in highp float x;\n"
        "layout(location=0) out vec4 color;\n"
        "void main() { color = vec4(x); }\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "differ in precision");
}

TEST_F(VkLayerTest, CreatePipelineVsFsMismatchByPrecisionBlock) {
    TEST_DESCRIPTION("Test that the RelaxedPrecision decoration is validated to match");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "out block { layout(location=0) mediump float x; };\n"
        "void main() { gl_Position = vec4(0); x = 1.0; }\n";
    char const *fsSource =
        "#version 450\n"
        "in block { layout(location=0) highp float x; };\n"
        "layout(location=0) out vec4 color;\n"
        "void main() { color = vec4(x); }\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in vec4 x;\n" /* not provided */
        "void main(){\n"
        "   gl_Position = x;\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in int x;\n" /* attrib provided float */
        "void main(){\n"
        "   gl_Position = vec4(x);\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Multiple shaders provided for stage VK_SHADER_STAGE_VERTEX_BIT");
}

TEST_F(VkLayerTest, CreatePipelineMissingEntrypoint) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this, "foo");

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "No entrypoint found named `foo`");
}

TEST_F(VkLayerTest, CreatePipelineDepthStencilRequired) {
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "pDepthStencilState is NULL when rasterization is enabled and subpass uses a depth/stencil attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *tcsSource =
        "#version 450\n"
        "layout(location=0) out int x[];\n"
        "layout(vertices=3) out;\n"
        "void main(){\n"
        "   gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;\n"
        "   gl_TessLevelInner[0] = 1;\n"
        "   x[gl_InvocationID] = gl_InvocationID;\n"
        "}\n";
    char const *tesSource =
        "#version 450\n"
        "layout(triangles, equal_spacing, cw) in;\n"
        "layout(location=0) patch in int x;\n"
        "void main(){\n"
        "   gl_Position.xyz = gl_TessCoord;\n"
        "   gl_Position.w = x;\n"
        "}\n";
    VkShaderObj tcs(m_device, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj tes(m_device, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);

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

    char const *tcsSource =
        "#version 450\n"
        "layout(vertices=3) out;\n"
        "void main(){\n"
        "   gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;\n"
        "   gl_TessLevelInner[0] = 1;\n"
        "}\n";
    char const *tesSource =
        "#version 450\n"
        "layout(triangles, equal_spacing, cw) in;\n"
        "void main(){\n"
        "   gl_Position.xyz = gl_TessCoord;\n"
        "   gl_Position.w = 0;\n"
        "}\n";
    VkShaderObj tcs(m_device, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj tes(m_device, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);

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

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) in float x;\n" /* attrib provided float */
        "void main(){\n"
        "   gl_Position = vec4(x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

    m_errorMonitor->VerifyFound();
    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.vi_ci_.pVertexBindingDescriptions = input_bindings;
        helper.vi_ci_.vertexBindingDescriptionCount = 2;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    m_errorMonitor->SetUnexpectedError("VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "Duplicate vertex input binding descriptions for binding 0");
}

TEST_F(VkLayerTest, CreatePipelineFragmentOutputNotWritten) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader which does not provide an output for one of the pipeline's color "
        "attachments");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj fs(m_device, bindStateMinimalShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.cb_attachments_.colorWriteMask = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Attachment 0 not written by fragment shader");
}

TEST_F(VkLayerTest, CreatePipelineFragmentOutputNotConsumed) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a fragment shader which provides a spurious output with no matching attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 x;\n"
        "layout(location=1) out vec4 y;\n" /* no matching attachment for this */
        "void main(){\n"
        "   x = vec4(1);\n"
        "   y = vec4(1);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    VkShaderObj fs(m_device, bindStateMinimalShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = {};
    ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
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
        "Test that an error is produced when alpha to coverage is enabled but output at location 0 doesn't have alpha channel.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(0u));

    char const *fsSource =
        "#version 450\n"
        "layout(location=0) out vec3 x;\n"
        "\n"
        "void main(){\n"
        "   x = vec3(1);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = {};
    ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
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

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out ivec4 x;\n" /* not UNORM */
        "void main(){\n"
        "   x = ivec4(1);\n"
        "}\n";

    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "does not match fragment shader output type");
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
        } else {
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

        std::string fsSourceStr =
            "#version 450\n"
            "\n"
            "layout(location=0) out vec4 color;\n"
            "\n"
            "void main(){\n"
            "    color = vec4(1);\n"
            "}\n";

        VkShaderObj vs(m_device, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT, this);
        VkShaderObj fs(m_device, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, this);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        switch (overflow) {
            case 2:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                  "Vertex shader output variable uses location that exceeds component limit "
                                                  "VkPhysicalDeviceLimits::maxVertexOutputComponents");
                break;
            case 1:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Vertex shader exceeds VkPhysicalDeviceLimits::maxVertexOutputComponents",
                                   "Vertex shader output variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxVertexOutputComponents"});
                break;
            default:
                assert(0);
            case 0:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
                break;
        }
    }
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
        } else {
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
        } else {
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

        VkShaderObj tcs(m_device, tcsSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
        VkShaderObj tes(m_device, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.pNext = NULL;
        inputAssemblyInfo.flags = 0;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessInfo = {};
        tessInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessInfo.pNext = NULL;
        tessInfo.flags = 0;
        tessInfo.patchControlPoints = 3;

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                                     helper.fs_->GetStageCreateInfo()};
            helper.gp_ci_.pTessellationState = &tessInfo;
            helper.gp_ci_.pInputAssemblyState = &inputAssemblyInfo;
        };

        switch (overflow) {
            case 2:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Tessellation control shader input variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents",
                                   "Tessellation control shader output variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents"});
                break;
            case 1:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Tessellation control shader exceeds "
                                   "VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents",
                                   "Tessellation control shader exceeds "
                                   "VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents",
                                   "Tessellation control shader input variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents",
                                   "Tessellation control shader output variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents"});
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
        } else {
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
        } else {
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

        VkShaderObj tcs(m_device, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
        VkShaderObj tes(m_device, tesSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.pNext = NULL;
        inputAssemblyInfo.flags = 0;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessInfo = {};
        tessInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessInfo.pNext = NULL;
        tessInfo.flags = 0;
        tessInfo.patchControlPoints = 3;

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                                     helper.fs_->GetStageCreateInfo()};
            helper.gp_ci_.pTessellationState = &tessInfo;
            helper.gp_ci_.pInputAssemblyState = &inputAssemblyInfo;
        };

        switch (overflow) {
            case 2:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Tessellation evaluation shader input variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents",
                                   "Tessellation evaluation shader output variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents"});
                break;
            case 1:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{
                        "Tessellation evaluation shader exceeds VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents",
                        "Tessellation evaluation shader exceeds VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents",
                        "Tessellation evaluation shader input variable uses location that exceeds component limit "
                        "VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents",
                        "Tessellation evaluation shader output variable uses location that exceeds component limit "
                        "VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents"});
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
    if (IsPlatform(kPixel3)) {
        printf("%s This test should not run on Pixel 3\n", kSkipPrefix);
        return;
    }
    if (IsPlatform(kPixel3aXL)) {
        printf("%s This test should not run on Pixel 3a XL\n", kSkipPrefix);
        return;
    }
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
        } else {
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

        VkShaderObj gs(m_device, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT, this);

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };

        switch (overflow) {
            case 2:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Geometry shader input variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxGeometryInputComponents",
                                   "Geometry shader output variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxGeometryOutputComponents"});
                break;
            case 1:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Geometry shader exceeds VkPhysicalDeviceLimits::maxGeometryInputComponents",
                                   "Geometry shader exceeds VkPhysicalDeviceLimits::maxGeometryOutputComponents",
                                   "Geometry shader input variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxGeometryInputComponents",
                                   "Geometry shader output variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxGeometryOutputComponents",
                                   "Geometry shader exceeds VkPhysicalDeviceLimits::maxGeometryTotalOutputComponents"});
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
        VkShaderObj fs(m_device, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, this);

        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Shader-InputNotProduced");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        switch (overflow) {
            case 2:
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                  "Fragment shader input variable uses location that exceeds component limit "
                                                  "VkPhysicalDeviceLimits::maxFragmentInputComponents");
                break;
            case 1:
                CreatePipelineHelper::OneshotTest(
                    *this, set_info, kErrorBit,
                    vector<string>{"Fragment shader exceeds VkPhysicalDeviceLimits::maxFragmentInputComponents",
                                   "Fragment shader input variable uses location that exceeds component limit "
                                   "VkPhysicalDeviceLimits::maxFragmentInputComponents"});
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
        VkShaderObj gs(m_device, gsSourceStr, VK_SHADER_STAGE_GEOMETRY_BIT, this);

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

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

TEST_F(VkLayerTest, CreatePipelinePushConstantsNotInLayout) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming push constants which are not provided in the pipeline layout");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource =
        "#version 450\n"
        "\n"
        "layout(push_constant, std430) uniform foo { float x; } consts;\n"
        "void main(){\n"
        "   gl_Position = vec4(consts.x);\n"
        "}\n";

    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    /* should have generated an error -- no push constant ranges provided! */
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "not declared in pipeline layout");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineInputAttachmentMissing) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming an input attachment which is not included in the subpass "
        "description");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;\n"
        "layout(location=0) out vec4 color;\n"
        "void main() {\n"
        "   color = subpassLoad(x);\n"
        "}\n";

    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;\n"
        "layout(location=0) out vec4 color;\n"
        "void main() {\n"
        "   color = subpassLoad(x);\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[1];\n"
        "layout(location=0) out vec4 color;\n"
        "void main() {\n"
        "   color = subpassLoad(xs[0]);\n"
        "}\n";

    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

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

    char const *csSource =
        "#version 450\n"
        "\n"
        "layout(local_size_x=1) in;\n"
        "layout(set=0, binding=0) buffer block { vec4 x; };\n"
        "void main(){\n"
        "   x = vec4(1);\n"
        "}\n";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Shader uses descriptor slot 0.0");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateComputePipelineDescriptorTypeMismatch) {
    TEST_DESCRIPTION("Test that an error is produced for a pipeline consuming a descriptor-backed resource of a mismatched type");

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *csSource =
        "#version 450\n"
        "\n"
        "layout(local_size_x=1) in;\n"
        "layout(set=0, binding=0) buffer block { vec4 x; };\n"
        "void main() {\n"
        "   x.x = 1.0f;\n"
        "}\n";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this));
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
    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
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
    VkPipelineMultisampleStateCreateInfo ms_state_ci = {};
    ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
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
        att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
        att[1].samples = test_case.depth_samples;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference dr = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &cr;
        sp.pResolveAttachments = NULL;
        sp.pDepthStencilAttachment = &dr;

        VkRenderPassCreateInfo rpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpi.attachmentCount = 2;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        VkRenderPass rp;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-pDepthStencilAttachment-01418");
        VkResult err = vk::CreateRenderPass(m_device->device(), &rpi, nullptr, &rp);
        m_errorMonitor->VerifyNotFound();

        ASSERT_VK_SUCCESS(err);

        VkPipelineDepthStencilStateCreateInfo ds = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        VkPipelineCoverageModulationStateCreateInfoNV cmi = {VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV};

        // Create a dummy modulation table that can be used for the positive
        // coverageModulationTableCount test.
        std::vector<float> cm_table{};

        const auto break_samples = [&cmi, &rp, &ds, &cm_table, &test_case](CreatePipelineHelper &helper) {
            cm_table.resize(test_case.raster_samples / test_case.color_samples);

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
        att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = ds_format;
        att[1].samples = test_case.depth_samples;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference dr = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &cr;
        sp.pResolveAttachments = NULL;
        sp.pDepthStencilAttachment = &dr;

        VkRenderPassCreateInfo rpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
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

        VkPipelineDepthStencilStateCreateInfo ds = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

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
    PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV
        vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
            (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)vk::GetInstanceProcAddr(
                instance(), "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");

    ASSERT_NO_FATAL_FAILURE(vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(gpu(), &combination_count, nullptr));
    if (combination_count < 1) {
        printf("%s No mixed sample combinations are supported, skipping tests.\n", kSkipPrefix);
        return;
    }
    combinations.resize(combination_count);
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
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
        att[1].samples = test_case.depth_samples;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference dr = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sp = {};
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &cr;
        sp.pResolveAttachments = nullptr;
        sp.pDepthStencilAttachment = (test_case.depth_samples) ? &dr : nullptr;

        VkRenderPassCreateInfo rpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpi.attachmentCount = (test_case.depth_samples) ? 2 : 1;
        rpi.pAttachments = att;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        VkRenderPass rp;
        ASSERT_VK_SUCCESS(vk::CreateRenderPass(m_device->device(), &rpi, nullptr, &rp));

        VkPipelineDepthStencilStateCreateInfo dss = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        VkPipelineCoverageReductionStateCreateInfoNV crs = {VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV};

        const auto break_samples = [&rp, &dss, &crs, &test_case](CreatePipelineHelper &helper) {
            crs.flags = 0;
            crs.coverageReductionMode = test_case.coverage_reduction_mode;

            helper.pipe_ms_state_ci_.pNext = &crs;
            helper.pipe_ms_state_ci_.rasterizationSamples = test_case.raster_samples;
            helper.gp_ci_.renderPass = rp;
            helper.gp_ci_.pDepthStencilState = (test_case.depth_samples) ? &dss : nullptr;
        };

        CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit, test_case.vuid, test_case.positiveTest);

        vk::DestroyRenderPass(m_device->device(), rp, nullptr);
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
        att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        att[1].format = VK_FORMAT_R8G8B8A8_UNORM;
        att[1].samples = VK_SAMPLE_COUNT_1_BIT;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

        VkRenderPassCreateInfo rpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpi.attachmentCount = att.size();
        rpi.pAttachments = att.data();
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sp;

        const std::array<VkPipelineColorBlendAttachmentState, 3> cba = {{{}, {}, {}}};

        VkPipelineColorBlendStateCreateInfo cbi = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        cbi.attachmentCount = cba.size();
        cbi.pAttachments = cba.data();

        VkRenderPass rp;
        VkResult err = vk::CreateRenderPass(m_device->device(), &rpi, nullptr, &rp);
        ASSERT_VK_SUCCESS(err);

        VkPipelineCoverageToColorStateCreateInfoNV cci = {VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV};

        const auto break_samples = [&cci, &cbi, &rp, &test_case](CreatePipelineHelper &helper) {
            cci.coverageToColorEnable = test_case.enabled;
            cci.coverageToColorLocation = test_case.location;

            helper.pipe_ms_state_ci_.pNext = &cci;
            helper.gp_ci_.renderPass = rp;
            helper.gp_ci_.pColorBlendState = &cbi;
        };

        CreatePipelineHelper::OneshotTest(*this, break_samples, kErrorBit,
                                          "VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404",
                                          test_case.positive);

        vk::DestroyRenderPass(m_device->device(), rp, nullptr);
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

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto float16_features = lvl_init_struct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto cooperative_matrix_features = lvl_init_struct<VkPhysicalDeviceCooperativeMatrixFeaturesNV>(&float16_features);
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&cooperative_matrix_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource =
        "#version 450\n"
        "#extension GL_NV_cooperative_matrix : enable\n"
        "#extension GL_KHR_shader_subgroup_basic : enable\n"
        "#extension GL_KHR_memory_scope_semantics : enable\n"
        "#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable\n"
        "layout(local_size_x = 32) in;\n"
        "layout(constant_id = 0) const uint C0 = 1;"
        "layout(constant_id = 1) const uint C1 = 1;"
        "void main() {\n"
        // Bad type
        "   fcoopmatNV<16, gl_ScopeSubgroup, 3, 5> badSize = fcoopmatNV<16, gl_ScopeSubgroup, 3, 5>(float16_t(0.0));\n"
        // Not a valid multiply when C0 != C1
        "   fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> A;\n"
        "   fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> B;\n"
        "   fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> C;\n"
        "   coopMatMulAddNV(A, B, C);\n"
        "}\n";

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
    pipe.cs_.reset(new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this, "main", false, &specInfo));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-module-04145");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixType");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixMulAdd");
    m_errorMonitor->SetUnexpectedError("VUID-VkPipelineShaderStageCreateInfo-module-04145");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SubgroupSupportedOperations) {
    TEST_DESCRIPTION("Test shader validation support for subgroup supportedOperations.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
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

    VkPhysicalDeviceSubgroupProperties subgroup_prop = GetSubgroupProperties(instance(), gpu());

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
    pipeline_layout_ci.flags = 0;
    pipeline_layout_ci.setLayoutCount = 0;
    pipeline_layout_ci.pSetLayouts = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);

    const std::pair<const char *, VkSubgroupFeatureFlagBits> capabilities[] = {
        {"GroupNonUniform", VK_SUBGROUP_FEATURE_BASIC_BIT},
        {"GroupNonUniformVote", VK_SUBGROUP_FEATURE_VOTE_BIT},
        {"GroupNonUniformArithmetic", VK_SUBGROUP_FEATURE_ARITHMETIC_BIT},
        {"GroupNonUniformBallot", VK_SUBGROUP_FEATURE_BALLOT_BIT},
        {"GroupNonUniformShuffle", VK_SUBGROUP_FEATURE_SHUFFLE_BIT},
        {"GroupNonUniformShuffleRelative", VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT},
        {"GroupNonUniformClustered", VK_SUBGROUP_FEATURE_CLUSTERED_BIT},
        {"GroupNonUniformQuad", VK_SUBGROUP_FEATURE_QUAD_BIT},
    };

    for (auto &capability : capabilities) {
        std::string spv_source[3];

        spv_source[0] = "OpCapability " + std::string(capability.first) + "\n" + R"(
                   OpCapability Shader
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
             %40 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

        spv_source[1] = "OpCapability " + std::string(capability.first) + "\n" + R"(
                   OpCapability Shader
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint Vertex %main "main"
           %void = OpTypeVoid
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
             %40 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

        spv_source[2] = "OpCapability " + std::string(capability.first) + "\n" + R"(
                   OpCapability Shader
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint Fragment %main "main"
                   OpExecutionMode %main OriginUpperLeft
           %void = OpTypeVoid
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
             %40 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

        VkShaderModule shader_module[3];
        VkPipelineShaderStageCreateInfo stage[3];

        for (int i = 0; i < 3; ++i) {
            // CreateShaderModule
            std::vector<unsigned int> spv;
            VkShaderModuleCreateInfo module_create_info;
            module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            module_create_info.pNext = NULL;
            ASMtoSPV(SPV_ENV_VULKAN_1_1, 0, spv_source[i].data(), spv);
            module_create_info.pCode = spv.data();
            module_create_info.codeSize = spv.size() * sizeof(unsigned int);
            module_create_info.flags = 0;

            VkResult result = vk::CreateShaderModule(m_device->handle(), &module_create_info, NULL, &shader_module[i]);

            // NOTE: It appears that for the case of invalid capabilities some drivers (recent AMD) fail at CreateShaderModule time.
            //       Likely the capability test should be moved up to CSM time, implementing ShaderModuleCreateInfo-pCode-01090
            //       Note(2) -- yes I truncated the above VUID s.t. the VUID checking tools would not catch it.
            if (result != VK_SUCCESS) shader_module[i] = VK_NULL_HANDLE;

            stage[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage[i].pNext = nullptr;
            stage[i].flags = 0;
            // stage[i].stage initialized later;
            stage[i].module = shader_module[i];
            stage[i].pName = "main";
            stage[i].pSpecializationInfo = nullptr;
        }

        // CreateComputePipelines
        VkComputePipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_info.pNext = nullptr;
        pipeline_info.flags = 0;
        pipeline_info.layout = pipeline_layout;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;
        pipeline_info.stage = stage[0];
        pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

        if (pipeline_info.stage.module != VK_NULL_HANDLE) {
            if (!(subgroup_prop.supportedOperations & capability.second)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VkPhysicalDeviceSubgroupProperties::supportedOperations");
            }
            if (!(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VkPhysicalDeviceSubgroupProperties::supportedStages");
            }

            VkPipeline cs_pipeline;
            vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &cs_pipeline);
            vk::DestroyPipeline(device(), cs_pipeline, nullptr);

            m_errorMonitor->VerifyFound();
        }

        if ((stage[1].module != VK_NULL_HANDLE) && (stage[2].module != VK_NULL_HANDLE)) {
            stage[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
            stage[2].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkPipelineObj pipe(m_device);
            pipe.AddShader(stage[1]);
            pipe.AddShader(stage[2]);
            pipe.AddDefaultColorAttachment();

            if (!(subgroup_prop.supportedOperations & capability.second)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VkPhysicalDeviceSubgroupProperties::supportedOperations");
            }
            if (!(subgroup_prop.supportedStages & VK_SHADER_STAGE_VERTEX_BIT)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VkPhysicalDeviceSubgroupProperties::supportedStages");
            }
            if (!(subgroup_prop.supportedOperations & capability.second)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VkPhysicalDeviceSubgroupProperties::supportedOperations");
            }
            if (!(subgroup_prop.supportedStages & VK_SHADER_STAGE_FRAGMENT_BIT)) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VkPhysicalDeviceSubgroupProperties::supportedStages");
            }
            pipe.CreateVKPipeline(pipeline_layout, renderPass());

            m_errorMonitor->VerifyFound();
        }

        vk::DestroyShaderModule(device(), shader_module[0], nullptr);
        vk::DestroyShaderModule(device(), shader_module[1], nullptr);
        vk::DestroyShaderModule(device(), shader_module[2], nullptr);
    }

    vk::DestroyPipelineLayout(device(), pipeline_layout, nullptr);
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

    auto float16_features = lvl_init_struct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto extended_types_features = lvl_init_struct<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(&float16_features);
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&extended_types_features);
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

    char const *csSource =
        "#version 450\n"
        "#extension GL_KHR_shader_subgroup_arithmetic : enable\n"
        "#extension GL_EXT_shader_subgroup_extended_types_float16 : enable\n"
        "#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable\n"
        "layout(local_size_x = 32) in;\n"
        "void main() {\n"
        "   subgroupAdd(float16_t(0.0));\n"
        "}\n";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(
        new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this, "main", false, nullptr, /*SPIR-V 1.3*/ 3));
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

    auto float16_features = lvl_init_struct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto extended_types_features = lvl_init_struct<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(&float16_features);
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&extended_types_features);
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

    char const *csSource =
        "#version 450\n"
        "#extension GL_KHR_shader_subgroup_arithmetic : enable\n"
        "#extension GL_EXT_shader_subgroup_extended_types_float16 : enable\n"
        "#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable\n"
        "layout(local_size_x = 32) in;\n"
        "void main() {\n"
        "   subgroupAdd(float16_t(0.0));\n"
        "}\n";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(
        new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this, "main", false, nullptr, /*SPIR-V 1.3*/ 3));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes");
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

    const std::string source = R"(
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
    pipe.cs_.reset(new VkShaderObj(m_device, source, VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Shader requires extension " VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
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

    auto feedback_info = lvl_init_struct<VkPipelineCreationFeedbackCreateInfoEXT>();
    VkPipelineCreationFeedbackEXT feedbacks[3] = {};
    // Set flags to known value that the driver has to overwrite
    feedbacks[0].flags = VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM_EXT;

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &feedback_info; };

    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit,
                                      "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02668",
                                      true);

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Driver data writeback check not supported by MockICD, skipping.\n", kSkipPrefix);
    } else {
        m_errorMonitor->ExpectSuccess();
        if (feedback_info.pPipelineCreationFeedback->flags == VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM_EXT) {
            m_errorMonitor->SetError("ValidationLayers did not return GraphicsPipelineFeedback driver data properly.");
        }
        m_errorMonitor->VerifyNotFound();
    }

    feedback_info.pipelineStageCreationFeedbackCount = 1;
    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit,
                                      "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02668",
                                      false);
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

    VkPipelineCreationFeedbackCreateInfoEXT feedback_info = {};
    VkPipelineCreationFeedbackEXT feedbacks[3] = {};
    feedback_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT;
    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 1;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    const auto set_info = [&](CreateComputePipelineHelper &helper) { helper.cp_ci_.pNext = &feedback_info; };

    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit, "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02669");
}

TEST_F(VkLayerTest, NVRayTracingPipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test NV ray tracing pipeline feedback stage count check.");

    if (!CreateNVRayTracingPipelineHelper::InitInstanceExtensions(*this, m_instance_extension_names)) {
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
        return;
    }

    if (!CreateNVRayTracingPipelineHelper::InitDeviceExtensions(*this, m_device_extension_names)) {
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto feedback_info = lvl_init_struct<VkPipelineCreationFeedbackCreateInfoEXT>();
    VkPipelineCreationFeedbackEXT feedbacks[4] = {};

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](CreateNVRayTracingPipelineHelper &helper) { helper.rp_ci_.pNext = &feedback_info; };

    feedback_info.pipelineStageCreationFeedbackCount = 3;
    CreateNVRayTracingPipelineHelper::OneshotPositiveTest(*this, set_feedback);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    CreateNVRayTracingPipelineHelper::OneshotTest(
        *this, set_feedback, "VUID-VkPipelineCreationFeedbackCreateInfoEXT-pipelineStageCreationFeedbackCount-02969");
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
    auto image_footprint_features = lvl_init_struct<VkPhysicalDeviceShaderImageFootprintFeaturesNV>();
    image_footprint_features.imageFootprint = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &image_footprint_features);

    char const *fsSource =
        "#version 450\n"
        "#extension GL_NV_shader_texture_footprint  : require\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(location=0) out vec4 color;\n"
        "void main(){\n"
        "  gl_TextureFootprint2DNV footprint;\n"
        "  if (textureFootprintNV(s, vec2(1.0), 5, false, footprint)) {\n"
        "    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "  } else {\n"
        "    color = vec4(vec2(footprint.anchor), vec2(footprint.offset));\n"
        "  }\n"
        "}\n";

    VkShaderObj vs(&test_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(&test_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj ds_layout(&test_device, {binding});
    ASSERT_TRUE(ds_layout.initialized());

    const VkPipelineLayoutObj pipeline_layout(&test_device, {&ds_layout});

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint but is not enabled on the device");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Shader requires extension VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint "
                                         "but is not enabled on the device");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckFragmentShaderBarycentricEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader barycentric feature which has not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Disable the fragment shader barycentric feature.
    auto fragment_shader_barycentric_features = lvl_init_struct<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV>();
    fragment_shader_barycentric_features.fragmentShaderBarycentric = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &fragment_shader_barycentric_features);

    char const *fsSource =
        "#version 450\n"
        "#extension GL_NV_fragment_shader_barycentric : require\n"
        "layout(location=0) out float value;\n"
        "void main(){\n"
        "  value = gl_BaryCoordNV.x;\n"
        "}\n";

    VkShaderObj vs(&test_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(&test_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::fragmentShaderBarycentric but is not enabled on the "
        "device");
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires extension VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::fragmentShaderBarycentric but is not "
        "enabled on the device");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckComputeShaderDerivativesEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the compute shader derivatives feature which has not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Disable the compute shader derivatives features.
    auto compute_shader_derivatives_features = lvl_init_struct<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV>();
    compute_shader_derivatives_features.computeDerivativeGroupLinear = VK_FALSE;
    compute_shader_derivatives_features.computeDerivativeGroupQuads = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &compute_shader_derivatives_features);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(&test_device, {binding});
    const VkPipelineLayoutObj pl(&test_device, {&dsl});

    char const *csSource =
        "#version 450\n"
        "#extension GL_NV_compute_shader_derivatives : require\n"
        "\n"
        "layout(local_size_x=2, local_size_y=4) in;\n"
        "layout(derivative_group_quadsNV) in;\n"
        "\n"
        "layout(set=0, binding=0) buffer InputOutputBuffer {\n"
        "  float values[];\n"
        "};\n"
        "\n"
        "void main(){\n"
        "   values[gl_LocalInvocationIndex] = dFdx(values[gl_LocalInvocationIndex]);"
        "}\n";

    VkShaderObj cs(&test_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this);

    VkComputePipelineCreateInfo cpci = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                        nullptr,
                                        0,
                                        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
                                         VK_SHADER_STAGE_COMPUTE_BIT, cs.handle(), "main", nullptr},
                                        pl.handle(),
                                        VK_NULL_HANDLE,
                                        -1};

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupQuads but is not enabled on the "
        "device");
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires extension VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupQuads but is not "
        "enabled on the device");

    VkPipeline pipe = VK_NULL_HANDLE;
    vk::CreateComputePipelines(test_device.device(), VK_NULL_HANDLE, 1, &cpci, nullptr, &pipe);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipeline(test_device.device(), pipe, nullptr);
    m_errorMonitor->VerifyFound();
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
    auto fragment_shader_interlock_features = lvl_init_struct<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>();
    fragment_shader_interlock_features.fragmentShaderSampleInterlock = VK_FALSE;
    fragment_shader_interlock_features.fragmentShaderPixelInterlock = VK_FALSE;
    fragment_shader_interlock_features.fragmentShaderShadingRateInterlock = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &fragment_shader_interlock_features);

    char const *fsSource =
        "#version 450\n"
        "#extension GL_ARB_fragment_shader_interlock : require\n"
        "layout(sample_interlock_ordered) in;\n"
        "void main(){\n"
        "}\n";

    VkShaderObj vs(&test_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(&test_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock but is not enabled on "
        "the device");
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires extension VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock but is not "
        "enabled on the device");
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineCheckDemoteToHelperInvocation) {
    TEST_DESCRIPTION("Create a pipeline requiring the demote to helper invocation feature which has not enabled on the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    std::vector<const char *> device_extension_names;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME)) {
        // Note: we intentionally do not add the required extension to the device extension list.
        //       in order to create the error below
    } else {
        // We skip this test if the extension is not supported by the driver as in some cases this will cause
        // the vk::CreateShaderModule to fail without generating an error message
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME);
        return;
    }

    auto features = m_device->phy().features();

    // Disable the demote to helper invocation feature.
    auto demote_features = lvl_init_struct<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>();
    demote_features.shaderDemoteToHelperInvocation = VK_FALSE;

    VkDeviceObj test_device(0, gpu(), device_extension_names, &features, &demote_features);

    char const *fsSource =
        "#version 450\n"
        "#extension GL_EXT_demote_to_helper_invocation : require\n"
        "void main(){\n"
        "    demote;\n"
        "}\n";

    VkShaderObj vs(&test_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(&test_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkRenderpassObj render_pass(&test_device);

    VkPipelineObj pipe(&test_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(&test_device);

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::shaderDemoteToHelperInvocation but is not "
        "enabled on "
        "the device");
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit,
        "Shader requires extension VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::shaderDemoteToHelperInvocation but "
        "is not "
        "enabled on the device");
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

    auto line_rasterization_features = lvl_init_struct<VkPhysicalDeviceLineRasterizationFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&line_rasterization_features);
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
    m_errorMonitor->VerifyFound();
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
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = binding_descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &storage_buffer_info;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = binding_descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;  // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[1].pBufferInfo = &uniform_buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    char const *csSource =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n "
        "layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;"
        "layout(set = 0, binding = 1) uniform UniformStruct { ivec4 dummy; int val; } ubo;\n"

        "void main() {\n"
        "    u_index.index = ubo.val;\n"
        "}\n";

    VkShaderObj shader_module(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this);

    VkPipelineShaderStageCreateInfo stage;
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pNext = nullptr;
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module.handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = nullptr;
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

    const std::string empty_shader = R"glsl(#version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_NV, this, "main");
    VkShaderObj ahit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_NV, this, "main");
    VkShaderObj chit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, this, "main");
    VkShaderObj miss_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_MISS_BIT_NV, this, "main");
    VkShaderObj intr_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_NV, this, "main");
    VkShaderObj call_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_NV, this, "main");

    m_errorMonitor->VerifyNotFound();

    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesNV"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesNV != nullptr);

    VkPipeline pipeline = VK_NULL_HANDLE;

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = {};
        stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = {};
        group_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_info.generalShader = VK_SHADER_UNUSED_NV;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoNV-stage-03425");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Two raygen stages
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[1].module = rgen_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = 1;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        VkPipelineShaderStageCreateInfo stage_create_info = {};
        stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = {};
        group_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 5;  // Invalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 5;  // Invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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

    auto pipleline_features = lvl_init_struct<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&pipleline_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    // Set this to true as it is a required feature
    pipleline_features.pipelineCreationCacheControl = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const std::string empty_shader = R"glsl(#version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_NV, this, "main");
    VkShaderObj ahit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_NV, this, "main");
    VkShaderObj chit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, this, "main");
    VkShaderObj miss_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_MISS_BIT_NV, this, "main");
    VkShaderObj intr_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_NV, this, "main");
    VkShaderObj call_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_NV, this, "main");
    m_errorMonitor->VerifyNotFound();
    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesNV"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesNV != nullptr);
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = {};
    stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    ;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoNV group_create_info = {};
    group_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
    group_create_info.generalShader = VK_SHADER_UNUSED_NV;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
    }
    // test for vkCreateRayTracingPipelinesNV
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
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
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_RAY_TRACING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_RAY_TRACING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_RAY_TRACING_EXTENSION_NAME);
        return;
    }
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto ray_tracing_features = lvl_init_struct<VkPhysicalDeviceRayTracingFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!ray_tracing_features.rayTracing) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const std::string empty_shader = R"glsl(#version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_KHR, this, "main");
    VkShaderObj ahit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_KHR, this, "main");
    VkShaderObj chit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, this, "main");
    VkShaderObj miss_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_MISS_BIT_KHR, this, "main");
    VkShaderObj intr_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_KHR, this, "main");
    VkShaderObj call_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_KHR, this, "main");
    m_errorMonitor->VerifyNotFound();
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = {};
    stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoKHR group_create_info = {};
    group_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 1;  // Bad index here
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.libraries.libraryCount = 0;
        pipeline_ci.stageCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-libraries-02958");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.stageCount = 1;
        pipeline_ci.groupCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-libraries-02959");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.groupCount = 1;
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.libraries.libraryCount = 1;
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetUnexpectedError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-libraryCount-03466");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
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
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03422");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    }
}

TEST_F(VkLayerTest, RayTracingPipelineShaderGroupsKHR) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_RAY_TRACING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_RAY_TRACING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_RAY_TRACING_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto ray_tracing_features = lvl_init_struct<VkPhysicalDeviceRayTracingFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!ray_tracing_features.rayTracing) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const std::string empty_shader = R"glsl(#version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_KHR, this, "main");
    VkShaderObj ahit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_KHR, this, "main");
    VkShaderObj chit_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, this, "main");
    VkShaderObj miss_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_MISS_BIT_KHR, this, "main");
    VkShaderObj intr_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_KHR, this, "main");
    VkShaderObj call_shader(m_device, empty_shader.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_KHR, this, "main");

    m_errorMonitor->VerifyNotFound();

    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);

    VkPipeline pipeline = VK_NULL_HANDLE;

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = {};
        stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = {};
        group_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Two raygen stages
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[1].module = rgen_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = 1;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyNotFound();
        vk::DestroyPipeline(m_device->device(), pipeline, NULL);
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = {};
        stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = {};
        group_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 5;  // IKHRalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 5;  // invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_ci.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
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

    VkImageMemoryBarrier imb = {};
    imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imb.pNext = nullptr;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-4098");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-4098");
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

        VkShaderObj fs(m_device, capability + source, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "Shader requires VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing but is not enabled on the device");
    }

    {
        std::string const capability{"OpCapability SampledImageArrayDynamicIndexing"};

        VkShaderObj fs(m_device, capability + source, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "Shader requires VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing but is not enabled on the device");
    }

    {
        std::string const capability{"OpCapability StorageBufferArrayDynamicIndexing"};

        VkShaderObj fs(m_device, capability + source, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "Shader requires VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing but is not enabled on the device");
    }

    {
        std::string const capability{"OpCapability StorageImageArrayDynamicIndexing"};

        VkShaderObj fs(m_device, capability + source, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "Shader requires VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing but is not enabled on the device");
    }
}

TEST_F(VkLayerTest, vertexStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if vertexPipelineStoresAndAtomics disable.");

    VkPhysicalDeviceFeatures features{};
    features.vertexPipelineStoresAndAtomics = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test StoreOp
    {
        char const *vsSource =
            "#version 450\n"
            "layout(set=0, binding=0, rgba8) uniform image2D si0;\n "
            "void main() {\n"
            "      imageStore(si0, ivec2(0), vec4(0));\n"
            "}\n";

        VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "Shader requires vertexPipelineStoresAndAtomics but is not enabled on the device");
    }

    // Test AtomicOp
    {
        char const *vsSource =
            "#version 450\n"
            "layout(set=0, binding=0, rgba8) uniform image2D si0;\n "
            "void main() {\n"
            "      imageAtomicExchange(si0, ivec2(0), 1);\n"
            "}\n";

        VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "Shader requires vertexPipelineStoresAndAtomics but is not enabled on the device");
    }
}

TEST_F(VkLayerTest, fragmentStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if fragmentStoresAndAtomics disable.");

    VkPhysicalDeviceFeatures features{};
    features.fragmentStoresAndAtomics = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(Init(&features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test StoreOp
    {
        char const *fsSource =
            "#version 450\n"
            "layout(set=0, binding=0, rgba8) uniform image2D si0;\n "
            "void main() {\n"
            "      imageStore(si0, ivec2(0), vec4(0));\n"
            "}\n";

        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "Shader requires fragmentStoresAndAtomics but is not enabled on the device");
    }

    // Test AtomicOp
    {
        char const *fsSource =
            "#version 450\n"
            "layout(set=0, binding=0, rgba8) uniform image2D si0;\n "
            "void main() {\n"
            "      imageAtomicExchange(si0, ivec2(0), 1);\n"
            "}\n";

        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "Shader requires fragmentStoresAndAtomics but is not enabled on the device");
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
    pipe.dyn_state_ci_ = {};
    pipe.dyn_state_ci_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipe.dyn_state_ci_.pNext = nullptr;
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
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor, false,
                                        false)) {
        return;
    }
    auto ray_tracing_features = lvl_init_struct<VkPhysicalDeviceRayTracingFeaturesKHR>();
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_tracing_features.rayTracingShaderGroupHandleCaptureReplay == VK_FALSE) {
        printf("%s rayTracingShaderGroupHandleCaptureReplay not enabled.\n", kSkipPrefix);
        return;
    }
    CreateNVRayTracingPipelineHelper rt_pipe(*this);
    rt_pipe.InitInfo(true /*isKHR*/);
    rt_pipe.InitState();
    rt_pipe.CreateKHRRayTracingPipeline();
    VkBuffer buffer;
    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
    auto ray_tracing_properties = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesKHR>();
    auto properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    // Check only when the reported size is
    if (ray_tracing_properties.shaderGroupHandleCaptureReplaySize > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
        vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 1, 1,
                                                          (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
        m_errorMonitor->VerifyFound();
    }
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
