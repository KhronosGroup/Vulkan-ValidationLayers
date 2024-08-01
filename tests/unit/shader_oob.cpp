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

class NegativeShaderOOB : public VkLayerTest {};

TEST_F(NegativeShaderOOB, ConstantBufferArray) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //     int i_data[4];
    //     int o_data;
    // };
    // void main() {
    //    o_data = i_data[4];
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 64
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
     %Block1 = OpTypeStruct %_arr_int_uint_4 %int
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0 %int_4
         %18 = OpLoad %int %17
         %19 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %19 %18
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderOOB, ConstantBufferArrayConditional) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //   int i_data[4];
    //   int o_data;
    // };
    // void main() {
    //   if (i_data[0] > 99) {
    //     o_data = i_data[3];
    //   }
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 64
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
     %Block1 = OpTypeStruct %_arr_int_uint_4 %int
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
     %int_99 = OpConstant %int 99
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0 %int_0
         %16 = OpLoad %int %15
         %19 = OpSGreaterThan %bool %16 %int_99
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %21
         %20 = OpLabel
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0 %int_4
         %25 = OpLoad %int %24
         %26 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %26 %25
               OpBranch %21
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderOOB, ConstantBufferArrayInStruct) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // struct Foo {
    //   int a[4];
    //   int b;
    // };
    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //   int o_data;
    //   Foo i_data[3];
    // };
    // void main() {
    //     o_data = i_data[1].a[4];
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpMemberDecorate %Foo 0 Offset 0
               OpMemberDecorate %Foo 1 Offset 64
               OpDecorate %_arr_Foo_uint_3 ArrayStride 80
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 16
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
        %Foo = OpTypeStruct %_arr_int_uint_4 %int
     %uint_3 = OpConstant %uint 3
%_arr_Foo_uint_3 = OpTypeArray %Foo %uint_3
     %Block1 = OpTypeStruct %int %_arr_Foo_uint_3
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1 %int_1 %int_0 %int_4
         %21 = OpLoad %int %20
         %22 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0
               OpStore %22 %21
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderOOB, ConstantBufferArrayInStruct2) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // struct Foo {
    //   int a[4];
    //   int b;
    // };
    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //   int o_data;
    //   Foo i_data[3];
    // };
    // void main() {
    //     o_data = i_data[1].a[4];
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpMemberDecorate %Foo 0 Offset 0
               OpMemberDecorate %Foo 1 Offset 64
               OpDecorate %_arr_Foo_uint_3 ArrayStride 80
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 16
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
        %Foo = OpTypeStruct %_arr_int_uint_4 %int
     %uint_3 = OpConstant %uint 3
%_arr_Foo_uint_3 = OpTypeArray %Foo %uint_3
     %Block1 = OpTypeStruct %int %_arr_Foo_uint_3
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
%ptr_array = OpTypePointer StorageBuffer %_arr_int_uint_4
%ptr_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
 %array_base = OpAccessChain %ptr_array %_ %int_1 %int_1 %int_0
         %20 = OpAccessChain %ptr_int %struct_base %int_4
         %21 = OpLoad %int %20
         %22 = OpAccessChain %ptr_int %_ %int_0
               OpStore %22 %21
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderOOB, ConstantBuffer2DArray) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //     int o_data;
    //     int i_data[4][4];
    // };
    // void main() {
    // o_data = i_data[1][4] + i_data[4][1];
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpDecorate %_arr__arr_int_uint_4_uint_4 ArrayStride 64
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 16
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%_arr__arr_int_uint_4_uint_4 = OpTypeArray %_arr_int_uint_4 %uint_4
     %Block1 = OpTypeStruct %int %_arr__arr_int_uint_4_uint_4
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1 %int_1 %int_4
         %19 = OpLoad %int %18
         %20 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1 %int_4 %int_1
         %21 = OpLoad %int %20
         %22 = OpIAdd %int %19 %21
         %23 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0
               OpStore %23 %22
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderOOB, ConstantBuffer3DArray) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //     int i_data;
    //     int o_data[4][4][4];
    // };
    // void main() {
    //    o_data[1][4][1] = i_data;
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpDecorate %_arr__arr_int_uint_4_uint_4 ArrayStride 64
               OpDecorate %_arr__arr__arr_int_uint_4_uint_4_uint_4 ArrayStride 256
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 16
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%_arr__arr_int_uint_4_uint_4 = OpTypeArray %_arr_int_uint_4 %uint_4
%_arr__arr__arr_int_uint_4_uint_4_uint_4 = OpTypeArray %_arr__arr_int_uint_4_uint_4 %uint_4
     %Block1 = OpTypeStruct %int %_arr__arr__arr_int_uint_4_uint_4_uint_4
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0
         %19 = OpLoad %int %18
         %20 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1 %int_1 %int_4 %int_1
               OpStore %20 %19
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderOOB, ConstantBufferFunction) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout(set = 0, binding = 0, std140) buffer Block1 {
    //     int i_data[4];
    //     int o_data[4];
    // };
    // void foo() {
    //    o_data[4] = i_data[4];
    // }
    // void main() {
    //     foo();
    // }
    char const *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_int_uint_4 ArrayStride 16
               OpDecorate %_arr_int_uint_4_0 ArrayStride 16
               OpMemberDecorate %Block1 0 Offset 0
               OpMemberDecorate %Block1 1 Offset 64
               OpDecorate %Block1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%_arr_int_uint_4_0 = OpTypeArray %int %uint_4
     %Block1 = OpTypeStruct %_arr_int_uint_4 %_arr_int_uint_4_0
%_ptr_StorageBuffer_Block1 = OpTypePointer StorageBuffer %Block1
          %_ = OpVariable %_ptr_StorageBuffer_Block1 StorageBuffer
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %22 = OpFunctionCall %void %foo_
               OpReturn
               OpFunctionEnd
       %foo_ = OpFunction %void None %3
          %7 = OpLabel
         %19 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0 %int_4
         %20 = OpLoad %int %19
         %21 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1 %int_4
               OpStore %21 %20
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}
