/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <spirv-tools/libspirv.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeShaderDebugInfo : public VkLayerTest {};

TEST_F(NegativeShaderDebugInfo, FileName1) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    char const *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
       %file = OpString "in.comp"
       %text = OpString "#version 450
layout(set = 0, binding = 0) buffer ssbo { uint y; };
void main() {
    atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
    %uint_64 = OpConstant %uint 64
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %ssbo = OpTypeStruct %uint
         %18 = OpExtInst %void %1 DebugSource %file %text
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %5
         %15 = OpLabel
         %48 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_4 %uint_0 %uint_0
         %47 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
               OpAtomicStore %47 %int_1 %uint_64 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-vulkanMemoryModel-06265
    m_errorMonitor->SetDesiredError("Error occurred at in.comp:4");
    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, FileName2) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    char const *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
       %file = OpString "in.comp"
       %text = OpString "#version 450
layout(set = 0, binding = 0) buffer ssbo { uint y; };
void main() {
    atomicStore(y, 1u, gl_ScopeDevice,
        gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
    %uint_64 = OpConstant %uint 64
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %ssbo = OpTypeStruct %uint
         %18 = OpExtInst %void %1 DebugSource %file %text
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %5
         %15 = OpLabel
         %48 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_5 %uint_0 %uint_0
         %47 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
               OpAtomicStore %47 %int_1 %uint_64 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-vulkanMemoryModel-06265
    m_errorMonitor->SetDesiredError("Error occurred at in.comp:4");
    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, FileName3) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    char const *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
       %file = OpString ""
       %text = OpString "#version 450
layout(set = 0, binding = 0) buffer ssbo { uint y; };
void main() {
    atomicStore(y, 1u, gl_ScopeDevice,
        gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
    %uint_64 = OpConstant %uint 64
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %ssbo = OpTypeStruct %uint
         %18 = OpExtInst %void %1 DebugSource %file %text
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %5
         %15 = OpLabel
         %48 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_5 %uint_0 %uint_0
         %47 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
               OpAtomicStore %47 %int_1 %uint_64 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-vulkanMemoryModel-06265
    m_errorMonitor->SetDesiredError("Error occurred at <source>:4");
    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, FileName4) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    char const *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
       %file = OpString "in.comp"
       %text = OpString "#version 450
layout(set = 0, binding = 0) buffer ssbo { uint y; };
void main() {
    atomicStore(y, 1u, gl_ScopeDevice,
        gl_StorageSemanticsBuffer,
        gl_SemanticsRelaxed);
}"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
     %uint_6 = OpConstant %uint 6
    %uint_30 = OpConstant %uint 30
    %uint_64 = OpConstant %uint 64
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %ssbo = OpTypeStruct %uint
         %18 = OpExtInst %void %1 DebugSource %file %text
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %5
         %15 = OpLabel
         %48 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_6 %uint_5 %uint_30
         %47 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
               OpAtomicStore %47 %int_1 %uint_64 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-vulkanMemoryModel-06265
    m_errorMonitor->SetDesiredError("Error occurred at in.comp:4:5");
    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, FloatShaderDebugInfo) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_shader_atomic_float : enable
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable

layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
void main() {
    y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
         %29 = OpString "float"
         %33 = OpString "y"
         %36 = OpString "ssbo"
         %43 = OpString ""
         %45 = OpString "int"
               OpSourceExtension "GL_EXT_shader_atomic_float"
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float32"
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpName %main "main"
               OpName %ssbo "ssbo"
               OpMemberName %ssbo 0 "y"
               OpName %_ ""
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.3"
               OpModuleProcessed "target-env vulkan1.1"
               OpModuleProcessed "entry-point main"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_7 = OpConstant %uint 7
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %21 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_7 %uint_0 %21 %16 %uint_3 %uint_7
      %float = OpTypeFloat 32
         %30 = OpExtInst %void %1 DebugTypeBasic %29 %uint_32 %uint_3 %uint_0
       %ssbo = OpTypeStruct %float
    %uint_54 = OpConstant %uint 54
         %32 = OpExtInst %void %1 DebugTypeMember %33 %30 %18 %uint_6 %uint_54 %uint_0 %uint_0 %uint_3
     %uint_8 = OpConstant %uint 8
         %35 = OpExtInst %void %1 DebugTypeComposite %36 %uint_1 %18 %uint_8 %uint_0 %21 %36 %uint_0 %uint_3 %32
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
    %uint_12 = OpConstant %uint 12
         %40 = OpExtInst %void %1 DebugTypePointer %35 %uint_12 %uint_0
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
         %42 = OpExtInst %void %1 DebugGlobalVariable %43 %35 %18 %uint_8 %uint_0 %21 %43 %_ %uint_8
        %int = OpTypeInt 32 1
         %46 = OpExtInst %void %1 DebugTypeBasic %45 %uint_32 %uint_4 %uint_0
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
         %50 = OpExtInst %void %1 DebugTypePointer %30 %uint_12 %uint_0
      %int_1 = OpConstant %int 1
     %int_64 = OpConstant %int 64
    %uint_64 = OpConstant %uint 64
     %uint_9 = OpConstant %uint 9
       %main = OpFunction %void None %5
         %15 = OpLabel
         %26 = OpExtInst %void %1 DebugScope %17
         %27 = OpExtInst %void %1 DebugLine %18 %uint_7 %uint_7 %uint_0 %uint_0
         %25 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %52 = OpExtInst %void %1 DebugLine %18 %uint_8 %uint_8 %uint_0 %uint_0
         %51 = OpAccessChain %_ptr_StorageBuffer_float %_ %int_0
         %56 = OpAtomicLoad %float %51 %int_1 %uint_64
         %57 = OpFAdd %float %float_1 %56
         %58 = OpAccessChain %_ptr_StorageBuffer_float %_ %int_0
               OpStore %58 %57
         %59 = OpExtInst %void %1 DebugLine %18 %uint_9 %uint_9 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-None-06284
    m_errorMonitor->SetDesiredError("atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);");
    VkShaderObj const cs(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, Int64Atomic) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderSharedInt64Atomics);  // to allow OpCapability Int64Atomics
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Int64Atomics
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "code.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_atomic_int64 : enable
#extension GL_KHR_memory_scope_semantics : enable
shared uint64_t x;
layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
void main() {
    int64_t value = 4;
    value *= 2;
    atomicAdd(y, value);
}"
         %29 = OpString "int64_t"
         %36 = OpString "value"
         %48 = OpString "uint64_t"
         %52 = OpString "y"
         %55 = OpString "ssbo"
         %61 = OpString ""
         %63 = OpString "int"
         %79 = OpString "x"
               OpSourceExtension "GL_EXT_shader_atomic_int64"
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int64"
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpDecorate %ssbo BufferBlock
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_7 = OpConstant %uint 7
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %21 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_7 %uint_0 %21 %16 %uint_3 %uint_7
       %long = OpTypeInt 64 1
    %uint_64 = OpConstant %uint 64
         %30 = OpExtInst %void %1 DebugTypeBasic %29 %uint_64 %uint_4 %uint_0
%_ptr_Function_long = OpTypePointer Function %long
         %33 = OpExtInst %void %1 DebugTypePointer %30 %uint_7 %uint_0
     %uint_8 = OpConstant %uint 8
         %35 = OpExtInst %void %1 DebugLocalVariable %36 %30 %18 %uint_8 %uint_0 %17 %uint_4
         %39 = OpExtInst %void %1 DebugExpression
     %long_4 = OpConstant %long 4
     %long_2 = OpConstant %long 2
     %uint_9 = OpConstant %uint 9
      %ulong = OpTypeInt 64 0
         %49 = OpExtInst %void %1 DebugTypeBasic %48 %uint_64 %uint_6 %uint_0
       %ssbo = OpTypeStruct %ulong
    %uint_53 = OpConstant %uint 53
         %51 = OpExtInst %void %1 DebugTypeMember %52 %49 %18 %uint_6 %uint_53 %uint_0 %uint_0 %uint_3
    %uint_10 = OpConstant %uint 10
         %54 = OpExtInst %void %1 DebugTypeComposite %55 %uint_1 %18 %uint_10 %uint_0 %21 %55 %uint_0 %uint_3 %51
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
         %58 = OpExtInst %void %1 DebugTypePointer %54 %uint_2 %uint_0
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
         %60 = OpExtInst %void %1 DebugGlobalVariable %61 %54 %18 %uint_10 %uint_0 %21 %61 %_ %uint_8
        %int = OpTypeInt 32 1
         %64 = OpExtInst %void %1 DebugTypeBasic %63 %uint_32 %uint_4 %uint_0
      %int_0 = OpConstant %int 0
%_ptr_Uniform_ulong = OpTypePointer Uniform %ulong
         %67 = OpExtInst %void %1 DebugTypePointer %49 %uint_2 %uint_0
    %uint_11 = OpConstant %uint 11
%_ptr_Workgroup_ulong = OpTypePointer Workgroup %ulong
         %76 = OpExtInst %void %1 DebugTypePointer %49 %uint_4 %uint_0
          %x = OpVariable %_ptr_Workgroup_ulong Workgroup
         %78 = OpExtInst %void %1 DebugGlobalVariable %79 %49 %18 %uint_11 %uint_0 %21 %79 %x %uint_8
       %main = OpFunction %void None %5
         %15 = OpLabel
      %value = OpVariable %_ptr_Function_long Function
         %26 = OpExtInst %void %1 DebugScope %17
         %27 = OpExtInst %void %1 DebugLine %18 %uint_7 %uint_7 %uint_0 %uint_0
         %25 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %40 = OpExtInst %void %1 DebugLine %18 %uint_8 %uint_8 %uint_0 %uint_0
         %38 = OpExtInst %void %1 DebugDeclare %35 %value %39
               OpStore %value %long_4
         %44 = OpExtInst %void %1 DebugLine %18 %uint_9 %uint_9 %uint_0 %uint_0
         %43 = OpLoad %long %value
         %46 = OpIMul %long %43 %long_2
               OpStore %value %46
         %69 = OpExtInst %void %1 DebugLine %18 %uint_10 %uint_10 %uint_0 %uint_0
         %68 = OpAccessChain %_ptr_Uniform_ulong %_ %int_0
         %70 = OpLoad %long %value
         %71 = OpBitcast %ulong %70
         %72 = OpAtomicIAdd %ulong %68 %uint_1 %uint_0 %71
         %73 = OpExtInst %void %1 DebugLine %18 %uint_11 %uint_11 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-None-06278
    m_errorMonitor->SetDesiredError("10:     atomicAdd(y, value);");
    VkShaderObj const cs(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, Int64AtomicSlang) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderBufferInt64Atomics);  // to allow OpCapability Int64Atomics
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"(
               OpCapability Int64
               OpCapability Int64Atomics
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %atomicVariable %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "
groupshared Atomic<uint64_t> atomicVariable;

[shader(\"compute\")]
[numthreads(1, 1, 1)]
void main(uint3 threadId: SV_DispatchThreadID)
{
    uint64_t value = 4ll * (uint64_t)threadId.x;

    atomicVariable.store(value);
}
"
          %5 = OpString "code.slang"
               OpSource Slang 1
         %20 = OpString "main"
         %36 = OpString "uint64"
         %40 = OpString "value"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_11 = OpConstant %uint 11
     %uint_5 = OpConstant %uint 5
   %uint_100 = OpConstant %uint 100
         %12 = OpTypeFunction %void
      %ulong = OpTypeInt 64 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
     %uint_0 = OpConstant %uint 0
     %uint_6 = OpConstant %uint 6
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
     %uint_8 = OpConstant %uint 8
    %uint_64 = OpConstant %uint 64
%uint_131072 = OpConstant %uint 131072
    %uint_14 = OpConstant %uint 14
    %ulong_4 = OpConstant %ulong 4
    %uint_10 = OpConstant %uint 10
     %uint_1 = OpConstant %uint 1
%_ptr_Workgroup_ulong = OpTypePointer Workgroup %ulong
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%atomicVariable = OpVariable %_ptr_Workgroup_ulong Workgroup
          %4 = OpExtInst %void %2 DebugSource %5 %1
         %10 = OpExtInst %void %2 DebugCompilationUnit %uint_100 %uint_5 %4 %uint_11
         %17 = OpExtInst %void %2 DebugTypeFunction %uint_0 %void
         %19 = OpExtInst %void %2 DebugFunction %20 %17 %4 %uint_6 %uint_6 %10 %20 %uint_0 %uint_6
         %35 = OpExtInst %void %2 DebugTypeBasic %36 %uint_64 %uint_6 %uint_131072
      %value = OpExtInst %void %2 DebugLocalVariable %40 %35 %4 %uint_8 %uint_14 %19 %uint_0
         %42 = OpExtInst %void %2 DebugExpression
       %main = OpFunction %void None %12
         %13 = OpLabel
         %16 = OpVariable %_ptr_Function_ulong Function
         %22 = OpExtInst %void %2 DebugFunctionDefinition %19 %main
         %63 = OpExtInst %void %2 DebugScope %19
         %28 = OpLoad %v3uint %gl_GlobalInvocationID
         %31 = OpExtInst %void %2 DebugLine %4 %uint_8 %uint_8 %uint_5 %uint_6
         %43 = OpExtInst %void %2 DebugDeclare %value %16 %42
         %44 = OpCompositeExtract %uint %28 0
         %45 = OpUConvert %ulong %44
    %value_0 = OpIMul %ulong %ulong_4 %45
               OpStore %16 %value_0
         %49 = OpExtInst %void %2 DebugLine %4 %uint_10 %uint_10 %uint_5 %uint_6
               OpAtomicStore %atomicVariable %uint_1 %uint_0 %value_0
               OpReturn
         %64 = OpExtInst %void %2 DebugNoScope
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-None-06279
    m_errorMonitor->SetDesiredError("10:     atomicVariable.store(value);\n        ^");
    VkShaderObj const cs(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderDebugInfo, WriteLessComponent) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "in.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450

layout(set = 0, binding = 0, Rgba8ui) uniform uimage2D storageImage;

void main(){
    imageStore(storageImage,
        ivec2(1, 1),
        uvec3(1, 1, 1)
    );
}"
         %30 = OpString "type.2d.image"
         %31 = OpString "@type.2d.image"
         %37 = OpString "storageImage"
         %42 = OpString "int"
               OpDecorate %storageImage Binding 0
               OpDecorate %storageImage DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
     %uint_6 = OpConstant %uint 6
     %uint_7 = OpConstant %uint 7
     %uint_8 = OpConstant %uint 8
     %uint_9 = OpConstant %uint 9
    %uint_10 = OpConstant %uint 10
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
         %21 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_5 %uint_0 %21 %16 %uint_3 %uint_5
         %28 = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
         %32 = OpExtInst %void %1 DebugInfoNone
         %29 = OpExtInst %void %1 DebugTypeComposite %30 %uint_0 %18 %uint_6 %uint_0 %21 %31 %32 %uint_3
%_ptr_UniformConstant_28 = OpTypePointer UniformConstant %28
         %34 = OpExtInst %void %1 DebugTypePointer %29 %uint_0 %uint_0
%storageImage = OpVariable %_ptr_UniformConstant_28 UniformConstant
         %36 = OpExtInst %void %1 DebugGlobalVariable %37 %29 %18 %uint_6 %uint_0 %21 %37 %storageImage %uint_8
        %int = OpTypeInt 32 1
         %43 = OpExtInst %void %1 DebugTypeBasic %42 %uint_32 %uint_4 %uint_0
      %v2int = OpTypeVector %int 2
         %45 = OpExtInst %void %1 DebugTypeVector %43 %uint_2
      %int_1 = OpConstant %int 1
         %47 = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
         %49 = OpExtInst %void %1 DebugTypeVector %9 %uint_3
         %50 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %5
         %15 = OpLabel
         %26 = OpExtInst %void %1 DebugScope %17
         %25 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %40 = OpExtInst %void %1 DebugLine %18 %uint_6 %uint_9 %uint_5 %uint_7
         %39 = OpLoad %28 %storageImage
               OpImageWrite %39 %47 %50
         %51 = OpExtInst %void %1 DebugLine %18 %uint_10 %uint_10 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    // VUID-RuntimeSpirv-OpImageWrite-07112
    m_errorMonitor->SetDesiredError(
        "6:     imageStore(storageImage,\n7:         ivec2(1, 1),\n8:         uvec3(1, 1, 1)\n9:     );");
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}