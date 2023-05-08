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

class PositiveShaderPushConstants : public VkPositiveLayerTest {};

TEST_F(PositiveShaderPushConstants, OverlappingPushConstantRange) {
    TEST_DESCRIPTION("Test overlapping push-constant ranges.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ci_ = pipeline_layout_info;
    pipe.InitState();

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderPushConstants, MultipleEntryPointVert) {
    TEST_DESCRIPTION("Test push-constant only being used by single entrypoint.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x[16]; } constants;
        void main(){
           gl_Position = vec4(constants.x[4]);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    // range A and B are the same while range C is different
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
    pipeline_helper_a.InitInfo();
    pipeline_helper_a.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_helper_a.pipeline_layout_ci_ = pipeline_layout_info_a;
    pipeline_helper_a.InitState();
    pipeline_helper_a.CreateGraphicsPipeline();
    pipeline_helper_b.InitInfo();
    pipeline_helper_b.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_helper_b.pipeline_layout_ci_ = pipeline_layout_info_b;
    pipeline_helper_b.InitState();
    pipeline_helper_b.CreateGraphicsPipeline();
    pipeline_helper_c.InitInfo();
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
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // case 1 - bind different layout with the same range
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_b);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 2 - bind layout with same range then push different range
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_b, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_b);
    m_commandBuffer->Draw(1, 0, 0, 0);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 3 - same range same layout then same range from a different layout and same range from the same layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_b, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 4 - same range same layout then diff range and same range update
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_c, VK_SHADER_STAGE_VERTEX_BIT, 16, pc_size, data);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 5 - update push constant bind different layout with the same range then bind correct layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_b);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 6 - update push constant then bind different layout with overlapping range then bind correct layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_c);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // case 7 - bind different layout with different range then update push constant and bind correct layout
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_c);
    vk::CmdPushConstants(m_commandBuffer->handle(), layout_a, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_size, data);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_a);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderPushConstants, StaticallyUnused) {
    TEST_DESCRIPTION("Test cases where creating pipeline with no use of push constants but still has ranges in layout");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Just in layout
    CreatePipelineHelper pipeline_unused(*this);
    pipeline_unused.InitInfo();
    pipeline_unused.shader_stages_ = {vsUnused.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_unused.pipeline_layout_ci_ = pipeline_layout_info;
    pipeline_unused.InitState();
    pipeline_unused.CreateGraphicsPipeline();

    // Shader never had a reference
    CreatePipelineHelper pipeline_empty(*this);
    pipeline_empty.InitInfo();
    pipeline_empty.shader_stages_ = {vsEmpty.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline_empty.pipeline_layout_ci_ = pipeline_layout_info;
    pipeline_empty.InitState();
    pipeline_empty.CreateGraphicsPipeline();

    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // Draw without ever pushing to the unused and empty pipelines
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_unused.pipeline_);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_empty.pipeline_);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
