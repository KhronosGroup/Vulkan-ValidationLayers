/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_helper.h"

void GpuAVSharedMemoryOobTest::InitSharedMemoryOob() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVSharedMemoryOob : public GpuAVSharedMemoryOobTest {
  protected:
    void TestHelper(const char* source);
};

void PositiveGpuAVSharedMemoryOob::TestHelper(const char* shader_source) {
    RETURN_IF_SKIP(InitSharedMemoryOob());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVSharedMemoryOob, Simple1DInBounds) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 4) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[4];
        void main() {
            arr[gl_LocalInvocationIndex % 4] = gl_LocalInvocationIndex;
            barrier();
            ssbo.data[gl_LocalInvocationIndex] = arr[gl_LocalInvocationIndex % 4];
        }
    )glsl";

    TestHelper(shader_source);
}

TEST_F(PositiveGpuAVSharedMemoryOob, Array2DInBounds) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[2][4];
        void main() {
            arr[0][3] = 1;
            arr[1][0] = 2;
            ssbo.data[0] = arr[0][3] + arr[1][0];
        }
    )glsl";

    TestHelper(shader_source);
}

TEST_F(PositiveGpuAVSharedMemoryOob, StructArrayInBounds) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        struct S { uint x; };
        shared S s[4];
        void main() {
            s[0].x = 1;
            s[3].x = 2;
            ssbo.data[0] = s[0].x + s[3].x;
        }
    )glsl";

    TestHelper(shader_source);
}

TEST_F(PositiveGpuAVSharedMemoryOob, BoundaryIndex) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[4];
        void main() {
            arr[3] = 42;
            ssbo.data[0] = arr[3];
        }
    )glsl";

    TestHelper(shader_source);
}

TEST_F(PositiveGpuAVSharedMemoryOob, VectorExtractInBounds) {
    const char* shader_source = R"spirv(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo %v
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %runtime_arr ArrayStride 4
               OpMemberDecorate %ssbo_struct 0 Offset 0
               OpDecorate %ssbo_struct Block
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
    %float_1 = OpConstant %float 1
%runtime_arr = OpTypeRuntimeArray %uint
%ssbo_struct = OpTypeStruct %runtime_arr
   %ssbo_ptr = OpTypePointer StorageBuffer %ssbo_struct
   %elem_ptr = OpTypePointer StorageBuffer %uint
       %ssbo = OpVariable %ssbo_ptr StorageBuffer
     %wg_ptr = OpTypePointer Workgroup %v4float
          %v = OpVariable %wg_ptr Workgroup
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
   %init_vec = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
               OpStore %v %init_vec
   %loaded_v = OpLoad %v4float %v
  %extracted = OpVectorExtractDynamic %float %loaded_v %uint_3
       %cast = OpBitcast %uint %extracted
      %chain = OpAccessChain %elem_ptr %ssbo %uint_0 %uint_0
               OpStore %chain %cast
               OpReturn
               OpFunctionEnd
    )spirv";

    RETURN_IF_SKIP(InitSharedMemoryOob());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVSharedMemoryOob, LongVectorInBounds) {
    AddRequiredFeature(vkt::Feature::longVector);
    AddRequiredExtensions(VK_EXT_SHADER_LONG_VECTOR_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_EXT_long_vector : enable
        layout(local_size_x = 5) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        layout(constant_id = 0) const uint N = 5;
        shared vector<uint, N> v;
        void main() {
            v[gl_LocalInvocationIndex % N] = gl_LocalInvocationIndex;
            barrier();
            ssbo.data[gl_LocalInvocationIndex] = v[gl_LocalInvocationIndex % N];
        }
    )glsl";

    TestHelper(shader_source);
}

TEST_F(PositiveGpuAVSharedMemoryOob, SlangNestedStruct) {
    RETURN_IF_SKIP(InitSharedMemoryOob());
    RETURN_IF_SKIP(CheckSlangSupport());

    const char* shader_source = R"slang(
        RWStructuredBuffer<uint> outputBuffer;

        [numthreads(2, 1, 1)]
        void main(uint3 groupThreadID : SV_GroupThreadID)
        {
            struct A {
                uint32_t b;
                uint32_t c;
            };
            struct S {
                uint32_t x;
                uint32_t y;
                A z[2];
            };

            static groupshared S temp;
            temp.z[groupThreadID.x].b = groupThreadID.x;
            outputBuffer[groupThreadID.x] = temp.z[groupThreadID.x].b;
        }
    )slang";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_SLANG);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

// Mixed constant + dynamic indexing on a 2D array, both in bounds.
TEST_F(PositiveGpuAVSharedMemoryOob, MixedConstantDynamicArray2D) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[2][4];
        void main() {
            uint i = ssbo.data[0] % 4;
            uint j = ssbo.data[0] % 2;
            arr[1][i] = 1;
            arr[j][2] = 2;
            ssbo.data[0] = arr[1][i] + arr[j][2];
        }
    )glsl";

    TestHelper(shader_source);
}

// Vector accessed by OpAccessChain with a constant index (the GLSL .x writes an access chain
// with a literal 0 index).
TEST_F(PositiveGpuAVSharedMemoryOob, VectorAccessChainConstantIndex) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uvec4 v;
        void main() {
            v.x = 1;
            v.w = 4;
            ssbo.data[0] = v.x + v.w;
        }
    )glsl";

    TestHelper(shader_source);
}

// OpCompositeExtract is not matched by the pass; this just confirms a pattern that lowers
// to OpCompositeExtract doesn't break anything.
TEST_F(PositiveGpuAVSharedMemoryOob, CompositeExtractConstantIndex) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uvec4 v;
        void main() {
            v = uvec4(1, 2, 3, 4);
            uvec4 loaded = v;
            ssbo.data[0] = loaded.x + loaded.z;
        }
    )glsl";

    TestHelper(shader_source);
}

// OpVectorExtractDynamic with a literal constant index, in bounds.
TEST_F(PositiveGpuAVSharedMemoryOob, VectorExtractDynamicConstantIndex) {
    const char* shader_source = R"spirv(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo %v
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %runtime_arr ArrayStride 4
               OpMemberDecorate %ssbo_struct 0 Offset 0
               OpDecorate %ssbo_struct Block
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %uint_0 = OpConstant %uint 0
     %uint_2 = OpConstant %uint 2
    %float_1 = OpConstant %float 1
%runtime_arr = OpTypeRuntimeArray %uint
%ssbo_struct = OpTypeStruct %runtime_arr
   %ssbo_ptr = OpTypePointer StorageBuffer %ssbo_struct
   %elem_ptr = OpTypePointer StorageBuffer %uint
       %ssbo = OpVariable %ssbo_ptr StorageBuffer
     %wg_ptr = OpTypePointer Workgroup %v4float
          %v = OpVariable %wg_ptr Workgroup
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
   %init_vec = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
               OpStore %v %init_vec
   %loaded_v = OpLoad %v4float %v
  %extracted = OpVectorExtractDynamic %float %loaded_v %uint_2
       %cast = OpBitcast %uint %extracted
      %chain = OpAccessChain %elem_ptr %ssbo %uint_0 %uint_0
               OpStore %chain %cast
               OpReturn
               OpFunctionEnd
    )spirv";

    RETURN_IF_SKIP(InitSharedMemoryOob());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

// Function-scope local array, dynamic index always in bounds.
TEST_F(PositiveGpuAVSharedMemoryOob, FunctionStorageArrayInBounds) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        void main() {
            uint arr[4];
            uint i = ssbo.data[0] & 3u;
            arr[i] = 42;
            ssbo.data[0] = arr[i];
        }
    )glsl";

    TestHelper(shader_source);
}

// File-scope global array, which glslang lowers to Private storage. Dynamic index in bounds.
TEST_F(PositiveGpuAVSharedMemoryOob, PrivateStorageArrayInBounds) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        uint privateArr[4];
        void main() {
            uint i = ssbo.data[0] & 3u;
            privateArr[i] = 42;
            ssbo.data[0] = privateArr[i];
        }
    )glsl";

    TestHelper(shader_source);
}

TEST_F(PositiveGpuAVSharedMemoryOob, VectorInsertInBounds) {
    const char* shader_source = R"spirv(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo %v
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %runtime_arr ArrayStride 4
               OpMemberDecorate %ssbo_struct 0 Offset 0
               OpDecorate %ssbo_struct Block
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
%runtime_arr = OpTypeRuntimeArray %uint
%ssbo_struct = OpTypeStruct %runtime_arr
   %ssbo_ptr = OpTypePointer StorageBuffer %ssbo_struct
   %elem_ptr = OpTypePointer StorageBuffer %uint
       %ssbo = OpVariable %ssbo_ptr StorageBuffer
     %wg_ptr = OpTypePointer Workgroup %v4float
          %v = OpVariable %wg_ptr Workgroup
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
   %init_vec = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
               OpStore %v %init_vec
   %loaded_v = OpLoad %v4float %v
   %inserted = OpVectorInsertDynamic %v4float %loaded_v %float_2 %uint_3
               OpStore %v %inserted
      %chain = OpAccessChain %elem_ptr %ssbo %uint_0 %uint_0
       %cast = OpBitcast %uint %float_2
               OpStore %chain %cast
               OpReturn
               OpFunctionEnd
    )spirv";

    RETURN_IF_SKIP(InitSharedMemoryOob());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}
