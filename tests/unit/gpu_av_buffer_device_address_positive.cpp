/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2024 Google, Inc.
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
#include "../framework/gpu_av_helper.h"

void GpuAVBufferDeviceAddressTest::InitGpuVUBufferDeviceAddress(void *p_next) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
}

TEST_F(PositiveGpuAVBufferDeviceAddress, Basic) {
    TEST_DESCRIPTION("Makes sure that writing to a buffer that was created after command buffer record doesn't get OOB error");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    uint32_t buffer_size = 12;  // 64 bit pointer + int
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

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
                u_info.data.a[i] = 0xdeadca71;
            }
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer0.handle(), 0, sizeof(uint32_t));
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Make another buffer to write to
    buffer_size = 64;  // Buffer should be 16*4 = 64 bytes
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer1(*m_device, buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    // Get device address of buffer to write to
    auto pBuffer = buffer1.address();

    auto *data = static_cast<VkDeviceAddress *>(buffer0.memory().map());
    data[0] = pBuffer;
    data[1] = 4;
    buffer0.memory().unmap();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveGpuAVBufferDeviceAddress, Store) {
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
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer bda_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    VkDeviceAddress buffer_ptr = bda_buffer.address();
    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &buffer_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();

    uint8_t *bda_buffer_ptr = (uint8_t *)bda_buffer.memory().map();
    uint32_t output = *((uint32_t *)(bda_buffer_ptr + 32));
    bda_buffer.memory().unmap();
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
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 16, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    float expected_output = 0x00EEAADD;
    uint8_t *block_buffer_ptr = (uint8_t *)block_buffer.memory().map();
    memcpy(block_buffer_ptr, &expected_output, sizeof(float));
    block_buffer.memory().unmap();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    VkDeviceAddress block_ptr = block_buffer.address();

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();

    in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    float output = *((float *)(in_buffer_ptr + sizeof(VkDeviceAddress)));
    in_buffer.memory().unmap();
    ASSERT_TRUE(output == expected_output);
}

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
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 32, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    float expected_output = 0x00EEAADD;
    uint8_t *block_buffer_ptr = (uint8_t *)block_buffer.memory().map();
    memcpy(block_buffer_ptr + 24, &expected_output, sizeof(float));
    block_buffer.memory().unmap();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    VkDeviceAddress block_ptr = block_buffer.address();

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();

    in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    float output = *((float *)(in_buffer_ptr + sizeof(VkDeviceAddress)));
    in_buffer.memory().unmap();
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

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    // Hold 4 indices
    vkt::Buffer block_buffer(*m_device, 48, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDeviceAddress block_ptr = block_buffer.address();
    const uint32_t n_reads = 4;  // uvec3[0] to uvec3[3]

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    memcpy(in_buffer_ptr + sizeof(VkDeviceAddress), &n_reads, sizeof(uint32_t));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7462
TEST_F(PositiveGpuAVBufferDeviceAddress, DISABLED_ArrayOfStruct) {
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
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 32, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);
    VkDeviceAddress block_ptr = block_buffer.address();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    uint8_t *buffer_ptr = (uint8_t *)storage_buffer.memory().map();
    const uint32_t index = 0;
    memcpy(buffer_ptr, &index, sizeof(uint32_t));
    memcpy(buffer_ptr + (1 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (2 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (3 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    storage_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, storage_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}