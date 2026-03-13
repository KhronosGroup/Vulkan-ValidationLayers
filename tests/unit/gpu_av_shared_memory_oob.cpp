/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
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

class NegativeGpuAVSharedMemoryOob : public GpuAVSharedMemoryOobTest {
  protected:
    void TestHelper(const char* source, uint32_t count = 1, const char* error = "SPIRV-SharedMemoryOob");
};

void NegativeGpuAVSharedMemoryOob::TestHelper(const char* shader_source, uint32_t count, const char* error) {
    RETURN_IF_SKIP(InitSharedMemoryOob());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
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

    m_errorMonitor->SetDesiredError(error, count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVSharedMemoryOob, Simple1DArray) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared uint arr[4];
        void main() {
            uint i = ssbo.data[0] + 4;
            arr[i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, Array1DDynamic) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint idx; } ssbo;
        shared uint arr[4];
        void main() {
            uint i = ssbo.idx + 4;
            arr[i] = 0;
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, Array2DInner) {
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

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, Array2DOuter) {
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

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, StructWithArray) {
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

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, ArrayOfStructs) {
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

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, VectorExtractDynamic) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared vec4 v;
        void main() {
            v = vec4(1.0);
            uint i = ssbo.data[0] + 4;
            float x = v[i];
            ssbo.data[1] = floatBitsToUint(x);
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, VectorInsertDynamic) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared vec4 v;
        void main() {
            v = vec4(1.0);
            uint i = ssbo.data[0] + 4;
            v[i] = 2.0;
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryOob, SlangNestedStruct) {
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
            uint i = outputBuffer[0] + 2;
            temp.z[i].b = 0;
            outputBuffer[groupThreadID.x] = temp.z[groupThreadID.x].b;
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

    m_errorMonitor->SetDesiredError("SPIRV-SharedMemoryOob", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVSharedMemoryOob, MeshShader) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    RETURN_IF_SKIP(InitGpuAvFramework(
        {{OBJECT_LAYER_NAME, "gpuav_shared_memory_data_race", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkFalse}}, false));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
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

    m_errorMonitor->SetDesiredError("SPIRV-SharedMemoryOob", 1);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
