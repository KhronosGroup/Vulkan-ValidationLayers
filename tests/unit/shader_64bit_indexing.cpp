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

class NegativeShader64BitIndexing : public VkLayerTest {};

TEST_F(NegativeShader64BitIndexing, Length64) {
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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpArrayLength-11807");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, ShaderObjectLength64) {
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpArrayLength-11807");
    const vkt::Shader shader(*m_device, create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, UntypedPointerLength64) {
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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpArrayLength-11807");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, CoopVecMul) {
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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulAddNV-11808");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, CoopVecLoad) {
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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpCooperativeVectorLoadNV-11809");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, PipelineMissingEnable) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    // don't enable shader64BitIndexing
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

    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11798");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, ShaderMissingEnable) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    // don't enable shader64BitIndexing
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

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-11758");
    const vkt::Shader shader(*m_device, create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShader64BitIndexing, ConstantSizeOfLength64) {
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
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
     %uint64 = OpTypeInt 64 0
         %15 = OpTypeBufferEXT StorageBuffer
       %16 = OpConstantSizeOfEXT %uint64 %15
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpConstantSizeOfEXT-11475");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}
