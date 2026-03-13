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

void GpuAVSharedMemoryOobTest::InitSharedMemoryOob() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework(
        {{OBJECT_LAYER_NAME, "gpuav_shared_memory_data_race", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkFalse}}, false));
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
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 4) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared vec4 v;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                v = vec4(1.0, 2.0, 3.0, 4.0);
            }
            barrier();
            float x = v[gl_LocalInvocationIndex % 4];
            ssbo.data[gl_LocalInvocationIndex] = floatBitsToUint(x);
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

TEST_F(PositiveGpuAVSharedMemoryOob, VectorInsertInBounds) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 4) in;
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } ssbo;
        shared vec4 v;
        void main() {
            v[gl_LocalInvocationIndex % 4] = float(gl_LocalInvocationIndex);
            barrier();
            ssbo.data[gl_LocalInvocationIndex] = floatBitsToUint(v[gl_LocalInvocationIndex % 4]);
        }
    )glsl";

    TestHelper(shader_source);
}
