/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "generated/vk_extension_helper.h"

TEST_F(PositiveVertexInput, AttributeMatrixType) {
    TEST_DESCRIPTION("Test that pipeline validation accepts matrices passed as vertex attributes");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[2];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 2; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in mat2x4 x;
        void main(){
           gl_Position = x[0] + x[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
    /* expect success */
}

TEST_F(PositiveVertexInput, AttributeArrayType) {
    TEST_DESCRIPTION("Input in OpTypeArray");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[2];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 2; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x[2];
        void main(){
           gl_Position = x[0] + x[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructType) {
    TEST_DESCRIPTION("Input is OpTypeStruct");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 32, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    input_attrib.location = 4;

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) vec4 x;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
  %_struct_3 = OpTypeStruct %v4float
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeWithArray) {
    TEST_DESCRIPTION("Input is OpTypeStruct that has an OpTypeArray. Locations are not in order netiher");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 48, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[3];
    memset(input_attribs, 0, sizeof(input_attribs));
    input_attribs[0].location = 1;
    input_attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;

    input_attribs[1].location = 4;
    input_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    input_attribs[2].location = 5;
    input_attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) vec4 y[2];
    //         layout(location = 1) vec3 x;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpMemberDecorate %_struct_3 1 Location 1
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
 %array_vec4 = OpTypeArray %v3float %uint_2
  %_struct_3 = OpTypeStruct %array_vec4 %v4float
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeSecondLocation) {
    TEST_DESCRIPTION("Input is OpTypeStruct with 2 locations");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[2] = {
        {4, 0, VK_FORMAT_R32G32B32A32_SINT, 0},
        {6, 0, VK_FORMAT_R32G32B32A32_UINT, 0},
    };

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) ivec4 x;
    //         layout(location = 6) uvec4 y;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpMemberDecorate %_struct_3 1 Location 6
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %sint = OpTypeInt 32 1
      %uint  = OpTypeInt 32 0
     %v4sint = OpTypeVector %sint 4
     %v4uint = OpTypeVector %uint 4
  %_struct_3 = OpTypeStruct %v4sint %v4uint
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeBlockLocation) {
    TEST_DESCRIPTION("Input is OpTypeStruct where the Block has the Location");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[2] = {
        {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {5, 0, VK_FORMAT_R32G32B32A32_UINT, 0},
    };

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     layout(location = 4) in VertexIn {
    //         vec4 x;
    //         uvec4 y;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpDecorate %_struct_3 Block
               OpDecorate %2 Location 4
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
      %uint  = OpTypeInt 32 0
    %v4float = OpTypeVector %float 4
     %v4uint = OpTypeVector %uint 4
  %_struct_3 = OpTypeStruct %v4float %v4uint
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeComponents) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts consuming a vertex attribute through multiple vertex shader inputs, each consuming "
        "a different subset of the components, and that fragment shader-attachment validation tolerates multiple duplicate "
        "location outputs");
    AddRequiredFeature(vkt::Feature::independentBlend);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[3];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 3; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        layout(location=1) in vec3 y1;
        layout(location=1, component=3) in float y2;
        layout(location=2) in vec4 z;
        void main(){
           gl_Position = x + vec4(y1, y2) + z;
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0, component=0) out float color0;
        layout(location=0, component=1) out float color1;
        layout(location=0, component=2) out float color2;
        layout(location=0, component=3) out float color3;
        layout(location=1, component=0) out vec2 second_color0;
        layout(location=1, component=2) out vec2 second_color1;
        void main(){
           color0 = float(1);
           second_color0 = vec2(1);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create a renderPass with two color attachments
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.CreateRenderPass();

    VkPipelineColorBlendAttachmentState cb_attachments[2];
    memset(cb_attachments, 0, sizeof(VkPipelineColorBlendAttachmentState) * 2);
    cb_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    cb_attachments[0].blendEnable = VK_FALSE;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = cb_attachments;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, CreatePipeline64BitAttributes) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts basic use of 64bit vertex attributes. This is interesting because they consume "
        "multiple locations.");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, &format_props);
    if (!(format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_R64G64B64A64_SFLOAT vertex buffers";
    }

    VkVertexInputBindingDescription input_bindings[1];
    memset(input_bindings, 0, sizeof(input_bindings));

    VkVertexInputAttributeDescription input_attribs[4];
    memset(input_attribs, 0, sizeof(input_attribs));
    input_attribs[0].location = 0;
    input_attribs[0].offset = 0;
    input_attribs[0].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[1].location = 2;
    input_attribs[1].offset = 32;
    input_attribs[1].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[2].location = 4;
    input_attribs[2].offset = 64;
    input_attribs[2].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[3].location = 6;
    input_attribs[3].offset = 96;
    input_attribs[3].format = VK_FORMAT_R64G64B64A64_SFLOAT;

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in dmat4 x;
        void main(){
           gl_Position = vec4(x[0][0]);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = input_bindings;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 4;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, VertexAttribute64bit) {
    TEST_DESCRIPTION("Use 64-bit Vertex format");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R64_SFLOAT;
    if ((m_device->FormatFeaturesBuffer(format) & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in float64_t pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription input_binding = {0, 0, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeBlockLocation64bit) {
    TEST_DESCRIPTION("Input is OpTypeStruct where the Block has the Location with 64-bit Vertex format");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, &format_props);
    if (!(format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_R64G64B64A64_SFLOAT vertex buffers";
    }

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[3] = {
        {4, 0, VK_FORMAT_R32G32B32A32_SINT, 0},
        {5, 0, VK_FORMAT_R64G64B64A64_SFLOAT, 0},  // takes 2 slots
        {7, 0, VK_FORMAT_R32G32B32A32_SINT, 0},
    };

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     layout(location = 4) in VertexIn {
    //         ivec4 x;
    //         float64 y;
    //         ivec4 z;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpCapability Float64
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpDecorate %_struct_3 Block
               OpDecorate %2 Location 4
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
    %float64 = OpTypeFloat 64
      %sint  = OpTypeInt 32 1
  %v4float64 = OpTypeVector %float64 4
     %v4sint = OpTypeVector %sint 4
  %_struct_3 = OpTypeStruct %v4sint %v4float64 %v4sint
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, Attribute64bitMissingComponent) {
    TEST_DESCRIPTION("Shader uses f64vec2, but provides too many component with R64G64B64A64, which is valid");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R64G64B64A64_SFLOAT;
    if ((m_device->FormatFeaturesBuffer(format) & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in f64vec2 pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription input_binding = {0, 32, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, VertexAttributeDivisorFirstInstance) {
    TEST_DESCRIPTION("Test VK_EXT_vertex_attribute_divisor with non zero first instance");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexAttributeInstanceRateZeroDivisor);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pdvad_props);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = vku::InitStructHelper();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.stride = 16;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    if (pdvad_props.maxVertexAttribDivisor < pvids_ci.vertexBindingDivisorCount) {
        GTEST_SKIP() << "This device does not support vertexBindingDivisors";
    }

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pNext = &pvids_ci;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &vibd;
    pipe.CreateGraphicsPipeline();

    vkt::Buffer vertex_buffer(*m_device, 256, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0u;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0u, 1u, &vertex_buffer.handle(), &offset);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 1u);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveVertexInput, VertextBindingNonLinear) {
    TEST_DESCRIPTION("Have Binding not be in a linear order");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription vtx_binding_des[3] = {
        {3, 0, VK_VERTEX_INPUT_RATE_VERTEX}, {5, 0, VK_VERTEX_INPUT_RATE_VERTEX}, {2, 0, VK_VERTEX_INPUT_RATE_VERTEX}};

    VkVertexInputAttributeDescription vtx_attri_des[3] = {
        {0, 5, VK_FORMAT_R8G8B8A8_UNORM, 0}, {1, 3, VK_FORMAT_R8G8B8A8_UNORM, 0}, {2, 2, VK_FORMAT_R8G8B8A8_UNORM, 0}};
    pipe.vi_ci_.vertexBindingDescriptionCount = 3;
    pipe.vi_ci_.pVertexBindingDescriptions = vtx_binding_des;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.vi_ci_.pVertexAttributeDescriptions = vtx_attri_des;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDeviceSize offsets[6] = {0, 0, 0, 0, 0, 0};
    VkBuffer buffers[6] = {buffer.handle(), buffer.handle(), buffer.handle(), buffer.handle(), buffer.handle(), buffer.handle()};
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 6, buffers, offsets);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveVertexInput, VertextBindingDynamicState) {
    TEST_DESCRIPTION("Test bad binding with VK_DYNAMIC_STATE_VERTEX_INPUT_EXT");
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offsets[2] = {0, 0};

    VkVertexInputBindingDescription2EXT bindings[3] = {vku::InitStructHelper(), vku::InitStructHelper(), vku::InitStructHelper()};
    bindings[0].binding = 3;
    bindings[0].divisor = 1;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[1].binding = 5;
    bindings[1].divisor = 1;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[2].binding = 2;
    bindings[2].divisor = 1;
    bindings[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription2EXT attributes[3] = {vku::InitStructHelper(), vku::InitStructHelper(),
                                                           vku::InitStructHelper()};
    attributes[0].location = 1;
    attributes[0].binding = 3;
    attributes[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[1].location = 2;
    attributes[1].binding = 5;
    attributes[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[2].location = 3;
    attributes[2].binding = 2;
    attributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkBuffer buffers[2] = {buffer.handle(), buffer.handle()};
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 5, 2, buffers, offsets);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 3, bindings, 3, attributes);
    // set later, shouldn't matter
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 2, 2, buffers, offsets);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 1);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveVertexInput, VertexStrideDynamicStride) {
    TEST_DESCRIPTION("set the Stride to fix bad stride in vkCmdBindVertexBuffers2");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription bindings = {0, 3, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes = {0, 0, VK_FORMAT_R16_UNORM, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &bindings;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &attributes;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkDeviceSize offset = 0;
    VkDeviceSize good_stride = 4;
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offset, nullptr, &good_stride);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveVertexInput, VertexStrideDoubleDynamicStride) {
    TEST_DESCRIPTION("set the Stride to invalid, then valid");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.divisor = 1;
    binding.stride = 4;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R16_UNORM;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    const VkDeviceSize offset = 0;
    const VkDeviceSize bad_stride = 3;
    const VkDeviceSize good_stride = 4;

    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offset, nullptr, &bad_stride);
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);  // set to valid
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    // flip order around
    binding.stride = static_cast<uint32_t>(bad_stride);
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
    vk::CmdBindVertexBuffers2EXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), &offset, nullptr, &good_stride);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveVertexInput, InputBindingMaxVertexInputBindingStrideDynamic) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test when stride is greater than VkPhysicalDeviceLimits::maxVertexInputBindingStride.
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.stride = m_device->phy().limits_.maxVertexInputBindingStride + 1;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveVertexInput, BindVertexBufferNull) {
    TEST_DESCRIPTION("Have null vertex but use nullDescriptor feature");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription bindings = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes = {0, 0, VK_FORMAT_R8G8B8A8_UNORM, 0};
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &bindings;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &attributes;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkDeviceSize offsets[2] = {0, 0};
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkBuffer buffers[2] = {buffer.handle(), VK_NULL_HANDLE};
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 2, buffers, offsets);

    // only uses first binding
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
