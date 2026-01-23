/*
 * Copyright (c) 2025 NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"

class PositiveShader64BitIndexing : public VkLayerTest {};

TEST_F(PositiveShader64BitIndexing, PragmaEnableLength64) {
    TEST_DESCRIPTION("Validate length64 supported when enabled by shader execution mode");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_64bit_indexing : enable
        #pragma shader_64bit_indexing
        layout(set = 0, binding = 0) readonly buffer B { float x[]; } b;
        void main(){ b.x.length64(); }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShader64BitIndexing, PipelineEnableLength64) {
    TEST_DESCRIPTION("Validate length64 supported when enabled by pipeline create flag");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_64bit_indexing : enable
        layout(set = 0, binding = 0) readonly buffer B { float x[]; } b;
        void main(){ b.x.length64(); }
    )glsl";

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_64_BIT_INDEXING_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShader64BitIndexing, ShaderObjectEnableLength64) {
    TEST_DESCRIPTION("Validate length64 supported when enabled by shader create flag");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_64bit_indexing : enable
        layout(set = 0, binding = 0) readonly buffer B { float x[]; } b;
        void main(){ b.x.length64(); }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &descriptor_set.layout_.handle());
    create_info.flags = VK_SHADER_CREATE_64_BIT_INDEXING_BIT_EXT;
    const vkt::Shader shader(*m_device, create_info);
}

TEST_F(PositiveShader64BitIndexing, UntypedPointerLength64) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability Int64
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %b
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_runtimearr_float ArrayStride 4
               OpDecorate %B Block
               OpMemberDecorate %B 0 NonWritable
               OpMemberDecorate %B 0 Offset 0
               OpDecorate %b NonWritable
               OpDecorate %b Binding 0
               OpDecorate %b DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
          %B = OpTypeStruct %_runtimearr_float
        %ptr = OpTypeUntypedPointerKHR StorageBuffer
          %b = OpUntypedVariableKHR %ptr StorageBuffer %B
      %ulong = OpTypeInt 64 0
       %long = OpTypeInt 64 1
       %main = OpFunction %void None %4
          %6 = OpLabel
         %13 = OpUntypedArrayLengthKHR %ulong %B %b 0
         %15 = OpBitcast %long %13
               OpReturn
               OpFunctionEnd
    )";

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_64_BIT_INDEXING_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShader64BitIndexing, CoopVecMul) {
    TEST_DESCRIPTION("Validate coopvec matmul 64-bit offset supported when enabled by pipeline create flag");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_NV_cooperative_vector : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_64bit_indexing : enable
        layout(set = 0, binding = 0) readonly buffer B { float x[]; } b;
        void main() {
            coopvecNV<float16_t, 16> A;
            coopvecNV<float16_t, 16> R;
            uint64_t offset = 0;
            coopVecMatMulNV(R, A, gl_ComponentTypeFloat16NV, b.x, offset, gl_ComponentTypeFloat16NV, 16, 16, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
        }
    )glsl";

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_64_BIT_INDEXING_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShader64BitIndexing, CoopVecLoad) {
    TEST_DESCRIPTION("Validate coopvec load 64-bit offset supported");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_NV_cooperative_vector : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_64bit_indexing : enable
        layout(set = 0, binding = 0) readonly buffer B { float x[]; } b;
        void main() {
            coopvecNV<float16_t, 16> A;
            uint64_t offset = 0;
            coopVecLoadNV(A, b.x, offset);
        }
    )glsl";

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_64_BIT_INDEXING_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShader64BitIndexing, ConstantSizeOfLength64) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_64BIT_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    AddRequiredFeature(vkt::Feature::shader64BitIndexing);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(Init());

    const char* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability Int64
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 BuiltIn ResourceHeapEXT
               OpMemberDecorate %A 0 Offset 0
               OpDecorateId %_runtimearr_15 ArrayStrideIdEXT %16
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint64 = OpTypeInt 64 0
          %A = OpTypeStruct %uint
     %uint_0 = OpConstant %uint 0
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %15 = OpTypeBufferEXT StorageBuffer
       %16 = OpConstantSizeOfEXT %uint64 %15
%_runtimearr_15 = OpTypeRuntimeArray %15
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
         %22 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_15 %resource_heap %int_0
         %18 = OpBufferPointerEXT %_ptr_StorageBuffer %14
         %19 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %18 %int_0
               OpStore %19 %uint_0
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_64_BIT_INDEXING_BIT_EXT | VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
}
