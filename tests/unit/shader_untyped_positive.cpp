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

#include "../framework/layer_validation_tests.h"
#include "../framework/shader_helper.h"

class PositiveShaderUntyped : public VkLayerTest {};

TEST_F(PositiveShaderUntyped, Basic) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%block = OpTypeStruct %int
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %block
%main = OpFunction %void None %void_fn
%entry = OpLabel
%access = OpUntypedAccessChainKHR %ptr %block %var %int_0
%ld = OpLoad %int %access
OpReturn
OpFunctionEnd
)";

    VkShaderObj::CreateFromASM(this, spirv.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->Finish();
}

TEST_F(PositiveShaderUntyped, Atomics) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%block = OpTypeStruct %int
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %block
%main = OpFunction %void None %void_fn
%entry = OpLabel
%access = OpUntypedAccessChainKHR %ptr %block %var %int_0
OpAtomicStore %access %int_1 %int_0 %int_1
OpReturn
OpFunctionEnd
)";

    VkShaderObj::CreateFromASM(this, spirv.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->Finish();
}

TEST_F(PositiveShaderUntyped, StructOf8Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storagePushConstant8);
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
    VkShaderObj vs(*m_device, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->Finish();
}

TEST_F(PositiveShaderUntyped, StructOf8BitUnaccessed) {
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
%uint_0 = OpConstant %uint 0
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
%pc_access = OpUntypedAccessChainKHR %ptr %Foo %pc %uint_0
%ld_pc = OpLoad %uint %pc_access
OpReturn
OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // capability
    VkShaderObj vs(*m_device, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->Finish();
}

TEST_F(PositiveShaderUntyped, StructOf16Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt16);
    AddRequiredFeature(vkt::Feature::storagePushConstant16);
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
    VkShaderObj vs(*m_device, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->Finish();
}

TEST_F(PositiveShaderUntyped, StructOf16BitWith8BitVec2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storagePushConstant16);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const std::string spirv = R"(
OpCapability Shader
OpCapability Int8
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
%char = OpTypeInt 8 1
%v2char = OpTypeVector %char 2
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%ptr_output_v4float = OpTypePointer Output %v4float
%pos = OpVariable %ptr_output_v4float Output
%ptr = OpTypeUntypedPointerKHR PushConstant
%Foo = OpTypeStruct %uint %v2char %v4float
%block = OpTypeStruct %Foo
%pc = OpUntypedVariableKHR %ptr PushConstant %block
%main = OpFunction %void None %void_fn
%entry = OpLabel
%pc_access = OpUntypedAccessChainKHR %ptr %Foo %pc %uint_1
%ld_pc = OpLoad %v2char %pc_access
OpReturn
OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant16-06333");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");      // capability
    VkShaderObj vs(*m_device, spirv.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->Finish();
}
