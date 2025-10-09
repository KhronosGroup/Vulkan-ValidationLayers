/*
 * Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

class NegativeGpuAVShaderDebugInfo : public GpuAVBufferDeviceAddressTest {
  public:
    void BasicSingleStorageBufferComputeOOB(const char *shader, const char *error);
};

// shader must have a SSBO at (set = 0, binding = 0)
void NegativeGpuAVShaderDebugInfo::BasicSingleStorageBufferComputeOOB(const char *shader, const char *error) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError(error);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, OpLine) {
    TEST_DESCRIPTION("Make sure basic OpLine works");

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString ""
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
               OpLine %1 42 24
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "Shader validation error occurred at <source>:42:24");
}

TEST_F(NegativeGpuAVShaderDebugInfo, OpLineColumn) {
    TEST_DESCRIPTION("Make sure the column in OpLine will add value to show which part the error occured");

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer { int indices[]; };
layout(set = 0, binding = 0) buffer foo { IndexBuffer data; int x; };
void main() {
    x = data.indices[16];
}"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
               OpLine %1 6 14
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "    x = data.indices[16];\n             ^");
}

TEST_F(NegativeGpuAVShaderDebugInfo, OpSourceContinued) {
    TEST_DESCRIPTION("Make sure can find source in OpSourceContinued");

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer { int indices[]; };"
               OpSourceContinued "layout(set = 0, binding = 0) buffer foo { IndexBuffer data; int x; };"
               OpSourceContinued "void main() {
    x = data.indices[16];
}"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
               OpLine %1 6 14
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "    x = data.indices[16];\n             ^");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BadLineNumber) {
    TEST_DESCRIPTION("OpLine gives a line number not in the source");

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer { int indices[]; };
layout(set = 0, binding = 0) buffer foo { IndexBuffer data; int x; };
void main() {
    x = data.indices[16];
}"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
               OpLine %1 20 14
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "Unable to find a suitable line in SPIR-V OpSource");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BasicGlslang) {
    TEST_DESCRIPTION("Make sure basic OpLine and OpSource are working with glslang");

    // Manually ran:
    //   glslangValidator -V -g in.comp -o out.spv --target-env vulkan1.2
    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
               OpSourceExtension "GL_EXT_buffer_reference"
               OpName %main "main"
               OpName %foo "foo"
               OpMemberName %foo 0 "data"
               OpMemberName %foo 1 "x"
               OpName %IndexBuffer "IndexBuffer"
               OpMemberName %IndexBuffer 0 "indices"
               OpName %_ ""
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.5"
               OpModuleProcessed "target-env vulkan1.2"
               OpModuleProcessed "entry-point main"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
               OpLine %1 10 11
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 11 0
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "Shader validation error occurred at a.comp:11\n\n    x = data.indices[16];");
}

TEST_F(NegativeGpuAVShaderDebugInfo, GlslLineDerective) {
    TEST_DESCRIPTION("Use the #line derective in GLSL");

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main()  {
#line 9000
    x = data.indices[16];
}"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
               OpLine %1 10 11
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 9000 0
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source,
                                       "Shader validation error occurred at a.comp:9000\n\n9000:     x = data.indices[16];");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BasicGlslangShaderDebugInfo) {
    TEST_DESCRIPTION("Make sure basic glslang with ShaderDebugInfo works");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_RELAXED_EXTENDED_INSTRUCTION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderRelaxedExtendedInstruction);

    // Manually ran:
    //   glslangValidator -V -gV in.comp -o out.spv --target-env vulkan1.2
    const char *shader_source = R"(
                       OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
               OpExtension "SPV_KHR_relaxed_extended_instruction"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %31 = OpString "int"
         %35 = OpString "data"
         %39 = OpString "x"
         %43 = OpString "foo"
         %49 = OpString "indices"
         %51 = OpString "IndexBuffer"
         %57 = OpString ""
               OpSourceExtension "GL_EXT_buffer_reference"
               OpName %main "main"
               OpName %foo "foo"
               OpMemberName %foo 0 "data"
               OpMemberName %foo 1 "x"
               OpName %IndexBuffer "IndexBuffer"
               OpMemberName %IndexBuffer 0 "indices"
               OpName %_ ""
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
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
         %18 = OpExtInst %void %1 DebugSource %2
    %uint_10 = OpConstant %uint 10
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_10 %uint_0 %20 %16 %uint_3 %uint_10
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
         %29 = OpExtInstWithForwardRefsKHR %void %1 DebugTypePointer %50 %uint_5349 %uint_0
        %int = OpTypeInt 32 1
         %32 = OpExtInst %void %1 DebugTypeBasic %31 %uint_32 %uint_4 %uint_0
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
     %uint_7 = OpConstant %uint 7
    %uint_17 = OpConstant %uint 17
         %34 = OpExtInst %void %1 DebugTypeMember %35 %29 %18 %uint_7 %uint_17 %uint_0 %uint_0 %uint_3
     %uint_8 = OpConstant %uint 8
     %uint_9 = OpConstant %uint 9
         %38 = OpExtInst %void %1 DebugTypeMember %39 %32 %18 %uint_8 %uint_9 %uint_0 %uint_0 %uint_3
    %uint_11 = OpConstant %uint 11
         %42 = OpExtInst %void %1 DebugTypeComposite %43 %uint_1 %18 %uint_11 %uint_0 %20 %43 %uint_0 %uint_3 %34 %38
%_runtimearr_int = OpTypeRuntimeArray %int
         %46 = OpExtInst %void %1 DebugTypeArray %32 %uint_0
%IndexBuffer = OpTypeStruct %_runtimearr_int
         %48 = OpExtInst %void %1 DebugTypeMember %49 %46 %18 %uint_4 %uint_9 %uint_0 %uint_0 %uint_3
         %50 = OpExtInst %void %1 DebugTypeComposite %51 %uint_1 %18 %uint_11 %uint_0 %20 %51 %uint_0 %uint_3 %48
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
    %uint_12 = OpConstant %uint 12
         %54 = OpExtInst %void %1 DebugTypePointer %42 %uint_12 %uint_0
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
         %56 = OpExtInst %void %1 DebugGlobalVariable %57 %42 %18 %uint_11 %uint_0 %20 %57 %_ %uint_8
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
         %61 = OpExtInst %void %1 DebugTypePointer %29 %uint_12 %uint_0
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
         %67 = OpExtInst %void %1 DebugTypePointer %32 %uint_5349 %uint_0
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
         %71 = OpExtInst %void %1 DebugTypePointer %32 %uint_12 %uint_0
       %main = OpFunction %void None %5
         %15 = OpLabel
         %25 = OpExtInst %void %1 DebugScope %17
         %26 = OpExtInst %void %1 DebugLine %18 %uint_10 %uint_10 %uint_0 %uint_0
         %24 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %63 = OpExtInst %void %1 DebugLine %18 %uint_11 %uint_11 %uint_0 %uint_0
         %62 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %64 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %62
         %68 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %64 %int_0 %int_16
         %69 = OpLoad %int %68 Aligned 4
         %72 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %72 %69
         %73 = OpExtInst %void %1 DebugLine %18 %uint_12 %uint_12 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source,
                                       "Shader validation error occurred at a.comp:11\nNo Text operand found in DebugSource");
}

// Manually ran:
//   glslangValidator -V -gVS in.comp -o out.spv --target-env vulkan1.2
static const char *kBasicGlslShaderSource = R"(
              OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
               OpExtension "SPV_KHR_relaxed_extended_instruction"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
         %32 = OpString "int"
         %36 = OpString "data"
         %40 = OpString "x"
         %44 = OpString "foo"
         %50 = OpString "indices"
         %52 = OpString "IndexBuffer"
         %58 = OpString ""
               OpSourceExtension "GL_EXT_buffer_reference"
               OpName %main "main"
               OpName %foo "foo"
               OpMemberName %foo 0 "data"
               OpMemberName %foo 1 "x"
               OpName %IndexBuffer "IndexBuffer"
               OpMemberName %IndexBuffer 0 "indices"
               OpName %_ ""
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
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
    %uint_10 = OpConstant %uint 10
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %21 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_10 %uint_0 %21 %16 %uint_3 %uint_10
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
         %30 = OpExtInstWithForwardRefsKHR %void %1 DebugTypePointer %51 %uint_5349 %uint_0
        %int = OpTypeInt 32 1
         %33 = OpExtInst %void %1 DebugTypeBasic %32 %uint_32 %uint_4 %uint_0
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
     %uint_7 = OpConstant %uint 7
    %uint_17 = OpConstant %uint 17
         %35 = OpExtInst %void %1 DebugTypeMember %36 %30 %18 %uint_7 %uint_17 %uint_0 %uint_0 %uint_3
     %uint_8 = OpConstant %uint 8
     %uint_9 = OpConstant %uint 9
         %39 = OpExtInst %void %1 DebugTypeMember %40 %33 %18 %uint_8 %uint_9 %uint_0 %uint_0 %uint_3
    %uint_11 = OpConstant %uint 11
         %43 = OpExtInst %void %1 DebugTypeComposite %44 %uint_1 %18 %uint_11 %uint_0 %21 %44 %uint_0 %uint_3 %35 %39
%_runtimearr_int = OpTypeRuntimeArray %int
         %47 = OpExtInst %void %1 DebugTypeArray %33 %uint_0
%IndexBuffer = OpTypeStruct %_runtimearr_int
         %49 = OpExtInst %void %1 DebugTypeMember %50 %47 %18 %uint_4 %uint_9 %uint_0 %uint_0 %uint_3
         %51 = OpExtInst %void %1 DebugTypeComposite %52 %uint_1 %18 %uint_11 %uint_0 %21 %52 %uint_0 %uint_3 %49
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
    %uint_12 = OpConstant %uint 12
         %55 = OpExtInst %void %1 DebugTypePointer %43 %uint_12 %uint_0
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
         %57 = OpExtInst %void %1 DebugGlobalVariable %58 %43 %18 %uint_11 %uint_0 %21 %58 %_ %uint_8
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
         %62 = OpExtInst %void %1 DebugTypePointer %30 %uint_12 %uint_0
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
         %68 = OpExtInst %void %1 DebugTypePointer %33 %uint_5349 %uint_0
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
         %72 = OpExtInst %void %1 DebugTypePointer %33 %uint_12 %uint_0
       %main = OpFunction %void None %5
         %15 = OpLabel
         %26 = OpExtInst %void %1 DebugScope %17
         %27 = OpExtInst %void %1 DebugLine %18 %uint_10 %uint_10 %uint_0 %uint_0
         %25 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %64 = OpExtInst %void %1 DebugLine %18 %uint_11 %uint_11 %uint_0 %uint_0
         %63 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %65 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %63
         %69 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %65 %int_0 %int_16
         %70 = OpLoad %int %69 Aligned 4
         %73 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %73 %70
         %74 = OpExtInst %void %1 DebugLine %18 %uint_12 %uint_12 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
)";

TEST_F(NegativeGpuAVShaderDebugInfo, BasicGlslangShaderDebugInfoWithSource) {
    TEST_DESCRIPTION("Make sure basic glslang with ShaderDebugInfo works");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_RELAXED_EXTENDED_INSTRUCTION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderRelaxedExtendedInstruction);

    BasicSingleStorageBufferComputeOOB(kBasicGlslShaderSource,
                                       "Shader validation error occurred at a.comp:11\n\n11:     x = data.indices[16];");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BasicGlslangShaderDebugInfoWithSourceShaderObject) {
    TEST_DESCRIPTION("Make sure basic glslang with ShaderDebugInfo works");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_RELAXED_EXTENDED_INSTRUCTION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderRelaxedExtendedInstruction);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    std::vector<uint32_t> cs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_2, 0, kBasicGlslShaderSource, cs_spv);
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, cs_spv, &descriptor_set.layout_.handle());

    m_command_buffer.Begin();
    m_command_buffer.BindCompShader(comp_shader);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Shader validation error occurred at a.comp:11\n\n11:     x = data.indices[16];");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, ShaderDebugInfoColumns) {
    TEST_DESCRIPTION("DebugLine has a Column Start and Column End");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
         %18 = OpExtInst %void %1 DebugSource %2 %19
    %uint_10 = OpConstant %uint 10
     %uint_1 = OpConstant %uint 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
    %uint_11 = OpConstant %uint 11
    %uint_14 = OpConstant %uint 14
    %uint_16 = OpConstant %uint 16
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %5
         %15 = OpLabel
         %64 = OpExtInst %void %1 DebugLine %18 %uint_11 %uint_11 %uint_14 %uint_16
         %63 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %65 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %63
         %69 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %65 %int_0 %int_16
         %70 = OpLoad %int %69 Aligned 4
         %73 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %73 %70
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "    x = data.indices[16];\n                 ^");
}

TEST_F(NegativeGpuAVShaderDebugInfo, ShaderDebugSourceContinued) {
    TEST_DESCRIPTION("Make sure can find source in DebugSourceContinued");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %16 = OpString "main"
         %s1 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};"
         %s2 = OpString "layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};"
         %s3 = OpString "void main() {
    x = data.indices[16];
}"
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
         %18 = OpExtInst %void %1 DebugSource %2 %s1
         %c1 = OpExtInst %void %1 DebugSourceContinued %s2
         %c2 = OpExtInst %void %1 DebugSourceContinued %s3
    %uint_10 = OpConstant %uint 10
     %uint_1 = OpConstant %uint 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
    %uint_11 = OpConstant %uint 11
    %uint_14 = OpConstant %uint 14
    %uint_16 = OpConstant %uint 16
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %5
         %15 = OpLabel
         %64 = OpExtInst %void %1 DebugLine %18 %uint_11 %uint_11 %uint_14 %uint_16
         %63 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %65 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %63
         %69 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %65 %int_0 %int_16
         %70 = OpLoad %int %69 Aligned 4
         %73 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %73 %70
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "    x = data.indices[16];\n                 ^");
}

TEST_F(NegativeGpuAVShaderDebugInfo, ShaderDebugLineMultiLine) {
    TEST_DESCRIPTION("DebugLine has a Line Start and Line End");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
         %18 = OpExtInst %void %1 DebugSource %2 %19
    %uint_10 = OpConstant %uint 10
    %uint_11 = OpConstant %uint 11
    %uint_12 = OpConstant %uint 12
     %uint_1 = OpConstant %uint 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %5
         %15 = OpLabel
         %64 = OpExtInst %void %1 DebugLine %18 %uint_10 %uint_12 %uint_11 %uint_11
         %63 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %65 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %63
         %69 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %65 %int_0 %int_16
         %70 = OpLoad %int %69 Aligned 4
         %73 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %73 %70
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "10: void main() {\n11:     x = data.indices[16];\n12: }");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BadShaderDebugLineStart) {
    TEST_DESCRIPTION("DebugLine Line Start has bad value");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
         %18 = OpExtInst %void %1 DebugSource %2 %19
    %uint_11 = OpConstant %uint 11
    %uint_20 = OpConstant %uint 20
     %uint_1 = OpConstant %uint 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %5
         %15 = OpLabel
         %64 = OpExtInst %void %1 DebugLine %18 %uint_20 %uint_20 %uint_11 %uint_11
         %63 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %65 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %63
         %69 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %65 %int_0 %int_16
         %70 = OpLoad %int %69 Aligned 4
         %73 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %73 %70
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "20: [No line found in source]");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BadShaderDebugLineEnd) {
    TEST_DESCRIPTION("DebugLine Line End has bad value");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
         %18 = OpExtInst %void %1 DebugSource %2 %19
    %uint_10 = OpConstant %uint 10
    %uint_11 = OpConstant %uint 11
    %uint_20 = OpConstant %uint 20
     %uint_1 = OpConstant %uint 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %5
         %15 = OpLabel
         %64 = OpExtInst %void %1 DebugLine %18 %uint_10 %uint_20 %uint_11 %uint_11
         %63 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %65 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %63
         %69 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %65 %int_0 %int_16
         %70 = OpLoad %int %69 Aligned 4
         %73 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %73 %70
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source,
                                       "10: void main() {\n11:     x = data.indices[16];\n12: }\n13: [No line found in source]");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BasicDXC) {
    TEST_DESCRIPTION("Make sure basic dxc with ShaderDebugInfo works");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // Manually ran:
    //   dxc -spirv -T cs_6_0 -E main -fspv-target-env=vulkan1.2 -fspv-extension=SPV_KHR_non_semantic_info
    //   -fspv-debug=vulkan-with-source in.hlsl
    const char *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data
               OpExecutionMode %main LocalSize 1 1 1
          %4 = OpString "a.hlsl"
          %5 = OpString "RWByteAddressBuffer data : register(u0);

[numthreads(1, 1, 1)]
void main() {
    data.Store(0, uint(int(data.Load(68))));
}
"
          %6 = OpString "main"
          %7 = OpString ""
          %8 = OpString "d9a5e97d"
          %9 = OpString " -E main -T cs_6_0 -spirv -fspv-target-env=vulkan1.2 -fspv-extension=SPV_KHR_non_semantic_info -fspv-debug=vulkan-with-source -Qembed_debug"
         %10 = OpString "uint"
         %11 = OpString "type.RWByteAddressBuffer"
         %12 = OpString "data"
               OpName %type_RWByteAddressBuffer "type.RWByteAddressBuffer"
               OpName %data "data"
               OpName %main "main"
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %type_RWByteAddressBuffer 0 Offset 0
               OpDecorate %type_RWByteAddressBuffer Block
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_32 = OpConstant %uint 32
%_runtimearr_uint = OpTypeRuntimeArray %uint
%type_RWByteAddressBuffer = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer_type_RWByteAddressBuffer = OpTypePointer StorageBuffer %type_RWByteAddressBuffer
       %void = OpTypeVoid
     %uint_3 = OpConstant %uint 3
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
    %uint_13 = OpConstant %uint 13
     %uint_6 = OpConstant %uint 6
    %uint_21 = OpConstant %uint 21
     %uint_8 = OpConstant %uint 8
         %40 = OpTypeFunction %void
    %uint_43 = OpConstant %uint 43
    %uint_28 = OpConstant %uint 28
    %uint_40 = OpConstant %uint 40
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
        %int = OpTypeInt 32 1
    %uint_19 = OpConstant %uint 19
       %data = OpVariable %_ptr_StorageBuffer_type_RWByteAddressBuffer StorageBuffer
    %uint_17 = OpConstant %uint 17
         %15 = OpExtInst %void %1 DebugInfoNone
         %16 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %4 %5
         %19 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_5
         %23 = OpExtInst %void %1 DebugFunction %6 %16 %18 %uint_4 %uint_1 %19 %7 %uint_3 %uint_4
         %24 = OpExtInst %void %1 DebugLexicalBlock %18 %uint_4 %uint_13 %23
         %26 = OpExtInst %void %1 DebugTypeBasic %10 %uint_32 %uint_6 %uint_0
         %30 = OpExtInst %void %1 DebugTypeArray %26 %uint_0
         %31 = OpExtInst %void %1 DebugTypeMember %7 %30 %18 %uint_0 %uint_0 %uint_0 %uint_0 %uint_3
         %32 = OpExtInst %void %1 DebugTypeComposite %11 %uint_1 %18 %uint_0 %uint_0 %19 %11 %uint_0 %uint_3 %31
         %33 = OpExtInst %void %1 DebugGlobalVariable %12 %32 %18 %uint_1 %uint_21 %19 %12 %data %uint_8
         %36 = OpExtInst %void %1 DebugEntryPoint %23 %19 %8 %9
       %main = OpFunction %void None %40
         %48 = OpLabel
         %77 = OpExtInst %void %1 DebugScope %24
         %49 = OpExtInst %void %1 DebugLine %18 %uint_5 %uint_5 %uint_28 %uint_40
         %50 = OpAccessChain %_ptr_StorageBuffer_uint %data %uint_0 %uint_17
         %52 = OpLoad %uint %50
         %54 = OpBitcast %int %52
         %55 = OpExtInst %void %1 DebugLine %18 %uint_5 %uint_5 %uint_19 %uint_19
         %56 = OpBitcast %uint %54
         %57 = OpExtInst %void %1 DebugLine %18 %uint_5 %uint_5 %uint_5 %uint_43
         %58 = OpAccessChain %_ptr_StorageBuffer_uint %data %uint_0 %uint_0
               OpStore %58 %56
         %78 = OpExtInst %void %1 DebugNoScope
         %60 = OpExtInst %void %1 DebugLine %18 %uint_6 %uint_6 %uint_1 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-storageBuffers-06936
    m_errorMonitor->SetDesiredError("5:     data.Store(0, uint(int(data.Load(68))));");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, NoLineInFunctionFirst) {
    TEST_DESCRIPTION("Test if first function listed, has no debug info, we don't use the functions after it");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
               OpExtension "SPV_KHR_physical_storage_buffer"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %20 = OpString "Bar"
         %23 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};

uint Bar() {
    return x == 0 ? 32 : 64; // both OOB
}

void main() { // has no debug info
    uint y = Bar();
    x = data.indices[y];
}"
         %29 = OpString "main"
         %43 = OpString "data"
         %64 = OpString ""
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
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
         %16 = OpTypeFunction %uint
         %17 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9
         %22 = OpExtInst %void %1 DebugSource %2 %23
    %uint_11 = OpConstant %uint 11
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %25 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %22 %uint_2
         %21 = OpExtInst %void %1 DebugFunction %20 %17 %22 %uint_11 %uint_0 %25 %20 %uint_3 %uint_11
    %uint_15 = OpConstant %uint 15
         %30 = OpExtInst %void %1 DebugFunction %29 %6 %22 %uint_15 %uint_0 %25 %29 %uint_3 %uint_15
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
     %uint_7 = OpConstant %uint 7
    %uint_17 = OpConstant %uint 17
     %uint_8 = OpConstant %uint 8
     %uint_9 = OpConstant %uint 9
    %uint_12 = OpConstant %uint 12
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
     %int_32 = OpConstant %int 32
     %int_64 = OpConstant %int 64
    %uint_13 = OpConstant %uint 13
%_ptr_Function_uint = OpTypePointer Function %uint
    %uint_16 = OpConstant %uint 16
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
    %uint_18 = OpConstant %uint 18
       %main = OpFunction %void None %5
         %15 = OpLabel
          %y = OpVariable %_ptr_Function_uint Function
         %97 = OpFunctionCall %uint %Bar_
               OpStore %y %97
        %100 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
        %102 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %100
        %103 = OpLoad %uint %y
        %106 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %102 %int_0 %103
        %107 = OpLoad %int %106 Aligned 4
        %108 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %108 %107
        %109 = OpExtInst %void %1 DebugLine %22 %uint_18 %uint_18 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
       %Bar_ = OpFunction %uint None %16
         %19 = OpLabel
         %33 = OpExtInst %void %1 DebugScope %21
         %34 = OpExtInst %void %1 DebugLine %22 %uint_11 %uint_11 %uint_0 %uint_0
         %32 = OpExtInst %void %1 DebugFunctionDefinition %21 %Bar_
         %69 = OpExtInst %void %1 DebugLine %22 %uint_12 %uint_12 %uint_0 %uint_0
         %68 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
         %70 = OpLoad %int %68
         %75 = OpIEqual %bool %70 %int_0
         %78 = OpSelect %int %75 %int_32 %int_64
         %79 = OpBitcast %uint %78
               OpReturnValue %79
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source,
                                       "(Unable to find shader source, build shader with debug info to get source information)");
}

TEST_F(NegativeGpuAVShaderDebugInfo, NoLineInFunctionLast) {
    TEST_DESCRIPTION("Test if last function listed, has no debug info, we don't use the functions before it");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    const char *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_non_semantic_info"
               OpExtension "SPV_KHR_physical_storage_buffer"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %20 = OpString "Bar"
         %23 = OpString "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};

uint Bar() {
    return x == 0 ? 32 : 64; // both OOB
}

void main() { // has no debug info
    uint y = Bar();
    x = data.indices[y];
}"
         %29 = OpString "main"
         %43 = OpString "data"
         %64 = OpString ""
               OpDecorate %foo Block
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %_runtimearr_int ArrayStride 4
               OpDecorate %IndexBuffer Block
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
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
         %16 = OpTypeFunction %uint
         %17 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9
         %22 = OpExtInst %void %1 DebugSource %2 %23
    %uint_11 = OpConstant %uint 11
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %25 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %22 %uint_2
         %21 = OpExtInst %void %1 DebugFunction %20 %17 %22 %uint_11 %uint_0 %25 %20 %uint_3 %uint_11
    %uint_15 = OpConstant %uint 15
         %30 = OpExtInst %void %1 DebugFunction %29 %6 %22 %uint_15 %uint_0 %25 %29 %uint_3 %uint_15
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
  %uint_5349 = OpConstant %uint 5349
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
     %uint_7 = OpConstant %uint 7
    %uint_17 = OpConstant %uint 17
     %uint_8 = OpConstant %uint 8
     %uint_9 = OpConstant %uint 9
    %uint_12 = OpConstant %uint 12
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
     %int_32 = OpConstant %int 32
     %int_64 = OpConstant %int 64
    %uint_13 = OpConstant %uint 13
%_ptr_Function_uint = OpTypePointer Function %uint
    %uint_16 = OpConstant %uint 16
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
    %uint_18 = OpConstant %uint 18
       %Bar_ = OpFunction %uint None %16
         %19 = OpLabel
         %33 = OpExtInst %void %1 DebugScope %21
         %34 = OpExtInst %void %1 DebugLine %22 %uint_11 %uint_11 %uint_0 %uint_0
         %32 = OpExtInst %void %1 DebugFunctionDefinition %21 %Bar_
         %69 = OpExtInst %void %1 DebugLine %22 %uint_12 %uint_12 %uint_0 %uint_0
         %68 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
         %70 = OpLoad %int %68
         %75 = OpIEqual %bool %70 %int_0
         %78 = OpSelect %int %75 %int_32 %int_64
         %79 = OpBitcast %uint %78
               OpReturnValue %79
               OpFunctionEnd
       %main = OpFunction %void None %5
         %15 = OpLabel
          %y = OpVariable %_ptr_Function_uint Function
         %97 = OpFunctionCall %uint %Bar_
               OpStore %y %97
        %100 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
        %102 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %100
        %103 = OpLoad %uint %y
        %106 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %102 %int_0 %103
        %107 = OpLoad %int %106 Aligned 4
        %108 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %108 %107
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source,
                                       "(Unable to find shader source, build shader with debug info to get source information)");
}

TEST_F(NegativeGpuAVShaderDebugInfo, PipelineHandles) {
    TEST_DESCRIPTION("Make sure we are printing out which pipeline the error is from");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();

    const char *pipe_name = "bad_pipeline";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.objectHandle = (uint64_t)pipe.Handle();
    name_info.pObjectName = pipe_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    const char *cb_name = "bad_command_buffer";
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    name_info.objectHandle = (uint64_t)m_command_buffer.handle();
    name_info.pObjectName = cb_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredErrorRegex("", "VkCommandBuffer.*bad_command_buffer.*\n.*VkPipeline.*bad_pipeline");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, ShaderObjectHandle) {
    TEST_DESCRIPTION("Make sure we are printing out which shader object the error is from");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderObject);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char comp_src[] = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    VkDescriptorSetLayout descriptorSetLayout = descriptor_set.layout_;
    VkShaderStageFlagBits shader_stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    const vkt::Shader comp_shader(*m_device, shader_stages[0], GLSLToSPV(shader_stages[0], comp_src), &descriptorSetLayout);

    const char *object_name = "bad_shader_object";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_EXT;
    name_info.objectHandle = (uint64_t)comp_shader.handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindShadersEXT(m_command_buffer, 1, shader_stages, &comp_shader.handle());
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Shader Object (bad_shader_object)");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, CommandBufferCommandIndex) {
    TEST_DESCRIPTION("Make sure we print which index in the command buffer the issue occured");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    CreateComputePipelineHelper empty_compute_pipe(*this);
    empty_compute_pipe.CreateComputePipeline();
    CreatePipelineHelper empty_graphics_pipe(*this);
    empty_graphics_pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, empty_compute_pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);  // dispatch index 0

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, empty_graphics_pipe);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);  // draw index 0
    m_command_buffer.EndRenderPass();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_.handle(), 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);  // dispatch index 1
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Compute Dispatch Index 1");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfo) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 2, 1, 1);
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Global invocation ID (x, y, z) = (1, 0, 0)");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfoWithDebugLabel1) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly, and debug label region management is correct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
        IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "Dispatch debug label region";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);
    // Dispatch is within debug label region
    vk::CmdDispatch(m_command_buffer, 2, 1, 1);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Dispatch debug label region");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfoWithDebugLabel2) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly, and debug label region management is correct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
        IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "Dispatch debug label region";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    // Dispatch is not within debug label region
    vk::CmdDispatch(m_command_buffer, 2, 1, 1);

    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredFailureMsgRegex(kErrorBit, "UNASSIGNED-Device address out of bounds",
                                              "Global invocation ID \\(x, y, z\\) = \\(1, 0, 0\\)", "Dispatch debug label region");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfoWithDebugLabel3) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly, and debug label region management is correct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
        IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_0_label);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);

    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_1_label);

    VkDebugUtilsLabelEXT region_2_label = vku::InitStructHelper();
    region_2_label.pLabelName = "region_2";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_2_label);

    // End of region 2
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    vk::CmdDispatch(m_command_buffer, 2, 1, 1);

    // End of region 1
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    // End of region 0
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("region_0::region_1");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfoWithDebugLabel4) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly, and debug label region management is correct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
        IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Use debug label regions in secondary command buffer
    secondary_cb.Begin();
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &region_0_label);

    vk::CmdBindPipeline(secondary_cb, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(secondary_cb, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);

    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &region_1_label);

    VkDebugUtilsLabelEXT region_2_label = vku::InitStructHelper();
    region_2_label.pLabelName = "region_2";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &region_2_label);

    // End of region 2
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);

    vk::CmdDispatch(secondary_cb, 2, 1, 1);

    // End of region 1
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);

    // End of region 0
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);

    secondary_cb.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_cb.handle());
    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("region_0::region_1");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

// Todo add support debug label region coming from primary command buffer in a secondary command buffer
// Todo add test case where the same secondary command buffer is executed multiple times
// => debug regions from primary command buffer should only be added
TEST_F(NegativeGpuAVShaderDebugInfo, DISABLED_StageInfoWithDebugLabel5) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly, and debug label region management is correct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
        IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Use debug label regions in secondary command buffer
    secondary_cb.Begin();
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &region_0_label);

    vk::CmdBindPipeline(secondary_cb, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(secondary_cb, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);

    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &region_1_label);

    VkDebugUtilsLabelEXT region_2_label = vku::InitStructHelper();
    region_2_label.pLabelName = "region_2";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &region_2_label);

    // End of region 2
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);

    vk::CmdDispatch(secondary_cb, 2, 1, 1);

    // End of region 1
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);

    // End of region 0
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);

    secondary_cb.End();

    m_command_buffer.Begin();

    VkDebugUtilsLabelEXT primary_region_label = vku::InitStructHelper();
    primary_region_label.pLabelName = "primary";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &primary_region_label);

    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_cb.handle());

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_command_buffer.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("primary::region_0::region_1");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfoWithDebugLabel6) {
    TEST_DESCRIPTION(
        "Make sure we print the stage info correctly, and debug label region management is correct. Start a debug label region in "
        "one command buffer, and end it in another.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_0_label);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);

    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_1_label);

    VkDebugUtilsLabelEXT region_2_label = vku::InitStructHelper();
    region_2_label.pLabelName = "region_2";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_2_label);

    // End of region 2
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    vk::CmdDispatch(m_command_buffer, 2, 1, 1);

    m_command_buffer.End();

    vkt::CommandBuffer cb_2(*m_device, m_command_pool);
    cb_2.Begin();
    // End of region 1
    vk::CmdEndDebugUtilsLabelEXT(cb_2);

    vk::CmdBindPipeline(cb_2, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(cb_2, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(cb_2, 2, 1, 1);

    // End of region 0
    vk::CmdEndDebugUtilsLabelEXT(cb_2);

    cb_2.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("region_0::region_1");
    m_errorMonitor->SetDesiredError("region_0");
    m_default_queue->Submit({m_command_buffer, cb_2});
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, StageInfoWithDebugLabel7) {
    TEST_DESCRIPTION(
        "Make sure we print the stage info correctly, and debug label region management is correct."
        "Start a debug label region in one command buffer, and end it in another.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
        IndexBuffer data;
            int x;
        };
        void main() {
            if (gl_WorkGroupID.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = VkShaderObj(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    bad_pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_.UpdateDescriptorSets();

    vkt::CommandBuffer cb_2_label_beginnings(*m_device, m_command_pool);

    cb_2_label_beginnings.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(cb_2_label_beginnings, &region_0_label);

    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(cb_2_label_beginnings, &region_1_label);

    cb_2_label_beginnings.End();

    vkt::CommandBuffer cb_4_label_ends(*m_device, m_command_pool);
    cb_4_label_ends.Begin();

    vk::CmdBindPipeline(cb_4_label_ends, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe);
    vk::CmdBindDescriptorSets(cb_4_label_ends, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_, 0, 1,
                              &bad_pipe.descriptor_set_.set_, 0, nullptr);

    vk::CmdDispatch(cb_4_label_ends, 2, 1, 1);

    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);
    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);

    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);

    vk::CmdDispatch(cb_4_label_ends, 2, 1, 1);

    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);

    cb_4_label_ends.End();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("region_0::region_1::region_0::region_1");
    m_errorMonitor->SetDesiredError("region_0");
    m_default_queue->Submit({cb_2_label_beginnings, cb_2_label_beginnings, cb_4_label_ends});
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, PostProcessingDebugLabelMultipleCommandBuffers) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };

        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0); // invalid
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    vkt::CommandBuffer cb_2_label_beginnings(*m_device, m_command_pool);

    cb_2_label_beginnings.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(cb_2_label_beginnings, &region_0_label);

    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(cb_2_label_beginnings, &region_1_label);

    cb_2_label_beginnings.End();

    vkt::CommandBuffer cb_4_label_ends(*m_device, m_command_pool);
    cb_4_label_ends.Begin();

    vk::CmdBindDescriptorSets(cb_4_label_ends, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(cb_4_label_ends, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(cb_4_label_ends, 1, 1, 1);

    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);
    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);
    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);
    vk::CmdEndDebugUtilsLabelEXT(cb_4_label_ends);
    cb_4_label_ends.End();

    // VUID-RuntimeSpirv-samples-08726
    m_errorMonitor->SetDesiredError("region_0::region_1::region_0::region_1");
    m_default_queue->Submit({cb_2_label_beginnings, cb_2_label_beginnings, cb_4_label_ends});
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, IndirectCommandDebugLabel) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());

    vkt::Buffer indirect_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.Memory().Map());
    ptr[0].x = 1;
    ptr[0].y = 1;
    ptr[0].z = 1;
    ptr[1].x = 1;
    ptr[1].y = 1;
    ptr[1].z = 1;
    ptr[2].x = 4;  // over
    ptr[2].y = 2;
    ptr[2].z = 1;

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    VkDebugUtilsLabelEXT region_2_label = vku::InitStructHelper();
    region_2_label.pLabelName = "region_2";

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);

    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_0_label);
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, sizeof(VkDispatchIndirectCommand));

    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_1_label);
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, sizeof(VkDispatchIndirectCommand) * 2);  // bad

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &region_2_label);
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_command_buffer.End();

    // VUID-VkDispatchIndirectCommand-x-00417
    m_errorMonitor->SetDesiredError("Debug region: region_0::region_1");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, ShaderInternalId) {
    TEST_DESCRIPTION("Make sure we print the shader id used internally correctly");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *cs_shader = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Foo { mat4 x; };
        void main() {
            mat4 y = x;
        }
    )glsl";

    // Create extra pipelines to increase the internal shader ID
    for (uint32_t i = 0; i < 16; i++) {
        CreateComputePipelineHelper unused_pipe(*this);
        unused_pipe.cs_ = VkShaderObj(this, cs_shader, VK_SHADER_STAGE_COMPUTE_BIT);
        unused_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
        unused_pipe.CreateComputePipeline();
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_shader, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-uniformBuffers-06935
    m_errorMonitor->SetDesiredError("(internal ID 17)");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, PostProcess) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char *fs_source = R"(
               OpCapability Shader
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
          %1 = OpString "bad_code.glsl"
               OpSource GLSL 450 %1 "#version 450
layout(set=0, binding=0) uniform sampler3D x;
layout(set=0, binding=1) uniform sampler2D y;
layout(location=0) out vec4 color;
void main() {
    color = texture(y, vec2(0)); // valid
    color += texture(x, vec3(0));
    color += texture(y, vec2(1)); // valid
}"
               OpName %main "main"                  ; id %5
               OpName %color "color"                ; id %10
               OpName %y "y"                        ; id %14
               OpName %x "x"                        ; id %23
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.3"
               OpModuleProcessed "target-env vulkan1.1"
               OpModuleProcessed "entry-point main"

               ; Annotations
               OpDecorate %color Location 0
               OpDecorate %y Binding 1
               OpDecorate %y DescriptorSet 0
               OpDecorate %x Binding 0
               OpDecorate %x DescriptorSet 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output   ; Location 0
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
          %y = OpVariable %_ptr_UniformConstant_12 UniformConstant  ; Binding 1, DescriptorSet 0
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %18 = OpConstantComposite %v2float %float_0 %float_0
         %20 = OpTypeImage %float 3D 0 0 0 1 Unknown
         %21 = OpTypeSampledImage %20
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
          %x = OpVariable %_ptr_UniformConstant_21 UniformConstant  ; Binding 0, DescriptorSet 0
    %v3float = OpTypeVector %float 3
         %26 = OpConstantComposite %v3float %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
         %32 = OpConstantComposite %v2float %float_1 %float_1
               OpLine %1 5 11

               ; Function main
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 6 0
         %15 = OpLoad %12 %y
         %19 = OpImageSampleImplicitLod %v4float %15 %18
               OpStore %color %19
               OpLine %1 7 0
         %24 = OpLoad %21 %x
         %27 = OpImageSampleImplicitLod %v4float %24 %26
         %28 = OpLoad %v4float %color
         %29 = OpFAdd %v4float %28 %27
               OpStore %color %29
               OpLine %1 8 0
         %30 = OpLoad %12 %y
         %33 = OpImageSampleImplicitLod %v4float %30 %32
         %34 = OpLoad %v4float %color
         %35 = OpFAdd %v4float %34 %33
               OpStore %color %35
               OpLine %1 9 0
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);

    vkt::Image image(*m_device, 16, 16, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("bad_code.glsl:7\n\n    color += texture(x, vec3(0));");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, PostProcessSlangGPL) {
    TEST_DESCRIPTION("Use both GPL and Slang to generate shader debug info");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char *fs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %y %s %x %entryPointParam_main
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
          %1 = OpString "Texture3D x : register(t0, space0);
Texture2D y : register(t1, space0);
SamplerState s : register(s0, space0);

[shader(\"fragment\")]
float4 main() : SV_Target {
    float4 color = y.Sample(s, float2(0.0f, 0.0f)); // valid
    color += x.Sample(s, float3(0.0f, 0.0f, 0.0f));
    color += y.Sample(s, float2(1.0f, 1.0f)); // valid
    return color;
}
"
          %5 = OpString "bad_code.slang"
               OpSource Slang 1
         %21 = OpString "main"
         %27 = OpString "slangc"
         %28 = OpString "-target spirv -matrix-layout-column-major -stage pixel -entry main -g2"
         %34 = OpString "float"
         %41 = OpString "color"
               OpName %color "color"                ; id %40
               OpName %y "y"                        ; id %48
               OpName %s "s"                        ; id %52
               OpName %sampledImage "sampledImage"  ; id %54
               OpName %sampled "sampled"            ; id %55
               OpName %x "x"                        ; id %66
               OpName %sampledImage_0 "sampledImage"    ; id %68
               OpName %sampled_0 "sampled"              ; id %69
               OpName %color_0 "color"                  ; id %73
               OpName %sampledImage_1 "sampledImage"    ; id %79
               OpName %sampled_1 "sampled"              ; id %80
               OpName %color_1 "color"                  ; id %84
               OpName %entryPointParam_main "entryPointParam_main"  ; id %89
               OpName %main "main"                                  ; id %11

               ; Annotations
               OpDecorate %y Binding 1
               OpDecorate %y DescriptorSet 0
               OpDecorate %s Binding 0
               OpDecorate %s DescriptorSet 0
               OpDecorate %x Binding 0
               OpDecorate %x DescriptorSet 0
               OpDecorate %entryPointParam_main Location 0

               ; Types, variables and constants
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_11 = OpConstant %uint 11
     %uint_5 = OpConstant %uint 5
   %uint_100 = OpConstant %uint 100
         %12 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
     %uint_0 = OpConstant %uint 0
     %uint_6 = OpConstant %uint 6
     %uint_8 = OpConstant %uint 8
     %uint_7 = OpConstant %uint 7
    %uint_32 = OpConstant %uint 32
     %uint_3 = OpConstant %uint 3
%uint_131072 = OpConstant %uint 131072
     %uint_4 = OpConstant %uint 4
    %uint_12 = OpConstant %uint 12
         %45 = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_45 = OpTypePointer UniformConstant %45
         %49 = OpTypeSampler
%_ptr_UniformConstant_49 = OpTypePointer UniformConstant %49
         %53 = OpTypeSampledImage %45
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %57 = OpConstantComposite %v2float %float_0 %float_0
         %63 = OpTypeImage %float 3D 2 0 0 1 Unknown
%_ptr_UniformConstant_63 = OpTypePointer UniformConstant %63
         %67 = OpTypeSampledImage %63
    %v3float = OpTypeVector %float 3
         %71 = OpConstantComposite %v3float %float_0 %float_0 %float_0
     %uint_9 = OpConstant %uint 9
    %float_1 = OpConstant %float 1
         %81 = OpConstantComposite %v2float %float_1 %float_1
    %uint_10 = OpConstant %uint 10
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %y = OpVariable %_ptr_UniformConstant_45 UniformConstant  ; Binding 1, DescriptorSet 0
          %s = OpVariable %_ptr_UniformConstant_49 UniformConstant  ; Binding 0, DescriptorSet 0
          %x = OpVariable %_ptr_UniformConstant_63 UniformConstant  ; Binding 0, DescriptorSet 0
%entryPointParam_main = OpVariable %_ptr_Output_v4float Output      ; Location 0
         %43 = OpExtInst %void %2 DebugExpression
          %4 = OpExtInst %void %2 DebugSource %5 %1
         %10 = OpExtInst %void %2 DebugCompilationUnit %uint_100 %uint_5 %4 %uint_11
         %18 = OpExtInst %void %2 DebugTypeFunction %uint_0 %void
         %20 = OpExtInst %void %2 DebugFunction %21 %18 %4 %uint_6 %uint_8 %10 %21 %uint_0 %uint_6
         %26 = OpExtInst %void %2 DebugEntryPoint %20 %10 %27 %28
         %33 = OpExtInst %void %2 DebugTypeBasic %34 %uint_32 %uint_3 %uint_131072
         %38 = OpExtInst %void %2 DebugTypeVector %33 %uint_4
      %color = OpExtInst %void %2 DebugLocalVariable %41 %38 %4 %uint_7 %uint_12 %20 %uint_0

               ; Function main
       %main = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v4float Function
         %24 = OpExtInst %void %2 DebugFunctionDefinition %20 %main
        %114 = OpExtInst %void %2 DebugScope %20
         %29 = OpExtInst %void %2 DebugLine %4 %uint_7 %uint_7 %uint_5 %uint_6
         %44 = OpExtInst %void %2 DebugDeclare %color %17 %43
         %46 = OpLoad %45 %y
         %50 = OpLoad %49 %s
%sampledImage = OpSampledImage %53 %46 %50
    %sampled = OpImageSampleImplicitLod %v4float %sampledImage %57 None
               OpStore %17 %sampled
         %61 = OpExtInst %void %2 DebugLine %4 %uint_8 %uint_8 %uint_5 %uint_6
         %62 = OpLoad %49 %s
         %64 = OpLoad %63 %x
%sampledImage_0 = OpSampledImage %67 %64 %62
  %sampled_0 = OpImageSampleImplicitLod %v4float %sampledImage_0 %71 None
    %color_0 = OpFAdd %v4float %sampled %sampled_0
               OpStore %17 %color_0
         %75 = OpExtInst %void %2 DebugLine %4 %uint_9 %uint_9 %uint_5 %uint_6
         %77 = OpLoad %45 %y
         %78 = OpLoad %49 %s
%sampledImage_1 = OpSampledImage %53 %77 %78
  %sampled_1 = OpImageSampleImplicitLod %v4float %sampledImage_1 %81 None
    %color_1 = OpFAdd %v4float %color_0 %sampled_1
               OpStore %17 %color_1
         %86 = OpExtInst %void %2 DebugLine %4 %uint_10 %uint_10 %uint_5 %uint_6
               OpStore %entryPointParam_main %color_1
               OpReturn
        %115 = OpExtInst %void %2 DebugNoScope
               OpFunctionEnd
    )";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    vkt::Image image(*m_device, 16, 16, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler);
    descriptor_set.UpdateDescriptorSets();
    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster_lib(*this);
    pre_raster_lib.InitPreRasterLibInfo(&vs.GetStageCreateInfo());
    pre_raster_lib.gp_ci_.layout = pipeline_layout;
    pre_raster_lib.CreateGraphicsPipeline();

    CreatePipelineHelper frag_shader_lib(*this);
    frag_shader_lib.InitFragmentLibInfo(&fs.GetStageCreateInfo());
    frag_shader_lib.gp_ci_.layout = pipeline_layout;
    frag_shader_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib,
        pre_raster_lib,
        frag_shader_lib,
        frag_out_lib,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("bad_code.slang:8:5\n\n8:     color += x.Sample(s, float3(0.0f, 0.0f, 0.0f));");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, ReachMaxActionsCommandValidationLimitUnknown) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0) buffer foo {
            uint x[32];
        };

        void main() {
            x[31] = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_empty(*this);
    pipe_empty.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_empty);
    for (int i = 0; i < 20000; ++i) {
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    }

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-storageBuffers-06936
    m_errorMonitor->SetDesiredError("Compute Dispatch Index Unknown");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, ReachMaxErrorLoggerLimitUnkown) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());

    vkt::Buffer indirect_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.Memory().Map());
    ptr[0].x = 1;
    ptr[0].y = 1;
    ptr[0].z = 1;
    ptr[1].x = 4;  // over
    ptr[1].y = 2;
    ptr[1].z = 1;

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    for (int i = 0; i < 10000; ++i) {
        vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);
    }

    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, sizeof(VkDispatchIndirectCommand));  // bad
    m_command_buffer.End();

    // VUID-VkDispatchIndirectCommand-x-00417
    m_errorMonitor->SetDesiredError("GPUAV-Overflow-Unknown");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
