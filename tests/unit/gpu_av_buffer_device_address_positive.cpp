/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "generated/vk_function_pointers.h"

void GpuAVBufferDeviceAddressTest::InitGpuVUBufferDeviceAddress(bool safe_mode) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework({}, safe_mode));
    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVBufferDeviceAddress : public GpuAVBufferDeviceAddressTest {};

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreStd140) {
    TEST_DESCRIPTION("Makes sure that writing to a buffer that was created after command buffer record doesn't get OOB error");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
        layout(set = 0, binding = 0) uniform ufoo {
            bufStruct data;
            int nWrites;
        } u_info;
        layout(buffer_reference, std140) buffer bufStruct {
            int a[4];
        };
        void main() {
            for (int i=0; i < u_info.nWrites; ++i) {
                u_info.data.a[i] = 42;
            }
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Make another buffer to write to
    const uint32_t storage_buffer_size = 16 * 4;
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();
    uniform_buffer_ptr[1] = 4;

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*storage_buffer_ptr, 42);
        storage_buffer_ptr += 4;
    }
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreStd140NumerousAddressRanges) {
    TEST_DESCRIPTION(
        "Makes sure that writing to a buffer that was created after command buffer record doesn't get OOB error, even when there "
        "are numerous valid address ranges");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
        layout(set = 0, binding = 0) uniform ufoo {
            bufStruct data;
            int nWrites;
        } u_info;
        layout(buffer_reference, std140) buffer bufStruct {
            int a[4];
        };
        void main() {
            for (int i=0; i < u_info.nWrites; ++i) {
                u_info.data.a[i] = 42;
            }
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Make another buffer to write to
    const uint32_t storage_buffer_size = 16 * 4;
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    // Create storage buffers for the sake of storing multiple device address ranges
    std::vector<vkt::Buffer> dummy_storage_buffers;
    for (int i = 0; i < 1024; ++i) {
        (void)dummy_storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address).Address();
    }

    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();
    uniform_buffer_ptr[1] = 4;

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*storage_buffer_ptr, 42);
        storage_buffer_ptr += 4;
    }
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreStd430) {
    TEST_DESCRIPTION("Makes sure that writing to a buffer that was created after command buffer record doesn't get OOB error");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
        layout(set = 0, binding = 0) uniform ufoo {
            bufStruct data;
            int nWrites;
        } u_info;
        layout(buffer_reference, std430) buffer bufStruct {
            int a[4];
        };
        void main() {
            for (int i=0; i < u_info.nWrites; ++i) {
                u_info.data.a[i] = 42;
            }
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Make another buffer to write to
    const uint32_t storage_buffer_size = 4 * 4;
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();
    uniform_buffer_ptr[1] = 4;

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(storage_buffer_ptr[i], 42);
    }
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreExplicitOffset) {
    TEST_DESCRIPTION("Do a OpStore to a PhysicalStorageBuffer");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, buffer_reference_align = 16) buffer bdaStruct;

        layout(set = 0, binding = 0) buffer foo {
            bdaStruct data;
        } in_buffer;

        layout(buffer_reference, std140) buffer bdaStruct {
            layout(offset = 0) int a[2];
            layout(offset = 32) int b;
        };

        void main() {
            in_buffer.data.b = 0xca7;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);

    VkDeviceAddress buffer_ptr = bda_buffer.Address();
    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &buffer_ptr, sizeof(VkDeviceAddress));

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    uint8_t *bda_buffer_ptr = (uint8_t *)bda_buffer.Memory().Map();
    uint32_t output = *((uint32_t *)(bda_buffer_ptr + 32));
    ASSERT_TRUE(output == 0xca7);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StructLoad) {
    TEST_DESCRIPTION("Do a OpLoad through a struct PhysicalStorageBuffer");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_ARB_gpu_shader_int64 : enable

        struct Test {
            float a;
        };

        layout(buffer_reference, std430, buffer_reference_align = 16) buffer TestBuffer {
            Test test;
        };

        Test GetTest(uint64_t ptr) {
            return TestBuffer(ptr).test;
        }

        layout(set = 0, binding = 0) buffer foo {
            TestBuffer data;
            float x;
        } in_buffer;

        void main() {
            in_buffer.x = GetTest(uint64_t(in_buffer.data)).a;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);

    float expected_output = 0x00EEAADD;
    uint8_t *block_buffer_ptr = (uint8_t *)block_buffer.Memory().Map();
    memcpy(block_buffer_ptr, &expected_output, sizeof(float));

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress block_ptr = block_buffer.Address();

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float output = *((float *)(in_buffer_ptr + sizeof(VkDeviceAddress)));
    ASSERT_TRUE(output == expected_output);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7797
// Emitting an OOB access error when it should not
TEST_F(PositiveGpuAVBufferDeviceAddress, StructLoadPadded) {
    TEST_DESCRIPTION("Do a OpLoad through a padded struct PhysicalStorageBuffer");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_ARB_gpu_shader_int64 : enable

        struct Test {
        uvec3    pad_1; // Offset 0 Size 12
        uint64_t pad_2; // Offset 16 Size 8 (alignment requirement)
        float    a;     // Offset 24 Size 4
        }; // Total Size 28

        layout(buffer_reference, std430, buffer_reference_align = 16) buffer TestBuffer {
            Test test;
        };

        float GetTest(uint64_t ptr) {
            return TestBuffer(ptr).test.a;
        }

        layout(set = 0, binding = 0) buffer foo {
            TestBuffer data;
            float x;
        } in_buffer;

        void main() {
            in_buffer.x = GetTest(uint64_t(in_buffer.data));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 32, 0, vkt::device_address);

    float expected_output = 0x00EEAADD;
    uint8_t *block_buffer_ptr = (uint8_t *)block_buffer.Memory().Map();
    memcpy(block_buffer_ptr + 24, &expected_output, sizeof(float));

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress block_ptr = block_buffer.Address();

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float output = *((float *)(in_buffer_ptr + sizeof(VkDeviceAddress)));
    ASSERT_TRUE(output == expected_output);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, UVec3Array) {
    SetTargetApiVersion(VK_API_VERSION_1_2);  // need to use 12Feature struct
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_scalar_block_layout : enable

        layout(buffer_reference, std430, scalar) readonly buffer IndexBuffer {
            uvec3 indices[]; // array stride is 12 in scalar
        };

        layout(set = 0, binding = 0) uniform foo {
            IndexBuffer data;
            int nReads;
        } in_buffer;

        void main() {
            uvec3 readvec;
            for (int i=0; i < in_buffer.nReads; ++i) {
                readvec = in_buffer.data.indices[i];
            }
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    // Hold 4 indices
    vkt::Buffer block_buffer(*m_device, 48, 0, vkt::device_address);

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress block_ptr = block_buffer.Address();
    const uint32_t n_reads = 4;  // uvec3[0] to uvec3[3]

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    memcpy(in_buffer_ptr + sizeof(VkDeviceAddress), &n_reads, sizeof(uint32_t));

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, ArrayOfStruct) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(std430, buffer_reference) buffer T1 {
            int a;
        } block_buffer;

        struct Foo {
            T1 b;
        };

        layout(set=0, binding=0) buffer storage_buffer {
            uint index;
            // Offset is 8
            Foo f[]; // each item is 8 bytes
        } foo;

        void main() {
            Foo new_foo = foo.f[foo.index];
            new_foo.b.a = 2;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 32, 0, vkt::device_address);
    VkDeviceAddress block_ptr = block_buffer.Address();

    vkt::Buffer storage_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    uint8_t *buffer_ptr = (uint8_t *)storage_buffer.Memory().Map();
    const uint32_t index = 0;
    memcpy(buffer_ptr, &index, sizeof(uint32_t));
    memcpy(buffer_ptr + (1 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (2 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (3 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));

    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, BitCastUvec2) {
    TEST_DESCRIPTION("test loading and storing with GL_EXT_buffer_reference_uvec2");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_buffer_reference_uvec2 : enable

        layout(buffer_reference, std430) buffer NodeA {
            int a;
        };

        layout(buffer_reference, std430) buffer NodeB {
            int b;
        };

        layout(set = 0, binding = 0) buffer Buffer {
            uvec2 nodes[2];
        } in_buffer;

        void main() {
            NodeA(in_buffer.nodes[0]).a = NodeB(in_buffer.nodes[1]).b;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer buffer_node_a(*m_device, 4, 0, vkt::device_address);
    vkt::Buffer buffer_node_b(*m_device, 4, 0, vkt::device_address);
    VkDeviceAddress block_a_ptr = buffer_node_a.Address();
    VkDeviceAddress block_b_ptr = buffer_node_b.Address();

    auto *buffer_ptr = static_cast<uint32_t *>(buffer_node_b.Memory().Map());
    *buffer_ptr = 1234;  // data to pass

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &block_a_ptr, sizeof(VkDeviceAddress));
    memcpy(in_buffer_ptr + sizeof(VkDeviceAddress), &block_b_ptr, sizeof(VkDeviceAddress));

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    buffer_ptr = static_cast<uint32_t *>(buffer_node_a.Memory().Map());
    ASSERT_TRUE(*buffer_ptr == 1234);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreRelaxedBlockLayout) {
    TEST_DESCRIPTION("No false OOB detected - use VK_KHR_relaxed_block_layout");
    AddRequiredExtensions(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // #version 450
    // #extension GL_EXT_buffer_reference : enable
    // layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
    // layout(set = 0, binding = 0) uniform ufoo { bufStruct ptr; }
    // ssbo;
    //
    // layout(buffer_reference, std430) buffer bufStruct {
    //     float f;
    //     vec3 v;
    // };
    // void main() {
    //     ssbo.ptr.f = 42.0;
    //     ssbo.ptr.v = uvec3(1.0, 2.0, 3.0);
    // }
    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_buffer_reference"
               OpName %main "main"
               OpName %ufoo "ufoo"
               OpMemberName %ufoo 0 "ptr"
               OpName %bufStruct "bufStruct"
               OpMemberName %bufStruct 0 "f"
               OpMemberName %bufStruct 1 "v"
               OpName %ssbo "ssbo"
               OpMemberDecorate %ufoo 0 Offset 0
               OpDecorate %ufoo Block
               OpMemberDecorate %bufStruct 0 Offset 0
               OpMemberDecorate %bufStruct 1 Offset 4
               OpDecorate %bufStruct Block
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_bufStruct PhysicalStorageBuffer
       %ufoo = OpTypeStruct %_ptr_PhysicalStorageBuffer_bufStruct
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
  %bufStruct = OpTypeStruct %float %v3float
%_ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer PhysicalStorageBuffer %bufStruct
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
       %ssbo = OpVariable %_ptr_Uniform_ufoo Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_bufStruct
   %float_42 = OpConstant %float 42
%_ptr_PhysicalStorageBuffer_float = OpTypePointer PhysicalStorageBuffer %float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %27 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%_ptr_PhysicalStorageBuffer_v3float = OpTypePointer PhysicalStorageBuffer %v3float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %ssbo %int_0
         %17 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %16
         %20 = OpAccessChain %_ptr_PhysicalStorageBuffer_float %17 %int_0
               OpStore %20 %float_42 Aligned 16
         %21 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %ssbo %int_0
         %22 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %21
         %29 = OpAccessChain %_ptr_PhysicalStorageBuffer_v3float %22 %int_1
               OpStore %29 %27 Aligned 4
               OpReturn
               OpFunctionEnd
    )";

    // Make a uniform buffer to be passed to the shader that contains the pointer
    const uint32_t uniform_buffer_size = 8;  // 64 bits pointer
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ =
        std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    const uint32_t storage_buffer_size = 4 * sizeof(float);  // float + vec3
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote to float and vec3
    auto *storage_buffer_ptr = static_cast<float *>(storage_buffer.Memory().Map());
    ASSERT_EQ(storage_buffer_ptr[0], 42.0f);
    ASSERT_EQ(storage_buffer_ptr[1], 1.0f);
    ASSERT_EQ(storage_buffer_ptr[2], 2.0f);
    ASSERT_EQ(storage_buffer_ptr[3], 3.0f);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreScalarBlockLayout) {
    TEST_DESCRIPTION("No false OOB detected - use VK_EXT_scalar_block_layout");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);

    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_scalar_block_layout : enable
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
        layout(set = 0, binding = 0) uniform ufoo {
            bufStruct ptr;
        } ssbo;

        layout(buffer_reference, scalar) buffer bufStruct {
            float f;
            vec3 v;
        };
        void main() {
            ssbo.ptr.f = 42.0;
            ssbo.ptr.v = uvec3(1.0, 2.0, 3.0);
        }
    )glsl";

    // Make a uniform buffer to be passed to the shader that contains the pointer
    const uint32_t uniform_buffer_size = 8;  // 64 bits pointer
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    const uint32_t storage_buffer_size = 4 * sizeof(float);  // float + vec3
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote to float and vec3
    auto *storage_buffer_ptr = static_cast<float *>(storage_buffer.Memory().Map());
    ASSERT_EQ(storage_buffer_ptr[0], 42.0f);
    ASSERT_EQ(storage_buffer_ptr[1], 1.0f);
    ASSERT_EQ(storage_buffer_ptr[2], 2.0f);
    ASSERT_EQ(storage_buffer_ptr[3], 3.0f);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, StoreStd430LinkedList) {
    TEST_DESCRIPTION("No false OOB accesses detected in a linked list");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 3 * sizeof(VkDeviceAddress);
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer Node;
        layout(buffer_reference, std430) buffer Node {
            vec3 v;
            Node next;
        };

        layout(set = 0, binding = 0) uniform foo {
            Node node_0;
            Node node_1;
            Node node_2;
        };

        void main() {
            node_0.next = node_1;
            node_1.next = node_2;

            node_0.v = vec3(1.0, 2.0, 3.0);
            node_0.next.v = vec3(4.0, 5.0, 6.0);
            node_0.next.next.v = vec3(7.0, 8.0, 9.0);
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // Make a list of storage buffers, each one holding a Node
    constexpr size_t nodes_count = 3;
    const uint32_t storage_buffer_size = (4 * sizeof(float)) + sizeof(VkDeviceAddress);
    std::vector<vkt::Buffer> storage_buffers;
    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());

    for (size_t i = 0; i < nodes_count; ++i) {
        const VkDeviceAddress addr = storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address).Address();
        uniform_buffer_ptr[i] = addr;
    }

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote values to all nodes
    for (auto [buffer_i, buffer] : vvl::enumerate(storage_buffers)) {
        auto storage_buffer_ptr = static_cast<float *>(buffer.Memory().Map());

        ASSERT_EQ(storage_buffer_ptr[0], float(3 * buffer_i + 1));
        ASSERT_EQ(storage_buffer_ptr[1], float(3 * buffer_i + 2));
        ASSERT_EQ(storage_buffer_ptr[2], float(3 * buffer_i + 3));
    }
}

TEST_F(PositiveGpuAVBufferDeviceAddress, MultipleBufferReferenceBlocks) {
    TEST_DESCRIPTION("No false OOB detected - store & load");

    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) buffer Foo {
            vec3 v;
            int i;
            float f;
        };

        layout(buffer_reference, std430) buffer Bar {
            int i;
            float f;
            vec3 v;
        };

        layout(set = 0, binding = 0) uniform Buffer {
            Foo foo;
            Bar bar;
        };

        void main() {
            bar.i = 42;
            foo.i = bar.i;
        }
    )glsl";

    // Make a uniform buffer to be passed to the shader that contains the pointer
    const uint32_t uniform_buffer_size = 2 * sizeof(VkDeviceAddress);  // 64 bits pointer
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    const uint32_t foo_storage_buffer_size = 4 * sizeof(float) + sizeof(float) + sizeof(int32_t);
    vkt::Buffer foo_storage_buffer(*m_device, foo_storage_buffer_size, 0, vkt::device_address);

    const uint32_t bar_storage_buffer_size = sizeof(float) + sizeof(int32_t) + 4 * sizeof(float);
    vkt::Buffer bar_storage_buffer(*m_device, bar_storage_buffer_size, 0, vkt::device_address);

    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = foo_storage_buffer.Address();
    uniform_buffer_ptr[1] = bar_storage_buffer.Address();

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote to float and vec3
    auto *foo_ptr = static_cast<int *>(foo_storage_buffer.Memory().Map());
    auto *bar_ptr = static_cast<int *>(bar_storage_buffer.Memory().Map());
    ASSERT_EQ(bar_ptr[0], 42);
    ASSERT_EQ(foo_ptr[3], bar_ptr[0]);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, LoadStoreStruct) {
    TEST_DESCRIPTION("No false OOB detected when using a struct");

    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_scalar_block_layout : enable
        #extension GL_EXT_buffer_reference : enable

        struct Vertex {
          float x, y, z;
          float r, g, b;
          vec2 uv;
        };

        layout(std430, buffer_reference) buffer VertexBuffer {
          Vertex vertices[];
        };

        layout(set = 0, binding = 0) uniform foo {
            VertexBuffer vb;
        } ssbo;

        void main() {
            ssbo.vb.vertices[1] = ssbo.vb.vertices[0];
            ssbo.vb.vertices[2] = ssbo.vb.vertices[1];
        }
    )glsl";

    // Make a uniform buffer to be passed to the shader that contains the pointer
    const uint32_t uniform_buffer_size = 8;  // 64 bits pointer
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    struct Vertex {
        float x, y, z;
        float r, g, b;
        float uv[2];
    };

    const uint32_t storage_buffer_size = 3 * sizeof(Vertex);  // float + vec3
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    // Write vertex 0
    auto vertex_buffer_ptr = static_cast<Vertex *>(storage_buffer.Memory().Map());
    vertex_buffer_ptr[0].x = 1.0f;
    vertex_buffer_ptr[0].y = 2.0f;
    vertex_buffer_ptr[0].z = 3.0f;

    vertex_buffer_ptr[0].r = 4.0f;
    vertex_buffer_ptr[0].g = 5.0f;
    vertex_buffer_ptr[0].b = 6.0f;

    vertex_buffer_ptr[0].uv[0] = 7.0f;
    vertex_buffer_ptr[0].uv[1] = 8.0f;

    auto data = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    data[0] = storage_buffer.Address();

    m_default_queue->SubmitAndWait(m_command_buffer);

    // Make sure shader wrote to float and vec3
    ASSERT_EQ(vertex_buffer_ptr[0].x, 1.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].y, 2.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].z, 3.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].r, 4.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].g, 5.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].b, 6.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].uv[0], 7.0f);
    ASSERT_EQ(vertex_buffer_ptr[0].uv[1], 8.0f);

    ASSERT_EQ(vertex_buffer_ptr[1].x, 1.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].y, 2.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].z, 3.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].r, 4.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].g, 5.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].b, 6.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].uv[0], 7.0f);
    ASSERT_EQ(vertex_buffer_ptr[1].uv[1], 8.0f);

    ASSERT_EQ(vertex_buffer_ptr[2].x, 1.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].y, 2.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].z, 3.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].r, 4.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].g, 5.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].b, 6.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].uv[0], 7.0f);
    ASSERT_EQ(vertex_buffer_ptr[2].uv[1], 8.0f);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, ConcurrentAccessesToBdaBuffer) {
    TEST_DESCRIPTION(
        "Make sure BDA buffer maintained in GPU-AV is correctly read/written to. When this buffer was not maintained per command "
        "buffer, and a global buffer was used instead, concurrent accesses were not handled correctly.");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, buffer_reference_align = 16, std430) buffer IntPtr {
            int i0;
            int i1;
        };

        layout(push_constant) uniform Uniforms {
            IntPtr ptr;
        };

        void main() {
            ptr.i1 = ptr.i0;
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    VkPushConstantRange pc;
    pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pc.offset = 0;
    pc.size = sizeof(VkDeviceAddress);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.pipeline_layout_ci_.pushConstantRangeCount = 1;
    pipe.pipeline_layout_ci_.pPushConstantRanges = &pc;
    pipe.CreateGraphicsPipeline();

    const uint32_t storage_buffer_size = 2 * sizeof(int);
    std::vector<vkt::CommandBuffer> cmd_buffers;
    std::vector<vkt::Buffer> storage_buffers;
    for (int i = 0; i < 64; ++i) {
        auto &cb = cmd_buffers.emplace_back(*m_device, m_command_pool);

        // Create a storage buffer and get its address,
        // effectively adding it to the BDA table
        auto &storage_buffer = storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address);

        auto storage_buffer_addr = storage_buffer.Address();

        // Read and write from storage buffer address
        cb.Begin();
        cb.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdPushConstants(cb, pipe.pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(storage_buffer_addr),
                             &storage_buffer_addr);
        vk::CmdDraw(cb, 3, 1, 0, 0);
        cb.EndRenderPass();
        cb.End();

        m_default_queue->Submit(cb);
    }

    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVBufferDeviceAddress, ProxyStructLoad) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8073");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_scalar_block_layout : require
        #extension GL_EXT_buffer_reference2 : require

        struct RealCamera {
            mat4 viewProjection; // [0, 63]
            vec4 frustum[6]; // [64, 160] - should not be factored if not accessing
        };
        layout(buffer_reference, scalar, buffer_reference_align = 4) restrict readonly buffer CameraBuffer {
            RealCamera camera;
        };

        layout(binding = 0, set = 0) buffer OutData {
            CameraBuffer cameraBuffer;
            mat4 out_mat;
        };

        void main() {
            restrict const RealCamera camera = cameraBuffer.camera;
            out_mat = camera.viewProjection;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 160, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress buffer_ptr = bda_buffer.Address();
    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &buffer_ptr, sizeof(VkDeviceAddress));

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, ProxyStructLoadLinkedList) {
    TEST_DESCRIPTION("Make sure we don't get in an infinite loop searching for BDA length");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_ARB_gpu_shader_int64 : enable

        layout(buffer_reference) buffer Node;
        layout(buffer_reference, std430) buffer Node {
            uint payload;
            Node next;
            uint64_t next2;
        };

        layout(set = 0, binding = 0) buffer foo {
            Node node_0;
            Node node_1;
        };

        void main() {
            node_0 = node_1;
            node_0.next = Node(node_1.next2);
            node_0.next.payload = 3;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveGpuAVBufferDeviceAddress, ProxyStructUnsafe) {
    TEST_DESCRIPTION("Make sure the range for a struct load is the whole struct.");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress(false));

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_buffer_reference2 : require

        struct RealCamera {
            mat4 viewProjection;
            vec4 frustum[6];
        };
        layout(buffer_reference) restrict readonly buffer CameraBuffer {
            RealCamera camera;
        };

        layout(binding = 0, set = 0) buffer OutData {
            CameraBuffer cameraBuffer;
            mat4 out_mat;
        };

        layout(buffer_reference) buffer PerFrame;
        layout(buffer_reference, std430) readonly buffer PerFrame {
            mat4 proj;
            uint texSkyboxRadiance;
            uint texSkyboxIrradiance;
            uint sampler0;
            uint texture0;
        };

        layout(push_constant, std430) uniform constants {
            PerFrame perFrame;
        } pc;

        void main() {
            uint a = pc.perFrame.texSkyboxRadiance;
            uint b = pc.perFrame.sampler0;

            restrict const RealCamera camera = cameraBuffer.camera;
            out_mat = camera.viewProjection;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress)};
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {pc_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 256, 0, vkt::device_address);
    vkt::Buffer ssbo_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress bda_buffer_addr = bda_buffer.Address();
    uint8_t *ssbo_buffer_ptr = (uint8_t *)ssbo_buffer.Memory().Map();
    memcpy(ssbo_buffer_ptr, &bda_buffer_addr, sizeof(VkDeviceAddress));

    descriptor_set.WriteDescriptorBufferInfo(0, ssbo_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress),
                         &bda_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, BasicRangeUnsafe) {
    TEST_DESCRIPTION("Simple test to examine how we do the range check in unsafe mode.");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress(false));

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_buffer_reference2 : require

        struct Foo {
            uvec4 x;
        };

        layout(buffer_reference, std430) buffer BDA {
            mat4 a;
            uint b[16];
            uint c;
            Foo d;
            uint e[16];
        };

        layout(push_constant, std430) uniform constants {
            BDA bda;
        };

        void bar() {
            // inst_buffer_device_address_range is 20
            bda.d.x = uvec4(0) * uvec4(bda.e[0]);
        }

        void main() {
            // inst_buffer_device_address_range is 36
            bda.b[1] = bda.b[9];
            bar();
        }
    )glsl";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress)};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {pc_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 256, 0, vkt::device_address);
    VkDeviceAddress bda_buffer_addr = bda_buffer.Address();

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress),
                         &bda_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, NonStructPointer) {
    TEST_DESCRIPTION("Slang allows BDA pointers to be with POD instead of a struct");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Slang code
    // uniform uint* data_ptr;
    // [numthreads(1,1,1)]
    // void computeMain() {
    //    data_ptr[2] = 999;
    // }
    char const *shader_source = R"(
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %computeMain "main" %globalParams
               OpExecutionMode %computeMain LocalSize 1 1 1
               OpDecorate %_ptr_PhysicalStorageBuffer_uint ArrayStride 4
               OpDecorate %GlobalParams_std140 Block
               OpMemberDecorate %GlobalParams_std140 0 Offset 0
               OpDecorate %globalParams Binding 0
               OpDecorate %globalParams DescriptorSet 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
   %uint_999 = OpConstant %uint 999
         %12 = OpTypeFunction %void
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
%GlobalParams_std140 = OpTypeStruct %_ptr_PhysicalStorageBuffer_uint
%_ptr_Uniform_GlobalParams_std140 = OpTypePointer Uniform %GlobalParams_std140
%_ptr_Uniform__ptr_PhysicalStorageBuffer_uint = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_uint
%globalParams = OpVariable %_ptr_Uniform_GlobalParams_std140 Uniform
%computeMain = OpFunction %void None %12
         %13 = OpLabel
         %35 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_uint %globalParams %int_0
         %36 = OpLoad %_ptr_PhysicalStorageBuffer_uint %35
         %37 = OpPtrAccessChain %_ptr_PhysicalStorageBuffer_uint %36 %int_2
               OpStore %37 %uint_999 Aligned 4
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 256, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = block_buffer.Address();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    auto block_buffer_ptr = static_cast<uint32_t *>(block_buffer.Memory().Map());
    ASSERT_TRUE(block_buffer_ptr[2] == 999);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, MultipleAccessChains) {
    TEST_DESCRIPTION("Slang will produce a chain of OpAccessChains");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Slang code
    // struct Data {
    //     uint x;
    //     uint payload[16]; // last item is OOB
    // }
    // uniform Data* bda;
    // [numthreads(1,1,1)]
    // void computeMain() {
    //    uint a = bda->payload[0] * bda->payload[14];
    //    bda->x = a;
    // }
    char const *shader_source = R"(
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %computeMain "main" %globalParams
               OpExecutionMode %computeMain LocalSize 1 1 1
               OpDecorate %_ptr_PhysicalStorageBuffer_Data_natural ArrayStride 68
               OpDecorate %GlobalParams_std140 Block
               OpMemberDecorate %GlobalParams_std140 0 Offset 0
               OpDecorate %globalParams Binding 0
               OpDecorate %globalParams DescriptorSet 0
               OpDecorate %_ptr_PhysicalStorageBuffer__Array_natural_uint16 ArrayStride 64
               OpDecorate %_arr_uint_int_16 ArrayStride 4
               OpDecorate %_ptr_PhysicalStorageBuffer__arr_uint_int_16 ArrayStride 64
               OpDecorate %_ptr_PhysicalStorageBuffer_uint ArrayStride 4
               OpMemberDecorate %_Array_natural_uint16 0 Offset 0
               OpMemberDecorate %Data_natural 0 Offset 0
               OpMemberDecorate %Data_natural 1 Offset 4
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_Data_natural PhysicalStorageBuffer
%GlobalParams_std140 = OpTypeStruct %_ptr_PhysicalStorageBuffer_Data_natural
%_ptr_Uniform_GlobalParams_std140 = OpTypePointer Uniform %GlobalParams_std140
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_7 = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_Data_natural
      %int_1 = OpConstant %int 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer__Array_natural_uint16 PhysicalStorageBuffer
       %uint = OpTypeInt 32 0
     %int_16 = OpConstant %int 16
%_arr_uint_int_16 = OpTypeArray %uint %int_16
%_ptr_PhysicalStorageBuffer__arr_uint_int_16 = OpTypePointer PhysicalStorageBuffer %_arr_uint_int_16
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
     %int_14 = OpConstant %int 14
%_Array_natural_uint16 = OpTypeStruct %_arr_uint_int_16
%Data_natural = OpTypeStruct %uint %_Array_natural_uint16
%_ptr_PhysicalStorageBuffer_Data_natural = OpTypePointer PhysicalStorageBuffer %Data_natural
%_ptr_PhysicalStorageBuffer__Array_natural_uint16 = OpTypePointer PhysicalStorageBuffer %_Array_natural_uint16
%globalParams = OpVariable %_ptr_Uniform_GlobalParams_std140 Uniform
%computeMain = OpFunction %void None %3
          %4 = OpLabel
         %13 = OpAccessChain %_ptr_Uniform_7 %globalParams %int_0
         %14 = OpLoad %_ptr_PhysicalStorageBuffer_Data_natural %13
         %18 = OpAccessChain %_ptr_PhysicalStorageBuffer__Array_natural_uint16 %14 %int_1
         %23 = OpAccessChain %_ptr_PhysicalStorageBuffer__arr_uint_int_16 %18 %int_0
         %25 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %23 %int_0
         %26 = OpLoad %uint %25 Aligned 4
         %27 = OpLoad %_ptr_PhysicalStorageBuffer_Data_natural %13
         %28 = OpAccessChain %_ptr_PhysicalStorageBuffer__Array_natural_uint16 %27 %int_1
         %29 = OpAccessChain %_ptr_PhysicalStorageBuffer__arr_uint_int_16 %28 %int_0
         %30 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %29 %int_14
         %32 = OpLoad %uint %30 Aligned 4
          %a = OpIMul %uint %26 %32
         %34 = OpLoad %_ptr_PhysicalStorageBuffer_Data_natural %13
         %35 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %34 %int_0
               OpStore %35 %a Aligned 4
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ubo_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    auto ubo_buffer_ptr = static_cast<VkDeviceAddress *>(ubo_buffer.Memory().Map());
    ubo_buffer_ptr[0] = bda_buffer.Address();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, ubo_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, AtomicExchangeSlang) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // struct Foo {
    //     uint pad_0;
    //     float3 pad_1;
    //     uint* a; // offset 16
    //     uint* b; // offset 24
    // }
    //
    // RWStructuredBuffer<uint64_t> rwNodeBuffer;
    //
    // [shader("compute")]
    // void main(uint3 threadId : SV_DispatchThreadID) {
    //     Foo* x = (Foo*)(rwNodeBuffer[0]);
    //     Foo foo = *x;
    //     InterlockedExchange(*foo.a, 0);
    //     InterlockedExchange(*(x->b), 0);
    // }
    char const *shader_source = R"(
               OpCapability Int64
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %rwNodeBuffer
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_runtimearr_ulong ArrayStride 8
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %rwNodeBuffer Binding 0
               OpDecorate %rwNodeBuffer DescriptorSet 0
               OpDecorate %_ptr_PhysicalStorageBuffer_Foo_natural ArrayStride 32
               OpDecorate %_ptr_PhysicalStorageBuffer_uint ArrayStride 4
               OpDecorate %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_uint ArrayStride 8
               OpMemberDecorate %Foo_natural 0 Offset 0
               OpMemberDecorate %Foo_natural 1 Offset 4
               OpMemberDecorate %Foo_natural 2 Offset 16
               OpMemberDecorate %Foo_natural 3 Offset 24
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %ulong = OpTypeInt 64 0
%_ptr_StorageBuffer_ulong = OpTypePointer StorageBuffer %ulong
%_runtimearr_ulong = OpTypeRuntimeArray %ulong
%RWStructuredBuffer = OpTypeStruct %_runtimearr_ulong
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_Foo_natural PhysicalStorageBuffer
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
%_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %_ptr_PhysicalStorageBuffer_uint
      %int_3 = OpConstant %int 3
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%Foo_natural = OpTypeStruct %uint %v3float %_ptr_PhysicalStorageBuffer_uint %_ptr_PhysicalStorageBuffer_uint
%_ptr_PhysicalStorageBuffer_Foo_natural = OpTypePointer PhysicalStorageBuffer %Foo_natural
%rwNodeBuffer = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
          %9 = OpAccessChain %_ptr_StorageBuffer_ulong %rwNodeBuffer %int_0 %int_0
         %14 = OpLoad %ulong %9
          %x = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_Foo_natural %14
         %22 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_uint %x %int_2
         %23 = OpLoad %_ptr_PhysicalStorageBuffer_uint %22 Aligned 8
         %25 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_uint %x %int_3
         %28 = OpAtomicExchange %uint %23 %uint_1 %uint_0 %uint_0
         %29 = OpLoad %_ptr_PhysicalStorageBuffer_uint %25 Aligned 8
         %30 = OpAtomicExchange %uint %29 %uint_1 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer pod_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer bda_buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = bda_buffer.Address();

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    // Update indirect pointers here
    VkDeviceAddress pod_buffer_addr = pod_buffer.Address();
    vk::CmdUpdateBuffer(m_command_buffer, bda_buffer, 16, sizeof(VkDeviceAddress), &pod_buffer_addr);
    vk::CmdUpdateBuffer(m_command_buffer, bda_buffer, 24, sizeof(VkDeviceAddress), &pod_buffer_addr);
    m_command_buffer.FullMemoryBarrier();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, MemoryModelOperand) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9018");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) buffer blockType {
            uint x[];
        };

        layout(set = 0, binding = 0, std430) buffer t2 {
            blockType node;
        };

        void main() {
            // Will produce a MakePointerAvailable next to the Aligned operand
            coherent blockType b = node;
            b.x[4] = 2;

            // will have a Volatile operand instead of Aligned
            volatile blockType b2 = node;
            b2.x[8] = 3;
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 128, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = bda_buffer.Address();

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, MemoryModelOperand2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9018");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #pragma use_vulkan_memory_model
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_buffer_reference : enable

        shared bool sharedSkip;
        layout(buffer_reference) buffer Node { uint x[]; };
        layout(set=0, binding=0) buffer SSBO {
            Node node;
            uint a;
            uint b;
        };

        void main() {
            bool skip = false;
            sharedSkip = false;

            if (a == 0) {
                skip = atomicLoad(node.x[0], gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsAcquire | gl_SemanticsMakeVisible) == 0;
                // will have a MakePointerVisible|NonPrivatePointer operand on OpStore
                sharedSkip = skip;
            }
            // will have a MakePointerVisible|NonPrivatePointer operand on OpLoad
            skip = sharedSkip;
            if (!skip) {
                b = 1;
            }
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 128, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = bda_buffer.Address();
    in_buffer_ptr[1] = 0;  // set SSBO.a to be zero

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, Atomics) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #pragma use_vulkan_memory_model
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer Node { uint x[]; };
        layout(set=0, binding=0) buffer SSBO {
            Node node;
            uint non_bda;
        };

        void main() {
            uint a = atomicLoad(node.x[8], gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            uint b = atomicLoad(non_bda, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);

            atomicStore(node.x[6], 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            atomicStore(non_bda, a + b, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);

            atomicExchange(node.x[2], node.x[4]);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 128, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = bda_buffer.Address();

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, Atomics2) {
    TEST_DESCRIPTION("Use BDA of the Pointer and Value operand of Atomics");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer SSBO {
            uint x;
            uint y;
            int z;
            uint w;
        };

        layout(set = 0, binding = 0) buffer SSBO_IN {
            SSBO a;
            uint b;
            int c;
        };
        void main() {
            // These use BDA on the pointer
            atomicAdd(a.x, 1); // OpAtomicIAdd
            atomicMax(a.y, 1); // OpAtomicUMax
            atomicMax(a.z, 1); // OpAtomicSMax
            atomicOr(a.w, 1);  // OpAtomicOr

            // These use BDA on the value
            // Will have a normal OpLoad before using in the atomic
            atomicAdd(b, a.x);
            atomicMax(b, a.y);
            atomicMax(c, a.z);
            atomicOr(b, a.w);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = bda_buffer.Address();

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, AtomicsWorkgroups) {
    TEST_DESCRIPTION("Found case where a potential BDA points to a variable not in the function");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450

        shared int x;

        layout(set = 0, binding = 0) buffer SSBO {
            int y;
        };

        void main() {
            atomicAdd(x, 1);
            y = x;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveGpuAVBufferDeviceAddress, PieceOfDataPointer) {
    TEST_DESCRIPTION("Slang can have a BDA pointer of a int that is not wrapped in a struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // RWStructuredBuffer<uint> result;
    // struct Data{
    //    uint* node;
    // };
    // [[vk::push_constant]] Data pc;
    // [shader("compute")]
    // void main(uint3 threadId : SV_DispatchThreadID) {
    //     result[0] = pc.node[1];
    // }
    char const *shader_source = R"(
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %result %pc
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %result Binding 0
               OpDecorate %result DescriptorSet 0
               OpDecorate %_ptr_PhysicalStorageBuffer_uint ArrayStride 4
               OpDecorate %Data_std430 Block
               OpMemberDecorate %Data_std430 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
%Data_std430 = OpTypeStruct %_ptr_PhysicalStorageBuffer_uint
%_ptr_PushConstant_Data_std430 = OpTypePointer PushConstant %Data_std430
%_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_uint
     %int_1 = OpConstant %int 1
     %result = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
         %pc = OpVariable %_ptr_PushConstant_Data_std430 PushConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
          %9 = OpAccessChain %_ptr_StorageBuffer_uint %result %int_0 %int_0
         %19 = OpAccessChain %_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint %pc %int_0
         %20 = OpLoad %_ptr_PhysicalStorageBuffer_uint %19
         %21 = OpPtrAccessChain %_ptr_PhysicalStorageBuffer_uint %20 %int_1
         %23 = OpLoad %uint %21 Aligned 4
               OpStore %9 %23
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto bda_buffer_ptr = static_cast<uint32_t *>(bda_buffer.Memory().Map());
    bda_buffer_ptr[0] = 33;
    bda_buffer_ptr[1] = 66;
    bda_buffer_ptr[2] = 99;

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress)};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {pc_range});

    descriptor_set.WriteDescriptorBufferInfo(0, out_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    VkDeviceAddress bda_buffer_addr = bda_buffer.Address();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress),
                         &bda_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    auto out_buffer_ptr = static_cast<uint32_t *>(out_buffer.Memory().Map());
    ASSERT_TRUE(out_buffer_ptr[0] == 66);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, PieceOfDataPointerInStruct) {
    TEST_DESCRIPTION("Slang can have a BDA pointer of a int that is not wrapped in a struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // RWStructuredBuffer<uint> result;
    // struct Foo {
    //     uint pad_0;
    //     float3 pad_1;
    //     uint* a; // offset 48 (16 + 32)
    // }
    //
    // struct Data{
    //    float4 pad_2;
    //    Foo node;
    // };
    // [[vk::push_constant]] Data pc;
    //
    // [shader("compute")]
    // void main(uint3 threadId : SV_DispatchThreadID) {
    //     result[0] = *pc.node.a;
    // }
    char const *shader_source = R"(
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %result %pc
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %result Binding 0
               OpDecorate %result DescriptorSet 0
               OpDecorate %_ptr_PhysicalStorageBuffer_uint ArrayStride 4
               OpMemberDecorate %Foo_std430 0 Offset 0
               OpMemberDecorate %Foo_std430 1 Offset 16
               OpMemberDecorate %Foo_std430 2 Offset 32
               OpDecorate %Data_std430 Block
               OpMemberDecorate %Data_std430 0 Offset 0
               OpMemberDecorate %Data_std430 1 Offset 16
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
 %Foo_std430 = OpTypeStruct %uint %v3float %_ptr_PhysicalStorageBuffer_uint
%Data_std430 = OpTypeStruct %v4float %Foo_std430
%_ptr_PushConstant_Data_std430 = OpTypePointer PushConstant %Data_std430
      %int_1 = OpConstant %int 1
%_ptr_PushConstant_Foo_std430 = OpTypePointer PushConstant %Foo_std430
      %int_2 = OpConstant %int 2
%_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_uint
     %result = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
         %pc = OpVariable %_ptr_PushConstant_Data_std430 PushConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
          %9 = OpAccessChain %_ptr_StorageBuffer_uint %result %int_0 %int_0
         %24 = OpAccessChain %_ptr_PushConstant_Foo_std430 %pc %int_1
         %27 = OpAccessChain %_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint %24 %int_2
         %28 = OpLoad %_ptr_PhysicalStorageBuffer_uint %27
         %29 = OpLoad %uint %28 Aligned 4
               OpStore %9 %29
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto bda_buffer_ptr = static_cast<uint32_t *>(bda_buffer.Memory().Map());
    bda_buffer_ptr[0] = 33;

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, 64};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {pc_range});

    descriptor_set.WriteDescriptorBufferInfo(0, out_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    VkDeviceAddress bda_buffer_addr = bda_buffer.Address();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 48, sizeof(VkDeviceAddress),
                         &bda_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    auto out_buffer_ptr = static_cast<uint32_t *>(out_buffer.Memory().Map());
    ASSERT_TRUE(out_buffer_ptr[0] == 33);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, SharedPipelineLayoutSubsetGraphicsPushConstants) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Create 2 pipeline layouts. Pipeline layout 2 starts the same as pipeline layout 1, with one push constant range,
    // but one more push constant range is added to it, for a total of 2.
    // The descriptor set layout of both pipeline layout are empty, thus compatible
    // GPU-AV should work as expected.

    std::array<VkPushConstantRange, 2> push_constant_ranges;
    push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_ranges[0].offset = 0;
    push_constant_ranges[0].size = sizeof(VkDeviceAddress) + 2 * sizeof(uint32_t);
    push_constant_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_ranges[1].offset = push_constant_ranges[0].size;
    push_constant_ranges[1].size = sizeof(uint32_t);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = push_constant_ranges.data();
    auto pipeline_layout_1 = std::make_unique<vkt::PipelineLayout>(*m_device, pipeline_layout_ci);

    pipeline_layout_ci.pushConstantRangeCount = 2;
    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) buffer Ptr {
            uint i;
        };

        layout(push_constant, std430) uniform foo_0 {
            Ptr ptr;
            uint a;
            uint b;
        };
        void main() {
            ptr.i = a + b;
        }
    )glsl";

    char const *fs_source = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo_1 { uint c; };
        void main() {}
        )glsl";

    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe_1(*this);
    pipe_1.shader_stages_ = {vs.GetStageCreateInfo(), pipe_1.fs_->GetStageCreateInfo()};
    pipe_1.gp_ci_.layout = pipeline_layout_1->handle();
    pipe_1.CreateGraphicsPipeline();
    pipeline_layout_1 = nullptr;

    CreatePipelineHelper pipe_2(*this);
    pipe_2.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe_2.gp_ci_.layout = pipeline_layout_2.handle();
    pipe_2.CreateGraphicsPipeline();

    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    std::array<VkDeviceAddress, 3> push_constants_data = {{out_buffer.Address(), VkDeviceAddress(2) << 32 | 1u, 3}};

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    const uint32_t pc_1_size = uint32_t(sizeof(VkDeviceAddress) + 2 * sizeof(uint32_t));
    const auto pipeline_layout_2_handle = pipeline_layout_2.handle();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout_2_handle, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_1_size,
                         &push_constants_data[0]);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout_2_handle, VK_SHADER_STAGE_FRAGMENT_BIT, pc_1_size, sizeof(uint32_t),
                         &push_constants_data[2]);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.Handle());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, SharedPipelineLayoutSubsetGraphicsPushConstantsShaderObject) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitDynamicRenderTarget();

    // Create 2 pipeline layouts. Pipeline layout 2 starts the same as pipeline layout 1, with one push constant range,
    // but one more push constant range is added to it, for a total of 2.
    // The descriptor set layout of both pipeline layout are empty, thus compatible
    // GPU-AV should work as expected.

    std::array<VkPushConstantRange, 2> push_constant_ranges;
    push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_ranges[0].offset = 0;
    push_constant_ranges[0].size = sizeof(VkDeviceAddress) + 2 * sizeof(uint32_t);
    push_constant_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_ranges[1].offset = push_constant_ranges[0].size;
    push_constant_ranges[1].size = sizeof(uint32_t);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 2;
    pipeline_layout_ci.pPushConstantRanges = push_constant_ranges.data();
    const vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    char const *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) buffer Ptr {
            uint i;
        };

        layout(push_constant, std430) uniform foo_0 {
            Ptr ptr;
            uint a;
            uint b;
        };
        void main() {
            ptr.i = a + b;
        }
    )glsl";

    char const *fs_source = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo_1 { uint c; };
        void main() {}
    )glsl";

    const std::vector<uint32_t> vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source);
    const std::vector<uint32_t> fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkShaderCreateInfoEXT shader_obj_ci = vku::InitStructHelper();
    shader_obj_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_obj_ci.nextStage = 0;
    shader_obj_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    shader_obj_ci.codeSize = vs_spv.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = vs_spv.data();
    shader_obj_ci.pName = "main";
    shader_obj_ci.pushConstantRangeCount = 1;
    shader_obj_ci.pPushConstantRanges = push_constant_ranges.data();
    vkt::Shader vs_1(*m_device, shader_obj_ci);

    shader_obj_ci.pushConstantRangeCount = 2;
    shader_obj_ci.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkt::Shader vs_2(*m_device, shader_obj_ci);

    shader_obj_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_obj_ci.nextStage = 0u;
    shader_obj_ci.codeSize = fs_spv.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = fs_spv.data();
    shader_obj_ci.pushConstantRangeCount = 2;
    vkt::Shader fs(*m_device, shader_obj_ci);

    const std::array<VkShaderStageFlagBits, 5> stages = {{VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                          VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                          VK_SHADER_STAGE_FRAGMENT_BIT}};
    const std::array<VkShaderEXT, 5> shaders_1 = {{vs_1, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}};
    const std::array<VkShaderEXT, 5> shaders_2 = {{vs_2, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fs}};

    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    std::array<VkDeviceAddress, 3> push_constants_data = {{out_buffer.Address(), VkDeviceAddress(2) << 32 | 1u, 3}};

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    const uint32_t pc_1_size = uint32_t(sizeof(VkDeviceAddress) + 2 * sizeof(uint32_t));
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, pc_1_size, &push_constants_data[0]);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, pc_1_size, sizeof(uint32_t),
                         &push_constants_data[2]);
    SetDefaultDynamicStatesAll(m_command_buffer);

    vk::CmdBindShadersEXT(m_command_buffer, size32(stages), stages.data(), shaders_2.data());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    vk::CmdBindShadersEXT(m_command_buffer, size32(stages), stages.data(), shaders_1.data());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    vk::CmdBindShadersEXT(m_command_buffer, size32(stages), stages.data(), shaders_2.data());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, PointerChain) {
    TEST_DESCRIPTION("Have BDA point to more BDA creating a chain");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer SSBO_A {
            uint x;
        };
        layout(buffer_reference) buffer SSBO_B {
            SSBO_A a;
            uint y;
        };
        layout(buffer_reference) buffer SSBO_C {
            SSBO_B b;
            uint z;
        };

        layout(set = 0, binding = 0) uniform UBO_IN {
            SSBO_C c;
        };

        void main() {
           c.b.a.x = 42;
        }
    )glsl";
    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer ssbo_a_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ssbo_b_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ssbo_c_buffer(*m_device, 64, 0, vkt::device_address);

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    auto buffer_ptr = (VkDeviceAddress *)in_buffer.Memory().Map();
    buffer_ptr[0] = ssbo_c_buffer.Address();

    buffer_ptr = (VkDeviceAddress *)ssbo_c_buffer.Memory().Map();
    buffer_ptr[0] = ssbo_b_buffer.Address();

    buffer_ptr = (VkDeviceAddress *)ssbo_b_buffer.Memory().Map();
    buffer_ptr[0] = ssbo_a_buffer.Address();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    auto out_buffer_ptr = (uint32_t *)ssbo_a_buffer.Memory().Map();
    ASSERT_TRUE(out_buffer_ptr[0] == 42);
}

TEST_F(PositiveGpuAVBufferDeviceAddress, ManyAccessToSameStruct) {
    TEST_DESCRIPTION("Used to mimic cases where apps have 100s of the same BDA instrumented.");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, buffer_reference_align = 16, std430) buffer BDA {
            uvec4 payload[16]; // some traces this is 8k large
            uint x;
        };

        layout(push_constant) uniform Uniforms {
            BDA ptr0;
            BDA ptr1;
        };

        void main() {
            uint a = 0;
            a += ptr0.payload[2].x + ptr0.payload[2].y;
            a += ptr0.payload[3].x + ptr0.payload[3].y;
            a += ptr0.payload[4].x + ptr0.payload[4].y;
            a += ptr0.payload[6].x + ptr0.payload[6].y;
            a += ptr0.payload[8].x + ptr0.payload[8].y;
            a += ptr0.payload[7].x + ptr0.payload[7].y;

            a += ptr1.payload[6].x + ptr1.payload[6].y;
            a += ptr1.payload[10].x + ptr1.payload[10].y;
            a += ptr1.payload[8].x + ptr1.payload[8].y;

            ptr0.x = a;
        }
    )glsl";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress) * 2};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {pc_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 1024, 0, vkt::device_address);
    auto bda_buffer_addr = bda_buffer.Address();

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress),
                         &bda_buffer_addr);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(VkDeviceAddress),
                         sizeof(VkDeviceAddress), &bda_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

// Used to test large accesses to a single struct from a single pointer
// If on Mesa, also add MESA_SHADER_CACHE_DISABLE=1
TEST_F(PositiveGpuAVBufferDeviceAddress, Stress) {
    // About a 5x speed up to run in unsafe mode
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress(false));
    InitRenderTarget();
    const uint32_t count = 32;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer BDA {
            vec3 x;
            vec3 payload[4096];
        };

        layout(push_constant) uniform Uniforms {
            BDA ptr;
        };

        void main() {
            vec3 a = vec3(0);
            // a += fma(vec3(ptr.payload[0].x, ptr.payload[0].y, ptr.payload[0].z),
            //          vec3(ptr.payload[1].x, ptr.payload[1].y, ptr.payload[1].z),
            //          vec3(ptr.payload[2].x, ptr.payload[2].y, ptr.payload[2].z));
            //
            // .... many times
            //
            // ptr.x = a;
    )glsl";

    for (uint32_t i = 0; i < count; i += 3) {
        cs_source << "a += fma(vec3(ptr.payload[" << i << "].x, ptr.payload[" << i << "].y, ptr.payload[" << i << "].z), ";
        cs_source << "vec3(ptr.payload[" << i + 1 << "].x, ptr.payload[" << i + 1 << "].y, ptr.payload[" << i + 1 << "].z), ";
        cs_source << "vec3(ptr.payload[" << i + 2 << "].x, ptr.payload[" << i + 2 << "].y, ptr.payload[" << i + 2 << "].z));\n";
    }
    cs_source << "\nptr.x = a;\n}";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress)};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {pc_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveGpuAVBufferDeviceAddress, GlobalInvocationIdIVec3) {
    TEST_DESCRIPTION("Found issue when we assumed GlobalInvocationId was a uvec3 and it was a ivec3");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Looks like the following, but we force the ivec3 over a uvec3
    //
    // layout(buffer_reference) buffer BDA { int x; };
    // void main() {
    //     ptr.x = int(gl_GlobalInvocationID.x);
    // }
    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %Uniforms Block
               OpMemberDecorate %Uniforms 0 Offset 0
               OpDecorate %BDA Block
               OpMemberDecorate %BDA 0 Offset 0
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_BDA PhysicalStorageBuffer
   %Uniforms = OpTypeStruct %_ptr_PhysicalStorageBuffer_BDA
        %int = OpTypeInt 32 1
        %BDA = OpTypeStruct %int
%_ptr_PhysicalStorageBuffer_BDA = OpTypePointer PhysicalStorageBuffer %BDA
%_ptr_PushConstant_Uniforms = OpTypePointer PushConstant %Uniforms
          %_ = OpVariable %_ptr_PushConstant_Uniforms PushConstant
      %int_0 = OpConstant %int 0
%_ptr_PushConstant__ptr_PhysicalStorageBuffer_BDA = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_BDA
       %uint = OpTypeInt 32 0
     %v3int = OpTypeVector %int 3
%_ptr_Input_v3uint = OpTypePointer Input %v3int
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %15 = OpAccessChain %_ptr_PushConstant__ptr_PhysicalStorageBuffer_BDA %_ %int_0
         %16 = OpLoad %_ptr_PhysicalStorageBuffer_BDA %15
         %23 = OpAccessChain %_ptr_Input_int %gl_GlobalInvocationID %uint_0
         %24 = OpLoad %int %23
         %27 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %16 %int_0
               OpStore %27 %24 Aligned 16
               OpReturn
               OpFunctionEnd
    )";
    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress)};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {pc_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveGpuAVBufferDeviceAddress, DualShaderLibrary) {
    TEST_DESCRIPTION("Create library with both vert and frag shader in it");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer BDA {
            vec4 x;
        };

        layout(set = 0, binding = 0) buffer SSBO {
            BDA bda;
        };

        layout(location = 0) out vec4 uFragColor;
        void main() {
            uFragColor = bda.x;
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper combined_lib(*this);
    combined_lib.gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>());
    combined_lib.gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                                   VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                   VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    combined_lib.gp_ci_ = vku::InitStructHelper(&combined_lib.gpl_info);
    combined_lib.gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    combined_lib.gp_ci_.pVertexInputState = &combined_lib.vi_ci_;
    combined_lib.gp_ci_.pInputAssemblyState = &combined_lib.ia_ci_;
    combined_lib.gp_ci_.pViewportState = &combined_lib.vp_state_ci_;
    combined_lib.gp_ci_.pRasterizationState = &combined_lib.rs_state_ci_;
    combined_lib.gp_ci_.pMultisampleState = &combined_lib.ms_ci_;
    combined_lib.gp_ci_.renderPass = RenderPass();
    combined_lib.gp_ci_.subpass = 0;
    combined_lib.gp_ci_.layout = pipeline_layout;

    combined_lib.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    combined_lib.gp_ci_.stageCount = combined_lib.shader_stages_.size();
    combined_lib.gp_ci_.pStages = combined_lib.shader_stages_.data();

    combined_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[2] = {
        combined_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGpuAVBufferDeviceAddress, DualShaderLibraryDestroyModule) {
    TEST_DESCRIPTION("Create library with both vert and frag shader in it, but destroy module before pipeline");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer BDA {
            vec4 x;
        };

        layout(set = 0, binding = 0) buffer SSBO {
            BDA bda;
        };

        layout(location = 0) out vec4 uFragColor;
        void main() {
            uFragColor = bda.x;
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper combined_lib(*this);
    combined_lib.gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>());
    combined_lib.gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                                   VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                   VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    combined_lib.gp_ci_ = vku::InitStructHelper(&combined_lib.gpl_info);
    combined_lib.gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    combined_lib.gp_ci_.pVertexInputState = &combined_lib.vi_ci_;
    combined_lib.gp_ci_.pInputAssemblyState = &combined_lib.ia_ci_;
    combined_lib.gp_ci_.pViewportState = &combined_lib.vp_state_ci_;
    combined_lib.gp_ci_.pRasterizationState = &combined_lib.rs_state_ci_;
    combined_lib.gp_ci_.pMultisampleState = &combined_lib.ms_ci_;
    combined_lib.gp_ci_.renderPass = RenderPass();
    combined_lib.gp_ci_.subpass = 0;
    combined_lib.gp_ci_.layout = pipeline_layout;

    combined_lib.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    combined_lib.gp_ci_.stageCount = combined_lib.shader_stages_.size();
    combined_lib.gp_ci_.pStages = combined_lib.shader_stages_.data();

    combined_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[2] = {
        combined_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    // Destroy VkShaderModule as not required to have when linking
    vs.destroy();
    fs.destroy();

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}
