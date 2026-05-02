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

class NegativeGpuAVArrayOob : public GpuAVArrayOobTest {
  protected:
    void TestHelper(const char* source, uint32_t expected_index, uint32_t expected_bound,
                    const char* vuid = "SPIRV-ArrayOob-OpAccessChain", SpvSourceType source_type = SPV_SOURCE_GLSL,
                    VkSpecializationInfo* spec_info = nullptr, const char* expected_variable_name = nullptr);
};

void NegativeGpuAVArrayOob::TestHelper(const char* shader_source, uint32_t expected_index, uint32_t expected_bound,
                                              const char* vuid, SpvSourceType source_type, VkSpecializationInfo* spec_info,
                                              const char* expected_variable_name) {
    RETURN_IF_SKIP(InitArrayOob());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, source_type, spec_info);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t* buf_ptr = (uint32_t*)in_buffer.Memory().Map();
    memset((void*)buf_ptr, 0, 32);
    in_buffer.Memory().Unmap();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    std::string regex;
    if (expected_variable_name) {
        regex = std::string("\"") + expected_variable_name + "\".*";
    }
    regex += std::to_string(expected_index) + " is >= .* " + std::to_string(expected_bound);
    m_errorMonitor->SetDesiredErrorRegex(vuid, regex.c_str());
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVArrayOob, Simple1DArray) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer {
            uint idx;
            uint data[];
        };
        shared uint arr[4];
        void main() {
            uint i = idx + data[0] + 4;
            arr[i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 4, 4);
}

TEST_F(NegativeGpuAVArrayOob, Array2DInner) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[2][4];
        void main() {
            uint i = ssbo.data[0] + 4;
            arr[0][i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 4, 4);
}

TEST_F(NegativeGpuAVArrayOob, Array2DOuter) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[2][4];
        void main() {
            uint i = ssbo.data[0] + 2;
            arr[i][0] = 0;
        }
    )glsl";

    TestHelper(shader_source, 2, 2);
}

// Outer index is a constant in bounds, inner is dynamic and OOB. Spot check that the inner
// check fires correctly when the outer is a constant.
TEST_F(NegativeGpuAVArrayOob, Array2DConstantOuterDynamicInner) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[2][4];
        void main() {
            uint i = ssbo.data[0] + 4;
            arr[1][i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 4, 4);
}

// Outer is dynamic and OOB, inner is a constant in bounds. Spot check that the outer check
// fires correctly when the inner is a constant.
TEST_F(NegativeGpuAVArrayOob, Array2DDynamicOuterConstantInner) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[2][4];
        void main() {
            uint i = ssbo.data[0] + 2;
            arr[i][3] = 0;
        }
    )glsl";

    TestHelper(shader_source, 2, 2);
}

TEST_F(NegativeGpuAVArrayOob, StructWithArray) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        struct S { uint a; uint b[4]; };
        shared S s;
        void main() {
            uint i = ssbo.data[0] + 4;
            s.b[i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 4, 4);
}

TEST_F(NegativeGpuAVArrayOob, ArrayOfStructs) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        struct S { uint x; };
        shared S s[2];
        void main() {
            uint i = ssbo.data[0] + 2;
            s[i].x = 0;
        }
    )glsl";

    TestHelper(shader_source, 2, 2);
}

TEST_F(NegativeGpuAVArrayOob, VectorExtractDynamic) {
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
     %uint_4 = OpConstant %uint 4
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
      %chain = OpAccessChain %elem_ptr %ssbo %uint_0 %uint_0
    %idx_val = OpLoad %uint %chain
    %oob_idx = OpIAdd %uint %idx_val %uint_4
   %loaded_v = OpLoad %v4float %v
  %extracted = OpVectorExtractDynamic %float %loaded_v %oob_idx
       %cast = OpBitcast %uint %extracted
               OpStore %chain %cast
               OpReturn
               OpFunctionEnd
    )spirv";

    TestHelper(shader_source, 4, 4, "SPIRV-ArrayOob-OpVectorExtractDynamic", SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVArrayOob, VectorInsertDynamic) {
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
     %uint_4 = OpConstant %uint 4
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
      %chain = OpAccessChain %elem_ptr %ssbo %uint_0 %uint_0
    %idx_val = OpLoad %uint %chain
    %oob_idx = OpIAdd %uint %idx_val %uint_4
   %loaded_v = OpLoad %v4float %v
  %inserted = OpVectorInsertDynamic %v4float %loaded_v %float_2 %oob_idx
               OpStore %v %inserted
       %cast = OpBitcast %uint %float_2
               OpStore %chain %cast
               OpReturn
               OpFunctionEnd
    )spirv";

    TestHelper(shader_source, 4, 4, "SPIRV-ArrayOob-OpVectorInsertDynamic", SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVArrayOob, LongVectorOob) {
    AddRequiredFeature(vkt::Feature::longVector);
    AddRequiredExtensions(VK_EXT_SHADER_LONG_VECTOR_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_EXT_long_vector : enable
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        layout(constant_id = 0) const uint N = 5;
        shared vector<uint, N> v;
        void main() {
            uint i = ssbo.data[0] + 5;
            v[i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 5, 5);
}

TEST_F(NegativeGpuAVArrayOob, SlangNestedStruct) {
    RETURN_IF_SKIP(InitArrayOob());
    RETURN_IF_SKIP(CheckSlangSupport());

    const char* shader_source = R"slang(
        RWStructuredBuffer<uint> outputBuffer;

        [numthreads(1, 1, 1)]
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
            uint i = outputBuffer[0] + 2;
            temp.z[i].b = 0;
            outputBuffer[0] = temp.z[0].b;
        }
    )slang";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_SLANG);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t* buf_ptr = (uint32_t*)in_buffer.Memory().Map();
    buf_ptr[0] = 0;
    in_buffer.Memory().Unmap();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("SPIRV-ArrayOob-OpAccessChain", "2 is >= array size 2");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVArrayOob, SpecConstantArraySize) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        layout(constant_id = 0) const uint SIZE = 4;
        shared uint arr[SIZE];
        void main() {
            arr[3] = 1;
            ssbo.data[0] = arr[3];
        }
    )glsl";

    uint32_t spec_value = 3;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info = {1, &entry, sizeof(uint32_t), &spec_value};
    TestHelper(shader_source, 3, 3, "SPIRV-ArrayOob-OpAccessChain", SPV_SOURCE_GLSL, &spec_info);
}

TEST_F(NegativeGpuAVArrayOob, MaxSharedMemorySize) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework(
        {{OBJECT_LAYER_NAME, "gpuav_shared_memory_data_race", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkFalse}}, false));
    RETURN_IF_SKIP(InitState());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t array_size = max_shared_memory_size / sizeof(uint32_t);

    std::ostringstream cs;
    cs << R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[)glsl";
    cs << array_size;
    cs << R"glsl(];
        void main() {
            uint i = ssbo.data[0] + )glsl";
    cs << array_size;
    cs << R"glsl(;
            arr[i] = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t* buf_ptr = (uint32_t*)in_buffer.Memory().Map();
    memset((void*)buf_ptr, 0, 32);
    in_buffer.Memory().Unmap();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    const std::string regex = std::to_string(array_size) + " is >= array size " + std::to_string(array_size);
    m_errorMonitor->SetDesiredErrorRegex("SPIRV-ArrayOob-OpAccessChain", regex.c_str());
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVArrayOob, MeshShader) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[4];
        void main() {
            SetMeshOutputsEXT(0, 0);
            uint i = ssbo.data[0] + 4;
            arr[i] = 0;
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t* buf_ptr = (uint32_t*)in_buffer.Memory().Map();
    buf_ptr[0] = 0;
    in_buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDrawMeshTasksEXT(m_command_buffer, 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("SPIRV-ArrayOob-OpAccessChain", "4 is >= array size 4");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

// Function-scope local array. Dynamic indexing keeps glslang/spirv-opt from promoting it
// to SSA, so it stays as a Function-storage OpVariable.
TEST_F(NegativeGpuAVArrayOob, FunctionStorageArrayOob) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        void main() {
            uint arr[4];
            arr[0] = 1;
            arr[1] = 2;
            arr[2] = 3;
            arr[3] = 4;
            uint i = ssbo.data[0] + 4;
            arr[i] = 99;
            ssbo.data[0] = arr[ssbo.data[0] & 3u];
        }
    )glsl";

    TestHelper(shader_source, 4, 4);
}

// File-scope global array, which glslang lowers to Private storage.
TEST_F(NegativeGpuAVArrayOob, PrivateStorageArrayOob) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        uint privateArr[4];
        void main() {
            privateArr[0] = 1;
            privateArr[1] = 2;
            privateArr[2] = 3;
            privateArr[3] = 4;
            uint i = ssbo.data[0] + 4;
            privateArr[i] = 99;
            ssbo.data[0] = privateArr[ssbo.data[0] & 3u];
        }
    )glsl";

    TestHelper(shader_source, 4, 4);
}

// Verify the host logger prints the variable name in the error message for each tracked
// storage class.
TEST_F(NegativeGpuAVArrayOob, VariableNameInMessageWorkgroup) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint workgroupArr[4];
        void main() {
            workgroupArr[ssbo.data[0] + 4] = 0;
        }
    )glsl";

    TestHelper(shader_source, 4, 4, "SPIRV-ArrayOob-OpAccessChain", SPV_SOURCE_GLSL, nullptr, "workgroupArr");
}

TEST_F(NegativeGpuAVArrayOob, VariableNameInMessagePrivate) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        uint privateArr[4];
        void main() {
            privateArr[ssbo.data[0] + 4] = 0;
        }
    )glsl";

    TestHelper(shader_source, 4, 4, "SPIRV-ArrayOob-OpAccessChain", SPV_SOURCE_GLSL, nullptr, "privateArr");
}

TEST_F(NegativeGpuAVArrayOob, VariableNameInMessageFunction) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        void main() {
            uint functionArr[4];
            functionArr[0] = 1;
            functionArr[1] = 2;
            functionArr[2] = 3;
            functionArr[3] = 4;
            functionArr[ssbo.data[0] + 4] = 99;
            ssbo.data[0] = functionArr[ssbo.data[0] & 3u];
        }
    )glsl";

    TestHelper(shader_source, 4, 4, "SPIRV-ArrayOob-OpAccessChain", SPV_SOURCE_GLSL, nullptr, "functionArr");
}
