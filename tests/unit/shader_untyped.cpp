/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"

class NegativeShaderUntyped : public VkLayerTest {};

TEST_F(NegativeShaderUntyped, FragmentStoresAndAtomicsFeatureBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const std::string spirv = R"(
        OpCapability Shader
        OpCapability UntypedPointersKHR
        OpExtension "SPV_KHR_untyped_pointers"
        OpMemoryModel Logical GLSL450
        OpEntryPoint Fragment %main "main"
        OpExecutionMode %main OriginUpperLeft
        OpDecorate %y DescriptorSet 0
        OpDecorate %y Binding 0
        OpDecorate %ssbo Block
        OpMemberDecorate %ssbo 0 Offset 0
        %void = OpTypeVoid
        %int = OpTypeInt 32 1
        %int_0 = OpConstant %int 0
        %int_1 = OpConstant %int 1 ; device scope
        %ssbo = OpTypeStruct %int
        %ptr = OpTypeUntypedPointerKHR StorageBuffer
        %y = OpUntypedVariableKHR %ptr StorageBuffer %ssbo
        %void_fn = OpTypeFunction %void
        %main = OpFunction %void None %void_fn
        %entry = OpLabel
        %access = OpUntypedAccessChainKHR %ptr %ssbo %y %int_0
        %old = OpAtomicSMin %int %access %int_1 %int_0 %int_1
        OpReturn
        OpFunctionEnd
    )";

    VkShaderObj fs(this, spirv.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);

    auto info_override = [&](CreatePipelineHelper &info) {
        info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        info.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    };

    CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, "VUID-RuntimeSpirv-NonWritable-06340");
}

TEST_F(NegativeShaderUntyped, ZeroInitializeWorkgroupMemory) {
    TEST_DESCRIPTION("Test initializing workgroup memory in compute shader");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const char *spv_source = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpCapability UntypedPointersKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %counter
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpName %counter "counter"
               OpDecorate %block Block
               OpMemberDecorate %block 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %ptr = OpTypeUntypedPointerKHR Workgroup
      %block = OpTypeStruct %uint
  %zero_uint = OpConstantNull %block
    %counter = OpUntypedVariableKHR %ptr Workgroup %block %zero_uint
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderUntyped, Atomics) {
    TEST_DESCRIPTION("Test atomics with shader objects.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderSharedInt64Atomics);  // to allow OpCapability Int64Atomics
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const std::string spirv = R"(
OpCapability Shader
OpCapability Int64
OpCapability Int64Atomics
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %ssbo_var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %ssbo Block
OpMemberDecorate %ssbo 0 Offset 0
OpDecorate %ssbo_var DescriptorSet 0
OpDecorate %ssbo_var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1 ; device
%int_0 = OpConstant %int 0 ; relaxed
%ulong = OpTypeInt 64 0
%ulong_1 = OpConstant %ulong 1
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%ssbo = OpTypeStruct %ulong
%ssbo_var = OpUntypedVariableKHR %ptr StorageBuffer %ssbo
%main = OpFunction %void None %void_fn
%entry = OpLabel
%access = OpUntypedAccessChainKHR %ptr %ssbo %ssbo_var %int_0
%old = OpAtomicIAdd %ulong %access %int_1 %int_0 %ulong_1
OpReturn
OpFunctionEnd
    )";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    std::vector<uint32_t> spv;
    const auto valid = ASMtoSPV(SPV_ENV_VULKAN_1_2, 0, spirv.c_str(), spv);
    EXPECT_TRUE(valid);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &descriptor_set.layout_.handle());
    VkShaderEXT shader;

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06278");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderUntyped, DescriptorNotAccessible) {
    TEST_DESCRIPTION(
        "Create a pipeline in which a descriptor used by a shader stage does not include that stage in its stageFlags.");
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT /*!*/, nullptr},
                                     });

    const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos
OpDecorate %ubuf DescriptorSet 0
OpDecorate %ubuf Binding 0
OpDecorate %buf Block
OpMemberDecorate %buf 0 Offset 0
OpMemberDecorate %buf 0 MatrixStride 16
OpMemberDecorate %buf 0 ColMajor
OpDecorate %pos BuiltIn Position
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%v4float_1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%mat4x4 = OpTypeMatrix %v4float 4
%buf = OpTypeStruct %mat4x4
%ptr = OpTypeUntypedPointerKHR Uniform
%ptr_output_v4float = OpTypePointer Output %v4float
%pos = OpVariable %ptr_output_v4float Output
%ubuf = OpUntypedVariableKHR %ptr Uniform %buf
%main = OpFunction %void None %void_fn
%entry = OpLabel
%access = OpUntypedAccessChainKHR %ptr %buf %ubuf %uint_0
%ld_mat = OpLoad %mat4x4 %access
%res = OpMatrixTimesVector %v4float %ld_mat %v4float_1
OpStore %pos %res
OpReturn
OpFunctionEnd
    )";

    VkShaderObj vs(this, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07988");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderUntyped, StructOf8Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    // storagePushConstant8 is not enabled
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const std::string spirv = R"(
OpCapability Shader
OpCapability Int8
OpCapability UntypedPointersKHR
OpCapability StoragePushConstant8
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos %pc
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %Foo 0 Offset 0
OpMemberDecorate %Foo 1 Offset 4
OpMemberDecorate %Foo 2 Offset 16
OpDecorate %pos BuiltIn Position
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%char = OpTypeInt 8 1
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%ptr_output_v4float = OpTypePointer Output %v4float
%pos = OpVariable %ptr_output_v4float Output
%ptr = OpTypeUntypedPointerKHR PushConstant
%Foo = OpTypeStruct %uint %char %v4float
%block = OpTypeStruct %Foo
%pc = OpUntypedVariableKHR %ptr PushConstant %block
%main = OpFunction %void None %void_fn
%entry = OpLabel
%pc_access = OpUntypedAccessChainKHR %ptr %Foo %pc %uint_1
%ld_pc = OpLoad %char %pc_access
OpReturn
OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // capability
    VkShaderObj vs(this, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderUntyped, StructOf16Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    // storagePushConstant16 is not enabled
    AddRequiredFeature(vkt::Feature::shaderInt16);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const std::string spirv = R"(
OpCapability Shader
OpCapability Int16
OpCapability UntypedPointersKHR
OpCapability StoragePushConstant16
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos %pc
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %Foo 0 Offset 0
OpMemberDecorate %Foo 1 Offset 4
OpMemberDecorate %Foo 2 Offset 16
OpDecorate %pos BuiltIn Position
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%short = OpTypeInt 16 1
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%ptr_output_v4float = OpTypePointer Output %v4float
%pos = OpVariable %ptr_output_v4float Output
%ptr = OpTypeUntypedPointerKHR PushConstant
%Foo = OpTypeStruct %uint %short %v4float
%block = OpTypeStruct %Foo
%pc = OpUntypedVariableKHR %ptr PushConstant %block
%main = OpFunction %void None %void_fn
%entry = OpLabel
%pc_access = OpUntypedAccessChainKHR %ptr %Foo %pc %uint_1
%ld_pc = OpLoad %short %pc_access
OpReturn
OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant16-06333");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");      // capability
    VkShaderObj vs(this, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderUntyped, OffsetMaxComputeSharedMemorySize) {
    TEST_DESCRIPTION("Have an offset that is over maxComputeSharedMemorySize");
    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;

    // layout(constant_id = 0) const uint value = 4;
    // shared X {
    //     vec4 x1[value];
    //     layout(offset = OVER_LIMIT) vec4 x2;
    // };
    std::stringstream csSource;
    csSource << R"asm(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpCapability UntypedPointersKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpExtension "SPV_KHR_untyped_pointers"
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
%_ptr_Workgroup = OpTypeUntypedPointerKHR Workgroup
          %_ = OpUntypedVariableKHR %_ptr_Workgroup Workgroup %X
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}
