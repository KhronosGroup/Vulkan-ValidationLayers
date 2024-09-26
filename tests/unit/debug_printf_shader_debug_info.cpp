/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeDebugPrintfShaderDebugInfo : public DebugPrintfTests {};

// These tests print out the verbose info to make sure that info is correct
static const VkBool32 verbose_value = true;
static const VkLayerSettingEXT layer_setting = {OBJECT_LAYER_NAME, "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                &verbose_value};
static VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &layer_setting};

TEST_F(NegativeDebugPrintfShaderDebugInfo, PipelineHandle) {
    TEST_DESCRIPTION("Make sure we are printing out which pipeline the error is from");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    const char *object_name = "bad_pipeline";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.objectHandle = (uint64_t)pipe.Handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Pipeline (bad_pipeline)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, ShaderObjectHandle) {
    TEST_DESCRIPTION("Make sure we are printing out which shader object the error is from");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    VkShaderStageFlagBits shader_stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    const vkt::Shader comp_shader(*m_device, shader_stages[0], GLSLToSPV(shader_stages[0], shader_source));

    const char *object_name = "bad_shader_object";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_EXT;
    name_info.objectHandle = (uint64_t)comp_shader.handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.begin();
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, shader_stages, &comp_shader.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Shader Object (bad_shader_object)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, OpLine) {
    TEST_DESCRIPTION("Make sure OpLine works");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "GLSL.std.450"
         %13 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
         %11 = OpString "float == %f"
               OpSource GLSL 450 %1 "// OpModuleProcessed client vulkan100
// OpModuleProcessed target-env vulkan1.0
// OpModuleProcessed entry-point main
#line 1
#version 450
#extension GL_EXT_debug_printf : enable
void main() {
    float myfloat = 3.1415f;
    debugPrintfEXT(\"float == %f\", myfloat);
}"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_3_1415 = OpConstant %float 3.1415
               OpLine %1 3 11
       %main = OpFunction %void None %4
          %6 = OpLabel
    %myfloat = OpVariable %_ptr_Function_float Function
               OpLine %1 4 0
               OpStore %myfloat %float_3_1415
               OpLine %1 5 0
         %12 = OpLoad %float %myfloat
         %14 = OpExtInst %void %13 1 %11 %12
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit,
        "Debug shader printf message generated in file a.comp at line 5\n\n5:     debugPrintfEXT(\"float == %f\", myfloat);");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, CommandBufferCommandIndex) {
    TEST_DESCRIPTION("Make sure we print which index in the command buffer the issue occured");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper empty_pipe(*this);
    empty_pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, empty_pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Compute Dispatch Index 3");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, CommandBufferCommandIndexMulti) {
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(push_constant) uniform PushConstants { int x; } pc;
        void main() {
            if (pc.x > 0) {
                debugPrintfEXT("int == %u", pc.x);
            }
        }
    )glsl";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.pushConstantRangeCount = 1;
    pipe_layout_ci.pPushConstantRanges = &pc_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    uint32_t skip = 0;
    uint32_t good = 4;
    cb0.begin();
    vk::CmdBindPipeline(cb0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &skip);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &good);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &skip);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    cb0.end();

    cb1.begin();
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdPushConstants(cb1.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &good);
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    vk::CmdPushConstants(cb1.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &skip);
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    cb1.end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Compute Dispatch Index 3");  // cb0
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Compute Dispatch Index 0");  // cb1

    VkCommandBuffer cbs[2] = {cb0.handle(), cb1.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = 2;
    submit.pCommandBuffers = cbs;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, StageInfo) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            if (gl_GlobalInvocationID.x == 3 && gl_GlobalInvocationID.y == 1) {
                debugPrintfEXT("float == %f", myfloat);
            }
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper empty_pipe(*this);
    empty_pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 4, 4, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Global invocation ID (x, y, z) = (3, 1, 0)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, Fragment) {
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;

        void main() {
            if (gl_FragCoord.x > 10 && gl_FragCoord.x < 11) {
                if (gl_FragCoord.y > 10 && gl_FragCoord.y < 12) {
                    debugPrintfEXT("gl_FragCoord.xy %1.2f, %1.2f\n", gl_FragCoord.x, gl_FragCoord.y);
                }
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Stage = Fragment.  Fragment coord (x,y) = (10.5, 10.5)");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Stage = Fragment.  Fragment coord (x,y) = (10.5, 11.5)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
