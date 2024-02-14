/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
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

    RETURN_IF_SKIP(InitFramework());
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

    RETURN_IF_SKIP(InitState());
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
    CreatePipelineHelper::OneshotTest(*this, validPipeline, kErrorBit);

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

    RETURN_IF_SKIP(Init());

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06376");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    auto cs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MinAndMaxTexelOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelOffset and greather than maxTexelOffset");

    RETURN_IF_SKIP(Init());
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

    // OpImageSampleImplicitLod
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06435");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    // // OpImageFetch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06435");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageSample-06436");
    VkShaderObj const fs(this, spv_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5208
// Asserting in MoltenVK
TEST_F(NegativeShaderLimits, DISABLED_MaxFragmentDualSrcAttachments) {
    TEST_DESCRIPTION("Test drawing with dual source blending with too many fragment output attachments.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
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

TEST_F(NegativeShaderLimits, OffsetMaxComputeSharedMemorySize) {
    TEST_DESCRIPTION("Have an offset that is over maxComputeSharedMemorySize");

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);
    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->phy().limits_.maxComputeSharedMemorySize;

    // layout(constant_id = 0) const uint value = 4;
    // shared X {
    //     vec4 x1[value];
    //     layout(offset = OVER_LIMIT) vec4 x2;
    // };
    std::stringstream csSource;
    csSource << R"asm(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %value SpecId 0
               OpDecorate %_arr_v4float_value ArrayStride 16
               OpMemberDecorate %X 0 Offset 0
               OpMemberDecorate %X 1 Offset )asm";
    // will be over the max if the spec constant uses default value
    csSource << (max_shared_memory_size + 16);
    csSource << R"asm(
               OpDecorate %X Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %value = OpSpecConstant %uint 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_arr_v4float_value = OpTypeArray %v4float %value
          %X = OpTypeStruct %_arr_v4float_value %v4float
%_ptr_Workgroup_X = OpTypePointer Workgroup %X
          %_ = OpVariable %_ptr_Workgroup_X Workgroup
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2,
                                             SPV_SOURCE_ASM);
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}
