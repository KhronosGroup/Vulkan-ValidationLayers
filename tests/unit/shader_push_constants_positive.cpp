/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(PositiveShaderPushConstants, OverlappingPushConstantRange) {
    TEST_DESCRIPTION("Test overlapping push-constant ranges.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x[8]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0]);
        }
    )glsl";

    char const *const fsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x[4]; } constants;
        layout(location=0) out vec4 o;
        void main(){
           o = vec4(constants.x[0]);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange push_constant_ranges[2]{{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 8},
                                                {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 4}};

    VkPipelineLayoutCreateInfo const pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 2, push_constant_ranges};

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ci_ = pipeline_layout_info;
    pipe.InitState();

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderPushConstants, MultipleEntryPointVert) {
    TEST_DESCRIPTION("Test push-constant only being used by single entrypoint.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // #version 450
    // layout(push_constant, std430) uniform foo { float x; } consts;
    // void main(){
    //    gl_Position = vec4(consts.x);
    // }
    //
    // #version 450
    // layout(location=0) out vec4 o;
    // void main(){
    //    o = vec4(1.0);
    // }
    const std::string source_body = R"(
                            OpExecutionMode %main_f OriginUpperLeft
                            OpSource GLSL 450
                            OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
                            OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
                            OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
                            OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
                            OpDecorate %gl_PerVertex Block
                            OpMemberDecorate %foo 0 Offset 0
                            OpDecorate %foo Block
                            OpDecorate %out_frag Location 0
                    %void = OpTypeVoid
                       %3 = OpTypeFunction %void
                   %float = OpTypeFloat 32
                 %v4float = OpTypeVector %float 4
                    %uint = OpTypeInt 32 0
                  %uint_1 = OpConstant %uint 1
       %_arr_float_uint_1 = OpTypeArray %float %uint_1
            %gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
                %out_vert = OpVariable %_ptr_Output_gl_PerVertex Output
                     %int = OpTypeInt 32 1
                   %int_0 = OpConstant %int 0
                     %foo = OpTypeStruct %float
   %_ptr_PushConstant_foo = OpTypePointer PushConstant %foo
                  %consts = OpVariable %_ptr_PushConstant_foo PushConstant
 %_ptr_PushConstant_float = OpTypePointer PushConstant %float
     %_ptr_Output_v4float = OpTypePointer Output %v4float
                %out_frag = OpVariable %_ptr_Output_v4float Output
                 %float_1 = OpConstant %float 1
                 %vec_1_0 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
                  %main_v = OpFunction %void None %3
                 %label_v = OpLabel
                      %20 = OpAccessChain %_ptr_PushConstant_float %consts %int_0
                      %21 = OpLoad %float %20
                      %22 = OpCompositeConstruct %v4float %21 %21 %21 %21
                      %24 = OpAccessChain %_ptr_Output_v4float %out_vert %int_0
                            OpStore %24 %22
                            OpReturn
                            OpFunctionEnd
                  %main_f = OpFunction %void None %3
                 %label_f = OpLabel
                            OpStore %out_frag %vec_1_0
                            OpReturn
                            OpFunctionEnd
    )";

    std::string vert_first = R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpEntryPoint Vertex %main_v "main_v" %out_vert
        OpEntryPoint Fragment %main_f "main_f" %out_frag
    )" + source_body;

    std::string frag_first = R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpEntryPoint Fragment %main_f "main_f" %out_frag
        OpEntryPoint Vertex %main_v "main_v" %out_vert
    )" + source_body;

    VkPushConstantRange push_constant_ranges[1]{{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float)}};
    VkPipelineLayoutCreateInfo const pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, push_constant_ranges};

    // Vertex entry point first
    {
        VkShaderObj const vs(this, vert_first.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_v");
        VkShaderObj const fs(this, vert_first.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_f");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.pipeline_layout_ci_ = pipeline_layout_info;
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    // Fragment entry point first
    {
        VkShaderObj const vs(this, frag_first.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_v");
        VkShaderObj const fs(this, frag_first.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_f");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.pipeline_layout_ci_ = pipeline_layout_info;
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderPushConstants, MultipleEntryPointFrag) {
    TEST_DESCRIPTION("Test push-constant only being used by single entrypoint.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // #version 450
    // void main(){
    //    gl_Position = vec4(1.0);
    // }
    //
    // #version 450
    // layout(push_constant, std430) uniform foo { float x; } consts;
    // layout(location=0) out vec4 o;
    // void main(){
    //    o = vec4(consts.x);
    // }
    const std::string source_body = R"(
                            OpExecutionMode %main_f OriginUpperLeft
                            OpSource GLSL 450
                            OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
                            OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
                            OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
                            OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
                            OpDecorate %gl_PerVertex Block
                            OpDecorate %out_frag Location 0
                            OpMemberDecorate %foo 0 Offset 0
                            OpDecorate %foo Block
                    %void = OpTypeVoid
                       %3 = OpTypeFunction %void
                   %float = OpTypeFloat 32
                 %v4float = OpTypeVector %float 4
                    %uint = OpTypeInt 32 0
                  %uint_1 = OpConstant %uint 1
       %_arr_float_uint_1 = OpTypeArray %float %uint_1
            %gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
                %out_vert = OpVariable %_ptr_Output_gl_PerVertex Output
                     %int = OpTypeInt 32 1
                   %int_0 = OpConstant %int 0
                 %float_1 = OpConstant %float 1
                      %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %_ptr_Output_v4float = OpTypePointer Output %v4float
                %out_frag = OpVariable %_ptr_Output_v4float Output
                     %foo = OpTypeStruct %float
   %_ptr_PushConstant_foo = OpTypePointer PushConstant %foo
                  %consts = OpVariable %_ptr_PushConstant_foo PushConstant
 %_ptr_PushConstant_float = OpTypePointer PushConstant %float
                  %main_v = OpFunction %void None %3
                 %label_v = OpLabel
                      %19 = OpAccessChain %_ptr_Output_v4float %out_vert %int_0
                            OpStore %19 %17
                            OpReturn
                            OpFunctionEnd
                  %main_f = OpFunction %void None %3
                 %label_f = OpLabel
                      %26 = OpAccessChain %_ptr_PushConstant_float %consts %int_0
                      %27 = OpLoad %float %26
                      %28 = OpCompositeConstruct %v4float %27 %27 %27 %27
                            OpStore %out_frag %28
                            OpReturn
                            OpFunctionEnd
    )";

    std::string vert_first = R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpEntryPoint Vertex %main_v "main_v" %out_vert
        OpEntryPoint Fragment %main_f "main_f" %out_frag
    )" + source_body;

    std::string frag_first = R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpEntryPoint Fragment %main_f "main_f" %out_frag
        OpEntryPoint Vertex %main_v "main_v" %out_vert
    )" + source_body;

    VkPushConstantRange push_constant_ranges[1]{{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float)}};
    VkPipelineLayoutCreateInfo const pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, push_constant_ranges};

    // Vertex entry point first
    {
        VkShaderObj const vs(this, vert_first.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_v");
        VkShaderObj const fs(this, vert_first.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_f");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.pipeline_layout_ci_ = pipeline_layout_info;
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    // Fragment entry point first
    {
        VkShaderObj const vs(this, frag_first.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_v");
        VkShaderObj const fs(this, frag_first.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr,
                             "main_f");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.pipeline_layout_ci_ = pipeline_layout_info;
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderPushConstants, CompatibilityGraphicsOnly) {
    TEST_DESCRIPTION("Based on verified valid examples from internal Vulkan Spec issue #2168");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { layout(offset = 16) vec4 x; } constants;
        void main(){
           gl_Position = constants.x;
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // range A and B are the same while range C is different
    // All 3 ranges fit the range from the shader
    const uint32_t pc_size = 32;
    VkPushConstantRange range_a = {VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size};
    VkPushConstantRange range_b = {VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size};
    VkPushConstantRange range_c = {VK_SHADER_STAGE_VERTEX_BIT, 16, pc_size};

    VkPipelineLayoutCreateInfo pipeline_layout_info_a = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &range_a};
    VkPipelineLayoutCreateInfo pipeline_layout_info_b = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &range_b};
    VkPipelineLayoutCreateInfo pipeline_layout_info_c = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &range_c};

    CreatePipelineHelper pipeline_helper_a(*this);  // layout_a and range_a
    CreatePipelineHelper pipeline_helper_b(*this);  // layout_b and range_b
    CreatePipelineHelper pipeline_helper_c(*this);  // layout_c and range_c
    pipeline_helper_a.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_helper_a.pipeline_layout_ci_ = pipeline_layout_info_a;
    pipeline_helper_a.InitState();
    pipeline_helper_a.CreateGraphicsPipeline();
    pipeline_helper_b.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_helper_b.pipeline_layout_ci_ = pipeline_layout_info_b;
    pipeline_helper_b.InitState();
    pipeline_helper_b.CreateGraphicsPipeline();
    pipeline_helper_c.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_helper_c.pipeline_layout_ci_ = pipeline_layout_info_c;
    pipeline_helper_c.InitState();
    pipeline_helper_c.CreateGraphicsPipeline();

    // Easier to see in command buffers
    const VkPipelineLayout layout_a = pipeline_helper_a.pipeline_layout_.handle();
    const VkPipelineLayout layout_b = pipeline_helper_b.pipeline_layout_.handle();
    const VkPipelineLayout layout_c = pipeline_helper_c.pipeline_layout_.handle();
    const VkPipeline pipeline_a = pipeline_helper_a.pipeline_;
    const VkPipeline pipeline_b = pipeline_helper_b.pipeline_;
    const VkPipeline pipeline_c = pipeline_helper_c.pipeline_;

    const float data[16] = {};  // dummy data to match shader size

    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // case 1 - bind different layout with the same range
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_b);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 2 - bind layout with same range then push different range
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_b, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_b);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 3 - same range same layout then same range from a different layout and same range from the same layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_b, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 4 - same range same layout then diff range and same range update
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_c, VK_SHADER_STAGE_VERTEX_BIT, 16, pc_size, data);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 5 - update push constant bind different layout with the same range then bind correct layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_b);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 6 - update push constant then bind different layout with overlapping range then bind correct layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_c);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 7 - bind different layout with different range then update push constant and bind correct layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_c);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderPushConstants, StaticallyUnused) {
    TEST_DESCRIPTION("Test cases where creating pipeline with no use of push constants but still has ranges in layout");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // Create set of Pipeline Layouts that cover variations of ranges
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};

    char const *vsSourceUnused = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(1.0);
        }
    )glsl";

    char const *vsSourceEmpty = R"glsl(
        #version 450
        void main(){
           gl_Position = vec4(1.0);
        }
    )glsl";

    VkShaderObj vsUnused(this, vsSourceUnused, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj vsEmpty(this, vsSourceEmpty, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Just in layout
    CreatePipelineHelper pipeline_unused(*this);
    pipeline_unused.shader_stages_ = {vsUnused.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_unused.pipeline_layout_ci_ = pipeline_layout_info;
    pipeline_unused.InitState();
    pipeline_unused.CreateGraphicsPipeline();

    // Shader never had a reference
    CreatePipelineHelper pipeline_empty(*this);
    pipeline_empty.shader_stages_ = {vsEmpty.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_empty.pipeline_layout_ci_ = pipeline_layout_info;
    pipeline_empty.InitState();
    pipeline_empty.CreateGraphicsPipeline();

    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // Draw without ever pushing to the unused and empty pipelines
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_unused.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_empty.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderPushConstants, OffsetVector) {
    TEST_DESCRIPTION("Vector uses offset in the shader.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450

        layout(push_constant) uniform Material {
            layout(offset = 16) vec4 color;
        }constants;

        void main() {
            gl_Position = constants.color;
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 16, 32};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    const float data[16] = {};  // dummy data to match shader size

    m_commandBuffer->begin();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 16, 16, data);
    m_commandBuffer->end();
}

TEST_F(PositiveShaderPushConstants, PhysicalStorageBufferBasic) {
    TEST_DESCRIPTION("Basic use of Physical Storage Buffers in a Push Constant block.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features12);
    if (VK_TRUE != features12.bufferDeviceAddress) {
        GTEST_SKIP() << "bufferDeviceAddress not supported and is required";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features12));
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450

        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_scalar_block_layout : enable

        layout(buffer_reference, buffer_reference_align=16, scalar) readonly buffer VectorBuffer {
            float v;
        };

        layout(push_constant, scalar) uniform pc {
            layout(offset = 16) VectorBuffer vb; // always 8 bytes
            float extra; // offset == 24
        } pcs;

        void main() {
            gl_Position = vec4(pcs.vb.v);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Use exact range
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 16, 28};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    const float data[12] = {};  // dummy data to match shader size

    m_commandBuffer->begin();
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 16, 12, data);
    m_commandBuffer->end();
}

TEST_F(PositiveShaderPushConstants, PhysicalStorageBufferVertFrag) {
    TEST_DESCRIPTION("Reproduces Github issue #2467 and effectively #2465 as well.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features12);
    if (VK_TRUE != features12.bufferDeviceAddress) {
        GTEST_SKIP() << "VkPhysicalDeviceVulkan12Features::bufferDeviceAddress not supported and is required";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features12));
    InitRenderTarget();

    const char *vertex_source = R"glsl(
        #version 450

        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_scalar_block_layout : enable

        layout(buffer_reference, buffer_reference_align=16, scalar) readonly buffer VectorBuffer {
            vec3 x;
            vec3 y;
            vec3 z;
        };

        layout(push_constant, scalar) uniform pc {
            VectorBuffer vb; // only 8 bytes, just a pointer
        } pcs;

        void main() {
            gl_Position = vec4(pcs.vb.x, 1.0);
        }
        )glsl";
    const VkShaderObj vs(this, vertex_source, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
        #version 450

        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_scalar_block_layout : enable

        layout(buffer_reference, buffer_reference_align=16, scalar) readonly buffer VectorBuffer {
            vec3 v;
        };

        layout(push_constant, scalar) uniform pushConstants {
            layout(offset=8) VectorBuffer vb;
        } pcs;

        layout(location=0) out vec4 o;
        void main() {
            o = vec4(pcs.vb.v, 1.0);
        }
    )glsl";
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::array<VkPushConstantRange, 2> push_ranges;
    push_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_ranges[0].size = sizeof(uint64_t);
    push_ranges[0].offset = 0;
    push_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_ranges[1].size = sizeof(uint64_t);
    push_ranges[1].offset = sizeof(uint64_t);

    VkPipelineLayoutCreateInfo const pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr,           0, 0, nullptr,
        static_cast<uint32_t>(push_ranges.size()),     push_ranges.data()};

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ci_ = pipeline_layout_info;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderPushConstants, MultipleStructs) {
    TEST_DESCRIPTION("Test having multiple structs Push Constant structs, but only one is used.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // Note - it is invalid SPIR-V for an entrypoint to have 2 Push Constant variables used. This is only valid because it is being
    // ignored
    //
    // What this looks like:
    //
    // layout(push_constant) uniform pc_a { layout(offset = 32) vec4 x; } a;
    // layout(push_constant) uniform pc_b { layout(offset = 16) vec4 x; } b;
    const char *source = R"(
                 OpCapability Shader
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Vertex %1 "main"
                 ; used in range [32 - 48]
                 OpDecorate %struct_pc Block
                 OpMemberDecorate %struct_pc 0 Offset 32
                 ; unused in range [16 - 32]
                 OpDecorate %struct_unused Block
                 OpMemberDecorate %struct_unused 0 Offset 16
         %void = OpTypeVoid
            %7 = OpTypeFunction %void
         %uint = OpTypeInt 32 0
        %float = OpTypeFloat 32
      %v4float = OpTypeVector %float 4
       %uint_0 = OpConstant %uint 0
       %uint_2 = OpConstant %uint 2
 %ptr_pc_float = OpTypePointer PushConstant %float
    %struct_pc = OpTypeStruct %v4float
%ptr_pc_struct = OpTypePointer PushConstant %struct_pc
          %var = OpVariable %ptr_pc_struct PushConstant
                ; Unused
                ; Vulkan 1.0 you do not need declare this in the OpEntryPoint
%struct_unused = OpTypeStruct %v4float
   %ptr_unused = OpTypePointer PushConstant %struct_unused
   %var_unused = OpVariable %ptr_unused PushConstant
            %1 = OpFunction %void None %7
           %16 = OpLabel
           %17 = OpAccessChain %ptr_pc_float %var %uint_0 %uint_2
                 OpReturn
                 OpFunctionEnd
    )";

    VkShaderObj const vs(this, source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 32, 16};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    const float data[16] = {};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 32, 16, data);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderPushConstants, SpecConstantSizeDefault) {
    TEST_DESCRIPTION("Use SpecConstant to adjust size of Push Constant Block, but use default value");
    RETURN_IF_SKIP(Init())

    const char *cs_source = R"glsl(
        #version 460
        layout (constant_id = 2) const int my_array_size = 1;
        layout (push_constant) uniform my_buf {
            float my_array[my_array_size];
        } pc;

        void main() {
            float a = pc.my_array[0];
        }
    )glsl";

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, 32};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderPushConstants, SpecConstantSizeSet) {
    TEST_DESCRIPTION("Use SpecConstant to adjust size of Push Constant Block");
    RETURN_IF_SKIP(Init())

    const char *cs_source = R"glsl(
        #version 460
        layout (constant_id = 0) const int my_array_size = 256;
        layout (push_constant) uniform my_buf {
            float my_array[my_array_size];
        } pc;

        void main() {
            float a = pc.my_array[0];
        }
    )glsl";

    // Setting makes the VkPushConstantRange valid
    uint32_t data = 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, 16};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateComputePipeline();
}
