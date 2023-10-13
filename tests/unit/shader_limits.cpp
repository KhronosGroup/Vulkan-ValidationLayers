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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

TEST_F(NegativeShaderLimits, MaxSampleMaskWords) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords.");

    RETURN_IF_SKIP(InitFramework())
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

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

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

TEST_F(NegativeShaderLimits, MinAndMaxTexelGatherOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelGatherOffset and greather than maxTexelGatherOffset");

    RETURN_IF_SKIP(Init())

    if (m_device->phy().limits_.minTexelGatherOffset <= -100 || m_device->phy().limits_.maxTexelGatherOffset >= 100) {
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
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
     %uint_0 = OpConstant %uint 0
      %int_0 = OpConstant %int 0
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2uint %uint_0 %uint_n100

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

    auto cs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ = std::move(cs);
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipeline.LateBindPipelineInfo();
    // as commented in SPIR-V should trigger the limits as following
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06376");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    cs_pipeline.CreateComputePipeline(false);  // need false to prevent late binding

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MinAndMaxTexelOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelOffset and greather than maxTexelOffset");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    if (m_device->phy().limits_.minTexelOffset <= -100 || m_device->phy().limits_.maxTexelOffset >= 100) {
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
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
     %uint_0 = OpConstant %uint 0
      %int_0 = OpConstant %int 0
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2uint %uint_0 %uint_n100
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
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
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

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5208
// Asserting in MoltenVK
TEST_F(NegativeShaderLimits, DISABLED_MaxFragmentDualSrcAttachments) {
    TEST_DESCRIPTION("Test drawing with dual source blending with too many fragment output attachments.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    uint32_t count = m_device->phy().limits_.maxFragmentDualSrcAttachments + 1;
    InitRenderTarget(count);

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
