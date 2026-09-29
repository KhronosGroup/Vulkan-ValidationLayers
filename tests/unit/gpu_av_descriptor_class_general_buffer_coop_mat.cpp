/*
 * Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
 * Copyright (c) 2020-2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "descriptor_helper.h"
#include "cooperative_matrix_helper.h"

class NegativeGpuAVDescriptorClassGeneralBufferCoopMat : public GpuAVDescriptorClassGeneralBufferCoopMat {};

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, Store) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uint8_t payload[]; };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
            coopMatLoad(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 256, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, Load) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uint8_t payload[]; };
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         void main() {
            // Stride and element are in units of uint8_t, so this reads up to byte 16 + 15 * 64 + 64
            coopMatLoad(matC, payload, 16, 64, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 1024, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, Function) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uint8_t payload[]; };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;

         void foo() {
            coopMatStore(matA, payload, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
         }
         void main() {
            coopMatLoad(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            foo();
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 256, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, MidStruct) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO {
            vec4 a;
            uint32_t payload[16];
            float c;
         };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
            coopMatLoad(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 128, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, ElementSize) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uvec4 payload[]; }; // each element is 16 bytes
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA; // 256 bytes
         void main() {
            // access 528 (16 * 33) bytes into payload
            coopMatLoad(matA, payload, 33, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 768, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, Mix) {
    TEST_DESCRIPTION("Mix OOB from both a CoopMat and a normal store OOB");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(true));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) buffer SSBO0 { uint8_t good_payload[1024]; };
         layout(set=0, binding=1) buffer SSBO1 { uint8_t bad_payload[1024]; };
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         void main() {
            coopMatLoad(matC, good_payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, bad_payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
            // only want 1 thread to have error
            if (subgroupElect()) {
                bad_payload[1000] = uint8_t(0);
            }
         }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    vkt::Buffer ssbo(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, ssbo, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, ssbo, 0, 512, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);  // only half bound
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, Robustness) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    // Not the correct robustness for CoopMat
    AddRequiredFeature(vkt::Feature::robustBufferAccess);

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uint8_t payload[]; };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
            coopMatLoad(matA, payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 256, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, DynamicStride) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(true));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) buffer SSBO {
            uint32_t stride;
            uint8_t payload[];
         };
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         void main() {
            coopMatLoad(matC, payload, 0, stride + 4, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
         }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 2048, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    uint32_t* buffer_ptr = (uint32_t*)in_buffer.Memory().Map();
    buffer_ptr[0] = 140;  // stride + 4 = 144 bytes, which keeps it 16 byte aligned

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-storageBuffers-06936
    // 4 (offset of payload) + 15 * 144 + 64 - 1
    m_errorMonitor->SetDesiredError("highest out of bounds access was at [2227] bytes");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, DynamicElement) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(true));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) buffer SSBO {
            int32_t element;
            uint8_t payload[];
         };
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         void main() {
            coopMatLoad(matC, payload, element, 16, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
         }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 2048, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    uint32_t* buffer_ptr = (uint32_t*)in_buffer.Memory().Map();
    buffer_ptr[0] = 2048;  // element

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-storageBuffers-06936
    // 4 (offset of payload) + 2048 + 15 * 16 + 64 - 1
    m_errorMonitor->SetDesiredError("highest out of bounds access was at [2355] bytes");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeGpuAVDescriptorClassGeneralBufferCoopMat, DynamicStride64) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(true));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    // layout(set=0, binding=0) buffer SSBO {
    //    uint64_t stride;
    //    layout(offset = 16) uint8_t payload[];
    // };
    // coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
    // coopMatLoad(matC, payload, 0, stride, gl_CooperativeMatrixLayoutColumnMajor);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability Int8
               OpCapability Int64
               OpCapability StorageBuffer8BitAccess
               OpCapability VulkanMemoryModel
               OpCapability CooperativeMatrixKHR
               OpExtension "SPV_KHR_cooperative_matrix"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main" %matC %ssbo
               OpExecutionMode %main LocalSize 64 1 1
               OpDecorate %_runtimearr_uchar ArrayStride 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 16
               OpDecorate %ssbo Binding 0
               OpDecorate %ssbo DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
     %uint_0 = OpConstant %uint 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
    %uint_16 = OpConstant %uint 16
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
   %coop_mat = OpTypeCooperativeMatrixKHR %uint %uint_3 %uint_16 %uint_16 %uint_2
%_ptr_Private_coop_mat = OpTypePointer Private %coop_mat
       %matC = OpVariable %_ptr_Private_coop_mat Private
      %uchar = OpTypeInt 8 0
%_runtimearr_uchar = OpTypeRuntimeArray %uchar
       %SSBO = OpTypeStruct %ulong %_runtimearr_uchar
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
       %ssbo = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
%_ptr_StorageBuffer_uchar = OpTypePointer StorageBuffer %uchar
%_ptr_StorageBuffer_ulong = OpTypePointer StorageBuffer %ulong
       %main = OpFunction %void None %3
          %5 = OpLabel
    %payload = OpAccessChain %_ptr_StorageBuffer_uchar %ssbo %int_1 %uint_0
 %stride_ptr = OpAccessChain %_ptr_StorageBuffer_ulong %ssbo %int_0
     %stride = OpLoad %ulong %stride_ptr
     %loaded = OpCooperativeMatrixLoadKHR %coop_mat %payload %int_1 %stride None
               OpStore %matC %loaded
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 2048, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    auto* buffer_ptr = static_cast<uint64_t*>(in_buffer.Memory().Map());
    buffer_ptr[0] = 144;  // stride
    in_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-storageBuffers-06936
    // 16 (offset of payload) + 15 * 144 + 64 - 1
    m_errorMonitor->SetDesiredError("highest out of bounds access was at [2239] bytes");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
